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

#ifndef _STORE_STORLCKB_HXX_
#define _STORE_STORLCKB_HXX_ "$Revision: 1.6.8.1 $"

#include "sal/types.h"

#include "rtl/ustring.h"
#include "rtl/ref.hxx"

#include "object.hxx"
#include "storbase.hxx"
#include "storpage.hxx"

namespace store
{

struct OStoreDataPageData;
struct OStoreDirectoryPageData;

/*========================================================================
 *
 * OStoreLockBytes interface.
 *
 *======================================================================*/
class OStoreLockBytes : public store::OStoreObject
{
public:
    /** Construction.
     */
    OStoreLockBytes (void);

    /** create (two-phase construction).
     *  @param  pManager [in]
     *  @param  pPath [in]
     *  @param  pName [in]
     *  @param  eMode [in]
     *  @return store_E_None upon success
     */
    storeError create (
        OStorePageManager *pManager,
        rtl_String        *pPath,
        rtl_String        *pName,
        storeAccessMode    eAccessMode);

    /** Read at Offset into Buffer.
     *  @param  nOffset [in]
     *  @param  pBuffer [out]
     *  @param  nBytes [in]
     *  @param  rnDone [out]
     *  @return store_E_None upon success
     */
    storeError readAt (
        sal_uInt32  nOffset,
        void       *pBuffer,
        sal_uInt32  nBytes,
        sal_uInt32 &rnDone);

    /** Write at Offset from Buffer.
     *  @param  nOffset [in]
     *  @param  pBuffer [in]
     *  @param  nBytes [in]
     *  @param  rnDone [out]
     *  @return store_E_None upon success
     */
    storeError writeAt (
        sal_uInt32  nOffset,
        const void *pBuffer,
        sal_uInt32  nBytes,
        sal_uInt32 &rnDone);

    /** flush.
     *  @return store_E_None upon success
     */
    storeError flush (void);

    /** setSize.
     *  @param  nSize [in]
     *  @return store_E_None upon success
     */
    storeError setSize (sal_uInt32 nSize);

    /** stat.
     *  @paran  rnSize [out]
     *  @return store_E_None upon success
     */
    storeError stat (sal_uInt32 &rnSize);

    /** IStoreHandle.
     */
    virtual sal_Bool SAL_CALL isKindOf (sal_uInt32 nMagic);

protected:
    /** Destruction (OReference).
     */
    virtual ~OStoreLockBytes (void);

private:
    /** IStoreHandle TypeId.
     */
    static const sal_uInt32 m_nTypeId;

    /** IStoreHandle query() template specialization.
     */
    friend OStoreLockBytes*
    SAL_CALL query<> (IStoreHandle *pHandle, OStoreLockBytes*);

    /** Representation.
     */
    rtl::Reference<OStorePageManager> m_xManager;

    typedef OStoreDataPageData        data;
    typedef OStoreDirectoryPageData   inode;

    typedef PageHolderObject< inode > inode_holder_type;
    inode_holder_type                 m_xNode;

    bool m_bWriteable;

    /** Not implemented.
     */
    OStoreLockBytes (const OStoreLockBytes&);
    OStoreLockBytes& operator= (const OStoreLockBytes&);
};

template<> inline OStoreLockBytes*
SAL_CALL query (IStoreHandle *pHandle, OStoreLockBytes*)
{
    if (pHandle && pHandle->isKindOf (OStoreLockBytes::m_nTypeId))
    {
        // Handle is kind of OStoreLockBytes.
        return static_cast<OStoreLockBytes*>(pHandle);
    }
    return 0;
}

/*========================================================================
 *
 * The End.
 *
 *======================================================================*/

} // namespace store

#endif /* !_STORE_STORLCKB_HXX_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
