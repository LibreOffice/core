/*************************************************************************
 *
 *  $RCSfile: storcach.cxx,v $
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

#define _STORE_STORCACH_CXX "$Revision: 1.1.1.1 $"

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif

#ifndef _VOS_DIAGNOSE_HXX_
#include <vos/diagnose.hxx>
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

#ifndef _STORE_STORBASE_HXX_
#include <storbase.hxx>
#endif
#ifndef _STORE_STORCACH_HXX_
#include <storcach.hxx>
#endif

#include <string.h>

#ifdef _USE_NAMESPACE
using namespace store;
#endif

/*========================================================================
 *
 * OStorePageCache internals.
 *
 *======================================================================*/
/*
 * __store_memcpy.
 */
inline void __store_memcpy (void *dst, const void *src, sal_uInt32 n)
{
    // rtl_copyMemory (dst, src, n);
    ::memcpy (dst, src, n);
}

/*========================================================================
 *
 * OStorePageCacheEntry.
 *
 *======================================================================*/
#ifdef _USE_NAMESPACE
namespace store {
#endif

struct OStorePageCacheEntry
{
    typedef OStorePageCacheEntry self;
    typedef OStorePageData       data;
    typedef OStorePageDescriptor D;

    /** Representation.
    */
    D     m_aDescr;
    data *m_pData;
    self *m_pNext;
    self *m_pPrev;

    /** Construction.
    */
    OStorePageCacheEntry (const D& rDescr, const data& rData)
        : m_aDescr (rDescr), m_pData (NULL), m_pNext (this), m_pPrev (this)
    {
        sal_uInt16 nSize = m_aDescr.m_nSize;
        m_pData = new(nSize) data(nSize);
        __store_memcpy (m_pData, &rData, nSize);
    }

    /** Data assignment.
    */
    void assign (const D& rDescr, const data& rData)
    {
        m_aDescr.m_nAddr = rDescr.m_nAddr;
        if (!(m_aDescr.m_nSize == rDescr.m_nSize))
        {
            delete m_pData;
            m_pData = new(rDescr.m_nSize) data(rDescr.m_nSize);
            m_aDescr.m_nSize = rDescr.m_nSize;
        }
        __store_memcpy (m_pData, &rData, m_aDescr.m_nSize);
    }

    /** Destruction.
    */
    ~OStorePageCacheEntry (void)
    {
        delete m_pData;
    }

    /** Comparison.
    */
    enum CompareResult
    {
        COMPARE_LESS    = -1,
        COMPARE_EQUAL   =  0,
        COMPARE_GREATER =  1
    };

    CompareResult compare (const D& rDescr) const
    {
        if (m_aDescr.m_nAddr == rDescr.m_nAddr)
            return COMPARE_EQUAL;
        if (m_aDescr.m_nAddr < rDescr.m_nAddr)
            return COMPARE_LESS;
        else
            return COMPARE_GREATER;
    }

    CompareResult compare (const self& rOther) const
    {
        return compare (rOther.m_aDescr);
    }

    /** Address operation.
    */
    void invalidate (void)
    {
        m_aDescr.m_nAddr = STORE_PAGE_NULL;
    }

    sal_Bool isValid (void) const
    {
        return (m_aDescr.m_nAddr != STORE_PAGE_NULL);
    }

    /** Index operation.
    */
    sal_uInt16 index (void) const
    {
        return (m_aDescr.m_nUsed & 0x7fff);
    }

    void index (sal_uInt16 nIndex)
    {
        m_aDescr.m_nUsed = ((m_aDescr.m_nUsed & 0x8000) | (nIndex & 0x7fff));
    }

    /** DirtyBit operation.
    */
    void clean (void)
    {
        m_aDescr.m_nUsed &= 0x7fff;
    }

    void dirty (void)
    {
        m_aDescr.m_nUsed |= 0x8000;
    }

    sal_Bool isDirty (void) const
    {
        return ((m_aDescr.m_nUsed & 0x8000) == 0x8000);
    }

    /** List operation.
    */
    void backlink (self& rOther)
    {
        rOther.m_pNext = this;
        rOther.m_pPrev = m_pPrev;
        m_pPrev = &rOther;
        rOther.m_pPrev->m_pNext = &rOther;
    }

    void unlink (void)
    {
        m_pNext->m_pPrev = m_pPrev;
        m_pPrev->m_pNext = m_pNext;
        m_pNext = this;
        m_pPrev = this;
    }
};

#ifdef _USE_NAMESPACE
}
#endif

/*========================================================================
 *
 * OStorePageCache debug internals.
 *
 *======================================================================*/
/*
 * __store_check_entry.
 */
static sal_Bool __store_check_entry (
    OStorePageCacheEntry **ppData, sal_uInt16 nUsed)
{
    if (nUsed > 1)
    {
        for (sal_uInt16 i = 0; i < nUsed - 1; i++)
        {
            sal_uInt32 ai = ppData[i    ]->m_aDescr.m_nAddr;
            sal_uInt32 ak = ppData[i + 1]->m_aDescr.m_nAddr;
            if (!(ai <= ak))
                return sal_False;
            if (!(i == ppData[i]->index()))
                return sal_False;
        }
    }
    return sal_True;
}

/*
 * __store_find_entry.
 */
static sal_uInt16 __store_find_entry (
    const OStorePageDescriptor &rDescr,
    const OStorePageCacheEntry *pHead)
{
    if (pHead)
    {
        if (pHead->m_aDescr.m_nAddr == rDescr.m_nAddr)
            return pHead->index();

        OStorePageCacheEntry *pEntry = pHead->m_pNext;
        while (pEntry != pHead)
        {
            if (pEntry->m_aDescr.m_nAddr == rDescr.m_nAddr)
                return pEntry->index();
            else
                pEntry = pEntry->m_pNext;
        }
    }
    return ((sal_uInt16)(-1));
}

/*========================================================================
 *
 * OStorePageCache implementation.
 *
 * (two-way association (sorted address array, LRU chain)).
 * (external OStorePageData representation).
 *
 *======================================================================*/
/*
 * OStorePageCache.
 */
OStorePageCache::OStorePageCache (sal_uInt16 nPages)
    : m_nSize    (STORE_LIMIT_CACHEPAGES),
      m_nUsed    (0),
      m_pHead    (0),
      m_nHit     (0),
      m_nMissed  (0),
      m_nUpdHit  (0),
      m_nUpdLRU  (0),
      m_nWrtBack (0)
{
    for (sal_uInt16 i = 0; i < m_nSize; i++)
        m_pData[i] = NULL;
    if (nPages < m_nSize)
        m_nSize = nPages;
}

/*
 * ~OStorePageCache.
 */
OStorePageCache::~OStorePageCache (void)
{
    double x = hitRatio();
    for (sal_uInt16 i = 0; i < m_nSize; i++)
        delete m_pData[i];
}

/*
 * find.
 */
sal_uInt16 OStorePageCache::find (const OStorePageDescriptor &rDescr) const
{
    register sal_Int32 l = 0;
    register sal_Int32 r = m_nUsed - 1;

    while (l < r)
    {
        register sal_Int32 m = ((l + r) >> 1);

        if (m_pData[m]->m_aDescr.m_nAddr == rDescr.m_nAddr)
            return ((sal_uInt16)(m));
        if (m_pData[m]->m_aDescr.m_nAddr < rDescr.m_nAddr)
            l = m + 1;
        else
            r = m - 1;
    }

    // Match or insert position. Caller must do final compare.
    return ((sal_uInt16)(r));
}

/*
 * move.
 */
void OStorePageCache::move (sal_uInt16 nSI, sal_uInt16 nDI)
{
    entry *p = m_pData[nSI];
    if (nSI < nDI)
    {
        // shift left.
        rtl_moveMemory (
            &m_pData[nSI    ],
            &m_pData[nSI + 1],
            (nDI - nSI) * sizeof(entry*));

        // re-index.
        for (sal_uInt16 i = nSI; i < nDI; i++)
            m_pData[i]->index(i);
    }
    if (nSI > nDI)
    {
        // shift right.
        rtl_moveMemory (
            &m_pData[nDI + 1],
            &m_pData[nDI    ],
            (nSI - nDI) * sizeof(entry*));

        // re-index.
        for (sal_uInt16 i = nSI; i > nDI; i--)
            m_pData[i]->index(i);
    }
    m_pData[nDI] = p;
    m_pData[nDI]->index(nDI);

#if 0  /* DEBUG */
    VOS_POSTCOND(
        __store_check_entry(&m_pData[0], m_nUsed),
        "OStorePageCache::move(): check_entry() failed");
#endif /* DEBUG */
}

/*
 * insert.
 */
storeError OStorePageCache::insert (
    sal_uInt16                  nDI,
    const OStorePageDescriptor &rDescr,
    const OStorePageData       &rData,
    OStorePageBIOS             &rBIOS,
    InsertMode                  eMode)
{
#if 0  /* DEBUG */
    VOS_PRECOND(
        __store_check_entry(&m_pData[0], m_nUsed),
        "OStorePageCache::insert(): check_entry() failed");
#endif /* DEBUG */

    entry::CompareResult result = entry::COMPARE_EQUAL;
    if (nDI < m_nUsed)
    {
        result = m_pData[nDI]->compare (rDescr);
        if (result == entry::COMPARE_LESS)
            nDI += 1;
    }

    if (nDI == (sal_uInt16)(-1))
        nDI = 0;
    if (nDI == m_nSize)
        nDI -= 1;

    if (m_nUsed < m_nSize)
    {
        // Allocate cache entry.
        m_pData[m_nUsed] = new entry (rDescr, rData);
        move (m_nUsed, nDI);
        m_nUsed++;

        // Update LRU.
        if (m_pHead)
            m_pHead->backlink (*m_pData[nDI]);
        m_pHead = m_pData[nDI];
    }
    else
    {
        // Check for invalidated cache entry.
        sal_uInt16 nSI = m_nUsed - 1;
        if (m_pData[nSI]->isValid())
        {
            // Replace least recently used cache entry.
            m_nUpdLRU++;

            m_pHead = m_pHead->m_pPrev;
            nSI     = m_pHead->index();

            // Check DirtyBit.
            if (m_pHead->isDirty())
            {
                // Save PageDescriptor.
                OStorePageDescriptor aDescr (m_pHead->m_aDescr);

                // Write page.
                storeError eErrCode = rBIOS.write (
                    aDescr.m_nAddr, m_pHead->m_pData, aDescr.m_nSize);
                if (eErrCode != store_E_None)
                    return eErrCode;

                // Mark as clean.
                m_pHead->clean();
                m_nWrtBack++;
            }
        }
        else
        {
            // Replace invalidated cache entry. Check LRU.
            if (!(m_pData[nSI] == m_pHead))
            {
                // Update LRU.
                m_pData[nSI]->unlink();
                m_pHead->backlink (*m_pData[nSI]);
                m_pHead = m_pData[nSI];
            }
        }

        // Check source and destination indices.
        if (nSI < nDI)
        {
            result = m_pData[nDI]->compare(rDescr);
            if (result == entry::COMPARE_GREATER)
                nDI -= 1;
        }

        // Assign data.
        m_pData[nSI]->assign (rDescr, rData);
        move (nSI, nDI);
    }

    // Check InsertMode.
    if (eMode == INSERT_CLEAN)
        m_pHead->clean();
    else
        m_pHead->dirty();

    // Done.
    return store_E_None;
}

/*
 * load.
 */
storeError OStorePageCache::load (
    const OStorePageDescriptor &rDescr,
    OStorePageData             &rData,
    OStorePageBIOS             &rBIOS,
    NAMESPACE_VOS(IMutex)      *pMutex)
{
    // Enter.
    STORE_METHOD_ENTER(pMutex);

    // Find cache index.
    sal_uInt16 i = find (rDescr);
    if (i < m_nUsed)
    {
        if (m_pData[i]->compare(rDescr) == entry::COMPARE_EQUAL)
        {
            // Cache hit.
            m_nHit++;

            if (!(m_pData[i] == m_pHead))
            {
                // Update LRU.
                m_pData[i]->unlink();
                m_pHead->backlink (*m_pData[i]);
                m_pHead = m_pData[i];
            }

            // Load data and Leave.
            __store_memcpy (&rData, m_pHead->m_pData, rDescr.m_nSize);
            STORE_METHOD_LEAVE(pMutex, store_E_None);
        }
    }

    // Cache miss.
    m_nMissed++;

    // Load data.
    storeError eErrCode = rBIOS.read (
        rDescr.m_nAddr, &rData, rDescr.m_nSize);
    if (eErrCode != store_E_None)
        STORE_METHOD_LEAVE(pMutex, eErrCode);

    // Insert data.
    eErrCode = insert (i, rDescr, rData, rBIOS, INSERT_CLEAN);
    if (eErrCode != store_E_None)
        STORE_METHOD_LEAVE(pMutex, eErrCode);

    // Leave with pending verification.
    STORE_METHOD_LEAVE(pMutex, store_E_Pending);
}

/*
 * update.
 */
storeError OStorePageCache::update (
    const OStorePageDescriptor &rDescr,
    const OStorePageData       &rData,
    OStorePageBIOS             &rBIOS,
    NAMESPACE_VOS(IMutex)      *pMutex,
    UpdateMode                  eMode)
{
    // Enter.
    STORE_METHOD_ENTER(pMutex);

    // Check UpdateMode.
    if (eMode == UPDATE_WRITE_THROUGH)
    {
        // Save data.
        storeError eErrCode = rBIOS.write (
            rDescr.m_nAddr, &rData, rDescr.m_nSize);
        if (eErrCode != store_E_None)
            STORE_METHOD_LEAVE(pMutex, eErrCode);
        m_nWrtBack++;
    }

    // Find cache index.
    sal_uInt16 i = find (rDescr);
    if (i < m_nUsed)
    {
        if (m_pData[i]->compare(rDescr) == entry::COMPARE_EQUAL)
        {
            // Cache hit. Check LRU.
            m_nUpdHit++;
            if (!(m_pData[i] == m_pHead))
            {
                // Update LRU.
                m_pData[i]->unlink();
                m_pHead->backlink (*m_pData[i]);
                m_pHead = m_pData[i];
            }

            // Check UpdateMode.
            if (eMode == UPDATE_WRITE_THROUGH)
                m_pHead->clean();
            else
                m_pHead->dirty();

            // Update data and leave.
            __store_memcpy (m_pHead->m_pData, &rData, rDescr.m_nSize);
            STORE_METHOD_LEAVE(pMutex, store_E_None);
        }
    }

    // Cache miss. Insert data and leave.
    storeError eErrCode = insert (
        i, rDescr, rData, rBIOS,
        ((eMode == UPDATE_WRITE_THROUGH) ? INSERT_CLEAN : INSERT_DIRTY));
    STORE_METHOD_LEAVE(pMutex, eErrCode);
}

/*
 * invalidate.
 */
storeError OStorePageCache::invalidate (
    const OStorePageDescriptor &rDescr,
    NAMESPACE_VOS(IMutex)      *pMutex)
{
    // Enter.
    STORE_METHOD_ENTER(pMutex);

    // Find cache index.
    sal_uInt16 i = find (rDescr);
    if (i < m_nUsed)
    {
        if (m_pData[i]->compare(rDescr) == entry::COMPARE_EQUAL)
        {
            // Cache hit. Update LRU.
            if (!(m_pData[i] == m_pHead))
            {
                m_pData[i]->unlink();
                m_pHead->backlink (*m_pData[i]);
            }
            else
            {
                m_pHead = m_pHead->m_pNext;
            }

            // Invalidate.
            m_pData[i]->clean();
            m_pData[i]->invalidate();
            move (i, m_nUsed - 1);
        }
    }

    // Leave.
    STORE_METHOD_LEAVE(pMutex, store_E_None);
}

/*
 * flush.
 */
storeError OStorePageCache::flush (
    OStorePageBIOS        &rBIOS,
    NAMESPACE_VOS(IMutex) *pMutex)
{
    // Enter.
    STORE_METHOD_ENTER(pMutex);

    // Check all entries.
    for (sal_uInt16 i = 0; i < m_nUsed; i++)
    {
        // Check for dirty entry.
        if (m_pData[i]->isDirty() && m_pData[i]->isValid())
        {
            // Save PageDescriptor.
            OStorePageDescriptor aDescr (m_pData[i]->m_aDescr);

            // Write page.
            storeError eErrCode = rBIOS.write (
                aDescr.m_nAddr, m_pData[i]->m_pData, aDescr.m_nSize);
            VOS_POSTCOND(
                eErrCode == store_E_None,
                "OStorePageCache::flush(): write() failed");

            // Mark entry clean.
            if (eErrCode == store_E_None)
                m_pData[i]->clean();
            m_nWrtBack++;
        }
    }

    // Leave.
    STORE_METHOD_LEAVE(pMutex, store_E_None);
}

