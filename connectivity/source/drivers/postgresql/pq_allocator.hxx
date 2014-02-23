/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  Effective License of whole file:
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 *  Parts "Copyright by Sun Microsystems, Inc" prior to August 2011:
 *
 *    The Contents of this file are made available subject to the terms of
 *    the GNU Lesser General Public License Version 2.1
 *
 *    Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All parts contributed on or after August 2011:
 *
 *    This Source Code Form is subject to the terms of the Mozilla Public
 *    License, v. 2.0. If a copy of the MPL was not distributed with this
 *    file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 ************************************************************************/

#ifndef _PQ_ALLOCATOR_
#define _PQ_ALLOCATOR_

#include <cstddef>
#include "sal/types.h"

/** jbu: This source has been copied from sal/inc/internal/allocator.hxx,
         because it is not a public interface. Thx a lot for figuring this
         out.
 */


// This is no general purpose STL allocator but one
// necessary to use STL for some implementation but
// avoid linking sal against the STLPort library!!!
// For more information on when and how to define a
// custom stl allocator have a look at Scott Meyers:
// "Effective STL", Nicolai M. Josuttis:
// "The C++ Standard Library - A Tutorial and Reference"
// and at http://www.josuttis.com/cppcode/allocator.html

namespace pq_sdbc_driver {

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


    template<class U>
    struct rebind
    {
        typedef Allocator<U> other;
    };


    pointer address (reference value) const
    {
        return &value;
    }


    const_pointer address (const_reference value) const
    {
        return &value;
    }


    Allocator() SAL_THROW(())
    {}


    template<class U>
    Allocator (const Allocator<U>&) SAL_THROW(())
    {}


    Allocator(const Allocator&) SAL_THROW(())
    {}


    ~Allocator() SAL_THROW(())
    {}


    size_type max_size() const SAL_THROW(())
    {
        return size_type(-1)/sizeof(T);
    }


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


    void deallocate (pointer p, SAL_UNUSED_PARAMETER size_type)
    {
        rtl_freeMemory(p);
    }


    void construct (pointer p, const T& value)
    {
        new ((void*)p)T(value);
    }

    // LEM: GNU libstdc++ vectors expect this one to exist,
    // at least if one intends to create vectors by giving
    // only a size and no initialising value.

    void construct (pointer p)
    {
        new ((void*)p)T;
    }


    void destroy (pointer p)
    {
        p->~T();
#ifdef _MSC_VER
        (void) p; // spurious warning C4100: 'p': unreferenced formal parameter
#endif
    }
};


// Custom STL allocators must be stateless (see
// references above) that's why the operators below
// return always true or false
template<class T, class U>
inline bool operator== (const Allocator<T>&, const Allocator<U>&) SAL_THROW(())
{
    return true;
}

template<class T, class U>
inline bool operator!= (const Allocator<T>&, const Allocator<U>&) SAL_THROW(())
{
    return false;
}

} /* namespace sal */

#endif /* _PQ_ALLOCATOR_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
