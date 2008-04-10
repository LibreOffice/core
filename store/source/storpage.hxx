/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: storpage.hxx,v $
 * $Revision: 1.6 $
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
#define _STORE_STORPAGE_HXX_ "$Revision: 1.6 $"

#include <sal/types.h>
#include <store/object.hxx>
#include <store/lockbyte.hxx>
#include <storbase.hxx>

namespace store
{

struct OStoreBTreeEntry;
struct OStoreBTreeNodeData;
class  OStoreBTreeNodeObject;

struct OStoreDataPageData;
struct OStoreIndirectionPageData;
struct OStoreDirectoryPageData;
class  OStoreDirectoryPageObject;
class  OStorePageCache;

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
    storeError initializeManager (
        ILockBytes      *pLockBytes,
        storeAccessMode  eAccessMode,
        sal_uInt16       nPageSize);

    /** isValid.
     *  @return sal_True  upon successful initialization,
     *          sal_False otherwise.
     */
    inline sal_Bool isValid (void) const;

    /** Page I/O (unmanaged).
     */
    virtual storeError free (
        OStorePageObject &rPage);

    virtual storeError load (
        OStorePageObject &rPage);

    virtual storeError save (
        OStorePageObject &rPage);

    virtual storeError flush (void);

    /** DirectoryPage I/O (managed).
     */
    storeError load (
        const OStorePageKey       &rKey,
        OStoreDirectoryPageObject &rPage);

    storeError save (
        const OStorePageKey       &rKey,
        OStoreDirectoryPageObject &rPage);

    /** attrib [nAttrib = ((nAttrib & ~nMask1) | nMask2)].
     */
    storeError attrib (
        const OStorePageKey &rKey,
        sal_uInt32           nMask1,
        sal_uInt32           nMask2,
        sal_uInt32          &rAttrib);

    /** link (insert Source Key as hardlink to Destination).
     */
    storeError link (
        const OStorePageKey &rSrcKey,
        const OStorePageKey &rDstKey);

    /** symlink (insert Source DirectoryPage as symlink to Destination).
     */
    storeError symlink (
        const rtl_String    *pSrcPath,
        const rtl_String    *pSrcName,
        const OStorePageKey &rDstKey);

    /** rename.
     */
    storeError rename (
        const OStorePageKey &rSrcKey,
        const rtl_String    *pDstPath,
        const rtl_String    *pDstName);

    /** remove.
     */
    storeError remove (
        const OStorePageKey &rKey);

    /** iterate.
     */
    storeError iterate (
        OStorePageKey    &rKey,
        OStorePageObject &rPage,
        sal_uInt32       &rAttrib);

    /** rebuild (combines recover and compact from 'Src' to 'Dst').
     *  @param  pSrcLB [in] accessed readonly.
     *  @param  pDstLB [in] truncated and accessed readwrite (as initialize()).
     *  @return store_E_None upon success.
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
    typedef OStoreIndirectionPageData indirect;
    typedef OStoreDataPageData        data;

    /** IStoreHandle TypeId.
     */
    static const sal_uInt32 m_nTypeId;

    /** IStoreHandle query() template function specialization.
     */
    friend OStorePageManager*
    SAL_CALL query<> (IStoreHandle *pHandle, OStorePageManager*);

    /** Representation.
    */
    OStorePageCache    *m_pCache;
    page               *m_pNode[3];
    inode              *m_pDirect;
    indirect           *m_pLink[3];
    data               *m_pData;
    sal_uInt16         m_nPageSize;

    /** find (node page, w/o split).
    */
    storeError find (
        const entry& rEntry, page& rPage);

    /** find (node page, possibly with split).
    */
    storeError find (
        const entry &rEntry, page &rPage, page &rPageL, page &rPageR);

    /** remove (possibly down from root).
    */
    storeError remove (
        entry &rEntry, page &rPage, page &rPageL);

    /** Not implemented.
    */
    OStorePageManager (const OStorePageManager&);
    OStorePageManager& operator= (const OStorePageManager&);
};

inline sal_Bool OStorePageManager::isValid (void) const
{
    return (base::isValid() && (m_nPageSize > 0));
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

