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

#include "alloc_impl.hxx"
#include "rtl/alloc.h"
#include <sal/log.hxx>
#include <sal/macros.h>

#include <cassert>
#include <string.h>
#include <stdio.h>

#include "internal/rtllifecycle.h"

AllocMode alloc_mode = AMode_UNSET;

#if !defined(FORCE_SYSALLOC)
static void determine_alloc_mode()
{
    assert(alloc_mode == AMode_UNSET);
    alloc_mode = (getenv("G_SLICE") == NULL ? AMode_CUSTOM : AMode_SYSTEM);
}

/* ================================================================= *
 *
 * custom allocator includes.
 *
 * ================================================================= */

#include "sal/macros.h"

/* ================================================================= *
 *
 * custom allocator internals.
 *
 * ================================================================= */

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
    0,
};

#define RTL_MEMALIGN       8
#define RTL_MEMALIGN_SHIFT 3

static rtl_cache_type * g_alloc_table[RTL_MEMORY_CACHED_LIMIT >> RTL_MEMALIGN_SHIFT] =
{
    0,
};

static rtl_arena_type * gp_alloc_arena = 0;

/* ================================================================= *
 *
 * custom allocator implemenation.
 *
 * ================================================================= */

void *
SAL_CALL rtl_allocateMemory_CUSTOM (sal_Size n) SAL_THROW_EXTERN_C()
{
    void * p = 0;
    if (n > 0)
    {
        char *     addr;
        sal_Size   size = RTL_MEMORY_ALIGN(n + RTL_MEMALIGN, RTL_MEMALIGN);

        assert(RTL_MEMALIGN >= sizeof(sal_Size));
        if (n >= SAL_MAX_SIZE - (RTL_MEMALIGN + RTL_MEMALIGN - 1))
        {
            /* requested size too large for roundup alignment */
            return 0;
        }

try_alloc:
        if (size <= RTL_MEMORY_CACHED_LIMIT)
            addr = (char*)rtl_cache_alloc(g_alloc_table[(size - 1) >> RTL_MEMALIGN_SHIFT]);
        else
            addr = (char*)rtl_arena_alloc (gp_alloc_arena, &size);

        if (addr != 0)
        {
            ((sal_Size*)(addr))[0] = size;
            p = addr + RTL_MEMALIGN;
        }
        else if (gp_alloc_arena == 0)
        {
            ensureMemorySingleton();
            if (gp_alloc_arena)
            {
                /* try again */
                goto try_alloc;
            }
        }
    }
    return (p);
}

/* ================================================================= */

void SAL_CALL rtl_freeMemory_CUSTOM (void * p) SAL_THROW_EXTERN_C()
{
    if (p != 0)
    {
        char *   addr = (char*)(p) - RTL_MEMALIGN;
        sal_Size size = ((sal_Size*)(addr))[0];

        if (size <= RTL_MEMORY_CACHED_LIMIT)
            rtl_cache_free(g_alloc_table[(size - 1) >> RTL_MEMALIGN_SHIFT], addr);
        else
            rtl_arena_free (gp_alloc_arena, addr, size);
    }
}

/* ================================================================= */

void * SAL_CALL rtl_reallocateMemory_CUSTOM (void * p, sal_Size n) SAL_THROW_EXTERN_C()
{
    if (n > 0)
    {
        if (p != 0)
        {
            void *   p_old = p;
            sal_Size n_old = ((sal_Size*)( (char*)(p) - RTL_MEMALIGN  ))[0] - RTL_MEMALIGN;

            p = rtl_allocateMemory (n);
            if (p != 0)
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
    else if (p != 0)
    {
        rtl_freeMemory (p), p = 0;
    }
    return (p);
}

#endif

/* ================================================================= *
 *
 * custom allocator initialization / finalization.
 *
 * ================================================================= */

void rtl_memory_init()
{
#if !defined(FORCE_SYSALLOC)
    {
        /* global memory arena */
        assert(gp_alloc_arena == 0);

        gp_alloc_arena = rtl_arena_create (
            "rtl_alloc_arena",
            2048,     /* quantum */
            0,        /* w/o quantum caching */
            0,        /* default source */
            rtl_arena_alloc,
            rtl_arena_free,
            0         /* flags */
        );
        assert(gp_alloc_arena != 0);
    }
    {
        sal_Size size;
        int i, n = RTL_MEMORY_CACHED_SIZES;

        for (i = 0; i < n; i++)
        {
            char name[RTL_CACHE_NAME_LENGTH + 1];
            (void) snprintf (name, sizeof(name), "rtl_alloc_%lu", g_alloc_sizes[i]);
            g_alloc_caches[i] = rtl_cache_create (name, g_alloc_sizes[i], 0, NULL, NULL, NULL, NULL, NULL, 0);
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
    // SAL_INFO("sal.rtl", "rtl_memory_init completed");
}

/* ================================================================= */

void rtl_memory_fini()
{
#if !defined(FORCE_SYSALLOC)
    int i, n;

    /* clear g_alloc_table */
    memset (g_alloc_table, 0, sizeof(g_alloc_table));

    /* cleanup g_alloc_caches */
    for (i = 0, n = RTL_MEMORY_CACHED_SIZES; i < n; i++)
    {
        if (g_alloc_caches[i] != 0)
        {
            rtl_cache_destroy (g_alloc_caches[i]);
            g_alloc_caches[i] = 0;
        }
    }

    /* cleanup gp_alloc_arena */
    if (gp_alloc_arena != 0)
    {
        rtl_arena_destroy (gp_alloc_arena);
        gp_alloc_arena = 0;
    }
#endif
    // SAL_INFO("sal.rtl", "rtl_memory_fini completed");
}

/* ================================================================= *
 *
 * system allocator implemenation.
 *
 * ================================================================= */

void * SAL_CALL rtl_allocateMemory_SYSTEM (sal_Size n)
{
    return malloc (n);
}

/* ================================================================= */

void SAL_CALL rtl_freeMemory_SYSTEM (void * p)
{
    free (p);
}

/* ================================================================= */

void * SAL_CALL rtl_reallocateMemory_SYSTEM (void * p, sal_Size n)
{
    return realloc (p, n);
}

/* ================================================================= */

void* SAL_CALL rtl_allocateMemory (sal_Size n) SAL_THROW_EXTERN_C()
{
    SAL_WARN_IF(
        n >= SAL_MAX_INT32, "sal.rtl",
        "suspicious massive alloc " << n);
#if !defined(FORCE_SYSALLOC)
    while (1)
    {
        if (alloc_mode == AMode_CUSTOM)
        {
            return rtl_allocateMemory_CUSTOM(n);
        }
        if (alloc_mode == AMode_SYSTEM)
        {
            return rtl_allocateMemory_SYSTEM(n);
        }
        determine_alloc_mode();
    }
#else
    return rtl_allocateMemory_SYSTEM(n);
#endif
}

void* SAL_CALL rtl_reallocateMemory (void * p, sal_Size n) SAL_THROW_EXTERN_C()
{
    SAL_WARN_IF(
        n >= SAL_MAX_INT32, "sal.rtl",
        "suspicious massive alloc " << n);
#if !defined(FORCE_SYSALLOC)
    while (1)
    {
        if (alloc_mode == AMode_CUSTOM)
        {
            return rtl_reallocateMemory_CUSTOM(p,n);
        }
        if (alloc_mode == AMode_SYSTEM)
        {
            return rtl_reallocateMemory_SYSTEM(p,n);
        }
        determine_alloc_mode();
    }
#else
    return rtl_reallocateMemory_SYSTEM(p,n);
#endif
}

void SAL_CALL rtl_freeMemory (void * p) SAL_THROW_EXTERN_C()
{
#if !defined(FORCE_SYSALLOC)
    while (1)
    {
        if (alloc_mode == AMode_CUSTOM)
        {
            rtl_freeMemory_CUSTOM(p);
            return;
        }
        if (alloc_mode == AMode_SYSTEM)
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

/* ================================================================= *
 *
 * rtl_(allocate|free)ZeroMemory() implemenation.
 *
 * ================================================================= */

void * SAL_CALL rtl_allocateZeroMemory (sal_Size n) SAL_THROW_EXTERN_C()
{
    void * p = rtl_allocateMemory (n);
    if (p != 0)
        memset (p, 0, n);
    return (p);
}

/* ================================================================= */

void SAL_CALL rtl_freeZeroMemory (void * p, sal_Size n) SAL_THROW_EXTERN_C()
{
    if (p != 0)
    {
        memset (p, 0, n);
        rtl_freeMemory (p);
    }
}

/* ================================================================= */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
