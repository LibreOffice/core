/*************************************************************************
 *
 *  $RCSfile: storpage.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:18:32 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _STORE_STORPAGE_HXX_
#define _STORE_STORPAGE_HXX_ "$Revision: 1.1.1.1 $"

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _VOS_MACROS_HXX_
#include <vos/macros.hxx>
#endif
#ifndef _VOS_OBJECT_HXX_
#include <vos/object.hxx>
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif

#ifndef _STORE_TYPES_H_
#include <store/types.h>
#endif
#ifndef _STORE_MACROS_HXX_
#include <store/macros.hxx>
#endif
#ifndef _STORE_LOCKBYTE_HXX_
#include <store/lockbyte.hxx>
#endif

#ifndef _STORE_STORBASE_HXX_
#include <storbase.hxx>
#endif
#ifndef _STORE_STORDMON_HXX_
#include <stordmon.hxx>
#endif

#ifdef _USE_NAMESPACE
namespace store {
#endif

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
class OStorePageManager : public NAMESPACE_STORE(OStorePageBIOS)
{
    VOS_DECLARE_CLASSINFO (VOS_NAMESPACE (OStorePageManager, store));

public:
    /** Construction.
     */
    OStorePageManager (void);

    /** Initialization (two-phase construction).
     */
    virtual storeError initialize (
        ILockBytes      *pLockBytes,
        storeAccessMode  eAccessMode,
        sal_uInt16       nPageSize = STORE_DEFAULT_PAGESIZE);

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

protected:
    /** Destruction (OReference).
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

    /** Representation.
    */
    typedef NAMESPACE_VOS(ORef)<OStorePageDaemon> OStorePageDaemonRef;
    OStorePageDaemonRef m_xDaemon;
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

/*========================================================================
 *
 * The End.
 *
 *======================================================================*/
#ifdef _USE_NAMESPACE
}
#endif

#endif /* !_STORE_STORPAGE_HXX_ */

