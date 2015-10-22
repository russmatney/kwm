#include "kwm.h"

extern std::vector<window_info> WindowLst;

node_container LeftVerticalContainerSplit(screen_info *Screen, tree_node *Node)
{
    node_container LeftContainer;

    LeftContainer.X = Node->Container.X;
    LeftContainer.Y = Node->Container.Y;
    LeftContainer.Width = (Node->Container.Width / 2) - (Screen->PaddingLeft / 2);
    LeftContainer.Height = Node->Container.Height;
    
    return LeftContainer;
}

node_container RightVerticalContainerSplit(screen_info *Screen, tree_node *Node)
{
    node_container RightContainer;

    RightContainer.X = Node->Container.X + (Node->Container.Width / 2);
    RightContainer.Y = Node->Container.Y;
    RightContainer.Width = Node->Container.Width / 2;
    RightContainer.Height = Node->Container.Height;

    return RightContainer;
}

node_container UpperHorizontalContainerSplit(screen_info *Screen, tree_node *Node)
{
    node_container UpperContainer;

    UpperContainer.X = Node->Container.X;
    UpperContainer.Y = Node->Container.Y;
    UpperContainer.Width = Node->Container.Width;
    UpperContainer.Height = Node->Container.Height / 2;

    return UpperContainer;
}

node_container LowerHorizontalContainerSplit(screen_info *Screen, tree_node *Node)
{
    node_container LowerContainer;

    LowerContainer.X = Node->Container.X;
    LowerContainer.Y = Node->Container.Y + ((Node->Container.Height + (Screen->PaddingTop / 2)) / 2);
    LowerContainer.Width = Node->Container.Width;
    LowerContainer.Height = Node->Container.Height / 2;

    return LowerContainer;
}

node_container FullscreenContainer(screen_info *Screen, tree_node *Node)
{
    node_container FullscreenContainer;

    FullscreenContainer.X = Node->Container.X;
    FullscreenContainer.Y = Node->Container.Y;
    FullscreenContainer.Width = Node->Container.Width - Screen->PaddingRight;
    FullscreenContainer.Height = Node->Container.Height;

    return FullscreenContainer;
}

tree_node *CreateLeafNode(screen_info *Screen, tree_node *Parent, int WindowID, int ContainerType)
{
    tree_node *Leaf = (tree_node*) malloc(sizeof(tree_node));
    Leaf->Parent = Parent;
    Leaf->WindowID = WindowID;

    if(ContainerType == 0)
        Leaf->Container = FullscreenContainer(Screen, Leaf->Parent);
    else if(ContainerType == 1)
        Leaf->Container = LeftVerticalContainerSplit(Screen, Leaf->Parent);
    else if(ContainerType == 2)
        Leaf->Container = RightVerticalContainerSplit(Screen, Leaf->Parent);
    else if(ContainerType == 3)
        Leaf->Container = UpperHorizontalContainerSplit(Screen, Leaf->Parent);
    else if(ContainerType == 4)
        Leaf->Container = LowerHorizontalContainerSplit(Screen, Leaf->Parent);

    Leaf->LeftChild = NULL;
    Leaf->RightChild = NULL;

    return Leaf;
}

tree_node *CreateRootNode()
{
    tree_node *RootNode = (tree_node*) malloc(sizeof(tree_node));
    std::memset(RootNode, '\0', sizeof(tree_node));

    RootNode->WindowID = -1;
    RootNode->Parent = NULL;
    RootNode->LeftChild = NULL;
    RootNode->RightChild = NULL;

    return RootNode;
}

void SetRootNodeContainer(tree_node *Node, int X, int Y, int Width, int Height)
{
    Node->Container.X = X;
    Node->Container.Y = Y,
    Node->Container.Width = Width;
    Node->Container.Height = Height;
}

void CreateLeafNodePair(screen_info *Screen, tree_node *Parent, int LeftWindowID, int RightWindowID, int SplitMode)
{
    if(SplitMode == 1)
    {
        // Vertical Split
        Parent->LeftChild = CreateLeafNode(Screen, Parent, LeftWindowID, 1);
        Parent->RightChild = CreateLeafNode(Screen, Parent, RightWindowID, 2);
    }
    else
    {
        // Horizontal Split
        Parent->LeftChild = CreateLeafNode(Screen, Parent, LeftWindowID, 3);
        Parent->RightChild = CreateLeafNode(Screen, Parent, RightWindowID, 4);
    }
}

tree_node *CreateTreeFromWindowIDList(screen_info *Screen, std::vector<int> Windows)
{
    tree_node *RootNode = CreateRootNode();
    SetRootNodeContainer(RootNode, Screen->PaddingLeft, Screen->PaddingTop,
                         Screen->Width - Screen->PaddingLeft - Screen->PaddingRight,
                         Screen->Height - Screen->PaddingTop - Screen->PaddingBottom);

    if(Windows.size() % 2 == 0)
    {
        // Even number of windows
        CreateLeafNodePair(Screen, RootNode, Windows[0], Windows[1], 1);
        for(int WindowIndex = 2; WindowIndex < Windows.size(); WindowIndex+=2)
        {
            tree_node *Left = RootNode->LeftChild;
            for(int Index = WindowIndex; Index < Windows.size(); Index+=2)
            {
                CreateLeafNodePair(Screen, Left, Left->WindowID, Windows[Index], 2);
                Left->WindowID = -1;
                Left = Left->LeftChild;
            }
            RootNode->LeftChild->WindowID = -1;

            tree_node *Right = RootNode->RightChild;
            for(int Index = WindowIndex + 1; Index < Windows.size(); Index+=2)
            {
                CreateLeafNodePair(Screen, Right, Right->WindowID, Windows[Index], 2);
                Right->WindowID = -1;
                Right = Right->LeftChild;
            }
            RootNode->RightChild->WindowID = -1;
        }
    }
    else
    {
        if(Windows.size() == 1)
        {
            RootNode->WindowID = Windows[0];
        }
        else
        {
            CreateLeafNodePair(Screen, RootNode, Windows[0], Windows[1], 1);
            for(int WindowIndex = 2; WindowIndex < Windows.size(); WindowIndex+=2)
            {
                tree_node *Right = RootNode->RightChild;
                for(int Index = WindowIndex; Index < Windows.size(); Index+=2)
                {
                    CreateLeafNodePair(Screen, Right, Right->WindowID, Windows[Index], 2);
                    Right->WindowID = -1;
                    Right = Right->LeftChild;
                }
                RootNode->RightChild->WindowID = -1;
            }
        }
    }

    return RootNode;
}

tree_node *GetNodeFromWindowID(tree_node *Node, int WindowID)
{
    tree_node *Result = NULL;

    if(Node)
    {
        if(Node->WindowID == WindowID)
        {
            DEBUG("GetNodeFromWindowID() " << WindowID)
            return Node;
        }

        if(Node->LeftChild)
        {
            Result = GetNodeFromWindowID(Node->LeftChild, WindowID);
            if(Result)
                return Result;

            Result = GetNodeFromWindowID(Node->RightChild, WindowID);
        }

        if(Node->RightChild)
        {
            GetNodeFromWindowID(Node->RightChild, WindowID);
            if(Result)
                return Result;

            GetNodeFromWindowID(Node->LeftChild, WindowID);
        }
    }

    DEBUG("GetNodeFromWindowID() NO MATCH")
    return Result;
}

tree_node *GetNearestNodeToTheLeft(tree_node *Node)
{
    if(Node)
    {
        if(Node->Parent)
        {
            tree_node *Root = Node->Parent;
            if(Root->LeftChild != Node)
            {
                if(Root->LeftChild->WindowID != -1)
                {
                    return Root->LeftChild;
                }
                else
                {
                    return Root->LeftChild->RightChild;
                }
            }
            else
            {
                return GetNearestNodeToTheLeft(Root);
            }
        }
    }

    return NULL;
}

tree_node *GetNearestNodeToTheRight(tree_node *Node)
{
    if(Node)
    {
        if(Node->Parent)
        {
            tree_node *Root = Node->Parent;
            if(Root->RightChild != Node)
            {
                if(Root->RightChild->WindowID != -1)
                {
                    return Root->RightChild;
                }
                else
                {
                    return Root->RightChild->LeftChild;
                }
            }
            else
            {
                return GetNearestNodeToTheRight(Root);
            }
        }
    }

    return NULL;
}

void ApplyNodeContainer(tree_node *Node)
{
    if(Node)
    {
        if(Node->LeftChild)
        {
            ApplyNodeContainer(Node->LeftChild);
        }

        if(Node->RightChild)
        {
            ApplyNodeContainer(Node->RightChild);
        }

        if(Node->WindowID != -1)
        {
            ResizeWindow(Node);
        }
    }
}

void DestroyNodeTree(tree_node *Node)
{
    if(Node)
    {
        if(Node->LeftChild)
        {
            DestroyNodeTree(Node->LeftChild);
        }

        if(Node->RightChild)
        {
            DestroyNodeTree(Node->RightChild);
        }

        free(Node);
    }
}
