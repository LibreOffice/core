/*************************************************************************
 *
 *  $RCSfile: storlckb.cxx,v $
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

#define _STORE_STORLCKB_CXX_ "$Revision: 1.1.1.1 $"

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif
#ifndef _RTL_TEXTCVT_H_
#include <rtl/textcvt.h>
#endif
#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif

#ifndef _STORE_TYPES_H_
#include <store/types.h>
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

#ifndef _STORE_STORBASE_HXX_
#include <storbase.hxx>
#endif
#ifndef _STORE_STORDATA_HXX_
#include <stordata.hxx>
#endif
#ifndef _STORE_STORPAGE_HXX_
#include <storpage.hxx>
#endif

#ifndef _STORE_STORLCKB_HXX_
#include <storlckb.hxx>
#endif

#ifdef _USE_NAMESPACE
using namespace rtl;
using namespace store;
#endif

/*========================================================================
 *
 * OStore... internals.
 *
 *======================================================================*/
/*
 * __store_convertTextToUnicode.
 */
inline sal_Int32 __store_convertTextToUnicode (
    rtl_TextToUnicodeConverter  hConverter,
    const sal_Char *pSrcBuffer, sal_Int32 nSrcLength,
    sal_Unicode    *pDstBuffer, sal_Int32 nDstLength)
{
    sal_uInt32 nCvtInfo, nCvtBytes = 0;
    return rtl_convertTextToUnicode (
        hConverter, 0,
        pSrcBuffer, nSrcLength,
        pDstBuffer, nDstLength,
        OSTRING_TO_OUSTRING_CVTFLAGS,
        &nCvtInfo, &nCvtBytes);
}

/*
 * __store_iget.
 * Precond: exclusive access.
 */
static storeError __store_iget (
    OStorePageManager       &rManager,
    OStoreDirectoryPageData &rNode,
    sal_uInt32               nAttrib,
    const rtl_String        *pPath,
    const rtl_String        *pName,
    storeAccessMode          eMode)
{
    // Setup inode page key.
    OStorePageKey aKey;
    storeError eErrCode = OStorePageNameBlock::namei (pPath, pName, aKey);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Check for directory.
    if (nAttrib & STORE_ATTRIB_ISDIR)
    {
        // Ugly, but necessary (backward compatibility).
        aKey.m_nLow = OStorePageGuard::crc32 (aKey.m_nLow, "/", 1);
    }

    // Load inode page.
    OStoreDirectoryPageObject aPage (rNode);
    eErrCode = rManager.load (aKey, aPage);
    if (eErrCode != store_E_None)
    {
        // Check mode and reason.
        if (eErrCode != store_E_NotExists)
            return eErrCode;

        if (eMode == store_AccessReadWrite)
            return store_E_NotExists;
        if (eMode == store_AccessReadOnly)
            return store_E_NotExists;

        if (!rManager.isWriteable())
            return store_E_AccessViolation;

        // Setup inode nameblock.
        rNode.m_aNameBlock.m_aKey    = aKey;
        rNode.m_aNameBlock.m_nAttrib = nAttrib;

        rtl_copyMemory (
            &rNode.m_aNameBlock.m_pData[0],
            pName->buffer, pName->length);

        // Save inode page.
        eErrCode = rManager.save (aKey, aPage);
        if (eErrCode != store_E_None)
            return eErrCode;
    }

    // Check for symbolic link.
    if (aPage.attrib() & STORE_ATTRIB_ISLINK)
    {
        // Obtain 'Destination' page key.
        OStorePageKey aDstKey;
        rtl_copyMemory (&aDstKey, &rNode.m_pData[0], sizeof(aDstKey));

#ifdef OSL_BIGENDIAN
        // Swap to internal representation.
        aDstKey.swap();
#endif /* OSL_BIGENDIAN */

        // Load 'Destination' inode.
        eErrCode = rManager.load (aDstKey, aPage);
        if (eErrCode != store_E_None)
            return eErrCode;
    }

    // Done.
    return store_E_None;
}

/*========================================================================
 *
 * OStoreDirectory implementation.
 *
 *======================================================================*/
VOS_IMPLEMENT_CLASSINFO(
    VOS_CLASSNAME (OStoreDirectory, store),
    VOS_NAMESPACE (OStoreDirectory, store),
    VOS_NAMESPACE (OStoreObject, store),
    0);

/*
 * OStoreDirectory.
 */
OStoreDirectory::OStoreDirectory (void)
    : m_xManager (NULL),
      m_pNode    (NULL),
      m_aDescr   (0, 0, 0),
      m_nPath    (0),
      m_hTextCvt (NULL)
{
}

/*
 * ~OStoreDirectory.
 */
OStoreDirectory::~OStoreDirectory (void)
{
    if (m_xManager.isValid())
    {
        NAMESPACE_VOS(OGuard) aGuard (*m_xManager);
        if (m_pNode)
        {
            m_xManager->releasePage (m_aDescr, store_AccessReadOnly);
        }
    }
    delete m_pNode;
    rtl_destroyTextToUnicodeConverter (m_hTextCvt);
}

/*
 * create.
 */
storeError OStoreDirectory::create (
    OStorePageManager *pManager,
    rtl_uString       *pPath,
    rtl_uString       *pName,
    storeAccessMode    eMode)
{
    NAMESPACE_VOS(ORef)<OStorePageManager> xManager (pManager);
    if (!xManager.isValid())
        return store_E_InvalidAccess;

    if (!(pPath && pName))
        return store_E_InvalidParameter;

    NAMESPACE_VOS(OGuard) aGuard (*xManager);
    storeError eErrCode = xManager->getPageSize (m_aDescr.m_nSize);
    if (eErrCode != store_E_None)
        return eErrCode;

    delete m_pNode;
    m_pNode = new(m_aDescr.m_nSize) inode(m_aDescr.m_nSize);
    if (!m_pNode)
        return store_E_OutOfMemory;

    OString aPath (pPath->buffer, pPath->length, RTL_TEXTENCODING_UTF8);
    OString aName (pName->buffer, pName->length, RTL_TEXTENCODING_UTF8);

    eErrCode = __store_iget (
        *xManager, *m_pNode, STORE_ATTRIB_ISDIR,
        aPath.pData, aName.pData, eMode);
    if (eErrCode != store_E_None)
        return eErrCode;

    sal_uInt32 nAttrib = m_pNode->m_aNameBlock.m_nAttrib;
    if (!(nAttrib & STORE_ATTRIB_ISDIR))
        return store_E_NotDirectory;

    m_aDescr = m_pNode->m_aDescr;
    eErrCode = xManager->acquirePage (m_aDescr, store_AccessReadOnly);
    if (eErrCode == store_E_None)
    {
        // Evaluate iteration path from NameBlock.
        typedef OStorePageGuard G;
        sal_Char *pszName = m_pNode->m_aNameBlock.m_pData;

        m_nPath = m_pNode->m_aNameBlock.m_aKey.m_nHigh;
        m_nPath = G::crc32 (m_nPath, pszName, rtl_str_getLength(pszName));
        m_nPath = G::crc32 (m_nPath, "/", 1);

        // Accept page manager.
        m_xManager = xManager;
    }
    return eErrCode;
}

/*
 * iterate.
 */
storeError OStoreDirectory::iterate (storeFindData &rFindData)
{
    if (!m_xManager.isValid())
        return store_E_InvalidAccess;

    storeError eErrCode = store_E_NoMoreFiles;
    if (!rFindData.m_nReserved)
        return eErrCode;

    // Acquire exclusive access.
    NAMESPACE_VOS(OGuard) aGuard (*m_xManager);

    // Check TextConverter.
    if (m_hTextCvt == NULL)
        m_hTextCvt = rtl_createTextToUnicodeConverter(RTL_TEXTENCODING_UTF8);

    // Setup iteration key and page buffer.
    OStorePageKey aKey (rFindData.m_nReserved, m_nPath);
    OStoreDirectoryPageObject aPage (*m_pNode);

    // Iterate.
    for (;;)
    {
        eErrCode = m_xManager->iterate (aKey, aPage, rFindData.m_nAttrib);
        if (!((eErrCode == store_E_None) && (aKey.m_nHigh == m_nPath)))
            break;

        if (!(rFindData.m_nAttrib & STORE_ATTRIB_ISLINK))
        {
            // Load page.
            eErrCode = m_xManager->load (aPage);
            if (eErrCode == store_E_None)
            {
                // Setup FindData.
                sal_Char *p = m_pNode->m_aNameBlock.m_pData;
                sal_Int32 n = rtl_str_getLength (p);
                sal_Int32 k = rFindData.m_nLength;

                n = __store_convertTextToUnicode (
                    m_hTextCvt, p, n,
                    rFindData.m_pszName, STORE_MAXIMUM_NAMESIZE - 1);
                if (k > n)
                {
                    k = (k - n) * sizeof(sal_Unicode);
                    rtl_zeroMemory (&rFindData.m_pszName[n], k);
                }

                rFindData.m_nLength  = n;
                rFindData.m_nAttrib |= aPage.attrib();
                rFindData.m_nSize    = aPage.dataLength();

                // Leave.
                rFindData.m_nReserved = aKey.m_nLow;
                return store_E_None;
            }
        }

        if (aKey.m_nLow > 0)
            aKey.m_nLow -= 1;
        else
            break;
    }

    // Finished.
    rtl_zeroMemory (&rFindData, sizeof (storeFindData));
    return store_E_NoMoreFiles;
}

/*========================================================================
 *
 * OStoreLockBytes implementation.
 *
 *======================================================================*/
VOS_IMPLEMENT_CLASSINFO(
    VOS_CLASSNAME (OStoreLockBytes, store),
    VOS_NAMESPACE (OStoreLockBytes, store),
    VOS_NAMESPACE (OStoreObject, store),
    0);

/*
 * OStoreLockBytes.
 */
OStoreLockBytes::OStoreLockBytes (void)
    : m_xManager   (NULL),
      m_nPageSize  (0),
      m_pNode      (NULL),
      m_pData      (NULL),
      m_pSingle    (NULL),
      m_pDouble    (NULL),
      m_pTriple    (NULL),
      m_bWriteable (sal_False)
{
}

/*
 * ~OStoreLockBytes.
 */
OStoreLockBytes::~OStoreLockBytes (void)
{
    if (m_xManager.isValid())
    {
        NAMESPACE_VOS(OGuard) aGuard (*m_xManager);
        if (m_pNode)
        {
            OStorePageDescriptor aDescr (m_pNode->m_aDescr);
            if (m_bWriteable)
                m_xManager->releasePage (aDescr, store_AccessReadWrite);
            else
                m_xManager->releasePage (aDescr, store_AccessReadOnly);
        }
    }

    delete m_pNode;
    delete m_pData;

    delete m_pSingle;
    delete m_pDouble;
    delete m_pTriple;
}

/*
 * acquire.
 */
NAMESPACE_VOS(IReference)::RefCount
SAL_CALL OStoreLockBytes::acquire (void)
{
    return OStoreObject::acquire();
}

/*
 * release.
 */
NAMESPACE_VOS(IReference)::RefCount
SAL_CALL OStoreLockBytes::release (void)
{
    return OStoreObject::release();
}

/*
 * referenced.
 */
NAMESPACE_VOS(IReference)::RefCount
SAL_CALL OStoreLockBytes::referenced (void) const
{
    return OStoreObject::referenced();
}

/*
 * create.
 */
storeError OStoreLockBytes::create (
    OStorePageManager *pManager,
    rtl_uString       *pPath,
    rtl_uString       *pName,
    storeAccessMode    eMode)
{
    NAMESPACE_VOS(ORef)<OStorePageManager> xManager (pManager);
    if (!xManager.isValid())
        return store_E_InvalidAccess;

    if (!(pPath && pName))
        return store_E_InvalidParameter;

    NAMESPACE_VOS(OGuard) aGuard (*xManager);
    storeError eErrCode = xManager->getPageSize (m_nPageSize);
    if (eErrCode != store_E_None)
        return eErrCode;

    delete m_pNode;
    m_pNode = new(m_nPageSize) inode(m_nPageSize);
    if (!m_pNode)
        return store_E_OutOfMemory;

    OString aPath (pPath->buffer, pPath->length, RTL_TEXTENCODING_UTF8);
    OString aName (pName->buffer, pName->length, RTL_TEXTENCODING_UTF8);

    eErrCode = __store_iget (
        *xManager, *m_pNode, STORE_ATTRIB_ISFILE,
        aPath.pData, aName.pData, eMode);
    if (eErrCode != store_E_None)
        return eErrCode;

    sal_uInt32 nAttrib = m_pNode->m_aNameBlock.m_nAttrib;
    if (!(nAttrib & STORE_ATTRIB_ISFILE))
    {
        // No ISFILE in older versions (backward compatibility).
        if (nAttrib & STORE_ATTRIB_ISLINK)
            return store_E_NotFile;
    }

    // ...
    OStorePageDescriptor aDescr (m_pNode->m_aDescr);
    if (eMode != store_AccessReadOnly)
        eErrCode = xManager->acquirePage (aDescr, store_AccessReadWrite);
    else
        eErrCode = xManager->acquirePage (aDescr, store_AccessReadOnly);
    if (eErrCode != store_E_None)
        return eErrCode;

    // ...
    m_xManager   = xManager;
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

    if (!m_xManager.isValid())
        return store_E_InvalidAccess;

    if (!pBuffer)
        return store_E_InvalidParameter;
    if (!nBytes)
        return store_E_None;

    // Acquire exclusive access.
    NAMESPACE_VOS(OGuard) aGuard (*m_xManager);

    // Determine data length.
    OStoreDirectoryPageObject aPage (*m_pNode);

    sal_uInt32 nDataLen = aPage.dataLength();
    if ((nOffset + nBytes) > nDataLen)
        nBytes = nDataLen - nOffset;

    // Read data.
    sal_uInt8 *pData = (sal_uInt8*)pBuffer;
    while ((0 < nBytes) && (nOffset < nDataLen))
    {
        // Determine 'Offset' scope.
        inode::ChunkScope eScope = m_pNode->scope (nOffset);
        if (eScope == inode::SCOPE_INTERNAL)
        {
            // Read from inode page (internal scope).
            inode::ChunkDescriptor aDescr (
                nOffset, m_pNode->capacity());
            sal_uInt32 nLength = VOS_MIN (aDescr.m_nLength, nBytes);

            rtl_copyMemory (
                &pData[rnDone],
                &m_pNode->m_pData[aDescr.m_nOffset],
                nLength);

            // Adjust counters.
            rnDone  += nLength;
            nOffset += nLength;
            nBytes  -= nLength;
        }
        else
        {
            // Read from data page (external scope).
            if (!m_pData)
                m_pData = new(m_nPageSize) data(m_nPageSize);
            if (!m_pData)
                return store_E_OutOfMemory;
            OStoreDataPageObject aData (*m_pData);

            inode::ChunkDescriptor aDescr (
                nOffset - m_pNode->capacity(), m_pData->capacity());
            sal_uInt32 nLength = VOS_MIN (aDescr.m_nLength, nBytes);

            storeError eErrCode = aPage.get (
                aDescr.m_nPage, m_pSingle, m_pDouble, m_pTriple,
                aData, *m_xManager, NULL);
            if (eErrCode != store_E_None)
            {
                if (eErrCode != store_E_NotExists)
                    return eErrCode;

                rtl_zeroMemory (
                    &pData[rnDone],
                    nLength);
            }
            else
            {
                rtl_copyMemory (
                    &pData[rnDone],
                    &m_pData->m_pData[aDescr.m_nOffset],
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

    if (!m_xManager.isValid())
        return store_E_InvalidAccess;
    if (!m_bWriteable)
        return store_E_AccessViolation;

    if (!pBuffer)
        return store_E_InvalidParameter;
    if (!nBytes)
        return store_E_None;

    // Acquire exclusive access.
    NAMESPACE_VOS(OGuard) aGuard (*m_xManager);

    // Write data.
    OStoreDirectoryPageObject aPage (*m_pNode);
    const sal_uInt8 *pData = (const sal_uInt8*)pBuffer;

    storeError eErrCode = store_E_None;
    while (nBytes > 0)
    {
        // Determine 'Offset' scope.
        inode::ChunkScope eScope = m_pNode->scope (nOffset);
        if (eScope == inode::SCOPE_INTERNAL)
        {
            // Write to inode page (internal scope).
            inode::ChunkDescriptor aDescr (
                nOffset, m_pNode->capacity());
            sal_uInt32 nLength = VOS_MIN (aDescr.m_nLength, nBytes);

            rtl_copyMemory (
                &m_pNode->m_pData[aDescr.m_nOffset],
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
            if (!m_pData)
                m_pData = new(m_nPageSize) data(m_nPageSize);
            if (!m_pData)
                return store_E_OutOfMemory;
            OStoreDataPageObject aData (*m_pData);

            inode::ChunkDescriptor aDescr (
                nOffset - m_pNode->capacity(), m_pData->capacity());
            sal_uInt32 nLength = aDescr.m_nLength;

            if ((aDescr.m_nOffset > 0) || (nBytes < nLength))
            {
                // Unaligned. Need to load/create data page.
                eErrCode = aPage.get (
                    aDescr.m_nPage, m_pSingle, m_pDouble, m_pTriple,
                    aData, *m_xManager, NULL);
                if (eErrCode != store_E_None)
                {
                    if (eErrCode != store_E_NotExists)
                        return eErrCode;

                    rtl_zeroMemory (
                        &m_pData->m_pData[0],
                        m_pData->capacity());
                }
            }

            // Modify data page.
            nLength = VOS_MIN (nLength, nBytes);

            rtl_copyMemory (
                &m_pData->m_pData[aDescr.m_nOffset],
                &pData[rnDone], nLength);

            // Save data page.
            eErrCode = aPage.put (
                aDescr.m_nPage, m_pSingle, m_pDouble, m_pTriple,
                aData, *m_xManager, NULL);
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
        return m_xManager->save (aPage);
    else
        return store_E_None;
}

/*
 * flush.
 */
storeError OStoreLockBytes::flush (void)
{
    if (!m_xManager.isValid())
        return store_E_InvalidAccess;

    return m_xManager->flush();
}

/*
 * setSize.
 */
storeError OStoreLockBytes::setSize (sal_uInt32 nSize)
{
    if (!m_xManager.isValid())
        return store_E_InvalidAccess;
    if (!m_bWriteable)
        return store_E_AccessViolation;

    // Acquire exclusive access.
    NAMESPACE_VOS(OGuard) aGuard (*m_xManager);

    // Determine current length.
    OStoreDirectoryPageObject aPage (*m_pNode);
    sal_uInt32 nDataLen = aPage.dataLength();

    if (nSize == nDataLen)
        return store_E_None;

    if (nSize < nDataLen)
    {
        // Truncate.
        storeError eErrCode = store_E_None;

        // Determine 'Size' scope.
        inode::ChunkScope eSizeScope = m_pNode->scope (nSize);
        if (eSizeScope == inode::SCOPE_INTERNAL)
        {
            // Internal 'Size' scope. Determine 'Data' scope.
            inode::ChunkScope eDataScope = m_pNode->scope (nDataLen);
            if (eDataScope == inode::SCOPE_EXTERNAL)
            {
                // External 'Data' scope.
                if (!m_pData)
                    m_pData = new(m_nPageSize) data(m_nPageSize);
                if (!m_pData)
                    return store_E_OutOfMemory;
                OStoreDataPageObject aData (*m_pData);

                // Truncate all external data pages.
                eErrCode = aPage.truncate (
                    0, m_pSingle, m_pDouble, m_pTriple,
                    aData, *m_xManager, NULL);
                if (eErrCode != store_E_None)
                    return eErrCode;
            }

            // Truncate internal data page.
            inode::ChunkDescriptor aDescr (nSize, m_pNode->capacity());
            rtl_zeroMemory (
                &m_pNode->m_pData[aDescr.m_nOffset],
                aDescr.m_nLength);
        }
        else
        {
            // External 'Size' scope.
            if (!m_pData)
                m_pData = new(m_nPageSize) data(m_nPageSize);
            if (!m_pData)
                return store_E_OutOfMemory;
            OStoreDataPageObject aData (*m_pData);

            // Truncate external data pages.
            inode::ChunkDescriptor aDescr (
                nSize - m_pNode->capacity(), m_pData->capacity());

            sal_uInt32 nPage = aDescr.m_nPage;
            if (aDescr.m_nOffset) nPage += 1;

            eErrCode = aPage.truncate (
                nPage, m_pSingle, m_pDouble, m_pTriple,
                aData, *m_xManager, NULL);
            if (eErrCode != store_E_None)
                return eErrCode;
        }
    }

    // Set (extended or truncated) size.
    aPage.dataLength (nSize);

    // Save modified inode.
    return m_xManager->save (aPage);
}

/*
 * stat.
 */
storeError OStoreLockBytes::stat (sal_uInt32 &rnSize)
{
    rnSize = 0;

    if (!m_xManager.isValid())
        return store_E_InvalidAccess;

    rnSize = m_pNode->m_aDataBlock.m_nDataLen;
    return store_E_None;
}

/*
 * lockRange.
 */
storeError OStoreLockBytes::lockRange (
    sal_uInt32 nOffset, sal_uInt32 nBytes)
{
    // (NYI).
    return store_E_None;
}

/*
 * unlockRange.
 */
storeError OStoreLockBytes::unlockRange (
    sal_uInt32 nOffset, sal_uInt32 nBytes)
{
    // (NYI).
    return store_E_None;
}

