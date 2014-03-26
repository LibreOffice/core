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

#ifndef INCLUDED_SALHELPER_REFOBJ_HXX
#define INCLUDED_SALHELPER_REFOBJ_HXX

#include <sal/types.h>
#include <rtl/alloc.h>
#include <rtl/ref.hxx>
#include <osl/diagnose.h>
#include <osl/interlck.h>

namespace salhelper
{



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
    virtual oslInterlockedCount SAL_CALL acquire() SAL_THROW(()) SAL_OVERRIDE
    {
        return osl_atomic_increment (&m_nReferenceCount);
    }

    virtual oslInterlockedCount SAL_CALL release() SAL_THROW(()) SAL_OVERRIDE
    {
        oslInterlockedCount result;
        result = ::osl_atomic_decrement (&m_nReferenceCount);
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



} // namespace salhelper

#endif /* ! INCLUDED_SALHELPER_REFOBJ_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
