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
#include "precompiled_store.hxx"

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
void OStoreObject::operator delete (void *p, size_t)
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
    oslInterlockedCount result = osl_incrementInterlockedCount (&m_nRefCount);
    return (result);
}

/*
 * release.
 */
oslInterlockedCount SAL_CALL OStoreObject::release (void)
{
    oslInterlockedCount result = osl_decrementInterlockedCount (&m_nRefCount);
    if (result == 0)
    {
        // Last reference released.
        delete this;
    }
    return (result);
}

} // namespace store

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
