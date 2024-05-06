/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <limits>

#include <rtl/alloc.h>

namespace sc {

/**
 * Custom allocator for STL container to ensure that the base address of
 * allocated storage is aligned to a specified boundary.
 */
template<typename T, size_t Alignment>
class AlignedAllocator
{
public:
    typedef T value_type;
    typedef size_t size_type;
    typedef std::ptrdiff_t difference_type;

    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T* void_pointer;

    typedef T& reference;
    typedef const T& const_reference;

    template<typename Type2>
    struct rebind
    {
        typedef AlignedAllocator<Type2,Alignment> other;
    };

    AlignedAllocator() {}

    template<typename Type2>
    AlignedAllocator(const AlignedAllocator<Type2,Alignment>&) {}

    static void construct(T* p, const value_type& val) { new(p) value_type(val); }
    static void destroy(T* p)
    {
        p->~value_type();
    }

    static size_type max_size()
    {
        return std::numeric_limits<size_type>::max() / sizeof(value_type);
    }

    bool operator== (const AlignedAllocator&) const { return true; }
    bool operator!= (const AlignedAllocator&) const { return false; }

    static pointer allocate(size_type n)
    {
        return static_cast<pointer>(rtl_allocateAlignedMemory(Alignment, n*sizeof(value_type)));
    }

    static void deallocate(pointer p, size_type)
    {
        rtl_freeAlignedMemory(p);
    }
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
