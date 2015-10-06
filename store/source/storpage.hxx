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

#ifndef INCLUDED_STORE_SOURCE_STORPAGE_HXX
#define INCLUDED_STORE_SOURCE_STORPAGE_HXX

#include "sal/types.h"

#include "object.hxx"
#include "lockbyte.hxx"

#include "storbase.hxx"
#include "storbios.hxx"
#include "stortree.hxx"

namespace store
{

struct OStoreDirectoryPageData;
class  OStoreDirectoryPageObject;

/*========================================================================
 *
 * OStorePageManager interface.
 *
 *======================================================================*/
class OStorePageManager : public store::OStorePageBIOS
{
public:
    /** Construction.
     */
    OStorePageManager();

    /** Initialization (two-phase construction).
     */
    virtual storeError initialize (
        ILockBytes *    pLockBytes,
        storeAccessMode eAccessMode,
        sal_uInt16 &    rnPageSize) SAL_OVERRIDE;

    /** isValid.
     *  @return sal_True  upon successful initialization,
     *          sal_False otherwise.
     */
    inline bool isValid() const;

    /** DirectoryPage I/O (managed).
     */
    static storeError namei (
        const rtl_String *pPath,
        const rtl_String *pName,
        OStorePageKey    &rKey);

    storeError iget (
        OStoreDirectoryPageObject & rPage, // [out]
        sal_uInt32                  nAttrib,
        const rtl_String *          pPath,
        const rtl_String *          pName,
        storeAccessMode             eMode);

    storeError iterate (
        OStorePageKey &  rKey,
        OStorePageLink & rLink,
        sal_uInt32 &     rAttrib);

    /** attrib [nAttrib = ((nAttrib & ~nMask1) | nMask2)].
     *  @see store_attrib()
     */
    storeError attrib (
        const OStorePageKey &rKey,
        sal_uInt32           nMask1,
        sal_uInt32           nMask2,
        sal_uInt32          &rAttrib);

    /** link (insert Source Key as hardlink to Destination).
     *  @see store_link()
     */
    storeError link (
        const OStorePageKey &rSrcKey,
        const OStorePageKey &rDstKey);

    /** symlink (insert Source DirectoryPage as symlink to Destination).
     *  @see store_symlink()
     */
    storeError symlink (
        const rtl_String    *pSrcPath,
        const rtl_String    *pSrcName,
        const OStorePageKey &rDstKey);

    /** rename.
     *  @see store_rename()
     */
    storeError rename (
        const OStorePageKey &rSrcKey,
        const rtl_String    *pDstPath,
        const rtl_String    *pDstName);

    /** remove.
     *  @see store_remove()
     */
    storeError remove (
        const OStorePageKey &rKey);

    /** rebuild (combines recover and compact from 'Src' to 'Dst').
     *  @param  pSrcLB [in] accessed readonly.
     *  @param  pDstLB [in] truncated and accessed readwrite (as initialize()).
     *  @return store_E_None upon success.
     *
     *  @see store_rebuildFile()
     */
    storeError rebuild (
        ILockBytes *pSrcLB,
        ILockBytes *pDstLB);

    /** IStoreHandle.
     */
    virtual bool isKindOf (sal_uInt32 nTypeId) SAL_OVERRIDE;

protected:
    /** Destruction.
    */
    virtual ~OStorePageManager();

private:
    /** Implementation.
    */
    typedef OStorePageBIOS            base;
    typedef OStorePageManager         self;

    typedef OStoreBTreeEntry          entry;
    typedef OStoreBTreeNodeData       page;
    typedef OStoreBTreeNodeObject     node;

    typedef OStoreDirectoryPageData   inode;
    typedef PageHolderObject< inode > inode_holder_type;

    /** IStoreHandle TypeId.
     */
    static const sal_uInt32 m_nTypeId;

    /** IStoreHandle query() template function specialization.
     */
    friend OStorePageManager*
    SAL_CALL query<> (OStoreObject *pHandle, OStorePageManager*);

    /** Representation.
    */
    OStoreBTreeRootObject m_aRoot;

    /** DirectoryPage I/O (managed).
     */
    storeError load_dirpage_Impl ( // @@@ => private: iget() @@@
        const OStorePageKey       &rKey,
        OStoreDirectoryPageObject &rPage);

    storeError save_dirpage_Impl ( // @@@ => private: iget(), rebuild() @@@
        const OStorePageKey       &rKey,
        OStoreDirectoryPageObject &rPage);

    /** find_lookup (node page and index, w/o split).
    */
    storeError find_lookup (
        OStoreBTreeNodeObject & rNode,
        sal_uInt16 &            rIndex,
        OStorePageKey const &   rKey);

    /** remove (possibly down from root).
    */
    storeError remove_Impl (entry & rEntry);

    OStorePageManager (const OStorePageManager&) SAL_DELETED_FUNCTION;
    OStorePageManager& operator= (const OStorePageManager&) SAL_DELETED_FUNCTION;
};

inline bool OStorePageManager::isValid() const
{
    return (base::isValid() /* @@@ NYI && (m_aRoot.is()) */);
}

template<> inline OStorePageManager*
SAL_CALL query (OStoreObject *pHandle, SAL_UNUSED_PARAMETER OStorePageManager*)
{
    if (pHandle && pHandle->isKindOf (OStorePageManager::m_nTypeId))
    {
        // Handle is kind of OStorePageManager.
        return static_cast<OStorePageManager*>(pHandle);
    }
    return 0;
}

/*========================================================================
 *
 * The End.
 *
 *======================================================================*/

} // namespace store

#endif // INCLUDED_STORE_SOURCE_STORPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
