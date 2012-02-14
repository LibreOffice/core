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


#include <assert.h>
#include <stdlib.h>             // fuer qsort
#include <tools/solar.h>
#include <tools/string.hxx>

#include "index.hxx"


TYPEINIT0(SwIndexReg);  // rtti


SwIndex::SwIndex(SwIndexReg *const pReg, xub_StrLen const nIdx)
    : m_nIndex( nIdx )
    , m_pIndexReg( pReg )
    , m_pNext( 0 )
    , m_pPrev( 0 )
{
    Init(m_nIndex);
}

SwIndex::SwIndex( const SwIndex& rIdx, short nDiff )
    : m_pIndexReg( rIdx.m_pIndexReg )
    , m_pNext( 0 )
    , m_pPrev( 0 )
{
    ChgValue( rIdx, rIdx.m_nIndex + nDiff );
}

SwIndex::SwIndex( const SwIndex& rIdx )
    : m_nIndex( rIdx.m_nIndex )
    , m_pIndexReg( rIdx.m_pIndexReg )
    , m_pNext( 0 )
    , m_pPrev( 0 )
{
    ChgValue( rIdx, rIdx.m_nIndex );
}

void SwIndex::Init(xub_StrLen const nIdx)
{
    if (!m_pIndexReg)
    {
        m_nIndex = 0; // always 0 if no IndexReg
    }
    else if (!m_pIndexReg->m_pFirst) // first Index?
    {
        assert(!m_pIndexReg->m_pLast);
        m_pIndexReg->m_pFirst = m_pIndexReg->m_pLast = this;
        m_nIndex = nIdx;
    }
    else if (nIdx > ((m_pIndexReg->m_pLast->m_nIndex
                        - m_pIndexReg->m_pFirst->m_nIndex) / 2))
    {
        ChgValue( *m_pIndexReg->m_pLast, nIdx );
    }
    else
    {
        ChgValue( *m_pIndexReg->m_pFirst, nIdx );
    }
}

SwIndex& SwIndex::ChgValue( const SwIndex& rIdx, xub_StrLen nNewValue )
{
    assert(m_pIndexReg == rIdx.m_pIndexReg);
    if (!m_pIndexReg)
    {
        return *this; // no IndexReg => no list to sort into; m_nIndex is 0
    }
    SwIndex* pFnd = const_cast<SwIndex*>(&rIdx);
    if (rIdx.m_nIndex > nNewValue) // move forwards
    {
        SwIndex* pPrv;
        while ((0 != (pPrv = pFnd->m_pPrev)) && (pPrv->m_nIndex > nNewValue))
            pFnd = pPrv;

        if( pFnd != this )
        {
            // remove from list at old position
            Remove();

            m_pNext = pFnd;
            m_pPrev = pFnd->m_pPrev;
            if (m_pPrev)
                m_pPrev->m_pNext = this;
            else
                m_pIndexReg->m_pFirst = this;
            pFnd->m_pPrev = this;
        }
    }
    else if (rIdx.m_nIndex < nNewValue)
    {
        SwIndex* pNxt;
        while ((0 != (pNxt = pFnd->m_pNext)) && (pNxt->m_nIndex < nNewValue))
            pFnd = pNxt;

        if( pFnd != this )
        {
            // remove from list at old position
            Remove();

            m_pPrev = pFnd;
            m_pNext = pFnd->m_pNext;
            if (m_pNext)
                m_pNext->m_pPrev = this;
            else
                m_pIndexReg->m_pLast = this;
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
            m_pIndexReg->m_pLast = this;
        else
            m_pNext->m_pPrev = this;
    }

    if (m_pIndexReg->m_pFirst == m_pNext)
        m_pIndexReg->m_pFirst = this;
    if (m_pIndexReg->m_pLast == m_pPrev)
        m_pIndexReg->m_pLast = this;

    m_nIndex = nNewValue;

    return *this;
}

void SwIndex::Remove()
{
    if (!m_pIndexReg)
    {
        assert(!m_pPrev && !m_pNext);
        return;
    }

    if (m_pPrev)
    {
        m_pPrev->m_pNext = m_pNext;
    }
    else if (m_pIndexReg->m_pFirst == this)
    {
        m_pIndexReg->m_pFirst = m_pNext;
    }

    if (m_pNext)
    {
        m_pNext->m_pPrev = m_pPrev;
    }
    else if (m_pIndexReg->m_pLast == this)
    {
        m_pIndexReg->m_pLast = m_pPrev;
    }
}

/*************************************************************************
|*    SwIndex & SwIndex::operator=( const SwIndex & aSwIndex )
*************************************************************************/
SwIndex& SwIndex::operator=( const SwIndex& rIdx )
{
    bool bEqual;
    if (rIdx.m_pIndexReg != m_pIndexReg) // unregister!
    {
        Remove();
        m_pIndexReg = rIdx.m_pIndexReg;
        m_pNext = m_pPrev = 0;
        bEqual = false;
    }
    else
        bEqual = rIdx.m_nIndex == m_nIndex;

    if( !bEqual )
        ChgValue( rIdx, rIdx.m_nIndex );
    return *this;
}

/*************************************************************************
|*    SwIndex &SwIndex::Assign
*************************************************************************/
SwIndex& SwIndex::Assign( SwIndexReg* pArr, xub_StrLen nIdx )
{
    if (pArr != m_pIndexReg) // unregister!
    {
        Remove();
        m_pIndexReg = pArr;
        m_pNext = m_pPrev = 0;
        Init(nIdx);
    }
    else if (m_nIndex != nIdx)
        ChgValue( *this, nIdx );
    return *this;
}

// SwIndexReg ///////////////////////////////////////////////////////

SwIndexReg::SwIndexReg()
    : m_pFirst( 0 ), m_pLast( 0 )
{
}

SwIndexReg::~SwIndexReg()
{
    assert(!m_pFirst && !m_pLast); // There are still indices registered
}

void SwIndexReg::Update( SwIndex const & rIdx, const xub_StrLen nDiff,
    const bool bNeg, const bool /* argument is only used in derived class*/ )
{
    SwIndex* pStt = const_cast<SwIndex*>(&rIdx);
    xub_StrLen nNewVal = rIdx.m_nIndex;
    if( bNeg )
    {
        xub_StrLen nLast = rIdx.GetIndex() + nDiff;
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
            pStt->m_nIndex = pStt->m_nIndex + nDiff;
            pStt = pStt->m_pNext;
        }
    }
}

#ifdef DBG_UTIL

/*************************************************************************
|*    SwIndex::operator++()
*************************************************************************/
xub_StrLen SwIndex::operator++(int)
{
    OSL_ASSERT( m_nIndex < INVALID_INDEX );

    xub_StrLen nOldIndex = m_nIndex;
    ChgValue( *this, m_nIndex+1 );
    return nOldIndex;
}

xub_StrLen SwIndex::operator++()
{
    OSL_ASSERT( m_nIndex < INVALID_INDEX );

    ChgValue( *this, m_nIndex+1 );
    return m_nIndex;
}

/*************************************************************************
|*    SwIndex::operator--()
*************************************************************************/
xub_StrLen SwIndex::operator--(int)
{
    OSL_ASSERT( m_nIndex );

    xub_StrLen nOldIndex = m_nIndex;
    ChgValue( *this, m_nIndex-1 );
    return nOldIndex;
}

xub_StrLen SwIndex::operator--()
{
    OSL_ASSERT( m_nIndex );
    return ChgValue( *this, m_nIndex-1 ).m_nIndex;
}

/*************************************************************************
|*    SwIndex::operator+=( xub_StrLen )
*************************************************************************/
xub_StrLen SwIndex::operator+=( xub_StrLen const nVal )
{
    OSL_ASSERT( m_nIndex < INVALID_INDEX - nVal );
    return ChgValue( *this, m_nIndex + nVal ).m_nIndex;
}

/*************************************************************************
|*    SwIndex::operator-=( xub_StrLen )
*************************************************************************/
xub_StrLen SwIndex::operator-=( xub_StrLen const nVal )
{
    OSL_ASSERT( m_nIndex >= nVal );
    return ChgValue( *this, m_nIndex - nVal ).m_nIndex;
}

/*************************************************************************
|*    SwIndex::operator+=( const SwIndex & )
*************************************************************************/
xub_StrLen SwIndex::operator+=( const SwIndex & rIndex )
{
    OSL_ASSERT( m_nIndex < INVALID_INDEX - rIndex.m_nIndex );
    return ChgValue( *this, m_nIndex + rIndex.m_nIndex ).m_nIndex;
}

/*************************************************************************
|*    SwIndex::operator-=( const SwIndex & )
*************************************************************************/
xub_StrLen SwIndex::operator-=( const SwIndex & rIndex )
{
    OSL_ASSERT( m_nIndex >= rIndex.m_nIndex );
    return ChgValue( *this, m_nIndex - rIndex.m_nIndex ).m_nIndex;
}

/*************************************************************************
|*    SwIndex::operator<( const SwIndex & )
*************************************************************************/
bool SwIndex::operator< ( const SwIndex & rIndex ) const
{
    // Attempt to compare indices into different arrays
    assert(m_pIndexReg == rIndex.m_pIndexReg);
    return m_nIndex < rIndex.m_nIndex;
}

/*************************************************************************
|*    SwIndex::operator<=( const SwIndex & )
*************************************************************************/
bool SwIndex::operator<=( const SwIndex & rIndex ) const
{
    // Attempt to compare indices into different arrays
    assert(m_pIndexReg == rIndex.m_pIndexReg);
    return m_nIndex <= rIndex.m_nIndex;
}

/*************************************************************************
|*    SwIndex::operator>( const SwIndex & )
*************************************************************************/
bool SwIndex::operator> ( const SwIndex & rIndex ) const
{
    // Attempt to compare indices into different arrays
    assert(m_pIndexReg == rIndex.m_pIndexReg);
    return m_nIndex > rIndex.m_nIndex;
}

/*************************************************************************
|*    SwIndex::operator>=( const SwIndex & )
*************************************************************************/
bool SwIndex::operator>=( const SwIndex & rIndex ) const
{
    // Attempt to compare indices into different arrays
    assert(m_pIndexReg == rIndex.m_pIndexReg);
    return m_nIndex >= rIndex.m_nIndex;
}

/*************************************************************************
|*    SwIndex & SwIndex::operator=( xub_StrLen )
*************************************************************************/
SwIndex& SwIndex::operator= ( xub_StrLen const nVal )
{
    if (m_nIndex != nVal)
        ChgValue( *this, nVal );

    return *this;
}

#endif // ifdef DBG_UTIL

void SwIndexReg::MoveTo( SwIndexReg& rArr )
{
    if (this != &rArr && m_pFirst)
    {
        SwIndex * pIdx = const_cast<SwIndex*>(m_pFirst);
        SwIndex * pNext;
        while( pIdx )
        {
            pNext = pIdx->m_pNext;
            pIdx->Assign( &rArr, pIdx->GetIndex() );
            pIdx = pNext;
        }
        m_pFirst = 0, m_pLast = 0;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
