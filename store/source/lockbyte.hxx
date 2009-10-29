/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lockbyte.hxx,v $
 *
 *  $Revision: 1.1.2.1 $
 *
 *  last change: $Author: mhu $ $Date: 2008/09/18 16:10:50 $
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

#ifndef _STORE_LOCKBYTE_HXX_
#define _STORE_LOCKBYTE_HXX_ "$Revision: 1.1.2.1 $"

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

#ifndef _RTL_REF_HXX_
#include "rtl/ref.hxx"
#endif
#ifndef _RTL_USTRING_H_
#include "rtl/ustring.h"
#endif

#ifndef _STORE_TYPES_H_
#include "store/types.h"
#endif
#ifndef _STORE_STORBASE_HXX_
#include "storbase.hxx"
#endif

namespace store
{

/*========================================================================
 *
 * ILockBytes interface.
 *
 *======================================================================*/
class ILockBytes : public rtl::IReference
{
public:
    /**
        @param  rxAllocator [out]
        @param  nPageSize [in]
    */
    storeError initialize (
        rtl::Reference< PageData::Allocator > & rxAllocator,
        sal_uInt16                              nPageSize);

    /**
        @param  rPage [out]
        @param  nOffset [in]
     */
    storeError readPageAt (
        PageHolder & rPage,
        sal_uInt32   nOffset);

    /**
        @param  rPage [in]
        @param  nOffset [in]
     */
    storeError writePageAt (
        PageHolder const & rPage,
        sal_uInt32         nOffset);

    /**
        @param  nOffset [in]
        @param  pBuffer [out]
        @param  nBytes [in]
        @return store_E_None upon success
     */
    storeError readAt (
        sal_uInt32  nOffset,
        void       *pBuffer,
        sal_uInt32  nBytes);

    /**
        @param  nOffset [in]
        @param  pBuffer [in]
        @param  nBytes [in]
        @return store_E_None upon success
     */
    storeError writeAt (
        sal_uInt32  nOffset,
        const void *pBuffer,
        sal_uInt32  nBytes);

    /**
        @param  rnSize [out]
        @return store_E_None upon success
     */
    storeError getSize (sal_uInt32 & rnSize);

    /**
        @param  nSize [in]
        @return store_E_None upon success
     */
    storeError setSize (sal_uInt32 nSize);

    /**
        @return store_E_None upon success
     */
    storeError flush();

private:
    /** Implementation (abstract).
     */
    virtual storeError initialize_Impl (
        rtl::Reference< PageData::Allocator > & rxAllocator,
        sal_uInt16                              nPageSize) = 0;

    virtual storeError readPageAt_Impl (
        PageHolder & rPage,
        sal_uInt32   nOffset) = 0;

    virtual storeError writePageAt_Impl (
        PageHolder const & rPage,
        sal_uInt32         nOffset) = 0;

    virtual storeError readAt_Impl (
        sal_uInt32  nOffset,
        void       *pBuffer,
        sal_uInt32  nBytes) = 0;

    virtual storeError writeAt_Impl (
        sal_uInt32  nOffset,
        const void *pBuffer,
        sal_uInt32  nBytes) = 0;

    virtual storeError getSize_Impl (
        sal_uInt32 & rnSize) = 0;

    virtual storeError setSize_Impl (
        sal_uInt32 nSize) = 0;

    virtual storeError flush_Impl() = 0;
};

/*========================================================================
 *
 * ILockBytes factories.
 *
 *======================================================================*/

storeError
FileLockBytes_createInstance (
  rtl::Reference< store::ILockBytes > & rxLockBytes,
  rtl_uString *   pFilename,
  storeAccessMode eAccessMode
);

storeError
MemoryLockBytes_createInstance (
  rtl::Reference< store::ILockBytes > & rxLockBytes
);

/*========================================================================
 *
 * The End.
 *
 *======================================================================*/

} // namespace store

#endif /* !_STORE_LOCKBYTE_HXX_ */

