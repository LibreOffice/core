/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filelckb.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:38:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _STORE_FILELCKB_HXX_
#define _STORE_FILELCKB_HXX_ "$Revision: 1.4 $"

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_USTRING_H_
#include <rtl/ustring.h>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _STORE_OBJECT_HXX_
#include <store/object.hxx>
#endif
#ifndef _STORE_LOCKBYTE_HXX_
#include <store/lockbyte.hxx>
#endif

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

