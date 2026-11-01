/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <config_fuzzers.h>

#include <vcl/throwawaymemory.hxx>
#include <pagememory.hxx>

#if defined __cpp_lib_memory_resource

#include <new>

namespace
{
/** Whole pages from vcl::pagememory, one mapping for each request. */
class PageResource final : public std::pmr::memory_resource
{
    virtual void* do_allocate(std::size_t nBytes, std::size_t /*nAlignment*/) override
    {
        void* p = vcl::pagememory::allocate(nBytes);
        if (!p)
            throw std::bad_alloc();
        return p;
    }

    virtual void do_deallocate(void* p, std::size_t nBytes, std::size_t /*nAlignment*/) override
    {
        // vcl::pagememory::allocate rounded the size up to whole pages, and does so again here.
        const std::size_t nPageSize = vcl::pagememory::pageSize();
        nBytes = (nBytes + nPageSize - 1) & ~(nPageSize - 1);
#if OSL_DEBUG_LEVEL > 0
        vcl::pagememory::makeInaccessible(p, nBytes);
#else
        vcl::pagememory::release(p, nBytes);
#endif
    }

    virtual bool do_is_equal(const std::pmr::memory_resource& rOther) const noexcept override
    {
        return &rOther == this;
    }
};

/** The size of the first block taken from the system. Later blocks grow from it. */
constexpr std::size_t INITIAL_SIZE = 64 * 1024;

std::pmr::memory_resource* createPages()
{
#if ENABLE_FUZZERS
    // Pages hide individual allocations from the leak checker, so use the default resource.
    return nullptr;
#else
    return new PageResource;
#endif
}
}

ThrowawayMemory::ThrowawayMemory()
    : mpPages(createPages())
    , maBuffer(INITIAL_SIZE, mpPages ? mpPages.get() : std::pmr::get_default_resource())
{
}

ThrowawayMemory::~ThrowawayMemory() = default;

void* ThrowawayMemory::do_allocate(std::size_t nBytes, std::size_t nAlignment)
{
    if (!mpPages)
        return std::pmr::get_default_resource()->allocate(nBytes, nAlignment);
    return maBuffer.allocate(nBytes, nAlignment);
}

void ThrowawayMemory::do_deallocate(void* p, std::size_t nBytes, std::size_t nAlignment)
{
    if (!mpPages)
        std::pmr::get_default_resource()->deallocate(p, nBytes, nAlignment);
}

bool ThrowawayMemory::do_is_equal(const std::pmr::memory_resource& rOther) const noexcept
{
    return &rOther == this;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
