/*************************************************************************
 *
 *  $RCSfile: operators_new_delete.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 17:40:45 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif

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
        void * p = rtl_allocateMemory (sal_uInt32(n));
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
