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

#include "sal/types.h"
#include "sal/log.hxx"
#include "rtl/string.h"
#include "rtl/ref.hxx"
#include "osl/diagnose.h"
#include "osl/mutex.hxx"

#include "store/types.h"

#include "object.hxx"
#include "lockbyte.hxx"

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
    if (!(i < n))
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
        i = xPage->find (rEntry), n = xPage->usageCount();
        if (!(i < n))
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
    if (!(pName->length < STORE_MAXIMUM_NAMESIZE))
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

        if (eMode == store_AccessReadWrite)
            return store_E_NotExists;
        if (eMode == store_AccessReadOnly)
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

    // Find NodePage and index.
    OStoreBTreeNodeObject aNode;
    sal_uInt16 i = 0;
    storeError eErrCode = find_lookup (aNode, i, rKey);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Existing entry.
    PageHolderObject< page > xNode (aNode.get());
    entry e (xNode->m_pData[i]);
    if (nMask1 != nMask2)
    {
        // Evaluate new attributes.
        sal_uInt32 nAttrib = store::ntohl(e.m_nAttrib);

        nAttrib &= ~nMask1;
        nAttrib |=  nMask2;

        if (store::htonl(nAttrib) != e.m_nAttrib)
        {
            // Check access mode.
            if (base::isWriteable())
            {
                // Set new attributes.
                e.m_nAttrib = store::htonl(nAttrib);
                xNode->m_pData[i] = e;

                // Save modified NodePage.
                eErrCode = saveObjectAt (aNode, aNode.location());
            }
            else
            {
                // Access denied.
                eErrCode = store_E_AccessViolation;
            }
        }
    }

    // Obtain current attributes.
    rAttrib = store::ntohl(e.m_nAttrib);
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

    // Find 'Destination' NodePage and Index.
    OStoreBTreeNodeObject aDstNode;
    sal_uInt16 i = 0;
    storeError eErrCode = find_lookup (aDstNode, i, rDstKey);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Existing 'Destination' entry.
    PageHolderObject< page > xDstNode (aDstNode.get());
    entry e (xDstNode->m_pData[i]);
    OStorePageLink aDstLink (e.m_aLink);

    // Find 'Source' NodePage and Index.
    OStoreBTreeNodeObject aSrcNode;
    eErrCode = m_aRoot.find_insert (aSrcNode, i, rSrcKey, *this);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Insert 'Source' entry.
    PageHolderObject< page > xSrcNode (aSrcNode.get());
    xSrcNode->insert (i + 1, entry (rSrcKey, aDstLink, STORE_ATTRIB_ISLINK));
    return saveObjectAt (aSrcNode, aSrcNode.location());
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
    eErrCode = namei (pSrcPath, pSrcName, aSrcKey);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Find 'Source' NodePage and Index.
    OStoreBTreeNodeObject aSrcNode;
    sal_uInt16 i = 0;
    eErrCode = m_aRoot.find_insert (aSrcNode, i, aSrcKey, *this);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Initialize directory page.
    OStoreDirectoryPageObject aPage;
    eErrCode = aPage.construct< inode >(base::allocator());
    if (eErrCode != store_E_None)
        return eErrCode;

    // Setup as 'Source' directory page.
    inode_holder_type xNode (aPage.get());
    aPage.key (aSrcKey);
    memcpy (
        &(xNode->m_aNameBlock.m_pData[0]),
        pSrcName->buffer, pSrcName->length);

    // Store 'Destination' page key.
    OStorePageKey aDstKey (rDstKey);
    memcpy (&(xNode->m_pData[0]), &aDstKey, sizeof(aDstKey));

    // Mark 'Source' as symbolic link to 'Destination'.
    aPage.attrib (STORE_ATTRIB_ISLINK);
    aPage.dataLength (sal_uInt32(sizeof(aDstKey)));

    // Allocate and save 'Source' directory page.
    eErrCode = base::allocate (aPage);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Insert 'Source' entry.
    PageHolderObject< page > xSrcNode (aSrcNode.get());
    OStorePageLink aSrcLink (aPage.location());
    xSrcNode->insert (i + 1, entry(aSrcKey, aSrcLink));

    // Save modified NodePage.
    return saveObjectAt (aSrcNode, aSrcNode.location());
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
    eErrCode = namei (pDstPath, pDstName, aDstKey);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Find 'Source' NodePage and Index.
    OStoreBTreeNodeObject aSrcNode;
    sal_uInt16 i = 0;
    eErrCode = find_lookup (aSrcNode, i, rSrcKey);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Existing 'Source' entry.
    PageHolderObject< page > xSrcNode (aSrcNode.get());
    entry e (xSrcNode->m_pData[i]);

    // Check for (not a) hardlink.
    OStoreDirectoryPageObject aPage;
    if (!(store::ntohl(e.m_nAttrib) & STORE_ATTRIB_ISLINK))
    {
        // Load directory page.
        eErrCode = base::loadObjectAt (aPage, e.m_aLink.location());
        if (eErrCode != store_E_None)
            return eErrCode;

        // Check for directory.
        if (aPage.attrib() & STORE_ATTRIB_ISDIR)
        {
            // Ugly, but necessary (backward compatibility).
            aDstKey.m_nLow = store::htonl(rtl_crc32 (store::ntohl(aDstKey.m_nLow), "/", 1));
        }
    }

    // Let 'Source' entry be 'Destination' entry.
    e.m_aKey = aDstKey;

    // Find 'Destination' NodePage and Index.
    OStoreBTreeNodeObject aDstNode;
    eErrCode = m_aRoot.find_insert (aDstNode, i, e.m_aKey, *this);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Insert 'Destination' entry.
    PageHolderObject< page > xDstNode (aDstNode.get());
    xDstNode->insert (i + 1, e);

    eErrCode = saveObjectAt (aDstNode, aDstNode.location());
    if (eErrCode != store_E_None)
        return eErrCode;

    // Check for (not a) hardlink.
    if (!(store::ntohl(e.m_nAttrib) & STORE_ATTRIB_ISLINK))
    {
        // Modify 'Source' directory page.
        inode_holder_type xNode (aPage.get());

        // Setup 'Destination' NameBlock.
        sal_Int32 nDstLen = pDstName->length;
        memcpy (
            &(xNode->m_aNameBlock.m_pData[0]),
            pDstName->buffer, pDstName->length);
        memset (
            &(xNode->m_aNameBlock.m_pData[nDstLen]),
            0, STORE_MAXIMUM_NAMESIZE - nDstLen);
        aPage.key (e.m_aKey);

        // Save directory page.
        eErrCode = base::saveObjectAt (aPage, e.m_aLink.location());
        if (eErrCode != store_E_None)
            return eErrCode;
    }

    // Remove 'Source' entry.
    e.m_aKey = rSrcKey;
    return remove_Impl (e);
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
        eErrCode = base::acquirePage (aDescr, store_AccessReadWrite);
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
        base::free (aPage.location());
    }

    // Remove entry.
    return remove_Impl (e);
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
    sal_uInt16                     m_nPageSize;

    /** Construction.
     */
    RebuildContext()
        : m_xBIOS     (new OStorePageBIOS()),
          m_nPageSize (0)
    {}

    /** initialize (PageBIOS and ScanContext).
    */
    storeError initialize (ILockBytes *pLockBytes, sal_uInt32 nMagic = 0)
    {
        storeError eErrCode = store_E_InvalidParameter;
        if (pLockBytes)
        {
            m_xBIOS->initialize (pLockBytes, store_AccessReadOnly, m_nPageSize);
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
    rtl::Reference<OStorePageBIOS> xSrcBIOS (aCtx.m_xBIOS);

    // Initialize as 'Destination' with 'Source' page size.
    eErrCode = self::initialize (pDstLB, store_AccessCreate, aCtx.m_nPageSize);
    if (eErrCode != store_E_None)
        return eErrCode;

    // Pass One: Scan 'Source' directory pages.
    {
        // Scan 'Source' directory pages.
        OStoreDirectoryPageObject aSrcPage;
        while ((eErrCode = aCtx.load(aSrcPage)) == store_E_None)
        {
            OStoreDirectoryPageObject aDstPage;
            eErrCode = aDstPage.construct< inode >(base::allocator());
            if (eErrCode != store_E_None)
                break;

            inode_holder_type xSrcDir (aSrcPage.get());
            inode_holder_type xDstDir (aDstPage.get());

            // Copy NameBlock @@@ OLD @@@
            memcpy (&(xDstDir->m_aNameBlock), &(xSrcDir->m_aNameBlock), sizeof(xSrcDir->m_aNameBlock));

            // Obtain 'Source' data length.
            sal_uInt32 nDataLen = aSrcPage.dataLength();
            if (nDataLen > 0)
            {
                // Copy internal data area @@@ OLD @@@
                memcpy (&(xDstDir->m_pData[0]), &(xSrcDir->m_pData[0]), xSrcDir->capacity());
            }

            // Insert 'Destination' directory page.
            eErrCode = save_dirpage_Impl (aDstPage.key(), aDstPage);
            if (eErrCode != store_E_None)
                break;

            // Check for external data page scope.
            if (xSrcDir->scope(nDataLen) != inode::SCOPE_INTERNAL)
            {
                // Initialize 'Destination' data page.
                typedef OStoreDataPageData data;
                PageHolderObject< data > xData;
                if (!xData.construct(base::allocator()))
                    return store_E_OutOfMemory;

                // Determine data page count.
                inode::ChunkDescriptor aDescr (
                    nDataLen - xDstDir->capacity(), xData->capacity());

                sal_uInt32 i, n = aDescr.m_nPage;
                if (aDescr.m_nOffset) n += 1;

                // Copy data pages.
                OStoreDataPageObject aData;
                for (i = 0; i < n; i++)
                {
                    // Read 'Source' data page.
                    osl::MutexGuard aSrcGuard (*xSrcBIOS);

                    eErrCode = aSrcPage.read (i, aData, *xSrcBIOS);
                    if (eErrCode != store_E_None)
                        continue;

                    // Write 'Destination' data page. @@@ READONLY @@@
                    (void) aDstPage.write (i, aData, *this);
                }
            }

            // Update 'Destination' directory page.
            aDstPage.dataLength (nDataLen);
            (void) base::saveObjectAt (aDstPage, aDstPage.location());
        }

        // Save directory scan results.
        flush();
    }

    // Pass Two: Scan 'Source' BTree nodes.
    {
        // Re-start 'Source' rebuild context.
        aCtx.initialize (STORE_MAGIC_BTREENODE);

        // Scan 'Source' BTree nodes.
        OStoreBTreeNodeObject aNode;
        while ((eErrCode = aCtx.load(aNode)) == store_E_None)
        {
            // Check for leaf node.
            PageHolderObject< page > xNode (aNode.get());
            if (xNode->depth() == 0)
            {
                sal_uInt16 i, n = xNode->usageCount();
                for (i = 0; i < n; i++)
                {
                    entry e (xNode->m_pData[i]);

                    // Check for Hard link.
                    if (e.m_nAttrib & STORE_ATTRIB_ISLINK)
                    {
                        // Load the hard link destination.
                        OStoreDirectoryPageObject aSrcPage;
                        eErrCode = xSrcBIOS->loadObjectAt (aSrcPage, e.m_aLink.location());
                        if (eErrCode == store_E_None)
                        {
                            OStorePageKey aDstKey (aSrcPage.key());
                            link (e.m_aKey, aDstKey);
                        }
                        e.m_nAttrib &= ~STORE_ATTRIB_ISLINK;
                    }

                    if (e.m_nAttrib)
                    {
                        // Ordinary attributes.
                        sal_uInt32 nAttrib = 0;
                        attrib (e.m_aKey, 0, e.m_nAttrib, nAttrib);
                    }
                }
            }
        }

        // Save BTree node scan results.
        flush();
    }

    // Done.
    return store_E_None;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
