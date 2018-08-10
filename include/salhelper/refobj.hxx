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

#include <cassert>

#include "sal/types.h"
#include "rtl/alloc.h"
#include "osl/interlck.h"

namespace salhelper
{

/** A base implementation for reference-counted objects.

    @deprecated use salhelper::SimpleReferenceObject instead
*/
class ReferenceObject
{
    /** Representation.
     */
    oslInterlockedCount m_nReferenceCount;

    ReferenceObject (const ReferenceObject&) SAL_DELETED_FUNCTION;
    ReferenceObject& operator= (const ReferenceObject&) SAL_DELETED_FUNCTION;

public:
    /** Allocation.
     */
    static void* operator new (size_t n)
    {
        return ::rtl_allocateMemory (n);
    }
    static void operator delete (void* p)
    {
        ::rtl_freeMemory (p);
    }
    static void* operator new (size_t, void* p)
    {
        return p;
    }
    static void operator delete (void*, void*)
    {}

public:
    /** Construction.
     */
    ReferenceObject() : m_nReferenceCount(0)
    {}


    void SAL_CALL acquire()
    {
        osl_atomic_increment(&m_nReferenceCount);
    }

    void SAL_CALL release()
    {
        if (osl_atomic_decrement(&m_nReferenceCount) == 0)
        {
            // Last reference released.
            delete this;
        }
    }

protected:
    /** Destruction.
     */
    virtual ~ReferenceObject()
    {
        assert(m_nReferenceCount == 0);
    }
};


} // namespace salhelper

#endif /* ! INCLUDED_SALHELPER_REFOBJ_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
