/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svl.hxx"
#include <svl/inethist.hxx>

#ifndef INCLUDED_ALGORITHM
#include <algorithm>
#define INCLUDED_ALGORITHM
#endif
#include "rtl/instance.hxx"
#include "rtl/crc.h"
#include "rtl/memory.h"
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

    void downheap (hash_entry a[], sal_uInt16 n, sal_uInt16 k);
    void heapsort (hash_entry a[], sal_uInt16 n);

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
 * downheap.
 */
void INetURLHistory_Impl::downheap (hash_entry a[], sal_uInt16 n, sal_uInt16 k)
{
    hash_entry h = a[k];
    while (k < n / 2)
    {
        sal_uInt16 i = k + k + 1;
        if (((i + 1) < n) && (a[i] < a[i + 1])) i++;
        if (!(h < a[i])) break;
        a[k] = a[i];
        k = i;
    }
    a[k] = h;
}

/*
 * heapsort.
 */
void INetURLHistory_Impl::heapsort (hash_entry a[], sal_uInt16 n)
{
    hash_entry h;

    for (sal_uInt16 k = (n - 1) / 2 + 1; k > 0; k--)
        downheap (a, n, k - 1);

    while (n > 0)
    {
        h        = a[0    ];
        a[0    ] = a[n - 1];
        a[n - 1] = h;
        downheap (a, --n, 0);
    }
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
        rtl_moveMemory (
            &m_pHash[nSI    ],
            &m_pHash[nSI + 1],
            (nDI - nSI) * sizeof(hash_entry));
    }
    if (nSI > nDI)
    {
        // shift right.
        rtl_moveMemory (
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
                rUrl.SetURLPath ("/");
            break;

        case INET_PROT_HTTPS:
            if (!rUrl.HasPort())
                rUrl.SetPort (INETHIST_DEF_HTTPS_PORT);
            if (!rUrl.HasURLPath())
                rUrl.SetURLPath ("/");
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
