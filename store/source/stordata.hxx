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

#ifndef _STORE_STORDATA_HXX_
#define _STORE_STORDATA_HXX_

#include "sal/types.h"
#include "sal/macros.h"

#include "store/types.h"
#include "storbase.hxx"

namespace store
{

/*========================================================================
 *
 * OStoreDataPageData.
 *
 *======================================================================*/
#define STORE_MAGIC_DATAPAGE sal_uInt32(0x94190310)

struct OStoreDataPageData : public store::OStorePageData
{
    typedef OStorePageData       base;
    typedef OStoreDataPageData   self;

    typedef OStorePageDescriptor D;

    /** Representation.
    */
    sal_uInt8 m_pData[1];

    /** type.
     */
    static const sal_uInt32 theTypeId = STORE_MAGIC_DATAPAGE;

    /** size.
    */
    static const size_t     theSize     = 0;
    static const sal_uInt16 thePageSize = base::theSize + self::theSize;
    STORE_STATIC_ASSERT(STORE_MINIMUM_PAGESIZE >= self::thePageSize);

    /** capacity.
    */
    static sal_uInt16 capacity (const D& rDescr) // @see inode::ChunkDescriptor
    {
        return (store::ntohs(rDescr.m_nSize) - self::thePageSize);
    }
    sal_uInt16 capacity() const
    {
        return self::capacity (base::m_aDescr);
    }

    /** usage.
    */
    sal_uInt16 usage() const
    {
        return (store::ntohs(base::m_aDescr.m_nUsed) - self::thePageSize);
    }

    /** Construction.
    */
    explicit OStoreDataPageData (sal_uInt16 nPageSize = self::thePageSize)
        : base (nPageSize)
    {
        base::m_aGuard.m_nMagic = store::htonl(self::theTypeId);
        base::m_aDescr.m_nUsed  = store::htons(self::thePageSize);
        if (capacity()) memset (m_pData, 0, capacity());
    }

    /** guard (external representation).
    */
    void guard() {}

    /** verify (external representation).
    */
    storeError verify() const { return store_E_None; }
};

/*========================================================================
 *
 * OStoreDataPageObject.
 *
 *======================================================================*/
class OStoreDataPageObject : public store::OStorePageObject
{
    typedef OStorePageObject     base;
    typedef OStoreDataPageData   page;

public:
    /** Construction.
    */
    explicit OStoreDataPageObject (PageHolder const & rxPage = PageHolder())
        : OStorePageObject (rxPage)
    {}

    /** External representation.
     */
    virtual storeError guard  (sal_uInt32 nAddr);
    virtual storeError verify (sal_uInt32 nAddr) const;
};

/*========================================================================
 *
 * OStoreIndirectionPageData.
 *
 *======================================================================*/
#define STORE_MAGIC_INDIRECTPAGE sal_uInt32(0x89191107)

struct OStoreIndirectionPageData : public store::OStorePageData
{
    typedef OStorePageData            base;
    typedef OStoreIndirectionPageData self;

    typedef OStorePageGuard           G;
    typedef OStorePageDescriptor      D;

    /** Representation.
    */
    G          m_aGuard;
    sal_uInt32 m_pData[1];

    /** type.
     */
    static const sal_uInt32 theTypeId = STORE_MAGIC_INDIRECTPAGE;

    /** size.
     */
    static const size_t     theSize     = sizeof(G);
    static const sal_uInt16 thePageSize = base::theSize + self::theSize;
    STORE_STATIC_ASSERT(STORE_MINIMUM_PAGESIZE >= self::thePageSize);

    /** capacity.
    */
    static sal_uInt16 capacity (const D& rDescr)
    {
        return (store::ntohs(rDescr.m_nSize) - self::thePageSize);
    }
    sal_uInt16 capacity() const
    {
        return self::capacity (base::m_aDescr);
    }

    /** capacityCount.
    */
    static sal_uInt16 capacityCount (const D& rDescr) // @see DirectoryPageObject::scope()
    {
        return sal_uInt16(capacity(rDescr) / sizeof(sal_uInt32));
    }
    sal_uInt16 capacityCount() const
    {
        return sal_uInt16(capacity() / sizeof(sal_uInt32));
    }

    /** Construction.
    */
    explicit OStoreIndirectionPageData (sal_uInt16 nPageSize)
        : base (nPageSize)
    {
        base::m_aGuard.m_nMagic = store::htonl(self::theTypeId);
        base::m_aDescr.m_nUsed  = store::htons(self::thePageSize);
        self::m_aGuard.m_nMagic = store::htonl(0);
        memset (m_pData, STORE_PAGE_NULL, capacity());
    }

    /** guard (external representation).
    */
    void guard()
    {
        sal_uInt32 nCRC32 = 0;
        nCRC32 = rtl_crc32 (nCRC32, &m_aGuard.m_nMagic, sizeof(sal_uInt32));
        nCRC32 = rtl_crc32 (nCRC32, m_pData, capacity());
        m_aGuard.m_nCRC32 = store::htonl(nCRC32);
    }

    /** verify (external representation).
    */
    storeError verify() const
    {
        sal_uInt32 nCRC32 = 0;
        nCRC32 = rtl_crc32 (nCRC32, &m_aGuard.m_nMagic, sizeof(sal_uInt32));
        nCRC32 = rtl_crc32 (nCRC32, m_pData, capacity());
        if (m_aGuard.m_nCRC32 != store::htonl(nCRC32))
            return store_E_InvalidChecksum;
        else
            return store_E_None;
    }
};

/*========================================================================
 *
 * OStoreIndirectionPageObject.
 *
 *======================================================================*/
class OStoreIndirectionPageObject : public store::OStorePageObject
{
    typedef OStorePageObject          base;
    typedef OStoreIndirectionPageData page;

public:
    /** Construction.
    */
    explicit OStoreIndirectionPageObject (PageHolder const & rxPage = PageHolder())
        : OStorePageObject (rxPage)
    {}

    /** External representation.
    */
    storeError loadOrCreate (
        sal_uInt32       nAddr,
        OStorePageBIOS & rBIOS);

    virtual storeError guard  (sal_uInt32 nAddr);
    virtual storeError verify (sal_uInt32 nAddr) const;

    /** read (indirect data page).
    */
    storeError read (
        sal_uInt16             nSingle,
        OStoreDataPageObject  &rData,
        OStorePageBIOS        &rBIOS);

    storeError read (
        sal_uInt16             nDouble,
        sal_uInt16             nSingle,
        OStoreDataPageObject  &rData,
        OStorePageBIOS        &rBIOS);

    storeError read (
        sal_uInt16             nTriple,
        sal_uInt16             nDouble,
        sal_uInt16             nSingle,
        OStoreDataPageObject  &rData,
        OStorePageBIOS        &rBIOS);

    /** write (indirect data page).
    */
    storeError write (
        sal_uInt16             nSingle,
        OStoreDataPageObject  &rData,
        OStorePageBIOS        &rBIOS);

    storeError write (
        sal_uInt16             nDouble,
        sal_uInt16             nSingle,
        OStoreDataPageObject  &rData,
        OStorePageBIOS        &rBIOS);

    storeError write (
        sal_uInt16             nTriple,
        sal_uInt16             nDouble,
        sal_uInt16             nSingle,
        OStoreDataPageObject  &rData,
        OStorePageBIOS        &rBIOS);

    /** truncate (indirect data page).
    */
    storeError truncate (
        sal_uInt16             nSingle,
        OStorePageBIOS        &rBIOS);

    storeError truncate (
        sal_uInt16             nDouble,
        sal_uInt16             nSingle,
        OStorePageBIOS        &rBIOS);

    storeError truncate (
        sal_uInt16             nTriple,
        sal_uInt16             nDouble,
        sal_uInt16             nSingle,
        OStorePageBIOS        &rBIOS);
};

/*========================================================================
 *
 * OStorePageNameBlock.
 *
 *======================================================================*/
struct OStorePageNameBlock
{
    typedef OStorePageGuard G;
    typedef OStorePageKey   K;

    /** Representation.
    */
    G          m_aGuard;
    K          m_aKey;
    sal_uInt32 m_nAttrib;
    sal_Char   m_pData[STORE_MAXIMUM_NAMESIZE];

    /** size.
    */
    static const size_t theSize = sizeof(G) + sizeof(K) + sizeof(sal_uInt32) + sizeof(sal_Char[STORE_MAXIMUM_NAMESIZE]);

    /** initialize.
    */
    void initialize (void)
    {
        m_aGuard  = G();
        m_aKey    = K();
        m_nAttrib = 0;
        memset (m_pData, 0, sizeof(m_pData));
    }

    /** Construction.
    */
    OStorePageNameBlock (void)
        : m_aGuard(), m_aKey(), m_nAttrib (0)
    {
        memset (m_pData, 0, sizeof(m_pData));
    }

    /** guard (external representation).
    */
    void guard()
    {
        sal_uInt32 nCRC32 = 0;
        nCRC32 = rtl_crc32 (nCRC32, &m_aGuard.m_nMagic, sizeof(sal_uInt32));
        nCRC32 = rtl_crc32 (nCRC32, &m_aKey, theSize - sizeof(G));
        m_aGuard.m_nCRC32 = store::htonl(nCRC32);
    }

    /** verify (external representation).
    */
    storeError verify() const
    {
        sal_uInt32 nCRC32 = 0;
        nCRC32 = rtl_crc32 (nCRC32, &m_aGuard.m_nMagic, sizeof(sal_uInt32));
        nCRC32 = rtl_crc32 (nCRC32, &m_aKey, theSize - sizeof(G));
        if (m_aGuard.m_nCRC32 != store::htonl(nCRC32))
            return store_E_InvalidChecksum;
        else
            return store_E_None;
    }
};

/*========================================================================
 *
 * OStoreDirectoryDataBlock.
 *
 *======================================================================*/
#define STORE_LIMIT_DATAPAGE_DIRECT 16
#define STORE_LIMIT_DATAPAGE_SINGLE  8
#define STORE_LIMIT_DATAPAGE_DOUBLE  1
#define STORE_LIMIT_DATAPAGE_TRIPLE  1

struct OStoreDirectoryDataBlock
{
    typedef OStorePageGuard G;

    /** LinkDescriptor.
    */
    struct LinkDescriptor
    {
        /** Representation.
        */
        sal_uInt16 m_nIndex0;
        sal_uInt16 m_nIndex1;
        sal_uInt16 m_nIndex2;
        sal_uInt16 m_nIndex3;

        /** Construction.
        */
        LinkDescriptor (void)
            : m_nIndex0 ((sal_uInt16)(~0)),
              m_nIndex1 ((sal_uInt16)(~0)),
              m_nIndex2 ((sal_uInt16)(~0)),
              m_nIndex3 ((sal_uInt16)(~0))
        {}
    };

    /** LinkTable.
    */
    struct LinkTable
    {
        /** Representation.
        */
        sal_uInt32 m_pDirect[STORE_LIMIT_DATAPAGE_DIRECT];
        sal_uInt32 m_pSingle[STORE_LIMIT_DATAPAGE_SINGLE];
        sal_uInt32 m_pDouble[STORE_LIMIT_DATAPAGE_DOUBLE];
        sal_uInt32 m_pTriple[STORE_LIMIT_DATAPAGE_TRIPLE];

        /** initialize.
        */
        void initialize (void)
        {
          memset(m_pDirect, STORE_PAGE_NULL, sizeof(m_pDirect));
          memset(m_pSingle, STORE_PAGE_NULL, sizeof(m_pSingle));
          memset(m_pDouble, STORE_PAGE_NULL, sizeof(m_pDouble));
          memset(m_pTriple, STORE_PAGE_NULL, sizeof(m_pTriple));
        }

        /** Construction.
        */
        LinkTable (void)
        {
          initialize();
        }
    };

    /** Representation.
    */
    G          m_aGuard;
    LinkTable  m_aTable;
    sal_uInt32 m_nDataLen;

    /** size.
     */
    static const size_t theSize = sizeof(G) + sizeof(LinkTable) + sizeof(sal_uInt32);

    /** initialize.
    */
    void initialize (void)
    {
        m_aGuard = G();
        m_aTable.initialize();
        m_nDataLen = 0;
    }

    /** Construction.
    */
    OStoreDirectoryDataBlock (void)
        : m_aGuard(), m_aTable(), m_nDataLen (0)
    {}

    /** guard (external representation).
    */
    void guard()
    {
        sal_uInt32 nCRC32 = 0;
        nCRC32 = rtl_crc32 (nCRC32, &m_aGuard.m_nMagic, sizeof(sal_uInt32));
        nCRC32 = rtl_crc32 (nCRC32, &m_aTable, theSize - sizeof(G));
        m_aGuard.m_nCRC32 = store::htonl(nCRC32);
    }

    /** verify (external representation).
    */
    storeError verify() const
    {
        sal_uInt32 nCRC32 = 0;
        nCRC32 = rtl_crc32 (nCRC32, &m_aGuard.m_nMagic, sizeof(sal_uInt32));
        nCRC32 = rtl_crc32 (nCRC32, &m_aTable, theSize - sizeof(G));
        if (m_aGuard.m_nCRC32 != store::htonl(nCRC32))
            return store_E_InvalidChecksum;
        else
            return store_E_None;
    }

    /** direct.
    */
    static sal_uInt16 directCount (void)
    {
        return ((sal_uInt16)(STORE_LIMIT_DATAPAGE_DIRECT));
    }
    sal_uInt32 directLink (sal_uInt16 nIndex) const
    {
        if (nIndex < directCount())
            return store::ntohl(m_aTable.m_pDirect[nIndex]);
        else
            return STORE_PAGE_NULL;
    }
    void directLink (sal_uInt16 nIndex, sal_uInt32 nAddr)
    {
        if (nIndex < directCount())
            m_aTable.m_pDirect[nIndex] = store::htonl(nAddr);
    }

    /** single.
    */
    static sal_uInt16 singleCount (void)
    {
        return ((sal_uInt16)(STORE_LIMIT_DATAPAGE_SINGLE));
    }
    sal_uInt32 singleLink (sal_uInt16 nIndex) const
    {
        if (nIndex < singleCount())
            return store::ntohl(m_aTable.m_pSingle[nIndex]);
        else
            return STORE_PAGE_NULL;
    }
    void singleLink (sal_uInt16 nIndex, sal_uInt32 nAddr)
    {
        if (nIndex < singleCount())
            m_aTable.m_pSingle[nIndex] = store::htonl(nAddr);
    }

    /** double.
    */
    static sal_uInt16 doubleCount (void)
    {
        return ((sal_uInt16)(STORE_LIMIT_DATAPAGE_DOUBLE));
    }
    sal_uInt32 doubleLink (sal_uInt16 nIndex) const
    {
        if (nIndex < doubleCount())
            return store::ntohl(m_aTable.m_pDouble[nIndex]);
        else
            return STORE_PAGE_NULL;
    }
    void doubleLink (sal_uInt16 nIndex, sal_uInt32 nAddr)
    {
        if (nIndex < doubleCount())
            m_aTable.m_pDouble[nIndex] = store::htonl(nAddr);
    }

    /** triple.
    */
    static sal_uInt16 tripleCount (void)
    {
        return ((sal_uInt16)(STORE_LIMIT_DATAPAGE_TRIPLE));
    }
    sal_uInt32 tripleLink (sal_uInt16 nIndex) const
    {
        if (nIndex < tripleCount())
            return store::ntohl(m_aTable.m_pTriple[nIndex]);
        else
            return STORE_PAGE_NULL;
    }
    void tripleLink (sal_uInt16 nIndex, sal_uInt32 nAddr)
    {
        if (nIndex < tripleCount())
            m_aTable.m_pTriple[nIndex] = store::htonl(nAddr);
    }
};

/*========================================================================
 *
 * OStoreDirectoryPageData.
 *
 *======================================================================*/
#define STORE_MAGIC_DIRECTORYPAGE sal_uInt32(0x62190120)

struct OStoreDirectoryPageData : public store::OStorePageData
{
    typedef OStorePageData           base;
    typedef OStoreDirectoryPageData  self;

    typedef OStorePageDescriptor     D;
    typedef OStorePageNameBlock      NameBlock;
    typedef OStoreDirectoryDataBlock DataBlock;

    /** Representation.
     */
    NameBlock m_aNameBlock;
    DataBlock m_aDataBlock;
    sal_uInt8 m_pData[1];

    /** type.
     */
    static const sal_uInt32 theTypeId = STORE_MAGIC_DIRECTORYPAGE;

    /** size.
     */
    static const size_t     theSize     = NameBlock::theSize + DataBlock::theSize;
    static const sal_uInt16 thePageSize = base::theSize + self::theSize;
    STORE_STATIC_ASSERT(STORE_MINIMUM_PAGESIZE >= self::thePageSize);

    /** capacity.
    */
    sal_uInt16 capacity() const
    {
        return (store::ntohs(base::m_aDescr.m_nSize) - self::thePageSize);
    }

    /** usage.
    */
    sal_uInt16 usage() const
    {
        return (store::ntohs(base::m_aDescr.m_nUsed) - self::thePageSize);
    }

    /** initialize.
    */
    void initialize (void)
    {
        base::m_aGuard.m_nMagic = store::htonl(self::theTypeId);
        base::m_aDescr.m_nUsed  = store::htons(self::thePageSize);

        m_aNameBlock.initialize();
        m_aDataBlock.initialize();

        memset (m_pData, 0, capacity());
    }

    /** Construction.
    */
    explicit OStoreDirectoryPageData (sal_uInt16 nPageSize)
        : base (nPageSize), m_aNameBlock(), m_aDataBlock()
    {
        base::m_aGuard.m_nMagic = store::htonl(self::theTypeId);
        base::m_aDescr.m_nUsed  = store::htons(self::thePageSize);
        memset (m_pData, 0, capacity());
    }

    /** guard (external representation).
    */
    void guard()
    {
        m_aNameBlock.guard();
        m_aDataBlock.guard();
    }

    /** verify (external representation).
    */
    storeError verify() const
    {
        storeError eErrCode = m_aNameBlock.verify();
        if (eErrCode == store_E_None)
            eErrCode = m_aDataBlock.verify();
        return eErrCode;
    }

    /** ChunkDescriptor.
    */
    struct ChunkDescriptor
    {
        /** Representation.
        */
        sal_uInt32 m_nPage;
        sal_uInt16 m_nOffset;
        sal_uInt16 m_nLength;

        /** Construction.
        */
        ChunkDescriptor (sal_uInt32 nPosition, sal_uInt16 nCapacity)
        {
            m_nPage   = nPosition / nCapacity;
            m_nOffset = (sal_uInt16)((nPosition % nCapacity) & 0xffff);
            m_nLength = nCapacity - m_nOffset;
        }
    };

    /** ChunkScope.
    */
    enum ChunkScope
    {
        SCOPE_INTERNAL,
        SCOPE_EXTERNAL,
        SCOPE_DIRECT,
        SCOPE_SINGLE,
        SCOPE_DOUBLE,
        SCOPE_TRIPLE,
        SCOPE_UNREACHABLE,
        SCOPE_UNKNOWN
    };

    /** scope (internal).
    */
    ChunkScope scope (sal_uInt32 nPosition) const
    {
        sal_uInt32 nCapacity = capacity();
        if (nPosition < nCapacity)
            return SCOPE_INTERNAL;
        else
            return SCOPE_EXTERNAL;
    }
};

/*========================================================================
 *
 * OStoreDirectoryPageObject.
 *
 *======================================================================*/
class OStoreDirectoryPageObject : public store::OStorePageObject
{
    typedef OStorePageObject          base;
    typedef OStoreDirectoryPageData   page;
    typedef OStoreIndirectionPageData indirect;

    typedef OStorePageDescriptor      D;

public:
    /** Construction.
    */
    explicit OStoreDirectoryPageObject (PageHolder const & rxPage = PageHolder())
        : OStorePageObject (rxPage)
    {}

    /** External representation.
    */
    virtual storeError guard  (sal_uInt32 nAddr);
    virtual storeError verify (sal_uInt32 nAddr) const;

    /** attrib.
    */
    sal_uInt32 attrib (void) const
    {
        return store::ntohl(PAGE().m_aNameBlock.m_nAttrib);
    }
    void attrib (sal_uInt32 nAttrib)
    {
        PAGE().m_aNameBlock.m_nAttrib = store::htonl(nAttrib);
        touch();
    }

    /** key.
    */
    OStorePageKey key (void) const
    {
        return PAGE().m_aNameBlock.m_aKey;
    }
    void key (OStorePageKey const & rKey)
    {
        PAGE().m_aNameBlock.m_aKey = rKey;
        touch();
    }

    /** path.
    */
    sal_uInt32 path (void) const
    {
        page const & rPage = PAGE();
        const sal_Char * pszName = rPage.m_aNameBlock.m_pData;
        sal_uInt32       nPath   = store::ntohl(rPage.m_aNameBlock.m_aKey.m_nHigh);
        return rtl_crc32 (nPath, pszName, rtl_str_getLength(pszName));
    }

    sal_Size getName (sal_Char * pBuffer, sal_Size nBufsize) const
    {
        sal_Char const * pszName = PAGE().m_aNameBlock.m_pData;
        sal_Size nLength = rtl_str_getLength(pszName);
        memcpy (pBuffer, pszName, nLength < nBufsize ? nLength : nBufsize);
        return nLength;
    }

    /** dataLength.
    */
    sal_uInt32 dataLength (void) const
    {
        return store::ntohl(PAGE().m_aDataBlock.m_nDataLen);
    }
    void dataLength (sal_uInt32 nLength)
    {
        PAGE().m_aDataBlock.m_nDataLen = store::htonl(nLength);
        touch();
    }

    /** direct.
    */
    sal_uInt32 directLink (sal_uInt16 nIndex) const
    {
        return PAGE().m_aDataBlock.directLink (nIndex);
    }
    void directLink (sal_uInt16 nIndex, sal_uInt32 nAddr)
    {
        PAGE().m_aDataBlock.directLink (nIndex, nAddr);
        touch();
    }

    /** single indirect.
    */
    sal_uInt32 singleLink (sal_uInt16 nIndex) const
    {
        return PAGE().m_aDataBlock.singleLink (nIndex);
    }
    void singleLink (sal_uInt16 nIndex, sal_uInt32 nAddr)
    {
        PAGE().m_aDataBlock.singleLink (nIndex, nAddr);
        touch();
    }

    /** double indirect.
    */
    sal_uInt32 doubleLink (sal_uInt16 nIndex) const
    {
        return PAGE().m_aDataBlock.doubleLink (nIndex);
    }
    void doubleLink (sal_uInt16 nIndex, sal_uInt32 nAddr)
    {
        PAGE().m_aDataBlock.doubleLink (nIndex, nAddr);
        touch();
    }

    /** triple indirect.
    */
    sal_uInt32 tripleLink (sal_uInt16 nIndex) const
    {
        return PAGE().m_aDataBlock.tripleLink (nIndex);
    }
    void tripleLink (sal_uInt16 nIndex, sal_uInt32 nAddr)
    {
        PAGE().m_aDataBlock.tripleLink (nIndex, nAddr);
        touch();
    }

    /** read (external data page).
    */
    storeError read (
        sal_uInt32             nPage,
        OStoreDataPageObject  &rData,
        OStorePageBIOS        &rBIOS);

    /** write (external data page).
    */
    storeError write (
        sal_uInt32             nPage,
        OStoreDataPageObject  &rData,
        OStorePageBIOS        &rBIOS);

    /** truncate (external data page).
    */
    storeError truncate (
        sal_uInt32             nPage,
        OStorePageBIOS        &rBIOS);

private:
    /** Representation.
    */
    page & PAGE()
    {
        page * pImpl = static_cast<page*>(m_xPage.get());
        OSL_PRECOND(pImpl != 0, "OStoreDirectoryPageObject::PAGE(): Null pointer");
        return (*pImpl);
    }
    page const & PAGE() const
    {
        page const * pImpl = static_cast<page const *>(m_xPage.get());
        OSL_PRECOND(pImpl != 0, "OStoreDirectoryPageObject::PAGE(): Null pointer");
        return (*pImpl);
    }

    /** scope (external data page; private).
    */
    page::ChunkScope scope (
        sal_uInt32                       nPage,
        page::DataBlock::LinkDescriptor &rDescr) const;

    /** truncate (external data page scope; private).
    */
    storeError truncate (
        page::ChunkScope       eScope,
        sal_uInt16             nRemain,
        OStorePageBIOS        &rBIOS);
};

/*========================================================================
 *
 * The End.
 *
 *======================================================================*/

} // namespace store

#endif /* !_STORE_STORDATA_HXX_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
