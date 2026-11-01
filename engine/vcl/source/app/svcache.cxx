/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <config_fuzzers.h>

#include <vcl/dropcache.hxx>
#include <pagememory.hxx>
#include <svcache.hxx>
#include <svdata.hxx>

CacheOwner::CacheOwner()
{
    if (ImplSVData* pSVData = ImplGetSVData())
    {
        pSVData->registerCacheOwner(*this);
        return;
    }
    SAL_WARN("vcl.app", "Cache owner ctor before ImplSVData created. This is useless.");
}

CacheOwner::~CacheOwner()
{
    if (ImplSVData* pSVData = ImplGetSVData())
        pSVData->deregisterCacheOwner(*this);
}

#if defined __cpp_lib_memory_resource

#define MEMORY_ALIGN(value, align) (((value) + ((align)-1)) & ~((align)-1))

void* CacheMemory::allocPages(rtl_arena_type* arena, sal_Size* size)
{
    CacheMemory* pCacheMemory = reinterpret_cast<CacheMemory*>(arena);

    std::size_t n = *size;
    void* p = vcl::pagememory::allocate(n);
    if (p != nullptr)
    {
        pCacheMemory->mnAllocatedPages += (n / vcl::pagememory::pageSize());
        pCacheMemory->mnMaxAllocatedPages
            = std::max(pCacheMemory->mnMaxAllocatedPages, pCacheMemory->mnAllocatedPages);
        *size = n;
    }
    return p;
}

void CacheMemory::freePages(rtl_arena_type* arena, void* address, sal_Size size)
{
    CacheMemory* pCacheMemory = reinterpret_cast<CacheMemory*>(arena);
    vcl::pagememory::release(address, size);
    pCacheMemory->mnAllocatedPages -= (size / vcl::pagememory::pageSize());
}

CacheMemory::CacheMemory()
    : maCacheArenas{ nullptr }
    , mnSmallest(SAL_MAX_SIZE)
    , mnLargest(0)
    , mnAllocatedPages(0)
    , mnMaxAllocatedPages(0)
{
}

size_t CacheMemory::GetAllocatedPages() const { return mnAllocatedPages; }

CacheMemory::~CacheMemory()
{
    SAL_INFO("vcl", "cachememory, smallest/largest are: "
                        << mnSmallest << ", " << mnLargest << "total pages allocated: "
                        << mnMaxAllocatedPages << ", current allocated pages" << mnAllocatedPages);
    for (size_t i = 0; i < maCacheArenas.size(); ++i)
    {
        if (!maCacheArenas[i])
            continue;
        SAL_INFO("vcl", "cachememory, destroying arena for alignment: " << (1 << i));
        rtl_arena_destroy(maCacheArenas[i]);
    }
}

void* CacheMemory::do_allocate(std::size_t bytes, std::size_t alignment)
{
    alignment = std::max<std::size_t>(alignment, 4);
    const unsigned int nSlot = o3tl::number_of_bits(alignment) - 1;
    if (!maCacheArenas[nSlot])
    {
        maCacheArenas[nSlot]
            = rtl_arena_create("cache_internal_arena", alignment, 0,
                               reinterpret_cast<rtl_arena_type*>(this), allocPages, freePages, 0);
    }

    mnSmallest = std::min(mnSmallest, bytes);
    mnLargest = std::max(mnLargest, bytes);
    sal_Size size = MEMORY_ALIGN(bytes, alignment);
    return rtl_arena_alloc(maCacheArenas[nSlot], &size);
}

void CacheMemory::do_deallocate(void* p, std::size_t bytes, std::size_t alignment)
{
    alignment = std::max<std::size_t>(alignment, 4);
    const unsigned int nSlot = o3tl::number_of_bits(alignment) - 1;
    sal_Size size = MEMORY_ALIGN(bytes, alignment);
    rtl_arena_free(maCacheArenas[nSlot], p, size);
}

bool CacheMemory::do_is_equal(const std::pmr::memory_resource& other) const noexcept
{
    SAL_WARN("vcl", "CacheMemory::do_is_equal called");
    return &other == this;
}

//static
CacheMemory& CacheMemory::GetMemoryResource()
{
    static CacheMemory aCacheMemory;
    return aCacheMemory;
}

//static
std::pmr::memory_resource& CacheOwner::GetMemoryResource()
{
#if ENABLE_FUZZERS
    // The arena allocates in large blocks, which hides individual allocations from the leak
    // checker. Hand out the default resource so every allocation is one the checker can see.
    return *std::pmr::get_default_resource();
#else
    return CacheMemory::GetMemoryResource();
#endif
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
