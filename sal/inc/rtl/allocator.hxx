/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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

#endif /* INCLUDED_RTL_ALLOCATOR_HXX */

