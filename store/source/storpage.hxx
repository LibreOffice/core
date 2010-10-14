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

#ifndef _STORE_STORPAGE_HXX_
#define _STORE_STORPAGE_HXX_ "$Revision: 1.6.8.2 $"

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
    OStorePageManager (void);

    /** Initialization (two-phase construction).
     */
    virtual storeError initialize (
        ILockBytes *    pLockBytes,
        storeAccessMode eAccessMode,
        sal_uInt16 &    rnPageSize);

    /** isValid.
     *  @return sal_True  upon successful initialization,
     *          sal_False otherwise.
     */
    inline sal_Bool isValid (void) const;

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
    virtual sal_Bool SAL_CALL isKindOf (sal_uInt32 nTypeId);

protected:
    /** Destruction.
    */
    virtual ~OStorePageManager (void);

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
    SAL_CALL query<> (IStoreHandle *pHandle, OStorePageManager*);

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

    /** Not implemented.
    */
    OStorePageManager (const OStorePageManager&);
    OStorePageManager& operator= (const OStorePageManager&);
};

inline sal_Bool OStorePageManager::isValid (void) const
{
    return (base::isValid() /* @@@ NYI && (m_aRoot.is()) */);
}

template<> inline OStorePageManager*
SAL_CALL query (IStoreHandle *pHandle, OStorePageManager*)
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

#endif /* !_STORE_STORPAGE_HXX_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
