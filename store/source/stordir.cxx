/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_store.hxx"

#include "stordir.hxx"

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_TEXTCVT_H_
#include <rtl/textcvt.h>
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _STORE_TYPES_H_
#include "store/types.h"
#endif
#ifndef _STORE_OBJECT_HXX_
#include "object.hxx"
#endif

#ifndef _STORE_STORBASE_HXX_
#include "storbase.hxx"
#endif
#ifndef _STORE_STORDATA_HXX_
#include "stordata.hxx"
#endif
#ifndef _STORE_STORPAGE_HXX_
#include "storpage.hxx"
#endif

using namespace store;

/*========================================================================
 *
 * OStore... internals.
 *
 *======================================================================*/
/*
 * __store_convertTextToUnicode.
 */
inline sal_Size __store_convertTextToUnicode (
    rtl_TextToUnicodeConverter  hConverter,
    const sal_Char *pSrcBuffer, sal_Size nSrcLength,
    sal_Unicode    *pDstBuffer, sal_Size nDstLength)
{
    sal_uInt32 nCvtInfo = 0;
    sal_Size nCvtBytes = 0;
    return rtl_convertTextToUnicode (
        hConverter, 0,
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
const sal_uInt32 OStoreDirectory_Impl::m_nTypeId = sal_uInt32(0x89191107);

/*
 * OStoreDirectory_Impl.
 */
OStoreDirectory_Impl::OStoreDirectory_Impl (void)
    : m_xManager (),
      m_aDescr   (0, 0, 0),
      m_nPath    (0),
      m_hTextCvt (NULL)
{}

/*
 * ~OStoreDirectory_Impl.
 */
OStoreDirectory_Impl::~OStoreDirectory_Impl (void)
{
    if (m_xManager.is())
    {
        if (m_aDescr.m_nAddr != STORE_PAGE_NULL)
            m_xManager->releasePage (m_aDescr, store_AccessReadOnly);
    }
    rtl_destroyTextToUnicodeConverter (m_hTextCvt);
}

/*
 * isKindOf.
 */
sal_Bool SAL_CALL OStoreDirectory_Impl::isKindOf (sal_uInt32 nTypeId)
{
    return (nTypeId == m_nTypeId);
}

/*
 * create.
 */
storeError OStoreDirectory_Impl::create (
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
        aPage, STORE_ATTRIB_ISDIR,
        pPath, pName, eMode);
    if (eErrCode != store_E_None)
        return eErrCode;

    if (!(aPage.attrib() & STORE_ATTRIB_ISDIR))
        return store_E_NotDirectory;

    inode_holder_type xNode (aPage.get());
    eErrCode = xManager->acquirePage (xNode->m_aDescr, store_AccessReadOnly);
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
    if (m_hTextCvt == NULL)
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
                sal_Size  n = rtl_str_getLength (p);
                sal_Size  k = rFindData.m_nLength;

                n = __store_convertTextToUnicode (
                    m_hTextCvt, p, n,
                    rFindData.m_pszName, STORE_MAXIMUM_NAMESIZE - 1);
                if (k > n)
                {
                    k = (k - n) * sizeof(sal_Unicode);
                    memset (&rFindData.m_pszName[n], 0, k);
                }

                rFindData.m_nLength  = n;
                rFindData.m_nAttrib |= aPage.attrib();
                rFindData.m_nSize    = aPage.dataLength();

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
