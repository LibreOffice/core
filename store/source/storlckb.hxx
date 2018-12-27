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

#ifndef INCLUDED_STORE_SOURCE_STORLCKB_HXX
#define INCLUDED_STORE_SOURCE_STORLCKB_HXX

#include <sal/types.h>

#include <rtl/string.h>
#include <rtl/ref.hxx>

#include "object.hxx"
#include "storbase.hxx"

namespace store
{

class OStorePageManager;
struct OStoreDataPageData;
struct OStoreDirectoryPageData;

class OStoreLockBytes : public store::OStoreObject
{
public:
    /** Construction.
     */
    OStoreLockBytes();

    /** create (two-phase construction).
     *  @param  pManager [in]
     *  @param  pPath [in]
     *  @param  pName [in]
     *  @param  eMode [in]
     *  @return store_E_None upon success
     */
    storeError create (
        OStorePageManager *pManager,
        rtl_String const  *pPath,
        rtl_String const  *pName,
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

    /** setSize.
     *  @param  nSize [in]
     *  @return store_E_None upon success
     */
    storeError setSize (sal_uInt32 nSize);

    /** IStoreHandle.
     */
    virtual bool isKindOf (sal_uInt32 nMagic) override;

protected:
    /** Destruction (OReference).
     */
    virtual ~OStoreLockBytes() override;

private:
    /** IStoreHandle TypeId.
     */
    static const sal_uInt32 m_nTypeId;

    /** IStoreHandle query() template specialization.
     */
    friend OStoreLockBytes*
    SAL_CALL query<> (OStoreObject *pHandle, OStoreLockBytes*);

    /** Representation.
     */
    rtl::Reference<OStorePageManager> m_xManager;

    typedef OStoreDataPageData        data;
    typedef OStoreDirectoryPageData   inode;

    typedef PageHolderObject< inode > inode_holder_type;
    inode_holder_type                 m_xNode;

    bool m_bWriteable;

    OStoreLockBytes (const OStoreLockBytes&) = delete;
    OStoreLockBytes& operator= (const OStoreLockBytes&) = delete;
};

template<> inline OStoreLockBytes*
SAL_CALL query (OStoreObject *pHandle, SAL_UNUSED_PARAMETER OStoreLockBytes*)
{
    if (pHandle && pHandle->isKindOf (OStoreLockBytes::m_nTypeId))
    {
        // Handle is kind of OStoreLockBytes.
        return static_cast<OStoreLockBytes*>(pHandle);
    }
    return nullptr;
}

} // namespace store

#endif // INCLUDED_STORE_SOURCE_STORLCKB_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
