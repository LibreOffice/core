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

#include "alloc_impl.hxx"
#include <rtl/alloc.h>
#include <sal/log.hxx>
#include <sal/macros.h>

#include <cassert>
#include <string.h>
#include <stdio.h>

#include <rtllifecycle.h>
#include <oslmemory.h>

AllocMode alloc_mode = AllocMode::UNSET;

#if !defined(FORCE_SYSALLOC)
static void determine_alloc_mode()
{
    assert(alloc_mode == AllocMode::UNSET);
    alloc_mode = AllocMode::SYSTEM;
}

static const sal_Size g_alloc_sizes[] =
{
    /* powers of 2**(1/4) */
    4 *    4,           6 *    4,
    4 *    8, 5 *    8, 6 *    8, 7 *    8,
    4 *   16, 5 *   16, 6 *   16, 7 *   16,
    4 *   32, 5 *   32, 6 *   32, 7 *   32,
    4 *   64, 5 *   64, 6 *   64, 7 *   64,
    4 *  128, 5 *  128, 6 *  128, 7 *  128,
    4 *  256, 5 *  256, 6 *  256, 7 *  256,
    4 *  512, 5 *  512, 6 *  512, 7 *  512,
    4 * 1024, 5 * 1024, 6 * 1024, 7 * 1024,
    4 * 2048, 5 * 2048, 6 * 2048, 7 * 2048,
    4 * 4096
};

#define RTL_MEMORY_CACHED_LIMIT 4 * 4096
#define RTL_MEMORY_CACHED_SIZES (SAL_N_ELEMENTS(g_alloc_sizes))

static rtl_cache_type * g_alloc_caches[RTL_MEMORY_CACHED_SIZES] =
{
    nullptr,
};

#define RTL_MEMALIGN       8
#define RTL_MEMALIGN_SHIFT 3

static rtl_cache_type * g_alloc_table[RTL_MEMORY_CACHED_LIMIT >> RTL_MEMALIGN_SHIFT] =
{
    nullptr,
};

static rtl_arena_type * gp_alloc_arena = nullptr;

void * SAL_CALL rtl_allocateMemory_CUSTOM(sal_Size n) SAL_THROW_EXTERN_C()
{
    void * p = nullptr;
    if (n > 0)
    {
        char *     addr;
        sal_Size   size = RTL_MEMORY_ALIGN(n + RTL_MEMALIGN, RTL_MEMALIGN);

        assert(RTL_MEMALIGN >= sizeof(sal_Size));
        if (n >= SAL_MAX_SIZE - (RTL_MEMALIGN + RTL_MEMALIGN - 1))
        {
            /* requested size too large for roundup alignment */
            return nullptr;
        }

try_alloc:
        if (size <= RTL_MEMORY_CACHED_LIMIT)
            addr = static_cast<char*>(rtl_cache_alloc(g_alloc_table[(size - 1) >> RTL_MEMALIGN_SHIFT]));
        else
            addr = static_cast<char*>(rtl_arena_alloc (gp_alloc_arena, &size));

        if (addr)
        {
            reinterpret_cast<sal_Size*>(addr)[0] = size;
            p = addr + RTL_MEMALIGN;
        }
        else if (!gp_alloc_arena)
        {
            ensureMemorySingleton();
            if (gp_alloc_arena)
            {
                /* try again */
                goto try_alloc;
            }
        }
    }
    return p;
}

void SAL_CALL rtl_freeMemory_CUSTOM (void * p) SAL_THROW_EXTERN_C()
{
    if (p)
    {
        char *   addr = static_cast<char*>(p) - RTL_MEMALIGN;
        sal_Size size = reinterpret_cast<sal_Size*>(addr)[0];

        if (size <= RTL_MEMORY_CACHED_LIMIT)
            rtl_cache_free(g_alloc_table[(size - 1) >> RTL_MEMALIGN_SHIFT], addr);
        else
            rtl_arena_free (gp_alloc_arena, addr, size);
    }
}

void * SAL_CALL rtl_reallocateMemory_CUSTOM (void * p, sal_Size n) SAL_THROW_EXTERN_C()
{
    if (n > 0)
    {
        if (p)
        {
            void *   p_old = p;
            sal_Size n_old = reinterpret_cast<sal_Size*>( static_cast<char*>(p) - RTL_MEMALIGN  )[0] - RTL_MEMALIGN;

            p = rtl_allocateMemory (n);
            if (p)
            {
                memcpy (p, p_old, (n < n_old) ? n : n_old);
                rtl_freeMemory (p_old);
            }
        }
        else
        {
            p = rtl_allocateMemory (n);
        }
    }
    else if (p)
    {
        rtl_freeMemory (p);
        p = nullptr;
    }
    return p;
}

#endif

void rtl_memory_init()
{
#if !defined(FORCE_SYSALLOC)
    {
        /* global memory arena */
        assert(!gp_alloc_arena);

        gp_alloc_arena = rtl_arena_create (
            "rtl_alloc_arena",
            2048,     /* quantum */
            0,        /* w/o quantum caching */
            nullptr,        /* default source */
            rtl_arena_alloc,
            rtl_arena_free,
            0         /* flags */
        );
        assert(gp_alloc_arena);
    }
    {
        sal_Size size;
        int i, n = RTL_MEMORY_CACHED_SIZES;

        for (i = 0; i < n; i++)
        {
            char name[RTL_CACHE_NAME_LENGTH + 1];
            (void) snprintf (name, sizeof(name), "rtl_alloc_%" SAL_PRIuUINTPTR, g_alloc_sizes[i]);
            g_alloc_caches[i] = rtl_cache_create (name, g_alloc_sizes[i], 0, nullptr, nullptr, nullptr, nullptr, nullptr, 0);
        }

        size = RTL_MEMALIGN;
        for (i = 0; i < n; i++)
        {
            while (size <= g_alloc_sizes[i])
            {
                g_alloc_table[(size - 1) >> RTL_MEMALIGN_SHIFT] = g_alloc_caches[i];
                size += RTL_MEMALIGN;
            }
        }
    }
#endif
}

void rtl_memory_fini()
{
#if !defined(FORCE_SYSALLOC)
    int i, n;

    /* clear g_alloc_table */
    // cppcheck-suppress sizeofwithsilentarraypointer
    memset (g_alloc_table, 0, sizeof(g_alloc_table));

    /* cleanup g_alloc_caches */
    for (i = 0, n = RTL_MEMORY_CACHED_SIZES; i < n; i++)
    {
        if (g_alloc_caches[i])
        {
            rtl_cache_destroy (g_alloc_caches[i]);
            g_alloc_caches[i] = nullptr;
        }
    }

    /* cleanup gp_alloc_arena */
    if (gp_alloc_arena)
    {
        rtl_arena_destroy (gp_alloc_arena);
        gp_alloc_arena = nullptr;
    }
#endif
}

void * SAL_CALL rtl_allocateMemory_SYSTEM(sal_Size n)
{
    return malloc (n);
}

void SAL_CALL rtl_freeMemory_SYSTEM(void * p)
{
    free (p);
}

void * SAL_CALL rtl_reallocateMemory_SYSTEM(void * p, sal_Size n)
{
    return realloc (p, n);
}

void* SAL_CALL rtl_allocateMemory(sal_Size n) SAL_THROW_EXTERN_C()
{
    SAL_WARN_IF(
        n >= SAL_MAX_INT32, "sal.rtl",
        "suspicious massive alloc " << n);
#if !defined(FORCE_SYSALLOC)
    while (true)
    {
        if (alloc_mode == AllocMode::CUSTOM)
        {
            return rtl_allocateMemory_CUSTOM(n);
        }
        if (alloc_mode == AllocMode::SYSTEM)
        {
            return rtl_allocateMemory_SYSTEM(n);
        }
        determine_alloc_mode();
    }
#else
    return rtl_allocateMemory_SYSTEM(n);
#endif
}

void* SAL_CALL rtl_reallocateMemory(void * p, sal_Size n) SAL_THROW_EXTERN_C()
{
    SAL_WARN_IF(
        n >= SAL_MAX_INT32, "sal.rtl",
        "suspicious massive alloc " << n);
#if !defined(FORCE_SYSALLOC)
    while (true)
    {
        if (alloc_mode == AllocMode::CUSTOM)
        {
            return rtl_reallocateMemory_CUSTOM(p,n);
        }
        if (alloc_mode == AllocMode::SYSTEM)
        {
            return rtl_reallocateMemory_SYSTEM(p,n);
        }
        determine_alloc_mode();
    }
#else
    return rtl_reallocateMemory_SYSTEM(p,n);
#endif
}

void SAL_CALL rtl_freeMemory(void * p) SAL_THROW_EXTERN_C()
{
#if !defined(FORCE_SYSALLOC)
    while (true)
    {
        if (alloc_mode == AllocMode::CUSTOM)
        {
            rtl_freeMemory_CUSTOM(p);
            return;
        }
        if (alloc_mode == AllocMode::SYSTEM)
        {
            rtl_freeMemory_SYSTEM(p);
            return;
        }
        determine_alloc_mode();
    }
#else
    rtl_freeMemory_SYSTEM(p);
#endif
}

void * SAL_CALL rtl_allocateZeroMemory(sal_Size n) SAL_THROW_EXTERN_C()
{
    void * p = rtl_allocateMemory (n);
    if (p)
        memset (p, 0, n);
    return p;
}

void SAL_CALL rtl_freeZeroMemory(void * p, sal_Size n) SAL_THROW_EXTERN_C()
{
    if (p)
    {
        rtl_secureZeroMemory (p, n);
        rtl_freeMemory (p);
    }
}

void* SAL_CALL rtl_allocateAlignedMemory(sal_Size Alignment, sal_Size Bytes) SAL_THROW_EXTERN_C()
{
    return osl_aligned_alloc(Alignment, Bytes);
}

void SAL_CALL rtl_freeAlignedMemory(void* Ptr) SAL_THROW_EXTERN_C()
{
    osl_aligned_free(Ptr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
