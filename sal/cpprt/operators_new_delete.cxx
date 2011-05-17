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
        n += sizeof(signature_type);
#endif  /* OSL_DEBUG_LEVEL  */
        return n;
    }

    void* init (void * p) const SAL_THROW(())
    {
#if OSL_DEBUG_LEVEL > 0
        memcpy (p, m_signature, sizeof(signature_type));
        p = static_cast<char*>(p) + sizeof(signature_type);
#endif  /* OSL_DEBUG_LEVEL */
        return p;
    }

    void* fini (void * p) const SAL_THROW(())
    {
#if OSL_DEBUG_LEVEL > 0
        p = static_cast<char*>(p) - sizeof(signature_type);
        if (memcmp (p, m_signature, sizeof(signature_type)) != 0)
        {
            OSL_FAIL("operator delete mismatch");
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
