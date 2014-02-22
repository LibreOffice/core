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
OStorePageManager::OStorePageManager (void)
{
}

/*
 * ~OStorePageManager.
 */
OStorePageManager::~OStorePageManager (void)
{
}

/*
 * isKindOf.
 */
bool SAL_CALL OStorePageManager::isKindOf (sal_uInt32 nTypeId)
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
    
    osl::MutexGuard aGuard(*this);

    
    if (!pLockBytes)
        return store_E_InvalidParameter;

    
    storeError eErrCode = base::initialize (pLockBytes, eAccessMode, rnPageSize);
    if (eErrCode != store_E_None)
        return eErrCode;

    
    if (!base::isWriteable())
    {
        
        return base::loadObjectAt (m_aRoot, rnPageSize);
    }

    
    eErrCode = m_aRoot.loadOrCreate (rnPageSize, *this);
    if (eErrCode == store_E_Pending)
    {
        
        PageHolderObject< page > xRoot (m_aRoot.get());

        
        OStorePageKey aKey (rtl_crc32 (0, "/", 1), 0);
        xRoot->insert (0, entry(aKey));

        
        eErrCode = base::saveObjectAt (m_aRoot, rnPageSize);
    }

    
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
    
    storeError eErrCode = m_aRoot.find_lookup (rNode, rIndex, rKey, *this);
    if (eErrCode != store_E_None)
        return eErrCode;

    
    PageHolderObject< page > xPage (rNode.get());
    OSL_POSTCOND(rIndex < xPage->usageCount(), "store::PageManager::find_lookup(): logic error");
    entry e (xPage->m_pData[rIndex]);

    
    if (e.compare(entry(rKey)) != entry::COMPARE_EQUAL)
    {
        
        return store_E_NotExists;
    }

    
    if (e.m_aLink.location() == STORE_PAGE_NULL)
    {
        
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

    
    PageHolderObject< page > xPage (aNode.get());
    sal_uInt16 i = xPage->find (rEntry), n = xPage->usageCount();
    if (!(i < n))
    {
        
        return store_E_NotExists;
    }

    
    entry::CompareResult result = rEntry.compare (xPage->m_pData[i]);

    
    while ((result == entry::COMPARE_GREATER) && (xPage->depth() > 0))
    {
        
        sal_uInt32 const nAddr = xPage->m_pData[i].m_aLink.location();
        if (nAddr == STORE_PAGE_NULL)
        {
            
            return store_E_NotExists;
        }

        
        storeError eErrCode = loadObjectAt (aNode, nAddr);
        if (eErrCode != store_E_None)
            return eErrCode;

        PageHolderObject< page > xNext (aNode.get());
        xNext.swap (xPage);

        
        i = xPage->find (rEntry), n = xPage->usageCount();
        if (!(i < n))
        {
            
            return store_E_NotExists;
        }

        
        result = rEntry.compare (xPage->m_pData[i]);
    }

    OSL_POSTCOND(
        result != entry::COMPARE_LESS,
        "OStorePageManager::remove(): find failed");

    
    if (result == entry::COMPARE_LESS)
    {
        
        return store_E_Unknown;
    }

    
    return aNode.remove (i, rEntry, *this);
}

/*
 * namei.
 * Precond: none (static).
 */
storeError OStorePageManager::namei (
    const rtl_String *pPath, const rtl_String *pName, OStorePageKey &rKey)
{
    
    if (!(pPath && pName))
        return store_E_InvalidParameter;

    
    if (!(pName->length < STORE_MAXIMUM_NAMESIZE))
        return store_E_NameTooLong;

    
    rKey.m_nLow  = store::htonl(rtl_crc32 (0, pName->buffer, pName->length));
    rKey.m_nHigh = store::htonl(rtl_crc32 (0, pPath->buffer, pPath->length));

    
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
    
    osl::MutexGuard aGuard(*this);

    
    if (!self::isValid())
        return store_E_InvalidAccess;

    
    OStorePageKey aKey;
    storeError eErrCode = namei (pPath, pName, aKey);
    if (eErrCode != store_E_None)
        return eErrCode;

    
    if (nAttrib & STORE_ATTRIB_ISDIR)
    {
        
        aKey.m_nLow = store::htonl(rtl_crc32 (store::ntohl(aKey.m_nLow), "/", 1));
    }

    
    eErrCode = load_dirpage_Impl (aKey, rPage);
    if (eErrCode != store_E_None)
    {
        
        if (eErrCode != store_E_NotExists)
            return eErrCode;

        if (eMode == store_AccessReadWrite)
            return store_E_NotExists;
        if (eMode == store_AccessReadOnly)
            return store_E_NotExists;

        if (!base::isWriteable())
            return store_E_AccessViolation;

        
        eErrCode = rPage.construct< inode >(base::allocator());
        if (eErrCode != store_E_None)
            return eErrCode;

        
        PageHolderObject< inode > xPage (rPage.get());

        rPage.key (aKey);
        rPage.attrib (nAttrib);

        memcpy (
            &(xPage->m_aNameBlock.m_pData[0]),
            pName->buffer, pName->length);

        
        eErrCode = save_dirpage_Impl (aKey, rPage);
        if (eErrCode != store_E_None)
            return eErrCode;
    }

    
    if (rPage.attrib() & STORE_ATTRIB_ISLINK)
    {
        
        PageHolderObject< inode > xPage (rPage.get());
        OStorePageKey aDstKey;
        memcpy (&aDstKey, &(xPage->m_pData[0]), sizeof(aDstKey));

        
        eErrCode = load_dirpage_Impl (aDstKey, rPage);
        if (eErrCode != store_E_None)
            return eErrCode;
    }

    
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
    
    osl::MutexGuard aGuard(*this);

    
    if (!self::isValid())
        return store_E_InvalidAccess;

    
    OStoreBTreeNodeObject aNode;
    sal_uInt16 i = 0;
    storeError eErrCode = m_aRoot.find_lookup (aNode, i, rKey, *this);
    if (eErrCode != store_E_None)
        return eErrCode;

    
    PageHolderObject< page > xNode (aNode.get());
    entry e (xNode->m_pData[i]);

    
    rKey    = e.m_aKey;
    rLink   = e.m_aLink;
    rAttrib = store::ntohl(e.m_nAttrib);

    
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
    
    OStoreBTreeNodeObject aNode;
    sal_uInt16 i = 0;
    storeError eErrCode = find_lookup (aNode, i, rKey);
    if (eErrCode != store_E_None)
        return eErrCode;

    
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
    
    node aNode;
    sal_uInt16 i = 0;

    storeError eErrCode = m_aRoot.find_insert (aNode, i, rKey, *this);
    PageHolderObject< page > xNode (aNode.get());
    if (eErrCode != store_E_None)
    {
        if (eErrCode != store_E_AlreadyExists)
            return eErrCode;

        
        entry e (xNode->m_pData[i]);
        if (e.m_aLink.location() != STORE_PAGE_NULL)
        {
            
            return saveObjectAt (rPage, e.m_aLink.location());
        }

        
        eErrCode = base::allocate (rPage);
        if (eErrCode != store_E_None)
            return eErrCode;

        
        xNode->m_pData[i].m_aLink = rPage.location();

        
        return saveObjectAt (aNode, aNode.location());
    }

    
    eErrCode = base::allocate (rPage);
    if (eErrCode != store_E_None)
        return eErrCode;

    
    OStorePageLink aLink (rPage.location());
    xNode->insert (i + 1, entry (rKey, aLink));

    
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
    
    osl::MutexGuard aGuard(*this);

    
    if (!self::isValid())
        return store_E_InvalidAccess;

    
    OStoreBTreeNodeObject aNode;
    sal_uInt16 i = 0;
    storeError eErrCode = find_lookup (aNode, i, rKey);
    if (eErrCode != store_E_None)
        return eErrCode;

    
    PageHolderObject< page > xNode (aNode.get());
    entry e (xNode->m_pData[i]);
    if (nMask1 != nMask2)
    {
        
        sal_uInt32 nAttrib = store::ntohl(e.m_nAttrib);

        nAttrib &= ~nMask1;
        nAttrib |=  nMask2;

        if (store::htonl(nAttrib) != e.m_nAttrib)
        {
            
            if (base::isWriteable())
            {
                
                e.m_nAttrib = store::htonl(nAttrib);
                xNode->m_pData[i] = e;

                
                eErrCode = saveObjectAt (aNode, aNode.location());
            }
            else
            {
                
                eErrCode = store_E_AccessViolation;
            }
        }
    }

    
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
    
    osl::MutexGuard aGuard(*this);

    
    if (!self::isValid())
        return store_E_InvalidAccess;

    if (!base::isWriteable())
        return store_E_AccessViolation;

    
    OStoreBTreeNodeObject aDstNode;
    sal_uInt16 i = 0;
    storeError eErrCode = find_lookup (aDstNode, i, rDstKey);
    if (eErrCode != store_E_None)
        return eErrCode;

    
    PageHolderObject< page > xDstNode (aDstNode.get());
    entry e (xDstNode->m_pData[i]);
    OStorePageLink aDstLink (e.m_aLink);

    
    OStoreBTreeNodeObject aSrcNode;
    eErrCode = m_aRoot.find_insert (aSrcNode, i, rSrcKey, *this);
    if (eErrCode != store_E_None)
        return eErrCode;

    
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
    
    osl::MutexGuard aGuard(*this);

    
    if (!self::isValid())
        return store_E_InvalidAccess;

    if (!base::isWriteable())
        return store_E_AccessViolation;

    
    storeError eErrCode = store_E_InvalidParameter;
    if (!(pSrcPath && pSrcName))
        return eErrCode;

    
    OStorePageKey aSrcKey;
    eErrCode = namei (pSrcPath, pSrcName, aSrcKey);
    if (eErrCode != store_E_None)
        return eErrCode;

    
    OStoreBTreeNodeObject aSrcNode;
    sal_uInt16 i = 0;
    eErrCode = m_aRoot.find_insert (aSrcNode, i, aSrcKey, *this);
    if (eErrCode != store_E_None)
        return eErrCode;

    
    OStoreDirectoryPageObject aPage;
    eErrCode = aPage.construct< inode >(base::allocator());
    if (eErrCode != store_E_None)
        return eErrCode;

    
    inode_holder_type xNode (aPage.get());
    aPage.key (aSrcKey);
    memcpy (
        &(xNode->m_aNameBlock.m_pData[0]),
        pSrcName->buffer, pSrcName->length);

    
    OStorePageKey aDstKey (rDstKey);
    memcpy (&(xNode->m_pData[0]), &aDstKey, sizeof(aDstKey));

    
    aPage.attrib (STORE_ATTRIB_ISLINK);
    aPage.dataLength (sal_uInt32(sizeof(aDstKey)));

    
    eErrCode = base::allocate (aPage);
    if (eErrCode != store_E_None)
        return eErrCode;

    
    PageHolderObject< page > xSrcNode (aSrcNode.get());
    OStorePageLink aSrcLink (aPage.location());
    xSrcNode->insert (i + 1, entry(aSrcKey, aSrcLink));

    
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
    
    osl::MutexGuard aGuard(*this);

    
    if (!self::isValid())
        return store_E_InvalidAccess;

    if (!base::isWriteable())
        return store_E_AccessViolation;

    
    storeError eErrCode = store_E_InvalidParameter;
    if (!(pDstPath && pDstName))
        return eErrCode;

    
    OStorePageKey aDstKey;
    eErrCode = namei (pDstPath, pDstName, aDstKey);
    if (eErrCode != store_E_None)
        return eErrCode;

    
    OStoreBTreeNodeObject aSrcNode;
    sal_uInt16 i = 0;
    eErrCode = find_lookup (aSrcNode, i, rSrcKey);
    if (eErrCode != store_E_None)
        return eErrCode;

    
    PageHolderObject< page > xSrcNode (aSrcNode.get());
    entry e (xSrcNode->m_pData[i]);

    
    OStoreDirectoryPageObject aPage;
    if (!(store::ntohl(e.m_nAttrib) & STORE_ATTRIB_ISLINK))
    {
        
        eErrCode = base::loadObjectAt (aPage, e.m_aLink.location());
        if (eErrCode != store_E_None)
            return eErrCode;

        
        if (aPage.attrib() & STORE_ATTRIB_ISDIR)
        {
            
            aDstKey.m_nLow = store::htonl(rtl_crc32 (store::ntohl(aDstKey.m_nLow), "/", 1));
        }
    }

    
    e.m_aKey = aDstKey;

    
    OStoreBTreeNodeObject aDstNode;
    eErrCode = m_aRoot.find_insert (aDstNode, i, e.m_aKey, *this);
    if (eErrCode != store_E_None)
        return eErrCode;

    
    PageHolderObject< page > xDstNode (aDstNode.get());
    xDstNode->insert (i + 1, e);

    eErrCode = saveObjectAt (aDstNode, aDstNode.location());
    if (eErrCode != store_E_None)
        return eErrCode;

    
    if (!(store::ntohl(e.m_nAttrib) & STORE_ATTRIB_ISLINK))
    {
        
        inode_holder_type xNode (aPage.get());

        
        sal_Int32 nDstLen = pDstName->length;
        memcpy (
            &(xNode->m_aNameBlock.m_pData[0]),
            pDstName->buffer, pDstName->length);
        memset (
            &(xNode->m_aNameBlock.m_pData[nDstLen]),
            0, STORE_MAXIMUM_NAMESIZE - nDstLen);
        aPage.key (e.m_aKey);

        
        eErrCode = base::saveObjectAt (aPage, e.m_aLink.location());
        if (eErrCode != store_E_None)
            return eErrCode;
    }

    
    e.m_aKey = rSrcKey;
    return remove_Impl (e);
}

/*
 * remove.
 * Precond: initialized, writeable.
 */
storeError OStorePageManager::remove (const OStorePageKey &rKey)
{
    
    osl::MutexGuard aGuard(*this);

    
    if (!self::isValid())
        return store_E_InvalidAccess;

    if (!base::isWriteable())
        return store_E_AccessViolation;

    
    OStoreBTreeNodeObject aNodePage;
    sal_uInt16 i = 0;
    storeError eErrCode = find_lookup (aNodePage, i, rKey);
    if (eErrCode != store_E_None)
        return eErrCode;

    
    PageHolderObject< page > xNodePage (aNodePage.get());
    entry e (xNodePage->m_pData[i]);

    
    if (!(store::ntohl(e.m_nAttrib) & STORE_ATTRIB_ISLINK))
    {
        
        OStoreDirectoryPageObject aPage;
        eErrCode = base::loadObjectAt (aPage, e.m_aLink.location());
        if (eErrCode != store_E_None)
            return eErrCode;

        inode_holder_type xNode (aPage.get());

        
        OStorePageDescriptor aDescr (xNode->m_aDescr);
        eErrCode = base::acquirePage (aDescr, store_AccessReadWrite);
        if (eErrCode != store_E_None)
            return eErrCode;

        
        if (!(aPage.attrib() & STORE_ATTRIB_ISLINK))
        {
            
            inode::ChunkScope eScope = xNode->scope (aPage.dataLength());
            if (eScope == inode::SCOPE_EXTERNAL)
            {
                
                eErrCode = aPage.truncate (0, *this);
                if (eErrCode != store_E_None)
                    return eErrCode;
            }

            
            memset (&(xNode->m_pData[0]), 0, xNode->capacity());
            aPage.dataLength (0);
        }

        
        eErrCode = base::releasePage (aDescr);

        
        eErrCode = base::free (aPage.location());
    }

    
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
    RebuildContext (void)
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
    
    osl::MutexGuard aGuard(*this);

    
    storeError eErrCode = store_E_InvalidParameter;
    if (!(pSrcLB && pDstLB))
        return eErrCode;

    
    RebuildContext aCtx;
    eErrCode = aCtx.initialize (pSrcLB, STORE_MAGIC_DIRECTORYPAGE);
    if (eErrCode != store_E_None)
        return eErrCode;
    rtl::Reference<OStorePageBIOS> xSrcBIOS (aCtx.m_xBIOS);

    
    eErrCode = self::initialize (pDstLB, store_AccessCreate, aCtx.m_nPageSize);
    if (eErrCode != store_E_None)
        return eErrCode;

    
    {
        
        OStoreDirectoryPageObject aSrcPage;
        while ((eErrCode = aCtx.load(aSrcPage)) == store_E_None)
        {
            OStoreDirectoryPageObject aDstPage;
            eErrCode = aDstPage.construct< inode >(base::allocator());
            if (eErrCode != store_E_None)
                break;

            inode_holder_type xSrcDir (aSrcPage.get());
            inode_holder_type xDstDir (aDstPage.get());

            
            memcpy (&(xDstDir->m_aNameBlock), &(xSrcDir->m_aNameBlock), sizeof(xSrcDir->m_aNameBlock));

            
            sal_uInt32 nDataLen = aSrcPage.dataLength();
            if (nDataLen > 0)
            {
                
                memcpy (&(xDstDir->m_pData[0]), &(xSrcDir->m_pData[0]), xSrcDir->capacity());
            }

            
            eErrCode = save_dirpage_Impl (aDstPage.key(), aDstPage);
            if (eErrCode != store_E_None)
                break;

            
            if (xSrcDir->scope(nDataLen) != inode::SCOPE_INTERNAL)
            {
                
                typedef OStoreDataPageData data;
                PageHolderObject< data > xData;
                if (!xData.construct(base::allocator()))
                    return store_E_OutOfMemory;

                
                inode::ChunkDescriptor aDescr (
                    nDataLen - xDstDir->capacity(), xData->capacity());

                sal_uInt32 i, n = aDescr.m_nPage;
                if (aDescr.m_nOffset) n += 1;

                
                OStoreDataPageObject aData;
                for (i = 0; i < n; i++)
                {
                    
                    osl::MutexGuard aSrcGuard (*xSrcBIOS);

                    eErrCode = aSrcPage.read (i, aData, *xSrcBIOS);
                    if (eErrCode != store_E_None)
                        continue;

                    
                    eErrCode = aDstPage.write (i, aData, *this);
                }
            }

            
            aDstPage.dataLength (nDataLen);
            eErrCode = base::saveObjectAt (aDstPage, aDstPage.location());
        }

        
        flush();
    }

    
    {
        
        aCtx.initialize (STORE_MAGIC_BTREENODE);

        
        OStoreBTreeNodeObject aNode;
        while ((eErrCode = aCtx.load(aNode)) == store_E_None)
        {
            
            PageHolderObject< page > xNode (aNode.get());
            if (xNode->depth() == 0)
            {
                sal_uInt16 i, n = xNode->usageCount();
                for (i = 0; i < n; i++)
                {
                    entry e (xNode->m_pData[i]);

                    
                    if (e.m_nAttrib & STORE_ATTRIB_ISLINK)
                    {
                        
                        OStoreDirectoryPageObject aSrcPage;
                        eErrCode = xSrcBIOS->loadObjectAt (aSrcPage, e.m_aLink.location());
                        if (eErrCode == store_E_None)
                        {
                            OStorePageKey aDstKey (aSrcPage.key());
                            eErrCode = link (e.m_aKey, aDstKey);
                        }
                        e.m_nAttrib &= ~STORE_ATTRIB_ISLINK;
                    }

                    if (e.m_nAttrib)
                    {
                        
                        sal_uInt32 nAttrib = 0;
                        eErrCode = attrib (e.m_aKey, 0, e.m_nAttrib, nAttrib);
                    }
                }
            }
        }

        
        flush();
    }

    
    return store_E_None;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
