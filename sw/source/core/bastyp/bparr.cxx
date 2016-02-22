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
    nBlock = nCur = 0;
    nSize = 0;
    nMaxBlock = nBlockGrowSize;
    ppInf = new BlockInfo* [ nMaxBlock ];
}

BigPtrArray::~BigPtrArray()
{
    if( nBlock )
    {
        BlockInfo** pp = ppInf;
        for( sal_uInt16 n = 0; n < nBlock; ++n, ++pp )
        {
            delete[] (*pp)->pData;
            delete    *pp;
        }
    }
    delete[] ppInf;
}

// Also moving is done simply here. Optimization is useless because of the
// division of this field into multiple parts.
void BigPtrArray::Move( sal_uLong from, sal_uLong to )
{
    if (from != to)
    {
        sal_uInt16 cur = Index2Block( from );
        BlockInfo* p = ppInf[ cur ];
        ElementPtr pElem = p->pData[ from - p->nStart ];
        Insert( pElem, to ); // insert first, then delete!
        Remove( ( to < from ) ? ( from + 1 ) : from );
    }
}

ElementPtr BigPtrArray::operator[]( sal_uLong idx ) const
{
    assert(idx < nSize); // operator[]: Index out of bounds
    nCur = Index2Block( idx );
    BlockInfo* p = ppInf[ nCur ];
    return p->pData[ idx - p->nStart ];
}

/** Search a block at a given position */
sal_uInt16 BigPtrArray::Index2Block( sal_uLong pos ) const
{
    // last used block?
    BlockInfo* p = ppInf[ nCur ];
    if( p->nStart <= pos && p->nEnd >= pos )
        return nCur;
    // Index = 0?
    if( !pos )
        return 0;

    // following one?
    if( nCur < ( nBlock - 1 ) )
    {
        p = ppInf[ nCur+1 ];
        if( p->nStart <= pos && p->nEnd >= pos )
            return nCur+1;
    }
    // previous one?
    else if( pos < p->nStart && nCur > 0 )
    {
        p = ppInf[ nCur-1 ];
        if( p->nStart <= pos && p->nEnd >= pos )
            return nCur-1;
    }

    // binary search: always successful
    sal_uInt16 lower = 0, upper = nBlock - 1;
    sal_uInt16 cur = 0;
    for(;;)
    {
        sal_uInt16 n = lower + ( upper - lower ) / 2;
        cur = ( n == cur ) ? n+1 : n;
        p = ppInf[ cur ];
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
    BlockInfo** pp = ppInf + pos;
    sal_uLong idx = (*pp)->nEnd + 1;
    while( ++pos < nBlock )
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
    if( nBlock == nMaxBlock )
    {
        // than extend the array first
        BlockInfo** ppNew = new BlockInfo* [ nMaxBlock + nBlockGrowSize ];
        memcpy( ppNew, ppInf, nMaxBlock * sizeof( BlockInfo* ));
        delete[] ppInf;
        nMaxBlock += nBlockGrowSize;
        ppInf = ppNew;
    }
    if( pos != nBlock )
    {
        memmove( ppInf + pos+1, ppInf + pos,
                 ( nBlock - pos ) * sizeof( BlockInfo* ));
    }
    ++nBlock;
    BlockInfo* p = new BlockInfo;
    ppInf[ pos ] = p;

    if( pos )
        p->nStart = p->nEnd = ppInf[ pos-1 ]->nEnd + 1;
    else
        p->nStart = p->nEnd = 0;

    p->nEnd--;  // no elements
    p->nElem = 0;
    p->pData = new ElementPtr [ MAXENTRY ];
    p->pBigArr = this;
    return p;
}

void BigPtrArray::BlockDel( sal_uInt16 nDel )
{
    nBlock = nBlock - nDel;
    if( nMaxBlock - nBlock > nBlockGrowSize )
    {
        // than shrink array
        nDel = (( nBlock / nBlockGrowSize ) + 1 ) * nBlockGrowSize;
        BlockInfo** ppNew = new BlockInfo* [ nDel ];
        memcpy( ppNew, ppInf, nBlock * sizeof( BlockInfo* ));
        delete[] ppInf;
        ppInf = ppNew;
        nMaxBlock = nDel;
    }
}

void BigPtrArray::Insert( const ElementPtr& rElem, sal_uLong pos )
{
    CHECKIDX( ppInf, nBlock, nSize, nCur );

    BlockInfo* p;
    sal_uInt16 cur;
    if( !nSize )
    {
        // special case: insert first element
        p = InsBlock( cur = 0 );
    }
    else if( pos == nSize )
    {
        // special case: insert at end
        cur = nBlock - 1;
        p = ppInf[ cur ];
        if( p->nElem == MAXENTRY )
            // the last block is full, create a new one
            p = InsBlock( ++cur );
    }
    else
    {
        // standard case:
        cur = Index2Block( pos );
        p = ppInf[ cur ];
    }

    if( p->nElem == MAXENTRY )
    {
        // does the last entry fit into the next block?
        BlockInfo* q;
        if( cur < ( nBlock - 1 ) && ppInf[ cur+1 ]->nElem < MAXENTRY )
        {
            q = ppInf[ cur+1 ];
            if( q->nElem )
            {
                int nCount = q->nElem;
                ElementPtr *pFrom = q->pData + nCount,
                                    *pTo = pFrom+1;
                while( nCount-- )
                    ++( *--pTo = *--pFrom )->nOffset;
            }
            q->nStart--;
            q->nEnd--;
        }
        else
        {
            // If it does not fit, then insert a new block. But if there is more
            // than 50% space in the array then compress first.
            if( /*nBlock == nMaxBlock &&*/
                nBlock > ( nSize / ( MAXENTRY / 2 ) ) &&
                cur >= Compress() )
            {
                // Something was moved before the current position and all
                // pointer might be invalid. Thus restart Insert.
                Insert( rElem, pos );
                return ;
            }

            q = InsBlock( cur+1 );
        }

        // entry does not fit anymore - clear space
        ElementPtr pLast = p->pData[ MAXENTRY-1 ];
        pLast->nOffset = 0;
        pLast->pBlock = q;

        q->pData[ 0 ] = pLast;
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
        ElementPtr *pFrom = p->pData + p->nElem;
        ElementPtr *pTo   = pFrom + 1;
        while( nCount-- )
            ++( *--pTo = *--pFrom )->nOffset;
    }
    // insert element and update indices
    rElem->nOffset = sal_uInt16(pos);
    rElem->pBlock = p;
    p->pData[ pos ] = rElem;
    p->nEnd++;
    p->nElem++;
    nSize++;
    if( cur != ( nBlock - 1 ) ) UpdIndex( cur );
    nCur = cur;

    CHECKIDX( ppInf, nBlock, nSize, nCur );
}

void BigPtrArray::Remove( sal_uLong pos, sal_uLong n )
{
    CHECKIDX( ppInf, nBlock, nSize, nCur );

    sal_uInt16 nBlkdel = 0;              // deleted blocks
    sal_uInt16 cur = Index2Block( pos ); // current block number
    sal_uInt16 nBlk1 = cur;              // 1st treated block
    sal_uInt16 nBlk1del = USHRT_MAX;     // 1st deleted block
    BlockInfo* p = ppInf[ cur ];
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
            ElementPtr *pTo = p->pData + pos;
            ElementPtr *pFrom = pTo + nel;
            int nCount = p->nElem - nel - sal_uInt16(pos);
            while( nCount-- )
            {
                *pTo = *pFrom++;
                (*pTo)->nOffset = (*pTo)->nOffset - nel;
                ++pTo;
            }
        }
        p->nEnd -= nel;
        p->nElem = p->nElem - nel;
        // possibly delete block completely
        if( !p->nElem )
        {
            delete[] p->pData;
            nBlkdel++;
            if( USHRT_MAX == nBlk1del )
                nBlk1del = cur;
        }
        nElem -= nel;
        if( !nElem )
            break;
        p = ppInf[ ++cur ];
        pos = 0;
    }

    // update table if blocks were removed
    if( nBlkdel )
    {
        for( sal_uInt16 i = nBlk1del; i < ( nBlk1del + nBlkdel ); i++ )
            delete ppInf[ i ];

        if( ( nBlk1del + nBlkdel ) < nBlock )
        {
            memmove( ppInf + nBlk1del, ppInf + nBlk1del + nBlkdel,
                     ( nBlock - nBlkdel - nBlk1del ) * sizeof( BlockInfo* ) );

            // UpdateIdx updates the successor thus start before first elem
            if( !nBlk1 )
            {
                p = ppInf[ 0 ];
                p->nStart = 0;
                p->nEnd = p->nElem-1;
            }
            else
            {
                --nBlk1;
            }
        }
        BlockDel( nBlkdel ); // blocks were deleted
    }

    nSize -= n;
    if( nBlk1 != ( nBlock - 1 ) && nSize )
        UpdIndex( nBlk1 );
    nCur = nBlk1;

    // call Compress() if there is more than 50% space in the array
    if( nBlock > ( nSize / ( MAXENTRY / 2 ) ) )
        Compress();

    CHECKIDX( ppInf, nBlock, nSize, nCur );
}

void BigPtrArray::Replace( sal_uLong idx, const ElementPtr& rElem)
{
    assert(idx < nSize); // Index out of bounds
    nCur = Index2Block( idx );
    BlockInfo* p = ppInf[ nCur ];
    rElem->nOffset = sal_uInt16(idx - p->nStart);
    rElem->pBlock = p;
    p->pData[ idx - p->nStart ] = rElem;
}

/** Compress the array */
sal_uInt16 BigPtrArray::Compress( short nMax )
{
    CHECKIDX( ppInf, nBlock, nSize, nCur );

    // Iterate over InfoBlock array from beginning to end. If there is a deleted
    // block in between so move all following ones accordingly. The pointer <pp>
    // represents the "old" and <qq> the "new" array.
    BlockInfo** pp = ppInf, **qq = pp;
    BlockInfo* p;
    BlockInfo* pLast(nullptr);                 // last empty block
    sal_uInt16 nLast = 0;                // missing elements
    sal_uInt16 nBlkdel = 0;              // number of deleted blocks
    sal_uInt16 nFirstChgPos = USHRT_MAX; // at which position was the 1st change?

    // convert fill percentage into number of remaining elements
    nMax = MAXENTRY - (long) MAXENTRY * nMax / 100;

    for( sal_uInt16 cur = 0; cur < nBlock; ++cur )
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
            ElementPtr* pElem = pLast->pData + pLast->nElem;
            ElementPtr* pFrom = p->pData;
            for( sal_uInt16 nCount = n, nOff = pLast->nElem;
                            nCount; --nCount, ++pElem )
            {
                *pElem = *pFrom++;
                (*pElem)->pBlock = pLast;
                (*pElem)->nOffset = nOff++;
            }

            // adjustment
            pLast->nElem = pLast->nElem + n;
            nLast = nLast - n;
            p->nElem = p->nElem - n;

            // Is the current block now empty as a result?
            if( !p->nElem )
            {
                // than remove
                delete[] p->pData;
                delete   p;
                p = nullptr;
                ++nBlkdel;
            }
            else
            {
                pElem = p->pData;
                pFrom = pElem + n;
                int nCount = p->nElem;
                while( nCount-- )
                {
                    *pElem = *pFrom++;
                    (*pElem)->nOffset = (*pElem)->nOffset - n;
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
        BlockDel( nBlkdel );

    // and re-index
    p = ppInf[ 0 ];
    p->nEnd = p->nElem - 1;
    UpdIndex( 0 );

    if( nCur >= nFirstChgPos )
        nCur = 0;

    CHECKIDX( ppInf, nBlock, nSize, nCur );

    return nFirstChgPos;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
