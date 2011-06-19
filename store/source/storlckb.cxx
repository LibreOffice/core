/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "storlckb.hxx"

#include "sal/types.h"
#include "sal/macros.h"
#include "rtl/string.h"
#include "rtl/ref.hxx"
#include "osl/mutex.hxx"

#include "store/types.h"
#include "object.hxx"

#include "storbase.hxx"
#include "stordata.hxx"
#include "storpage.hxx"

using namespace store;

/*========================================================================
 *
 * OStoreLockBytes implementation.
 *
 *======================================================================*/
const sal_uInt32 OStoreLockBytes::m_nTypeId = sal_uInt32(0x94190310);

/*
 * OStoreLockBytes.
 */
OStoreLockBytes::OStoreLockBytes (void)
    : m_xManager   (),
      m_xNode      (),
      m_bWriteable (false)
{
}

/*
 * ~OStoreLockBytes.
 */
OStoreLockBytes::~OStoreLockBytes (void)
{
    if (m_xManager.is())
    {
        if (m_xNode.is())
        {
            OStorePageDescriptor aDescr (m_xNode->m_aDescr);
            if (m_bWriteable)
                m_xManager->releasePage (aDescr, store_AccessReadWrite);
            else
                m_xManager->releasePage (aDescr, store_AccessReadOnly);
        }
    }
}

/*
 * isKindOf.
 */
sal_Bool SAL_CALL OStoreLockBytes::isKindOf (sal_uInt32 nTypeId)
{
    return (nTypeId == m_nTypeId);
}

/*
 * create.
 */
storeError OStoreLockBytes::create (
    OStorePageManager *pManager,
    rtl_String        *pPath,
    rtl_String        *pName,
    storeAccessMode    eMode)
{
    rtl::Reference<OStorePageManager> xManager (pManager);
    if (!xManager.is())
        return store_E_InvalidAccess;

    if (!(pPath && pName))
        return store_E_InvalidParameter;

    OStoreDirectoryPageObject aPage;
    storeError eErrCode = xManager->iget (
        aPage, STORE_ATTRIB_ISFILE,
        pPath, pName, eMode);
    if (eErrCode != store_E_None)
        return eErrCode;

    if (!(aPage.attrib() & STORE_ATTRIB_ISFILE))
    {
        // No ISFILE in older versions (backward compatibility).
        if (aPage.attrib() & STORE_ATTRIB_ISLINK)
            return store_E_NotFile;
    }

    // ...
    inode_holder_type xNode (aPage.get());
    if (eMode != store_AccessReadOnly)
        eErrCode = xManager->acquirePage (xNode->m_aDescr, store_AccessReadWrite);
    else
        eErrCode = xManager->acquirePage (xNode->m_aDescr, store_AccessReadOnly);
    if (eErrCode != store_E_None)
        return eErrCode;

    // ...
    m_xManager   = xManager;
    m_xNode      = xNode;
    m_bWriteable = (eMode != store_AccessReadOnly);

    // Check for truncation.
    if (eMode == store_AccessCreate)
    {
        // Truncate to zero length.
        eErrCode = setSize(0);
    }
    return eErrCode;
}

/*
 * readAt.
 */
storeError OStoreLockBytes::readAt (
    sal_uInt32  nOffset,
    void       *pBuffer,
    sal_uInt32  nBytes,
    sal_uInt32 &rnDone)
{
    rnDone = 0;

    if (!m_xManager.is())
        return store_E_InvalidAccess;

    if (!pBuffer)
        return store_E_InvalidParameter;
    if (!nBytes)
        return store_E_None;

    // Acquire exclusive access.
    osl::MutexGuard aGuard (*m_xManager);

    // Determine data length.
    OStoreDirectoryPageObject aPage (m_xNode.get());

    sal_uInt32 nDataLen = aPage.dataLength();
    if ((nOffset + nBytes) > nDataLen)
        nBytes = nDataLen - nOffset;

    // Read data.
    OStoreDataPageObject aData;
    sal_uInt8 *pData = (sal_uInt8*)pBuffer;
    while ((0 < nBytes) && (nOffset < nDataLen))
    {
        // Determine 'Offset' scope.
        inode::ChunkScope eScope = m_xNode->scope (nOffset);
        if (eScope == inode::SCOPE_INTERNAL)
        {
            // Read from inode page (internal scope).
            inode::ChunkDescriptor aDescr (
                nOffset, m_xNode->capacity());

            sal_uInt32 nLength = sal_uInt32(aDescr.m_nLength);
            nLength = SAL_MIN(nLength, nBytes);

            memcpy (
                &pData[rnDone],
                &m_xNode->m_pData[aDescr.m_nOffset],
                nLength);

            // Adjust counters.
            rnDone  += nLength;
            nOffset += nLength;
            nBytes  -= nLength;
        }
        else
        {
            // Read from data page (external scope).
            inode::ChunkDescriptor aDescr (
                nOffset - m_xNode->capacity(), OStoreDataPageData::capacity(m_xNode->m_aDescr)); // @@@

            sal_uInt32 nLength = sal_uInt32(aDescr.m_nLength);
            nLength = SAL_MIN(nLength, nBytes);

            storeError eErrCode = aPage.read (aDescr.m_nPage, aData, *m_xManager);
            if (eErrCode != store_E_None)
            {
                if (eErrCode != store_E_NotExists)
                    return eErrCode;

                memset (
                    &pData[rnDone],
                    0,
                    nLength);
            }
            else
            {
                PageHolderObject< data > xData (aData.makeHolder<data>());
                memcpy (
                    &pData[rnDone],
                    &xData->m_pData[aDescr.m_nOffset],
                    nLength);
            }

            // Adjust counters.
            rnDone  += nLength;
            nOffset += nLength;
            nBytes  -= nLength;
        }
    }

    // Done.
    return store_E_None;
}

/*
 * writeAt.
 */
storeError OStoreLockBytes::writeAt (
    sal_uInt32  nOffset,
    const void *pBuffer,
    sal_uInt32  nBytes,
    sal_uInt32 &rnDone)
{
    rnDone = 0;

    if (!m_xManager.is())
        return store_E_InvalidAccess;
    if (!m_bWriteable)
        return store_E_AccessViolation;

    if (!pBuffer)
        return store_E_InvalidParameter;
    if (!nBytes)
        return store_E_None;

    // Acquire exclusive access.
    osl::MutexGuard aGuard (*m_xManager);

    // Write data.
    OStoreDirectoryPageObject aPage (m_xNode.get());
    const sal_uInt8 *pData = (const sal_uInt8*)pBuffer;

    storeError eErrCode = store_E_None;
    while (nBytes > 0)
    {
        // Determine 'Offset' scope.
        inode::ChunkScope eScope = m_xNode->scope (nOffset);
        if (eScope == inode::SCOPE_INTERNAL)
        {
            // Write to inode page (internal scope).
            inode::ChunkDescriptor aDescr (
                nOffset, m_xNode->capacity());

            sal_uInt32 nLength = sal_uInt32(aDescr.m_nLength);
            nLength = SAL_MIN(nLength, nBytes);

            memcpy (
                &m_xNode->m_pData[aDescr.m_nOffset],
                &pData[rnDone], nLength);

            // Mark inode dirty.
            aPage.touch();

            // Adjust counters.
            rnDone  += nLength;
            nOffset += nLength;
            nBytes  -= nLength;

            // Adjust data length.
            if (aPage.dataLength() < nOffset)
                aPage.dataLength (nOffset);
        }
        else
        {
            // Write to data page (external scope).
            OStoreDataPageObject aData;

            inode::ChunkDescriptor aDescr (
                nOffset - m_xNode->capacity(), OStoreDataPageData::capacity(m_xNode->m_aDescr)); // @@@

            sal_uInt32 nLength = sal_uInt32(aDescr.m_nLength);
            if ((aDescr.m_nOffset > 0) || (nBytes < nLength))
            {
                // Unaligned. Need to load/create data page.
// @@@ loadOrCreate()
                eErrCode = aPage.read (aDescr.m_nPage, aData, *m_xManager);
                if (eErrCode != store_E_None)
                {
                    if (eErrCode != store_E_NotExists)
                        return eErrCode;

                    eErrCode = aData.construct<data>(m_xManager->allocator());
                    if (eErrCode != store_E_None)
                        return eErrCode;
                }
            }

            PageHolderObject< data > xData (aData.makeHolder<data>());
            if (!xData.is())
            {
                eErrCode = aData.construct<data>(m_xManager->allocator());
                if (eErrCode != store_E_None)
                    return eErrCode;
                xData = aData.makeHolder<data>();
            }

            // Modify data page.
            nLength = SAL_MIN(nLength, nBytes);
            memcpy (
                &xData->m_pData[aDescr.m_nOffset],
                &pData[rnDone], nLength);

            // Save data page.
            eErrCode = aPage.write (aDescr.m_nPage, aData, *m_xManager);
            if (eErrCode != store_E_None)
                return eErrCode;

            // Adjust counters.
            rnDone  += nLength;
            nOffset += nLength;
            nBytes  -= nLength;

            // Adjust data length.
            if (aPage.dataLength() < nOffset)
                aPage.dataLength (nOffset);
        }
    }

    // Check for modified inode.
    if (aPage.dirty())
        return m_xManager->saveObjectAt (aPage, aPage.location());
    else
        return store_E_None;
}

/*
 * flush.
 */
storeError OStoreLockBytes::flush (void)
{
    if (!m_xManager.is())
        return store_E_InvalidAccess;

    return m_xManager->flush();
}

/*
 * setSize.
 */
storeError OStoreLockBytes::setSize (sal_uInt32 nSize)
{
    if (!m_xManager.is())
        return store_E_InvalidAccess;
    if (!m_bWriteable)
        return store_E_AccessViolation;

    // Acquire exclusive access.
    osl::MutexGuard aGuard (*m_xManager);

    // Determine current length.
    OStoreDirectoryPageObject aPage (m_xNode.get());
    sal_uInt32 nDataLen = aPage.dataLength();

    if (nSize == nDataLen)
        return store_E_None;

    if (nSize < nDataLen)
    {
        // Truncate.
        storeError eErrCode = store_E_None;

        // Determine 'Size' scope.
        inode::ChunkScope eSizeScope = m_xNode->scope (nSize);
        if (eSizeScope == inode::SCOPE_INTERNAL)
        {
            // Internal 'Size' scope. Determine 'Data' scope.
            inode::ChunkScope eDataScope = m_xNode->scope (nDataLen);
            if (eDataScope == inode::SCOPE_EXTERNAL)
            {
                // External 'Data' scope. Truncate all external data pages.
                eErrCode = aPage.truncate (0, *m_xManager);
                if (eErrCode != store_E_None)
                    return eErrCode;
            }

            // Truncate internal data page.
            inode::ChunkDescriptor aDescr (nSize, m_xNode->capacity());
            memset (
                &(m_xNode->m_pData[aDescr.m_nOffset]),
                0, aDescr.m_nLength);
        }
        else
        {
            // External 'Size' scope. Truncate external data pages.
            inode::ChunkDescriptor aDescr (
                nSize - m_xNode->capacity(), OStoreDataPageData::capacity(m_xNode->m_aDescr)); // @@@

            sal_uInt32 nPage = aDescr.m_nPage;
            if (aDescr.m_nOffset) nPage += 1;

            eErrCode = aPage.truncate (nPage, *m_xManager);
            if (eErrCode != store_E_None)
                return eErrCode;
        }
    }

    // Set (extended or truncated) size.
    aPage.dataLength (nSize);

    // Save modified inode.
    return m_xManager->saveObjectAt (aPage, aPage.location());
}

/*
 * stat.
 */
storeError OStoreLockBytes::stat (sal_uInt32 &rnSize)
{
    rnSize = 0;

    if (!m_xManager.is())
        return store_E_InvalidAccess;

    OStoreDirectoryPageObject aPage (m_xNode.get());
    rnSize = aPage.dataLength();
    return store_E_None;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
