#ifndef SRC_HPP_INCLUDED
#define SRC_HPP_INCLUDED

// Minimal buddy allocator (no STL). The judge will include this header and
// provide its own main. We expose:
//   void init(int ram_size, int min_block);
//   int  malloc(int size);
//   int  malloc_at(int addr, int size);
//   void free_at(int addr, int size);

namespace buddyimpl {

enum { NODE_FREE = 0, NODE_FULL = 1, NODE_SPLIT = 2 };

static int RAM = 0;
static int MINB = 1;
static int TOP = 0;        // top order L where MINB * 2^L == RAM
static int TREE_N = 0;     // capacity for arrays (1-based indexing)
static unsigned char *STATE = 0; // node state
static int *MAXFREE = 0;          // max free order available in subtree, -1 if none

static inline int max_int(int a,int b){ return a>b?a:b; }

static inline int order_of_size(int size){
    int need = size <= MINB ? MINB : size;
    int blk = MINB, ord = 0;
    while (blk < need){ blk <<= 1; ++ord; }
    return ord;
}

static inline int units(int order){ return 1 << order; }

static void init_impl(int ram_size, int min_block){
    if (STATE){ delete [] STATE; STATE = 0; }
    if (MAXFREE){ delete [] MAXFREE; MAXFREE = 0; }
    RAM = ram_size; MINB = min_block;
    int cnt = RAM / MINB;
    int L = 0, v = 1; while (v < cnt){ v <<= 1; ++L; }
    TOP = L;
    TREE_N = 1 << (L + 2);
    STATE = new unsigned char[TREE_N];
    MAXFREE = new int[TREE_N];
    for (int i=0;i<TREE_N;i++){ STATE[i]=NODE_FREE; MAXFREE[i]=-1; }
    STATE[1] = NODE_FREE; MAXFREE[1] = TOP;
}

static inline void ensure_children(int node, int order){
    int lc=node<<1, rc=lc|1;
    STATE[lc]=NODE_FREE; STATE[rc]=NODE_FREE;
    MAXFREE[lc]=order-1; MAXFREE[rc]=order-1;
}

static int alloc_first_fit_rec(int node,int order,int need,int base){
    if (MAXFREE[node] < need) return -1;
    if (STATE[node] == NODE_FREE){
        if (order == need){ STATE[node]=NODE_FULL; MAXFREE[node]=-1; return base; }
        STATE[node] = NODE_SPLIT; ensure_children(node, order);
    }
    if (STATE[node] == NODE_SPLIT){
        int half = units(order-1);
        int lc=node<<1, rc=lc|1;
        int res = alloc_first_fit_rec(lc, order-1, need, base);
        if (res == -1) res = alloc_first_fit_rec(rc, order-1, need, base+half);
        int lm=MAXFREE[lc], rm=MAXFREE[rc];
        if (lm==-1 && rm==-1){ STATE[node]=NODE_FULL; MAXFREE[node]=-1; }
        else { STATE[node]=NODE_SPLIT; MAXFREE[node]=max_int(lm,rm); }
        return res;
    }
    return -1;
}

static int alloc_at_rec(int node,int order,int need,int base,int target){
    if (MAXFREE[node] < need) return -1;
    if (STATE[node] == NODE_FREE){
        if (order == need){ STATE[node]=NODE_FULL; MAXFREE[node]=-1; return base; }
        STATE[node] = NODE_SPLIT; ensure_children(node, order);
    }
    if (STATE[node] == NODE_SPLIT){
        int half=units(order-1); int lc=node<<1, rc=lc|1;
        int mid = base + half;
        int res = (target < mid)
            ? alloc_at_rec(lc, order-1, need, base, target)
            : alloc_at_rec(rc, order-1, need, base+half, target);
        int lm=MAXFREE[lc], rm=MAXFREE[rc];
        if (lm==-1 && rm==-1){ STATE[node]=NODE_FULL; MAXFREE[node]=-1; }
        else if (lm==order-1 && rm==order-1 && STATE[lc]==NODE_FREE && STATE[rc]==NODE_FREE){ STATE[node]=NODE_FREE; MAXFREE[node]=order; }
        else { STATE[node]=NODE_SPLIT; MAXFREE[node]=max_int(lm,rm); }
        return res;
    }
    return -1;
}

static bool free_rec(int node,int order,int need,int base,int target){
    if (order == need){ STATE[node]=NODE_FREE; MAXFREE[node]=order; return true; }
    if (STATE[node] == NODE_FREE){ STATE[node]=NODE_SPLIT; ensure_children(node, order); }
    int half=units(order-1); int lc=node<<1, rc=lc|1; bool ok;
    if (target < base+half) ok = free_rec(lc, order-1, need, base, target);
    else ok = free_rec(rc, order-1, need, base+half, target);
    int lm=MAXFREE[lc], rm=MAXFREE[rc];
    if (lm==order-1 && rm==order-1 && STATE[lc]==NODE_FREE && STATE[rc]==NODE_FREE){ STATE[node]=NODE_FREE; MAXFREE[node]=order; }
    else if (lm==-1 && rm==-1 && STATE[lc]==NODE_FULL && STATE[rc]==NODE_FULL){ STATE[node]=NODE_FULL; MAXFREE[node]=-1; }
    else { STATE[node]=NODE_SPLIT; MAXFREE[node]=max_int(lm,rm); }
    return ok;
}

static int malloc_first_fit(int size){ int need=order_of_size(size); int idx=alloc_first_fit_rec(1,TOP,need,0); return idx<0?-1:idx*MINB; }
static int malloc_at(int addr, int size){ if(addr<0||size<=0) return -1; if(addr+size>RAM) return -1; if(addr%size) return -1; if(size%MINB) return -1; int need=order_of_size(size); int idx=alloc_at_rec(1,TOP,need,0,addr/MINB); return idx<0?-1:idx*MINB; }
static void free_at(int addr,int size){ if(addr<0||size<=0) return; int need=order_of_size(size); (void)free_rec(1,TOP,need,0,addr/MINB); }

} // namespace buddyimpl

inline void init(int ram_size, int min_block){ buddyimpl::init_impl(ram_size,min_block); }
inline int malloc(int size){ return buddyimpl::malloc_first_fit(size); }
inline int malloc_at(int addr, int size){ return buddyimpl::malloc_at(addr,size); }
inline void free_at(int addr, int size){ buddyimpl::free_at(addr,size); }

#endif
