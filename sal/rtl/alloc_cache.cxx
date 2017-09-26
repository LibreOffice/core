/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "alloc_cache.hxx"
#include "alloc_impl.hxx"
#include "alloc_arena.hxx"
#include "rtllifecycle.h"
#include "sal/macros.h"
#include <osl/thread.hxx>

#include <cassert>
#include <string.h>
#include <stdio.h>

#if defined(SAL_UNX)
#include <sys/time.h>
#endif

/**
 *  @internal
 */
struct rtl_cache_list_st
{
    rtl_memory_lock_type m_lock;
    rtl_cache_type       m_cache_head;

#if defined(SAL_UNX)
    pthread_t            m_update_thread;
    pthread_cond_t       m_update_cond;
#elif defined(SAL_W32)
    HANDLE               m_update_thread;
    HANDLE               m_update_cond;
#endif /* SAL_UNX || SAL_W32 */
    int                  m_update_done;
};

static rtl_cache_list_st g_cache_list;

/**
    provided for cache_type allocations, and hash_table resizing.

    @internal
*/
static rtl_arena_type * gp_cache_arena = nullptr;

/**
    @internal
*/
static rtl_cache_type * gp_cache_magazine_cache = nullptr;

/**
    @internal
*/
static rtl_cache_type * gp_cache_slab_cache = nullptr;

/**
    @internal
*/
static rtl_cache_type * gp_cache_bufctl_cache = nullptr;

#define RTL_CACHE_HASH_INDEX_IMPL(a, s, q, m) \
     ((((a) + ((a) >> (s)) + ((a) >> ((s) << 1))) >> (q)) & (m))

#define RTL_CACHE_HASH_INDEX(cache, addr) \
    RTL_CACHE_HASH_INDEX_IMPL((addr), (cache)->m_hash_shift, (cache)->m_type_shift, ((cache)->m_hash_size - 1))

namespace
{

void rtl_cache_hash_rescale(
    rtl_cache_type * cache,
    sal_Size         new_size
)
{
    rtl_cache_bufctl_type ** new_table;
    sal_Size                 new_bytes;

    new_bytes = new_size * sizeof(rtl_cache_bufctl_type*);
    new_table = static_cast<rtl_cache_bufctl_type**>(rtl_arena_alloc(gp_cache_arena, &new_bytes));

    if (new_table)
    {
        rtl_cache_bufctl_type ** old_table;
        sal_Size                 old_size, i;

        memset (new_table, 0, new_bytes);

        RTL_MEMORY_LOCK_ACQUIRE(&(cache->m_slab_lock));

        old_table = cache->m_hash_table;
        old_size  = cache->m_hash_size;

        cache->m_hash_table = new_table;
        cache->m_hash_size  = new_size;
        const auto bit = highbit(cache->m_hash_size);
        assert(bit > 0);
        cache->m_hash_shift = bit - 1;

        for (i = 0; i < old_size; i++)
        {
            rtl_cache_bufctl_type * curr = old_table[i];
            while (curr)
            {
                rtl_cache_bufctl_type  * next = curr->m_next;
                rtl_cache_bufctl_type ** head;

                head = &(cache->m_hash_table[RTL_CACHE_HASH_INDEX(cache, curr->m_addr)]);
                curr->m_next = (*head);
                (*head) = curr;

                curr = next;
            }
            old_table[i] = nullptr;
        }

        RTL_MEMORY_LOCK_RELEASE(&(cache->m_slab_lock));

        if (old_table != cache->m_hash_table_0)
        {
            sal_Size old_bytes = old_size * sizeof(rtl_cache_bufctl_type*);
            rtl_arena_free (gp_cache_arena, old_table, old_bytes);
        }
    }
}

inline sal_uIntPtr rtl_cache_hash_insert(
    rtl_cache_type *        cache,
    rtl_cache_bufctl_type * bufctl
)
{
    rtl_cache_bufctl_type ** ppHead;

    ppHead = &(cache->m_hash_table[RTL_CACHE_HASH_INDEX(cache, bufctl->m_addr)]);

    bufctl->m_next = (*ppHead);
    (*ppHead) = bufctl;

    return bufctl->m_addr;
}

rtl_cache_bufctl_type * rtl_cache_hash_remove(
    rtl_cache_type * cache,
    sal_uIntPtr      addr
)
{
    rtl_cache_bufctl_type ** ppHead;
    rtl_cache_bufctl_type  * bufctl;
    sal_Size                 lookups = 0;

    ppHead = &(cache->m_hash_table[RTL_CACHE_HASH_INDEX(cache, addr)]);
    while ((bufctl = *ppHead))
    {
        if (bufctl->m_addr == addr)
        {
            *ppHead = bufctl->m_next;
            bufctl->m_next = nullptr;
            break;
        }

        lookups += 1;
        ppHead = &(bufctl->m_next);
    }

    assert(bufctl); // bad free

    if (lookups > 1)
    {
        sal_Size nbuf = (sal_Size)(cache->m_slab_stats.m_alloc - cache->m_slab_stats.m_free);
        if (nbuf > 4 * cache->m_hash_size)
        {
            if (!(cache->m_features & RTL_CACHE_FEATURE_RESCALE))
            {
                sal_Size ave = nbuf >> cache->m_hash_shift;
                const auto bit = highbit(ave);
                assert(bit > 0);
                sal_Size new_size = cache->m_hash_size << (bit - 1);

                cache->m_features |= RTL_CACHE_FEATURE_RESCALE;
                RTL_MEMORY_LOCK_RELEASE(&(cache->m_slab_lock));
                rtl_cache_hash_rescale (cache, new_size);
                RTL_MEMORY_LOCK_ACQUIRE(&(cache->m_slab_lock));
                cache->m_features &= ~RTL_CACHE_FEATURE_RESCALE;
            }
        }
    }

    return bufctl;
}

#define RTL_CACHE_SLAB(addr, size) \
    ((reinterpret_cast<rtl_cache_slab_type*>(RTL_MEMORY_P2END(reinterpret_cast<sal_uIntPtr>(addr), (size)))) - 1)

int rtl_cache_slab_constructor(void * obj, SAL_UNUSED_PARAMETER void *)
{
    rtl_cache_slab_type * slab = static_cast<rtl_cache_slab_type*>(obj);

    QUEUE_START_NAMED(slab, slab_);
    slab->m_ntypes = 0;

    return 1;
}

void rtl_cache_slab_destructor(void * obj, SAL_UNUSED_PARAMETER void *)
{
    rtl_cache_slab_type * slab = static_cast< rtl_cache_slab_type * >(obj);
    assert(QUEUE_STARTED_NAMED(slab, slab_)); // assure removed from queue(s)
    assert(slab->m_ntypes == 0); // assure no longer referenced
    (void) slab; // avoid warnings
}

/**
    @precond cache->m_slab_lock released.
*/
rtl_cache_slab_type * rtl_cache_slab_create(rtl_cache_type * cache)
{
    rtl_cache_slab_type * slab = nullptr;
    void *                addr;
    sal_Size              size;

    size = cache->m_slab_size;
    addr = rtl_arena_alloc (cache->m_source, &size);
    if (SAL_LIKELY(addr))
    {
        assert(size >= cache->m_slab_size);

        if (cache->m_features & RTL_CACHE_FEATURE_HASH)
        {
            /* allocate slab struct from slab cache */
            assert(cache != gp_cache_slab_cache);
            slab = static_cast<rtl_cache_slab_type*>(rtl_cache_alloc (gp_cache_slab_cache));
        }
        else
        {
            /* construct embedded slab struct */
            slab = RTL_CACHE_SLAB(addr, cache->m_slab_size);
            (void) rtl_cache_slab_constructor (slab, nullptr);
        }
        if (SAL_LIKELY(slab))
        {
            slab->m_data = reinterpret_cast<sal_uIntPtr>(addr);

            /* dynamic freelist initialization */
            slab->m_bp = slab->m_data;
            slab->m_sp = nullptr;
        }
        else
        {
            rtl_arena_free (cache->m_source, addr, size);
        }
    }
    return slab;
}

/**
    @precond cache->m_slab_lock released.
*/
void rtl_cache_slab_destroy(
    rtl_cache_type *      cache,
    rtl_cache_slab_type * slab
)
{
    void *   addr   = reinterpret_cast<void*>(slab->m_data);
    sal_Size refcnt = slab->m_ntypes; slab->m_ntypes = 0;

    if (cache->m_features & RTL_CACHE_FEATURE_HASH)
    {
        /* cleanup bufctl(s) for free buffer(s) */
        sal_Size ntypes = (slab->m_bp - slab->m_data) / cache->m_type_size;
        for (ntypes -= refcnt; slab->m_sp; ntypes--)
        {
            rtl_cache_bufctl_type * bufctl = slab->m_sp;

            /* pop from freelist */
            slab->m_sp = bufctl->m_next;
            bufctl->m_next = nullptr;

            /* return bufctl struct to bufctl cache */
            rtl_cache_free (gp_cache_bufctl_cache, bufctl);
        }
        assert(ntypes == 0);

        /* return slab struct to slab cache */
        rtl_cache_free (gp_cache_slab_cache, slab);
    }
    else
    {
        /* destruct embedded slab struct */
        rtl_cache_slab_destructor (slab, nullptr);
    }

    if (refcnt == 0 || cache->m_features & RTL_CACHE_FEATURE_BULKDESTROY)
    {
        /* free memory */
        rtl_arena_free (cache->m_source, addr, cache->m_slab_size);
    }
}

/**
    @precond cache->m_slab_lock acquired.
*/
bool rtl_cache_slab_populate(rtl_cache_type * cache)
{
    rtl_cache_slab_type * slab;

    RTL_MEMORY_LOCK_RELEASE(&(cache->m_slab_lock));
    slab = rtl_cache_slab_create (cache);
    RTL_MEMORY_LOCK_ACQUIRE(&(cache->m_slab_lock));
    if (slab)
    {
        /* update buffer start addr w/ current color */
        slab->m_bp += cache->m_ncolor;

        /* update color for next slab */
        cache->m_ncolor += cache->m_type_align;
        if (cache->m_ncolor > cache->m_ncolor_max)
            cache->m_ncolor = 0;

        /* update stats */
        cache->m_slab_stats.m_mem_total += cache->m_slab_size;

        /* insert onto 'free' queue */
        QUEUE_INSERT_HEAD_NAMED(&(cache->m_free_head), slab, slab_);
    }
    return (slab != nullptr);
}

/**
    Allocate a buffer from slab layer; used by magazine layer.
*/
void * rtl_cache_slab_alloc (rtl_cache_type * cache)
{
    void                * addr = nullptr;
    rtl_cache_slab_type * head;

    RTL_MEMORY_LOCK_ACQUIRE(&(cache->m_slab_lock));

    head = &(cache->m_free_head);
    if (head->m_slab_next != head || rtl_cache_slab_populate (cache))
    {
        rtl_cache_slab_type   * slab;
        rtl_cache_bufctl_type * bufctl;

        slab = head->m_slab_next;
        assert(slab->m_ntypes < cache->m_ntypes);

        if (!slab->m_sp)
        {
            /* initialize bufctl w/ current 'slab->m_bp' */
            assert(slab->m_bp < slab->m_data + cache->m_ntypes * cache->m_type_size + cache->m_ncolor_max);
            if (cache->m_features & RTL_CACHE_FEATURE_HASH)
            {
                /* allocate bufctl */
                assert(cache != gp_cache_bufctl_cache);
                bufctl = static_cast<rtl_cache_bufctl_type*>(rtl_cache_alloc (gp_cache_bufctl_cache));
                if (!bufctl)
                {
                    /* out of memory */
                    RTL_MEMORY_LOCK_RELEASE(&(cache->m_slab_lock));
                    return nullptr;
                }

                bufctl->m_addr = slab->m_bp;
                bufctl->m_slab = reinterpret_cast<sal_uIntPtr>(slab);
            }
            else
            {
                /* embedded bufctl */
                bufctl = reinterpret_cast<rtl_cache_bufctl_type*>(slab->m_bp);
            }
            bufctl->m_next = nullptr;

            /* update 'slab->m_bp' to next free buffer */
            slab->m_bp += cache->m_type_size;

            /* assign bufctl to freelist */
            slab->m_sp = bufctl;
        }

        /* pop front */
        bufctl = slab->m_sp;
        slab->m_sp = bufctl->m_next;

        /* increment usage, check for full slab */
        if ((slab->m_ntypes += 1) == cache->m_ntypes)
        {
            /* remove from 'free' queue */
            QUEUE_REMOVE_NAMED(slab, slab_);

            /* insert onto 'used' queue (tail) */
            QUEUE_INSERT_TAIL_NAMED(&(cache->m_used_head), slab, slab_);
        }

        /* update stats */
        cache->m_slab_stats.m_alloc     += 1;
        cache->m_slab_stats.m_mem_alloc += cache->m_type_size;

        if (cache->m_features & RTL_CACHE_FEATURE_HASH)
            addr = reinterpret_cast<void*>(rtl_cache_hash_insert (cache, bufctl));
        else
            addr = bufctl;
    }

    RTL_MEMORY_LOCK_RELEASE(&(cache->m_slab_lock));
    return addr;
}

/**
    Return a buffer to slab layer; used by magazine layer.
*/
void rtl_cache_slab_free(
    rtl_cache_type * cache,
    void *           addr
)
{
    rtl_cache_bufctl_type * bufctl;
    rtl_cache_slab_type   * slab;

    RTL_MEMORY_LOCK_ACQUIRE(&(cache->m_slab_lock));

    /* determine slab from addr */
    if (cache->m_features & RTL_CACHE_FEATURE_HASH)
    {
        bufctl = rtl_cache_hash_remove (cache, reinterpret_cast<sal_uIntPtr>(addr));
        slab = (bufctl != nullptr) ? reinterpret_cast<rtl_cache_slab_type*>(bufctl->m_slab) : nullptr;
    }
    else
    {
        /* embedded slab struct */
        bufctl = static_cast<rtl_cache_bufctl_type*>(addr);
        slab = RTL_CACHE_SLAB(addr, cache->m_slab_size);
    }

    if (slab)
    {
        /* check for full slab */
        if (slab->m_ntypes == cache->m_ntypes)
        {
            /* remove from 'used' queue */
            QUEUE_REMOVE_NAMED(slab, slab_);

            /* insert onto 'free' queue (head) */
            QUEUE_INSERT_HEAD_NAMED(&(cache->m_free_head), slab, slab_);
        }

        /* push front */
        bufctl->m_next = slab->m_sp;
        slab->m_sp = bufctl;

        /* update stats */
        cache->m_slab_stats.m_free      += 1;
        cache->m_slab_stats.m_mem_alloc -= cache->m_type_size;

        /* decrement usage, check for empty slab */
        if ((slab->m_ntypes -= 1) == 0)
        {
            /* remove from 'free' queue */
            QUEUE_REMOVE_NAMED(slab, slab_);

            /* update stats */
            cache->m_slab_stats.m_mem_total -= cache->m_slab_size;

            /* free 'empty' slab */
            RTL_MEMORY_LOCK_RELEASE(&(cache->m_slab_lock));
            rtl_cache_slab_destroy (cache, slab);
            return;
        }
    }

    RTL_MEMORY_LOCK_RELEASE(&(cache->m_slab_lock));
}

int rtl_cache_magazine_constructor(void * obj, SAL_UNUSED_PARAMETER void *)
{
    rtl_cache_magazine_type * mag = static_cast<rtl_cache_magazine_type*>(obj);
    /* @@@ sal_Size size = (sal_Size)(arg); @@@ */

    mag->m_mag_next = nullptr;
    mag->m_mag_size = RTL_CACHE_MAGAZINE_SIZE;
    mag->m_mag_used = 0;

    return 1;
}

void rtl_cache_magazine_destructor(void * obj, SAL_UNUSED_PARAMETER void *)
{
    rtl_cache_magazine_type * mag = static_cast< rtl_cache_magazine_type * >(
        obj);
    assert(!mag->m_mag_next); // assure removed from queue(s)
    assert(mag->m_mag_used == 0); // assure no longer referenced
    (void) mag; // avoid warnings
}

void rtl_cache_magazine_clear(
    rtl_cache_type *          cache,
    rtl_cache_magazine_type * mag
)
{
    for (; mag->m_mag_used > 0; --mag->m_mag_used)
    {
        void * obj = mag->m_objects[mag->m_mag_used-1];
        mag->m_objects[mag->m_mag_used-1] = nullptr;

        if (cache->m_destructor)
        {
            /* destruct object */
            (cache->m_destructor)(obj, cache->m_userarg);
        }

        /* return buffer to slab layer */
        rtl_cache_slab_free (cache, obj);
    }
}

/**
    @precond cache->m_depot_lock acquired.
*/
inline void rtl_cache_depot_enqueue(
    rtl_cache_depot_type *    depot,
    rtl_cache_magazine_type * mag
)
{
    /* enqueue empty magazine */
    mag->m_mag_next = depot->m_mag_next;
    depot->m_mag_next = mag;

    /* update depot stats */
    depot->m_mag_count++;
}

/**
    @precond cache->m_depot_lock acquired.
*/
inline rtl_cache_magazine_type * rtl_cache_depot_dequeue(
    rtl_cache_depot_type * depot
)
{
    rtl_cache_magazine_type * mag = nullptr;
    if (depot->m_mag_count > 0)
    {
        /* dequeue magazine */
        assert(depot->m_mag_next);

        mag = depot->m_mag_next;
        depot->m_mag_next = mag->m_mag_next;
        mag->m_mag_next = nullptr;

        /* update depot stats */
        depot->m_mag_count--;
        if(depot->m_curr_min > depot->m_mag_count)
        {
            depot->m_curr_min = depot->m_mag_count;
        }
    }
    return mag;
}

/**
    @precond cache->m_depot_lock acquired.
*/
inline rtl_cache_magazine_type * rtl_cache_depot_exchange_alloc(
    rtl_cache_type *          cache,
    rtl_cache_magazine_type * empty
)
{
    rtl_cache_magazine_type * full;

    assert(!empty || empty->m_mag_used == 0);

    /* dequeue full magazine */
    full = rtl_cache_depot_dequeue (&(cache->m_depot_full));
    if (full && empty)
    {
        /* enqueue empty magazine */
        rtl_cache_depot_enqueue (&(cache->m_depot_empty), empty);
    }

    assert(!full || (full->m_mag_used > 0));

    return full;
}

/**
    @precond cache->m_depot_lock acquired.
*/
inline rtl_cache_magazine_type * rtl_cache_depot_exchange_free(
    rtl_cache_type *          cache,
    rtl_cache_magazine_type * full
)
{
    rtl_cache_magazine_type * empty;

    assert(!full || (full->m_mag_used > 0));

    /* dequeue empty magazine */
    empty = rtl_cache_depot_dequeue (&(cache->m_depot_empty));
    if (empty && full)
    {
        /* enqueue full magazine */
        rtl_cache_depot_enqueue (&(cache->m_depot_full), full);
    }

    assert(!empty || (empty->m_mag_used == 0));

    return empty;
}

/**
    @precond cache->m_depot_lock acquired.
*/
bool rtl_cache_depot_populate(rtl_cache_type * cache)
{
    rtl_cache_magazine_type * empty = nullptr;

    if (cache->m_magazine_cache)
    {
        /* allocate new empty magazine */
        RTL_MEMORY_LOCK_RELEASE(&(cache->m_depot_lock));
        empty = static_cast<rtl_cache_magazine_type*>(rtl_cache_alloc (cache->m_magazine_cache));
        RTL_MEMORY_LOCK_ACQUIRE(&(cache->m_depot_lock));
        if (empty)
        {
            /* enqueue (new) empty magazine */
            rtl_cache_depot_enqueue (&(cache->m_depot_empty), empty);
        }
    }
    return (empty != nullptr);
}

int rtl_cache_constructor(void * obj)
{
    rtl_cache_type * cache = static_cast<rtl_cache_type*>(obj);

    memset (cache, 0, sizeof(rtl_cache_type));

    /* linkage */
    QUEUE_START_NAMED(cache, cache_);

    /* slab layer */
    RTL_MEMORY_LOCK_INIT(&(cache->m_slab_lock));

    QUEUE_START_NAMED(&(cache->m_free_head), slab_);
    QUEUE_START_NAMED(&(cache->m_used_head), slab_);

    cache->m_hash_table = cache->m_hash_table_0;
    cache->m_hash_size  = RTL_CACHE_HASH_SIZE;
    cache->m_hash_shift = highbit(cache->m_hash_size) - 1;

    /* depot layer */
    RTL_MEMORY_LOCK_INIT(&(cache->m_depot_lock));

    return 1;
}

void rtl_cache_destructor(void * obj)
{
    rtl_cache_type * cache = static_cast<rtl_cache_type*>(obj);

    /* linkage */
    assert(QUEUE_STARTED_NAMED(cache, cache_));

    /* slab layer */
    RTL_MEMORY_LOCK_DESTROY(&(cache->m_slab_lock));

    assert(QUEUE_STARTED_NAMED(&(cache->m_free_head), slab_));
    assert(QUEUE_STARTED_NAMED(&(cache->m_used_head), slab_));

    assert(cache->m_hash_table == cache->m_hash_table_0);
    assert(cache->m_hash_size  == RTL_CACHE_HASH_SIZE);
    assert(cache->m_hash_shift == highbit(cache->m_hash_size) - 1);

    /* depot layer */
    RTL_MEMORY_LOCK_DESTROY(&(cache->m_depot_lock));
}

rtl_cache_type * rtl_cache_activate(
    rtl_cache_type * cache,
    const char *     name,
    size_t           objsize,
    size_t           objalign,
    int  (SAL_CALL * constructor)(void * obj, void * userarg),
    void (SAL_CALL * destructor) (void * obj, void * userarg),
    void *           userarg,
    rtl_arena_type * source,
    int              flags
)
{
    assert(cache);
    if (cache)
    {
        sal_Size slabsize;

        snprintf (cache->m_name, sizeof(cache->m_name), "%s", name);

        /* ensure minimum size (embedded bufctl linkage) */
        if(objsize < sizeof(rtl_cache_bufctl_type*))
        {
            objsize = sizeof(rtl_cache_bufctl_type*);
        }

        if (objalign == 0)
        {
            /* determine default alignment */
            if (objsize >= RTL_MEMORY_ALIGNMENT_8)
                objalign = RTL_MEMORY_ALIGNMENT_8;
            else
                objalign = RTL_MEMORY_ALIGNMENT_4;
        }
        else
        {
            /* ensure minimum alignment */
            if(objalign < RTL_MEMORY_ALIGNMENT_4)
            {
                objalign = RTL_MEMORY_ALIGNMENT_4;
            }
        }
        assert(RTL_MEMORY_ISP2(objalign));

        cache->m_type_size  = objsize = RTL_MEMORY_P2ROUNDUP(objsize, objalign);
        cache->m_type_align = objalign;
        cache->m_type_shift = highbit(cache->m_type_size) - 1;

        cache->m_constructor = constructor;
        cache->m_destructor  = destructor;
        cache->m_userarg     = userarg;

        /* slab layer */
        cache->m_source = source;

        slabsize = source->m_quantum; /* minimum slab size */
        if (flags & RTL_CACHE_FLAG_QUANTUMCACHE)
        {
            /* next power of 2 above 3 * qcache_max */
            if (slabsize < (((sal_Size)1) << highbit(3 * source->m_qcache_max)))
            {
                slabsize = (((sal_Size)1) << highbit(3 * source->m_qcache_max));
            }
        }
        else
        {
            /* waste at most 1/8 of slab */
            if(slabsize < cache->m_type_size * 8)
            {
                slabsize = cache->m_type_size * 8;
            }
        }

        slabsize = RTL_MEMORY_P2ROUNDUP(slabsize, source->m_quantum);
        if (!RTL_MEMORY_ISP2(slabsize))
            slabsize = (((sal_Size)1) << highbit(slabsize));
        cache->m_slab_size = slabsize;

        if (cache->m_slab_size > source->m_quantum)
        {
            assert(gp_cache_slab_cache);
            assert(gp_cache_bufctl_cache);

            cache->m_features  |= RTL_CACHE_FEATURE_HASH;
            cache->m_ntypes     = cache->m_slab_size / cache->m_type_size;
            cache->m_ncolor_max = cache->m_slab_size % cache->m_type_size;
        }
        else
        {
            /* embedded slab struct */
            cache->m_ntypes     = (cache->m_slab_size - sizeof(rtl_cache_slab_type)) / cache->m_type_size;
            cache->m_ncolor_max = (cache->m_slab_size - sizeof(rtl_cache_slab_type)) % cache->m_type_size;
        }

        assert(cache->m_ntypes > 0);
        cache->m_ncolor = 0;

        if (flags & RTL_CACHE_FLAG_BULKDESTROY)
        {
            /* allow bulk slab delete upon cache deactivation */
            cache->m_features |= RTL_CACHE_FEATURE_BULKDESTROY;
        }

        /* magazine layer */
        if (!(flags & RTL_CACHE_FLAG_NOMAGAZINE))
        {
            assert(gp_cache_magazine_cache);
            cache->m_magazine_cache = gp_cache_magazine_cache;
        }

        /* insert into cache list */
        RTL_MEMORY_LOCK_ACQUIRE(&(g_cache_list.m_lock));
        QUEUE_INSERT_TAIL_NAMED(&(g_cache_list.m_cache_head), cache, cache_);
        RTL_MEMORY_LOCK_RELEASE(&(g_cache_list.m_lock));
    }
    return cache;
}

void rtl_cache_deactivate(rtl_cache_type * cache)
{
    /* remove from cache list */
    RTL_MEMORY_LOCK_ACQUIRE(&(g_cache_list.m_lock));
    bool active = !QUEUE_STARTED_NAMED(cache, cache_);
    QUEUE_REMOVE_NAMED(cache, cache_);
    RTL_MEMORY_LOCK_RELEASE(&(g_cache_list.m_lock));

    assert(active); // orphaned cache
    (void)active;

    /* cleanup magazine layer */
    if (cache->m_magazine_cache)
    {
        rtl_cache_type *          mag_cache;
        rtl_cache_magazine_type * mag;

        /* prevent recursion */
        mag_cache = cache->m_magazine_cache;
        cache->m_magazine_cache = nullptr;

        /* cleanup cpu layer */
        if ((mag = cache->m_cpu_curr))
        {
            // coverity[missing_lock]
            cache->m_cpu_curr = nullptr;
            rtl_cache_magazine_clear (cache, mag);
            rtl_cache_free (mag_cache, mag);
        }

        if ((mag = cache->m_cpu_prev))
        {
            // coverity[missing_lock]
            cache->m_cpu_prev = nullptr;
            rtl_cache_magazine_clear (cache, mag);
            rtl_cache_free (mag_cache, mag);
        }

        /* cleanup depot layer */
        while ((mag = rtl_cache_depot_dequeue(&(cache->m_depot_full))))
        {
            rtl_cache_magazine_clear (cache, mag);
            rtl_cache_free (mag_cache, mag);
        }

        while ((mag = rtl_cache_depot_dequeue(&(cache->m_depot_empty))))
        {
            rtl_cache_magazine_clear (cache, mag);
            rtl_cache_free (mag_cache, mag);
        }
    }

    /* cleanup slab layer */
    if (cache->m_slab_stats.m_alloc > cache->m_slab_stats.m_free)
    {
        if (cache->m_features & RTL_CACHE_FEATURE_HASH)
        {
            /* cleanup bufctl(s) for leaking buffer(s) */
            sal_Size i, n = cache->m_hash_size;
            for (i = 0; i < n; i++)
            {
                rtl_cache_bufctl_type * bufctl;
                while ((bufctl = cache->m_hash_table[i]))
                {
                    /* pop from hash table */
                    cache->m_hash_table[i] = bufctl->m_next;
                    bufctl->m_next = nullptr;

                    /* return to bufctl cache */
                    rtl_cache_free (gp_cache_bufctl_cache, bufctl);
                }
            }
        }
        {
            /* force cleanup of remaining slabs */
            rtl_cache_slab_type *head, *slab;

            head = &(cache->m_used_head);
            for (slab = head->m_slab_next; slab != head; slab = head->m_slab_next)
            {
                /* remove from 'used' queue */
                QUEUE_REMOVE_NAMED(slab, slab_);

                /* update stats */
                cache->m_slab_stats.m_mem_total -= cache->m_slab_size;

                /* free slab */
                rtl_cache_slab_destroy (cache, slab);
            }

            head = &(cache->m_free_head);
            for (slab = head->m_slab_next; slab != head; slab = head->m_slab_next)
            {
                /* remove from 'free' queue */
                QUEUE_REMOVE_NAMED(slab, slab_);

                /* update stats */
                cache->m_slab_stats.m_mem_total -= cache->m_slab_size;

                /* free slab */
                rtl_cache_slab_destroy (cache, slab);
            }
        }
    }

    if (cache->m_hash_table != cache->m_hash_table_0)
    {
        rtl_arena_free (
            gp_cache_arena,
            cache->m_hash_table,
            cache->m_hash_size * sizeof(rtl_cache_bufctl_type*));

        cache->m_hash_table = cache->m_hash_table_0;
        cache->m_hash_size  = RTL_CACHE_HASH_SIZE;
        cache->m_hash_shift = highbit(cache->m_hash_size) - 1;
    }
}

} //namespace

rtl_cache_type * SAL_CALL rtl_cache_create(
    const char *     name,
    sal_Size         objsize,
    sal_Size         objalign,
    int  (SAL_CALL * constructor)(void * obj, void * userarg),
    void (SAL_CALL * destructor) (void * obj, void * userarg),
    void (SAL_CALL * /*reclaim*/)    (void * userarg),
    void *           userarg,
    rtl_arena_type * source,
    int              flags
) SAL_THROW_EXTERN_C()
{
    rtl_cache_type * result = nullptr;
    sal_Size         size   = sizeof(rtl_cache_type);

try_alloc:
    result = static_cast<rtl_cache_type*>(rtl_arena_alloc (gp_cache_arena, &size));
    if (result)
    {
        rtl_cache_type * cache = result;
        (void) rtl_cache_constructor (cache);

        if (!source)
        {
            /* use default arena */
            assert(gp_default_arena);
            source = gp_default_arena;
        }

        result = rtl_cache_activate (
            cache,
            name,
            objsize,
            objalign,
            constructor,
            destructor,
            userarg,
            source,
            flags
        );

        if (!result)
        {
            /* activation failed */
            rtl_cache_deactivate (cache);
            rtl_cache_destructor (cache);
            rtl_arena_free (gp_cache_arena, cache, size);
        }
    }
    else if (!gp_cache_arena)
    {
        ensureCacheSingleton();
        if (gp_cache_arena)
        {
            /* try again */
            goto try_alloc;
        }
    }
    return result;
}

void SAL_CALL rtl_cache_destroy(rtl_cache_type * cache) SAL_THROW_EXTERN_C()
{
    if (cache)
    {
        rtl_cache_deactivate (cache);
        rtl_cache_destructor (cache);
        rtl_arena_free (gp_cache_arena, cache, sizeof(rtl_cache_type));
    }
}

void * SAL_CALL rtl_cache_alloc(rtl_cache_type * cache) SAL_THROW_EXTERN_C()
{
    void * obj = nullptr;

    if (!cache)
        return nullptr;

    if (alloc_mode == AllocMode::SYSTEM)
    {
        obj = rtl_allocateMemory(cache->m_type_size);
        if (obj && cache->m_constructor)
        {
            if (!(cache->m_constructor)(obj, cache->m_userarg))
            {
                /* construction failure */
                rtl_freeMemory(obj);
                obj = nullptr;
            }
        }
        return obj;
    }

    RTL_MEMORY_LOCK_ACQUIRE(&(cache->m_depot_lock));
    if (SAL_LIKELY(cache->m_cpu_curr))
    {
        for (;;)
        {
            /* take object from magazine layer */
            rtl_cache_magazine_type *curr, *prev, *temp;

            curr = cache->m_cpu_curr;
            if (curr && (curr->m_mag_used > 0))
            {
                obj = curr->m_objects[--curr->m_mag_used];
                cache->m_cpu_stats.m_alloc += 1;
                RTL_MEMORY_LOCK_RELEASE(&(cache->m_depot_lock));

                return obj;
            }

            prev = cache->m_cpu_prev;
            if (prev && (prev->m_mag_used > 0))
            {
                temp = cache->m_cpu_curr;
                cache->m_cpu_curr = cache->m_cpu_prev;
                cache->m_cpu_prev = temp;

                continue;
            }

            temp = rtl_cache_depot_exchange_alloc (cache, prev);
            if (temp)
            {
                cache->m_cpu_prev = cache->m_cpu_curr;
                cache->m_cpu_curr = temp;

                continue;
            }

            /* no full magazine: fall through to slab layer */
            break;
        }
    }
    RTL_MEMORY_LOCK_RELEASE(&(cache->m_depot_lock));

    /* alloc buffer from slab layer */
    obj = rtl_cache_slab_alloc (cache);
    if (obj && (cache->m_constructor))
    {
        /* construct object */
        if (!(cache->m_constructor)(obj, cache->m_userarg))
        {
            /* construction failure */
            rtl_cache_slab_free (cache, obj);
            obj = nullptr;
        }
    }
    return obj;
}

void SAL_CALL rtl_cache_free(
    rtl_cache_type * cache,
    void *           obj
) SAL_THROW_EXTERN_C()
{
    if (obj && cache)
    {
        if (alloc_mode == AllocMode::SYSTEM)
        {
            if (cache->m_destructor)
            {
                /* destruct object */
                (cache->m_destructor)(obj, cache->m_userarg);
            }
            rtl_freeMemory(obj);
            return;
        }

        RTL_MEMORY_LOCK_ACQUIRE(&(cache->m_depot_lock));

        for (;;)
        {
            /* return object to magazine layer */
            rtl_cache_magazine_type *curr, *prev, *temp;

            curr = cache->m_cpu_curr;
            if (curr && (curr->m_mag_used < curr->m_mag_size))
            {
                curr->m_objects[curr->m_mag_used++] = obj;
                cache->m_cpu_stats.m_free += 1;
                RTL_MEMORY_LOCK_RELEASE(&(cache->m_depot_lock));

                return;
            }

            prev = cache->m_cpu_prev;
            if (prev && (prev->m_mag_used == 0))
            {
                temp = cache->m_cpu_curr;
                cache->m_cpu_curr = cache->m_cpu_prev;
                cache->m_cpu_prev = temp;

                continue;
            }

            temp = rtl_cache_depot_exchange_free (cache, prev);
            if (temp)
            {
                cache->m_cpu_prev = cache->m_cpu_curr;
                cache->m_cpu_curr = temp;

                continue;
            }

            if (rtl_cache_depot_populate(cache))
            {
                continue;
            }

            /* no empty magazine: fall through to slab layer */
            break;
        }

        RTL_MEMORY_LOCK_RELEASE(&(cache->m_depot_lock));

        /* no space for constructed object in magazine layer */
        if (cache->m_destructor)
        {
            /* destruct object */
            (cache->m_destructor)(obj, cache->m_userarg);
        }

        /* return buffer to slab layer */
        rtl_cache_slab_free (cache, obj);
    }
}

#if defined(SAL_UNX)

void SAL_CALL rtl_secureZeroMemory(void *Ptr, sal_Size Bytes) SAL_THROW_EXTERN_C()
{
    //currently glibc doesn't implement memset_s
    volatile char *p = static_cast<volatile char*>(Ptr);
    while (Bytes--)
        *p++ = 0;
}

static void * rtl_cache_wsupdate_all(void * arg);

static void rtl_cache_wsupdate_init()
{
    RTL_MEMORY_LOCK_ACQUIRE(&(g_cache_list.m_lock));
    g_cache_list.m_update_done = 0;
    (void) pthread_cond_init (&(g_cache_list.m_update_cond), nullptr);
    if (pthread_create (
            &(g_cache_list.m_update_thread), nullptr, rtl_cache_wsupdate_all, reinterpret_cast<void*>(10)) != 0)
    {
        /* failure */
        g_cache_list.m_update_thread = pthread_t();
    }
    RTL_MEMORY_LOCK_RELEASE(&(g_cache_list.m_lock));
}

static void rtl_cache_wsupdate_wait(unsigned int seconds)
{
    if (seconds > 0)
    {
        timeval  now;
        timespec wakeup;

        gettimeofday(&now, nullptr);
        wakeup.tv_sec  = now.tv_sec + seconds;
        wakeup.tv_nsec = now.tv_usec * 1000;

        (void) pthread_cond_timedwait (
            &(g_cache_list.m_update_cond),
            &(g_cache_list.m_lock),
            &wakeup);
    }
}

static void rtl_cache_wsupdate_fini()
{
    RTL_MEMORY_LOCK_ACQUIRE(&(g_cache_list.m_lock));
    g_cache_list.m_update_done = 1;
    pthread_cond_signal (&(g_cache_list.m_update_cond));
    RTL_MEMORY_LOCK_RELEASE(&(g_cache_list.m_lock));

    if (g_cache_list.m_update_thread != pthread_t())
        pthread_join (g_cache_list.m_update_thread, nullptr);
}

#elif defined(SAL_W32)

void SAL_CALL rtl_secureZeroMemory(void *Ptr, sal_Size Bytes) SAL_THROW_EXTERN_C()
{
    RtlSecureZeroMemory(Ptr, Bytes);
}

static DWORD WINAPI rtl_cache_wsupdate_all(void * arg);

static void rtl_cache_wsupdate_init()
{
    DWORD dwThreadId;

    RTL_MEMORY_LOCK_ACQUIRE(&(g_cache_list.m_lock));
    g_cache_list.m_update_done = 0;
    g_cache_list.m_update_cond = CreateEventW (nullptr, TRUE, FALSE, nullptr);

    g_cache_list.m_update_thread =
        CreateThread (nullptr, 0, rtl_cache_wsupdate_all, reinterpret_cast<LPVOID>(10), 0, &dwThreadId);
    RTL_MEMORY_LOCK_RELEASE(&(g_cache_list.m_lock));
}

static void rtl_cache_wsupdate_wait(unsigned int seconds)
{
    if (seconds > 0)
    {
        RTL_MEMORY_LOCK_RELEASE(&(g_cache_list.m_lock));
        WaitForSingleObject (g_cache_list.m_update_cond, (DWORD)(seconds * 1000));
        RTL_MEMORY_LOCK_ACQUIRE(&(g_cache_list.m_lock));
    }
}

static void rtl_cache_wsupdate_fini()
{
    RTL_MEMORY_LOCK_ACQUIRE(&(g_cache_list.m_lock));
    g_cache_list.m_update_done = 1;
    SetEvent (g_cache_list.m_update_cond);
    RTL_MEMORY_LOCK_RELEASE(&(g_cache_list.m_lock));

    WaitForSingleObject (g_cache_list.m_update_thread, INFINITE);
}

#endif /* SAL_UNX || SAL_W32 */

/**
    update depot stats and purge excess magazines.

    @precond cache->m_depot_lock acquired
*/
static void rtl_cache_depot_wsupdate(
    rtl_cache_type *       cache,
    rtl_cache_depot_type * depot
)
{
    sal_Size npurge;

    depot->m_prev_min = depot->m_curr_min;
    depot->m_curr_min = depot->m_mag_count;

    npurge = depot->m_curr_min < depot->m_prev_min ? depot->m_curr_min : depot->m_prev_min;
    for (; npurge > 0; npurge--)
    {
        rtl_cache_magazine_type * mag = rtl_cache_depot_dequeue (depot);
        if (mag)
        {
            RTL_MEMORY_LOCK_RELEASE(&(cache->m_depot_lock));
            rtl_cache_magazine_clear (cache, mag);
            rtl_cache_free (cache->m_magazine_cache, mag);
            RTL_MEMORY_LOCK_ACQUIRE(&(cache->m_depot_lock));
        }
    }
    // coverity[missing_unlock]
}

/**
    @precond cache->m_depot_lock released
*/
static void rtl_cache_wsupdate(rtl_cache_type * cache)
{
    if (cache->m_magazine_cache)
    {
        RTL_MEMORY_LOCK_ACQUIRE(&(cache->m_depot_lock));

        rtl_cache_depot_wsupdate (cache, &(cache->m_depot_full));
        rtl_cache_depot_wsupdate (cache, &(cache->m_depot_empty));

        RTL_MEMORY_LOCK_RELEASE(&(cache->m_depot_lock));
    }
}

#if defined(SAL_UNX)
static void *
#elif defined(SAL_W32)
static DWORD WINAPI
#endif /* SAL_UNX || SAL_W32 */
rtl_cache_wsupdate_all(void * arg)
{
    osl::Thread::setName("rtl_cache_wsupdate_all");
    unsigned int seconds = sal::static_int_cast< unsigned int >(
        reinterpret_cast< sal_uIntPtr >(arg));

    RTL_MEMORY_LOCK_ACQUIRE(&(g_cache_list.m_lock));
    while (!g_cache_list.m_update_done)
    {
        rtl_cache_wsupdate_wait (seconds);
        if (!g_cache_list.m_update_done)
        {
            rtl_cache_type * head, * cache;

            head = &(g_cache_list.m_cache_head);
            for (cache  = head->m_cache_next;
                 cache != head;
                 cache  = cache->m_cache_next)
            {
                rtl_cache_wsupdate (cache);
            }
        }
    }
    RTL_MEMORY_LOCK_RELEASE(&(g_cache_list.m_lock));

#if defined(SAL_UNX)
    return nullptr;
#elif defined(SAL_W32)
    return 0;
#endif
}

void rtl_cache_init()
{
    {
        /* list of caches */
        RTL_MEMORY_LOCK_INIT(&(g_cache_list.m_lock));
        (void) rtl_cache_constructor (&(g_cache_list.m_cache_head));
    }
    {
        /* cache: internal arena */
        assert(!gp_cache_arena);

        gp_cache_arena = rtl_arena_create (
            "rtl_cache_internal_arena",
            64,   /* quantum */
            0,    /* no quantum caching */
            nullptr, /* default source */
            rtl_arena_alloc,
            rtl_arena_free,
            0     /* flags */
        );
        assert(gp_cache_arena);

        /* check 'gp_default_arena' initialization */
        assert(gp_default_arena);
    }
    {
        /* cache: magazine cache */
        static rtl_cache_type g_cache_magazine_cache;

        assert(!gp_cache_magazine_cache);
        (void) rtl_cache_constructor (&g_cache_magazine_cache);

        gp_cache_magazine_cache = rtl_cache_activate (
            &g_cache_magazine_cache,
            "rtl_cache_magazine_cache",
            sizeof(rtl_cache_magazine_type), /* objsize  */
            0,                               /* objalign */
            rtl_cache_magazine_constructor,
            rtl_cache_magazine_destructor,
            nullptr, /* userarg: NYI */
            gp_default_arena, /* source */
            RTL_CACHE_FLAG_NOMAGAZINE /* during bootstrap; activated below */
        );
        assert(gp_cache_magazine_cache);

        /* activate magazine layer */
        g_cache_magazine_cache.m_magazine_cache = gp_cache_magazine_cache;
    }
    {
        /* cache: slab (struct) cache */
        static rtl_cache_type g_cache_slab_cache;

        assert(!gp_cache_slab_cache);
        (void) rtl_cache_constructor (&g_cache_slab_cache);

        gp_cache_slab_cache = rtl_cache_activate (
            &g_cache_slab_cache,
            "rtl_cache_slab_cache",
            sizeof(rtl_cache_slab_type), /* objsize  */
            0,                           /* objalign */
            rtl_cache_slab_constructor,
            rtl_cache_slab_destructor,
            nullptr,                           /* userarg: none */
            gp_default_arena,            /* source */
            0                            /* flags: none */
        );
        assert(gp_cache_slab_cache);
    }
    {
        /* cache: bufctl cache */
        static rtl_cache_type g_cache_bufctl_cache;

        assert(!gp_cache_bufctl_cache);
        (void) rtl_cache_constructor (&g_cache_bufctl_cache);

        gp_cache_bufctl_cache = rtl_cache_activate (
            &g_cache_bufctl_cache,
            "rtl_cache_bufctl_cache",
            sizeof(rtl_cache_bufctl_type), /* objsize */
            0,                             /* objalign  */
            nullptr,                /* constructor */
            nullptr,                /* destructor */
            nullptr,                /* userarg */
            gp_default_arena, /* source */
            0                 /* flags: none */
        );
        assert(gp_cache_bufctl_cache);
    }

    rtl_cache_wsupdate_init();
}

void rtl_cache_fini()
{
    if (gp_cache_arena)
    {
        rtl_cache_type * cache, * head;

        rtl_cache_wsupdate_fini();

        if (gp_cache_bufctl_cache)
        {
            cache = gp_cache_bufctl_cache;
            gp_cache_bufctl_cache = nullptr;
            rtl_cache_deactivate (cache);
            rtl_cache_destructor (cache);
        }

        if (gp_cache_slab_cache)
        {
            cache = gp_cache_slab_cache;
            gp_cache_slab_cache = nullptr;
            rtl_cache_deactivate (cache);
            rtl_cache_destructor (cache);
        }

        if (gp_cache_magazine_cache)
        {
            cache = gp_cache_magazine_cache;
            gp_cache_magazine_cache = nullptr;
            rtl_cache_deactivate (cache);
            rtl_cache_destructor (cache);
        }

        if (gp_cache_arena)
        {
            rtl_arena_destroy (gp_cache_arena);
            gp_cache_arena = nullptr;
        }

        RTL_MEMORY_LOCK_ACQUIRE(&(g_cache_list.m_lock));
        head = &(g_cache_list.m_cache_head);
        for (cache = head->m_cache_next; cache != head; cache = cache->m_cache_next)
        {
            // noop
        }
        RTL_MEMORY_LOCK_RELEASE(&(g_cache_list.m_lock));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
