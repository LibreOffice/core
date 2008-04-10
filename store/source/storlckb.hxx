/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: storlckb.hxx,v $
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

#ifndef _STORE_STORLCKB_HXX_
#define _STORE_STORLCKB_HXX_ "$Revision: 1.6 $"

#include <sal/types.h>

#ifndef _RTL_TEXTCVT_H_
#include <rtl/textcvt.h>
#endif
#include <rtl/ustring.h>
#include <rtl/ref.hxx>
#include <store/object.hxx>
#include <store/lockbyte.hxx>
#include <storbase.hxx>
#include <storpage.hxx>

namespace store
{

struct OStoreDataPageData;
struct OStoreDirectoryPageData;
struct OStoreIndirectionPageData;

/*========================================================================
 *
 * OStoreDirectory interface.
 *
 *======================================================================*/
class OStoreDirectory : public store::OStoreObject
{
public:
    /** Construction.
     */
    OStoreDirectory (void);

    /** create (two-phase construction).
     *  @param  pManager [in]
     *  @param  pszPath [in]
     *  @param  pszName [in]
     *  @param  eAccessMode [in]
     *  @return store_E_None upon success.
     */
    storeError create (
        OStorePageManager *pManager,
        rtl_uString       *pPath,
        rtl_uString       *pName,
        storeAccessMode    eAccessMode);

    /** iterate.
     *  @param  rFindData [out]
     *  @return store_E_None        upon success,
     *          store_E_NoMoreFiles upon end of iteration.
     */
    storeError iterate (
        storeFindData &rFindData);

    /** IStoreHandle.
     */
    virtual sal_Bool SAL_CALL isKindOf (sal_uInt32 nTypeId);

protected:
    /** Destruction.
     */
    virtual ~OStoreDirectory (void);

private:
    /** IStoreHandle TypeId.
     */
    static const sal_uInt32 m_nTypeId;

    /** IStoreHandle query() template function specialization.
     */
    friend OStoreDirectory*
    SAL_CALL query<> (IStoreHandle *pHandle, OStoreDirectory*);

    /** Representation.
     */
    rtl::Reference<OStorePageManager> m_xManager;

    typedef OStoreDirectoryPageData inode;
    inode                     *m_pNode;

    OStorePageDescriptor       m_aDescr;
    sal_uInt32                 m_nPath;
    rtl_TextToUnicodeConverter m_hTextCvt;

    /** Not implemented.
     */
    OStoreDirectory (const OStoreDirectory&);
    OStoreDirectory& operator= (const OStoreDirectory&);
};

template<> inline OStoreDirectory*
SAL_CALL query (IStoreHandle *pHandle, OStoreDirectory*)
{
    if (pHandle && pHandle->isKindOf (OStoreDirectory::m_nTypeId))
    {
        // Handle is kind of OStoreDirectory.
        return static_cast<OStoreDirectory*>(pHandle);
    }
    return 0;
}

/*========================================================================
 *
 * OStoreLockBytes interface.
 *
 *======================================================================*/
class OStoreLockBytes :
    public store::OStoreObject,
    public store::ILockBytes
{
public:
    /** Construction.
     */
    OStoreLockBytes (void);

    /** create (two-phase construction).
     *  @param  pManager [in]
     *  @param  rNode [in]
     *  @param  eMode [in]
     *  @return store_E_None upon success
     */
    storeError create (
        OStorePageManager *pManager,
        rtl_uString       *pPath,
        rtl_uString       *pName,
        storeAccessMode    eAccessMode);

    /** Read at Offset into Buffer.
     *  @param  nOffset [in]
     *  @param  pBuffer [out]
     *  @param  nBytes [in]
     *  @param  rnDone [out]
     *  @return store_E_None upon success
     */
    virtual storeError readAt (
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
    virtual storeError writeAt (
        sal_uInt32  nOffset,
        const void *pBuffer,
        sal_uInt32  nBytes,
        sal_uInt32 &rnDone);

    /** flush.
     *  @return store_E_None upon success
     */
    virtual storeError flush (void);

    /** setSize.
     *  @param  nSize [in]
     *  @return store_E_None upon success
     */
    virtual storeError setSize (sal_uInt32 nSize);

    /** stat.
     *  @paran  rnSize [out]
     *  @return store_E_None upon success
     */
    virtual storeError stat (sal_uInt32 &rnSize);

    /** Lock range at Offset.
     *  @param  nOffset [in]
     *  @param  nBytes [in]
     *  @return store_E_None upon success
     *          store_E_LockingViolation
     */
    virtual storeError lockRange (
        sal_uInt32 nOffset,
        sal_uInt32 nBytes);

    /** Unlock range at Offset.
     *  @param  nOffset [in]
     *  @param  nBytes [in]
     *  @return store_E_None upon success
     *          store_E_LockingViolation
     */
    virtual storeError unlockRange (
        sal_uInt32 nOffset,
        sal_uInt32 nBytes);

    /** IStoreHandle.
     */
    virtual sal_Bool SAL_CALL isKindOf (sal_uInt32 nMagic);

    /** Delegate multiple inherited IReference.
     */
    virtual oslInterlockedCount SAL_CALL acquire (void);
    virtual oslInterlockedCount SAL_CALL release (void);

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
    typedef OStoreIndirectionPageData indirect;

    inode     *m_pNode;
    data      *m_pData;

    indirect  *m_pSingle;
    indirect  *m_pDouble;
    indirect  *m_pTriple;

    sal_uInt16 m_nPageSize;
    sal_Bool   m_bWriteable;

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

