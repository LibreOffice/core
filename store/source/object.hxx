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

#ifndef _STORE_OBJECT_HXX_
#define _STORE_OBJECT_HXX_

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

#ifndef _RTL_REF_HXX_
#include "rtl/ref.hxx"
#endif

#ifndef _OSL_INTERLCK_H_
#include "osl/interlck.h"
#endif

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
    virtual sal_Bool SAL_CALL isKindOf (sal_uInt32 nTypeId) = 0;
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
    static void  operator delete (void *p, size_t);

    /** IStoreHandle.
     */
    virtual sal_Bool SAL_CALL isKindOf (sal_uInt32 nTypeId);

    /** IReference.
     */
    virtual oslInterlockedCount SAL_CALL acquire (void);
    virtual oslInterlockedCount SAL_CALL release (void);

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
