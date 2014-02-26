/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __SC_STLALGORITHM_HXX__
#define __SC_STLALGORITHM_HXX__

#include <functional>

/**
 * Function object to allow deleting instances stored in STL containers as
 * pointers.
 */
template<typename T>
struct ScDeleteObjectByPtr : public ::std::unary_function<T*, void>
{
    void operator() (T* p)
    {
        delete p;
    }
};

namespace sc {

/**
 * Custom allocator for STL container to ensure that the base address of
 * allocated storage is aligned to a specified boundary.
 */
template<typename T, std::size_t N>
class AlignedAllocator
{
public:
    typedef T value_type;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T* void_pointer;

    typedef T& reference;
    typedef const T& const_reference;

public:
    AlignedAllocator() throw() {}

    template<typename T2>
    AlignedAllocator(const AlignedAllocator<T2, N>&) throw() {}

    ~AlignedAllocator() throw() {}

    pointer adress(reference r)
    {
        return &r;
    }

    const_pointer adress(const_reference r) const
    {
        return &r;
    }

    pointer allocate(size_type n)
    {
#ifdef WNT
        return (pointer)_aligned_malloc(n * sizeof(value_type), N);
#else
        return (pointer)aligned_alloc(N, n * sizeof(value_type));
#endif
    }

    void deallocate(pointer p, size_type)
    {
#ifdef WNT
        _aligned_free(p);
#else
        free(p);
#endif
    }

    void construct(pointer p, const value_type& wert)
    {
        new(p) value_type(wert);
    }

    void destroy(pointer p)
    {
        p->~value_type();
    }

    size_type max_size() const throw()
    {
        return size_type(-1) / sizeof(value_type);
    }

    template<typename T2>
    struct rebind
    {
        typedef AlignedAllocator<T2, N> other;
    };

    bool operator==(const AlignedAllocator<T, N>& other) const
    {
        return true;
    }

    bool operator!=(const AlignedAllocator<T, N>& other) const
    {
        return !(*this == other);
    }
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
