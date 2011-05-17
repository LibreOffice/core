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

#ifndef _SALHELPER_REFOBJ_HXX_
#define _SALHELPER_REFOBJ_HXX_

#include <sal/types.h>
#include <rtl/alloc.h>
#include <rtl/ref.hxx>
#include <osl/diagnose.h>
#include <osl/interlck.h>

namespace salhelper
{

//----------------------------------------------------------------------------

class ReferenceObject : public rtl::IReference
{
    /** Representation.
     */
    oslInterlockedCount m_nReferenceCount;

    /** Not implemented.
     */
    ReferenceObject (const ReferenceObject&);
    ReferenceObject& operator= (const ReferenceObject&);

public:
    /** Allocation.
     */
    static void* operator new (size_t n) SAL_THROW(())
    {
        return ::rtl_allocateMemory (n);
    }
    static void operator delete (void* p) SAL_THROW(())
    {
        ::rtl_freeMemory (p);
    }
    static void* operator new (size_t, void* p) SAL_THROW(())
    {
        return (p);
    }
    static void operator delete (void*, void*) SAL_THROW(())
    {}

public:
    /** Construction.
     */
    inline ReferenceObject() SAL_THROW(()) : m_nReferenceCount (0)
    {}


    /** IReference.
     */
    virtual oslInterlockedCount SAL_CALL acquire() SAL_THROW(())
    {
        return ::osl_incrementInterlockedCount (&m_nReferenceCount);
    }

    virtual oslInterlockedCount SAL_CALL release() SAL_THROW(())
    {
        oslInterlockedCount result;
        result = ::osl_decrementInterlockedCount (&m_nReferenceCount);
        if (result == 0)
        {
            // Last reference released.
            delete this;
        }
        return (result);
    }

protected:
    /** Destruction.
     */
    virtual ~ReferenceObject() SAL_THROW(())
    {
        OSL_ASSERT(m_nReferenceCount == 0);
    }
};

//----------------------------------------------------------------------------

} // namespace salhelper

#endif /* !_SALHELPER_REFOBJ_HXX_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
