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
#include <rtllifecycle.h>

#include <cassert>
#include <cstdlib>
#include <string.h>
#include <stdio.h>

/**
    provided for cache_type allocations, and hash_table resizing.

    @internal
*/
static rtl_arena_type * gp_cache_arena = nullptr;

namespace
{

rtl_cache_type * rtl_cache_activate(
    rtl_cache_type * cache,
    const char *     name,
    size_t           objsize,
    size_t           objalign,
    int  (SAL_CALL * constructor)(void * obj, void * userarg),
    void (SAL_CALL * destructor) (void * obj, void * userarg),
    void *           userarg
)
{
    assert(cache);

    snprintf (cache->m_name, sizeof(cache->m_name), "%s", name);

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

    cache->m_type_size = RTL_MEMORY_P2ROUNDUP(objsize, objalign);

    cache->m_constructor = constructor;
    cache->m_destructor  = destructor;
    cache->m_userarg     = userarg;

    return cache;
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
    rtl_arena_type *,
    int
) SAL_THROW_EXTERN_C()
{
    rtl_cache_type * result = nullptr;
    sal_Size         size   = sizeof(rtl_cache_type);

try_alloc:
    result = static_cast<rtl_cache_type*>(rtl_arena_alloc (gp_cache_arena, &size));
    if (result)
    {
        rtl_cache_type * cache = result;
        memset (cache, 0, sizeof(rtl_cache_type));

        result = rtl_cache_activate (
            cache,
            name,
            objsize,
            objalign,
            constructor,
            destructor,
            userarg
        );

        if (!result)
        {
            /* activation failed */
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
        rtl_arena_free (gp_cache_arena, cache, sizeof(rtl_cache_type));
    }
}

void * SAL_CALL rtl_cache_alloc(rtl_cache_type * cache) SAL_THROW_EXTERN_C()
{
    void * obj = nullptr;

    if (!cache)
        return nullptr;

    obj = std::malloc(cache->m_type_size);
    if (obj && cache->m_constructor)
    {
        if (!(cache->m_constructor)(obj, cache->m_userarg))
        {
            /* construction failure */
            std::free(obj);
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
        if (cache->m_destructor)
        {
            /* destruct object */
            (cache->m_destructor)(obj, cache->m_userarg);
        }
        std::free(obj);
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

#elif defined(_WIN32)

void SAL_CALL rtl_secureZeroMemory(void *Ptr, sal_Size Bytes) SAL_THROW_EXTERN_C()
{
    RtlSecureZeroMemory(Ptr, Bytes);
}

#endif /* SAL_UNX || _WIN32 */

void rtl_cache_init()
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

void rtl_cache_fini()
{
    if (gp_cache_arena)
    {
        rtl_arena_destroy (gp_cache_arena);
        gp_cache_arena = nullptr;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
