/*************************************************************************
 *
 *  $RCSfile: txtrange.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

#ifndef _XOUTX_HXX
#include <xoutx.hxx>
#endif

#include "txtrange.hxx"
#include <math.h>

#ifndef _POLY_HXX //autogen
#include <vcl/poly.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif

/*************************************************************************
|*
|*    TextRanger::TextRanger()
|*
|*    Beschreibung
|*    Ersterstellung    20.01.97
|*    Letzte Aenderung  20.01.97 AMA
|*
*************************************************************************/

#ifdef WIN
#pragma optimize ( "", off )
#endif

TextRanger::TextRanger( const XPolyPolygon& rXPoly, const XPolyPolygon* pXLine,
    USHORT nCacheSz, USHORT nLft, USHORT nRght, BOOL bSimpl, BOOL bInnr ) :
    pBound( NULL ), nCacheSize( nCacheSz ), nCacheIdx( 0 ), nPointCount( 0 ),
    nLeft( nLft ), nRight( nRght ), nUpper( 0 ), nLower( 0 ),
    bSimple( bSimpl ), bInner( bInnr )
{
#ifndef PRODUCT
    bFlag2 = bFlag3 = bFlag4 = bFlag5 = bFlag6 = bFlag7 = FALSE;
#endif
    pRangeArr = new Range[ nCacheSize ];
    pCache = new SvLongsPtr[ nCacheSize ];
    memset( pRangeArr, 0, nCacheSize * sizeof( Range ) );
    memset( pCache, 0, nCacheSize * sizeof( SvLongsPtr ) );
    USHORT nCount = rXPoly.Count();
    pPoly = new PolyPolygon( nCount );
    for( USHORT i = 0; i < nCount; ++i )
    {
        Polygon aTmp = XOutCreatePolygon( rXPoly[ i ], NULL, 100 );
        nPointCount += aTmp.GetSize();
        pPoly->Insert( aTmp, i );
    }
    if( pXLine )
    {
        nCount = pXLine->Count();
        pLine = new PolyPolygon();
        for( USHORT i = 0; i < nCount; ++i )
        {
            Polygon aTmp = XOutCreatePolygon( (*pXLine)[ i ], NULL, 100 );
            nPointCount += aTmp.GetSize();
            pLine->Insert( aTmp, i );
        }
    }
    else
        pLine = NULL;

#if 0
    ULONG nPolyPtr = (ULONG)&rXPoly;
    String aDbgFile( "d:\\" );
    aDbgFile += nPolyPtr;
    aDbgFile += ".pol";
    SvFileStream aStream( aDbgFile, STREAM_WRITE|STREAM_TRUNC );
    aStream << "pPoly: " << String( (ULONG)pPoly ).GetStr();
    for ( USHORT nPoly = 0; nPoly < pPoly->Count(); nPoly++ )
    {
        const Polygon& rPoly = pPoly->GetObject( nPoly );
        for ( USHORT n = 0; n < rPoly.GetSize(); n++ )
        {
            const Point& rPoint = rPoly.GetPoint( n );
            aStream << String( rPoint.X() ).GetStr() << ", " << String( rPoint.Y() ).GetStr()  << endl;
        }
    }
#endif
}

#ifdef WIN
#pragma optimize ( "", on )
#endif

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
    for( USHORT i = 0; i < nCacheSize; ++i )
        delete pCache[i];
    delete pCache;
    delete pRangeArr;
    delete pPoly;
    delete pLine;
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
    USHORT nCut;
    USHORT nLast;
    USHORT nNext;
    BYTE nAct;
    BYTE nFirst;
    BOOL bClosed : 1;
    BOOL bInner : 1;
    BOOL bMultiple : 1;
    BOOL bConcat : 1;
    void NoteRange( BOOL bToggle );
    long Cut( long nY, const Point& rPt1, const Point& rPt2 );
    void Add();
    void _NoteFarPoint( long nPx, long nPyDiff, long nDiff );
    void NoteFarPoint( long nPx, long nPyDiff, long nDiff )
        { if( nDiff ) _NoteFarPoint( nPx, nPyDiff, nDiff ); }
    long CalcMax( const Point& rPt1, const Point& rPt2, long nRange, long nFar );
    void CheckCut( const Point& rLst, const Point& rNxt );
public:
    SvxBoundArgs( TextRanger* pRanger, SvLongs *pLong, const Range& rRange )
        : aBoolArr( 4, 4 ), pLongArr( pLong ), pTextRanger( pRanger ),
        nTop( rRange.Min() ), nBottom( rRange.Max() ),
        // pRanger->GetUpper() ist der Abstand des Objekts nach _oben_, von der
        // Textzeile aus betrachtet ist dies der Bereich _unter_ der Zeile,
        // der mit beruecksichtigt werden muss.
        nLowDiff( pRanger->GetUpper() ), nUpDiff( pRanger->GetLower() ),
        bInner( pRanger->IsInner() ), bMultiple( bInner || !pRanger->IsSimple() ),
        bConcat( FALSE )
        {
            nUpper = nTop - nUpDiff;
            nLower = nBottom + nLowDiff;
            pLongArr->Remove( 0, pLongArr->Count() );
        }

    void NotePoint( const long nX )
    { NoteMargin( nX - pTextRanger->GetLeft(), nX + pTextRanger->GetRight() );}
    void NoteMargin( const long nL, const long nR )
        { if( nMin > nL ) nMin = nL; if( nMax < nR ) nMax = nR; }
    USHORT Area( const Point& rPt );
    void NoteUpLow( long nX, const BYTE nArea );
    void Calc( const PolyPolygon& rPoly );
    void Concat( const PolyPolygon* pPoly );
    // inlines
    void NoteLast() { if( bMultiple ) NoteRange( nAct == nFirst ); }
    void SetClosed( const BOOL bNew ){ bClosed = bNew; }
    BOOL IsClosed() const { return bClosed; }
    void SetConcat( const BOOL bNew ){ bConcat = bNew; }
    BOOL IsConcat() const { return bConcat; }
    BYTE GetAct() const { return nAct; }
};

long SvxBoundArgs::CalcMax( const Point& rPt1, const Point& rPt2,
    long nRange, long nFarRange )
{
    double nDx = Cut( nRange, rPt1, rPt2 ) - Cut( nFarRange, rPt1, rPt2 );
    double nYps;
    if( nDx < 0 )
    {
        nDx = -nDx;
        nYps = pTextRanger->GetRight();
    }
    else
        nYps = pTextRanger->GetLeft();
    nYps *= nYps;
    nYps += nDx * nDx;
    nYps = nRange + nDx * ( nFarRange - nRange ) / sqrt( nYps );

    BOOL bNote;
    if( nYps < rPt2.Y() )
        bNote = nYps > rPt1.Y();
    else
        bNote = nYps < rPt1.Y();
    if( bNote )
        return( long( nYps ) );
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

void SvxBoundArgs::_NoteFarPoint( long nPx, long nPyDiff, long nDiff )
{
    long nTmpX;
#ifndef PRODUCT
    if( pTextRanger->IsFlag7() || pTextRanger->IsFlag6() )
    {
        if( pTextRanger->IsFlag7() )
            nPyDiff = nDiff;
        nTmpX = nPx - ( nPyDiff * pTextRanger->GetLeft() ) / nDiff;
        nPyDiff = nPx + ( nPyDiff * pTextRanger->GetRight() ) / nDiff;
    }
    else
#endif
    {
        double nQuot = 2 * nDiff - nPyDiff;
        nQuot *= nPyDiff;
        nQuot = sqrt( nQuot );
        nQuot /= nDiff;
        nTmpX = nPx - long( pTextRanger->GetLeft() * nQuot );
        nPyDiff = nPx + long( pTextRanger->GetRight() * nQuot );
    }
    NoteMargin( nTmpX, nPyDiff );
}

void SvxBoundArgs::NoteRange( BOOL bToggle )
{
    DBG_ASSERT( nMax >= nMin || bInner, "NoteRange: Min > Max?");
    if( nMax < nMin )
        return;
    if( !bClosed )
        bToggle = FALSE;
    USHORT nIdx = 0;
    USHORT nCount = pLongArr->Count();
    DBG_ASSERT( nCount == 2 * aBoolArr.Count(), "NoteRange: Incompatible Sizes" );
    while( nIdx < nCount && (*pLongArr)[ nIdx ] < nMin )
        ++nIdx;
    BOOL bOdd = nIdx % 2 ? TRUE : FALSE;
    // Kein Ueberlappung mit vorhandenen Intervallen?
    if( nIdx == nCount || ( !bOdd && nMax < (*pLongArr)[ nIdx ] ) )
    {   // Dann wird ein neues eingefuegt ...
        pLongArr->Insert( nMin, nIdx );
        pLongArr->Insert( nMax, nIdx + 1 );
        aBoolArr.Insert( bToggle, nIdx / 2 );
    }
    else
    {   // ein vorhandes Intervall erweitern ...
        USHORT nMaxIdx = nIdx;
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
        USHORT nDiff = nMaxIdx - nIdx;
        nMaxIdx = nIdx / 2; // Ab hier ist nMaxIdx der Index im BoolArray.
        if( nDiff )
        {
            (*pLongArr).Remove( nIdx + 1, nDiff );
            nDiff /= 2;
            USHORT nStop = nMaxIdx + nDiff;
            for( USHORT i = nMaxIdx; i < nStop; ++i )
                bToggle ^= aBoolArr[ i ];
            aBoolArr.Remove( nMaxIdx, nDiff );
        }
        DBG_ASSERT( nMaxIdx < aBoolArr.Count(), "NoteRange: Too much deleted" );
        aBoolArr[ nMaxIdx ] ^= bToggle;
    }
}

void SvxBoundArgs::Calc( const PolyPolygon& rPoly )
{
    USHORT nCount;
    nAct = 0;
    for( USHORT i = 0; i < rPoly.Count(); ++i )
    {
        const Polygon& rPol = rPoly[ i ];
        nCount = rPol.GetSize();
        if( nCount )
        {
            const Point& rNull = rPol[ 0 ];
            SetClosed( IsConcat() || ( rNull == rPol[ nCount - 1 ] ) );
#ifndef PRODUCT
#ifdef DEBUG
            static bAllwaysClosed = FALSE;
            if( bAllwaysClosed )
                SetClosed( TRUE );
#endif
#endif
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
                        NoteFarPoint( rNull.X(), nLower - rNull.Y(), nLowDiff );
                    else
                        NoteFarPoint( rNull.X(), rNull.Y() - nUpper, nUpDiff );
                }
                else
                {
                    if( bMultiple || !nAct )
                    {
                        nMin = rNull.X() - pTextRanger->GetLeft();
                        nMax = rNull.X() + pTextRanger->GetRight();
                    }
                    else
                        NotePoint( rNull.X() );
                }
                nFirst = 0; // In welcher Richtung wird die Zeile verlassen?
                nAct = 3;   // Wir sind z.Z. innerhalb der Zeile.
            }
            if( nCount > 1 )
            {
                USHORT nIdx = 1;
                while( TRUE )
                {
                    const Point& rLast = rPol[ nIdx - 1 ];
                    if( nIdx == nCount )
                        nIdx = 0;
                    const Point& rNext = rPol[ nIdx ];
                    nNext = Area( rNext );
                    nCut = nNext ^ nLast;
                    USHORT nOldAct = nAct;
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
                            NotePoint( rNext.X() );
                        else if( nNext & 1 )
                            NoteFarPoint( rNext.X(),nLower-rNext.Y(),nLowDiff );
                        else
                            NoteFarPoint( rNext.X(),rNext.Y()-nUpper,nUpDiff );
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
                    nTmpMin = nMin + 2 * pTextRanger->GetLeft();
                    nTmpMax = nMax - 2 * pTextRanger->GetRight();
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
    USHORT nLongIdx = 1;
    USHORT nCount = aBoolArr.Count();
    if( nCount && ( !bInner || !pTextRanger->IsSimple() ) )
    {
        BOOL bDelete = aBoolArr[ 0 ];
        if( bInner )
            bDelete = !bDelete;
        for( USHORT nBoolIdx = 1; nBoolIdx < nCount; ++nBoolIdx )
        {
            if( bDelete )
            {
                USHORT nNext = 2;
                while( nBoolIdx < nCount && !aBoolArr[ nBoolIdx++ ] &&
                       (!bInner || nBoolIdx < nCount ) )
                    nNext += 2;
                pLongArr->Remove( nLongIdx, nNext );
                nNext /= 2;
                nBoolIdx -= nNext;
                nCount -= nNext;
                aBoolArr.Remove( nBoolIdx, nNext );
                if( nBoolIdx )
                    aBoolArr[ nBoolIdx - 1 ] = FALSE;
#ifdef DEBUG
                else
                    ++nNext;
#endif
            }
            bDelete = nBoolIdx < nCount && aBoolArr[ nBoolIdx ];
            nLongIdx += 2;
            DBG_ASSERT( nLongIdx == 2*nBoolIdx+1, "BoundArgs: Array-Idx Confusion" );
            DBG_ASSERT( aBoolArr.Count()*2 == pLongArr->Count(),
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
    SetConcat( TRUE );
    DBG_ASSERT( pPoly, "Nothing to do?" );
    SvLongs *pOld = pLongArr;
    pLongArr = new SvLongs( 2, 8 );
    aBoolArr.Remove( 0, aBoolArr.Count() );
    bInner = FALSE;
    Calc( *pPoly );
    USHORT nCount = pLongArr->Count();
    USHORT nIdx = 0;
    USHORT i = 0;
    BOOL bSubtract = pTextRanger->IsInner();
    while( i < nCount )
    {
        USHORT nOldCount = pOld->Count();
        if( nIdx == nOldCount )
        {   // Am Ende des alten Arrays angelangt...
            if( !bSubtract )
                pOld->Insert( pLongArr, nIdx, i, USHRT_MAX );
            break;
        }
        long nLeft = (*pLongArr)[ i++ ];
        long nRight = (*pLongArr)[ i++ ];
        USHORT nLeftPos = nIdx + 1;
        while( nLeftPos < nOldCount && nLeft > (*pOld)[ nLeftPos ] )
            nLeftPos += 2;
        if( nLeftPos >= nOldCount )
        {   // Das aktuelle Intervall gehoert ans Ende des alten Arrays...
            if( !bSubtract )
                pOld->Insert( pLongArr, nOldCount, i - 2, USHRT_MAX );
            break;
        }
        USHORT nRightPos = nLeftPos - 1;
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

USHORT SvxBoundArgs::Area( const Point& rPt )
{
    if( rPt.Y() >= nBottom )
    {
        if( rPt.Y() >= nLower )
            return 5;
        return 1;
    }
    if( rPt.Y() <= nTop )
    {
        if( rPt.Y() <= nUpper )
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

long SvxBoundArgs::Cut( long nY, const Point& rPt1, const Point& rPt2 )
{
    double nQuot = nY - rPt1.Y();
    nQuot /= ( rPt2.Y() - rPt1.Y() );
    nQuot *= ( rPt2.X() - rPt1.X() );
    return long( rPt1.X() + nQuot );
}

void SvxBoundArgs::NoteUpLow( long nX, const BYTE nArea )
{
    if( nAct )
    {
#ifndef PRODUCT
        if( pTextRanger->IsFlag7() )
            NotePoint( nX );
        else
#endif
        NoteMargin( nX, nX );
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
#ifndef PRODUCT
        if( pTextRanger->IsFlag7() )
        {
            nMin = nX - pTextRanger->GetLeft();
            nMax = nX + pTextRanger->GetRight();
        }
        else
#endif
        {
            nMin = nX;
            nMax = nX;
        }
    }
}

SvLongsPtr TextRanger::GetTextRanges( const Range& rRange )
{
    DBG_ASSERT( rRange.Min() || rRange.Max(), "Zero-Range not allowed, Bye Bye" );
    USHORT nIndex = 0;
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
        aArg.Calc( *pPoly );
        if( pLine )
            aArg.Concat( pLine );
    }
    return pCache[ nIndex ];
}

const Rectangle& TextRanger::_GetBoundRect()
{
    DBG_ASSERT( 0 == pBound, "Don't call twice." );
    pBound = new Rectangle( pPoly->GetBoundRect() );
    return *pBound;
}


