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


#include "stortree.hxx"

#include "sal/types.h"
#include "osl/diagnose.h"

#include "store/types.h"

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
    : OStorePageData (nPageSize)
{
    base::m_aGuard.m_nMagic = store::htonl(self::theTypeId);
    base::m_aDescr.m_nUsed  = store::htons(self::thePageSize); 
    self::m_aGuard.m_nMagic = store::htonl(0); 

    sal_uInt16 const n = capacityCount();
    T const          t;

    for (sal_uInt16 i = 1; i < n; i++)
        m_pData[i] = t;
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
            return ((sal_uInt16)(m));
        if (t.m_aKey < m_pData[m].m_aKey)
            r = m - 1;
        else
            l = m + 1;
    }

    sal_uInt16 const k = ((sal_uInt16)(r));
    if ((k < capacityCount()) && (t.m_aKey < m_pData[k].m_aKey))
        return(k - 1);
    else
        return(k);
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
        
        memmove (&(m_pData[i + 1]), &(m_pData[i]), (n - i) * sizeof(T));

        
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
        
        memmove (&(m_pData[i]), &(m_pData[i + 1]), (n - i - 1) * sizeof(T));

        
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

    
    if (!rxPageL.is())
        return store_E_InvalidAccess;
    if (!rxPageL->querySplit())
        return store_E_None;

    
    PageHolderObject< page > xPageR;
    if (!xPageR.construct (rBIOS.allocator()))
        return store_E_OutOfMemory;

    
    xPageR->split (*rxPageL);
    xPageR->depth (rxPageL->depth());

    
    self aNodeR (xPageR.get());
    storeError eErrCode = rBIOS.allocate (aNodeR);
    if (eErrCode != store_E_None)
        return eErrCode;

    
    rxPageL->truncate (rxPageL->capacityCount() / 2);

    
    self aNodeL (rxPageL.get());
    eErrCode = rBIOS.saveObjectAt (aNodeL, aNodeL.location());
    if (eErrCode != store_E_None)
        return eErrCode;

    
    OStorePageLink aLink (xPageR->location());
    xPage->insert (nIndexL + 1, T(xPageR->m_pData[0].m_aKey, aLink));

    
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
    page & rPage = (*xImpl);

    
    storeError eErrCode = store_E_None;
    if (rPage.depth())
    {
        
        T const aEntryL (rPage.m_pData[nIndexL]);
        if (!(rEntryL.compare (aEntryL) == T::COMPARE_EQUAL))
            return store_E_InvalidAccess;

        
        self aNodeL;
        eErrCode = rBIOS.loadObjectAt (aNodeL, aEntryL.m_aLink.location());
        if (eErrCode != store_E_None)
            return eErrCode;

        
        eErrCode = aNodeL.remove (0, rEntryL, rBIOS);
        if (eErrCode != store_E_None)
            return eErrCode;

        
        PageHolderObject< page > xPageL (aNodeL.get());
        if (xPageL->usageCount() == 0)
        {
            
            eErrCode = rBIOS.free (xPageL->location());
            if (eErrCode != store_E_None)
                return eErrCode;

            
            rPage.remove (nIndexL);
            touch();
        }
        else
        {

            
            rPage.m_pData[nIndexL].m_aKey = xPageL->m_pData[0].m_aKey;
            touch();
        }
    }
    else
    {
        
        if (!(rEntryL.compare (rPage.m_pData[nIndexL]) == T::COMPARE_EQUAL))
            return store_E_NotExists;

        
        rEntryL = rPage.m_pData[nIndexL];

        
        rPage.remove (nIndexL);
        touch();
    }

    
    if (dirty())
    {
        
        eErrCode = rBIOS.saveObjectAt (*this, location());
    }

    
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
bool OStoreBTreeRootObject::testInvariant (char const * message)
{
    OSL_PRECOND(m_xPage.get() != 0, "OStoreBTreeRootObject::testInvariant(): Null pointer");
    bool result = ((m_xPage->location() - m_xPage->size()) == 0);
    OSL_POSTCOND(result, message); (void) message;
    return result;
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

    
    (void) testInvariant("OStoreBTreeRootObject::loadOrCreate(): leave");
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
    (void) testInvariant("OStoreBTreeRootObject::change(): enter");

    
    sal_uInt32 const nRootAddr = xPage->location();

    
    if (!rxPageL.construct (rBIOS.allocator()))
        return store_E_OutOfMemory;

    
    storeError eErrCode = rBIOS.allocate (*this);
    if (eErrCode != store_E_None)
        return store_E_OutOfMemory;

    
    rxPageL->depth (xPage->depth() + 1);
    rxPageL->m_pData[0] = xPage->m_pData[0];
    rxPageL->m_pData[0].m_aLink = xPage->location();
    rxPageL->usageCount(1);

    
    rxPageL.swap (xPage);
    {
        PageHolder tmp (xPage.get());
        tmp.swap (m_xPage);
    }

    
    eErrCode = rBIOS.saveObjectAt (*this, nRootAddr);
    (void) testInvariant("OStoreBTreeRootObject::change(): leave");
    return eErrCode;
}

/*
 * find_lookup (w/o split()).
 * Precond: root node page loaded.
 */
storeError OStoreBTreeRootObject::find_lookup (
    OStoreBTreeNodeObject & rNode,  
    sal_uInt16 &            rIndex, 
    OStorePageKey const &   rKey,
    OStorePageBIOS &        rBIOS)
{
    
    (void) testInvariant("OStoreBTreeRootObject::find_lookup(): enter");
    {
        PageHolder tmp (m_xPage);
        tmp.swap (rNode.get());
    }

    
    T const entry (rKey);

    
    PageHolderObject< page > xPage (rNode.get());
    for (; xPage->depth() > 0; xPage = rNode.makeHolder< page >())
    {
        
        page const & rPage = (*xPage);
        sal_uInt16 const i = rPage.find(entry);
        sal_uInt16 const n = rPage.usageCount();
        if (!(i < n))
        {
            
            return store_E_NotExists;
        }

        
        sal_uInt32 const nAddr = rPage.m_pData[i].m_aLink.location();
        if (nAddr == STORE_PAGE_NULL)
        {
            
            return store_E_NotExists;
        }

        
        storeError eErrCode = rBIOS.loadObjectAt (rNode, nAddr);
        if (eErrCode != store_E_None)
            return eErrCode;
    }

    
    page const & rPage = (*xPage);
    rIndex = rPage.find(entry);
    if (!(rIndex < rPage.usageCount()))
        return store_E_NotExists;

    
    T::CompareResult eResult = entry.compare(rPage.m_pData[rIndex]);
    OSL_POSTCOND(eResult != T::COMPARE_LESS, "store::BTreeRoot::find_lookup(): sort error");
    if (eResult == T::COMPARE_LESS)
        return store_E_Unknown;

    
    (void) testInvariant("OStoreBTreeRootObject::find_lookup(): leave");
    return store_E_None;
}

/*
 * find_insert (possibly with split()).
 * Precond: root node page loaded.
 */
storeError OStoreBTreeRootObject::find_insert (
    OStoreBTreeNodeObject & rNode,  
    sal_uInt16 &            rIndex, 
    OStorePageKey const &   rKey,
    OStorePageBIOS &        rBIOS)
{
    (void) testInvariant("OStoreBTreeRootObject::find_insert(): enter");

    
    PageHolderObject< page > xRoot (m_xPage);
    if (xRoot->querySplit())
    {
        PageHolderObject< page > xPageL;

        
        storeError eErrCode = change (xPageL, rBIOS);
        if (eErrCode != store_E_None)
            return eErrCode;

        
        eErrCode = split (0, xPageL, rBIOS);
        if (eErrCode != store_E_None)
            return eErrCode;
    }

    
    {
        PageHolder tmp (m_xPage);
        tmp.swap (rNode.get());
    }

    
    T const entry (rKey);

    
    PageHolderObject< page > xPage (rNode.get());
    for (; xPage->depth() > 0; xPage = rNode.makeHolder< page >())
    {
        
        page const & rPage = (*xPage);
        sal_uInt16 const i = rPage.find (entry);
        sal_uInt16 const n = rPage.usageCount();
        if (!(i < n))
        {
            
            return store_E_NotExists;
        }

        
        sal_uInt32 const nAddr = rPage.m_pData[i].m_aLink.location();
        if (nAddr == STORE_PAGE_NULL)
        {
            
            return store_E_NotExists;
        }

        
        OStoreBTreeNodeObject aNext;
        storeError eErrCode = rBIOS.loadObjectAt (aNext, nAddr);
        if (eErrCode != store_E_None)
            return eErrCode;

        
        PageHolderObject< page > xNext (aNext.get());
        if (xNext->querySplit())
        {
            
            eErrCode = rNode.split (i, xNext, rBIOS);
            if (eErrCode != store_E_None)
                return eErrCode;

            
            continue;
        }

        
        PageHolder tmp (aNext.get());
        tmp.swap (rNode.get());
    }

    
    page const & rPage = (*xPage);
    rIndex = rPage.find(entry);
    if (rIndex < rPage.usageCount())
    {
        
        T::CompareResult result = entry.compare (rPage.m_pData[rIndex]);
        OSL_POSTCOND(result != T::COMPARE_LESS, "store::BTreeRoot::find_insert(): sort error");
        if (result == T::COMPARE_LESS)
            return store_E_Unknown;

        if (result == T::COMPARE_EQUAL)
            return store_E_AlreadyExists;
    }

    
    (void) testInvariant("OStoreBTreeRootObject::find_insert(): leave");
    return store_E_None;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
