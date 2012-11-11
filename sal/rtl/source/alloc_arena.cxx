/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#define _BSD_SOURCE /* sys/mman.h: MAP_ANON */
#include "alloc_arena.hxx"

#include "alloc_impl.hxx"
#include "internal/rtllifecycle.h"
#include "sal/macros.h"
#include "osl/diagnose.h"

#include <cassert>
#include <string.h>
#include <stdio.h>

extern AllocMode alloc_mode;

/* ================================================================= *
 *
 * arena internals.
 *
 * ================================================================= */

/** g_arena_list
 *  @internal
 */
struct rtl_arena_list_st
{
    rtl_memory_lock_type m_lock;
    rtl_arena_type       m_arena_head;
};

static rtl_arena_list_st g_arena_list;


/** gp_arena_arena
 *  provided for arena_type allocations, and hash_table resizing.
 *
 *  @internal
 */
static rtl_arena_type * gp_arena_arena = 0;


/** gp_machdep_arena
 *
 *  Low level virtual memory (pseudo) arena
 *  (platform dependent implementation)
 *
 *  @internal
 */
static rtl_arena_type * gp_machdep_arena = 0;


static void *
SAL_CALL rtl_machdep_alloc (
    rtl_arena_type * pArena,
    sal_Size *       pSize
);

static void
SAL_CALL rtl_machdep_free (
    rtl_arena_type * pArena,
    void *           pAddr,
    sal_Size         nSize
);

static sal_Size
rtl_machdep_pagesize();


/** gp_default_arena
 */
rtl_arena_type * gp_default_arena = 0;


/* ================================================================= */

/** rtl_arena_segment_constructor()
 */
static int
rtl_arena_segment_constructor (void * obj)
{
    rtl_arena_segment_type * segment = (rtl_arena_segment_type*)(obj);

    QUEUE_START_NAMED(segment, s);
    QUEUE_START_NAMED(segment, f);

    return (1);
}


/** rtl_arena_segment_destructor()
 */
static void
rtl_arena_segment_destructor (void * obj)
{
    rtl_arena_segment_type * segment = static_cast< rtl_arena_segment_type * >(
        obj);
    assert(QUEUE_STARTED_NAMED(segment, s));
    assert(QUEUE_STARTED_NAMED(segment, f));
    (void) segment; // avoid warnings
}

/* ================================================================= */

/** rtl_arena_segment_populate()
 *
 *  @precond  arena->m_lock acquired.
 */
static int
rtl_arena_segment_populate (
    rtl_arena_type * arena
)
{
    rtl_arena_segment_type *span;
    sal_Size                size = rtl_machdep_pagesize();

    span = static_cast< rtl_arena_segment_type * >(
        rtl_machdep_alloc(gp_machdep_arena, &size));
    if (span != 0)
    {
        rtl_arena_segment_type *first, *last, *head;
        sal_Size                count = size / sizeof(rtl_arena_segment_type);

        /* insert onto reserve span list */
        QUEUE_INSERT_TAIL_NAMED(&(arena->m_segment_reserve_span_head), span, s);
        QUEUE_START_NAMED(span, f);
        span->m_addr = (sal_uIntPtr)(span);
        span->m_size = size;
        span->m_type = RTL_ARENA_SEGMENT_TYPE_SPAN;

        /* insert onto reserve list */
        head  = &(arena->m_segment_reserve_head);
        for (first = span + 1, last = span + count; first < last; ++first)
        {
            QUEUE_INSERT_TAIL_NAMED(head, first, s);
            QUEUE_START_NAMED(first, f);
            first->m_addr = 0;
            first->m_size = 0;
            first->m_type = 0;
        }
    }
    return (span != 0);
}


/** rtl_arena_segment_get()
 *
 *  @precond  arena->m_lock acquired.
 *  @precond  (*ppSegment == 0)
 */
static RTL_MEMORY_INLINE void
rtl_arena_segment_get (
    rtl_arena_type *          arena,
    rtl_arena_segment_type ** ppSegment
)
{
    rtl_arena_segment_type * head;

    assert(*ppSegment == 0);

    head = &(arena->m_segment_reserve_head);
    if ((head->m_snext != head) || rtl_arena_segment_populate (arena))
    {
        (*ppSegment) = head->m_snext;
        QUEUE_REMOVE_NAMED((*ppSegment), s);
    }
}

#if defined(__SUNPRO_C) || defined(__SUNPRO_CC)
#pragma inline(rtl_arena_segment_get)
#endif


/** rtl_arena_segment_put()
 *
 *  @precond  arena->m_lock acquired.
 *  @postcond (*ppSegment == 0)
 */
static RTL_MEMORY_INLINE void
rtl_arena_segment_put (
    rtl_arena_type *          arena,
    rtl_arena_segment_type ** ppSegment
)
{
    rtl_arena_segment_type * head;

    assert(QUEUE_STARTED_NAMED((*ppSegment), s));
    assert(QUEUE_STARTED_NAMED((*ppSegment), f));

    (*ppSegment)->m_addr = 0;
    (*ppSegment)->m_size = 0;

    assert((*ppSegment)->m_type != RTL_ARENA_SEGMENT_TYPE_HEAD);
    (*ppSegment)->m_type = 0;

    /* keep as reserve */
    head = &(arena->m_segment_reserve_head);
    QUEUE_INSERT_HEAD_NAMED(head, (*ppSegment), s);

    /* clear */
    (*ppSegment) = 0;
}

#if defined(__SUNPRO_C) || defined(__SUNPRO_CC)
#pragma inline(rtl_arena_segment_put)
#endif

/* ================================================================= */

/** rtl_arena_freelist_insert()
 *
 *  @precond arena->m_lock acquired.
 */
static RTL_MEMORY_INLINE void
rtl_arena_freelist_insert (
    rtl_arena_type *         arena,
    rtl_arena_segment_type * segment
)
{
    rtl_arena_segment_type * head;

    head = &(arena->m_freelist_head[highbit(segment->m_size) - 1]);
    QUEUE_INSERT_TAIL_NAMED(head, segment, f);

    arena->m_freelist_bitmap |= head->m_size;
}

#if defined(__SUNPRO_C) || defined(__SUNPRO_CC)
#pragma inline(rtl_arena_freelist_insert)
#endif /* __SUNPRO_C */


/** rtl_arena_freelist_remove()
 *
 *  @precond arena->m_lock acquired.
 */
static RTL_MEMORY_INLINE void
rtl_arena_freelist_remove (
    rtl_arena_type *         arena,
    rtl_arena_segment_type * segment
)
{
    if ((segment->m_fnext->m_type == RTL_ARENA_SEGMENT_TYPE_HEAD) &&
        (segment->m_fprev->m_type == RTL_ARENA_SEGMENT_TYPE_HEAD)    )
    {
        rtl_arena_segment_type * head;

        head = segment->m_fprev;
        assert(arena->m_freelist_bitmap & head->m_size);
        arena->m_freelist_bitmap ^= head->m_size;
    }
    QUEUE_REMOVE_NAMED(segment, f);
}

#if defined(__SUNPRO_C) || defined(__SUNPRO_CC)
#pragma inline(rtl_arena_freelist_remove)
#endif /* __SUNPRO_C */


/* ================================================================= */

/** RTL_ARENA_HASH_INDEX()
 */
#define RTL_ARENA_HASH_INDEX_IMPL(a, s, q, m) \
     ((((a) + ((a) >> (s)) + ((a) >> ((s) << 1))) >> (q)) & (m))

#define RTL_ARENA_HASH_INDEX(arena, addr) \
    RTL_ARENA_HASH_INDEX_IMPL((addr), (arena)->m_hash_shift, (arena)->m_quantum_shift, ((arena)->m_hash_size - 1))

/** rtl_arena_hash_rescale()
 *
 * @precond arena->m_lock released.
 */
static void
rtl_arena_hash_rescale (
    rtl_arena_type * arena,
    sal_Size         new_size
)
{
    rtl_arena_segment_type ** new_table;
    sal_Size                  new_bytes;

    new_bytes = new_size * sizeof(rtl_arena_segment_type*);
    new_table = (rtl_arena_segment_type **)rtl_arena_alloc (gp_arena_arena, &new_bytes);

    if (new_table != 0)
    {
        rtl_arena_segment_type ** old_table;
        sal_Size                  old_size, i;

        memset (new_table, 0, new_bytes);

        RTL_MEMORY_LOCK_ACQUIRE(&(arena->m_lock));

        old_table = arena->m_hash_table;
        old_size  = arena->m_hash_size;

        // SAL_INFO(
        //  "sal.rtl",
        //  "rtl_arena_hash_rescale(" << arena->m_name << "): nseg: "
        //      << (arena->m_stats.m_alloc - arena->m_stats.m_free) << " (ave: "
        //      << ((arena->m_stats.m_alloc - arena->m_stats.m_free)
        //          >> arena->m_hash_shift)
        //      << "), frees: " << arena->m_stats.m_free << " [old_size: "
        //      << old_size << ", new_size: " << new_size << ']');

        arena->m_hash_table = new_table;
        arena->m_hash_size  = new_size;
        arena->m_hash_shift = highbit(arena->m_hash_size) - 1;

        for (i = 0; i < old_size; i++)
        {
            rtl_arena_segment_type * curr = old_table[i];
            while (curr != 0)
            {
                rtl_arena_segment_type  * next = curr->m_fnext;
                rtl_arena_segment_type ** head;

                head = &(arena->m_hash_table[RTL_ARENA_HASH_INDEX(arena, curr->m_addr)]);
                curr->m_fnext = (*head);
                (*head) = curr;

                curr = next;
            }
            old_table[i] = 0;
        }

        RTL_MEMORY_LOCK_RELEASE(&(arena->m_lock));

        if (old_table != arena->m_hash_table_0)
        {
            sal_Size old_bytes = old_size * sizeof(rtl_arena_segment_type*);
            rtl_arena_free (gp_arena_arena, old_table, old_bytes);
        }
    }
}


/** rtl_arena_hash_insert()
 *  ...and update stats.
 */
static RTL_MEMORY_INLINE void
rtl_arena_hash_insert (
    rtl_arena_type *         arena,
    rtl_arena_segment_type * segment
)
{
    rtl_arena_segment_type ** ppSegment;

    ppSegment = &(arena->m_hash_table[RTL_ARENA_HASH_INDEX(arena, segment->m_addr)]);

    segment->m_fnext = (*ppSegment);
    (*ppSegment) = segment;

    arena->m_stats.m_alloc     += 1;
    arena->m_stats.m_mem_alloc += segment->m_size;
}

#if defined(__SUNPRO_C) || defined(__SUNPRO_CC)
#pragma inline(rtl_arena_hash_insert)
#endif /* __SUNPRO_C */


/** rtl_arena_hash_remove()
 *  ...and update stats.
 */
static rtl_arena_segment_type *
rtl_arena_hash_remove (
    rtl_arena_type * arena,
    sal_uIntPtr      addr,
    sal_Size         size
)
{
    rtl_arena_segment_type *segment, **segpp;
    sal_Size lookups = 0;

    segpp = &(arena->m_hash_table[RTL_ARENA_HASH_INDEX(arena, addr)]);
    while ((segment = *segpp) != 0)
    {
        if (segment->m_addr == addr)
        {
            *segpp = segment->m_fnext, segment->m_fnext = segment->m_fprev = segment;
            break;
        }

        /* update lookup miss stats */
        lookups += 1;
        segpp = &(segment->m_fnext);
    }

    assert(segment != 0); // bad free
    if (segment != 0)
    {
        assert(segment->m_size == size);
        (void) size; // avoid warnings

        arena->m_stats.m_free      += 1;
        arena->m_stats.m_mem_alloc -= segment->m_size;

        if (lookups > 1)
        {
            sal_Size nseg = (sal_Size)(arena->m_stats.m_alloc - arena->m_stats.m_free);
            if (nseg > 4 * arena->m_hash_size)
            {
                if (!(arena->m_flags & RTL_ARENA_FLAG_RESCALE))
                {
                    sal_Size ave = nseg >> arena->m_hash_shift;
                    sal_Size new_size = arena->m_hash_size << (highbit(ave) - 1);

                    arena->m_flags |= RTL_ARENA_FLAG_RESCALE;
                    RTL_MEMORY_LOCK_RELEASE(&(arena->m_lock));
                    rtl_arena_hash_rescale (arena, new_size);
                    RTL_MEMORY_LOCK_ACQUIRE(&(arena->m_lock));
                    arena->m_flags &= ~RTL_ARENA_FLAG_RESCALE;
                }
            }
        }
    }

    return (segment);
}

/* ================================================================= */

/** rtl_arena_segment_alloc()
 *  allocate (and remove) segment from freelist
 *
 *  @precond arena->m_lock acquired
 *  @precond (*ppSegment == 0)
 */
static int
rtl_arena_segment_alloc (
    rtl_arena_type *          arena,
    sal_Size                  size,
    rtl_arena_segment_type ** ppSegment
)
{
    int index = 0;

    assert(*ppSegment == 0);
    if (!RTL_MEMORY_ISP2(size))
    {
        int msb = highbit(size);
        if (RTL_ARENA_FREELIST_SIZE == sal::static_int_cast< size_t >(msb))
        {
            /* highest possible freelist: fall back to first fit */
            rtl_arena_segment_type *head, *segment;

            head = &(arena->m_freelist_head[msb - 1]);
            for (segment = head->m_fnext; segment != head; segment = segment->m_fnext)
            {
                if (segment->m_size >= size)
                {
                    /* allocate first fit segment */
                    (*ppSegment) = segment;
                    break;
                }
            }
            goto dequeue_and_leave;
        }

        /* roundup to next power of 2 */
        size = (1UL << msb);
    }

    index = lowbit(RTL_MEMORY_P2ALIGN(arena->m_freelist_bitmap, size));
    if (index > 0)
    {
        /* instant fit: allocate first free segment */
        rtl_arena_segment_type *head;

        head = &(arena->m_freelist_head[index - 1]);
        (*ppSegment) = head->m_fnext;
        assert((*ppSegment) != head);
    }

dequeue_and_leave:
    if (*ppSegment != 0)
    {
        /* remove from freelist */
        rtl_arena_freelist_remove (arena, (*ppSegment));
    }
    return (*ppSegment != 0);
}


/** rtl_arena_segment_create()
 *  import new (span) segment from source arena
 *
 *  @precond arena->m_lock acquired
 *  @precond (*ppSegment == 0)
 */
static int
rtl_arena_segment_create (
    rtl_arena_type *          arena,
    sal_Size                  size,
    rtl_arena_segment_type ** ppSegment
)
{
    assert((*ppSegment) == 0);
    if (arena->m_source_alloc != 0)
    {
        rtl_arena_segment_get (arena, ppSegment);
        if (*ppSegment != 0)
        {
            rtl_arena_segment_type * span = 0;
            rtl_arena_segment_get (arena, &span);
            if (span != 0)
            {
                /* import new span from source arena */
                RTL_MEMORY_LOCK_RELEASE(&(arena->m_lock));

                span->m_size = size;
                span->m_addr = (sal_uIntPtr)(arena->m_source_alloc)(
                    arena->m_source_arena, &(span->m_size));

                RTL_MEMORY_LOCK_ACQUIRE(&(arena->m_lock));
                if (span->m_addr != 0)
                {
                    /* insert onto segment list, update stats */
                    span->m_type = RTL_ARENA_SEGMENT_TYPE_SPAN;
                    QUEUE_INSERT_HEAD_NAMED(&(arena->m_segment_head), span, s);
                    arena->m_stats.m_mem_total += span->m_size;

                    (*ppSegment)->m_addr = span->m_addr;
                    (*ppSegment)->m_size = span->m_size;
                    (*ppSegment)->m_type = RTL_ARENA_SEGMENT_TYPE_FREE;
                    QUEUE_INSERT_HEAD_NAMED(span, (*ppSegment), s);

                    /* report success */
                    return (1);
                }
                rtl_arena_segment_put (arena, &span);
            }
            rtl_arena_segment_put (arena, ppSegment);
        }
    }
    return (0);
}


/** rtl_arena_segment_coalesce()
 *  mark as free and join with adjacent free segment(s)
 *
 *  @precond arena->m_lock acquired
 *  @precond segment marked 'used'
 */
static void
rtl_arena_segment_coalesce (
    rtl_arena_type *         arena,
    rtl_arena_segment_type * segment
)
{
    rtl_arena_segment_type *next, *prev;

    /* mark segment free */
    assert(segment->m_type == RTL_ARENA_SEGMENT_TYPE_USED);
    segment->m_type = RTL_ARENA_SEGMENT_TYPE_FREE;

    /* try to merge w/ next segment */
    next = segment->m_snext;
    if (next->m_type == RTL_ARENA_SEGMENT_TYPE_FREE)
    {
        assert(segment->m_addr + segment->m_size == next->m_addr);
        segment->m_size += next->m_size;

        /* remove from freelist */
        rtl_arena_freelist_remove (arena, next);

        /* remove from segment list */
        QUEUE_REMOVE_NAMED(next, s);

        /* release segment descriptor */
        rtl_arena_segment_put (arena, &next);
    }

    /* try to merge w/ prev segment */
    prev = segment->m_sprev;
    if (prev->m_type == RTL_ARENA_SEGMENT_TYPE_FREE)
    {
        assert(prev->m_addr + prev->m_size == segment->m_addr);
        segment->m_addr  = prev->m_addr;
        segment->m_size += prev->m_size;

        /* remove from freelist */
        rtl_arena_freelist_remove (arena, prev);

        /* remove from segment list */
        QUEUE_REMOVE_NAMED(prev, s);

        /* release segment descriptor */
        rtl_arena_segment_put (arena, &prev);
    }
}

/* ================================================================= */

/** rtl_arena_constructor()
 */
static void
rtl_arena_constructor (void * obj)
{
    rtl_arena_type * arena = (rtl_arena_type*)(obj);
    rtl_arena_segment_type * head;
    size_t i;

    memset (arena, 0, sizeof(rtl_arena_type));

    QUEUE_START_NAMED(arena, arena_);

    (void) RTL_MEMORY_LOCK_INIT(&(arena->m_lock));

    head = &(arena->m_segment_reserve_span_head);
    rtl_arena_segment_constructor (head);
    head->m_type = RTL_ARENA_SEGMENT_TYPE_HEAD;

    head = &(arena->m_segment_reserve_head);
    rtl_arena_segment_constructor (head);
    head->m_type = RTL_ARENA_SEGMENT_TYPE_HEAD;

    head = &(arena->m_segment_head);
    rtl_arena_segment_constructor (head);
    head->m_type = RTL_ARENA_SEGMENT_TYPE_HEAD;

    for (i = 0; i < RTL_ARENA_FREELIST_SIZE; i++)
    {
        head = &(arena->m_freelist_head[i]);
        rtl_arena_segment_constructor (head);

        head->m_size = (1UL << i);
        head->m_type = RTL_ARENA_SEGMENT_TYPE_HEAD;
    }

    arena->m_hash_table = arena->m_hash_table_0;
    arena->m_hash_size  = RTL_ARENA_HASH_SIZE;
    arena->m_hash_shift = highbit(arena->m_hash_size) - 1;
}


/** rtl_arena_destructor()
 */
static void
rtl_arena_destructor (void * obj)
{
    rtl_arena_type * arena = (rtl_arena_type*)(obj);
    rtl_arena_segment_type * head;
    size_t i;

    assert(QUEUE_STARTED_NAMED(arena, arena_));

    RTL_MEMORY_LOCK_DESTROY(&(arena->m_lock));

    head = &(arena->m_segment_reserve_span_head);
    assert(head->m_type == RTL_ARENA_SEGMENT_TYPE_HEAD);
    rtl_arena_segment_destructor (head);

    head = &(arena->m_segment_reserve_head);
    assert(head->m_type == RTL_ARENA_SEGMENT_TYPE_HEAD);
    rtl_arena_segment_destructor (head);

    head = &(arena->m_segment_head);
    assert(head->m_type == RTL_ARENA_SEGMENT_TYPE_HEAD);
    rtl_arena_segment_destructor (head);

    for (i = 0; i < RTL_ARENA_FREELIST_SIZE; i++)
    {
        head = &(arena->m_freelist_head[i]);

        assert(head->m_size == (1UL << i));
        assert(head->m_type == RTL_ARENA_SEGMENT_TYPE_HEAD);

        rtl_arena_segment_destructor (head);
    }

    assert(arena->m_hash_table == arena->m_hash_table_0);
    assert(arena->m_hash_size  == RTL_ARENA_HASH_SIZE);
    assert(
        arena->m_hash_shift ==
        sal::static_int_cast< unsigned >(highbit(arena->m_hash_size) - 1));
}

/* ================================================================= */

/** rtl_arena_activate()
 */
static rtl_arena_type *
rtl_arena_activate (
    rtl_arena_type *   arena,
    const char *       name,
    sal_Size           quantum,
    sal_Size           quantum_cache_max,
    rtl_arena_type *   source_arena,
    void * (SAL_CALL * source_alloc)(rtl_arena_type *, sal_Size *),
    void   (SAL_CALL * source_free) (rtl_arena_type *, void *, sal_Size)
)
{
    assert(arena != 0);
    if (arena != 0)
    {
        (void) snprintf (arena->m_name, sizeof(arena->m_name), "%s", name);

        if (!RTL_MEMORY_ISP2(quantum))
        {
            /* roundup to next power of 2 */
            quantum = (1UL << highbit(quantum));
        }
        quantum_cache_max = RTL_MEMORY_P2ROUNDUP(quantum_cache_max, quantum);

        arena->m_quantum = quantum;
        arena->m_quantum_shift = highbit(arena->m_quantum) - 1;
        arena->m_qcache_max = quantum_cache_max;

        arena->m_source_arena = source_arena;
        arena->m_source_alloc = source_alloc;
        arena->m_source_free  = source_free;

        if (arena->m_qcache_max > 0)
        {
            char namebuf[RTL_ARENA_NAME_LENGTH + 1];
            int  i, n = (arena->m_qcache_max >> arena->m_quantum_shift);

            sal_Size size = n * sizeof(rtl_cache_type*);
            arena->m_qcache_ptr = (rtl_cache_type**)rtl_arena_alloc (gp_arena_arena, &size);
            if (!(arena->m_qcache_ptr))
            {
                /* out of memory */
                return (0);
            }
            for (i = 1; i <= n; i++)
            {
                size = i * arena->m_quantum;
                (void) snprintf (namebuf, sizeof(namebuf), "%s_%lu", arena->m_name, size);
                arena->m_qcache_ptr[i - 1] = rtl_cache_create(namebuf, size, 0, NULL, NULL, NULL, NULL, arena, RTL_CACHE_FLAG_QUANTUMCACHE);
            }
        }

        /* insert into arena list */
        RTL_MEMORY_LOCK_ACQUIRE(&(g_arena_list.m_lock));
        QUEUE_INSERT_TAIL_NAMED(&(g_arena_list.m_arena_head), arena, arena_);
        RTL_MEMORY_LOCK_RELEASE(&(g_arena_list.m_lock));
    }
    return (arena);
}

/** rtl_arena_deactivate()
 */
static void
rtl_arena_deactivate (
    rtl_arena_type * arena
)
{
    rtl_arena_segment_type * head, * segment;

    /* remove from arena list */
    RTL_MEMORY_LOCK_ACQUIRE(&(g_arena_list.m_lock));
    QUEUE_REMOVE_NAMED(arena, arena_);
    RTL_MEMORY_LOCK_RELEASE(&(g_arena_list.m_lock));

    /* cleanup quantum cache(s) */
    if ((arena->m_qcache_max > 0) && (arena->m_qcache_ptr != 0))
    {
        int  i, n = (arena->m_qcache_max >> arena->m_quantum_shift);
        for (i = 1; i <= n; i++)
        {
            if (arena->m_qcache_ptr[i - 1] != 0)
            {
                rtl_cache_destroy (arena->m_qcache_ptr[i - 1]);
                arena->m_qcache_ptr[i - 1] = 0;
            }
        }
        rtl_arena_free (
            gp_arena_arena,
            arena->m_qcache_ptr,
            n * sizeof(rtl_cache_type*));

        arena->m_qcache_ptr = 0;
    }

    /* check for leaked segments */
    // SAL_INFO(
    //  "sal.rtl",
    //  "rtl_arena_deactivate(" << arena->m_name << "): allocs: "
    //      << arena->m_stats.m_alloc << ", frees: " << arena->m_stats.m_free
    //      << "; total: " << arena->m_stats.m_mem_total << ", used: "
    //      << arena->m_stats.m_mem_alloc);
    if (arena->m_stats.m_alloc > arena->m_stats.m_free)
    {
        sal_Size i, n;

        // SAL_INFO(
        //  "sal.rtl",
        //  "rtl_arena_deactivate(" << arena->m_name << "): cleaning up "
        //      << (arena->m_stats.m_alloc - arena->m_stats.m_free)
        //      << " leaked segment(s) [" << arena->m_stats.m_mem_alloc
        //      << " bytes]");

        /* cleanup still used segment(s) */
        for (i = 0, n = arena->m_hash_size; i < n; i++)
        {
            while ((segment = arena->m_hash_table[i]) != 0)
            {
                /* pop from hash table */
                arena->m_hash_table[i] = segment->m_fnext, segment->m_fnext = segment->m_fprev = segment;

                /* coalesce w/ adjacent free segment(s) */
                rtl_arena_segment_coalesce (arena, segment);

                /* insert onto freelist */
                rtl_arena_freelist_insert (arena, segment);
            }
        }
    }

    /* cleanup hash table */
    if (arena->m_hash_table != arena->m_hash_table_0)
    {
        rtl_arena_free (
            gp_arena_arena,
            arena->m_hash_table,
            arena->m_hash_size * sizeof(rtl_arena_segment_type*));

        arena->m_hash_table = arena->m_hash_table_0;
        arena->m_hash_size  = RTL_ARENA_HASH_SIZE;
        arena->m_hash_shift = highbit(arena->m_hash_size) - 1;
    }

    /* cleanup segment list */
    head = &(arena->m_segment_head);
    for (segment = head->m_snext; segment != head; segment = head->m_snext)
    {
        if (segment->m_type == RTL_ARENA_SEGMENT_TYPE_FREE)
        {
            /* remove from freelist */
            rtl_arena_freelist_remove (arena, segment);
        }
        else
        {
            /* can have only free and span segments here */
            assert(segment->m_type == RTL_ARENA_SEGMENT_TYPE_SPAN);
        }

        /* remove from segment list */
        QUEUE_REMOVE_NAMED(segment, s);

        /* release segment descriptor */
        rtl_arena_segment_put (arena, &segment);
    }

    /* cleanup segment reserve list */
    head = &(arena->m_segment_reserve_head);
    for (segment = head->m_snext; segment != head; segment = head->m_snext)
    {
        /* remove from segment list */
        QUEUE_REMOVE_NAMED(segment, s);
    }

    /* cleanup segment reserve span(s) */
    head = &(arena->m_segment_reserve_span_head);
    for (segment = head->m_snext; segment != head; segment = head->m_snext)
    {
        /* can have only span segments here */
        assert(segment->m_type == RTL_ARENA_SEGMENT_TYPE_SPAN);

        /* remove from segment list */
        QUEUE_REMOVE_NAMED(segment, s);

        /* return span to g_machdep_arena */
        rtl_machdep_free (gp_machdep_arena, (void*)(segment->m_addr), segment->m_size);
    }
}

/* ================================================================= *
 *
 * arena implementation.
 *
 * ================================================================= */

/** rtl_arena_create()
 */
rtl_arena_type *
SAL_CALL rtl_arena_create (
    const char *       name,
    sal_Size           quantum,
    sal_Size           quantum_cache_max,
    rtl_arena_type *   source_arena,
    void * (SAL_CALL * source_alloc)(rtl_arena_type *, sal_Size *),
    void   (SAL_CALL * source_free) (rtl_arena_type *, void *, sal_Size),
    SAL_UNUSED_PARAMETER int
) SAL_THROW_EXTERN_C()
{
    rtl_arena_type * result = 0;
    sal_Size         size   = sizeof(rtl_arena_type);

try_alloc:
    result = (rtl_arena_type*)rtl_arena_alloc (gp_arena_arena, &size);
    if (result != 0)
    {
        rtl_arena_type * arena = result;
        rtl_arena_constructor (arena);

        if (!source_arena)
        {
            assert(gp_default_arena != 0);
            source_arena = gp_default_arena;
        }

        result = rtl_arena_activate (
            arena,
            name,
            quantum,
            quantum_cache_max,
            source_arena,
            source_alloc,
            source_free
        );

        if (result == 0)
        {
            rtl_arena_deactivate (arena);
            rtl_arena_destructor (arena);
            rtl_arena_free (gp_arena_arena, arena, size);
        }
    }
    else if (gp_arena_arena == 0)
    {
        ensureArenaSingleton();
        if (gp_arena_arena)
        {
            /* try again */
            goto try_alloc;
        }
    }
    return (result);
}

/** rtl_arena_destroy()
 */
void
SAL_CALL rtl_arena_destroy (
    rtl_arena_type * arena
) SAL_THROW_EXTERN_C()
{
    if (arena != 0)
    {
        rtl_arena_deactivate (arena);
        rtl_arena_destructor (arena);
        rtl_arena_free (gp_arena_arena, arena, sizeof(rtl_arena_type));
    }
}

/** rtl_arena_alloc()
 */
void *
SAL_CALL rtl_arena_alloc (
    rtl_arena_type * arena,
    sal_Size *       pSize
) SAL_THROW_EXTERN_C()
{
    void * addr = 0;

    if ((arena != 0) && (pSize != 0))
    {
        sal_Size size;

        if (alloc_mode == AMode_SYSTEM)
            return rtl_allocateMemory(*pSize);

        size = RTL_MEMORY_ALIGN((*pSize), arena->m_quantum);
        if (size > arena->m_qcache_max)
        {
            /* allocate from segment list */
            rtl_arena_segment_type *segment = 0;

            RTL_MEMORY_LOCK_ACQUIRE(&(arena->m_lock));
            if (rtl_arena_segment_alloc (arena, size, &segment) ||
                rtl_arena_segment_create(arena, size, &segment)    )
            {
                /* shrink to fit */
                sal_Size oversize;

                /* mark segment used */
                assert(segment->m_type == RTL_ARENA_SEGMENT_TYPE_FREE);
                segment->m_type = RTL_ARENA_SEGMENT_TYPE_USED;

                /* resize */
                assert(segment->m_size >= size);
                oversize = segment->m_size - size;
                if ((oversize >= arena->m_quantum) && (oversize >= arena->m_qcache_max))
                {
                    rtl_arena_segment_type * remainder = 0;
                    rtl_arena_segment_get (arena, &remainder);
                    if (remainder != 0)
                    {
                        segment->m_size = size;

                        remainder->m_addr = segment->m_addr + segment->m_size;
                        remainder->m_size = oversize;
                        remainder->m_type = RTL_ARENA_SEGMENT_TYPE_FREE;
                        QUEUE_INSERT_HEAD_NAMED(segment, remainder, s);

                        rtl_arena_freelist_insert (arena, remainder);
                    }
                }

                rtl_arena_hash_insert (arena, segment);

                (*pSize) = segment->m_size;
                addr = (void*)(segment->m_addr);
            }
            RTL_MEMORY_LOCK_RELEASE(&(arena->m_lock));
        }
        else if (size > 0)
        {
            /* allocate from quantum cache(s) */
            int index = (size >> arena->m_quantum_shift) - 1;
            assert(arena->m_qcache_ptr[index] != 0);

            addr = rtl_cache_alloc (arena->m_qcache_ptr[index]);
            if (addr != 0)
                (*pSize) = size;
        }
    }
    return (addr);
}

/** rtl_arena_free()
 */
void
SAL_CALL rtl_arena_free (
    rtl_arena_type * arena,
    void *           addr,
    sal_Size         size
) SAL_THROW_EXTERN_C()
{
    if (arena != 0)
    {
        if (alloc_mode == AMode_SYSTEM)
        {
            rtl_freeMemory(addr);
            return;
        }

        size = RTL_MEMORY_ALIGN(size, arena->m_quantum);
        if (size > arena->m_qcache_max)
        {
            /* free to segment list */
            rtl_arena_segment_type * segment;

            RTL_MEMORY_LOCK_ACQUIRE(&(arena->m_lock));

            segment = rtl_arena_hash_remove (arena, (sal_uIntPtr)(addr), size);
            if (segment != 0)
            {
                rtl_arena_segment_type *next, *prev;

                /* coalesce w/ adjacent free segment(s) */
                rtl_arena_segment_coalesce (arena, segment);

                /* determine (new) next and prev segment */
                next = segment->m_snext, prev = segment->m_sprev;

                /* entire span free when prev is a span, and next is either a span or a list head */
                if (((prev->m_type == RTL_ARENA_SEGMENT_TYPE_SPAN)) &&
                    ((next->m_type == RTL_ARENA_SEGMENT_TYPE_SPAN)  ||
                     (next->m_type == RTL_ARENA_SEGMENT_TYPE_HEAD))    )
                {
                    assert(
                        prev->m_addr == segment->m_addr
                        && prev->m_size == segment->m_size);

                    if (arena->m_source_free)
                    {
                        addr = (void*)(prev->m_addr);
                        size = prev->m_size;

                        /* remove from segment list */
                        QUEUE_REMOVE_NAMED(segment, s);

                        /* release segment descriptor */
                        rtl_arena_segment_put (arena, &segment);

                        /* remove from segment list */
                        QUEUE_REMOVE_NAMED(prev, s);

                        /* release (span) segment descriptor */
                        rtl_arena_segment_put (arena, &prev);

                        /* update stats, return span to source arena */
                        arena->m_stats.m_mem_total -= size;
                        RTL_MEMORY_LOCK_RELEASE(&(arena->m_lock));

                        (arena->m_source_free)(arena->m_source_arena, addr, size);
                        return;
                    }
                }

                /* insert onto freelist */
                rtl_arena_freelist_insert (arena, segment);
            }

            RTL_MEMORY_LOCK_RELEASE(&(arena->m_lock));
        }
        else if (size > 0)
        {
            /* free to quantum cache(s) */
            int index = (size >> arena->m_quantum_shift) - 1;
            assert(arena->m_qcache_ptr[index] != 0);

            rtl_cache_free (arena->m_qcache_ptr[index], addr);
        }
    }
}

/* ================================================================= *
 *
 * machdep internals.
 *
 * ================================================================= */

#if defined(SAL_UNX)
#include <sys/mman.h>
#elif defined(SAL_W32)
#define MAP_FAILED 0
#endif /* SAL_UNX || SAL_W32 */

/** rtl_machdep_alloc()
 */
static void *
SAL_CALL rtl_machdep_alloc (
    rtl_arena_type * pArena,
    sal_Size *       pSize
)
{
    void *   addr;
    sal_Size size = (*pSize);

    assert(pArena == gp_machdep_arena);

#if defined(SOLARIS) && defined(SPARC)
    /* see @ mmap(2) man pages */
    size += (pArena->m_quantum + pArena->m_quantum); /* "red-zone" pages */
    if (size > (4 << 20))
        size = RTL_MEMORY_P2ROUNDUP(size, (4 << 20));
    else if (size > (512 << 10))
        size = RTL_MEMORY_P2ROUNDUP(size, (512 << 10));
    else
        size = RTL_MEMORY_P2ROUNDUP(size, (64 << 10));
    size -= (pArena->m_quantum + pArena->m_quantum); /* "red-zone" pages */
#else
    /* default allocation granularity */
    if(pArena->m_quantum < (64 << 10))
    {
        size = RTL_MEMORY_P2ROUNDUP(size, (64 << 10));
    }
    else
    {
        size = RTL_MEMORY_P2ROUNDUP(size, pArena->m_quantum);
    }
#endif

#if defined(SAL_UNX)
    addr = mmap (NULL, (size_t)(size), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
#elif defined(SAL_W32)
    addr = VirtualAlloc (NULL, (SIZE_T)(size), MEM_COMMIT, PAGE_READWRITE);
#endif /* (SAL_UNX || SAL_W32) */

    if (addr != MAP_FAILED)
    {
        pArena->m_stats.m_alloc += 1;
        pArena->m_stats.m_mem_total += size;
        pArena->m_stats.m_mem_alloc += size;

        (*pSize) = size;
        return (addr);
    }
    return (NULL);
}

/** rtl_machdep_free()
 */
static void
SAL_CALL rtl_machdep_free (
    rtl_arena_type * pArena,
    void *           pAddr,
    sal_Size         nSize
)
{
    assert(pArena == gp_machdep_arena);

    pArena->m_stats.m_free += 1;
    pArena->m_stats.m_mem_total -= nSize;
    pArena->m_stats.m_mem_alloc -= nSize;

#if defined(SAL_UNX)
    (void) munmap(pAddr, nSize);
#elif defined(SAL_W32)
    (void) VirtualFree ((LPVOID)(pAddr), (SIZE_T)(0), MEM_RELEASE);
#endif /* (SAL_UNX || SAL_W32) */
}

/** rtl_machdep_pagesize()
 */
static sal_Size
rtl_machdep_pagesize()
{
#if defined(SAL_UNX)
#if defined(FREEBSD) || defined(NETBSD) || defined(DRAGONFLY)
    return ((sal_Size)getpagesize());
#else  /* POSIX */
    return ((sal_Size)sysconf(_SC_PAGESIZE));
#endif /* xBSD || POSIX */
#elif defined(SAL_W32)
    SYSTEM_INFO info;
    GetSystemInfo (&info);
    return ((sal_Size)(info.dwPageSize));
#endif /* (SAL_UNX || SAL_W32) */
}

/* ================================================================= *
 *
 * arena initialization.
 *
 * ================================================================= */

void
rtl_arena_init()
{
    {
        /* list of arenas */
        RTL_MEMORY_LOCK_INIT(&(g_arena_list.m_lock));
        rtl_arena_constructor (&(g_arena_list.m_arena_head));
    }
    {
        /* machdep (pseudo) arena */
        static rtl_arena_type g_machdep_arena;

        assert(gp_machdep_arena == 0);
        rtl_arena_constructor (&g_machdep_arena);

        gp_machdep_arena = rtl_arena_activate (
            &g_machdep_arena,
            "rtl_machdep_arena",
            rtl_machdep_pagesize(),
            0,       /* no quantum caching */
            0, 0, 0  /* no source */
        );
        assert(gp_machdep_arena != 0);
    }
    {
        /* default arena */
        static rtl_arena_type g_default_arena;

        assert(gp_default_arena == 0);
        rtl_arena_constructor (&g_default_arena);

        gp_default_arena = rtl_arena_activate (
            &g_default_arena,
            "rtl_default_arena",
            rtl_machdep_pagesize(),
            0,                 /* no quantum caching */
            gp_machdep_arena,  /* source */
            rtl_machdep_alloc,
            rtl_machdep_free
        );
        assert(gp_default_arena != 0);
    }
    {
        /* arena internal arena */
        static rtl_arena_type g_arena_arena;

        assert(gp_arena_arena == 0);
        rtl_arena_constructor (&g_arena_arena);

        gp_arena_arena = rtl_arena_activate (
            &g_arena_arena,
            "rtl_arena_internal_arena",
            64,                /* quantum */
            0,                 /* no quantum caching */
            gp_default_arena,  /* source */
            rtl_arena_alloc,
            rtl_arena_free
        );
        assert(gp_arena_arena != 0);
    }
    // SAL_INFO("sal.rtl", "rtl_arena_init completed");
}

/* ================================================================= */

void
rtl_arena_fini()
{
    if (gp_arena_arena != 0)
    {
        rtl_arena_type * arena, * head;

        RTL_MEMORY_LOCK_ACQUIRE(&(g_arena_list.m_lock));
        head = &(g_arena_list.m_arena_head);

        for (arena = head->m_arena_next; arena != head; arena = arena->m_arena_next)
        {
            // SAL_INFO(
            //  "sal.rtl",
            //  "rtl_arena_fini(" << arena->m_name << "): allocs: "
            //      << arena->m_stats.m_alloc << ", frees: "
            //      << arena->m_stats.m_free << "; total: "
            //      << arena->m_stats.m_mem_total << ", used: "
            //      << arena->m_stats.m_mem_alloc);
        }
        RTL_MEMORY_LOCK_RELEASE(&(g_arena_list.m_lock));
    }
    // SAL_INFO("sal.rtl", "rtl_arena_fini completed");
}

/* ================================================================= */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
