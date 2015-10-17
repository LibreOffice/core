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

#include "stordata.hxx"

#include <sal/types.h>
#include <sal/log.hxx>
#include <osl/diagnose.h>

#include "store/types.h"
#include "storbase.hxx"
#include "storbios.hxx"

using namespace store;

/*========================================================================
 *
 * OStoreDataPageObject implementation.
 *
 *======================================================================*/
/*
 * guard.
 */
storeError OStoreDataPageObject::guard (sal_uInt32 nAddr)
{
    return PageHolderObject< page >::guard (m_xPage, nAddr);
}

/*
 * verify.
 */
storeError OStoreDataPageObject::verify (sal_uInt32 nAddr) const
{
    return PageHolderObject< page >::verify (m_xPage, nAddr);
}

/*========================================================================
 *
 * OStoreIndirectionPageObject implementation.
 *
 *======================================================================*/
/*
  * store_truncate_Impl (single indirect page).
  */
static storeError store_truncate_Impl (
    sal_uInt32      nAddr,
    sal_uInt16      nSingle,
    OStorePageBIOS &rBIOS)
{
    if (nAddr != STORE_PAGE_NULL)
    {
        // Load single indirect page.
        OStoreIndirectionPageObject aSingle;
        storeError eErrCode = rBIOS.loadObjectAt (aSingle, nAddr);
        if (eErrCode == store_E_None)
        {
            // Truncate to 'nSingle' direct pages.
            eErrCode = aSingle.truncate (nSingle, rBIOS);
            if (eErrCode != store_E_None)
                return eErrCode;
        }
        else
        {
            if (eErrCode != store_E_InvalidChecksum)
                return eErrCode;
        }

        // Check for complete truncation.
        if (nSingle == 0)
        {
            // Free single indirect page.
            eErrCode = rBIOS.free (nAddr);
            if (eErrCode != store_E_None)
                return eErrCode;
        }
    }
    return store_E_None;
}

/*
 * store_truncate_Impl (double indirect page).
 */
static storeError store_truncate_Impl (
    sal_uInt32       nAddr,
    sal_uInt16       nDouble,
    sal_uInt16       nSingle,
    OStorePageBIOS  &rBIOS)
{
    if (nAddr != STORE_PAGE_NULL)
    {
        // Load double indirect page.
        OStoreIndirectionPageObject aDouble;
        storeError eErrCode = rBIOS.loadObjectAt (aDouble, nAddr);
        if (eErrCode == store_E_None)
        {
            // Truncate to 'nDouble', 'nSingle' pages.
            eErrCode = aDouble.truncate (nDouble, nSingle, rBIOS);
            if (eErrCode != store_E_None)
                return eErrCode;
        }
        else
        {
            if (eErrCode != store_E_InvalidChecksum)
                return eErrCode;
        }

        // Check for complete truncation.
        if ((nDouble + nSingle) == 0)
        {
            // Free double indirect page.
            eErrCode = rBIOS.free (nAddr);
            if (eErrCode != store_E_None)
                return eErrCode;
        }
    }
    return store_E_None;
}

/*
 * store_truncate_Impl (triple indirect page).
 */
static storeError store_truncate_Impl (
    sal_uInt32       nAddr,
    sal_uInt16       nTriple,
    sal_uInt16       nDouble,
    sal_uInt16       nSingle,
    OStorePageBIOS  &rBIOS)
{
    if (nAddr != STORE_PAGE_NULL)
    {
        // Load triple indirect page.
        OStoreIndirectionPageObject aTriple;
        storeError eErrCode = rBIOS.loadObjectAt (aTriple, nAddr);
        if (eErrCode != store_E_None)
            return eErrCode;

        // Truncate to 'nTriple', 'nDouble', 'nSingle' pages.
        eErrCode = aTriple.truncate (nTriple, nDouble, nSingle, rBIOS);
        if (eErrCode != store_E_None)
            return eErrCode;

        // Check for complete truncation.
        if ((nTriple + nDouble + nSingle) == 0)
        {
            // Free triple indirect page.
            eErrCode = rBIOS.free (nAddr);
            if (eErrCode != store_E_None)
                return eErrCode;
        }
    }
    return store_E_None;
}

/*
 * loadOrCreate.
 */
storeError OStoreIndirectionPageObject::loadOrCreate (
    sal_uInt32       nAddr,
    OStorePageBIOS & rBIOS)
{
    if (nAddr == STORE_PAGE_NULL)
    {
        storeError eErrCode = construct<page>(rBIOS.allocator());
        if (eErrCode != store_E_None)
            return eErrCode;

        eErrCode = rBIOS.allocate (*this);
        if (eErrCode != store_E_None)
            return eErrCode;

        // Save location pending at caller.
        return store_E_Pending;
    }
    return rBIOS.loadObjectAt (*this, nAddr);
}

/*
 * guard.
 */
storeError OStoreIndirectionPageObject::guard (sal_uInt32 nAddr)
{
    return PageHolderObject< page >::guard (m_xPage, nAddr);
}

/*
 * verify.
 */
storeError OStoreIndirectionPageObject::verify (sal_uInt32 nAddr) const
{
    return PageHolderObject< page >::verify (m_xPage, nAddr);
}

/*
 * read (single indirect).
 */
storeError OStoreIndirectionPageObject::read (
    sal_uInt16             nSingle,
    OStoreDataPageObject  &rData,
    OStorePageBIOS        &rBIOS)
{
    PageHolderObject< page > xImpl (m_xPage);
    page const & rPage = (*xImpl);

    // Check arguments.
    sal_uInt16 const nLimit = rPage.capacityCount();
    if (!(nSingle < nLimit))
        return store_E_InvalidAccess;

    // Obtain data page location.
    sal_uInt32 const nAddr = store::ntohl(rPage.m_pData[nSingle]);
    if (nAddr == STORE_PAGE_NULL)
        return store_E_NotExists;

    // Load data page and leave.
    return rBIOS.loadObjectAt (rData, nAddr);
}

/*
 * read (double indirect).
 */
storeError OStoreIndirectionPageObject::read (
    sal_uInt16             nDouble,
    sal_uInt16             nSingle,
    OStoreDataPageObject  &rData,
    OStorePageBIOS        &rBIOS)
{
    PageHolderObject< page > xImpl (m_xPage);
    page const & rPage = (*xImpl);

    // Check arguments.
    sal_uInt16 const nLimit = rPage.capacityCount();
    if (!((nDouble < nLimit) && (nSingle < nLimit)))
        return store_E_InvalidAccess;

    // Check single indirect page location.
    sal_uInt32 const nAddr = store::ntohl(rPage.m_pData[nDouble]);
    if (nAddr == STORE_PAGE_NULL)
        return store_E_NotExists;

    // Load single indirect page.
    OStoreIndirectionPageObject aSingle;
    storeError eErrCode = rBIOS.loadObjectAt (aSingle, nAddr);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Read single indirect and leave.
    return aSingle.read (nSingle, rData, rBIOS);
}

/*
 * read (triple indirect).
 */
storeError OStoreIndirectionPageObject::read (
    sal_uInt16             nTriple,
    sal_uInt16             nDouble,
    sal_uInt16             nSingle,
    OStoreDataPageObject  &rData,
    OStorePageBIOS        &rBIOS)
{
    PageHolderObject< page > xImpl (m_xPage);
    page const & rPage = (*xImpl);

    // Check arguments.
    sal_uInt16 const nLimit = rPage.capacityCount();
    if (!((nTriple < nLimit) && (nDouble < nLimit) && (nSingle < nLimit)))
        return store_E_InvalidAccess;

    // Check double indirect page location.
    sal_uInt32 const nAddr = store::ntohl(rPage.m_pData[nTriple]);
    if (nAddr == STORE_PAGE_NULL)
        return store_E_NotExists;

    // Load double indirect page.
    OStoreIndirectionPageObject aDouble;
    storeError eErrCode = rBIOS.loadObjectAt (aDouble, nAddr);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Read double indirect and leave.
    return aDouble.read (nDouble, nSingle, rData, rBIOS);
}

/*
 * write (single indirect).
 */
storeError OStoreIndirectionPageObject::write (
    sal_uInt16             nSingle,
    OStoreDataPageObject  &rData,
    OStorePageBIOS        &rBIOS)
{
    PageHolderObject< page > xImpl (m_xPage);
    page & rPage = (*xImpl);

    // Check arguments.
    sal_uInt16 const nLimit = rPage.capacityCount();
    if (!(nSingle < nLimit))
        return store_E_InvalidAccess;

    // Obtain data page location.
    sal_uInt32 const nAddr = store::ntohl(rPage.m_pData[nSingle]);
    if (nAddr == STORE_PAGE_NULL)
    {
        // Allocate data page.
        storeError eErrCode = rBIOS.allocate (rData);
        if (eErrCode != store_E_None)
            return eErrCode;

        // Store data page location.
        rPage.m_pData[nSingle] = store::htonl(rData.location());

        // Save this page.
        return rBIOS.saveObjectAt (*this, location());
    }
    else
    {
        // Save data page.
        return rBIOS.saveObjectAt (rData, nAddr);
    }
}

/*
 * write (double indirect).
 */
storeError OStoreIndirectionPageObject::write (
    sal_uInt16             nDouble,
    sal_uInt16             nSingle,
    OStoreDataPageObject  &rData,
    OStorePageBIOS        &rBIOS)
{
    PageHolderObject< page > xImpl (m_xPage);
    page & rPage = (*xImpl);

    // Check arguments.
    sal_uInt16 const nLimit = rPage.capacityCount();
    if (!((nDouble < nLimit) && (nSingle < nLimit)))
        return store_E_InvalidAccess;

    // Load or create single indirect page.
    OStoreIndirectionPageObject aSingle;
    storeError eErrCode = aSingle.loadOrCreate (store::ntohl(rPage.m_pData[nDouble]), rBIOS);
    if (eErrCode != store_E_None)
    {
        if (eErrCode != store_E_Pending)
            return eErrCode;
        rPage.m_pData[nDouble] = store::htonl(aSingle.location());

        eErrCode = rBIOS.saveObjectAt (*this, location());
        if (eErrCode != store_E_None)
            return eErrCode;
    }

    // Write single indirect and leave.
    return aSingle.write (nSingle, rData, rBIOS);
}

/*
 * write (triple indirect).
 */
storeError OStoreIndirectionPageObject::write (
    sal_uInt16             nTriple,
    sal_uInt16             nDouble,
    sal_uInt16             nSingle,
    OStoreDataPageObject  &rData,
    OStorePageBIOS        &rBIOS)
{
    PageHolderObject< page > xImpl (m_xPage);
    page & rPage = (*xImpl);

    // Check arguments.
    sal_uInt16 const nLimit = rPage.capacityCount();
    if (!((nTriple < nLimit) && (nDouble < nLimit) && (nSingle < nLimit)))
        return store_E_InvalidAccess;

    // Load or create double indirect page.
    OStoreIndirectionPageObject aDouble;
    storeError eErrCode = aDouble.loadOrCreate (store::ntohl(rPage.m_pData[nTriple]), rBIOS);
    if (eErrCode != store_E_None)
    {
        if (eErrCode != store_E_Pending)
            return eErrCode;
        rPage.m_pData[nTriple] = store::htonl(aDouble.location());

        eErrCode = rBIOS.saveObjectAt (*this, location());
        if (eErrCode != store_E_None)
            return eErrCode;
    }

    // Write double indirect and leave.
    return aDouble.write (nDouble, nSingle, rData, rBIOS);
}

/*
 * truncate (single indirect).
 */
storeError OStoreIndirectionPageObject::truncate (
    sal_uInt16       nSingle,
    OStorePageBIOS & rBIOS)
{
    PageHolderObject< page > xImpl (m_xPage);
    page & rPage = (*xImpl);

    // Check arguments.
    sal_uInt16 const nLimit = rPage.capacityCount();
    if (!(nSingle < nLimit))
        return store_E_InvalidAccess;

    // Truncate.
    storeError eErrCode = store_E_None;
    for (sal_uInt16 i = nLimit; i > nSingle; i--)
    {
        // Obtain data page location.
        sal_uInt32 const nAddr = store::ntohl(rPage.m_pData[i - 1]);
        if (nAddr != STORE_PAGE_NULL)
        {
            // Free data page.
            eErrCode = rBIOS.free (nAddr);
            if (eErrCode != store_E_None)
                return eErrCode;

            // Clear pointer to data page.
            rPage.m_pData[i - 1] = STORE_PAGE_NULL;
            touch();
        }
    }

    // Check for modified page.
    if (dirty())
    {
        // Save this page.
        eErrCode = rBIOS.saveObjectAt (*this, location());
    }

    // Done.
    return eErrCode;
}

/*
 * truncate (double indirect).
 */
storeError OStoreIndirectionPageObject::truncate (
    sal_uInt16             nDouble,
    sal_uInt16             nSingle,
    OStorePageBIOS        &rBIOS)
{
    PageHolderObject< page > xImpl (m_xPage);
    page & rPage = (*xImpl);

    // Check arguments.
    sal_uInt16 const nLimit = rPage.capacityCount();
    if (!((nDouble < nLimit) && (nSingle < nLimit)))
        return store_E_InvalidAccess;

    // Truncate.
    storeError eErrCode = store_E_None;
    for (sal_uInt16 i = nLimit; i > nDouble + 1; i--)
    {
        // Truncate single indirect page to zero direct pages.
        eErrCode = store_truncate_Impl (store::ntohl(rPage.m_pData[i - 1]), 0, rBIOS);
        if (eErrCode != store_E_None)
            return eErrCode;

        // Clear pointer to single indirect page.
        rPage.m_pData[i - 1] = STORE_PAGE_NULL;
        touch();
    }

    // Truncate last single indirect page to 'nSingle' direct pages.
    eErrCode = store_truncate_Impl (store::ntohl(rPage.m_pData[nDouble]), nSingle, rBIOS);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Check for complete truncation.
    if (nSingle == 0)
    {
        // Clear pointer to last single indirect page.
        rPage.m_pData[nDouble] = STORE_PAGE_NULL;
        touch();
    }

    // Check for modified page.
    if (dirty())
    {
        // Save this page.
        eErrCode = rBIOS.saveObjectAt (*this, location());
    }

    // Done.
    return eErrCode;
}

/*
 * truncate (triple indirect).
 */
storeError OStoreIndirectionPageObject::truncate (
    sal_uInt16             nTriple,
    sal_uInt16             nDouble,
    sal_uInt16             nSingle,
    OStorePageBIOS        &rBIOS)
{
    PageHolderObject< page > xImpl (m_xPage);
    page & rPage = (*xImpl);

    // Check arguments.
    sal_uInt16 const nLimit = rPage.capacityCount();
    if (!((nTriple < nLimit) && (nDouble < nLimit) && (nSingle < nLimit)))
        return store_E_InvalidAccess;

    // Truncate.
    storeError eErrCode = store_E_None;
    for (sal_uInt16 i = nLimit; i > nTriple + 1; i--)
    {
        // Truncate double indirect page to zero single indirect pages.
        eErrCode = store_truncate_Impl (store::ntohl(rPage.m_pData[i - 1]), 0, 0, rBIOS);
        if (eErrCode != store_E_None)
            return eErrCode;

        // Clear pointer to double indirect page.
        rPage.m_pData[i - 1] = STORE_PAGE_NULL;
        touch();
    }

    // Truncate last double indirect page to 'nDouble', 'nSingle' pages.
    eErrCode = store_truncate_Impl (store::ntohl(rPage.m_pData[nTriple]), nDouble, nSingle, rBIOS);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Check for complete truncation.
    if ((nDouble + nSingle) == 0)
    {
        // Clear pointer to last double indirect page.
        rPage.m_pData[nTriple] = STORE_PAGE_NULL;
        touch();
    }

    // Check for modified page.
    if (dirty())
    {
        // Save this page.
        eErrCode = rBIOS.saveObjectAt (*this, location());
    }

    // Done.
    return eErrCode;
}

/*========================================================================
 *
 * OStoreDirectoryPageObject implementation.
 *
 *======================================================================*/
/*
 * guard.
 */
storeError OStoreDirectoryPageObject::guard (sal_uInt32 nAddr)
{
    return PageHolderObject< page >::guard (m_xPage, nAddr);
}

/*
 * verify.
 */
storeError OStoreDirectoryPageObject::verify (sal_uInt32 nAddr) const
{
    return PageHolderObject< page >::verify (m_xPage, nAddr);
    // OLD: m_rPage.verifyVersion (STORE_MAGIC_DIRECTORYPAGE);
}

/*
 * scope (external data page; private).
 */
OStoreDirectoryPageData::ChunkScope
OStoreDirectoryPageObject::scope (
    sal_uInt32                       nPage,
    page::DataBlock::LinkDescriptor &rDescr) const
{
    page const & rPage = PAGE();

    sal_uInt32 index0, index1, index2, index3;

    // direct.
    sal_uInt32 nCount = OStoreDirectoryDataBlock::directCount;
    sal_uInt32 nLimit = nCount;
    if (nPage < nLimit)
    {
        // Page to index reduction.
        index0 = nPage;

        // Setup LinkDescriptor indices.
        rDescr.m_nIndex0 = (sal_uInt16)(index0 & 0xffff);

        // Done.
        return page::SCOPE_DIRECT;
    }
    nPage -= nLimit;

    // single indirect.
    sal_uInt32 const nCapacity = indirect::capacityCount(rPage.m_aDescr);
    nCount = OStoreDirectoryDataBlock::singleCount;
    nLimit = nCount * nCapacity;
    if (nPage < nLimit)
    {
        // Page to index reduction.
        sal_uInt32 n = nPage;

        // Reduce to single indirect i(1), direct n = i(0).
        index1 = n / nCapacity;
        index0 = n % nCapacity;

        // Verify reduction.
        n = index1 * nCapacity + index0;
        if (n != nPage)
        {
            SAL_WARN("store", "wrong math on indirect indices");
            return page::SCOPE_UNKNOWN;
        }

        // Setup LinkDescriptor indices.
        rDescr.m_nIndex0 = (sal_uInt16)(index0 & 0xffff);
        rDescr.m_nIndex1 = (sal_uInt16)(index1 & 0xffff);

        // Done.
        return page::SCOPE_SINGLE;
    }
    nPage -= nLimit;

    // double indirect.
    nCount = OStoreDirectoryDataBlock::doubleCount;
    nLimit = nCount * nCapacity * nCapacity;
    if (nPage < nLimit)
    {
        // Page to index reduction.
        sal_uInt32 n = nPage;

        // Reduce to double indirect i(2), single indirect n = i(0).
        index2 = n / (nCapacity * nCapacity);
        n      = n % (nCapacity * nCapacity);

        // Reduce to single indirect i(1), direct n = i(0).
        index1 = n / nCapacity;
        index0 = n % nCapacity;

        // Verify reduction.
        n = index2 * nCapacity * nCapacity +
            index1 * nCapacity + index0;
        if (n != nPage)
        {
            SAL_WARN("store", "wrong math on double indirect indices");
            return page::SCOPE_UNKNOWN;
        }

        // Setup LinkDescriptor indices.
        rDescr.m_nIndex0 = (sal_uInt16)(index0 & 0xffff);
        rDescr.m_nIndex1 = (sal_uInt16)(index1 & 0xffff);
        rDescr.m_nIndex2 = (sal_uInt16)(index2 & 0xffff);

        // Done.
        return page::SCOPE_DOUBLE;
    }
    nPage -= nLimit;

    // triple indirect.
    nCount = OStoreDirectoryDataBlock::tripleCount;
    nLimit = nCount * nCapacity * nCapacity * nCapacity;
    if (nPage < nLimit)
    {
        // Page to index reduction.
        sal_uInt32 n = nPage;

        // Reduce to triple indirect i(3), double indirect n.
        index3 = n / (nCapacity * nCapacity * nCapacity);
        n      = n % (nCapacity * nCapacity * nCapacity);

        // Reduce to double indirect i(2), single indirect n.
        index2 = n / (nCapacity * nCapacity);
        n      = n % (nCapacity * nCapacity);

        // Reduce to single indirect i(1), direct n = i(0).
        index1 = n / nCapacity;
        index0 = n % nCapacity;

        // Verify reduction.
        n = index3 * nCapacity * nCapacity * nCapacity +
            index2 * nCapacity * nCapacity +
            index1 * nCapacity + index0;
        if (n != nPage)
        {
            SAL_WARN("store", "wrong math on triple indirect indices");
            return page::SCOPE_UNKNOWN;
        }

        // Setup LinkDescriptor indices.
        rDescr.m_nIndex0 = (sal_uInt16)(index0 & 0xffff);
        rDescr.m_nIndex1 = (sal_uInt16)(index1 & 0xffff);
        rDescr.m_nIndex2 = (sal_uInt16)(index2 & 0xffff);
        rDescr.m_nIndex3 = (sal_uInt16)(index3 & 0xffff);

        // Done.
        return page::SCOPE_TRIPLE;
    }

    // Unreachable (more than triple indirect).
    return page::SCOPE_UNREACHABLE;
}

/*
 * read (external data page).
 */
storeError OStoreDirectoryPageObject::read (
    sal_uInt32             nPage,
    OStoreDataPageObject  &rData,
    OStorePageBIOS        &rBIOS)
{
    // Determine scope and link indices.
    page::DataBlock::LinkDescriptor aLink;
    page::ChunkScope eScope = scope (nPage, aLink);

    storeError eErrCode = store_E_None;
    if (eScope == page::SCOPE_DIRECT)
    {
        sal_uInt32 const nAddr = directLink (aLink.m_nIndex0);
        if (nAddr == STORE_PAGE_NULL)
            return store_E_NotExists;

        eErrCode = rBIOS.loadObjectAt (rData, nAddr);
    }
    else if (eScope == page::SCOPE_SINGLE)
    {
        sal_uInt32 const nAddr = singleLink (aLink.m_nIndex1);
        if (nAddr == STORE_PAGE_NULL)
            return store_E_NotExists;

        OStoreIndirectionPageObject aSingle;
        eErrCode = rBIOS.loadObjectAt (aSingle, nAddr);
        if (eErrCode != store_E_None)
            return eErrCode;

        eErrCode = aSingle.read (aLink.m_nIndex0, rData, rBIOS);
    }
    else if (eScope == page::SCOPE_DOUBLE)
    {
        sal_uInt32 const nAddr = doubleLink (aLink.m_nIndex2);
        if (nAddr == STORE_PAGE_NULL)
            return store_E_NotExists;

        OStoreIndirectionPageObject aDouble;
        eErrCode = rBIOS.loadObjectAt (aDouble, nAddr);
        if (eErrCode != store_E_None)
            return eErrCode;

        eErrCode = aDouble.read (aLink.m_nIndex1, aLink.m_nIndex0, rData, rBIOS);
    }
    else if (eScope == page::SCOPE_TRIPLE)
    {
        sal_uInt32 const nAddr = tripleLink (aLink.m_nIndex3);
        if (nAddr == STORE_PAGE_NULL)
            return store_E_NotExists;

        OStoreIndirectionPageObject aTriple;
        eErrCode = rBIOS.loadObjectAt (aTriple, nAddr);
        if (eErrCode != store_E_None)
            return eErrCode;

        eErrCode = aTriple.read (aLink.m_nIndex2, aLink.m_nIndex1, aLink.m_nIndex0, rData, rBIOS);
    }
    else if (eScope == page::SCOPE_UNREACHABLE)
    {
        // Out of scope.
        eErrCode = store_E_CantSeek;
    }
    else
    {
        // Unknown scope.
        OSL_TRACE("OStoreDirectoryPageObject::get(): scope failed");
        eErrCode = store_E_Unknown;
    }

    // Leave.
    return eErrCode;
}

/*
 * write (external data page).
 */
storeError OStoreDirectoryPageObject::write (
    sal_uInt32             nPage,
    OStoreDataPageObject  &rData,
    OStorePageBIOS        &rBIOS)
{
    // Determine scope and link indices.
    page::DataBlock::LinkDescriptor aLink;
    page::ChunkScope eScope = scope (nPage, aLink);

    storeError eErrCode = store_E_None;
    if (eScope == page::SCOPE_DIRECT)
    {
        sal_uInt32 const nAddr = directLink (aLink.m_nIndex0);
        if (nAddr == STORE_PAGE_NULL)
        {
            // Allocate data page.
            eErrCode = rBIOS.allocate (rData);
            if (eErrCode != store_E_None)
                return eErrCode;

            // Store data page location.
            directLink (aLink.m_nIndex0, rData.location());
        }
        else
        {
            // Save data page.
            eErrCode = rBIOS.saveObjectAt (rData, nAddr);
        }
    }
    else if (eScope == page::SCOPE_SINGLE)
    {
        OStoreIndirectionPageObject aSingle;
        eErrCode = aSingle.loadOrCreate (singleLink (aLink.m_nIndex1), rBIOS);
        if (eErrCode != store_E_None)
        {
            if (eErrCode != store_E_Pending)
                return eErrCode;
            singleLink (aLink.m_nIndex1, aSingle.location());
        }

        eErrCode = aSingle.write (aLink.m_nIndex0, rData, rBIOS);
    }
    else if (eScope == page::SCOPE_DOUBLE)
    {
        OStoreIndirectionPageObject aDouble;
        eErrCode = aDouble.loadOrCreate (doubleLink (aLink.m_nIndex2), rBIOS);
        if (eErrCode != store_E_None)
        {
            if (eErrCode != store_E_Pending)
                return eErrCode;
            doubleLink (aLink.m_nIndex2, aDouble.location());
        }

        eErrCode = aDouble.write (aLink.m_nIndex1, aLink.m_nIndex0, rData, rBIOS);
    }
    else if (eScope == page::SCOPE_TRIPLE)
    {
        OStoreIndirectionPageObject aTriple;
        eErrCode = aTriple.loadOrCreate (tripleLink (aLink.m_nIndex3), rBIOS);
        if (eErrCode != store_E_None)
        {
            if (eErrCode != store_E_Pending)
                return eErrCode;
            tripleLink (aLink.m_nIndex3, aTriple.location());
        }

        eErrCode = aTriple.write (aLink.m_nIndex2, aLink.m_nIndex1, aLink.m_nIndex0, rData, rBIOS);
    }
    else if (eScope == page::SCOPE_UNREACHABLE)
    {
        // Out of scope.
        eErrCode = store_E_CantSeek;
    }
    else
    {
        // Unknown scope.
        OSL_TRACE("OStoreDirectoryPageObject::put(): scope failed");
        eErrCode = store_E_Unknown;
    }

    // Leave.
    return eErrCode;
}

/*
 * truncate (external data page).
 */
storeError OStoreDirectoryPageObject::truncate (
    sal_uInt32             nPage,
    OStorePageBIOS        &rBIOS)
{
    // Determine scope and link indices.
    page::DataBlock::LinkDescriptor aLink;
    page::ChunkScope eScope = scope (nPage, aLink);

    storeError eErrCode = store_E_None;
    if (eScope == page::SCOPE_DIRECT)
    {
        // Truncate all triple indirect pages.
        eErrCode = truncate (page::SCOPE_TRIPLE, 0, rBIOS);
        if (eErrCode != store_E_None)
            return eErrCode;

        // Truncate all double indirect pages.
        eErrCode = truncate (page::SCOPE_DOUBLE, 0, rBIOS);
        if (eErrCode != store_E_None)
            return eErrCode;

        // Truncate all single indirect pages.
        eErrCode = truncate (page::SCOPE_SINGLE, 0, rBIOS);
        if (eErrCode != store_E_None)
            return eErrCode;

        // Truncate direct pages, including 'aLink.m_nIndex0'.
        eErrCode = truncate (eScope, aLink.m_nIndex0, rBIOS);
    }
    else if (eScope == page::SCOPE_SINGLE)
    {
        // Truncate all triple indirect pages.
        eErrCode = truncate (page::SCOPE_TRIPLE, 0, rBIOS);
        if (eErrCode != store_E_None)
            return eErrCode;

        // Truncate all double indirect pages.
        eErrCode = truncate (page::SCOPE_DOUBLE, 0, rBIOS);
        if (eErrCode != store_E_None)
            return eErrCode;

        // Truncate single indirect pages, downto 'aLink.m_nIndex1'.
        eErrCode = truncate (eScope, aLink.m_nIndex1 + 1, rBIOS);
        if (eErrCode != store_E_None)
            return eErrCode;

        // Truncate last single indirect page to ... pages.
        eErrCode = store_truncate_Impl (singleLink (aLink.m_nIndex1), aLink.m_nIndex0, rBIOS);
        if (eErrCode != store_E_None)
            return eErrCode;

        // Check for complete truncation.
        if (aLink.m_nIndex0 == 0)
        {
            // Clear pointer to last single indirect page.
            singleLink (aLink.m_nIndex1, STORE_PAGE_NULL);
        }
    }
    else if (eScope == page::SCOPE_DOUBLE)
    {
        // Truncate all triple indirect pages.
        eErrCode = truncate (page::SCOPE_TRIPLE, 0, rBIOS);
        if (eErrCode != store_E_None)
            return eErrCode;

        // Truncate double indirect pages, downto 'aLink.m_nIndex2'.
        eErrCode = truncate (eScope, aLink.m_nIndex2 + 1, rBIOS);
        if (eErrCode != store_E_None)
            return eErrCode;

        // Truncate last double indirect page to ... pages.
        eErrCode = store_truncate_Impl (
            doubleLink (aLink.m_nIndex2), aLink.m_nIndex1, aLink.m_nIndex0, rBIOS);
        if (eErrCode != store_E_None)
            return eErrCode;

        // Check for complete truncation.
        if ((aLink.m_nIndex1 + aLink.m_nIndex0) == 0)
        {
            // Clear pointer to last double indirect page.
            doubleLink (aLink.m_nIndex2, STORE_PAGE_NULL);
        }
    }
    else if (eScope == page::SCOPE_TRIPLE)
    {
        // Truncate triple indirect pages, downto 'aLink.m_nIndex3'.
        eErrCode = truncate (eScope, aLink.m_nIndex3 + 1, rBIOS);
        if (eErrCode != store_E_None)
            return eErrCode;

        // Truncate last triple indirect page to ... pages.
        eErrCode = store_truncate_Impl (
            tripleLink (aLink.m_nIndex3), aLink.m_nIndex2, aLink.m_nIndex1, aLink.m_nIndex0, rBIOS);
        if (eErrCode != store_E_None)
            return eErrCode;

        // Check for complete truncation.
        if ((aLink.m_nIndex2 + aLink.m_nIndex1 + aLink.m_nIndex0) == 0)
        {
            // Clear pointer to last triple indirect page.
            tripleLink (aLink.m_nIndex3, STORE_PAGE_NULL);
        }
    }
    else if (eScope == page::SCOPE_UNREACHABLE)
    {
        // Out of scope.
        eErrCode = store_E_CantSeek;
    }
    else
    {
        // Unknown scope.
        OSL_TRACE("OStoreDirectoryPageObject::put(): scope failed");
        eErrCode = store_E_Unknown;
    }

    // Leave.
    return eErrCode;
}

/*
 * truncate (external data page scope; private).
 */
storeError OStoreDirectoryPageObject::truncate (
    page::ChunkScope       eScope,
    sal_uInt16             nRemain,
    OStorePageBIOS        &rBIOS)
{
    // Enter.
    storeError eErrCode = store_E_None;
    if (eScope == page::SCOPE_DIRECT)
    {
        // Truncate direct data pages.
        sal_uInt16 i, n = OStoreDirectoryDataBlock::directCount;
        for (i = n; i > nRemain; i--)
        {
            // Obtain data page location.
            sal_uInt32 nAddr = directLink (i - 1);
            if (nAddr == STORE_PAGE_NULL) continue;

            // Free data page.
            eErrCode = rBIOS.free (nAddr);
            if (eErrCode != store_E_None)
                break;

            // Clear pointer to data page.
            directLink (i - 1, STORE_PAGE_NULL);
        }

        // Done.
        return eErrCode;
    }

    if (eScope == page::SCOPE_SINGLE)
    {
        // Truncate single indirect pages.
        sal_uInt16 i, n = OStoreDirectoryDataBlock::singleCount;
        for (i = n; i > nRemain; i--)
        {
            // Truncate single indirect page to zero data pages.
            eErrCode = store_truncate_Impl (singleLink (i - 1), 0, rBIOS);
            if (eErrCode != store_E_None)
                break;

            // Clear pointer to single indirect page.
            singleLink (i - 1, STORE_PAGE_NULL);
        }

        // Done.
        return eErrCode;
    }

    if (eScope == page::SCOPE_DOUBLE)
    {
        // Truncate double indirect pages.
        sal_uInt16 i, n = OStoreDirectoryDataBlock::doubleCount;
        for (i = n; i > nRemain; i--)
        {
            // Truncate double indirect page to zero single indirect pages.
            eErrCode = store_truncate_Impl (doubleLink (i - 1), 0, 0, rBIOS);
            if (eErrCode != store_E_None)
                break;

            // Clear pointer to double indirect page.
            doubleLink (i - 1, STORE_PAGE_NULL);
        }

        // Done.
        return eErrCode;
    }

    if (eScope == page::SCOPE_TRIPLE)
    {
        // Truncate triple indirect pages.
        sal_uInt16 i, n = OStoreDirectoryDataBlock::tripleCount;
        for (i = n; i > nRemain; i--)
        {
            // Truncate to zero double indirect pages.
            eErrCode = store_truncate_Impl (tripleLink (i - 1), 0, 0, 0, rBIOS);
            if (eErrCode != store_E_None)
                break;

            // Clear pointer to triple indirect page.
            tripleLink (i - 1, STORE_PAGE_NULL);
        }

        // Done.
        return eErrCode;
    }

    // Invalid scope.
    return store_E_InvalidAccess;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
