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

#include <svl/inethist.hxx>

#include <algorithm>
#include <string.h>

#include "rtl/instance.hxx"
#include "rtl/crc.h"
#include <tools/solar.h>
#include <tools/debug.hxx>
#include <tools/string.hxx>
#include <tools/urlobj.hxx>

/*========================================================================
 *
 * INetURLHistory internals.
 *
 *======================================================================*/
#define INETHIST_DEF_FTP_PORT    21
#define INETHIST_DEF_HTTP_PORT   80
#define INETHIST_DEF_HTTPS_PORT 443

#define INETHIST_SIZE_LIMIT   1024
#define INETHIST_MAGIC_HEAD   0x484D4849UL

/*
 * INetURLHistoryHint implementation.
 */
IMPL_PTRHINT (INetURLHistoryHint, const INetURLObject);

/*========================================================================
 *
 * INetURLHistory_Impl interface.
 *
 *======================================================================*/
class INetURLHistory_Impl
{
    /** head_entry.
    */
    struct head_entry
    {
        /** Representation.
        */
        sal_uInt32 m_nMagic;
        sal_uInt16 m_nNext;
        sal_uInt16 m_nMBZ;

        /** Initialization.
        */
        void initialize (void)
        {
            m_nMagic = INETHIST_MAGIC_HEAD;
            m_nNext  = 0;
            m_nMBZ   = 0;
        }
    };

    /** hash_entry.
    */
    struct hash_entry
    {
        /** Representation.
        */
        sal_uInt32 m_nHash;
        sal_uInt16 m_nLru;
        sal_uInt16 m_nMBZ;

        /** Initialization.
        */
        void initialize (sal_uInt16 nLru, sal_uInt32 nHash = 0)
        {
            m_nHash = nHash;
            m_nLru  = nLru;
            m_nMBZ  = 0;
        }

        /** Comparison.
        */
        sal_Bool operator== (const hash_entry &rOther) const
        {
            return (m_nHash == rOther.m_nHash);
        }
        sal_Bool operator< (const hash_entry &rOther) const
        {
            return (m_nHash < rOther.m_nHash);
        }

        sal_Bool operator== (sal_uInt32 nHash) const
        {
            return (m_nHash == nHash);
        }
        sal_Bool operator< (sal_uInt32 nHash) const
        {
            return (m_nHash < nHash);
        }
    };

    /** lru_entry.
    */
    struct lru_entry
    {
        /** Representation.
        */
        sal_uInt32 m_nHash;
        sal_uInt16 m_nNext;
        sal_uInt16 m_nPrev;

        /** Initialization.
        */
        void initialize (sal_uInt16 nThis, sal_uInt32 nHash = 0)
        {
            m_nHash = nHash;
            m_nNext = nThis;
            m_nPrev = nThis;
        }
    };

    /** Representation.
    */
    head_entry m_aHead;
    hash_entry m_pHash[INETHIST_SIZE_LIMIT];
    lru_entry  m_pList[INETHIST_SIZE_LIMIT];

    /** Initialization.
    */
    void initialize (void);

    /** capacity.
    */
    sal_uInt16 capacity (void) const
    {
        return (sal_uInt16)(INETHIST_SIZE_LIMIT);
    }

    /** crc32.
    */
    sal_uInt32 crc32 (UniString const & rData) const
    {
        return rtl_crc32 (0, rData.GetBuffer(), rData.Len() * sizeof(sal_Unicode));
    }

    /** find.
    */
    sal_uInt16 find (sal_uInt32 nHash) const;

    /** move.
    */
    void move (sal_uInt16 nSI, sal_uInt16 nDI);

    /** backlink.
    */
    void backlink (sal_uInt16 nThis, sal_uInt16 nTail)
    {
        register lru_entry &rThis = m_pList[nThis];
        register lru_entry &rTail = m_pList[nTail];

        rTail.m_nNext = nThis;
        rTail.m_nPrev = rThis.m_nPrev;
        rThis.m_nPrev = nTail;
        m_pList[rTail.m_nPrev].m_nNext = nTail;
    }

    /** unlink.
    */
    void unlink (sal_uInt16 nThis)
    {
        register lru_entry &rThis = m_pList[nThis];

        m_pList[rThis.m_nPrev].m_nNext = rThis.m_nNext;
        m_pList[rThis.m_nNext].m_nPrev = rThis.m_nPrev;
        rThis.m_nNext = nThis;
        rThis.m_nPrev = nThis;
    }

    /** Not implemented.
    */
    INetURLHistory_Impl (const INetURLHistory_Impl&);
    INetURLHistory_Impl& operator= (const INetURLHistory_Impl&);

public:
    INetURLHistory_Impl (void);
    ~INetURLHistory_Impl (void);

    /** putUrl/queryUrl.
    */
    void putUrl   (const String &rUrl);
    sal_Bool queryUrl (const String &rUrl);
};

/*========================================================================
 *
 * INetURLHistory_Impl implementation.
 *
 *======================================================================*/
/*
 * INetURLHistory_Impl.
 */
INetURLHistory_Impl::INetURLHistory_Impl (void)
{
    initialize();
}

/*
 * ~INetURLHistory_Impl.
 */
INetURLHistory_Impl::~INetURLHistory_Impl (void)
{
}

/*
 * initialize.
 */
void INetURLHistory_Impl::initialize (void)
{
    m_aHead.initialize();

    sal_uInt16 i, n = capacity();
    for (i = 0; i < n; i++)
        m_pHash[i].initialize(i);
    for (i = 0; i < n; i++)
        m_pList[i].initialize(i);
    for (i = 1; i < n; i++)
        backlink (m_aHead.m_nNext, i);
}

/*
 * find.
 */
sal_uInt16 INetURLHistory_Impl::find (sal_uInt32 nHash) const
{
    sal_uInt16 l = 0;
    sal_uInt16 r = capacity() - 1;
    sal_uInt16 c = capacity();

    while ((l < r) && (r < c))
    {
        sal_uInt16 m = (l + r) / 2;
        if (m_pHash[m] == nHash)
            return m;

        if (m_pHash[m] < nHash)
            l = m + 1;
        else
            r = m - 1;
    }
    return l;
}

/*
 * move.
 */
void INetURLHistory_Impl::move (sal_uInt16 nSI, sal_uInt16 nDI)
{
    hash_entry e = m_pHash[nSI];
    if (nSI < nDI)
    {
        // shift left.
        memmove (
            &m_pHash[nSI    ],
            &m_pHash[nSI + 1],
            (nDI - nSI) * sizeof(hash_entry));
    }
    if (nSI > nDI)
    {
        // shift right.
        memmove (
            &m_pHash[nDI + 1],
            &m_pHash[nDI    ],
            (nSI - nDI) * sizeof(hash_entry));
    }
    m_pHash[nDI] = e;
}

/*
 * putUrl.
 */
void INetURLHistory_Impl::putUrl (const String &rUrl)
{
    sal_uInt32 h = crc32 (rUrl);
    sal_uInt16 k = find (h);
    if ((k < capacity()) && (m_pHash[k] == h))
    {
        // Cache hit.
        sal_uInt16 nMRU = m_pHash[k].m_nLru;
        if (nMRU != m_aHead.m_nNext)
        {
            // Update LRU chain.
            unlink (nMRU);
            backlink (m_aHead.m_nNext, nMRU);

            // Rotate LRU chain.
            m_aHead.m_nNext = m_pList[m_aHead.m_nNext].m_nPrev;
        }
    }
    else
    {
        // Cache miss. Obtain least recently used.
        sal_uInt16 nLRU = m_pList[m_aHead.m_nNext].m_nPrev;

        sal_uInt16 nSI = find (m_pList[nLRU].m_nHash);
        if (!(nLRU == m_pHash[nSI].m_nLru))
        {
            // Update LRU chain.
            nLRU = m_pHash[nSI].m_nLru;
            unlink (nLRU);
            backlink (m_aHead.m_nNext, nLRU);
        }

        // Rotate LRU chain.
        m_aHead.m_nNext = m_pList[m_aHead.m_nNext].m_nPrev;

        // Check source and destination.
        sal_uInt16 nDI = std::min (k, sal_uInt16(capacity() - 1));
        if (nSI < nDI)
        {
            if (!(m_pHash[nDI] < h))
                nDI -= 1;
        }
        if (nDI < nSI)
        {
            if (m_pHash[nDI] < h)
                nDI += 1;
        }

        // Assign data.
        m_pList[m_aHead.m_nNext].m_nHash = m_pHash[nSI].m_nHash = h;
        move (nSI, nDI);
    }
}

/*
 * queryUrl.
 */
sal_Bool INetURLHistory_Impl::queryUrl (const String &rUrl)
{
    sal_uInt32 h = crc32 (rUrl);
    sal_uInt16 k = find (h);
    if ((k < capacity()) && (m_pHash[k] == h))
    {
        // Cache hit.
        return sal_True;
    }
    else
    {
        // Cache miss.
        return sal_False;
    }
}

/*========================================================================
 *
 * INetURLHistory::StaticInstance implementation.
 *
 *======================================================================*/
INetURLHistory * INetURLHistory::StaticInstance::operator ()()
{
    static INetURLHistory g_aInstance;
    return &g_aInstance;
}

/*========================================================================
 *
 * INetURLHistory implementation.
 *
 *======================================================================*/
/*
 * INetURLHistory.
 */
INetURLHistory::INetURLHistory() : m_pImpl (new INetURLHistory_Impl())
{
}

/*
 * ~INetURLHistory.
 */
INetURLHistory::~INetURLHistory()
{
    DELETEZ (m_pImpl);
}

/*
 * GetOrCreate.
 */
INetURLHistory* INetURLHistory::GetOrCreate()
{
    return rtl_Instance<
        INetURLHistory, StaticInstance,
        osl::MutexGuard, osl::GetGlobalMutex >::create (
            StaticInstance(), osl::GetGlobalMutex());
}

/*
 * NormalizeUrl_Impl.
 */
void INetURLHistory::NormalizeUrl_Impl (INetURLObject &rUrl)
{
    switch (rUrl.GetProtocol())
    {
        case INET_PROT_FILE:
            if (!rUrl.IsCaseSensitive())
            {
                String aPath (rUrl.GetURLPath(INetURLObject::NO_DECODE));
                aPath.ToLowerAscii();
                rUrl.SetURLPath (aPath, INetURLObject::NOT_CANONIC);
            }
            break;

        case INET_PROT_FTP:
            if (!rUrl.HasPort())
                rUrl.SetPort (INETHIST_DEF_FTP_PORT);
            break;

        case INET_PROT_HTTP:
            if (!rUrl.HasPort())
                rUrl.SetPort (INETHIST_DEF_HTTP_PORT);
            if (!rUrl.HasURLPath())
                rUrl.SetURLPath (rtl::OString("/"));
            break;

        case INET_PROT_HTTPS:
            if (!rUrl.HasPort())
                rUrl.SetPort (INETHIST_DEF_HTTPS_PORT);
            if (!rUrl.HasURLPath())
                rUrl.SetURLPath (rtl::OString("/"));
            break;

        default:
            break;
    }
}

/*
 * PutUrl_Impl.
 */
void INetURLHistory::PutUrl_Impl (const INetURLObject &rUrl)
{
    DBG_ASSERT (m_pImpl, "PutUrl_Impl(): no Implementation");
    if (m_pImpl)
    {
        INetURLObject aHistUrl (rUrl);
        NormalizeUrl_Impl (aHistUrl);

        m_pImpl->putUrl (aHistUrl.GetMainURL(INetURLObject::NO_DECODE));
        Broadcast (INetURLHistoryHint (&rUrl));

        if (aHistUrl.HasMark())
        {
            aHistUrl.SetURL (aHistUrl.GetURLNoMark(INetURLObject::NO_DECODE),
                             INetURLObject::NOT_CANONIC);

            m_pImpl->putUrl (aHistUrl.GetMainURL(INetURLObject::NO_DECODE));
            Broadcast (INetURLHistoryHint (&aHistUrl));
        }
    }
}

/*
 * QueryUrl_Impl.
 */
sal_Bool INetURLHistory::QueryUrl_Impl (const INetURLObject &rUrl)
{
    DBG_ASSERT (m_pImpl, "QueryUrl_Impl(): no Implementation");
    if (m_pImpl)
    {
        INetURLObject aHistUrl (rUrl);
        NormalizeUrl_Impl (aHistUrl);

        return m_pImpl->queryUrl (aHistUrl.GetMainURL(INetURLObject::NO_DECODE));
    }
    return sal_False;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
