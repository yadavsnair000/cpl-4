#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct block
{
    struct block* sno;
    int size;
    struct block* next;

}Block;

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
    freed_end->size = 10000 + sizeof(Block);
    freed_end->sno = NULL;
}

int main()
{
    initialise();
    print_lists();
    int * x = (int *)allocate_memory(sizeof(int)*20);//104
    print_lists();
    int * y = (int *)allocate_memory(sizeof(int)*30);//144
    print_lists();
    int * z = (int *)allocate_memory(sizeof(int)*40);//184
    print_lists();
    //int * k = (int *)allocate_memory(sizeof(int)*1);
    //print_lists();
    //free_block((Block *)x);
    free_block((Block *)y);
    print_lists();
    int * k = (int *)allocate_memory(sizeof(int)*7);//52
    print_lists();
    free_block((Block *)x);
    print_lists();
    int * m = (int *)allocate_memory(sizeof(int)*8);//56
    print_lists();
    free_block((Block *)z);
    print_lists();
    free_block((Block *)k);
    print_lists();
    free_block((Block *)m);
    print_lists();
    return 0;
}
