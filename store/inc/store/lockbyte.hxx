/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lockbyte.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:38:20 $
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
#define _STORE_LOCKBYTE_HXX_ "$Revision: 1.5 $"

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef _STORE_TYPES_H_
#include <store/types.h>
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
        sal_uInt32 &rnDone) = 0;

    /**
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
        sal_uInt32 &rnDone) = 0;

    /**
        @return store_E_None upon success
     */
    virtual storeError flush (void) = 0;

    /**
        @param  nSize [in]
        @return store_E_None upon success
     */
    virtual storeError setSize (sal_uInt32 nSize) = 0;

    /**
        @param  rnSize [out]
        @return store_E_None upon success
     */
    virtual storeError stat (sal_uInt32 &rnSize) = 0;

    /**
        @param  nOffset [in]
        @param  nBytes [in]
        @return store_E_None upon success
                store_E_LockingViolation
     */
    virtual storeError lockRange (
        sal_uInt32 nOffset,
        sal_uInt32 nBytes) = 0;

    /**
        @param  nOffset [in]
        @param  nBytes [in]
        @return store_E_None upon success
                store_E_LockingViolation
     */
    virtual storeError unlockRange (
        sal_uInt32 nOffset,
        sal_uInt32 nBytes) = 0;
};

/*========================================================================
 *
 * The End.
 *
 *======================================================================*/

} // namespace store

#endif /* !_STORE_LOCKBYTE_HXX_ */

