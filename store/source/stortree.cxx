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

#include <sal/config.h>

#include <memory>
#include <string.h>

#include "stortree.hxx"

#include <sal/types.h>
#include <sal/log.hxx>
#include <osl/diagnose.h>

#include <store/types.h>

#include "storbase.hxx"
#include "storbios.hxx"

using namespace store;

/*========================================================================
 *
 * OStoreBTreeNodeData implementation.
 *
 *======================================================================*/
/*
 * OStoreBTreeNodeData.
 */
OStoreBTreeNodeData::OStoreBTreeNodeData (sal_uInt16 nPageSize)
    : PageData (nPageSize)
{
    base::m_aGuard.m_nMagic = store::htonl(self::theTypeId);
    base::m_aDescr.m_nUsed  = store::htons(self::thePageSize); // usageCount(0)
    self::m_aGuard.m_nMagic = store::htonl(0); // depth(0)

    sal_uInt16 const n = capacityCount();
    T const          t;

    for (sal_uInt16 i = 1; i < n; i++)
    {
        // cppcheck-suppress arrayIndexOutOfBounds
        m_pData[i] = t;
    }
}

/*
 * find.
 */
sal_uInt16 OStoreBTreeNodeData::find (const T& t) const
{
    sal_Int32 l = 0;
    sal_Int32 r = usageCount() - 1;

    while (l < r)
    {
        sal_Int32 const m = ((l + r) >> 1);

        if (t.m_aKey == m_pData[m].m_aKey)
            return static_cast<sal_uInt16>(m);
        if (t.m_aKey < m_pData[m].m_aKey)
            r = m - 1;
        else
            l = m + 1;
    }

    sal_uInt16 const k = static_cast<sal_uInt16>(r);
    if ((k < capacityCount()) && (t.m_aKey < m_pData[k].m_aKey))
        return k - 1;
    else
        return k;
}

/*
 * insert.
 */
void OStoreBTreeNodeData::insert (sal_uInt16 i, const T& t)
{
    sal_uInt16 const n = usageCount();
    sal_uInt16 const m = capacityCount();
    if ((n < m) && (i < m))
    {
        // shift right.
        memmove (&(m_pData[i + 1]), &(m_pData[i]), (n - i) * sizeof(T));

        // insert.
        m_pData[i] = t;
        usageCount (n + 1);
    }
}

/*
 * remove.
 */
void OStoreBTreeNodeData::remove (sal_uInt16 i)
{
    sal_uInt16 const n = usageCount();
    if (i < n)
    {
        // shift left.
        memmove (&(m_pData[i]), &(m_pData[i + 1]), (n - i - 1) * sizeof(T));

        // truncate.
        m_pData[n - 1] = T();
        usageCount (n - 1);
    }
}

/*
 * split (left half copied from right half of left page).
 */
void OStoreBTreeNodeData::split (const self& rPageL)
{
    sal_uInt16 const h = capacityCount() / 2;
    memcpy (&(m_pData[0]), &(rPageL.m_pData[h]), h * sizeof(T));
    truncate (h);
}

/*
 * truncate.
 */
void OStoreBTreeNodeData::truncate (sal_uInt16 n)
{
    sal_uInt16 const m = capacityCount();
    T const          t;

    for (sal_uInt16 i = n; i < m; i++)
        m_pData[i] = t;
    usageCount (n);
}

/*========================================================================
 *
 * OStoreBTreeNodeObject implementation.
 *
 *======================================================================*/
/*
 * guard.
 */
storeError OStoreBTreeNodeObject::guard (sal_uInt32 nAddr)
{
    return PageHolderObject< page >::guard (m_xPage, nAddr);
}

/*
 * verify.
 */
storeError OStoreBTreeNodeObject::verify (sal_uInt32 nAddr) const
{
    return PageHolderObject< page >::verify (m_xPage, nAddr);
}

/*
 * split.
 */
storeError OStoreBTreeNodeObject::split (
    sal_uInt16                 nIndexL,
    PageHolderObject< page > & rxPageL,
    OStorePageBIOS           & rBIOS)
{
    PageHolderObject< page > xPage (m_xPage);
    if (!xPage.is())
        return store_E_InvalidAccess;

    // Check left page usage.
    if (!rxPageL.is())
        return store_E_InvalidAccess;
    if (!rxPageL->querySplit())
        return store_E_None;

    // Construct right page.
    PageHolderObject< page > xPageR;
    if (!xPageR.construct (rBIOS.allocator()))
        return store_E_OutOfMemory;

    // Split right page off left page.
    xPageR->split (*rxPageL);
    xPageR->depth (rxPageL->depth());

    // Allocate right page.
    self aNodeR (xPageR.get());
    storeError eErrCode = rBIOS.allocate (aNodeR);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Truncate left page.
    rxPageL->truncate (rxPageL->capacityCount() / 2);

    // Save left page.
    self aNodeL (rxPageL.get());
    eErrCode = rBIOS.saveObjectAt (aNodeL, aNodeL.location());
    if (eErrCode != store_E_None)
        return eErrCode;

    // Insert right page.
    OStorePageLink aLink (xPageR->location());
    xPage->insert (nIndexL + 1, T(xPageR->m_pData[0].m_aKey, aLink));

    // Save this page and leave.
    return rBIOS.saveObjectAt (*this, location());
}

/*
 * remove (down to leaf node, recursive).
 */
storeError OStoreBTreeNodeObject::remove (
    sal_uInt16         nIndexL,
    OStoreBTreeEntry & rEntryL,
    OStorePageBIOS &   rBIOS)
{
    PageHolderObject< page > xImpl (m_xPage);
    page & rPage = *xImpl;

    // Check depth.
    storeError eErrCode = store_E_None;
    if (rPage.depth())
    {
        // Check link entry.
        T const aEntryL (rPage.m_pData[nIndexL]);
        if (rEntryL.compare (aEntryL) != T::COMPARE_EQUAL)
            return store_E_InvalidAccess;

        // Load link node.
        self aNodeL;
        eErrCode = rBIOS.loadObjectAt (aNodeL, aEntryL.m_aLink.location());
        if (eErrCode != store_E_None)
            return eErrCode;

        // Recurse: remove from link node.
        eErrCode = aNodeL.remove (0, rEntryL, rBIOS);
        if (eErrCode != store_E_None)
            return eErrCode;

        // Check resulting link node usage.
        PageHolderObject< page > xPageL (aNodeL.get());
        if (xPageL->usageCount() == 0)
        {
            // Free empty link node.
            eErrCode = rBIOS.free (xPageL->location());
            if (eErrCode != store_E_None)
                return eErrCode;

            // Remove index.
            rPage.remove (nIndexL);
            touch();
        }
        else
        {

            // Relink.
            rPage.m_pData[nIndexL].m_aKey = xPageL->m_pData[0].m_aKey;
            touch();
        }
    }
    else
    {
        // Check leaf entry.
        if (rEntryL.compare (rPage.m_pData[nIndexL]) != T::COMPARE_EQUAL)
            return store_E_NotExists;

        // Save leaf entry.
        rEntryL = rPage.m_pData[nIndexL];

        // Remove leaf index.
        rPage.remove (nIndexL);
        touch();
    }

    // Check for modified node.
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
 * OStoreBTreeRootObject implementation.
 *
 *======================================================================*/
/*
 * testInvariant.
 * Precond: root node page loaded.
 */
void OStoreBTreeRootObject::testInvariant (char const * message) const
{
    OSL_PRECOND(m_xPage != nullptr, "OStoreBTreeRootObject::testInvariant(): Null pointer");
    SAL_WARN_IF( (m_xPage->location() - m_xPage->size()) != 0, "store", message);
}

/*
 * loadOrCreate.
 */
storeError OStoreBTreeRootObject::loadOrCreate (
    sal_uInt32       nAddr,
    OStorePageBIOS & rBIOS)
{
    storeError eErrCode = rBIOS.loadObjectAt (*this, nAddr);
    if (eErrCode != store_E_NotExists)
        return eErrCode;

    eErrCode = construct<page>(rBIOS.allocator());
    if (eErrCode != store_E_None)
        return eErrCode;

    eErrCode = rBIOS.allocate (*this);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Notify caller of the creation.
    testInvariant("OStoreBTreeRootObject::loadOrCreate(): leave");
    return store_E_Pending;
}

/*
 * change.
 */
storeError OStoreBTreeRootObject::change (
    PageHolderObject< page > & rxPageL,
    OStorePageBIOS &           rBIOS)
{
    PageHolderObject< page > xPage (m_xPage);
    testInvariant("OStoreBTreeRootObject::change(): enter");

    // Save root location.
    sal_uInt32 const nRootAddr = xPage->location();

    // Construct new root.
    if (!rxPageL.construct (rBIOS.allocator()))
        return store_E_OutOfMemory;

    // Save this as prev root.
    storeError eErrCode = rBIOS.allocate (*this);
    if (eErrCode != store_E_None)
        return store_E_OutOfMemory;

    // Setup new root.
    rxPageL->depth (xPage->depth() + 1);
    rxPageL->m_pData[0] = xPage->m_pData[0];
    rxPageL->m_pData[0].m_aLink = xPage->location();
    rxPageL->usageCount(1);

    // Change root.
    rxPageL.swap (xPage);
    {
        std::shared_ptr<PageData> tmp (xPage.get());
        tmp.swap (m_xPage);
    }

    // Save this as new root and finish.
    eErrCode = rBIOS.saveObjectAt (*this, nRootAddr);
    testInvariant("OStoreBTreeRootObject::change(): leave");
    return eErrCode;
}

/*
 * find_lookup (w/o split()).
 * Precond: root node page loaded.
 */
storeError OStoreBTreeRootObject::find_lookup (
    OStoreBTreeNodeObject & rNode,  // [out]
    sal_uInt16 &            rIndex, // [out]
    OStorePageKey const &   rKey,
    OStorePageBIOS &        rBIOS) const
{
    // Init node w/ root page.
    testInvariant("OStoreBTreeRootObject::find_lookup(): enter");
    {
        std::shared_ptr<PageData> tmp (m_xPage);
        tmp.swap (rNode.get());
    }

    // Setup BTree entry.
    T const entry (rKey);

    // Check current page.
    PageHolderObject< page > xPage (rNode.get());
    for (; xPage->depth() > 0; xPage = rNode.makeHolder< page >())
    {
        // Find next page.
        page const & rPage = *xPage;
        sal_uInt16 const i = rPage.find(entry);
        sal_uInt16 const n = rPage.usageCount();
        if (i >= n)
        {
            // Path to entry not exists (Must not happen(?)).
            return store_E_NotExists;
        }

        // Check address.
        sal_uInt32 const nAddr = rPage.m_pData[i].m_aLink.location();
        if (nAddr == STORE_PAGE_NULL)
        {
            // Path to entry not exists (Must not happen(?)).
            return store_E_NotExists;
        }

        // Load next page.
        storeError eErrCode = rBIOS.loadObjectAt (rNode, nAddr);
        if (eErrCode != store_E_None)
            return eErrCode;
    }

    // Find index.
    page const & rPage = *xPage;
    rIndex = rPage.find(entry);
    if (rIndex >= rPage.usageCount())
        return store_E_NotExists;

    // Compare entry.
    T::CompareResult eResult = entry.compare(rPage.m_pData[rIndex]);
    if (eResult == T::COMPARE_LESS)
    {
        SAL_WARN("store", "store::BTreeRoot::find_lookup(): sort error");
        return store_E_Unknown;
    }

    // Greater or Equal.
    testInvariant("OStoreBTreeRootObject::find_lookup(): leave");
    return store_E_None;
}

/*
 * find_insert (possibly with split()).
 * Precond: root node page loaded.
 */
storeError OStoreBTreeRootObject::find_insert (
    OStoreBTreeNodeObject & rNode,  // [out]
    sal_uInt16 &            rIndex, // [out]
    OStorePageKey const &   rKey,
    OStorePageBIOS &        rBIOS)
{
    testInvariant("OStoreBTreeRootObject::find_insert(): enter");

    // Check for RootNode split.
    PageHolderObject< page > xRoot (m_xPage);
    if (xRoot->querySplit())
    {
        PageHolderObject< page > xPageL;

        // Change root.
        storeError eErrCode = change (xPageL, rBIOS);
        if (eErrCode != store_E_None)
            return eErrCode;

        // Split left page (prev root).
        eErrCode = split (0, xPageL, rBIOS);
        if (eErrCode != store_E_None)
            return eErrCode;
    }

    // Init node w/ root page.
    {
        std::shared_ptr<PageData> tmp (m_xPage);
        tmp.swap (rNode.get());
    }

    // Setup BTree entry.
    T const entry (rKey);

    // Check current Page.
    PageHolderObject< page > xPage (rNode.get());
    for (; xPage->depth() > 0; xPage = rNode.makeHolder< page >())
    {
        // Find next page.
        page const & rPage = *xPage;
        sal_uInt16 const i = rPage.find (entry);
        sal_uInt16 const n = rPage.usageCount();
        if (i >= n)
        {
            // Path to entry not exists (Must not happen(?)).
            return store_E_NotExists;
        }

        // Check address.
        sal_uInt32 const nAddr = rPage.m_pData[i].m_aLink.location();
        if (nAddr == STORE_PAGE_NULL)
        {
            // Path to entry not exists (Must not happen(?)).
            return store_E_NotExists;
        }

        // Load next page.
        OStoreBTreeNodeObject aNext;
        storeError eErrCode = rBIOS.loadObjectAt (aNext, nAddr);
        if (eErrCode != store_E_None)
            return eErrCode;

        // Check for next node split.
        PageHolderObject< page > xNext (aNext.get());
        if (xNext->querySplit())
        {
            // Split next node.
            eErrCode = rNode.split (i, xNext, rBIOS);
            if (eErrCode != store_E_None)
                return eErrCode;

            // Restart.
            continue;
        }

        // Let next page be current.
        std::shared_ptr<PageData> tmp (aNext.get());
        tmp.swap (rNode.get());
    }

    // Find index.
    page const & rPage = *xPage;
    rIndex = rPage.find(entry);
    if (rIndex < rPage.usageCount())
    {
        // Compare entry.
        T::CompareResult result = entry.compare (rPage.m_pData[rIndex]);
        if (result == T::COMPARE_LESS)
        {
            SAL_WARN("store", "store::BTreeRoot::find_insert(): sort error");
            return store_E_Unknown;
        }

        if (result == T::COMPARE_EQUAL)
            return store_E_AlreadyExists;
    }

    // Greater or not (yet) existing.
    testInvariant("OStoreBTreeRootObject::find_insert(): leave");
    return store_E_None;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
