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

#include <index.hxx>

#include <assert.h>
#include <sal/log.hxx>

#include <crossrefbookmark.hxx>

SwIndex::SwIndex(SwContentNode *const pContentNode, sal_Int32 const nIdx)
    : m_nIndex( nIdx )
    , m_pContentNode( pContentNode )
    , m_pNext( nullptr )
    , m_pPrev( nullptr )
    , m_pMark( nullptr )
{
    Init(m_nIndex);
}

SwIndex::SwIndex( const SwIndex& rIdx, short nDiff )
    : m_pContentNode( rIdx.m_pContentNode )
    , m_pNext( nullptr )
    , m_pPrev( nullptr )
    , m_pMark( nullptr )
{
    ChgValue( rIdx, rIdx.m_nIndex + nDiff );
}

SwIndex::SwIndex( const SwIndex& rIdx )
    : m_nIndex( rIdx.m_nIndex )
    , m_pContentNode( rIdx.m_pContentNode )
    , m_pNext( nullptr )
    , m_pPrev( nullptr )
    , m_pMark( nullptr )
{
    ChgValue( rIdx, rIdx.m_nIndex );
}

void SwIndex::Init(sal_Int32 const nIdx)
{
    if (!m_pContentNode)
    {
        m_nIndex = 0; // always 0 if no IndexReg
    }
    else if (!m_pContentNode->m_pFirst) // first Index?
    {
        assert(!m_pContentNode->m_pLast);
        m_pContentNode->m_pFirst = m_pContentNode->m_pLast = this;
        m_nIndex = nIdx;
    }
    else if (nIdx > ((m_pContentNode->m_pLast->m_nIndex
                        - m_pContentNode->m_pFirst->m_nIndex) / 2))
    {
        ChgValue( *m_pContentNode->m_pLast, nIdx );
    }
    else
    {
        ChgValue( *m_pContentNode->m_pFirst, nIdx );
    }
}

SwIndex& SwIndex::ChgValue( const SwIndex& rIdx, sal_Int32 nNewValue )
{
    assert(m_pContentNode == rIdx.m_pContentNode);
    if (!m_pContentNode)
    {
        m_nIndex = 0;
        return *this; // no IndexReg => no list to sort into; m_nIndex is 0
    }
    SwIndex* pFnd = const_cast<SwIndex*>(&rIdx);
    if (rIdx.m_nIndex > nNewValue) // move forwards
    {
        for (;;)
        {
            SwIndex* pPrv = pFnd->m_pPrev;
            if (!pPrv || pPrv->m_nIndex <= nNewValue)
                break;
            pFnd = pPrv;
        }

        if( pFnd != this )
        {
            // remove from list at old position
            Remove();

            m_pNext = pFnd;
            m_pPrev = pFnd->m_pPrev;
            if (m_pPrev)
                m_pPrev->m_pNext = this;
            else
                m_pContentNode->m_pFirst = this;
            pFnd->m_pPrev = this;
        }
    }
    else if (rIdx.m_nIndex < nNewValue)
    {
        for (;;)
        {
            SwIndex* pNxt = pFnd->m_pNext;
            if (!pNxt || pNxt->m_nIndex >= nNewValue)
                break;
            pFnd = pNxt;
        }

        if( pFnd != this )
        {
            // remove from list at old position
            Remove();

            m_pPrev = pFnd;
            m_pNext = pFnd->m_pNext;
            if (m_pNext)
                m_pNext->m_pPrev = this;
            else
                m_pContentNode->m_pLast = this;
            pFnd->m_pNext = this;
        }
    }
    else if( pFnd != this )
    {
        // remove from list at old position
        Remove();

        m_pPrev = pFnd; // == &rIdx here
        m_pNext = rIdx.m_pNext;
        m_pPrev->m_pNext = this;

        if (!m_pNext) // last in the list
            m_pContentNode->m_pLast = this;
        else
            m_pNext->m_pPrev = this;
    }

    if (m_pContentNode->m_pFirst == m_pNext)
        m_pContentNode->m_pFirst = this;
    if (m_pContentNode->m_pLast == m_pPrev)
        m_pContentNode->m_pLast = this;

    m_nIndex = nNewValue;

    return *this;
}

void SwIndex::Remove()
{
    if (!m_pContentNode)
    {
        assert(!m_pPrev && !m_pNext);
        return;
    }

    if (m_pPrev)
    {
        m_pPrev->m_pNext = m_pNext;
    }
    else if (m_pContentNode->m_pFirst == this)
    {
        m_pContentNode->m_pFirst = m_pNext;
    }

    if (m_pNext)
    {
        m_pNext->m_pPrev = m_pPrev;
    }
    else if (m_pContentNode->m_pLast == this)
    {
        m_pContentNode->m_pLast = m_pPrev;
    }
}

SwIndex& SwIndex::operator=( const SwIndex& rIdx )
{
    bool bEqual;
    if (rIdx.m_pContentNode != m_pContentNode) // unregister!
    {
        Remove();
        m_pContentNode = rIdx.m_pContentNode;
        m_pNext = m_pPrev = nullptr;
        bEqual = false;
    }
    else
        bEqual = rIdx.m_nIndex == m_nIndex;

    if( !bEqual )
        ChgValue( rIdx, rIdx.m_nIndex );
    return *this;
}

SwIndex& SwIndex::Assign( SwContentNode* pArr, sal_Int32 nIdx )
{
    if (pArr != m_pContentNode) // unregister!
    {
        Remove();
        m_pContentNode = pArr;
        m_pNext = m_pPrev = nullptr;
        Init(nIdx);
    }
    else if (m_nIndex != nIdx)
    {
        ChgValue( *this, nIdx );
    }
    return *this;
}

void SwIndex::SetMark(const sw::mark::IMark* pMark)
{
    m_pMark = pMark;
}

SwIndexReg::SwIndexReg()
    : m_pFirst( nullptr ), m_pLast( nullptr )
{
}

SwIndexReg::~SwIndexReg()
{
    assert(!m_pFirst && !m_pLast && "There are still indices registered");
}

void SwIndexReg::Update(
    SwIndex const & rIdx,
    const sal_Int32 nDiff,
    const bool bNeg,
    const bool /* argument is only used in derived class*/ )
{
    SwIndex* pStt = const_cast<SwIndex*>(&rIdx);
    const sal_Int32 nNewVal = rIdx.m_nIndex;
    if( bNeg )
    {
        const sal_Int32 nLast = rIdx.GetIndex() + nDiff;
        while (pStt && pStt->m_nIndex == nNewVal)
        {
            pStt->m_nIndex = nNewVal;
            pStt = pStt->m_pPrev;
        }
        pStt = rIdx.m_pNext;
        while (pStt && pStt->m_nIndex >= nNewVal
                    && pStt->m_nIndex <= nLast)
        {
            pStt->m_nIndex = nNewVal;
            pStt = pStt->m_pNext;
        }
        while( pStt )
        {
            pStt->m_nIndex = pStt->m_nIndex - nDiff;
            pStt = pStt->m_pNext;
        }
    }
    else
    {
        while (pStt && pStt->m_nIndex == nNewVal)
        {
            pStt->m_nIndex = pStt->m_nIndex + nDiff;
            pStt = pStt->m_pPrev;
        }
        pStt = rIdx.m_pNext;
        while( pStt )
        {
            // HACK: avoid updating position of cross-ref bookmarks
            if (!pStt->m_pMark || nullptr == dynamic_cast<
                    ::sw::mark::CrossRefBookmark const*>(pStt->m_pMark))
            {
                pStt->m_nIndex = pStt->m_nIndex + nDiff;
            }
            pStt = pStt->m_pNext;
        }
    }
}

void SwIndexReg::MoveTo( SwContentNode& rArr )
{
    if (!(this != &rArr && m_pFirst))
        return;

    SwIndex * pIdx = const_cast<SwIndex*>(m_pFirst);
    SwIndex * pNext;
    while( pIdx )
    {
        pNext = pIdx->m_pNext;
        pIdx->Assign( &rArr, pIdx->GetIndex() );
        pIdx = pNext;
    }
    m_pFirst = nullptr;
    m_pLast = nullptr;
}

#ifdef DBG_UTIL

// SwIndex

sal_Int32 SwIndex::operator++()
{
    SAL_WARN_IF( !(m_nIndex < SAL_MAX_INT32), "sw.core",
                 "SwIndex::operator++() wraps around" );

    ChgValue( *this, m_nIndex+1 );
    return m_nIndex;
}

sal_Int32 SwIndex::operator--(int)
{
    SAL_WARN_IF( !(m_nIndex > 0), "sw.core",
                 "SwIndex::operator--(int) wraps around" );

    const sal_Int32 nOldIndex = m_nIndex;
    ChgValue( *this, m_nIndex-1 );
    return nOldIndex;
}

sal_Int32 SwIndex::operator--()
{
    SAL_WARN_IF( !( m_nIndex > 0), "sw.core",
                 "SwIndex::operator--() wraps around" );
    return ChgValue( *this, m_nIndex-1 ).m_nIndex;
}

sal_Int32 SwIndex::operator+=( sal_Int32 const nVal )
{
    SAL_WARN_IF( !(m_nIndex <= SAL_MAX_INT32 - nVal), "sw.core",
                 "SwIndex SwIndex::operator+=(sal_Int32) wraps around" );
    return ChgValue( *this, m_nIndex + nVal ).m_nIndex;
}

sal_Int32 SwIndex::operator-=( sal_Int32 const nVal )
{
    SAL_WARN_IF( !(m_nIndex >= nVal), "sw.core",
                 "SwIndex::operator-=(sal_Int32) wraps around" );
    return ChgValue( *this, m_nIndex - nVal ).m_nIndex;
}

bool SwIndex::operator< ( const SwIndex & rIndex ) const
{
    // Attempt to compare indices into different arrays
    assert(m_pContentNode == rIndex.m_pContentNode);
    return m_nIndex < rIndex.m_nIndex;
}

bool SwIndex::operator<=( const SwIndex & rIndex ) const
{
    // Attempt to compare indices into different arrays
    assert(m_pContentNode == rIndex.m_pContentNode);
    return m_nIndex <= rIndex.m_nIndex;
}

bool SwIndex::operator> ( const SwIndex & rIndex ) const
{
    // Attempt to compare indices into different arrays
    assert(m_pContentNode == rIndex.m_pContentNode);
    return m_nIndex > rIndex.m_nIndex;
}

bool SwIndex::operator>=( const SwIndex & rIndex ) const
{
    // Attempt to compare indices into different arrays
    assert(m_pContentNode == rIndex.m_pContentNode);
    return m_nIndex >= rIndex.m_nIndex;
}

SwIndex& SwIndex::operator= ( sal_Int32 const nVal )
{
    if (m_nIndex != nVal)
        ChgValue( *this, nVal );

    return *this;
}

#endif

std::ostream& operator <<(std::ostream& s, const SwIndex& index)
{
    return s << "SwIndex offset (" << index.GetIndex() << ")";
}

std::ostream& operator <<(std::ostream& s, const SwNodeOffset& index)
{
    return s << sal_Int32(index);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
