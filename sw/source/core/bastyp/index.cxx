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

#include <contentindex.hxx>

#include <assert.h>
#include <sal/log.hxx>

#include <crossrefbookmark.hxx>

ISwContentIndexOwner::~ISwContentIndexOwner() {}

SwContentIndex::SwContentIndex(const SwContentNode * pContentNode, sal_Int32 const nIdx)
    : m_nIndex( nIdx )
    , m_pContentNode( const_cast<SwContentNode*>(pContentNode) )
    , m_pNext( nullptr )
    , m_pPrev( nullptr )
{
    Init(m_nIndex);
}

SwContentIndex::SwContentIndex( const SwContentIndex& rIdx, short nDiff )
    : m_pContentNode( rIdx.m_pContentNode )
    , m_pNext( nullptr )
    , m_pPrev( nullptr )
{
    ChgValue( rIdx, rIdx.m_nIndex + nDiff );
}

SwContentIndex::SwContentIndex( const SwContentIndex& rIdx )
    : m_nIndex( rIdx.m_nIndex )
    , m_pContentNode( rIdx.m_pContentNode )
    , m_pNext( nullptr )
    , m_pPrev( nullptr )
{
    ChgValue( rIdx, rIdx.m_nIndex );
}

void SwContentIndex::Init(sal_Int32 const nIdx)
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

SwContentIndex& SwContentIndex::ChgValue( const SwContentIndex& rIdx, sal_Int32 nNewValue )
{
    assert(m_pContentNode == rIdx.m_pContentNode);
    if (!m_pContentNode)
    {
        m_nIndex = 0;
        return *this; // no IndexReg => no list to sort into; m_nIndex is 0
    }
    SwContentIndex* pFnd = const_cast<SwContentIndex*>(&rIdx);
    if (rIdx.m_nIndex > nNewValue) // move forwards
    {
        for (;;)
        {
            SwContentIndex* pPrv = pFnd->m_pPrev;
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
            SwContentIndex* pNxt = pFnd->m_pNext;
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

void SwContentIndex::Remove()
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

SwContentIndex& SwContentIndex::operator=( const SwContentIndex& rIdx )
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

SwContentIndex& SwContentIndex::Assign( const SwContentNode* pArr, sal_Int32 nIdx )
{
    if (pArr != m_pContentNode) // unregister!
    {
        Remove();
        m_pContentNode = const_cast<SwContentNode*>(pArr);
        m_pNext = m_pPrev = nullptr;
        Init(nIdx);
    }
    else if (m_nIndex != nIdx)
    {
        ChgValue( *this, nIdx );
    }
    return *this;
}

SwContentIndexReg::SwContentIndexReg()
    : m_pFirst( nullptr ), m_pLast( nullptr )
{
}

SwContentIndexReg::~SwContentIndexReg()
{
    assert(!m_pFirst && !m_pLast && "There are still indices registered");
}

void SwContentIndexReg::Update(
    SwContentIndex const & rIdx,
    const sal_Int32 nDiff,
    UpdateMode const eMode)
{
    SwContentIndex* pStart = const_cast<SwContentIndex*>(&rIdx);
    const sal_Int32 nNewVal = rIdx.m_nIndex;
    if (eMode & UpdateMode::Negative)
    {
        const sal_Int32 nLast = rIdx.m_nIndex + nDiff;
        pStart = rIdx.m_pNext;
        // skip over the ones that already have the right value
        while (pStart && pStart->m_nIndex == nNewVal)
            pStart = pStart->m_pNext;
        while (pStart && pStart->m_nIndex <= nLast)
        {
            pStart->m_nIndex = nNewVal;
            pStart = pStart->m_pNext;
        }
        while( pStart )
        {
            pStart->m_nIndex = pStart->m_nIndex - nDiff;
            pStart = pStart->m_pNext;
        }
    }
    else
    {
        while (pStart && pStart->m_nIndex == nNewVal)
        {
            pStart->m_nIndex = pStart->m_nIndex + nDiff;
            pStart = pStart->m_pPrev;
        }
        pStart = rIdx.m_pNext;
        while( pStart )
        {
            // HACK: avoid updating position of cross-ref bookmarks
            if (pStart->m_pOwner && pStart->m_pOwner->GetOwnerType() == SwContentIndexOwnerType::Mark
                && dynamic_cast< ::sw::mark::CrossRefBookmark const*>(pStart->m_pOwner))
                ; // do nothing
            else
                pStart->m_nIndex = pStart->m_nIndex + nDiff;
            pStart = pStart->m_pNext;
        }
    }
}

void SwContentIndexReg::MoveTo( SwContentNode& rArr )
{
    if (!(this != &rArr && m_pFirst))
        return;

    SwContentIndex * pIdx = const_cast<SwContentIndex*>(m_pFirst);
    SwContentIndex * pNext;
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

// SwContentIndex

sal_Int32 SwContentIndex::operator++()
{
    SAL_WARN_IF( !(m_nIndex < SAL_MAX_INT32), "sw.core",
                 "SwContentIndex::operator++() wraps around" );

    ChgValue( *this, m_nIndex+1 );
    return m_nIndex;
}

sal_Int32 SwContentIndex::operator--(int)
{
    SAL_WARN_IF( !(m_nIndex > 0), "sw.core",
                 "SwContentIndex::operator--(int) wraps around" );

    const sal_Int32 nOldIndex = m_nIndex;
    ChgValue( *this, m_nIndex-1 );
    return nOldIndex;
}

sal_Int32 SwContentIndex::operator--()
{
    SAL_WARN_IF( !( m_nIndex > 0), "sw.core",
                 "SwContentIndex::operator--() wraps around" );
    return ChgValue( *this, m_nIndex-1 ).m_nIndex;
}

sal_Int32 SwContentIndex::operator+=( sal_Int32 const nVal )
{
    SAL_WARN_IF( !(nVal > 0 ? m_nIndex <= SAL_MAX_INT32 - nVal : m_nIndex >= nVal), "sw.core",
                 "SwContentIndex SwContentIndex::operator+=(sal_Int32) wraps around" );
    return ChgValue( *this, m_nIndex + nVal ).m_nIndex;
}

sal_Int32 SwContentIndex::operator-=( sal_Int32 const nVal )
{
    SAL_WARN_IF( !(m_nIndex >= nVal), "sw.core",
                 "SwContentIndex::operator-=(sal_Int32) wraps around" );
    return ChgValue( *this, m_nIndex - nVal ).m_nIndex;
}

bool SwContentIndex::operator< ( const SwContentIndex & rIndex ) const
{
    // Attempt to compare indices into different arrays
    assert(m_pContentNode == rIndex.m_pContentNode);
    return m_nIndex < rIndex.m_nIndex;
}

bool SwContentIndex::operator<=( const SwContentIndex & rIndex ) const
{
    // Attempt to compare indices into different arrays
    assert(m_pContentNode == rIndex.m_pContentNode);
    return m_nIndex <= rIndex.m_nIndex;
}

bool SwContentIndex::operator> ( const SwContentIndex & rIndex ) const
{
    // Attempt to compare indices into different arrays
    assert(m_pContentNode == rIndex.m_pContentNode);
    return m_nIndex > rIndex.m_nIndex;
}

bool SwContentIndex::operator>=( const SwContentIndex & rIndex ) const
{
    // Attempt to compare indices into different arrays
    assert(m_pContentNode == rIndex.m_pContentNode);
    return m_nIndex >= rIndex.m_nIndex;
}

SwContentIndex& SwContentIndex::operator= ( sal_Int32 const nVal )
{
    if (m_nIndex != nVal)
        ChgValue( *this, nVal );

    return *this;
}

#endif

std::ostream& operator <<(std::ostream& s, const SwContentIndex& index)
{
    return s << "SwContentIndex offset (" << index.GetIndex() << ")";
}

std::ostream& operator <<(std::ostream& s, const SwNodeOffset& index)
{
    return s << sal_Int32(index);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
