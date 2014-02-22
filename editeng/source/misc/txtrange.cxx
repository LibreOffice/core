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


#include <editeng/txtrange.hxx>
#include <math.h>
#include <tools/poly.hxx>
#include <tools/debug.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

#include <vector>

TextRanger::TextRanger( const basegfx::B2DPolyPolygon& rPolyPolygon,
                        const basegfx::B2DPolyPolygon* pLinePolyPolygon,
                        sal_uInt16 nCacheSz, sal_uInt16 nLft, sal_uInt16 nRght,
                        sal_Bool bSimpl, sal_Bool bInnr, sal_Bool bVert ) :
    pBound( NULL ),
    nCacheSize( nCacheSz ),
    nRight( nRght ),
    nLeft( nLft ),
    nUpper( 0 ),
    nLower( 0 ),
    nPointCount( 0 ),
    bSimple( bSimpl ),
    bInner( bInnr ),
    bVertical( bVert )
{
    sal_uInt32 nCount(rPolyPolygon.count());
    mpPolyPolygon = new PolyPolygon( (sal_uInt16)nCount );

    for(sal_uInt32 i(0L); i < nCount; i++)
    {
        const basegfx::B2DPolygon aCandidate(rPolyPolygon.getB2DPolygon(i).getDefaultAdaptiveSubdivision());
        nPointCount += aCandidate.count();
        mpPolyPolygon->Insert( Polygon(aCandidate), (sal_uInt16)i );
    }

    if( pLinePolyPolygon )
    {
        nCount = pLinePolyPolygon->count();
        mpLinePolyPolygon = new PolyPolygon();

        for(sal_uInt32 i(0L); i < nCount; i++)
        {
            const basegfx::B2DPolygon aCandidate(pLinePolyPolygon->getB2DPolygon(i).getDefaultAdaptiveSubdivision());
            nPointCount += aCandidate.count();
            mpLinePolyPolygon->Insert( Polygon(aCandidate), (sal_uInt16)i );
        }
    }
    else
        mpLinePolyPolygon = NULL;
}


TextRanger::~TextRanger()
{
    mRangeCache.clear();
    delete mpPolyPolygon;
    delete mpLinePolyPolygon;
    delete pBound;
}

/* TextRanger::SetVertical(..)
   If there's is a change in the writing direction,
   the cache has to be cleared.
*/
void TextRanger::SetVertical( sal_Bool bNew )
{
    if( IsVertical() != bNew )
    {
        bVertical = bNew;
        mRangeCache.clear();
    }
}

//! SvxBoundArgs is used to perform temporary calculations on a range array.
//! Temporary instances are created in TextRanger::GetTextRanges()
class SvxBoundArgs
{
    std::vector<bool> aBoolArr;
    LongDqPtr pLongArr;
    TextRanger *pTextRanger;
    long nMin;
    long nMax;
    long nTop;
    long nBottom;
    long nUpDiff;
    long nLowDiff;
    long nUpper;
    long nLower;
    long nStart;
    long nEnd;
    sal_uInt16 nCut;
    sal_uInt16 nLast;
    sal_uInt16 nNext;
    sal_uInt8 nAct;
    sal_uInt8 nFirst;
    sal_Bool bClosed : 1;
    sal_Bool bInner : 1;
    sal_Bool bMultiple : 1;
    sal_Bool bConcat : 1;
    sal_Bool bRotate : 1;
    void NoteRange( bool bToggle );
    long Cut( long nY, const Point& rPt1, const Point& rPt2 );
    void Add();
    void _NoteFarPoint( long nPx, long nPyDiff, long nDiff );
    void NoteFarPoint( long nPx, long nPyDiff, long nDiff )
        { if( nDiff ) _NoteFarPoint( nPx, nPyDiff, nDiff ); }
    long CalcMax( const Point& rPt1, const Point& rPt2, long nRange, long nFar );
    void CheckCut( const Point& rLst, const Point& rNxt );
    inline long A( const Point& rP ) const { return bRotate ? rP.Y() : rP.X(); }
    inline long B( const Point& rP ) const { return bRotate ? rP.X() : rP.Y(); }
public:
    SvxBoundArgs( TextRanger* pRanger, LongDqPtr pLong, const Range& rRange );
    void NotePoint( const long nA ) { NoteMargin( nA - nStart, nA + nEnd ); }
    void NoteMargin( const long nL, const long nR )
        { if( nMin > nL ) nMin = nL; if( nMax < nR ) nMax = nR; }
    sal_uInt16 Area( const Point& rPt );
    void NoteUpLow( long nA, const sal_uInt8 nArea );
    void Calc( const PolyPolygon& rPoly );
    void Concat( const PolyPolygon* pPoly );
    
    void NoteLast() { if( bMultiple ) NoteRange( nAct == nFirst ); }
    void SetClosed( const sal_Bool bNew ){ bClosed = bNew; }
    sal_Bool IsClosed() const { return bClosed; }
    void SetConcat( const sal_Bool bNew ){ bConcat = bNew; }
    sal_Bool IsConcat() const { return bConcat; }
    sal_uInt8 GetAct() const { return nAct; }
};

SvxBoundArgs::SvxBoundArgs( TextRanger* pRanger, LongDqPtr pLong,
    const Range& rRange )
    : pLongArr( pLong ), pTextRanger( pRanger ),
    nTop( rRange.Min() ), nBottom( rRange.Max() ),
    bInner( pRanger->IsInner() ), bMultiple( bInner || !pRanger->IsSimple() ),
    bConcat( sal_False ), bRotate( pRanger->IsVertical() )
{
    if( bRotate )
    {
        nStart = pRanger->GetUpper();
        nEnd = pRanger->GetLower();
        nLowDiff = pRanger->GetLeft();
        nUpDiff = pRanger->GetRight();
    }
    else
    {
        nStart = pRanger->GetLeft();
        nEnd = pRanger->GetRight();
        nLowDiff = pRanger->GetUpper();
        nUpDiff = pRanger->GetLower();
    }
    nUpper = nTop - nUpDiff;
    nLower = nBottom + nLowDiff;
    pLongArr->clear();
}

long SvxBoundArgs::CalcMax( const Point& rPt1, const Point& rPt2,
    long nRange, long nFarRange )
{
    double nDa = Cut( nRange, rPt1, rPt2 ) - Cut( nFarRange, rPt1, rPt2 );
    double nB;
    if( nDa < 0 )
    {
        nDa = -nDa;
        nB = nEnd;
    }
    else
        nB = nStart;
    nB *= nB;
    nB += nDa * nDa;
    nB = nRange + nDa * ( nFarRange - nRange ) / sqrt( nB );

    sal_Bool bNote;
    if( nB < B(rPt2) )
        bNote = nB > B(rPt1);
    else
        bNote = nB < B(rPt1);
    if( bNote )
        return( long( nB ) );
    return 0;
}

void SvxBoundArgs::CheckCut( const Point& rLst, const Point& rNxt )
{
    if( nCut & 1 )
        NotePoint( Cut( nBottom, rLst, rNxt ) );
    if( nCut & 2 )
        NotePoint( Cut( nTop, rLst, rNxt ) );
    if( rLst.X() != rNxt.X() && rLst.Y() != rNxt.Y() )
    {
        long nYps;
        if( nLowDiff && ( ( nCut & 1 ) || nLast == 1 || nNext == 1 ) )
        {
            nYps = CalcMax( rLst, rNxt, nBottom, nLower );
            if( nYps )
                _NoteFarPoint( Cut( nYps, rLst, rNxt ), nLower-nYps, nLowDiff );
        }
        if( nUpDiff && ( ( nCut & 2 ) || nLast == 2 || nNext == 2 ) )
        {
            nYps = CalcMax( rLst, rNxt, nTop, nUpper );
            if( nYps )
                _NoteFarPoint( Cut( nYps, rLst, rNxt ), nYps-nUpper, nUpDiff );
        }
    }
}

void SvxBoundArgs::_NoteFarPoint( long nPa, long nPbDiff, long nDiff )
{
    long nTmpA;
    double nQuot = 2 * nDiff - nPbDiff;
    nQuot *= nPbDiff;
    nQuot = sqrt( nQuot );
    nQuot /= nDiff;
    nTmpA = nPa - long( nStart * nQuot );
    nPbDiff = nPa + long( nEnd * nQuot );
    NoteMargin( nTmpA, nPbDiff );
}

void SvxBoundArgs::NoteRange( bool bToggle )
{
    DBG_ASSERT( nMax >= nMin || bInner, "NoteRange: Min > Max?");
    if( nMax < nMin )
        return;
    if( !bClosed )
        bToggle = false;
    sal_uInt16 nIdx = 0;
    sal_uInt16 nCount = pLongArr->size();
    DBG_ASSERT( nCount == 2 * aBoolArr.size(), "NoteRange: Incompatible Sizes" );
    while( nIdx < nCount && (*pLongArr)[ nIdx ] < nMin )
        ++nIdx;
    sal_Bool bOdd = (nIdx % 2) ? sal_True : sal_False;
    
    if( nIdx == nCount || ( !bOdd && nMax < (*pLongArr)[ nIdx ] ) )
    {   
        pLongArr->insert( pLongArr->begin() + nIdx, nMin );
        pLongArr->insert( pLongArr->begin() + nIdx + 1, nMax );
        aBoolArr.insert( aBoolArr.begin() + (nIdx/2), bToggle );
    }
    else
    {   
        sal_uInt16 nMaxIdx = nIdx;
        
        if( bOdd )
            --nIdx;
        else
            (*pLongArr)[ nIdx ] = nMin;
        while( nMaxIdx < nCount && (*pLongArr)[ nMaxIdx ] < nMax )
            ++nMaxIdx;
        DBG_ASSERT( nMaxIdx > nIdx || nMin == nMax, "NoteRange: Funny Situation." );
        if( nMaxIdx )
            --nMaxIdx;
        if( nMaxIdx < nIdx )
            nMaxIdx = nIdx;
        
        if( nMaxIdx % 2 )
            (*pLongArr)[ nMaxIdx-- ] = nMax;
        
        sal_uInt16 nDiff = nMaxIdx - nIdx;
        nMaxIdx = nIdx / 2; 
        if( nDiff )
        {
            pLongArr->erase( pLongArr->begin() + nIdx + 1, pLongArr->begin() + nIdx + 1 + nDiff );
            nDiff /= 2;
            sal_uInt16 nStop = nMaxIdx + nDiff;
            for( sal_uInt16 i = nMaxIdx; i < nStop; ++i )
                bToggle ^= aBoolArr[ i ];
            aBoolArr.erase( aBoolArr.begin() + nMaxIdx, aBoolArr.begin() + (nMaxIdx + nDiff) );
        }
        DBG_ASSERT( nMaxIdx < aBoolArr.size(), "NoteRange: Too much deleted" );
        aBoolArr[ nMaxIdx ] = aBoolArr[ nMaxIdx ] != bToggle;
    }
}

void SvxBoundArgs::Calc( const PolyPolygon& rPoly )
{
    sal_uInt16 nCount;
    nAct = 0;
    for( sal_uInt16 i = 0; i < rPoly.Count(); ++i )
    {
        const Polygon& rPol = rPoly[ i ];
        nCount = rPol.GetSize();
        if( nCount )
        {
            const Point& rNull = rPol[ 0 ];
            SetClosed( IsConcat() || ( rNull == rPol[ nCount - 1 ] ) );
            nLast = Area( rNull );
            if( nLast & 12 )
            {
                nFirst = 3;
                if( bMultiple )
                    nAct = 0;
            }
            else
            {
                
                if( nLast )
                {
                    if( bMultiple || !nAct )
                    {
                        nMin = USHRT_MAX;
                        nMax = 0;
                    }
                    if( nLast & 1 )
                        NoteFarPoint( A(rNull), nLower - B(rNull), nLowDiff );
                    else
                        NoteFarPoint( A(rNull), B(rNull) - nUpper, nUpDiff );
                }
                else
                {
                    if( bMultiple || !nAct )
                    {
                        nMin = A(rNull);
                        nMax = nMin + nEnd;
                        nMin -= nStart;
                    }
                    else
                        NotePoint( A(rNull) );
                }
                nFirst = 0; 
                nAct = 3;   
            }
            if( nCount > 1 )
            {
                sal_uInt16 nIdx = 1;
                while( true )
                {
                    const Point& rLast = rPol[ nIdx - 1 ];
                    if( nIdx == nCount )
                        nIdx = 0;
                    const Point& rNext = rPol[ nIdx ];
                    nNext = Area( rNext );
                    nCut = nNext ^ nLast;
                    sal_uInt16 nOldAct = nAct;
                    if( nAct )
                        CheckCut( rLast, rNext );
                    if( nCut & 4 )
                    {
                        NoteUpLow( Cut( nLower, rLast, rNext ), 2 );
                        if( nAct && nAct != nOldAct )
                        {
                            nOldAct = nAct;
                            CheckCut( rLast, rNext );
                        }
                    }
                    if( nCut & 8 )
                    {
                        NoteUpLow( Cut( nUpper, rLast, rNext ), 1 );
                        if( nAct && nAct != nOldAct )
                            CheckCut( rLast, rNext );
                    }
                    if( !nIdx )
                    {
                        if( !( nNext & 12 ) )
                            NoteLast();
                        break;
                    }
                    if( !( nNext & 12 ) )
                    {
                        if( !nNext )
                            NotePoint( A(rNext) );
                        else if( nNext & 1 )
                            NoteFarPoint( A(rNext), nLower-B(rNext), nLowDiff );
                        else
                            NoteFarPoint( A(rNext), B(rNext)-nUpper, nUpDiff );
                    }
                    nLast = nNext;
                    if( ++nIdx == nCount && !IsClosed() )
                    {
                        if( !( nNext & 12 ) )
                            NoteLast();
                        break;
                    }
                }
            }
            if( bMultiple && IsConcat() )
            {
                Add();
                nAct = 0;
            }
        }
    }
    if( !bMultiple )
    {
        DBG_ASSERT( pLongArr->empty(), "I said: Simple!" );
        if( nAct )
        {
            if( bInner )
            {
                long nTmpMin = nMin + 2 * nStart;
                long nTmpMax = nMax - 2 * nEnd;
                if( nTmpMin <= nTmpMax )
                {
                    pLongArr->push_front(nTmpMax);
                    pLongArr->push_front(nTmpMin);
                }
            }
            else
            {
                pLongArr->push_front(nMax);
                pLongArr->push_front(nMin);
            }
        }
    }
    else if( !IsConcat() )
        Add();
}

void SvxBoundArgs::Add()
{
    size_t nCount = aBoolArr.size();
    if( nCount && ( !bInner || !pTextRanger->IsSimple() ) )
    {
        sal_Bool bDelete = aBoolArr.front();
        if( bInner )
            bDelete = !bDelete;
        sal_uInt16 nLongIdx = 1;
        for( size_t nBoolIdx = 1; nBoolIdx < nCount; ++nBoolIdx )
        {
            if( bDelete )
            {
                sal_uInt16 next = 2;
                while( nBoolIdx < nCount && !aBoolArr[ nBoolIdx++ ] &&
                       (!bInner || nBoolIdx < nCount ) )
                    next += 2;
                pLongArr->erase( pLongArr->begin() + nLongIdx, pLongArr->begin() + nLongIdx + next );
                next /= 2;
                nBoolIdx = nBoolIdx - next;
                nCount = nCount - next;
                aBoolArr.erase( aBoolArr.begin() + nBoolIdx, aBoolArr.begin() + (nBoolIdx + next) );
                if( nBoolIdx )
                    aBoolArr[ nBoolIdx - 1 ] = false;
#if OSL_DEBUG_LEVEL > 1
                else
                    ++next;
#endif
            }
            bDelete = nBoolIdx < nCount && aBoolArr[ nBoolIdx ];
            nLongIdx += 2;
            DBG_ASSERT( nLongIdx == 2*nBoolIdx+1, "BoundArgs: Array-Idx Confusion" );
            DBG_ASSERT( aBoolArr.size()*2 == pLongArr->size(),
                        "BoundArgs: Array-Count: Confusion" );
        }
    }
    if( !pLongArr->empty() )
    {
        if( bInner )
        {
            pLongArr->pop_front();
            pLongArr->pop_back();

            
            
            
            
            if( pTextRanger->IsSimple() && pLongArr->size() > 2 )
                pLongArr->erase( pLongArr->begin() + 1, pLongArr->end() - 1 );

        }
    }
}

void SvxBoundArgs::Concat( const PolyPolygon* pPoly )
{
    SetConcat( sal_True );
    DBG_ASSERT( pPoly, "Nothing to do?" );
    LongDqPtr pOld = pLongArr;
    pLongArr = new std::deque<long>();
    aBoolArr.clear();
    bInner = sal_False;
    Calc( *pPoly ); 
    sal_uInt16 nCount = pLongArr->size();
    sal_uInt16 nIdx = 0;
    sal_uInt16 i = 0;
    sal_Bool bSubtract = pTextRanger->IsInner();
    while( i < nCount )
    {
        sal_uLong nOldCount = pOld->size();
        if( nIdx == nOldCount )
        {   
            if( !bSubtract )
                pOld->insert( pOld->begin() + nIdx, pLongArr->begin() + i, pLongArr->end() );
            break;
        }
        long nLeft = (*pLongArr)[ i++ ];
        long nRight = (*pLongArr)[ i++ ];
        sal_uInt16 nLeftPos = nIdx + 1;
        while( nLeftPos < nOldCount && nLeft > (*pOld)[ nLeftPos ] )
            nLeftPos += 2;
        if( nLeftPos >= nOldCount )
        {   
            if( !bSubtract )
                pOld->insert( pOld->begin() + nOldCount, pLongArr->begin() + i - 2, pLongArr->end() );
            break;
        }
        sal_uInt16 nRightPos = nLeftPos - 1;
        while( nRightPos < nOldCount && nRight >= (*pOld)[ nRightPos ] )
            nRightPos += 2;
        if( nRightPos < nLeftPos )
        {   
            if( !bSubtract )
                pOld->insert( pOld->begin() + nRightPos, pLongArr->begin() + i - 2, pLongArr->begin() + i );
            nIdx = nRightPos + 2;
        }
        else if( bSubtract ) 
        {
            long nOld;
            if( nLeft > ( nOld = (*pOld)[ nLeftPos - 1 ] ) )
            {   
                if( nLeft - 1 > nOld )
                {
                    pOld->insert( pOld->begin() + nLeftPos - 1, nOld );
                    pOld->insert( pOld->begin() + nLeftPos, nLeft - 1 );
                    nLeftPos += 2;
                    nRightPos += 2;
                }
            }
            if( nRightPos - nLeftPos > 1 )
                pOld->erase( pOld->begin() + nLeftPos, pOld->begin() + nRightPos - 1 );
            if( ++nRight >= ( nOld = (*pOld)[ nLeftPos ] ) )
                pOld->erase( pOld->begin() + nLeftPos - 1, pOld->begin() + nLeftPos + 1 );
            else
                (*pOld)[ nLeftPos - 1 ] = nRight;
        }
        else 
        {
            if( nLeft < (*pOld)[ nLeftPos - 1 ] )
                (*pOld)[ nLeftPos - 1 ] = nLeft;
            if( nRight > (*pOld)[ nRightPos - 1 ] )
                (*pOld)[ nRightPos - 1 ] = nRight;
            if( nRightPos - nLeftPos > 1 )
                pOld->erase( pOld->begin() + nLeftPos, pOld->begin() + nRightPos - 1 );

        }
        nIdx = nLeftPos - 1;
    }
    delete pLongArr;
}

/*************************************************************************
 * SvxBoundArgs::Area returns the area in which the point is located.
 * 0 = within the line
 * 1 = below, but within the upper edge
 * 2 = above, but within the lower edge
 * 5 = below the upper edge
 *10 = above the lower edge
 *************************************************************************/

sal_uInt16 SvxBoundArgs::Area( const Point& rPt )
{
    long nB = B( rPt );
    if( nB >= nBottom )
    {
        if( nB >= nLower )
            return 5;
        return 1;
    }
    if( nB <= nTop )
    {
        if( nB <= nUpper )
            return 10;
        return 2;
    }
    return 0;
}

/*************************************************************************
 * lcl_Cut calculates the X-Coordinate of the distance (Pt1-Pt2) at the
 * Y-Coordinate nY.
 * It is assumed that the one of the points are located above and the other
 * one below the Y-Coordinate.
 *************************************************************************/

long SvxBoundArgs::Cut( long nB, const Point& rPt1, const Point& rPt2 )
{
    if( pTextRanger->IsVertical() )
    {
        double nQuot = nB - rPt1.X();
        nQuot /= ( rPt2.X() - rPt1.X() );
        nQuot *= ( rPt2.Y() - rPt1.Y() );
        return long( rPt1.Y() + nQuot );
    }
    double nQuot = nB - rPt1.Y();
    nQuot /= ( rPt2.Y() - rPt1.Y() );
    nQuot *= ( rPt2.X() - rPt1.X() );
    return long( rPt1.X() + nQuot );
}

void SvxBoundArgs::NoteUpLow( long nA, const sal_uInt8 nArea )
{
    if( nAct )
    {
        NoteMargin( nA, nA );
        if( bMultiple )
        {
            NoteRange( nArea != nAct );
            nAct = 0;
        }
        if( !nFirst )
            nFirst = nArea;
    }
    else
    {
        nAct = nArea;
        nMin = nA;
        nMax = nA;
    }
}

LongDqPtr TextRanger::GetTextRanges( const Range& rRange )
{
    DBG_ASSERT( rRange.Min() || rRange.Max(), "Zero-Range not allowed, Bye Bye" );
    //Can we find the result we need in the cache?
    for (std::deque<RangeCache>::iterator it = mRangeCache.begin(); it != mRangeCache.end(); ++it)
    {
        if (it->range == rRange)
            return &(it->results);
    }
    //Calculate a new result
    RangeCache rngCache(rRange);
    SvxBoundArgs aArg( this, &(rngCache.results), rRange );
    aArg.Calc( *mpPolyPolygon );
    if( mpLinePolyPolygon )
        aArg.Concat( mpLinePolyPolygon );
    //Add new result to the cache
    mRangeCache.push_back(rngCache);
    if (mRangeCache.size() > nCacheSize)
        mRangeCache.pop_front();
    return &(mRangeCache.back().results);
}

const Rectangle& TextRanger::_GetBoundRect()
{
    DBG_ASSERT( 0 == pBound, "Don't call twice." );
    pBound = new Rectangle( mpPolyPolygon->GetBoundRect() );
    return *pBound;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
