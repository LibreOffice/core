/*************************************************************************
 *
 *  $RCSfile: stortree.cxx,v $
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

#define _STORE_STORTREE_CXX "$Revision: 1.1.1.1 $"

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif

#ifndef _VOS_DIAGNOSE_HXX_
#include <vos/diagnose.hxx>
#endif
#ifndef _VOS_MACROS_HXX_
#include <vos/macros.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _STORE_TYPES_H_
#include <store/types.h>
#endif
#ifndef _STORE_MACROS_HXX_
#include <store/macros.hxx>
#endif

#ifndef _STORE_STORBASE_HXX
#include <storbase.hxx>
#endif
#ifndef _STORE_STORTREE_HXX
#include <stortree.hxx>
#endif

#ifdef _USE_NAMESPACE
using namespace store;
#endif

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
    initialize();
}

/*
 * initialize.
 */
void OStoreBTreeNodeData::initialize (void)
{
    base::m_aGuard.m_nMagic = STORE_MAGIC_BTREENODE;
    base::m_aDescr.m_nUsed  = base::size() + self::size();
    self::m_aGuard.m_nMagic = 0;

    sal_uInt16 i, n = capacityCount();
    T          t;

    for (i = 1; i < n; i++)
        m_pData[i] = t;
}

/*
 * swap.
 */
void OStoreBTreeNodeData::swap (const D& rDescr)
{
#ifdef OSL_BIGENDIAN
    m_aGuard.swap();

    sal_uInt16 i, n = capacity(rDescr) / sizeof(T);
    for (i = 0; i < n; i++)
        m_pData[i].swap();
#endif /* OSL_BIGENDIAN */
}

/*
 * find.
 */
sal_uInt16 OStoreBTreeNodeData::find (const T& t) const
{
    register sal_Int32 l = 0;
    register sal_Int32 r = usageCount() - 1;

    while (l < r)
    {
        register sal_Int32 m = ((l + r) >> 1);

        if (t.m_aKey == m_pData[m].m_aKey)
            return ((sal_uInt16)(m));
        if (t.m_aKey < m_pData[m].m_aKey)
            r = m - 1;
        else
            l = m + 1;
    }

    sal_uInt16 k = ((sal_uInt16)(r));
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
    sal_uInt16 n = usageCount();
    sal_uInt16 m = capacityCount();
    if ((n < m) && (i < m))
    {
        // shift right.
        rtl_moveMemory (&m_pData[i + 1], &m_pData[i], (n - i) * sizeof(T));

        // insert.
        m_pData[i] = t;
        base::m_aDescr.m_nUsed += sizeof(T);
    }
}

/*
 * remove.
 */
void OStoreBTreeNodeData::remove (sal_uInt16 i)
{
    sal_uInt16 n = usageCount();
    sal_uInt16 m = capacityCount();
    if (i < n)
    {
        // shift left.
        rtl_moveMemory (
            &m_pData[i], &m_pData[i + 1], (n - i - 1) * sizeof(T));

        // truncate.
        m_pData[n - 1] = T();
        base::m_aDescr.m_nUsed -= sizeof(T);
    }
}

/*
 * merge.
 */
void OStoreBTreeNodeData::merge (const self& rPageR)
{
    if (queryMerge (rPageR))
    {
        sal_uInt16 n = usageCount();
        sal_uInt16 m = rPageR.usageCount();
        rtl_copyMemory (&m_pData[n], &rPageR.m_pData[0], m * sizeof(T));
        usageCount (n + m);
    }
}

/*
 * split.
 */
void OStoreBTreeNodeData::split (const self& rPageL)
{
    sal_uInt16 h = capacityCount() / 2;
    rtl_copyMemory (&m_pData[0], &rPageL.m_pData[h], h * sizeof(T));
    truncate (h);
}

/*
 * truncate.
 */
void OStoreBTreeNodeData::truncate (sal_uInt16 n)
{
    sal_uInt16 m = capacityCount();
    T          t;

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
 * swap.
 */
void OStoreBTreeNodeObject::swap (const D& rDescr)
{
#ifdef OSL_BIGENDIAN
    base::swap (rDescr);
    m_rPage.swap (rDescr);
#endif /* OSL_BIGENDIAN */
}

/*
 * guard.
 */
void OStoreBTreeNodeObject::guard (const D& rDescr)
{
    base::guard (rDescr);
    m_rPage.guard (rDescr);
}

/*
 * verify.
 */
storeError OStoreBTreeNodeObject::verify (const D& rDescr)
{
    storeError eErrCode = base::verify (rDescr);
    if (eErrCode != store_E_None)
        return eErrCode;
    else
        return m_rPage.verify (rDescr);
}

/*
 * split.
 */
storeError OStoreBTreeNodeObject::split (
    sal_uInt16             nIndexL,
    OStoreBTreeNodeData   &rPageL,
    OStoreBTreeNodeData   &rPageR,
    OStorePageBIOS        &rBIOS,
    NAMESPACE_VOS(IMutex) *pMutex)
{
    // Check usage.
    if (!rPageL.querySplit())
        return store_E_None;

    // Enter.
    STORE_METHOD_ENTER(pMutex);

    // Save PageDescriptor.
    D aDescr (m_rPage.m_aDescr);

    // Acquire Lock.
    storeError eErrCode = rBIOS.acquireLock (aDescr.m_nAddr, aDescr.m_nSize);
    if (eErrCode != store_E_None)
        STORE_METHOD_LEAVE(pMutex, eErrCode);

    // Begin PageL Lock (NYI).

    // Split right page off left page.
    rPageR.split (rPageL);
    rPageR.depth (rPageL.depth());

    // Allocate right page.
    self aNodeR (rPageR);
    eErrCode = rBIOS.allocate (aNodeR);
    if (eErrCode != store_E_None)
    {
        rBIOS.releaseLock (aDescr.m_nAddr, aDescr.m_nSize);
        STORE_METHOD_LEAVE(pMutex, eErrCode);
    }

    // Truncate left page.
    rPageL.truncate (rPageL.capacityCount() / 2);

    // Save left page.
    self aNodeL (rPageL);
    eErrCode = rBIOS.save (aNodeL);
    if (eErrCode != store_E_None)
    {
        // Must not happen.
        VOS_TRACE("OStoreBTreeNodeObject::split(): save() failed");

        // Release Lock and Leave.
        rBIOS.releaseLock (aDescr.m_nAddr, aDescr.m_nSize);
        STORE_METHOD_LEAVE(pMutex, eErrCode);
    }

    // End PageL Lock (NYI).

    // Insert right page.
    T entry;
    entry.m_aKey          = rPageR.m_pData[0].m_aKey;
    entry.m_aLink.m_nAddr = rPageR.location();

    m_rPage.insert (nIndexL + 1, entry);

    // Save this page.
    eErrCode = rBIOS.save (*this);
    if (eErrCode != store_E_None)
    {
        // Must not happen.
        VOS_TRACE("OStoreBTreeNodeObject::split(): save() failed");

        // Release Lock and Leave.
        rBIOS.releaseLock (aDescr.m_nAddr, aDescr.m_nSize);
        STORE_METHOD_LEAVE(pMutex, eErrCode);
    }

#if 0  /* PERFORMANCE */
    eErrCode = rBIOS.flush();
#endif /* PERFORMANCE */

    // Release Lock and Leave.
    eErrCode = rBIOS.releaseLock (aDescr.m_nAddr, aDescr.m_nSize);
    STORE_METHOD_LEAVE(pMutex, eErrCode);
}

/*
 * remove (down to leaf node, recursive).
 */
storeError OStoreBTreeNodeObject::remove (
    sal_uInt16             nIndexL,
    OStoreBTreeEntry      &rEntryL,
    OStoreBTreeNodeData   &rPageL,
#if 0  /* NYI */
    OStoreBTreeNodeData   &rPageR,
#endif /* NYI */
    OStorePageBIOS        &rBIOS,
    NAMESPACE_VOS(IMutex) *pMutex)
{
    // Enter.
    STORE_METHOD_ENTER(pMutex);

    // Save PageDescriptor.
    D aDescr (m_rPage.m_aDescr);

    // Acquire Lock.
    storeError eErrCode = rBIOS.acquireLock (aDescr.m_nAddr, aDescr.m_nSize);
    if (eErrCode != store_E_None)
        STORE_METHOD_LEAVE(pMutex, eErrCode);

    // Check depth.
    if (m_rPage.depth())
    {
        // Check link entry.
        if (!(rEntryL.compare (m_rPage.m_pData[nIndexL]) == T::COMPARE_EQUAL))
        {
            rBIOS.releaseLock (aDescr.m_nAddr, aDescr.m_nSize);
            STORE_METHOD_LEAVE(pMutex, store_E_InvalidAccess);
        }

        // Load link node.
        self aNodeL (rPageL);
        aNodeL.location (m_rPage.m_pData[nIndexL].m_aLink.m_nAddr);

        eErrCode = rBIOS.load (aNodeL);
        if (eErrCode != store_E_None)
        {
            rBIOS.releaseLock (aDescr.m_nAddr, aDescr.m_nSize);
            STORE_METHOD_LEAVE(pMutex, eErrCode);
        }

        // Remove from link node (using current page as link buffer).
        eErrCode = aNodeL.remove (0, rEntryL, m_rPage, rBIOS, NULL);
        if (eErrCode != store_E_None)
        {
            rBIOS.releaseLock (aDescr.m_nAddr, aDescr.m_nSize);
            STORE_METHOD_LEAVE(pMutex, eErrCode);
        }

        // Reload current page.
        m_rPage.location (aDescr.m_nAddr);
        eErrCode = rBIOS.load (*this);
        if (eErrCode != store_E_None)
        {
            // Must not happen.
            VOS_TRACE("OStoreBTreeNodeObject::remove(): load() failed");

            // Release Lock and Leave.
            rBIOS.releaseLock (aDescr.m_nAddr, aDescr.m_nSize);
            STORE_METHOD_LEAVE(pMutex, eErrCode);
        }

        // Check link node usage.
        if (rPageL.usageCount() == 0)
        {
            // Free empty link node.
            eErrCode = rBIOS.free (aNodeL);
            if (eErrCode != store_E_None)
            {
                rBIOS.releaseLock (aDescr.m_nAddr, aDescr.m_nSize);
                STORE_METHOD_LEAVE(pMutex, eErrCode);
            }

            // Remove index.
            m_rPage.remove (nIndexL);
            touch();
        }
        else
        {
#if 0   /* NYI */
            // Check for right sibling.
            sal_uInt16 nIndexR = nIndexL + 1;
            if (nIndexR < m_rPage.usageCount())
            {
                // Load right link node.
                self aNodeR (rPageR);
                aNodeR.location (m_rPage.m_pData[nIndexR].m_aLink.m_nAddr);

                eErrCode = rBIOS.load (aNodeR);
                if (eErrCode == store_E_None)
                {
                    if (rPageL.queryMerge (rPageR))
                    {
                        rPageL.merge (rPageR);

                        eErrCode = rBIOS.free (rPageR);
                    }
                }
            }
#endif  /* NYI */

            // Relink.
            m_rPage.m_pData[nIndexL].m_aKey = rPageL.m_pData[0].m_aKey;
            touch();
        }
    }
    else
    {
        // Check leaf entry.
        if (!(rEntryL.compare (m_rPage.m_pData[nIndexL]) == T::COMPARE_EQUAL))
        {
            rBIOS.releaseLock (aDescr.m_nAddr, aDescr.m_nSize);
            STORE_METHOD_LEAVE(pMutex, store_E_NotExists);
        }

        // Save leaf entry.
        rEntryL = m_rPage.m_pData[nIndexL];

        // Remove leaf index.
        m_rPage.remove (nIndexL);
        touch();
    }

    // Check for modified node.
    if (dirty())
    {
        // Save this page.
        eErrCode = rBIOS.save (*this);
        if (eErrCode != store_E_None)
        {
            // Must not happen.
            VOS_TRACE("OStoreBTreeNodeObject::remove(): save() failed");

            // Release Lock and Leave.
            rBIOS.releaseLock (aDescr.m_nAddr, aDescr.m_nSize);
            STORE_METHOD_LEAVE(pMutex, eErrCode);
        }
    }

    // Release Lock and Leave.
    eErrCode = rBIOS.releaseLock (aDescr.m_nAddr, aDescr.m_nSize);
    STORE_METHOD_LEAVE(pMutex, eErrCode);
}

/*========================================================================
 *
 * OStoreBTreeRootObject implementation.
 *
 *======================================================================*/
/*
 * change.
 */
storeError OStoreBTreeRootObject::change (
    OStoreBTreeNodeData   &rPageL,
    OStorePageBIOS        &rBIOS,
    NAMESPACE_VOS(IMutex) *pMutex)
{
    // Enter.
    STORE_METHOD_ENTER(pMutex);

    // Save PageDescriptor.
    typedef OStorePageDescriptor D;
    D aDescr (m_rPage.m_aDescr);

    // Acquire Lock.
    storeError eErrCode = rBIOS.acquireLock (aDescr.m_nAddr, aDescr.m_nSize);
    if (eErrCode != store_E_None)
        STORE_METHOD_LEAVE(pMutex, eErrCode);

    // Change root.
    rPageL = m_rPage;

    base aNodeL (rPageL);
    eErrCode = rBIOS.allocate (aNodeL);
    if (eErrCode != store_E_None)
    {
        // Release Lock and Leave.
        rBIOS.releaseLock (aDescr.m_nAddr, aDescr.m_nSize);
        STORE_METHOD_LEAVE(pMutex, eErrCode);
    }

    m_rPage.m_pData[0].m_aKey = rPageL.m_pData[0].m_aKey;
    m_rPage.m_pData[0].m_aLink.m_nAddr = rPageL.location();

    m_rPage.truncate (1);
    m_rPage.depth (m_rPage.depth() + 1);

    // Save root.
    eErrCode = rBIOS.save (*this);
    if (eErrCode != store_E_None)
    {
        // Must not happen.
        VOS_TRACE("OStoreBTreeRootObject::change(): save() failed");

        // Release Lock and Leave.
        rBIOS.releaseLock (aDescr.m_nAddr, aDescr.m_nSize);
        STORE_METHOD_LEAVE(pMutex, eErrCode);
    }

#if 1  /* ROBUSTNESS */
    eErrCode = rBIOS.flush();
#endif /* ROBUSTNESS */

    // Done. Release Lock and Leave.
    eErrCode = rBIOS.releaseLock (aDescr.m_nAddr, aDescr.m_nSize);
    STORE_METHOD_LEAVE(pMutex, eErrCode);
}

/*
 * split.
 */
storeError OStoreBTreeRootObject::split (
    sal_uInt16             nIndexL,
    OStoreBTreeNodeData   &rPageL,
    OStoreBTreeNodeData   &rPageR,
    OStorePageBIOS        &rBIOS,
    NAMESPACE_VOS(IMutex) *pMutex)
{
    // Check usage.
    if (!querySplit())
        return store_E_None;

    // Enter.
    STORE_METHOD_ENTER(pMutex);

    // Change root.
    storeError eErrCode = change (rPageL, rBIOS, NULL);
    if (eErrCode != store_E_None)
        STORE_METHOD_LEAVE(pMutex, eErrCode);

    // Split Left Page.
    eErrCode = base::split (0, rPageL, rPageR, rBIOS, NULL);

    // Leave.
    STORE_METHOD_LEAVE(pMutex, eErrCode);
}

