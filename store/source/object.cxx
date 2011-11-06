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
