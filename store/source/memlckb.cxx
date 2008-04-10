/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: memlckb.cxx,v $
 * $Revision: 1.10 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_store.hxx"

#include <store/memlckb.hxx>

#ifndef INCLUDED_STDDEF_H
#include <stddef.h>
#define INCLUDED_STDDEF_H
#endif

#ifndef INCLUDED_STRING_H
#include <string.h>
#define INCLUDED_STRING_H
#endif
#include <sal/types.h>
#include <sal/macros.h>
#include <rtl/alloc.h>
#include <osl/mutex.hxx>
#include <store/types.h>
#include <store/object.hxx>
#include <store/lockbyte.hxx>

using namespace store;

/*========================================================================
 *
 * OMemoryLockBytes internals.
 *
 *======================================================================*/

/*
 * __store_memcpy.
 */
inline void __store_memcpy (void * dst, const void * src, sal_uInt32 n)
{
    ::memcpy (dst, src, n);
}

/*
 * __store_memset.
 */
inline void __store_memset (void * dst, int val, sal_uInt32 n)
{
    ::memset (dst, val, n);
}

/*========================================================================
 *
 * OMemoryLockBytes_Impl interface.
 *
 *======================================================================*/
namespace store
{

class OMemoryLockBytes_Impl
{
    sal_uInt8  *m_pBuffer;
    sal_uInt32  m_nSize;

public:
    static void * operator new (size_t n) SAL_THROW(())
    {
        return rtl_allocateMemory (sal_uInt32(n));
    }
    static void operator delete (void * p, size_t) SAL_THROW(())
    {
        rtl_freeMemory (p);
    }

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

} // namespace store

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
            __store_memset (m_pBuffer + m_nSize, 0, nSize - m_nSize);
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

        nBytes = SAL_MIN(nOffset + nBytes, m_nSize) - nOffset;
        if (!(nBytes > 0))
            return store_E_None;

        __store_memcpy (pBuffer, m_pBuffer + nOffset, nBytes);
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

    __store_memcpy (m_pBuffer + nOffset, pBuffer, nBytes);
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
/*
 * OMemoryLockBytes.
 */
OMemoryLockBytes::OMemoryLockBytes (void)
    : m_pImpl (new OMemoryLockBytes_Impl())
{
}

/*
 * ~OMemoryLockBytes.
 */
OMemoryLockBytes::~OMemoryLockBytes (void)
{
    delete m_pImpl;
}

/*
 * acquire.
 */
oslInterlockedCount SAL_CALL OMemoryLockBytes::acquire (void)
{
    return OStoreObject::acquire();
}

/*
 * release.
 */
oslInterlockedCount SAL_CALL OMemoryLockBytes::release (void)
{
    return OStoreObject::release();
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
    osl::MutexGuard aGuard (m_aMutex);
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
    osl::MutexGuard aGuard (m_aMutex);
    return m_pImpl->writeAt (nOffset, pBuffer, nBytes, rnDone);
}

/*
 * flush.
 */
storeError OMemoryLockBytes::flush (void)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);
    return store_E_None;
}

/*
 * setSize.
 */
storeError OMemoryLockBytes::setSize (sal_uInt32 nSize)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);
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
    osl::MutexGuard aGuard (m_aMutex);
    return m_pImpl->stat (rnSize);
}

/*
 * lockRange.
 */
storeError OMemoryLockBytes::lockRange (sal_uInt32, sal_uInt32)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);
    return store_E_None; // E_Unsupported
}

/*
 * unlockRange.
 */
storeError OMemoryLockBytes::unlockRange (sal_uInt32, sal_uInt32)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard (m_aMutex);
    return store_E_None; // E_Unsupported
}

