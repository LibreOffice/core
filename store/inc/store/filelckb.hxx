/*************************************************************************
 *
 *  $RCSfile: filelckb.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:18:31 $
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

#ifndef _STORE_FILELCKB_HXX_
#define _STORE_FILELCKB_HXX_ "$Revision: 1.1.1.1 $"

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_USTRING_H_
#include <rtl/ustring.h>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _STORE_MACROS_HXX_
#include <store/macros.hxx>
#endif
#ifndef _STORE_OBJECT_HXX_
#include <store/object.hxx>
#endif
#ifndef _STORE_LOCKBYTE_HXX_
#include <store/lockbyte.hxx>
#endif

#ifdef _USE_NAMESPACE
namespace store {
#endif

class OFileLockBytes_Impl;

/*========================================================================
 *
 * OFileLockBytes interface.
 *
 *======================================================================*/
class OFileLockBytes :
    public NAMESPACE_STORE(ILockBytes),
    public NAMESPACE_STORE(OStoreObject)
{
    VOS_DECLARE_CLASSINFO (VOS_NAMESPACE (OFileLockBytes, store));

public:
    OFileLockBytes (void);

    /** create.
     *  @param  pFilename [in]
     *  @param  eAccessMode [in]
     *  @return store_E_None upon success
     */
    storeError create (
        rtl_uString     *pFilename,
        storeAccessMode  eAccessMode);

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
     *  @param  rnSize [out]
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

    /** Delegate multiple inherited IReference.
     */
    virtual RefCount SAL_CALL acquire (void);
    virtual RefCount SAL_CALL release (void);
    virtual RefCount SAL_CALL referenced (void) const;

protected:
    virtual ~OFileLockBytes (void);

private:
    /** Representation.
     */
    NAMESPACE_OSL(Mutex)  m_aMutex;
    OFileLockBytes_Impl  *m_pImpl;

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
#ifdef _USE_NAMESPACE
}
#endif

#endif /* !_STORE_FILELCKB_HXX_ */

