/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <rtl/alloc.h>
#include <o3tl/numeric.hxx>
#include <array>
#include <map>
#include <version>
#if defined __cpp_lib_memory_resource
#include <memory_resource>
#endif

#if defined __cpp_lib_memory_resource

constexpr unsigned int nAlignments = o3tl::number_of_bits(alignof(std::max_align_t));

class CacheMemory : public std::pmr::memory_resource
{
public:
    CacheMemory();
    ~CacheMemory();

    static CacheMemory& GetMemoryResource();

    size_t GetAllocatedPages() const;

private:
    std::array<rtl_arena_type*, nAlignments> maCacheArenas;
    size_t mnSmallest;
    size_t mnLargest;
    size_t mnPageSize;
    size_t mnAllocatedPages;
    size_t mnMaxAllocatedPages;

    static void* allocPages(rtl_arena_type* arena, sal_Size* size);
    static void freePages(rtl_arena_type* arena, void* address, sal_Size size);

    virtual void* do_allocate(std::size_t bytes, std::size_t alignment) override;
    virtual void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) override;
    virtual bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
