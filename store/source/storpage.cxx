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

#include "storpage.hxx"

#include <sal/types.h>
#include <sal/log.hxx>
#include <rtl/string.h>
#include <osl/mutex.hxx>

#include <store/types.h>

#include "storbase.hxx"
#include "stordata.hxx"
#include "stortree.hxx"

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
OStorePageManager::OStorePageManager()
{
}

/*
 * ~OStorePageManager.
 */
OStorePageManager::~OStorePageManager()
{
}

/*
 * isKindOf.
 */
bool OStorePageManager::isKindOf (sal_uInt32 nTypeId)
{
    return (nTypeId == m_nTypeId);
}

/*
 * initialize (two-phase construction).
 * Precond: none.
 */
storeError OStorePageManager::initialize (
    ILockBytes *    pLockBytes,
    storeAccessMode eAccessMode,
    sal_uInt16 &    rnPageSize)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard(*this);

    // Check arguments.
    if (!pLockBytes)
        return store_E_InvalidParameter;

    // Initialize base.
    storeError eErrCode = base::initialize (pLockBytes, eAccessMode, rnPageSize);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Check for (not) writeable.
    if (!base::isWriteable())
    {
        // Readonly. Load RootNode.
        return base::loadObjectAt (m_aRoot, rnPageSize);
    }

    // Writeable. Load or Create RootNode.
    eErrCode = m_aRoot.loadOrCreate (rnPageSize, *this);
    if (eErrCode == store_E_Pending)
    {
        // Creation notification.
        PageHolderObject< page > xRoot (m_aRoot.get());

        // Pre-allocate left most entry (ugly, but we can't insert to left).
        OStorePageKey aKey (rtl_crc32 (0, "/", 1), 0);
        xRoot->insert (0, entry(aKey));

        // Save RootNode.
        eErrCode = base::saveObjectAt (m_aRoot, rnPageSize);
    }

    // Done.
    return eErrCode;
}

/*
 * find_lookup (w/o split()).
 * Internal: Precond: initialized, readable, exclusive access.
 */
storeError OStorePageManager::find_lookup (
    OStoreBTreeNodeObject & rNode,
    sal_uInt16 &            rIndex,
    OStorePageKey const &   rKey)
{
    // Find Node and Index.
    storeError eErrCode = m_aRoot.find_lookup (rNode, rIndex, rKey, *this);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Greater or Equal.
    PageHolderObject< page > xPage (rNode.get());
    SAL_WARN_IF(rIndex >= xPage->usageCount(), "store", "store::PageManager::find_lookup(): logic error");
    entry e (xPage->m_pData[rIndex]);

    // Check for exact match.
    if (e.compare(entry(rKey)) != entry::COMPARE_EQUAL)
    {
        // Page not present.
        return store_E_NotExists;
    }

    // Check address.
    if (e.m_aLink.location() == STORE_PAGE_NULL)
    {
        // Page not present.
        return store_E_NotExists;
    }

    return store_E_None;
}

/*
 * remove_Impl (possibly down from root).
 * Internal: Precond: initialized, writeable, exclusive access.
 */

storeError OStorePageManager::remove_Impl (entry & rEntry)
{
    OStoreBTreeNodeObject aNode (m_aRoot.get());

    // Check current page index.
    PageHolderObject< page > xPage (aNode.get());
    sal_uInt16 i = xPage->find (rEntry), n = xPage->usageCount();
    if (i >= n)
    {
        // Path to entry not exists (Must not happen(?)).
        return store_E_NotExists;
    }

    // Compare entry.
    entry::CompareResult result = rEntry.compare (xPage->m_pData[i]);

    // Iterate down until equal match.
    while ((result == entry::COMPARE_GREATER) && (xPage->depth() > 0))
    {
        // Check link address.
        sal_uInt32 const nAddr = xPage->m_pData[i].m_aLink.location();
        if (nAddr == STORE_PAGE_NULL)
        {
            // Path to entry not exists (Must not happen(?)).
            return store_E_NotExists;
        }

        // Load link page.
        storeError eErrCode = loadObjectAt (aNode, nAddr);
        if (eErrCode != store_E_None)
            return eErrCode;

        PageHolderObject< page > xNext (aNode.get());
        xNext.swap (xPage);

        // Check index.
        i = xPage->find (rEntry);
        n = xPage->usageCount();
        if (i >= n)
        {
            // Path to entry not exists (Must not happen(?)).
            return store_E_NotExists;
        }

        // Compare entry.
        result = rEntry.compare (xPage->m_pData[i]);
    }

    SAL_WARN_IF(
        result == entry::COMPARE_LESS,
        "store",
        "OStorePageManager::remove(): find failed");

    // Check entry comparison.
    if (result == entry::COMPARE_LESS)
    {
        // Must not happen.
        return store_E_Unknown;
    }

    // Remove down from current page (recursive).
    return aNode.remove (i, rEntry, *this);
}

/*
 * namei.
 * Precond: none (static).
 */
storeError OStorePageManager::namei (
    const rtl_String *pPath, const rtl_String *pName, OStorePageKey &rKey)
{
    // Check parameter.
    if (!(pPath && pName))
        return store_E_InvalidParameter;

    // Check name length.
    if (pName->length >= STORE_MAXIMUM_NAMESIZE)
        return store_E_NameTooLong;

    // Transform pathname into key.
    rKey.m_nLow  = store::htonl(rtl_crc32 (0, pName->buffer, pName->length));
    rKey.m_nHigh = store::htonl(rtl_crc32 (0, pPath->buffer, pPath->length));

    // Done.
    return store_E_None;
}

/*
 * iget.
 * Precond: initialized.
 */
storeError OStorePageManager::iget (
    OStoreDirectoryPageObject & rPage,
    sal_uInt32                  nAttrib,
    const rtl_String          * pPath,
    const rtl_String          * pName,
    storeAccessMode             eMode)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard(*this);

    // Check precond.
    if (!self::isValid())
        return store_E_InvalidAccess;

    // Setup inode page key.
    OStorePageKey aKey;
    storeError eErrCode = namei (pPath, pName, aKey);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Check for directory.
    if (nAttrib & STORE_ATTRIB_ISDIR)
    {
        // Ugly, but necessary (backward compatibility).
        aKey.m_nLow = store::htonl(rtl_crc32 (store::ntohl(aKey.m_nLow), "/", 1));
    }

    // Load inode page.
    eErrCode = load_dirpage_Impl (aKey, rPage);
    if (eErrCode != store_E_None)
    {
        // Check mode and reason.
        if (eErrCode != store_E_NotExists)
            return eErrCode;

        if (eMode == storeAccessMode::ReadWrite)
            return store_E_NotExists;
        if (eMode == storeAccessMode::ReadOnly)
            return store_E_NotExists;

        if (!base::isWriteable())
            return store_E_AccessViolation;

        // Create inode page.
        eErrCode = rPage.construct< inode >(base::allocator());
        if (eErrCode != store_E_None)
            return eErrCode;

        // Setup inode nameblock.
        PageHolderObject< inode > xPage (rPage.get());

        rPage.key (aKey);
        rPage.attrib (nAttrib);

        memcpy (
            &(xPage->m_aNameBlock.m_pData[0]),
            pName->buffer, pName->length);

        // Save inode page.
        eErrCode = save_dirpage_Impl (aKey, rPage);
        if (eErrCode != store_E_None)
            return eErrCode;
    }

    // Check for symbolic link.
    if (rPage.attrib() & STORE_ATTRIB_ISLINK)
    {
        // Obtain 'Destination' page key.
        PageHolderObject< inode > xPage (rPage.get());
        OStorePageKey aDstKey;
        memcpy (&aDstKey, &(xPage->m_pData[0]), sizeof(aDstKey));

        // Load 'Destination' inode.
        eErrCode = load_dirpage_Impl (aDstKey, rPage);
        if (eErrCode != store_E_None)
            return eErrCode;
    }

    // Done.
    return store_E_None;
}

/*
 * iterate.
 * Precond: initialized.
 * ToDo: skip hardlink entries.
 */
storeError OStorePageManager::iterate (
    OStorePageKey &  rKey,
    OStorePageLink & rLink,
    sal_uInt32 &     rAttrib)
{
    // Acquire exclusive access.
    osl::MutexGuard aGuard(*this);

    // Check precond.
    if (!self::isValid())
        return store_E_InvalidAccess;

    // Find NodePage and Index.
    OStoreBTreeNodeObject aNode;
    sal_uInt16 i = 0;
    storeError eErrCode = m_aRoot.find_lookup (aNode, i, rKey, *this);
    if (eErrCode != store_E_None)
        return eErrCode;

    // GreaterEqual. Found next entry.
    PageHolderObject< page > xNode (aNode.get());
    entry e (xNode->m_pData[i]);

    // Setup result.
    rKey    = e.m_aKey;
    rLink   = e.m_aLink;
    rAttrib = store::ntohl(e.m_nAttrib);

    // Done.
    return store_E_None;
}

/*
 * load => private: iget() @@@
 * Internal: Precond: initialized, exclusive access.
 */
storeError OStorePageManager::load_dirpage_Impl (
    const OStorePageKey       &rKey,
    OStoreDirectoryPageObject &rPage)
{
    // Find Node and Index.
    OStoreBTreeNodeObject aNode;
    sal_uInt16 i = 0;
    storeError eErrCode = find_lookup (aNode, i, rKey);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Existing entry. Load page.
    PageHolderObject< page > xNode (aNode.get());
    entry e (xNode->m_pData[i]);
    return loadObjectAt (rPage, e.m_aLink.location());
}

/*
 * save => private: iget(), rebuild() @@@
 * Internal: Precond: initialized, writeable, exclusive access.
 */
storeError OStorePageManager::save_dirpage_Impl (
    const OStorePageKey       &rKey,
    OStoreDirectoryPageObject &rPage)
{
    // Find NodePage and Index.
    node aNode;
    sal_uInt16 i = 0;

    storeError eErrCode = m_aRoot.find_insert (aNode, i, rKey, *this);
    PageHolderObject< page > xNode (aNode.get());
    if (eErrCode != store_E_None)
    {
        if (eErrCode != store_E_AlreadyExists)
            return eErrCode;

        // Existing entry.
        entry e (xNode->m_pData[i]);
        if (e.m_aLink.location() != STORE_PAGE_NULL)
        {
            // Save page to existing location.
            return saveObjectAt (rPage, e.m_aLink.location());
        }

        // Allocate page.
        eErrCode = base::allocate (rPage);
        if (eErrCode != store_E_None)
            return eErrCode;

        // Update page location.
        xNode->m_pData[i].m_aLink = rPage.location();

        // Save modified NodePage.
        return saveObjectAt (aNode, aNode.location());
    }

    // Allocate page.
    eErrCode = base::allocate (rPage);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Insert.
    OStorePageLink aLink (rPage.location());
    xNode->insert (i + 1, entry (rKey, aLink));

    // Save modified NodePage.
    return saveObjectAt (aNode, aNode.location());
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

    // Find NodePage and index.
    OStoreBTreeNodeObject aNodePage;
    sal_uInt16 i = 0;
    storeError eErrCode = find_lookup (aNodePage, i, rKey);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Existing entry.
    PageHolderObject< page > xNodePage (aNodePage.get());
    entry e (xNodePage->m_pData[i]);

    // Check for (not a) hardlink.
    if (!(store::ntohl(e.m_nAttrib) & STORE_ATTRIB_ISLINK))
    {
        // Load directory page.
        OStoreDirectoryPageObject aPage;
        eErrCode = base::loadObjectAt (aPage, e.m_aLink.location());
        if (eErrCode != store_E_None)
            return eErrCode;

        inode_holder_type xNode (aPage.get());

        // Acquire page write access.
        OStorePageDescriptor aDescr (xNode->m_aDescr);
        eErrCode = base::acquirePage (aDescr, storeAccessMode::ReadWrite);
        if (eErrCode != store_E_None)
            return eErrCode;

        // Check for symbolic link.
        if (!(aPage.attrib() & STORE_ATTRIB_ISLINK))
        {
            // Ordinary inode. Determine 'Data' scope.
            inode::ChunkScope eScope = xNode->scope (aPage.dataLength());
            if (eScope == inode::SCOPE_EXTERNAL)
            {
                // External 'Data' scope. Truncate all external data pages.
                eErrCode = aPage.truncate (0, *this);
                if (eErrCode != store_E_None)
                    return eErrCode;
            }

            // Truncate internal data page.
            memset (&(xNode->m_pData[0]), 0, xNode->capacity());
            aPage.dataLength (0);
        }

        // Release page write access.
        base::releasePage (aDescr);

        // Release and free directory page.
        (void)base::free (aPage.location());
    }

    // Remove entry.
    return remove_Impl (e);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
