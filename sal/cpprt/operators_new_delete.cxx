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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"

#ifdef WNT /* avoid 'std::bad_alloc' unresolved externals */
#define _CRTIMP
#define _NTSDK
#endif /* WNT */

#ifndef INCLUDED_ALGORITHM
#include <algorithm>
#define INCLUDED_ALGORITHM
#endif

#ifndef INCLUDED_NEW
#include <new>
#define INCLUDED_NEW
#endif

#ifndef INCLUDED_STRING_H
#include <string.h>
#define INCLUDED_STRING_H
#endif
#include <osl/diagnose.h>
#include <rtl/alloc.h>

using std::nothrow_t;

// =======================================================================
// AllocatorTraits
// =======================================================================

namespace
{

struct AllocatorTraits
{
    typedef char const signature_type[8];
    const signature_type & m_signature;

    explicit AllocatorTraits (signature_type const & s) SAL_THROW(())
        : m_signature (s)
    {}

    std::size_t size (std::size_t n) const SAL_THROW(())
    {
        n = std::max(n, std::size_t(1));
#if OSL_DEBUG_LEVEL > 0
# ifdef NEED_ALIGN16
        n += 2*sizeof(signature_type);
# else
        n += sizeof(signature_type);
# endif
#endif  /* OSL_DEBUG_LEVEL  */
        return n;
    }

    void* init (void * p) const SAL_THROW(())
    {
#if OSL_DEBUG_LEVEL > 0
        memcpy (p, m_signature, sizeof(signature_type));
# ifdef NEED_ALIGN16
        p = static_cast<char*>(p) + 2*sizeof(signature_type);
# else
        p = static_cast<char*>(p) + sizeof(signature_type);
# endif
#endif  /* OSL_DEBUG_LEVEL */
        return p;
    }

    void* fini (void * p) const SAL_THROW(())
    {
#if OSL_DEBUG_LEVEL > 0
# ifdef NEED_ALIGN16
        p = static_cast<char*>(p) - 2*sizeof(signature_type);
# else
        p = static_cast<char*>(p) - sizeof(signature_type);
# endif
        if (memcmp (p, m_signature, sizeof(signature_type)) != 0)
        {
            OSL_ENSURE(0, "operator delete mismatch");
        }
#endif  /* OSL_DEBUG_LEVEL */
        return p;
    }
};

// =======================================================================

struct VectorTraits : public AllocatorTraits
{
    static const signature_type g_signature;

    VectorTraits() SAL_THROW(())
        : AllocatorTraits (g_signature)
    {}
};

struct ScalarTraits : public AllocatorTraits
{
    static const signature_type g_signature;

    ScalarTraits() SAL_THROW(())
        : AllocatorTraits (g_signature)
    {}
};

const AllocatorTraits::signature_type VectorTraits::g_signature = "new[]()";
const AllocatorTraits::signature_type ScalarTraits::g_signature = "new()  ";

} // anonymous namespace

// =======================================================================
// Allocator
// =======================================================================

static void default_handler (void)
{
    // Multithreading race in 'std::set_new_handler()' call sequence below.
    throw std::bad_alloc();
}

// =======================================================================

static void* allocate (
    std::size_t n, AllocatorTraits const & rTraits)
    SAL_THROW((std::bad_alloc))
{
    n = rTraits.size (n);
    for (;;)
    {
        void * p = rtl_allocateMemory (sal_Size(n));
        if (p != 0)
            return rTraits.init (p);

        std::new_handler d = default_handler, f = std::set_new_handler (d);
        if (f != d)
            std::set_new_handler (f);

        if (f == 0)
            throw std::bad_alloc();
        (*f)();
    }
}

// =======================================================================

static void* allocate (
    std::size_t n, AllocatorTraits const & rTraits, std::nothrow_t const &)
    SAL_THROW(())
{
    try
    {
        return allocate (n, rTraits);
    }
    catch (std::bad_alloc const &)
    {
        return (0);
    }
}

// =======================================================================

static void deallocate (void * p, AllocatorTraits const & rTraits)
    SAL_THROW(())
{
    if (p)
    {
        rtl_freeMemory (rTraits.fini(p));
    }
}

// =======================================================================
// T * p = new T; delete p;
// =======================================================================

void* SAL_CALL operator new (std::size_t n) throw (std::bad_alloc)
{
    return allocate (n, ScalarTraits());
}

// =======================================================================

void SAL_CALL operator delete (void * p) throw ()
{
    deallocate (p, ScalarTraits());
}

// =======================================================================
// T * p = new(nothrow) T; delete(nothrow) p;
// =======================================================================

void* SAL_CALL operator new (std::size_t n, std::nothrow_t const &) throw ()
{
    return allocate (n, ScalarTraits(), nothrow_t());
}

// =======================================================================

void SAL_CALL operator delete (void * p, std::nothrow_t const &) throw ()
{
    deallocate (p, ScalarTraits());
}

// =======================================================================
// T * p = new T[n]; delete[] p;
// =======================================================================

void* SAL_CALL operator new[] (std::size_t n) throw (std::bad_alloc)
{
    return allocate (n, VectorTraits());
}

// =======================================================================

void SAL_CALL operator delete[] (void * p) throw ()
{
    deallocate (p, VectorTraits());
}

// =======================================================================
// T * p = new(nothrow) T[n]; delete(nothrow)[] p;
// =======================================================================

void* SAL_CALL operator new[] (std::size_t n, std::nothrow_t const &) throw ()
{
    return allocate (n, VectorTraits(), nothrow_t());
}

// =======================================================================

void SAL_CALL operator delete[] (void * p, std::nothrow_t const &) throw ()
{
    deallocate (p, VectorTraits());
}

// =======================================================================
