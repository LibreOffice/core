/*************************************************************************
 *
 *  $RCSfile: memlckb.cxx,v $
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

#define _STORE_MEMLCKB_CXX_ "$Revision: 1.1.1.1 $"

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif
#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
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
#ifndef _STORE_MEMLCKB_HXX_
#include <store/memlckb.hxx>
#endif

#ifndef _STORE_TYPES_H_
#include <store/types.h>
#endif

#ifdef _USE_NAMESPACE
using namespace store;
#endif

/*========================================================================
 *
 * OMemoryLockBytes internals.
 *
 *======================================================================*/
#ifdef DEBUG
#ifdef inline
#undef inline
#endif
#define inline
#endif /* DEBUG */

/*========================================================================
 *
 * OMemoryLockBytes_Impl interface.
 *
 *======================================================================*/
#ifdef _USE_NAMESPACE
namespace store {
#endif

class OMemoryLockBytes_Impl
{
    sal_uInt8  *m_pBuffer;
    sal_uInt32  m_nSize;

public:
    OMemoryLockBytes_Impl (void);
    ~OMemoryLockBytes_Impl (void);

    storeError resize (sal_uInt32 nSize);

    storeError readAt (
        sal_uInt32  nOffset,
        void       *pBuffer,
        sal_uInt32  nBytes,
        sal_uInt32 &rnDone);
    storeError writeAt (
        sal_uInt32  nOffset,
        const void *pBuffer,
        sal_uInt32  nBytes,
        sal_uInt32 &rnDone);

    storeError stat (sal_uInt32 &rnSize);
};

#ifdef _USE_NAMESPACE
}
#endif

/*========================================================================
 *
 * OMemoryLockBytes_Impl (inline) implementation.
 *
 *======================================================================*/
/*
 * OMemoryLockBytes_Impl.
 */
inline OMemoryLockBytes_Impl::OMemoryLockBytes_Impl (void)
    : m_pBuffer (0), m_nSize (0)
{
}

/*
 * ~OMemoryLockBytes_Impl.
 */
inline OMemoryLockBytes_Impl::~OMemoryLockBytes_Impl (void)
{
    rtl_freeMemory (m_pBuffer);
}

/*
 * resize.
 */
inline storeError OMemoryLockBytes_Impl::resize (sal_uInt32 nSize)
{
    if (!(nSize == m_nSize))
    {
        m_pBuffer = (sal_uInt8*)(rtl_reallocateMemory (m_pBuffer, nSize));
        if (!m_pBuffer)
        {
            m_nSize = 0;
            if (nSize > 0)
                return store_E_OutOfMemory;
            else
                return store_E_None;
        }

        if (nSize > m_nSize)
            rtl_zeroMemory (m_pBuffer + m_nSize, nSize - m_nSize);
        m_nSize = nSize;
    }
    return store_E_None;
}

/*
 * readAt.
 */
inline storeError OMemoryLockBytes_Impl::readAt (
    sal_uInt32  nOffset,
    void       *pBuffer,
    sal_uInt32  nBytes,
    sal_uInt32 &rnDone)
{
    if (m_pBuffer)
    {
        if (!(nOffset < m_nSize))
            return store_E_None;

        nBytes = VOS_MIN(nOffset + nBytes, m_nSize) - nOffset;
        if (!(nBytes > 0))
            return store_E_None;

        rtl_copyMemory (pBuffer, m_pBuffer + nOffset, nBytes);
        rnDone = nBytes;
    }
    return store_E_None;
}

/*
 * writeAt.
 */
inline storeError OMemoryLockBytes_Impl::writeAt (
        sal_uInt32  nOffset,
        const void *pBuffer,
        sal_uInt32  nBytes,
        sal_uInt32 &rnDone)
{
    if (m_nSize < (nOffset + nBytes))
    {
        storeError eErrCode = resize (nOffset + nBytes);
        if (eErrCode != store_E_None)
            return eErrCode;
    }

    rtl_copyMemory (m_pBuffer + nOffset, pBuffer, nBytes);
    rnDone = nBytes;

    return store_E_None;
}

/*
 * stat.
 */
inline storeError OMemoryLockBytes_Impl::stat (sal_uInt32 &rnSize)
{
    rnSize = m_nSize;
    return store_E_None;
}

/*========================================================================
 *
 * OMemoryLockBytes implementation.
 *
 *======================================================================*/
VOS_IMPLEMENT_CLASSINFO(
    VOS_CLASSNAME (OMemoryLockBytes, store),
    VOS_NAMESPACE (OMemoryLockBytes, store),
    VOS_NAMESPACE (OStoreObject, store),
    0);

/*
 * OMemoryLockBytes.
 */
OMemoryLockBytes::OMemoryLockBytes (void)
{
    // Acquire exclusive access.
    NAMESPACE_VOS(OGuard) aGuard (m_aMutex);
    m_pImpl = new OMemoryLockBytes_Impl();
}

/*
 * ~OMemoryLockBytes.
 */
OMemoryLockBytes::~OMemoryLockBytes (void)
{
    // Acquire exclusive access.
    NAMESPACE_VOS(OGuard) aGuard (m_aMutex);
    delete m_pImpl;
}

/*
 * acquire.
 */
NAMESPACE_VOS(IReference)::RefCount
SAL_CALL OMemoryLockBytes::acquire (void)
{
    return OStoreObject::acquire();
}

/*
 * release.
 */
NAMESPACE_VOS(IReference)::RefCount
SAL_CALL OMemoryLockBytes::release (void)
{
    return OStoreObject::release();
}

/*
 * referenced.
 */
NAMESPACE_VOS(IReference)::RefCount
SAL_CALL OMemoryLockBytes::referenced (void) const
{
    return OStoreObject::referenced();
}

/*
 * readAt.
 */
storeError OMemoryLockBytes::readAt (
    sal_uInt32  nOffset,
    void       *pBuffer,
    sal_uInt32  nBytes,
    sal_uInt32 &rnDone)
{
    // Initialize [out] param.
    rnDone = 0;

    // Acquire exclusive access.
    NAMESPACE_VOS(OGuard) aGuard (m_aMutex);
    return m_pImpl->readAt (nOffset, pBuffer, nBytes, rnDone);
}

/*
 * writeAt.
 */
storeError OMemoryLockBytes::writeAt (
    sal_uInt32  nOffset,
    const void *pBuffer,
    sal_uInt32  nBytes,
    sal_uInt32 &rnDone)
{
    // Initialize [out] param.
    rnDone = 0;

    // Acquire exclusive access.
    NAMESPACE_VOS(OGuard) aGuard (m_aMutex);
    return m_pImpl->writeAt (nOffset, pBuffer, nBytes, rnDone);
}

/*
 * flush.
 */
storeError OMemoryLockBytes::flush (void)
{
    // Acquire exclusive access.
    NAMESPACE_VOS(OGuard) aGuard (m_aMutex);
    return store_E_None;
}

/*
 * setSize.
 */
storeError OMemoryLockBytes::setSize (sal_uInt32 nSize)
{
    // Acquire exclusive access.
    NAMESPACE_VOS(OGuard) aGuard (m_aMutex);
    return m_pImpl->resize (nSize);
}

/*
 * stat.
 */
storeError OMemoryLockBytes::stat (sal_uInt32 &rnSize)
{
    // Initialize [out] param.
    rnSize = 0;

    // Acquire exclusive access.
    NAMESPACE_VOS(OGuard) aGuard (m_aMutex);
    return m_pImpl->stat (rnSize);
}

/*
 * lockRange.
 */
storeError OMemoryLockBytes::lockRange (
    sal_uInt32 nOffset, sal_uInt32 nBytes)
{
    // Acquire exclusive access.
    NAMESPACE_VOS(OGuard) aGuard (m_aMutex);
    return store_E_None; // E_Unsupported
}

/*
 * unlockRange.
 */
storeError OMemoryLockBytes::unlockRange (
    sal_uInt32 nOffset, sal_uInt32 nBytes)
{
    // Acquire exclusive access.
    NAMESPACE_VOS(OGuard) aGuard (m_aMutex);
    return store_E_None; // E_Unsupported
}

