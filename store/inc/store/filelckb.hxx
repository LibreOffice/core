/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: filelckb.hxx,v $
 * $Revision: 1.5 $
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

#ifndef _STORE_FILELCKB_HXX_
#define _STORE_FILELCKB_HXX_ "$Revision: 1.5 $"

#include <sal/types.h>
#include <rtl/ustring.h>
#include <osl/mutex.hxx>
#include <store/object.hxx>
#include <store/lockbyte.hxx>

namespace store
{

class OFileLockBytes_Impl;

/*========================================================================
 *
 * OFileLockBytes interface.
 *
 *======================================================================*/
class OFileLockBytes :
    public store::OStoreObject,
    public store::ILockBytes
{
public:
    /** Construction.
     */
    OFileLockBytes (void);

    /** create.
        @param  pFilename [in]
        @param  eAccessMode [in]
        @return store_E_None upon success
     */
    storeError create (
        rtl_uString     *pFilename,
        storeAccessMode  eAccessMode);

    /** Read at Offset into Buffer.
        @param  nOffset [in]
        @param  pBuffer [out]
        @param  nBytes [in]
        @param  rnDone [out]
        @return store_E_None upon success
     */
    virtual storeError readAt (
        sal_uInt32  nOffset,
        void       *pBuffer,
        sal_uInt32  nBytes,
        sal_uInt32 &rnDone);

    /** Write at Offset from Buffer.
        @param  nOffset [in]
        @param  pBuffer [in]
        @param  nBytes [in]
        @param  rnDone [out]
        @return store_E_None upon success
     */
    virtual storeError writeAt (
        sal_uInt32  nOffset,
        const void *pBuffer,
        sal_uInt32  nBytes,
        sal_uInt32 &rnDone);

    /** flush.
        @return store_E_None upon success
     */
    virtual storeError flush (void);

    /** setSize.
        @param  nSize [in]
        @return store_E_None upon success
     */
    virtual storeError setSize (sal_uInt32 nSize);

    /** stat.
        @param  rnSize [out]
        @return store_E_None upon success
     */
    virtual storeError stat (sal_uInt32 &rnSize);

    /** Lock range at Offset.
        @param  nOffset [in]
        @param  nBytes [in]
        @return store_E_None upon success
                store_E_LockingViolation
     */
    virtual storeError lockRange (
        sal_uInt32 nOffset,
        sal_uInt32 nBytes);

    /** Unlock range at Offset.
        @param  nOffset [in]
        @param  nBytes [in]
        @return store_E_None upon success
                store_E_LockingViolation
     */
    virtual storeError unlockRange (
        sal_uInt32 nOffset,
        sal_uInt32 nBytes);

    /** Delegate multiple inherited IReference.
     */
    virtual oslInterlockedCount SAL_CALL acquire (void);
    virtual oslInterlockedCount SAL_CALL release (void);

protected:
    /** Destruction.
     */
    virtual ~OFileLockBytes (void);

private:
    /** Representation.
     */
    osl::Mutex           m_aMutex;
    OFileLockBytes_Impl *m_pImpl;

    /** Not implemented.
     */
    OFileLockBytes (const OFileLockBytes&);
    OFileLockBytes& operator= (const OFileLockBytes&);
};

/*========================================================================
 *
 * The End.
 *
 *======================================================================*/

} // namespace store

#endif /* !_STORE_FILELCKB_HXX_ */

