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

#include "stordir.hxx"

#include <sal/types.h>

#include <rtl/textcvt.h>
#include <rtl/ref.hxx>
#include <rtl/ustring.h>

#include <osl/mutex.hxx>

#include <store/types.h>

#include "storbase.hxx"
#include "stordata.hxx"
#include "storpage.hxx"

using namespace store;

/*========================================================================
 *
 * OStore... internals.
 *
 *======================================================================*/
/*
 * convertTextToUnicode.
 */
static sal_Size convertTextToUnicode (
    rtl_TextToUnicodeConverter  hConverter,
    const sal_Char *pSrcBuffer, sal_Size nSrcLength,
    sal_Unicode    *pDstBuffer, sal_Size nDstLength)
{
    sal_uInt32 nCvtInfo = 0;
    sal_Size nCvtBytes = 0;
    return rtl_convertTextToUnicode (
        hConverter, nullptr,
        pSrcBuffer, nSrcLength,
        pDstBuffer, nDstLength,
        OSTRING_TO_OUSTRING_CVTFLAGS,
        &nCvtInfo, &nCvtBytes);
}

/*========================================================================
 *
 * OStoreDirectory_Impl implementation.
 *
 *======================================================================*/
const sal_uInt32 OStoreDirectory_Impl::m_nTypeId(0x89191107);

/*
 * OStoreDirectory_Impl.
 */
OStoreDirectory_Impl::OStoreDirectory_Impl()
    : m_xManager (),
      m_aDescr   (0, 0, 0),
      m_nPath    (0),
      m_hTextCvt (nullptr)
{}

/*
 * ~OStoreDirectory_Impl.
 */
OStoreDirectory_Impl::~OStoreDirectory_Impl()
{
    if (m_xManager.is())
    {
        if (m_aDescr.m_nAddr != STORE_PAGE_NULL)
            m_xManager->releasePage (m_aDescr);
    }
    rtl_destroyTextToUnicodeConverter (m_hTextCvt);
}

/*
 * isKindOf.
 */
bool OStoreDirectory_Impl::isKindOf (sal_uInt32 nTypeId)
{
    return (nTypeId == m_nTypeId);
}

/*
 * create.
 */
storeError OStoreDirectory_Impl::create (
    OStorePageManager *pManager,
    rtl_String const  *pPath,
    rtl_String const  *pName,
    storeAccessMode    eMode)
{
    rtl::Reference<OStorePageManager> xManager (pManager);
    if (!xManager.is())
        return store_E_InvalidAccess;

    if (!(pPath && pName))
        return store_E_InvalidParameter;

    OStoreDirectoryPageObject aPage;
    storeError eErrCode = xManager->iget (
        aPage, STORE_ATTRIB_ISDIR,
        pPath, pName, eMode);
    if (eErrCode != store_E_None)
        return eErrCode;

    if (!(aPage.attrib() & STORE_ATTRIB_ISDIR))
        return store_E_NotDirectory;

    inode_holder_type xNode (aPage.get());
    eErrCode = xManager->acquirePage (xNode->m_aDescr, storeAccessMode::ReadOnly);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Evaluate iteration path.
    m_nPath = aPage.path();
    m_nPath = rtl_crc32 (m_nPath, "/", 1);

    // Save page manager, and descriptor.
    m_xManager = xManager;
    m_aDescr   = xNode->m_aDescr;

    return store_E_None;
}

/*
 * iterate.
 */
storeError OStoreDirectory_Impl::iterate (storeFindData &rFindData)
{
    if (!m_xManager.is())
        return store_E_InvalidAccess;

    storeError eErrCode = store_E_NoMoreFiles;
    if (!rFindData.m_nReserved)
        return eErrCode;

    // Acquire exclusive access.
    osl::MutexGuard aGuard (*m_xManager);

    // Check TextConverter.
    if (m_hTextCvt == nullptr)
        m_hTextCvt = rtl_createTextToUnicodeConverter(RTL_TEXTENCODING_UTF8);

    // Setup iteration key.
    OStorePageKey aKey (rFindData.m_nReserved, m_nPath);

    // Iterate.
    for (;;)
    {
        OStorePageLink aLink;
        eErrCode = m_xManager->iterate (aKey, aLink, rFindData.m_nAttrib);
        if (!((eErrCode == store_E_None) && (aKey.m_nHigh == store::htonl(m_nPath))))
            break;

        if (!(rFindData.m_nAttrib & STORE_ATTRIB_ISLINK))
        {
            // Load page.
            OStoreDirectoryPageObject aPage;
            eErrCode = m_xManager->loadObjectAt (aPage, aLink.location());
            if (eErrCode == store_E_None)
            {
                inode_holder_type xNode (aPage.get());

                // Setup FindData.
                sal_Char *p = xNode->m_aNameBlock.m_pData;
                sal_Int32 n = rtl_str_getLength (p);
                sal_Int32 k = rFindData.m_nLength;

                n = convertTextToUnicode (
                    m_hTextCvt, p, n,
                    rFindData.m_pszName, STORE_MAXIMUM_NAMESIZE - 1);
                if (k > n)
                {
                    k = (k - n) * sizeof(sal_Unicode);
                    memset (&rFindData.m_pszName[n], 0, k);
                }

                rFindData.m_nLength  = n;
                rFindData.m_nAttrib |= aPage.attrib();

                // Leave.
                rFindData.m_nReserved = store::ntohl(aKey.m_nLow);
                return store_E_None;
            }
        }

        if (aKey.m_nLow == 0)
            break;
        aKey.m_nLow = store::htonl(store::ntohl(aKey.m_nLow) - 1);
    }

    // Finished.
    memset (&rFindData, 0, sizeof (storeFindData));
    return store_E_NoMoreFiles;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
