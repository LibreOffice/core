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

#ifndef _STORE_OBJECT_HXX_
#define _STORE_OBJECT_HXX_

#include "sal/types.h"

#include "rtl/ref.hxx"

#include "osl/interlck.h"

namespace store
{

/*========================================================================
 *
 * IStoreHandle interface.
 *
 *======================================================================*/
class IStoreHandle : public rtl::IReference
{
public:
    /** Replaces dynamic_cast type checking.
     */
    virtual bool isKindOf (sal_uInt32 nTypeId) = 0;

protected:
    ~IStoreHandle() {}
};

/** Template helper function as dynamic_cast replacement.
 */
template<class store_handle_type>
store_handle_type * SAL_CALL query (
    IStoreHandle * pHandle, store_handle_type *);

/*========================================================================
 *
 * OStoreObject interface.
 *
 *======================================================================*/
class OStoreObject : public store::IStoreHandle
{
    /** Template function specialization as dynamic_cast replacement.
     */
    friend OStoreObject*
    SAL_CALL query<> (IStoreHandle *pHandle, OStoreObject*);

public:
    /** Construction.
     */
    OStoreObject (void);

    /** Allocation.
     */
    static void* operator new (size_t n);
    static void  operator delete (void *p);

    /** IStoreHandle.
     */
    virtual bool isKindOf (sal_uInt32 nTypeId) SAL_OVERRIDE;

    /** IReference.
     */
    virtual oslInterlockedCount SAL_CALL acquire (void) SAL_OVERRIDE;
    virtual oslInterlockedCount SAL_CALL release (void) SAL_OVERRIDE;

protected:
    /** Destruction.
     */
    virtual ~OStoreObject (void);

private:
    /** The IStoreHandle TypeId.
     */
    static const sal_uInt32 m_nTypeId;

    /** Representation.
     */
    oslInterlockedCount m_nRefCount;

    /** Not implemented.
     */
    OStoreObject (const OStoreObject&);
    OStoreObject& operator= (const OStoreObject&);
};

/** Template function specialization as dynamic_cast replacement.
 */
template<> inline OStoreObject*
SAL_CALL query (IStoreHandle *pHandle, OStoreObject*)
{
    if (pHandle && pHandle->isKindOf (OStoreObject::m_nTypeId))
    {
        // Handle is kind of OStoreObject.
        return static_cast<OStoreObject*>(pHandle);
    }
    return 0;
}

/*========================================================================
 *
 * The End.
 *
 *======================================================================*/

} // namespace store

#endif /* !_STORE_OBJECT_HXX_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
