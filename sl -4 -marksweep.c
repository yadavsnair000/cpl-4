#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

int * val1;
int * val2;

typedef struct TreeNode
{
    char name;
    struct TreeNode * left;
    struct TreeNode * right;
}TNode;

typedef struct block
{
    struct block* sno;
    int size;
    struct block* next;
    int mark;

}Block;

Block * root[200000];
int root_ct = 0;
Block * allocated_end;
Block * allocated_start;
Block * freed_end;
Block * freed_start;

void* allocate_memory(size_t val)
{
    Block * l;
    Block * p;
    Block * y;
    l = freed_start;
    //Best fit algorithm
    while (l!=NULL && l->size < val + sizeof(Block))
    {
        l = l->next;
    }
    p = l;
    while (p!=NULL)
    {
        if (p->size < l->size && p->size >= val + sizeof(Block))
        {
            l = p;
        }
        p = p->next;
    }
    if (l!=NULL)
    {
        if (l->size - val > 2*sizeof(Block))        //l->size - sizeof(Block) > val + sizeof(Block)
        {
            int v;
            v = l->size - (val + sizeof(Block));
            l->size = v;
            Block * n = (Block *)(((void *)l) + v + sizeof(Block));
            p = l->sno;
            l->sno = n;
            n->size = val + sizeof(Block);
            n->next = NULL;
            n->sno = p;
            if (allocated_end == NULL)
            {
                allocated_end = n;
                allocated_start = n;
            }
            else
            {
                allocated_end->next = n;
                allocated_end = n;
            }
            return ((void *)n + sizeof(Block));
        }
        else if ((l->size - val <= 2*sizeof(Block)) && freed_start != l)        //l->size - sizeof(Block) <= val + sizeof(Block)
        {
            p = freed_start;
            while (p->next!=l)
            {
                p = p->next;
            }
            p->next = p->next->next;
            if (allocated_end == NULL)
            {
                allocated_end = l;
                allocated_start = l;
            }
            else
            {
                allocated_end->next = l;
                allocated_end = l;
            }
            return ((void *)l + sizeof(Block));
        }
        else
        {
            printf("Heap is full");
            return NULL;
        }
    }
    else
    {
        printf("Heap full");
        return NULL;
    }
}

void free_block(Block * l)
{
    l = (Block *)(((void *)l) - sizeof(Block));
    l->mark = -1;
    Block * t;
    Block * y;
    t = allocated_start;
    if (allocated_start == l)
    {
        allocated_start = allocated_start->next;
        l->next = NULL;
    }
    else
    {
        while (t!=NULL && t->next != l)
        {
            t = t->next;
        }
        t->next = t->next->next;
        l->next = NULL;
    }
    if (t==NULL)
    {
        printf("Failed: No such allocation ");
    }
    else
    {
        t = freed_start;
        if (t == NULL)
        {
            l->next = freed_start;
            freed_start = l;
        }
        else
        {
            int k=0;
            y = freed_start;
            Block * prev = freed_start;
            while (y!=NULL && k==0)
            {
                if (y == l)
                {
                    k = 1;
                }
                else
                {
                    if (t==y)
                    {
                        prev = t;
                        t = t->next;
                    }
                    y = y->sno;
                }
            }
            y = prev->next;
            prev->next = l;
            l->next = y;
            y = freed_start;
            while (y->next!=NULL)
            {
                if (y->sno == y->next)
                {
                    y->size += y->next->size;
                    y->next = y->next->next;
                    y->sno = y->sno->sno;
                }
                else
                {
                    y = y->next;
                }
            }
        }
    }
}

void print_lists()
{
    Block * l;
    l = freed_start;
    printf("Free list: ");
    while (l!=NULL)
    {
        printf("%d ",l->size);
        l = l->next;
    }
    printf("\n");
    l = freed_start;
    printf("All blocks: ");
    while (l!=NULL)
    {
        printf("%d ",l->size);
        l = l->sno;
    }
    printf("\n");
    l = allocated_start;
    printf("Allocated list: ");
    while (l!=NULL)
    {
        printf("%d ",l->size);
        l = l->next;
    }
    printf("\n\n");
}
void initialise()
{
    allocated_end = NULL;
    freed_end = (Block *)malloc(sizeof(Block) + 10000);
    allocated_start = allocated_end;
    freed_start = freed_end;
    freed_end->next = NULL;
    freed_end->mark = -1;
    freed_end->size = 10000 + sizeof(Block);
    freed_end->sno = NULL;
}

int search_LL(Block * b)
{
    Block * l;
    l = allocated_start;
    while (l!=NULL)
    {
        if (l == b)
        {
            return 1;
        }
        l = l->next;
    }
    return 0;
}

void set_mark_to_zero()
{
    Block * l;
    l = allocated_start;
    while (l!=NULL)
    {
        l->mark = 0;
        l = l->next;
    }
}

void mark()
{
    set_stack();
    int i = 0;
    while (i<root_ct)
    {
        if (root[i]!=NULL)
        {
            Block * ptr;
            ptr = (Block *)((void *)root[i] - sizeof(Block));
            ptr->mark = 1;
            void * s = (void *)ptr + sizeof(Block);
            void * e = s + (ptr->size - sizeof(Block)) + 1;
            for (; s<e; s++)
            {
                Block * p;
                p = ((Block **)s)[0];
                p = (Block *)((void *)p - sizeof(Block));
                if (search_LL(p) == 1)
                {
                    root[root_ct++] = ((Block **)s)[0];
                }
            }
        }
        i++;
    }
}

void sweep()
{
    Block * l;
    Block * y;
    l = allocated_start;
    while (l!=NULL)
    {
        if (l->mark == 0)
        {
            y = (Block *)((void *)l + sizeof(Block));
            l = l->next;
            free_block(y);
        }
        else
        {
            l = l->next;
        }
    }
    print_lists();
}

void print_stack()
{
    int i=0;
    while (i<root_ct)
    {
        printf("%lld ",root[i]);
        i++;
    }
    printf("\n");
}

void mark_sweep_gc()
{
    set_mark_to_zero();
    mark();
    sweep();
}


TNode * createGraph() {
    TNode * H = (TNode *)allocate_memory(sizeof(TNode));
    H->name = 'H';
    H->left = NULL;
    H->right = NULL;
    TNode * G = (TNode *)allocate_memory(sizeof(TNode));
    G->name = 'G';
    G->left = NULL;
    G->right = H;
    TNode * F =(TNode *)allocate_memory(sizeof(TNode));
    F->name = 'F';
    F->left = NULL;
    F->right = NULL;
    TNode * E = (TNode *)allocate_memory(sizeof(TNode));
    E->name = 'E';
    E->left = F;
    E->right = G;
    TNode * D = (TNode *)allocate_memory(sizeof(TNode));
    D->name = 'D';
    D->left = NULL;
    D->right = NULL;
    TNode * C = (TNode *)allocate_memory(sizeof(TNode));
    C->name = 'C';
    C->left = D;
    C->right = E;
    TNode * B = (TNode *)allocate_memory(sizeof(TNode));
    B->name = 'B';
    B->left = NULL;
    B->right = NULL;
    TNode * A = (TNode *)allocate_memory(sizeof(TNode));
    A->name = 'A';
    A->left = B;
    A->right = C;
    C->right = NULL;
    printf("Nodes of the tree at memory locations(in heap): \n");
    printf("A:%lld \nB:%lld \nC:%lld \nD:%lld \nE:%lld \nF:%lld \nG:%lld \nH:%lld\n\n",A,B,C,D,E,F,G,H);
    return A;  // Root
}

void print_adjacency()
{
    int i;
    set_stack();
    while (i<root_ct)
    {
        if (root[i]!=NULL)
        {
            Block * ptr;
            ptr = (Block *)((void *)root[i] - sizeof(Block));
            ptr->mark = 1;
            void * s = (void *)ptr + sizeof(Block);
            void * e = s + (ptr->size - sizeof(Block)) + 1;
            printf("%lld ",root[i]);
            for (; s<e; s++)
            {
                Block * p;
                p = ((Block **)s)[0];
                p = (Block *)((void *)p - sizeof(Block));
                if (search_LL(p) == 1)
                {
                    printf("--> %lld ",((Block **)s)[0]);
                    root[root_ct++] = ((Block **)s)[0];
                }
            }
            printf("\n");
        }
        i++;
    }
}
void set_stack()
{
    void * l;
    l = (void *)val2;
    root_ct = 0;
    while (l<(void *)val1)
    {
        Block * p;
        p = ((Block **)l)[0];
        p = p - 1;
        if (search_LL(p) == 1)
        {
            p->mark = 1;
            root[root_ct++] = ((Block **)l)[0];
        }
        l++;
    }
}

int main()
{
    int dum1;
    val1 = &dum1;
    initialise();
    TNode * n;
    printf("Tree Root pointed by pointer(in stack): %lld \n",(&n));
    n = createGraph();
    int * dum2 = &dum1;
    val2 = &dum2;
    set_stack();
    print_stack();
    print_adjacency();
    print_lists();
    mark_sweep_gc();
    print_adjacency();
    return 0;
}
