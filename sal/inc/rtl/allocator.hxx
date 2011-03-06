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
#if !defined INCLUDED_RTL_ALLOCATOR_HXX
#define INCLUDED_RTL_ALLOCATOR_HXX

#if ! defined(_SAL_TYPES_H_)
#include "sal/types.h"
#endif
#if ! defined(_RTL_ALLOC_H_)
#include "rtl/alloc.h"
#endif

#include <cstddef>

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

/** @internal */
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
    Allocator (const Allocator<U>&) SAL_THROW(())
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
    pointer allocate (size_type n, const void* hint = 0)
    {
        hint = hint; /* avoid warnings */
        return reinterpret_cast<pointer>(
            rtl_allocateMemory(sal_uInt32(n * sizeof(T))));
    }

    //-----------------------------------------
    void deallocate (pointer p, size_type /* n */)
    {
        rtl_freeMemory(p);
    }

    //-----------------------------------------
    void construct (pointer p, const T& value)
    {
        new ((void*)p)T(value);
    }

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

/** @internal */
template<class T, class U>
inline bool operator== (const Allocator<T>&, const Allocator<U>&) SAL_THROW(())
{
    return true;
}

/** @internal */
template<class T, class U>
inline bool operator!= (const Allocator<T>&, const Allocator<U>&) SAL_THROW(())
{
    return false;
}

} /* namespace rtl */

//######################################################
/** REQUIRED BY STLPort (see stlport '_alloc.h'):
    Hack for compilers that do not support member
    template classes (e.g. MSVC 6)

    @internal
*/
namespace _STL
{
    /** @internal */
    template<class T, class U>
    inline ::rtl::Allocator<U> & __stl_alloc_rebind (::rtl::Allocator<T> & a, U const *)
    {
        return (::rtl::Allocator<U>&)(a);
    }
}

#endif /* INCLUDED_RTL_ALLOCATOR_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
