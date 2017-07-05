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

#include "bparr.hxx"

#include <limits.h>
#include <string.h>
#include <algorithm>

/** Resize block management by this constant.
    As a result there are approx. 20 * MAXENTRY == 20000 entries available */
static const sal_uInt16 nBlockGrowSize = 20;

#if OSL_DEBUG_LEVEL > 2
#define CHECKIDX( p, n, i, c ) CheckIdx( p, n, i, c );
void CheckIdx( BlockInfo** ppInf, sal_uInt16 nBlock, sal_uLong nSize, sal_uInt16 nCur )
{
    assert( !nSize || nCur < nBlock ); // BigPtrArray: CurIndex invalid

    sal_uLong nIdx = 0;
    for( sal_uInt16 nCnt = 0; nCnt < nBlock; ++nCnt, ++ppInf )
    {
        nIdx += (*ppInf)->nElem;
        // Array with holes is not allowed
        assert( !nCnt || (*(ppInf-1))->nEnd + 1 == (*ppInf)->nStart );
    }
    assert(nIdx == nSize); // invalid count in nSize
}
#else
#define CHECKIDX( p, n, i, c )
#endif

BigPtrArray::BigPtrArray()
{
    m_nCur = 0;
    m_nSize = 0;
    m_vpInf.reserve( nBlockGrowSize );
}

BigPtrArray::~BigPtrArray()
{
    if( m_vpInf.size() )
    {
        BlockInfo** pp = m_vpInf.data();
        for( sal_uInt16 n = 0; n < sal_uInt16(m_vpInf.size()); ++n, ++pp )
        {
            delete *pp;
        }
    }
}

// Also moving is done simply here. Optimization is useless because of the
// division of this field into multiple parts.
void BigPtrArray::Move( sal_uLong from, sal_uLong to )
{
    if (from != to)
    {
        sal_uInt16 cur = Index2Block( from );
        BlockInfo* p = m_vpInf[ cur ];
        BigPtrEntry* pElem = p->mvData[ from - p->nStart ];
        Insert( pElem, to ); // insert first, then delete!
        Remove( ( to < from ) ? ( from + 1 ) : from );
    }
}

BigPtrEntry* BigPtrArray::operator[]( sal_uLong idx ) const
{
    assert(idx < m_nSize); // operator[]: Index out of bounds
    m_nCur = Index2Block( idx );
    BlockInfo* p = m_vpInf[ m_nCur ];
    return p->mvData[ idx - p->nStart ];
}

/** Search a block at a given position */
sal_uInt16 BigPtrArray::Index2Block( sal_uLong pos ) const
{
    // last used block?
    BlockInfo* p = m_vpInf[ m_nCur ];
    if( p->nStart <= pos && p->nEnd >= pos )
        return m_nCur;
    // Index = 0?
    if( !pos )
        return 0;

    // following one?
    if( m_nCur < ( m_vpInf.size() - 1 ) )
    {
        p = m_vpInf[ m_nCur+1 ];
        if( p->nStart <= pos && p->nEnd >= pos )
            return m_nCur+1;
    }
    // previous one?
    else if( pos < p->nStart && m_nCur > 0 )
    {
        p = m_vpInf[ m_nCur-1 ];
        if( p->nStart <= pos && p->nEnd >= pos )
            return m_nCur-1;
    }

    // binary search: always successful
    sal_uInt16 lower = 0, upper = m_vpInf.size() - 1;
    sal_uInt16 cur = 0;
    for(;;)
    {
        sal_uInt16 n = lower + ( upper - lower ) / 2;
        cur = ( n == cur ) ? n+1 : n;
        p = m_vpInf[ cur ];
        if( p->nStart <= pos && p->nEnd >= pos )
            return cur;

        if( p->nStart > pos )
            upper = cur;
        else
            lower = cur;
    }
}

/** Update all index areas

    @param pos last correct block (starting point)
*/
void BigPtrArray::UpdIndex( sal_uInt16 pos )
{
    BlockInfo** pp = m_vpInf.data() + pos;
    sal_uLong idx = (*pp)->nEnd + 1;
    while( ++pos < m_vpInf.size() )
    {
        BlockInfo* p = *++pp;
        p->nStart = idx;
        idx += p->nElem;
        p->nEnd = idx - 1;
    }
}

/** Create and insert new block

    Existing blocks will be moved rearward.

    @param pos Position at which the new block should be created.
*/
BlockInfo* BigPtrArray::InsBlock( sal_uInt16 pos )
{
    BlockInfo* p = new BlockInfo(this);
    m_vpInf.insert( m_vpInf.begin() + pos, p );

    if( pos )
        p->nStart = p->nEnd = m_vpInf[ pos-1 ]->nEnd + 1;
    else
        p->nStart = p->nEnd = 0;

    p->nEnd--;  // no elements
    return p;
}

void BigPtrArray::Insert( BigPtrEntry* pElem, sal_uLong pos )
{
    CHECKIDX( ppInf, nBlock, nSize, nCur );

    BlockInfo* p;
    sal_uInt16 cur;
    if( !m_nSize )
    {
        // special case: insert first element
        p = InsBlock( cur = 0 );
    }
    else if( pos == m_nSize )
    {
        // special case: insert at end
        cur = m_vpInf.size() - 1;
        p = m_vpInf[ cur ];
        if( p->nElem == MAXENTRY )
            // the last block is full, create a new one
            p = InsBlock( ++cur );
    }
    else
    {
        // standard case:
        cur = Index2Block( pos );
        p = m_vpInf[ cur ];
    }

    if( p->nElem == MAXENTRY )
    {
        // does the last entry fit into the next block?
        BlockInfo* q;
        if( cur < ( m_vpInf.size() - 1 ) && m_vpInf[ cur+1 ]->nElem < MAXENTRY )
        {
            q = m_vpInf[ cur+1 ];
            if( q->nElem )
            {
                int nCount = q->nElem;
                auto pFrom = q->mvData.begin() + nCount;
                auto pTo   = pFrom + 1;
                while( nCount-- )
                    ++( *--pTo = *--pFrom )->m_nOffset;
            }
            q->nStart--;
            q->nEnd--;
        }
        else
        {
            // If it does not fit, then insert a new block. But if there is more
            // than 50% space in the array then compress first.
            if( /*nBlock == nMaxBlock &&*/
                m_vpInf.size() > ( m_nSize / ( MAXENTRY / 2 ) ) &&
                cur >= Compress() )
            {
                // Something was moved before the current position and all
                // pointer might be invalid. Thus restart Insert.
                Insert( pElem, pos );
                return ;
            }

            q = InsBlock( cur+1 );
        }

        // entry does not fit anymore - clear space
        BigPtrEntry* pLast = p->mvData[ MAXENTRY-1 ];
        pLast->m_nOffset = 0;
        pLast->m_pBlock = q;

        q->mvData[ 0 ] = pLast;
        q->nElem++;
        q->nEnd++;

        p->nEnd--;
        p->nElem--;
    }
    // now we have free space - insert
    pos -= p->nStart;
    assert(pos < MAXENTRY);
    if( pos != p->nElem )
    {
        int nCount = p->nElem - sal_uInt16(pos);
        auto pFrom = p->mvData.begin() + p->nElem;
        auto pTo   = pFrom + 1;
        while( nCount-- )
            ++( *--pTo = *--pFrom )->m_nOffset;
    }
    // insert element and update indices
    pElem->m_nOffset = sal_uInt16(pos);
    pElem->m_pBlock = p;
    p->mvData[ pos ] = pElem;
    p->nEnd++;
    p->nElem++;
    m_nSize++;
    if( cur != ( m_vpInf.size() - 1 ) ) UpdIndex( cur );
    m_nCur = cur;

    CHECKIDX( ppInf, nBlock, nSize, nCur );
}

void BigPtrArray::Remove( sal_uLong pos, sal_uLong n )
{
    CHECKIDX( ppInf, nBlock, nSize, nCur );

    sal_uInt16 nBlkdel = 0;              // deleted blocks
    sal_uInt16 cur = Index2Block( pos ); // current block number
    sal_uInt16 nBlk1 = cur;              // 1st treated block
    sal_uInt16 nBlk1del = USHRT_MAX;     // 1st deleted block
    BlockInfo* p = m_vpInf[ cur ];
    pos -= p->nStart;

    sal_uLong nElem = n;
    while( nElem )
    {
        sal_uInt16 nel = p->nElem - sal_uInt16(pos);
        if( sal_uLong(nel) > nElem )
            nel = sal_uInt16(nElem);
        // move elements if needed
        if( ( pos + nel ) < sal_uLong(p->nElem) )
        {
            auto pTo = p->mvData.begin() + pos;
            auto pFrom = pTo + nel;
            int nCount = p->nElem - nel - sal_uInt16(pos);
            while( nCount-- )
            {
                *pTo = *pFrom++;
                (*pTo)->m_nOffset = (*pTo)->m_nOffset - nel;
                ++pTo;
            }
        }
        p->nEnd -= nel;
        p->nElem = p->nElem - nel;
        // possibly delete block completely
        if( !p->nElem )
        {
            nBlkdel++;
            if( USHRT_MAX == nBlk1del )
                nBlk1del = cur;
        }
        nElem -= nel;
        if( !nElem )
            break;
        p = m_vpInf[ ++cur ];
        pos = 0;
    }

    // update table if blocks were removed
    if( nBlkdel )
    {
        for( sal_uInt16 i = nBlk1del; i < ( nBlk1del + nBlkdel ); i++ )
            delete m_vpInf[ i ];

        m_vpInf.erase( m_vpInf.begin() + nBlk1del, m_vpInf.begin() + nBlk1del + nBlkdel );

        if( ( nBlk1del + nBlkdel ) < sal_uInt16(m_vpInf.size()) )
        {
            // UpdateIdx updates the successor thus start before first elem
            if( !nBlk1 )
            {
                p = m_vpInf[ 0 ];
                p->nStart = 0;
                p->nEnd = p->nElem-1;
            }
            else
            {
                --nBlk1;
            }
        }
    }

    m_nSize -= n;
    if( nBlk1 != ( m_vpInf.size() - 1 ) && m_nSize )
        UpdIndex( nBlk1 );
    m_nCur = nBlk1;

    // call Compress() if there is more than 50% space in the array
    if( m_vpInf.size() > ( m_nSize / ( MAXENTRY / 2 ) ) )
        Compress();

    CHECKIDX( ppInf, nBlock, nSize, nCur );
}

void BigPtrArray::Replace( sal_uLong idx, BigPtrEntry* pElem)
{
    assert(idx < m_nSize); // Index out of bounds
    m_nCur = Index2Block( idx );
    BlockInfo* p = m_vpInf[ m_nCur ];
    pElem->m_nOffset = sal_uInt16(idx - p->nStart);
    pElem->m_pBlock = p;
    p->mvData[ idx - p->nStart ] = pElem;
}

/** Compress the array */
sal_uInt16 BigPtrArray::Compress()
{
    CHECKIDX( ppInf, nBlock, nSize, nCur );

    // Iterate over InfoBlock array from beginning to end. If there is a deleted
    // block in between so move all following ones accordingly. The pointer <pp>
    // represents the "old" and <qq> the "new" array.
    BlockInfo** pp = m_vpInf.data(), **qq = pp;
    BlockInfo* p;
    BlockInfo* pLast(nullptr);                 // last empty block
    sal_uInt16 nLast = 0;                // missing elements
    sal_uInt16 nBlkdel = 0;              // number of deleted blocks
    sal_uInt16 nFirstChgPos = USHRT_MAX; // at which position was the 1st change?

    // convert fill percentage into number of remaining elements
    short const nMax = MAXENTRY - (long) MAXENTRY * COMPRESSLVL / 100;

    for( sal_uInt16 cur = 0; cur < sal_uInt16(m_vpInf.size()); ++cur )
    {
        p = *pp++;
        sal_uInt16 n = p->nElem;
        // Check if a not completely full block will be ignored. This happens if
        // the current block would have to be split but the filling of the
        // inspected block is already over its threshold value. In this case we
        // do not fill more (it's expensive because of a double memmove() call)
        if( nLast && ( n > nLast ) && ( nLast < nMax ) )
            nLast = 0;
        if( nLast )
        {
            if( USHRT_MAX == nFirstChgPos )
                nFirstChgPos = cur;

            // Not full yet? Than fill up.
            if( n > nLast )
                n = nLast;

            // move elements from current to last block
            auto pElem = pLast->mvData.begin() + pLast->nElem;
            auto pFrom = p->mvData.begin();
            for( sal_uInt16 nCount = n, nOff = pLast->nElem;
                            nCount; --nCount, ++pElem )
            {
                *pElem = *pFrom++;
                (*pElem)->m_pBlock = pLast;
                (*pElem)->m_nOffset = nOff++;
            }

            // adjustment
            pLast->nElem = pLast->nElem + n;
            nLast = nLast - n;
            p->nElem = p->nElem - n;

            // Is the current block now empty as a result?
            if( !p->nElem )
            {
                // than remove
                delete   p;
                p = nullptr;
                *(pp-1) = nullptr;
                ++nBlkdel;
            }
            else
            {
                pElem = p->mvData.begin();
                pFrom = pElem + n;
                int nCount = p->nElem;
                while( nCount-- )
                {
                    *pElem = *pFrom++;
                    (*pElem)->m_nOffset = (*pElem)->m_nOffset - n;
                    ++pElem;
                }
            }
        }

        if( p ) // BlockInfo was not deleted
        {
            *qq++ = p; // adjust to correct position

            // keep the potentially existing last half-full block
            if( !nLast && p->nElem < MAXENTRY )
            {
                pLast = p;
                nLast = MAXENTRY - p->nElem;
            }
        }
    }

    // if blocks were deleted shrink BlockInfo array if needed
    if( nBlkdel )
        m_vpInf.erase( std::remove_if(m_vpInf.begin(), m_vpInf.end(), [](BlockInfo* bi) { return !bi; }),
                       m_vpInf.end() );

    // and re-index
    p = m_vpInf[ 0 ];
    p->nEnd = p->nElem - 1;
    UpdIndex( 0 );

    if( m_nCur >= nFirstChgPos )
        m_nCur = 0;

    CHECKIDX( ppInf, nBlock, nSize, nCur );

    return nFirstChgPos;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
