/*************************************************************************
 *
 *  $RCSfile: storpage.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mhu $ $Date: 2001-03-13 20:45:39 $
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
 *  Contributor(s): Matthias Huetsch <matthias.huetsch@sun.com>
 *
 *
 ************************************************************************/

#define _STORE_STORPAGE_CXX_ "$Revision: 1.2 $"

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif
#ifndef _RTL_STRING_H_
#include <rtl/string.h>
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _STORE_TYPES_H_
#include <store/types.h>
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
#ifndef _STORE_STORCACH_HXX_
#include <storcach.hxx>
#endif
#ifndef _STORE_STORDMON_HXX_
#include <stordmon.hxx>
#endif
#ifndef _STORE_STORDATA_HXX_
#include <stordata.hxx>
#endif
#ifndef _STORE_STORTREE_HXX_
#include <stortree.hxx>
#endif

#ifndef _STORE_STORPAGE_HXX_
#include <storpage.hxx>
#endif

using namespace store;

/*========================================================================
 *
 * OStorePageManager implementation.
 *
 *======================================================================*/
const sal_uInt32 OStorePageManager::m_nTypeId = sal_uInt32(0x62190120);

/*
 * OStorePageManager.
 */
OStorePageManager::OStorePageManager (void)
    : m_xDaemon   (NULL),
      m_pCache    (NULL),
      m_pDirect   (NULL),
      m_pData     (NULL),
      m_nPageSize (0)
{
    // Node pages.
    m_pNode[0] = NULL;
    m_pNode[1] = NULL;
    m_pNode[2] = NULL;

    // Indirect pages.
    m_pLink[0] = NULL;
    m_pLink[1] = NULL;
    m_pLink[2] = NULL;

    // Daemon (kflushd :-).
    OStorePageDaemon::getOrCreate (m_xDaemon);
}

/*
 * ~OStorePageManager.
 */
OStorePageManager::~OStorePageManager (void)
{
    osl::MutexGuard aGuard (*this);
    if (m_xDaemon.is())
    {
        m_xDaemon->remove (this);
        m_xDaemon.clear();
    }

    delete m_pCache;
    delete m_pDirect;
    delete m_pData;

    delete m_pNode[0];
    delete m_pNode[1];
    delete m_pNode[2];

    delete m_pLink[0];
    delete m_pLink[1];
    delete m_pLink[2];
}

/*
 * isKindOf.
 */
sal_Bool SAL_CALL OStorePageManager::isKindOf (sal_uInt32 nTypeId)
{
    return (nTypeId == m_nTypeId);
}

/*
 * initialize (two-phase construction).
 * Precond: none.
 */
storeError OStorePageManager::initialize (
    ILockBytes      *pLockBytes,
    storeAccessMode  eAccessMode,
    sal_uInt16       nPageSize)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard(*this);

    // Check arguments.
    if (!pLockBytes)
        return store_E_InvalidParameter;

    // Initialize base.
    storeError eErrCode = base::initialize (pLockBytes, eAccessMode);
    if (eErrCode != store_E_None)
    {
        // Check mode and reason.
        if (eErrCode != store_E_NotExists)
            return eErrCode;

        if (eAccessMode == store_AccessReadWrite)
            return store_E_NotExists;
        if (eAccessMode == store_AccessReadOnly)
            return store_E_NotExists;

        // Create.
        eErrCode = base::create (nPageSize);
        if (eErrCode != store_E_None)
            return eErrCode;
    }

    // Obtain page size.
    eErrCode = base::getPageSize (m_nPageSize);
    OSL_POSTCOND(
        eErrCode == store_E_None,
        "OStorePageManager::initialize(): getPageSize() failed");
    if (eErrCode != store_E_None)
        return eErrCode;

    // Cleanup.
    __STORE_DELETEZ (m_pCache);
    __STORE_DELETEZ (m_pDirect);
    __STORE_DELETEZ (m_pData);

    __STORE_DELETEZ (m_pNode[0]);
    __STORE_DELETEZ (m_pNode[1]);
    __STORE_DELETEZ (m_pNode[2]);

    __STORE_DELETEZ (m_pLink[0]);
    __STORE_DELETEZ (m_pLink[1]);
    __STORE_DELETEZ (m_pLink[2]);

    // Initialize page buffers.
    m_pNode[0] = new(m_nPageSize) page(m_nPageSize);
    if (eAccessMode != store_AccessReadOnly)
    {
        m_pNode[1] = new(m_nPageSize) page(m_nPageSize);
        m_pNode[2] = new(m_nPageSize) page(m_nPageSize);

        if (m_xDaemon.is())
        {
            // Request to be flushed.
            m_xDaemon->insert (this);
        }
    }

    // Initialize page cache.
    m_pCache = new OStorePageCache();

    // Done.
    return eErrCode;
}

/*
 * free (unmanaged).
 * Precond: initialized, writeable.
 */
storeError OStorePageManager::free (OStorePageObject &rPage)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard(*this);

    // Check precond.
    if (!self::isValid())
        return store_E_InvalidAccess;

    if (!base::isWriteable())
        return store_E_AccessViolation;

    // Check for cacheable page.
    OStorePageData &rData = rPage.getData();
    if (rData.m_aGuard.m_nMagic == STORE_MAGIC_BTREENODE)
    {
        // Invalidate cache entry.
        storeError eErrCode = m_pCache->invalidate (rData.m_aDescr);
        if (eErrCode != store_E_None)
            return eErrCode;
    }

    // Free page.
    return base::free (rPage);
}

/*
 * load (unmanaged).
 * Precond: initialized.
 */
storeError OStorePageManager::load (OStorePageObject &rPage)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard(*this);

    // Check precond.
    if (!self::isValid())
        return store_E_InvalidAccess;

    // Check for cacheable page.
    OStorePageData &rData = rPage.getData();
    if (rData.m_aGuard.m_nMagic == STORE_MAGIC_BTREENODE)
    {
        // Save PageDescriptor.
        OStorePageDescriptor aDescr (rData.m_aDescr);

        // Load (cached) page.
        storeError eErrCode = m_pCache->load (aDescr, rData, *this);
        if (eErrCode != store_E_None)
        {
            // Check for pending verification.
            if (eErrCode != store_E_Pending)
                return eErrCode;

            // Verify page.
            eErrCode = rPage.verify (aDescr);
            if (eErrCode != store_E_None)
                return eErrCode;
        }

#ifdef OSL_BIGENDIAN
        // Swap to internal representation.
        rPage.swap (aDescr);
#endif /* OSL_BIGENDIAN */

        // Done.
        return store_E_None;
    }

    // Load (uncached) page.
    return base::load (rPage);
}

/*
 * save (unmanaged).
 * Precond: initialized, writeable.
 */
storeError OStorePageManager::save (OStorePageObject &rPage)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard(*this);

    // Check precond.
    if (!self::isValid())
        return store_E_InvalidAccess;

    if (!base::isWriteable())
        return store_E_AccessViolation;

    // Check for cacheable page.
    OStorePageData &rData = rPage.getData();
    if (rData.m_aGuard.m_nMagic == STORE_MAGIC_BTREENODE)
    {
        // Save PageDescriptor.
        OStorePageDescriptor aDescr (rData.m_aDescr);

#ifdef OSL_BIGENDIAN
        // Swap to external representation.
        rPage.swap (aDescr);
#endif /* OSL_BIGENDIAN */

        // Guard page.
        rPage.guard (aDescr);

        // Save (cached) page.
#if 0  /* EXPERIMENTAL */
        storeError eErrCode = m_pCache->update (
            aDescr, rData, *this, NULL,
            OStorePageCache::UPDATE_WRITE_DELAYED);
#else
        storeError eErrCode = m_pCache->update (
            aDescr, rData, *this, NULL,
            OStorePageCache::UPDATE_WRITE_THROUGH);
#endif /* EXPERIMENTAL */

#ifdef OSL_BIGENDIAN
        // Swap back to internal representation.
        rPage.swap (aDescr);
#endif /* OSL_BIGENDIAN */

        // Mark page clean.
        if (eErrCode == store_E_None)
            rPage.clean();

        // Done.
        return eErrCode;
    }

    // Save (uncached) page.
    return base::save (rPage);
}

/*
 * flush.
 * Precond: initialized.
 */
storeError OStorePageManager::flush (void)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard(*this);

    // Check precond.
    if (!self::isValid())
        return store_E_InvalidAccess;

    // Check access mode.
    if (!base::isWriteable())
        return store_E_None;

    // Flush cache.
    storeError eErrCode = m_pCache->flush (*this, NULL);
    OSL_POSTCOND(
        eErrCode == store_E_None,
        "OStorePageManager::flush(): cache::flush() failed");

    // Flush base.
    return base::flush();
}

/*
 * find (w/o split()).
 * Internal: Precond: initialized, readable, exclusive access.
 */
storeError OStorePageManager::find (const entry &rEntry, page &rPage)
{
    // Load RootNode.
    OStoreBTreeRootObject aRoot (rPage);
    aRoot.location (m_nPageSize);

    storeError eErrCode = load (aRoot);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Check current Page.
    while (rPage.depth())
    {
        // Find next page.
        sal_uInt16 i = rPage.find(rEntry), n = rPage.usageCount();
        if (!(i < n))
        {
            // Path to entry not exists (Must not happen(?)).
            return store_E_NotExists;
        }

        // Check address.
        sal_uInt32 nAddr = rPage.m_pData[i].m_aLink.m_nAddr;
        if (nAddr == STORE_PAGE_NULL)
        {
            // Path to entry not exists (Must not happen(?)).
            return store_E_NotExists;
        }

        // Load next page.
        node aNode (rPage);
        aNode.location (nAddr);

        eErrCode = load (aNode);
        if (eErrCode != store_E_None)
            return eErrCode;
    }

    // Done.
    return store_E_None;
}

/*
 * find (possibly with split()).
 * Internal: Precond: initialized, writeable, exclusive access.
 */
storeError OStorePageManager::find (
    const entry &rEntry, page &rPage, page &rPageL, page &rPageR)
{
    // Load RootNode.
    OStoreBTreeRootObject aRoot (rPage);
    aRoot.location (m_nPageSize);

    storeError eErrCode = load (aRoot);
    if (eErrCode != store_E_None)
    {
        // Check existence.
        if (eErrCode != store_E_NotExists)
            return eErrCode;

        // Pre-allocate left most entry (ugly, but we can't insert to left).
        rPage.insert (0, entry());
        rPage.m_pData[0].m_aKey.m_nLow = OStorePageGuard::crc32 (0, "/", 1);

        // Allocate RootNode.
        eErrCode = base::allocate (aRoot, ALLOCATE_EOF);
        if (eErrCode != store_E_None)
            return eErrCode;
    }
    else
    {
        // Check for RootNode split.
        if (aRoot.querySplit())
        {
            // Split root.
            eErrCode = aRoot.split (0, rPageL, rPageR, *this);
            if (eErrCode != store_E_None)
                return eErrCode;
        }
    }

    // Check current Page.
    while (rPage.depth())
    {
        // Find next page.
        sal_uInt16 i = rPage.find (rEntry), n = rPage.usageCount();
        if (!(i < n))
        {
            // Path to entry not exists (Must not happen(?)).
            return store_E_NotExists;
        }

        // Check address.
        sal_uInt32 nAddr = rPage.m_pData[i].m_aLink.m_nAddr;
        if (nAddr == STORE_PAGE_NULL)
        {
            // Path to entry not exists (Must not happen(?)).
            return store_E_NotExists;
        }

        // Load next page.
        node aNode (rPageL);
        aNode.location (nAddr);

        eErrCode = load (aNode);
        if (eErrCode != store_E_None)
            return eErrCode;

        // Check for node split.
        if (rPageL.querySplit())
        {
            // Split node.
            node aParent (rPage);
            eErrCode = aParent.split (i, rPageL, rPageR, *this);
            if (eErrCode != store_E_None)
                return eErrCode;

            // Restart.
            continue;
        }
        else
        {
            // Let next page be current.
            rPage = rPageL;
            continue;
        }
    }

    // Done.
    return store_E_None;
}

/*
 * remove (possibly down from root).
 * Internal: Precond: initialized, writeable, exclusive access.
 */
storeError OStorePageManager::remove (
    entry &rEntry, page &rPage, page &rPageL)
{
    // Load RootNode.
    OStoreBTreeRootObject aRoot (rPage);
    aRoot.location (m_nPageSize);

    storeError eErrCode = load (aRoot);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Check index.
    sal_uInt16 i = rPage.find (rEntry), n = rPage.usageCount();
    if (!(i < n))
    {
        // Path to entry not exists (Must not happen(?)).
        return store_E_NotExists;
    }

    // Compare entry.
    entry::CompareResult result = rEntry.compare (rPage.m_pData[i]);

    // Iterate down until equal match.
    while ((result == entry::COMPARE_GREATER) && (rPage.depth() > 0))
    {
        // Check link address.
        sal_uInt32 nAddr = rPage.m_pData[i].m_aLink.m_nAddr;
        if (nAddr == STORE_PAGE_NULL)
        {
            // Path to entry not exists (Must not happen(?)).
            return store_E_NotExists;
        }

        // Load link page.
        node aNode (rPage);
        aNode.location (nAddr);

        eErrCode = load (aNode);
        if (eErrCode != store_E_None)
            return eErrCode;

        // Check index.
        i = rPage.find (rEntry), n = rPage.usageCount();
        if (!(i < n))
        {
            // Path to entry not exists (Must not happen(?)).
            return store_E_NotExists;
        }

        // Compare entry.
        result = rEntry.compare (rPage.m_pData[i]);
    }

    OSL_POSTCOND(
        result != entry::COMPARE_LESS,
        "OStorePageManager::remove(): find failed");

    // Check entry comparison.
    if (result == entry::COMPARE_LESS)
    {
        // Must not happen.
        return store_E_Unknown;
    }

    // Remove down from current page (recursive).
    node aNode (rPage);
    return aNode.remove (i, rEntry, rPageL, *this, NULL);
}

/*
 * load.
 * Precond: initialized.
 */
storeError OStorePageManager::load (
    const OStorePageKey       &rKey,
    OStoreDirectoryPageObject &rPage)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard(*this);

    // Check precond.
    if (!self::isValid())
        return store_E_InvalidAccess;

    // Setup BTree entry.
    entry e;
    e.m_aKey = rKey;

    // Find NodePage.
    storeError eErrCode = find (e, *m_pNode[0]);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Find Index.
    sal_uInt16 i = m_pNode[0]->find(e), n = m_pNode[0]->usageCount();
    if (!(i < n))
    {
        // Page not present.
        return store_E_NotExists;
    }

    // Check for exact match.
    if (!(e.compare (m_pNode[0]->m_pData[i]) == entry::COMPARE_EQUAL))
    {
        // Page not present.
        return store_E_NotExists;
    }

    // Existing entry. Check address.
    sal_uInt32 nAddr = m_pNode[0]->m_pData[i].m_aLink.m_nAddr;
    if (nAddr == STORE_PAGE_NULL)
    {
        // Page not present.
        return store_E_NotExists;
    }

    // Load page.
    rPage.location (nAddr);
    return load (rPage);
}

/*
 * save.
 * Precond: initialized, writeable.
 */
storeError OStorePageManager::save (
    const OStorePageKey       &rKey,
    OStoreDirectoryPageObject &rPage)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard(*this);

    // Check precond.
    if (!self::isValid())
        return store_E_InvalidAccess;

    if (!base::isWriteable())
        return store_E_AccessViolation;

    // Setup BTree entry.
    entry e;
    e.m_aKey = rKey;

    // Find NodePage.
    storeError eErrCode = find (e, *m_pNode[0], *m_pNode[1], *m_pNode[2]);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Find Index.
    sal_uInt16 i = m_pNode[0]->find(e), n = m_pNode[0]->usageCount();
    if (i < n)
    {
        // Compare entry.
        entry::CompareResult result = e.compare (m_pNode[0]->m_pData[i]);
        OSL_POSTCOND(
            result != entry::COMPARE_LESS,
            "OStorePageManager::save(): find failed");

        // Check result.
        if (result == entry::COMPARE_LESS)
        {
            // Must not happen.
            return store_E_Unknown;
        }

        if (result == entry::COMPARE_EQUAL)
        {
            // Existing entry. Check address.
            sal_uInt32 nAddr = m_pNode[0]->m_pData[i].m_aLink.m_nAddr;
            if (nAddr == STORE_PAGE_NULL)
            {
                // Allocate page.
                eErrCode = base::allocate (rPage);
                if (eErrCode != store_E_None)
                    return eErrCode;

                // Modify page address.
                m_pNode[0]->m_pData[i].m_aLink.m_nAddr = rPage.location();

                // Save modified NodePage.
                node aNode (*m_pNode[0]);
                return save (aNode);
            }
            else
            {
                // Save page.
                rPage.location (nAddr);
                return save (rPage);
            }
        }
    }

    // Allocate.
    eErrCode = base::allocate (rPage);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Insert.
    e.m_aLink.m_nAddr = rPage.location();
    m_pNode[0]->insert (i + 1, e);

    // Save modified NodePage.
    node aNode (*m_pNode[0]);
    return save (aNode);
}

/*
 * attrib [nAttrib = ((nAttrib & ~nMask1) | nMask2)].
 * Precond: initialized.
 */
storeError OStorePageManager::attrib (
    const OStorePageKey &rKey,
    sal_uInt32           nMask1,
    sal_uInt32           nMask2,
    sal_uInt32          &rAttrib)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard(*this);

    // Check precond.
    if (!self::isValid())
        return store_E_InvalidAccess;

    // Setup BTree entry.
    entry e;
    e.m_aKey = rKey;

    // Find NodePage.
    storeError eErrCode = find (e, *m_pNode[0]);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Find Index.
    sal_uInt16 i = m_pNode[0]->find(e), n = m_pNode[0]->usageCount();
    if (!(i < n))
    {
        // Page not present.
        return store_E_NotExists;
    }

    // Check for exact match.
    if (!(e.compare (m_pNode[0]->m_pData[i]) == entry::COMPARE_EQUAL))
    {
        // Page not present.
        return store_E_NotExists;
    }

    // Existing entry.
    e = m_pNode[0]->m_pData[i];
    if (nMask1 != nMask2)
    {
        // Evaluate new attributes.
        sal_uInt32 nAttrib = e.m_nAttrib;

        nAttrib &= ~nMask1;
        nAttrib |=  nMask2;

        if (nAttrib != e.m_nAttrib)
        {
            // Check access mode.
            if (base::isWriteable())
            {
                // Set new attributes.
                e.m_nAttrib = nAttrib;
                m_pNode[0]->m_pData[i] = e;

                // Save modified NodePage.
                node aNode (*m_pNode[0]);
                eErrCode = save (aNode);
            }
            else
            {
                // Access denied.
                eErrCode = store_E_AccessViolation;
            }
        }
    }

    // Obtain current attributes.
    rAttrib = e.m_nAttrib;
    return eErrCode;
}

/*
 * link (insert 'Source' as hardlink to 'Destination').
 * Precond: initialized, writeable.
 */
storeError OStorePageManager::link (
    const OStorePageKey &rSrcKey,
    const OStorePageKey &rDstKey)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard(*this);

    // Check precond.
    if (!self::isValid())
        return store_E_InvalidAccess;

    if (!base::isWriteable())
        return store_E_AccessViolation;

    // Setup 'Destination' BTree entry.
    entry e;
    e.m_aKey = rDstKey;

    // Find 'Destination' NodePage.
    storeError eErrCode = find (e, *m_pNode[0]);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Find 'Destination' Index.
    sal_uInt16 i = m_pNode[0]->find(e), n = m_pNode[0]->usageCount();
    if (!(i < n))
    {
        // Page not present.
        return store_E_NotExists;
    }

    // Check for exact match.
    if (!(e.compare (m_pNode[0]->m_pData[i]) == entry::COMPARE_EQUAL))
    {
        // Page not present.
        return store_E_NotExists;
    }

    // Existing entry. Check address.
    e = m_pNode[0]->m_pData[i];
    if (e.m_aLink.m_nAddr == STORE_PAGE_NULL)
    {
        // Page not present.
        return store_E_NotExists;
    }

    // Setup 'Source' BTree entry.
    e.m_aKey    = rSrcKey;
    e.m_nAttrib = STORE_ATTRIB_ISLINK;

    // Find 'Source' NodePage.
    eErrCode = find (e, *m_pNode[0], *m_pNode[1], *m_pNode[2]);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Find 'Source' Index.
    i = m_pNode[0]->find(e), n = m_pNode[0]->usageCount();
    if (i < n)
    {
        // Compare entry.
        entry::CompareResult result = e.compare (m_pNode[0]->m_pData[i]);
        OSL_POSTCOND(
            result != entry::COMPARE_LESS,
            "OStorePageManager::link(): find failed");

        // Check result.
        if (result == entry::COMPARE_LESS)
        {
            // Must not happen.
            return store_E_Unknown;
        }

        if (result == entry::COMPARE_EQUAL)
        {
            // Existing 'Source' entry.
            return store_E_AlreadyExists;
        }
    }

    // Insert 'Source' entry.
    m_pNode[0]->insert (i + 1, e);

    node aNode (*m_pNode[0]);
    return save (aNode);
}

/*
 * symlink (insert 'Source' DirectoryPage as symlink to 'Destination').
 * Precond: initialized, writeable.
 */
storeError OStorePageManager::symlink (
    const rtl_String    *pSrcPath,
    const rtl_String    *pSrcName,
    const OStorePageKey &rDstKey)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard(*this);

    // Check precond.
    if (!self::isValid())
        return store_E_InvalidAccess;

    if (!base::isWriteable())
        return store_E_AccessViolation;

    // Check 'Source' parameter.
    storeError eErrCode = store_E_InvalidParameter;
    if (!(pSrcPath && pSrcName))
        return eErrCode;

    // Setup 'Source' page key.
    OStorePageKey aSrcKey;
    eErrCode = OStorePageNameBlock::namei (pSrcPath, pSrcName, aSrcKey);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Setup 'Source' BTree entry.
    entry e;
    e.m_aKey = aSrcKey;

    // Find 'Source' NodePage.
    eErrCode = find (e, *m_pNode[0], *m_pNode[1], *m_pNode[2]);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Find 'Source' Index.
    sal_uInt16 i = m_pNode[0]->find(e), n = m_pNode[0]->usageCount();
    if (i < n)
    {
        // Compare entry.
        entry::CompareResult result = e.compare (m_pNode[0]->m_pData[i]);
        OSL_POSTCOND(
            result != entry::COMPARE_LESS,
            "OStorePageManager::symlink(): find failed");

        // Check result.
        if (result == entry::COMPARE_LESS)
        {
            // Must not happen.
            return store_E_Unknown;
        }

        if (result == entry::COMPARE_EQUAL)
        {
            // Existing 'Source' entry.
            return store_E_AlreadyExists;
        }
    }

    // Initialize directory page buffer.
    if (m_pDirect)
        m_pDirect->initialize();
    if (!m_pDirect)
        m_pDirect = new(m_nPageSize) inode(m_nPageSize);
    if (!m_pDirect)
        return store_E_OutOfMemory;

    // Setup as 'Source' directory page.
    m_pDirect->m_aNameBlock.m_aKey = aSrcKey;
    rtl_copyMemory (
        &m_pDirect->m_aNameBlock.m_pData[0],
        pSrcName->buffer, pSrcName->length);

    // Store 'Destination' page key.
    OStorePageKey aDstKey (rDstKey);
#ifdef OSL_BIGENDIAN
    aDstKey.swap(); // Swap to external representation.
#endif /* OSL_BIGENDIAN */
    rtl_copyMemory (&m_pDirect->m_pData[0], &aDstKey, sizeof(aDstKey));

    // Mark 'Source' as symbolic link to 'Destination'.
    OStoreDirectoryPageObject aPage (*m_pDirect);
    aPage.attrib (STORE_ATTRIB_ISLINK);
    aPage.dataLength (sizeof (aDstKey));

    // Allocate and save 'Source' directory page.
    eErrCode = base::allocate (aPage);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Insert 'Source' entry.
    e.m_aLink.m_nAddr = aPage.location();
    m_pNode[0]->insert (i + 1, e);

    // Save modified NodePage.
    node aNode (*m_pNode[0]);
    return save (aNode);
}

/*
 * rename.
 * Precond: initialized, writeable.
 */
storeError OStorePageManager::rename (
    const OStorePageKey &rSrcKey,
    const rtl_String    *pDstPath,
    const rtl_String    *pDstName)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard(*this);

    // Check precond.
    if (!self::isValid())
        return store_E_InvalidAccess;

    if (!base::isWriteable())
        return store_E_AccessViolation;

    // Check 'Destination' parameter.
    storeError eErrCode = store_E_InvalidParameter;
    if (!(pDstPath && pDstName))
        return eErrCode;

    // Setup 'Destination' page key.
    OStorePageKey aDstKey;
    eErrCode = OStorePageNameBlock::namei (pDstPath, pDstName, aDstKey);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Setup 'Source' BTree entry.
    entry e;
    e.m_aKey = rSrcKey;

    // Find 'Source' NodePage.
    eErrCode = find (e, *m_pNode[0]);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Find 'Source' Index.
    sal_uInt16 i = m_pNode[0]->find(e), n = m_pNode[0]->usageCount();
    if (!(i < n))
    {
        // Page not present.
        return store_E_NotExists;
    }

    // Check for exact match.
    if (!(e.compare (m_pNode[0]->m_pData[i]) == entry::COMPARE_EQUAL))
    {
        // Page not present.
        return store_E_NotExists;
    }

    // Existing 'Source' entry. Check address.
    e = m_pNode[0]->m_pData[i];
    if (e.m_aLink.m_nAddr == STORE_PAGE_NULL)
    {
        // Page not present.
        return store_E_NotExists;
    }

    // Check for hardlink.
    if (!(e.m_nAttrib & STORE_ATTRIB_ISLINK))
    {
        // Check directory page buffer.
        if (!m_pDirect)
            m_pDirect = new(m_nPageSize) inode(m_nPageSize);
        if (!m_pDirect)
            return store_E_OutOfMemory;

        // Load directory page.
        OStoreDirectoryPageObject aPage (*m_pDirect);
        aPage.location (e.m_aLink.m_nAddr);

        eErrCode = base::load (aPage);
        if (eErrCode != store_E_None)
            return eErrCode;

        // Check for directory.
        if (aPage.attrib() & STORE_ATTRIB_ISDIR)
        {
            // Ugly, but necessary (backward compatibility).
            aDstKey.m_nLow = OStorePageGuard::crc32 (aDstKey.m_nLow, "/", 1);
        }
    }

    // Let 'Source' entry be 'Destination' entry.
    e.m_aKey = aDstKey;

    // Find 'Destination' NodePage.
    eErrCode = find (e, *m_pNode[0], *m_pNode[1], *m_pNode[2]);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Find 'Destination' Index.
    i = m_pNode[0]->find(e), n = m_pNode[0]->usageCount();
    if (i < n)
    {
        // Compare entry.
        entry::CompareResult result = e.compare (m_pNode[0]->m_pData[i]);
        OSL_POSTCOND(
            result != entry::COMPARE_LESS,
            "OStorePageManager::rename(): find failed");

        // Check result.
        if (result == entry::COMPARE_LESS)
        {
            // Must not happen.
            return store_E_Unknown;
        }

        if (result == entry::COMPARE_EQUAL)
        {
            // Existing 'Destination' entry.
            return store_E_AlreadyExists;
        }
    }

    // Insert 'Destination' entry.
    node aNode (*m_pNode[0]);
    m_pNode[0]->insert (i + 1, e);

    eErrCode = save (aNode);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Check for hardlink.
    if (!(e.m_nAttrib & STORE_ATTRIB_ISLINK))
    {
        // Setup 'Destination' NameBlock.
        sal_Int32 nDstLen = pDstName->length;
        rtl_copyMemory (
            &m_pDirect->m_aNameBlock.m_pData[0],
            pDstName->buffer, nDstLen);
        rtl_zeroMemory (
            &m_pDirect->m_aNameBlock.m_pData[nDstLen],
            STORE_MAXIMUM_NAMESIZE - nDstLen);
        m_pDirect->m_aNameBlock.m_aKey = e.m_aKey;

        // Save directory page.
        OStoreDirectoryPageObject aPage (*m_pDirect);
        aPage.location (e.m_aLink.m_nAddr);

        eErrCode = base::save (aPage);
        if (eErrCode != store_E_None)
            return eErrCode;
    }

    // Remove 'Source' entry.
    e.m_aKey = rSrcKey;
    return remove (e, *m_pNode[0], *m_pNode[1]);
}

/*
 * remove.
 * Precond: initialized, writeable.
 */
storeError OStorePageManager::remove (const OStorePageKey &rKey)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard(*this);

    // Check precond.
    if (!self::isValid())
        return store_E_InvalidAccess;

    if (!base::isWriteable())
        return store_E_AccessViolation;

    // Setup BTree entry.
    entry e;
    e.m_aKey = rKey;

    // Find NodePage.
    storeError eErrCode = find (e, *m_pNode[0]);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Find Index.
    sal_uInt16 i = m_pNode[0]->find(e), n = m_pNode[0]->usageCount();
    if (!(i < n))
    {
        // Page not present.
        return store_E_NotExists;
    }

    // Check for exact match.
    if (!(e.compare (m_pNode[0]->m_pData[i]) == entry::COMPARE_EQUAL))
    {
        // Page not present.
        return store_E_NotExists;
    }

    // Existing entry. Check address.
    e = m_pNode[0]->m_pData[i];
    if (e.m_aLink.m_nAddr == STORE_PAGE_NULL)
    {
        // Page not present.
        return store_E_NotExists;
    }

    // Check for hardlink.
    if (!(e.m_nAttrib & STORE_ATTRIB_ISLINK))
    {
        // Check directory page buffer.
        if (!m_pDirect)
            m_pDirect = new(m_nPageSize) inode(m_nPageSize);
        if (!m_pDirect)
            return store_E_OutOfMemory;

        // Load directory page.
        OStoreDirectoryPageObject aPage (*m_pDirect);
        aPage.location (e.m_aLink.m_nAddr);

        eErrCode = base::load (aPage);
        if (eErrCode != store_E_None)
            return eErrCode;

        // Acquire page write access.
        OStorePageDescriptor aDescr (m_pDirect->m_aDescr);
        eErrCode = base::acquirePage (aDescr, store_AccessReadWrite);
        if (eErrCode != store_E_None)
            return eErrCode;

        // Check for symbolic link.
        if (!(aPage.attrib() & STORE_ATTRIB_ISLINK))
        {
            // Ordinary inode. Determine 'Data' scope.
            inode::ChunkScope eScope = m_pDirect->scope (aPage.dataLength());
            if (eScope == inode::SCOPE_EXTERNAL)
            {
                // External 'Data' scope. Check data page buffer.
                if (!m_pData)
                    m_pData = new(m_nPageSize) data(m_nPageSize);
                if (!m_pData)
                    return store_E_OutOfMemory;

                // Truncate all external data pages.
                OStoreDataPageObject aData (*m_pData);
                eErrCode = aPage.truncate (
                    0, m_pLink[0], m_pLink[1], m_pLink[2], aData, *this);
                if (eErrCode != store_E_None)
                    return eErrCode;
            }

            // Truncate internal data page.
            rtl_zeroMemory (&m_pDirect->m_pData[0], m_pDirect->capacity());
            aPage.dataLength (0);
        }

        // Release page write access.
        eErrCode = base::releasePage (aDescr, store_AccessReadWrite);

        // Release and free directory page.
        eErrCode = base::free (aPage);
    }

    // Remove entry.
    return remove (e, *m_pNode[0], *m_pNode[1]);
}

/*
 * iterate.
 * Precond: initialized.
 * ToDo: skip hardlink entries.
 */
storeError OStorePageManager::iterate (
    OStorePageKey    &rKey,
    OStorePageObject &rPage,
    sal_uInt32       &rAttrib)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard(*this);

    // Check precond.
    if (!self::isValid())
        return store_E_InvalidAccess;

    // Setup BTree entry.
    entry e;
    e.m_aKey = rKey;

    // Find NodePage.
    storeError eErrCode = find (e, *m_pNode[0]);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Find Index.
    sal_uInt16 i = m_pNode[0]->find(e), n = m_pNode[0]->usageCount();
    if (!(i < n))
    {
        // Not found.
        return store_E_NotExists;
    }

    // Compare entry.
    entry::CompareResult result = e.compare (m_pNode[0]->m_pData[i]);
    OSL_POSTCOND(
        result != entry::COMPARE_LESS,
        "OStorePageManager::iterate(): find failed");

    // Check result.
    if (result == entry::COMPARE_LESS)
    {
        // Must not happen.
        return store_E_Unknown;
    }

    // GreaterEqual. Found next entry.
    e = m_pNode[0]->m_pData[i];

    // Setup result.
    rKey    = e.m_aKey;
    rAttrib = e.m_nAttrib;

    rPage.location (e.m_aLink.m_nAddr);

    // Done.
    return store_E_None;
}

/*
 * RebuildContext.
 */
struct RebuildContext
{
    /** Representation.
    */
    rtl::Reference<OStorePageBIOS> m_xBIOS;
    OStorePageBIOS::ScanContext    m_aCtx;

    /** Construction.
     */
    RebuildContext (void)
        : m_xBIOS (new OStorePageBIOS())
    {}

    /** initialize (PageBIOS and ScanContext).
    */
    storeError initialize (ILockBytes *pLockBytes, sal_uInt32 nMagic = 0)
    {
        storeError eErrCode = store_E_InvalidParameter;
        if (pLockBytes)
        {
            m_xBIOS->initialize (pLockBytes, store_AccessReadOnly);
            eErrCode = m_xBIOS->scanBegin (m_aCtx, nMagic);
        }
        return eErrCode;
    }

    /** initialize (ScanContext).
    */
    storeError initialize (sal_uInt32 nMagic = 0)
    {
        return m_xBIOS->scanBegin (m_aCtx, nMagic);
    }

    /** load (next ScanContext matching page).
    */
    storeError load (OStorePageObject &rPage)
    {
        if (m_aCtx.isValid())
            return m_xBIOS->scanNext (m_aCtx, rPage);
        else
            return store_E_CantSeek;
    }

    /** getPageSize.
     */
    storeError getPageSize (sal_uInt16 &rnPageSize)
    {
        return m_xBIOS->getPageSize (rnPageSize);
    }
};

/*
 * rebuild.
 * Precond: none.
 */
storeError OStorePageManager::rebuild (
    ILockBytes *pSrcLB, ILockBytes *pDstLB)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard(*this);

    // Check arguments.
    storeError eErrCode = store_E_InvalidParameter;
    if (!(pSrcLB && pDstLB))
        return eErrCode;

    // Initialize 'Source' rebuild context.
    RebuildContext aCtx;
    eErrCode = aCtx.initialize (pSrcLB, STORE_MAGIC_DIRECTORYPAGE);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Obtain 'Source' page size.
    sal_uInt16 nPageSize = 0;
    eErrCode = aCtx.getPageSize (nPageSize);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Prevent flush() attempt from daemon during exclusive access.
    m_xDaemon.clear();

    // Initialize as 'Destination' with 'Source' page size.
    eErrCode = self::initialize (pDstLB, store_AccessCreate, nPageSize);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Initialize directory and data page buffers.
    if (!m_pDirect)
        m_pDirect = new(m_nPageSize) inode(m_nPageSize);
    if (!m_pData)
        m_pData = new(m_nPageSize) data(m_nPageSize);
    if (!(m_pDirect && m_pData))
        return store_E_OutOfMemory;

    // Initialize 'Source' directory page.
    inode *pDirect = new(m_nPageSize) inode(m_nPageSize);
    if (!pDirect)
        return store_E_OutOfMemory;

    // Scan 'Source' directory pages.
    OStoreDirectoryPageObject aSrcPage (*pDirect);
    while ((eErrCode = aCtx.load(aSrcPage)) == store_E_None)
    {
        // Obtain page key and data length.
        OStorePageKey aKey (pDirect->m_aNameBlock.m_aKey);
        sal_uInt32 nDataLen = pDirect->m_aDataBlock.m_nDataLen;

        // Determine data page scope.
        inode::ChunkScope eScope = pDirect->scope (nDataLen);
        if (eScope == inode::SCOPE_INTERNAL)
        {
            // Internal scope. Just insert directory node.
            eErrCode = save (aKey, aSrcPage);
            if (eErrCode != store_E_None)
                break;
        }
        else
        {
            // External scope.
            OStoreDirectoryPageObject aDstPage (*m_pDirect);
            rtl_copyMemory (m_pDirect, pDirect, m_nPageSize);

            m_pDirect->m_aDataBlock.initialize();
            m_pDirect->m_aDataBlock.m_nDataLen = m_pDirect->capacity();

            // Insert 'Destination' directory page.
            eErrCode = save (aKey, aDstPage);
            if (eErrCode != store_E_None)
                break;

            // Determine data page count.
            inode::ChunkDescriptor aDescr (
                nDataLen - m_pDirect->capacity(), m_pData->capacity());

            sal_uInt32 i, n = aDescr.m_nPage;
            if (aDescr.m_nOffset) n += 1;

            // Copy data pages.
            OStoreDataPageObject aData (*m_pData);
            for (i = 0; i < n; i++)
            {
                // Re-initialize data page size.
                m_pData->m_aDescr.m_nSize = m_nPageSize;

                // Read 'Source' data page.
                OStorePageBIOS &rBIOS  = *(aCtx.m_xBIOS);
                osl::Mutex     &rMutex = rBIOS;

                eErrCode = aSrcPage.get (
                    i, m_pLink[0], m_pLink[1], m_pLink[2],
                    aData, rBIOS, &rMutex);
                if (eErrCode != store_E_None)
                    continue;

                // Write 'Destination' data page.
                eErrCode = aDstPage.put (
                    i, m_pLink[0], m_pLink[1], m_pLink[2],
                    aData, *this, NULL);
            }

            // Update 'Destination' directory page.
            m_pDirect->m_aDataBlock.m_nDataLen = nDataLen;
            eErrCode = base::save (aDstPage);
        }
    }

    // Save directory scan results.
    flush();

    // Scan 'Source' BTree nodes.
    page *pNode = new(m_nPageSize) page(m_nPageSize);
    node aNode (*pNode);
    entry e;

    aCtx.initialize (STORE_MAGIC_BTREENODE);
    while ((eErrCode = aCtx.load(aNode)) == store_E_None)
    {
        // Check for leaf node.
        if (pNode->depth() == 0)
        {
            sal_uInt16 i, n = pNode->usageCount();
            for (i = 0; i < n; i++)
            {
                e = pNode->m_pData[i];
                if (e.m_nAttrib & STORE_ATTRIB_ISLINK)
                {
                    // Hard link.
                    aSrcPage.location (e.m_aLink.m_nAddr);
                    pDirect->m_aDescr.m_nSize = m_nPageSize;

                    eErrCode = aCtx.m_xBIOS->load (aSrcPage);
                    if (eErrCode == store_E_None)
                    {
                        OStorePageKey aDstKey (pDirect->m_aNameBlock.m_aKey);
                        eErrCode = link (e.m_aKey, aDstKey);
                    }
                    e.m_nAttrib &= ~STORE_ATTRIB_ISLINK;
                }

                if (e.m_nAttrib)
                {
                    // Ordinary attributes.
                    sal_uInt32 nAttrib = 0;
                    eErrCode = attrib (e.m_aKey, 0, e.m_nAttrib, nAttrib);
                }
            }
        }
    }

    // Save BTree node scan results.
    flush();

    // Cleanup.
    delete pDirect;
    delete pNode;

    // Done.
    return store_E_None;
}
