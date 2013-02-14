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
#ifndef INCLUDED_RTL_ALLOCATOR_HXX
#define INCLUDED_RTL_ALLOCATOR_HXX

#include "sal/types.h"
#include "rtl/alloc.h"
#include <cstddef>

#if defined LIBO_INTERNAL_ONLY
#include "config_global.h"
#endif

/// @cond INTERNAL

//######################################################
// This is no general purpose STL allocator but one
// necessary to use STL for some implementation but
// avoid linking sal against the STLPort library!!!
// For more information on when and how to define a
// custom stl allocator have a look at Scott Meyers:
// "Effective STL", Nicolai M. Josuttis:
// "The C++ Standard Library - A Tutorial and Reference"
// and at http://www.josuttis.com/cppcode/allocator.html

namespace rtl {

template<class T>
class Allocator
{
public:
    typedef T value_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef ::std::size_t size_type;
    typedef ::std::ptrdiff_t difference_type;

    //-----------------------------------------
    template<class U>
    struct rebind
    {
        typedef Allocator<U> other;
    };

    //-----------------------------------------
    pointer address (reference value) const
    {
        return &value;
    }

    //-----------------------------------------
    const_pointer address (const_reference value) const
    {
        return &value;
    }

    //-----------------------------------------
    Allocator() SAL_THROW(())
    {}

    //-----------------------------------------
    template<class U>
    Allocator (SAL_UNUSED_PARAMETER const Allocator<U>&) SAL_THROW(())
    {}

    //-----------------------------------------
    Allocator(const Allocator&) SAL_THROW(())
    {}

    //-----------------------------------------
    ~Allocator() SAL_THROW(())
    {}

    //-----------------------------------------
    size_type max_size() const SAL_THROW(())
    {
        return size_type(-1)/sizeof(T);
    }

    //-----------------------------------------
    /* Normally the code for allocate should
       throw a std::bad_alloc exception if the
       requested memory could not be allocated:
       (C++ standard 20.4.1.1):

       pointer allocate (size_type n, const void* hint = 0)
       {
         pointer p = reinterpret_cast<pointer>(
             rtl_allocateMemory(sal_uInt32(n * sizeof(T))));

         if (NULL == p)
             throw ::std::bad_alloc();

         return p;
       }

       but some compilers do not compile it if exceptions
       are not enabled, e.g. GCC under Linux and it is
       in general not desired to compile sal with exceptions
       enabled. */
    pointer allocate (size_type n, SAL_UNUSED_PARAMETER const void* = 0)
    {
        return reinterpret_cast<pointer>(
            rtl_allocateMemory(sal_uInt32(n * sizeof(T))));
    }

    //-----------------------------------------
    void deallocate (pointer p, SAL_UNUSED_PARAMETER size_type /* n */)
    {
        rtl_freeMemory(p);
    }

    //-----------------------------------------
#if defined HAVE_CXX11_PERFECT_FORWARDING
    template< typename... Args >
    void construct (pointer p, Args &&... value)
    {
        new ((void*)p)T(std::forward< Args >(value)...);
    }
#else
    void construct (pointer p, const T& value)
    {
        new ((void*)p)T(value);
    }
#endif

    //-----------------------------------------
    void destroy (pointer p)
    {
        p->~T();
        (void)p; //MSVC2005 annoyingly warns this is unused
    }
};

//######################################################
// Custom STL allocators must be stateless (see
// references above) that's why the operators below
// return always true or false

template<class T, class U> inline bool operator ==(
    SAL_UNUSED_PARAMETER const Allocator<T>&,
    SAL_UNUSED_PARAMETER const Allocator<U>&) SAL_THROW(())
{
    return true;
}

template<class T, class U>
inline bool operator!= (const Allocator<T>&, const Allocator<U>&) SAL_THROW(())
{
    return false;
}

} /* namespace rtl */

/** REQUIRED BY STLPort (see stlport '_alloc.h'):
    Hack for compilers that do not support member
    template classes (e.g. MSVC 6)
*/
namespace _STL
{
    template<class T, class U>
    inline ::rtl::Allocator<U> & __stl_alloc_rebind (::rtl::Allocator<T> & a, U const *)
    {
        return (::rtl::Allocator<U>&)(a);
    }
}

/// @endcond

#endif /* INCLUDED_RTL_ALLOCATOR_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
