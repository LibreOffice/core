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


#include "object.hxx"

#include "sal/types.h"
#include "rtl/alloc.h"
#include "rtl/ref.hxx"
#include "osl/diagnose.h"
#include "osl/interlck.h"

namespace store
{

/*========================================================================
 *
 * OStoreObject implementation.
 *
 *======================================================================*/
const sal_uInt32 OStoreObject::m_nTypeId = sal_uInt32(0x58190322);

/*
 * OStoreObject.
 */
OStoreObject::OStoreObject (void)
    : m_nRefCount (0)
{
}

/*
 * ~OStoreObject.
 */
OStoreObject::~OStoreObject (void)
{
    OSL_ASSERT(m_nRefCount == 0);
}

/*
 * operator new.
 */
void* OStoreObject::operator new (size_t n)
{
    return rtl_allocateMemory (n);
}

/*
 * operator delete.
 */
void OStoreObject::operator delete (void *p)
{
    rtl_freeMemory (p);
}

/*
 * isKindOf.
 */
sal_Bool SAL_CALL OStoreObject::isKindOf (sal_uInt32 nTypeId)
{
    return (nTypeId == m_nTypeId);
}

/*
 * acquire.
 */
oslInterlockedCount SAL_CALL OStoreObject::acquire (void)
{
    oslInterlockedCount result = osl_atomic_increment (&m_nRefCount);
    return (result);
}

/*
 * release.
 */
oslInterlockedCount SAL_CALL OStoreObject::release (void)
{
    oslInterlockedCount result = osl_atomic_decrement (&m_nRefCount);
    if (result == 0)
    {
        // Last reference released.
        delete this;
    }
    return (result);
}

} // namespace store

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
