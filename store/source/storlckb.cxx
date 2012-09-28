/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


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
    if (m_xManager.is() && m_xNode.is())
    {
        m_xManager->releasePage(m_xNode->m_aDescr);
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
            if(nLength > nBytes)
            {
                nLength = nBytes;
            }
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
            if(nLength > nBytes)
            {
                nLength = nBytes;
            }

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
            if(nLength > nBytes)
            {
                nLength = nBytes;
            }

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
            if(nLength > nBytes)
            {
                nLength = nBytes;
            }
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
