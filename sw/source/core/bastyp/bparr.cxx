/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "bparr.hxx"

#include <limits.h>
#include <string.h>

/** Resize block management by this constant.
    As a result there are approx. 20 * MAXENTRY == 20000 entries available */
const sal_uInt16 nBlockGrowSize = 20;

#if OSL_DEBUG_LEVEL > 2
#define CHECKIDX( p, n, i, c ) CheckIdx( p, n, i, c );
void CheckIdx( BlockInfo** ppInf, sal_uInt16 nBlock, sal_uLong nSize, sal_uInt16 nCur )
{
    assert( !nSize || nCur < nBlock ); 

    sal_uLong nIdx = 0;
    for( sal_uInt16 nCnt = 0; nCnt < nBlock; ++nCnt, ++ppInf )
    {
        nIdx += (*ppInf)->nElem;
        
        assert( !nCnt || (*(ppInf-1))->nEnd + 1 == (*ppInf)->nStart );
    }
    assert(nIdx == nSize); 
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



void BigPtrArray::Move( sal_uLong from, sal_uLong to )
{
    sal_uInt16 cur = Index2Block( from );
    BlockInfo* p = ppInf[ cur ];
    ElementPtr pElem = p->pData[ from - p->nStart ];
    Insert( pElem, to ); 
    Remove( ( to < from ) ? ( from + 1 ) : from );
}

/** Apply function to every element.

    @param nStart First element to start with
    @param nEnd   Last element (exclusive!)
    @param fn     Function
    @param pArgs  Additional arguments for <fn>
*/
void BigPtrArray::ForEach( sal_uLong nStart, sal_uLong nEnd,
                            FnForEach fn, void* pArgs )
{
    if( nEnd > nSize )
        nEnd = nSize;

    if( nStart < nEnd )
    {
        sal_uInt16 cur = Index2Block( nStart );
        BlockInfo** pp = ppInf + cur;
        BlockInfo* p = *pp;
        sal_uInt16 nElem = sal_uInt16( nStart - p->nStart );
        ElementPtr* pElem = p->pData + nElem;
        nElem = p->nElem - nElem;
        for(;;)
        {
            if( !(*fn)( *pElem++, pArgs ) || ++nStart >= nEnd )
                break;

            
            if( !--nElem )
            {
                
                p = *++pp;
                pElem = p->pData;
                nElem = p->nElem;
            }
        }
    }
}

ElementPtr BigPtrArray::operator[]( sal_uLong idx ) const
{
    assert(idx < nSize); 
    nCur = Index2Block( idx );
    BlockInfo* p = ppInf[ nCur ];
    return p->pData[ idx - p->nStart ];
}

/** Search a block at a given position */
sal_uInt16 BigPtrArray::Index2Block( sal_uLong pos ) const
{
    
    BlockInfo* p = ppInf[ nCur ];
    if( p->nStart <= pos && p->nEnd >= pos )
        return nCur;
    
    if( !pos )
        return 0;

    
    if( nCur < ( nBlock - 1 ) )
    {
        p = ppInf[ nCur+1 ];
        if( p->nStart <= pos && p->nEnd >= pos )
            return nCur+1;
    }
    
    else if( pos < p->nStart && nCur > 0 )
    {
        p = ppInf[ nCur-1 ];
        if( p->nStart <= pos && p->nEnd >= pos )
            return nCur-1;
    }

    
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
    BlockInfo* p;
    while( ++pos < nBlock )
    {
        p = *++pp;
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

    p->nEnd--;  
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
        
        p = InsBlock( cur = 0 );
    }
    else if( pos == nSize )
    {
        
        cur = nBlock - 1;
        p = ppInf[ cur ];
        if( p->nElem == MAXENTRY )
            
            p = InsBlock( ++cur );
    }
    else
    {
        
        cur = Index2Block( pos );
        p = ppInf[ cur ];
    }

    if( p->nElem == MAXENTRY )
    {
        
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
            
            
            if( /*nBlock == nMaxBlock &&*/
                nBlock > ( nSize / ( MAXENTRY / 2 ) ) &&
                cur >= Compress() )
            {
                
                
                Insert( rElem, pos );
                return ;
            }

            q = InsBlock( cur+1 );
        }

        
        ElementPtr pLast = p->pData[ MAXENTRY-1 ];
        pLast->nOffset = 0;
        pLast->pBlock = q;

        q->pData[ 0 ] = pLast;
        q->nElem++;
        q->nEnd++;

        p->nEnd--;
        p->nElem--;
    }
    
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

    sal_uInt16 nBlkdel = 0;              
    sal_uInt16 cur = Index2Block( pos ); 
    sal_uInt16 nBlk1 = cur;              
    sal_uInt16 nBlk1del = USHRT_MAX;     
    BlockInfo* p = ppInf[ cur ];
    pos -= p->nStart;

    sal_uLong nElem = n;
    while( nElem )
    {
        sal_uInt16 nel = p->nElem - sal_uInt16(pos);
        if( sal_uLong(nel) > nElem )
            nel = sal_uInt16(nElem);
        
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

    
    if( nBlkdel )
    {
        for( sal_uInt16 i = nBlk1del; i < ( nBlk1del + nBlkdel ); i++ )
            delete ppInf[ i ];

        if( ( nBlk1del + nBlkdel ) < nBlock )
        {
            memmove( ppInf + nBlk1del, ppInf + nBlk1del + nBlkdel,
                     ( nBlock - nBlkdel - nBlk1del ) * sizeof( BlockInfo* ) );

            
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
        BlockDel( nBlkdel ); 
    }

    nSize -= n;
    if( nBlk1 != ( nBlock - 1 ) && nSize )
        UpdIndex( nBlk1 );
    nCur = nBlk1;

    
    if( nBlock > ( nSize / ( MAXENTRY / 2 ) ) )
        Compress();

    CHECKIDX( ppInf, nBlock, nSize, nCur );
}

void BigPtrArray::Replace( sal_uLong idx, const ElementPtr& rElem)
{
    assert(idx < nSize); 
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

    
    
    
    BlockInfo** pp = ppInf, **qq = pp;
    BlockInfo* p;
    BlockInfo* pLast(0);                 
    sal_uInt16 nLast = 0;                
    sal_uInt16 nBlkdel = 0;              
    sal_uInt16 nFirstChgPos = USHRT_MAX; 

    
    nMax = MAXENTRY - (long) MAXENTRY * nMax / 100;

    for( sal_uInt16 cur = 0; cur < nBlock; ++cur )
    {
        p = *pp++;
        sal_uInt16 n = p->nElem;
        
        
        
        
        if( nLast && ( n > nLast ) && ( nLast < nMax ) )
            nLast = 0;
        if( nLast )
        {
            if( USHRT_MAX == nFirstChgPos )
                nFirstChgPos = cur;

            
            if( n > nLast )
                n = nLast;

            
            ElementPtr* pElem = pLast->pData + pLast->nElem;
            ElementPtr* pFrom = p->pData;
            for( sal_uInt16 nCount = n, nOff = pLast->nElem;
                            nCount; --nCount, ++pElem )
            {
                *pElem = *pFrom++,
                    (*pElem)->pBlock = pLast,
                    (*pElem)->nOffset = nOff++;
            }

            
            pLast->nElem = pLast->nElem + n;
            nLast = nLast - n;
            p->nElem = p->nElem - n;

            
            if( !p->nElem )
            {
                
                delete[] p->pData;
                delete   p, p = 0;
                ++nBlkdel;
            }
            else
            {
                pElem = p->pData, pFrom = pElem + n;
                int nCount = p->nElem;
                while( nCount-- )
                {
                    *pElem = *pFrom++;
                    (*pElem)->nOffset = (*pElem)->nOffset - n;
                    ++pElem;
                }
            }
        }

        if( p ) 
        {
            *qq++ = p; 

            
            if( !nLast && p->nElem < MAXENTRY )
            {
                pLast = p;
                nLast = MAXENTRY - p->nElem;
            }
        }
    }

    
    if( nBlkdel )
        BlockDel( nBlkdel );

    
    p = ppInf[ 0 ];
    p->nEnd = p->nElem - 1;
    UpdIndex( 0 );

    if( nCur >= nFirstChgPos )
        nCur = 0;

    CHECKIDX( ppInf, nBlock, nSize, nCur );

    return nFirstChgPos;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
