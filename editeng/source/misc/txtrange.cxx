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
#include "precompiled_editeng.hxx"

#include <editeng/txtrange.hxx>
#include <math.h>
#include <tools/poly.hxx>
#include <tools/debug.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

/*************************************************************************
|*
|*    TextRanger::TextRanger()
|*
|*    Beschreibung
|*    Ersterstellung    20.01.97
|*    Letzte Aenderung  20.01.97 AMA
|*
*************************************************************************/

TextRanger::TextRanger( const basegfx::B2DPolyPolygon& rPolyPolygon, const basegfx::B2DPolyPolygon* pLinePolyPolygon,
    sal_uInt16 nCacheSz, sal_uInt16 nLft, sal_uInt16 nRght, sal_Bool bSimpl, sal_Bool bInnr,
    sal_Bool bVert ) :
    pBound( NULL ),
    nCacheSize( nCacheSz ),
    nCacheIdx( 0 ),
    nRight( nRght ),
    nLeft( nLft ),
    nUpper( 0 ),
    nLower( 0 ),
    nPointCount( 0 ),
    bSimple( bSimpl ),
    bInner( bInnr ),
    bVertical( bVert )
{
#ifdef DBG_UTIL
    bFlag3 = bFlag4 = bFlag5 = bFlag6 = bFlag7 = sal_False;
#endif
    pRangeArr = new Range[ nCacheSize ];
    pCache = new SvLongsPtr[ nCacheSize ];
    memset( pRangeArr, 0, nCacheSize * sizeof( Range ) );
    memset( pCache, 0, nCacheSize * sizeof( SvLongsPtr ) );
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

/*************************************************************************
|*
|*    TextRanger::~TextRanger()
|*
|*    Beschreibung
|*    Ersterstellung    20.01.97
|*    Letzte Aenderung  20.01.97 AMA
|*
*************************************************************************/

TextRanger::~TextRanger()
{
    for( sal_uInt16 i = 0; i < nCacheSize; ++i )
        delete pCache[i];
    delete[] pCache;
    delete[] pRangeArr;
    delete mpPolyPolygon;
    delete mpLinePolyPolygon;
}

/*-----------------17.11.00 09:49-------------------
 * TextRanger::SetVertical(..)
 * If there's is a change in the writing direction,
 * the cache has to be cleared.
 * --------------------------------------------------*/

void TextRanger::SetVertical( sal_Bool bNew )
{
    if( IsVertical() != bNew )
    {
        bVertical = bNew;
        for( sal_uInt16 i = 0; i < nCacheSize; ++i )
            delete pCache[i];
        memset( pRangeArr, 0, nCacheSize * sizeof( Range ) );
        memset( pCache, 0, nCacheSize * sizeof( SvLongsPtr ) );
    }
}

/*************************************************************************
|*
|*    SvxBoundArgs
|*
|*    Beschreibung
|*    Ersterstellung    20.01.97
|*    Letzte Aenderung  20.01.97 AMA
|*
*************************************************************************/

class SvxBoundArgs
{
    SvBools aBoolArr;
    SvLongs *pLongArr;
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
    void NoteRange( sal_Bool bToggle );
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
    SvxBoundArgs( TextRanger* pRanger, SvLongs *pLong, const Range& rRange );
    void NotePoint( const long nA ) { NoteMargin( nA - nStart, nA + nEnd ); }
    void NoteMargin( const long nL, const long nR )
        { if( nMin > nL ) nMin = nL; if( nMax < nR ) nMax = nR; }
    sal_uInt16 Area( const Point& rPt );
    void NoteUpLow( long nA, const sal_uInt8 nArea );
    void Calc( const PolyPolygon& rPoly );
    void Concat( const PolyPolygon* pPoly );
    // inlines
    void NoteLast() { if( bMultiple ) NoteRange( nAct == nFirst ); }
    void SetClosed( const sal_Bool bNew ){ bClosed = bNew; }
    sal_Bool IsClosed() const { return bClosed; }
    void SetConcat( const sal_Bool bNew ){ bConcat = bNew; }
    sal_Bool IsConcat() const { return bConcat; }
    sal_uInt8 GetAct() const { return nAct; }
};

SvxBoundArgs::SvxBoundArgs( TextRanger* pRanger, SvLongs *pLong,
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
    pLongArr->Remove( 0, pLongArr->Count() );
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

void SvxBoundArgs::NoteRange( sal_Bool bToggle )
{
    DBG_ASSERT( nMax >= nMin || bInner, "NoteRange: Min > Max?");
    if( nMax < nMin )
        return;
    if( !bClosed )
        bToggle = sal_False;
    sal_uInt16 nIdx = 0;
    sal_uInt16 nCount = pLongArr->Count();
    DBG_ASSERT( nCount == 2 * aBoolArr.size(), "NoteRange: Incompatible Sizes" );
    while( nIdx < nCount && (*pLongArr)[ nIdx ] < nMin )
        ++nIdx;
    sal_Bool bOdd = nIdx % 2 ? sal_True : sal_False;
    // Kein Ueberlappung mit vorhandenen Intervallen?
    if( nIdx == nCount || ( !bOdd && nMax < (*pLongArr)[ nIdx ] ) )
    {   // Dann wird ein neues eingefuegt ...
        pLongArr->Insert( nMin, nIdx );
        pLongArr->Insert( nMax, nIdx + 1 );
        aBoolArr.insert( aBoolArr.begin() + nIdx / 2, bToggle );
    }
    else
    {   // ein vorhandes Intervall erweitern ...
        sal_uInt16 nMaxIdx = nIdx;
        // Wenn wir auf einer linken Intervallgrenze gelandet sind, muss diese
        // auf nMin gesenkt werden.
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
        // Wenn wir auf einer rechten Intervallgrenze landen, muss diese
        // auf nMax angehoben werden.
        if( nMaxIdx % 2 )
            (*pLongArr)[ nMaxIdx-- ] = nMax;
        // Jetzt werden eventuell noch Intervalle verschmolzen
        sal_uInt16 nDiff = nMaxIdx - nIdx;
        nMaxIdx = nIdx / 2; // Ab hier ist nMaxIdx der Index im BoolArray.
        if( nDiff )
        {
            (*pLongArr).Remove( nIdx + 1, nDiff );
            nDiff /= 2;
            sal_uInt16 nStop = nMaxIdx + nDiff;
            for( sal_uInt16 i = nMaxIdx; i < nStop; ++i )
                bToggle ^= aBoolArr[ i ];
            aBoolArr.erase( aBoolArr.begin() + nMaxIdx, aBoolArr.begin() + (nMaxIdx + nDiff) );
        }
        DBG_ASSERT( nMaxIdx < aBoolArr.size(), "NoteRange: Too much deleted" );
        aBoolArr[ nMaxIdx ] ^= bToggle;
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
                // Der erste Punkt des Polygons liegt innerhalb der Zeile.
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
                nFirst = 0; // In welcher Richtung wird die Zeile verlassen?
                nAct = 3;   // Wir sind z.Z. innerhalb der Zeile.
            }
            if( nCount > 1 )
            {
                sal_uInt16 nIdx = 1;
                while( sal_True )
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
        DBG_ASSERT( pLongArr->Count() == 0, "I said: Simple!" );
        if( nAct )
        {
            if( bInner )
            {
                long nTmpMin, nTmpMax;
                {
                    nTmpMin = nMin + 2 * nStart;
                    nTmpMax = nMax - 2 * nEnd;
                    if( nTmpMin <= nTmpMax )
                    {
                        pLongArr->Insert( nTmpMin, 0 );
                        pLongArr->Insert( nTmpMax, 1 );
                    }
                }
            }
            else
            {
                pLongArr->Insert( nMin, 0 );
                pLongArr->Insert( nMax, 1 );
            }
        }
    }
    else if( !IsConcat() )
        Add();
}

void SvxBoundArgs::Add()
{
    sal_uInt16 nLongIdx = 1;
    size_t nCount = aBoolArr.size();
    if( nCount && ( !bInner || !pTextRanger->IsSimple() ) )
    {
        sal_Bool bDelete = aBoolArr.front();
        if( bInner )
            bDelete = !bDelete;
        for( size_t nBoolIdx = 1; nBoolIdx < nCount; ++nBoolIdx )
        {
            if( bDelete )
            {
                sal_uInt16 next = 2;
                while( nBoolIdx < nCount && !aBoolArr[ nBoolIdx++ ] &&
                       (!bInner || nBoolIdx < nCount ) )
                    next += 2;
                pLongArr->Remove( nLongIdx, next );
                next /= 2;
                nBoolIdx = nBoolIdx - next;
                nCount = nCount - next;
                aBoolArr.erase( aBoolArr.begin() + nBoolIdx, aBoolArr.begin() + (nBoolIdx + next) );
                if( nBoolIdx )
                    aBoolArr[ nBoolIdx - 1 ] = sal_False;
#if OSL_DEBUG_LEVEL > 1
                else
                    ++next;
#endif
            }
            bDelete = nBoolIdx < nCount && aBoolArr[ nBoolIdx ];
            nLongIdx += 2;
            DBG_ASSERT( nLongIdx == 2*nBoolIdx+1, "BoundArgs: Array-Idx Confusion" );
            DBG_ASSERT( aBoolArr.size()*2 == pLongArr->Count(),
                        "BoundArgs: Array-Count: Confusion" );
        }
    }
    if( 0 != ( nCount = pLongArr->Count() ) )
    {
        if( bInner )
        {
            pLongArr->Remove( 0, 1 );
            pLongArr->Remove( pLongArr->Count() - 1, 1 );

            // Hier wird die Zeile beim "einfachen" Konturumfluss im Innern
            // in ein grosses Rechteck zusammengefasst.
            // Zur Zeit (April 1999) wertet die EditEngine nur das erste Rechteck
            // aus, falls sie eines Tages in der Lage ist, eine Zeile in mehreren
            // Teilen auszugeben, kann es sinnvoll sein, die folgenden Zeilen
            // zu loeschen.
            if( pTextRanger->IsSimple() && pLongArr->Count() > 2 )
                pLongArr->Remove( 1, pLongArr->Count() - 2 );

        }
    }
}

void SvxBoundArgs::Concat( const PolyPolygon* pPoly )
{
    SetConcat( sal_True );
    DBG_ASSERT( pPoly, "Nothing to do?" );
    SvLongs *pOld = pLongArr;
    pLongArr = new SvLongs( 2, 8 );
    aBoolArr.clear();
    bInner = sal_False;
    Calc( *pPoly );
    sal_uInt16 nCount = pLongArr->Count();
    sal_uInt16 nIdx = 0;
    sal_uInt16 i = 0;
    sal_Bool bSubtract = pTextRanger->IsInner();
    while( i < nCount )
    {
        sal_uInt16 nOldCount = pOld->Count();
        if( nIdx == nOldCount )
        {   // Am Ende des alten Arrays angelangt...
            if( !bSubtract )
                pOld->Insert( pLongArr, nIdx, i, USHRT_MAX );
            break;
        }
        long nLeft = (*pLongArr)[ i++ ];
        long nRight = (*pLongArr)[ i++ ];
        sal_uInt16 nLeftPos = nIdx + 1;
        while( nLeftPos < nOldCount && nLeft > (*pOld)[ nLeftPos ] )
            nLeftPos += 2;
        if( nLeftPos >= nOldCount )
        {   // Das aktuelle Intervall gehoert ans Ende des alten Arrays...
            if( !bSubtract )
                pOld->Insert( pLongArr, nOldCount, i - 2, USHRT_MAX );
            break;
        }
        sal_uInt16 nRightPos = nLeftPos - 1;
        while( nRightPos < nOldCount && nRight >= (*pOld)[ nRightPos ] )
            nRightPos += 2;
        if( nRightPos < nLeftPos )
        {   // Das aktuelle Intervall gehoert zwischen zwei alte Intervalle
            if( !bSubtract )
                pOld->Insert( pLongArr, nRightPos, i - 2, i );
            nIdx = nRightPos + 2;
        }
        else if( bSubtract ) // Subtrahieren ggf. Trennen
        {
            long nOld;
            if( nLeft > ( nOld = (*pOld)[ nLeftPos - 1 ] ) )
            {   // Jetzt spalten wir den linken Teil ab...
                if( nLeft - 1 > nOld )
                {
                    pOld->Insert( nOld, nLeftPos - 1 );
                    pOld->Insert( nLeft - 1, nLeftPos );
                    nLeftPos += 2;
                    nRightPos += 2;
                }
            }
            if( nRightPos - nLeftPos > 1 )
                pOld->Remove( nLeftPos, nRightPos - nLeftPos - 1 );
            if( ++nRight >= ( nOld = (*pOld)[ nLeftPos ] ) )
                pOld->Remove( nLeftPos - 1, 2 );
            else
                (*pOld)[ nLeftPos - 1 ] = nRight;
        }
        else // Verschmelzen
        {
            if( nLeft < (*pOld)[ nLeftPos - 1 ] )
                (*pOld)[ nLeftPos - 1 ] = nLeft;
            if( nRight > (*pOld)[ nRightPos - 1 ] )
                (*pOld)[ nRightPos - 1 ] = nRight;
            if( nRightPos - nLeftPos > 1 )
                pOld->Remove( nLeftPos, nRightPos - nLeftPos - 1 );

        }
        nIdx = nLeftPos - 1;
    }
    delete pLongArr;
}

/*************************************************************************
 * SvxBoundArgs::Area ermittelt den Bereich, in dem sich der Punkt befindet
 * 0 = innerhalb der Zeile
 * 1 = unterhalb, aber innerhalb der oberen Randes
 * 2 = oberhalb, aber innerhalb der unteren Randes
 * 5 = unterhalb des oberen Randes
 *10 = oberhalb des unteren Randes
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
 * lcl_Cut berechnet die X-Koordinate der Strecke (Pt1-Pt2) auf der
 * Y-Koordinate nY.
 * Vorausgesetzt wird, dass einer der Punkte oberhalb und der andere
 * unterhalb der Y-Koordinate liegt.
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

SvLongsPtr TextRanger::GetTextRanges( const Range& rRange )
{
    DBG_ASSERT( rRange.Min() || rRange.Max(), "Zero-Range not allowed, Bye Bye" );
    sal_uInt16 nIndex = 0;
    while( nIndex < nCacheSize && rRange != pRangeArr[ nIndex ] )
        ++nIndex;
    if( nIndex >= nCacheSize )
    {
        ++nCacheIdx;
        nCacheIdx %= nCacheSize;
        pRangeArr[ nCacheIdx ] = rRange;
        if( !pCache[ nCacheIdx ] )
            pCache[ nCacheIdx ] = new SvLongs( 2, 8 );
        nIndex = nCacheIdx;
        SvxBoundArgs aArg( this, pCache[ nCacheIdx ], rRange );
        aArg.Calc( *mpPolyPolygon );
        if( mpLinePolyPolygon )
            aArg.Concat( mpLinePolyPolygon );
    }
    return pCache[ nIndex ];
}

const Rectangle& TextRanger::_GetBoundRect()
{
    DBG_ASSERT( 0 == pBound, "Don't call twice." );
    pBound = new Rectangle( mpPolyPolygon->GetBoundRect() );
    return *pBound;
}


