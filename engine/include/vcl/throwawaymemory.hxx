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

#pragma once

#include <vcl/dllapi.h>
#include <memory>
#include <version>
#if defined __cpp_lib_memory_resource
#include <memory_resource>

/** Memory for objects that all end together, such as the temporary objects of a document import.

    Allocations take the next free bytes of pages taken from the system for this memory alone, and
    nothing is freed one at a time. Destroying the ThrowawayMemory returns all of its pages to the
    system at once, so these objects leave no gaps between longer-lived data in the malloc heap.
    Every object allocated from it must be destroyed before it is. In debug builds the pages are
    made inaccessible instead of returned, so a use after that faults at once.

    Not thread-safe: use one per thread. */
class VCL_DLLPUBLIC ThrowawayMemory final : public std::pmr::memory_resource
{
public:
    ThrowawayMemory();
    virtual ~ThrowawayMemory() override;

    ThrowawayMemory(const ThrowawayMemory&) = delete;
    ThrowawayMemory& operator=(const ThrowawayMemory&) = delete;

private:
    virtual void* do_allocate(std::size_t nBytes, std::size_t nAlignment) override;
    virtual void do_deallocate(void* p, std::size_t nBytes, std::size_t nAlignment) override;
    virtual bool do_is_equal(const std::pmr::memory_resource& rOther) const noexcept override;

    std::unique_ptr<std::pmr::memory_resource> mpPages;
    std::pmr::monotonic_buffer_resource maBuffer;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
