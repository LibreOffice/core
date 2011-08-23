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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef _XOUTX_HXX
#include <xoutx.hxx>
#endif

#include "txtrange.hxx"
#include <math.h>


namespace binfilter {

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

/*N*/ TextRanger::TextRanger( const XPolyPolygon& rXPoly, const XPolyPolygon* pXLine,
/*N*/ 	USHORT nCacheSz, USHORT nLft, USHORT nRght, BOOL bSimpl, BOOL bInnr,
/*N*/ 	BOOL bVert ) :
/*N*/ 	pBound( NULL ), nCacheSize( nCacheSz ), nCacheIdx( 0 ), nPointCount( 0 ),
/*N*/ 	nLeft( nLft ), nRight( nRght ),	nUpper( 0 ), nLower( 0 ),
/*N*/ 	bSimple( bSimpl ), bInner( bInnr ), bVertical( bVert )
/*N*/ {
/*N*/ #ifdef DBG_UTIL
/*N*/ 	bFlag3 = bFlag4 = bFlag5 = bFlag6 = bFlag7 = FALSE;
/*N*/ #endif
/*N*/ 	pRangeArr = new Range[ nCacheSize ];
/*N*/ 	pCache = new SvLongsPtr[ nCacheSize ];
/*N*/ 	memset( pRangeArr, 0, nCacheSize * sizeof( Range ) );
/*N*/ 	memset( pCache, 0, nCacheSize * sizeof( SvLongsPtr ) );
/*N*/ 	USHORT nCount = rXPoly.Count();
/*N*/ 	pPoly = new PolyPolygon( nCount );
/*N*/ 	for( USHORT i = 0; i < nCount; ++i )
/*N*/ 	{
/*N*/ 		Polygon aTmp = XOutCreatePolygon( rXPoly[ i ], NULL, 100 );
/*N*/ 		nPointCount += aTmp.GetSize();
/*N*/ 		pPoly->Insert( aTmp, i );
/*N*/ 	}
/*N*/ 	if( pXLine )
/*N*/ 	{
/*N*/ 		nCount = pXLine->Count();
/*N*/ 		pLine = new PolyPolygon();
/*N*/ 		for( USHORT i = 0; i < nCount; ++i )
/*N*/ 		{
/*N*/ 			Polygon aTmp = XOutCreatePolygon( (*pXLine)[ i ], NULL, 100 );
/*N*/ 			nPointCount += aTmp.GetSize();
/*N*/ 			pLine->Insert( aTmp, i );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 		pLine = NULL;
/*N*/ 
/*N*/ #if 0
/*N*/ 	ULONG nPolyPtr = (ULONG)&rXPoly;
/*N*/ 	String aDbgFile( "d:\\" );
/*N*/ 	aDbgFile += nPolyPtr;
/*N*/ 	aDbgFile += ".pol";
/*N*/ 	SvFileStream aStream( aDbgFile, STREAM_WRITE|STREAM_TRUNC );
/*N*/ 	aStream << "pPoly: " << String( (ULONG)pPoly ).GetStr();
/*N*/ 	for ( USHORT nPoly = 0; nPoly < pPoly->Count(); nPoly++ )
/*N*/ 	{
/*N*/ 		const Polygon& rPoly = pPoly->GetObject( nPoly );
/*N*/ 		for ( USHORT n = 0; n < rPoly.GetSize(); n++ )
/*N*/ 		{
/*N*/ 			const Point& rPoint = rPoly.GetPoint( n );
/*N*/ 			aStream << String( rPoint.X() ).GetStr() << ", " << String( rPoint.Y() ).GetStr()  << endl;
/*N*/ 		}
/*N*/ 	}
/*N*/ #endif
/*N*/ }

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

/*N*/ TextRanger::~TextRanger()
/*N*/ {
/*N*/ 	for( USHORT i = 0; i < nCacheSize; ++i )
/*N*/ 		delete pCache[i];
/*N*/ 	delete[] pCache;
/*N*/ 	delete[] pRangeArr;
/*N*/ 	delete pPoly;
/*N*/ 	delete pLine;
/*N*/ }

/*-----------------17.11.00 09:49-------------------
 * TextRanger::SetVertical(..)
 * If there's is a change in the writing direction,
 * the cache has to be cleared.
 * --------------------------------------------------*/


/*************************************************************************
|*
|*    SvxBoundArgs
|*
|*    Beschreibung
|*    Ersterstellung    20.01.97
|*    Letzte Aenderung  20.01.97 AMA
|*
*************************************************************************/

/*N*/ class SvxBoundArgs
/*N*/ {
/*N*/ 	SvBools aBoolArr;
/*N*/ 	SvLongs *pLongArr;
/*N*/ 	TextRanger *pTextRanger;
/*N*/ 	long nMin;
/*N*/ 	long nMax;
/*N*/ 	long nTop;
/*N*/ 	long nBottom;
/*N*/ 	long nUpDiff;
/*N*/ 	long nLowDiff;
/*N*/ 	long nUpper;
/*N*/ 	long nLower;
/*N*/ 	long nStart;
/*N*/ 	long nEnd;
/*N*/ 	USHORT nCut;
/*N*/ 	USHORT nLast;
/*N*/ 	USHORT nNext;
/*N*/ 	BYTE nAct;
/*N*/ 	BYTE nFirst;
/*N*/ 	BOOL bClosed : 1;
/*N*/ 	BOOL bInner : 1;
/*N*/ 	BOOL bMultiple : 1;
/*N*/ 	BOOL bConcat : 1;
/*N*/ 	BOOL bRotate : 1;
/*N*/ 	void NoteRange( BOOL bToggle );
/*N*/ 	long Cut( long nY, const Point& rPt1, const Point& rPt2 );
/*N*/ 	void Add();
/*N*/ 	void CheckCut( const Point& rLst, const Point& rNxt );
/*N*/ 	inline long A( const Point& rP ) const { return bRotate ? rP.Y() : rP.X(); }
/*N*/ 	inline long B( const Point& rP ) const { return bRotate ? rP.X() : rP.Y(); }
/*N*/ public:
/*N*/ 	SvxBoundArgs( TextRanger* pRanger, SvLongs *pLong, const Range& rRange );
/*N*/ 	void NotePoint( const long nA ) { NoteMargin( nA - nStart, nA + nEnd ); }
/*N*/ 	void NoteMargin( const long nL, const long nR )
/*N*/ 		{ if( nMin > nL ) nMin = nL; if( nMax < nR ) nMax = nR; }
/*N*/ 	USHORT Area( const Point& rPt );
/*N*/ 	void NoteUpLow( long nA, const BYTE nArea );
/*N*/ 	void Calc( const PolyPolygon& rPoly );
/*N*/ 	void Concat( const PolyPolygon* pPoly );
/*N*/ 	// inlines
/*N*/ 	void NoteLast() { if( bMultiple ) NoteRange( nAct == nFirst ); }
/*N*/ 	void SetClosed( const BOOL bNew ){ bClosed = bNew; }
/*N*/ 	BOOL IsClosed() const { return bClosed; }
/*N*/ 	void SetConcat( const BOOL bNew ){ bConcat = bNew; }
/*N*/ 	BOOL IsConcat() const { return bConcat; }
/*N*/ 	BYTE GetAct() const { return nAct; }
/*N*/ };

/*N*/ SvxBoundArgs::SvxBoundArgs( TextRanger* pRanger, SvLongs *pLong,
/*N*/ 	const Range& rRange )
/*N*/ 	: aBoolArr( 4, 4 ), pLongArr( pLong ), pTextRanger( pRanger ),
/*N*/ 	nTop( rRange.Min() ), nBottom( rRange.Max() ),
/*N*/ 	bInner( pRanger->IsInner() ), bMultiple( bInner || !pRanger->IsSimple() ),
/*N*/ 	bConcat( FALSE ), bRotate( pRanger->IsVertical() )
/*N*/ {
/*N*/ 	if( bRotate )
/*N*/ 	{
/*?*/ 		nStart = pRanger->GetUpper();
/*?*/ 		nEnd = pRanger->GetLower();
/*?*/ 		nLowDiff = pRanger->GetLeft();
/*?*/ 		nUpDiff = pRanger->GetRight();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		nStart = pRanger->GetLeft();
/*N*/ 		nEnd = pRanger->GetRight();
/*N*/ 		nLowDiff = pRanger->GetUpper();
/*N*/ 		nUpDiff = pRanger->GetLower();
/*N*/ 	}
/*N*/ 	nUpper = nTop - nUpDiff;
/*N*/ 	nLower = nBottom + nLowDiff;
/*N*/ 	pLongArr->Remove( 0, pLongArr->Count() );
/*N*/ }


/*N*/ void SvxBoundArgs::CheckCut( const Point& rLst, const Point& rNxt )
/*N*/ {
/*N*/ 	if( nCut & 1 )
/*N*/ 		NotePoint( Cut( nBottom, rLst, rNxt ) );
/*N*/ 	if( nCut & 2 )
/*N*/ 		NotePoint( Cut( nTop, rLst, rNxt ) );
/*N*/ 	if( rLst.X() != rNxt.X() && rLst.Y() != rNxt.Y() )
/*N*/ 	{
/*N*/ 		long nYps;
/*N*/ 		if( nLowDiff && ( ( nCut & 1 ) || nLast == 1 || nNext == 1 ) )
/*N*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 nYps = CalcMax( rLst, rNxt, nBottom, nLower );
/*N*/ 		}
/*N*/ 		if( nUpDiff && ( ( nCut & 2 ) || nLast == 2 || nNext == 2 ) )
/*N*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 nYps = CalcMax( rLst, rNxt, nTop, nUpper );
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ void SvxBoundArgs::NoteRange( BOOL bToggle )
/*N*/ {
/*N*/ 	DBG_ASSERT( nMax >= nMin || bInner, "NoteRange: Min > Max?");
/*N*/ 	if( nMax < nMin )
/*N*/ 		return;
/*N*/ 	if( !bClosed )
/*N*/ 		bToggle = FALSE;
/*N*/ 	USHORT nIdx = 0;
/*N*/ 	USHORT nCount = pLongArr->Count();
/*N*/ 	DBG_ASSERT( nCount == 2 * aBoolArr.Count(), "NoteRange: Incompatible Sizes" );
/*N*/ 	while( nIdx < nCount && (*pLongArr)[ nIdx ] < nMin )
/*N*/ 		++nIdx;
/*N*/ 	BOOL bOdd = nIdx % 2 ? TRUE : FALSE;
/*N*/ 	// Kein Ueberlappung mit vorhandenen Intervallen?
/*N*/ 	if( nIdx == nCount || ( !bOdd && nMax < (*pLongArr)[ nIdx ] ) )
/*N*/ 	{	// Dann wird ein neues eingefuegt ...
/*N*/ 		pLongArr->Insert( nMin, nIdx );
/*N*/ 		pLongArr->Insert( nMax, nIdx + 1 );
/*N*/ 		aBoolArr.Insert( bToggle, nIdx / 2 );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{   // ein vorhandes Intervall erweitern ...
/*N*/ 		USHORT nMaxIdx = nIdx;
/*N*/ 		// Wenn wir auf einer linken Intervallgrenze gelandet sind, muss diese
/*N*/ 		// auf nMin gesenkt werden.
/*N*/ 		if( bOdd )
/*N*/ 			--nIdx;
/*N*/ 		else
/*N*/ 			(*pLongArr)[ nIdx ] = nMin;
/*N*/ 		while( nMaxIdx < nCount && (*pLongArr)[ nMaxIdx ] < nMax )
/*N*/ 			++nMaxIdx;
/*N*/ 		DBG_ASSERT( nMaxIdx > nIdx || nMin == nMax, "NoteRange: Funny Situation." );
/*N*/ 		if( nMaxIdx )
/*N*/ 			--nMaxIdx;
/*N*/ 		if( nMaxIdx < nIdx )
/*N*/ 			nMaxIdx = nIdx;
/*N*/ 		// Wenn wir auf einer rechten Intervallgrenze landen, muss diese
/*N*/ 		// auf nMax angehoben werden.
/*N*/ 		if( nMaxIdx % 2 )
/*N*/ 			(*pLongArr)[ nMaxIdx-- ] = nMax;
/*N*/ 		// Jetzt werden eventuell noch Intervalle verschmolzen
/*N*/ 		USHORT nDiff = nMaxIdx - nIdx;
/*N*/ 		nMaxIdx = nIdx / 2; // Ab hier ist nMaxIdx der Index im BoolArray.
/*N*/ 		if( nDiff )
/*N*/ 		{
/*N*/ 			(*pLongArr).Remove( nIdx + 1, nDiff );
/*N*/ 			nDiff /= 2;
/*N*/ 			USHORT nStop = nMaxIdx + nDiff;
/*N*/ 			for( USHORT i = nMaxIdx; i < nStop; ++i )
/*N*/ 				bToggle ^= aBoolArr[ i ];
/*N*/ 			aBoolArr.Remove( nMaxIdx, nDiff );
/*N*/ 		}
/*N*/ 		DBG_ASSERT( nMaxIdx < aBoolArr.Count(), "NoteRange: Too much deleted" );
/*N*/ 		aBoolArr[ nMaxIdx ] ^= bToggle;
/*N*/ 	}
/*N*/ }

/*N*/ void SvxBoundArgs::Calc( const PolyPolygon& rPoly )
/*N*/ {
/*N*/ 	USHORT nCount;
/*N*/ 	nAct = 0;
/*N*/ 	for( USHORT i = 0; i < rPoly.Count(); ++i )
/*N*/ 	{
/*N*/ 		const Polygon& rPol = rPoly[ i ];
/*N*/ 		nCount = rPol.GetSize();
/*N*/ 		if( nCount )
/*N*/ 		{
/*N*/ 			const Point& rNull = rPol[ 0 ];
/*N*/ 			SetClosed( IsConcat() || ( rNull == rPol[ nCount - 1 ] ) );
/*N*/ 			nLast = Area( rNull );
/*N*/ 			if( nLast & 12 )
/*N*/ 			{
/*N*/ 				nFirst = 3;
/*N*/ 				if( bMultiple )
/*N*/ 					nAct = 0;
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				// Der erste Punkt des Polygons liegt innerhalb der Zeile.
/*N*/ 				if( nLast )
/*N*/ 				{
/*?*/ 					DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if( bMultiple || !nAct )
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					if( bMultiple || !nAct )
/*N*/ 					{
/*N*/ 						nMin = A(rNull);
/*N*/ 						nMax = nMin + nEnd;
/*N*/ 						nMin -= nStart;
/*N*/ 					}
/*N*/ 					else
/*?*/ 						NotePoint( A(rNull) );
/*N*/ 				}
/*N*/ 				nFirst = 0;	// In welcher Richtung wird die Zeile verlassen?
/*N*/ 				nAct = 3;	// Wir sind z.Z. innerhalb der Zeile.
/*N*/ 			}
/*N*/ 			if( nCount > 1 )
/*N*/ 			{
/*N*/ 				USHORT nIdx = 1;
/*N*/ 				while( TRUE )
/*N*/ 				{
/*N*/ 					const Point& rLast = rPol[ nIdx - 1 ];
/*N*/ 					if( nIdx == nCount )
/*N*/ 						nIdx = 0;
/*N*/ 					const Point& rNext = rPol[ nIdx ];
/*N*/ 					nNext = Area( rNext );
/*N*/ 					nCut = nNext ^ nLast;
/*N*/ 					USHORT nOldAct = nAct;
/*N*/ 					if( nAct )
/*N*/ 						CheckCut( rLast, rNext );
/*N*/ 					if( nCut & 4 )
/*N*/ 					{
/*N*/ 						NoteUpLow( Cut( nLower, rLast, rNext ), 2 );
/*N*/ 						if( nAct && nAct != nOldAct )
/*N*/ 						{
/*N*/ 							nOldAct = nAct;
/*N*/ 							CheckCut( rLast, rNext );
/*N*/ 						}
/*N*/ 					}
/*N*/ 					if( nCut & 8 )
/*N*/ 					{
/*N*/ 						NoteUpLow( Cut( nUpper, rLast, rNext ), 1 );
/*N*/ 						if( nAct && nAct != nOldAct )
/*N*/ 							CheckCut( rLast, rNext );
/*N*/ 					}
/*N*/ 					if( !nIdx )
/*N*/ 					{
/*N*/ 						if( !( nNext & 12 ) )
/*N*/ 							NoteLast();
/*N*/ 						break;
/*N*/ 					}
/*N*/ 					if( !( nNext & 12 ) )
/*N*/ 					{
/*N*/ 						if( !nNext )
/*N*/ 							NotePoint( A(rNext) );
/*N*/ 						else if( nNext & 1 )
/*?*/ 						{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 	NoteFarPoint( A(rNext), nLower-B(rNext), nLowDiff );
/*N*/ 						else
/*?*/ 						{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 	NoteFarPoint( A(rNext), B(rNext)-nUpper, nUpDiff );
/*N*/ 					}
/*N*/ 					nLast = nNext;
/*N*/ 					if( ++nIdx == nCount && !IsClosed() )
/*N*/ 					{
/*N*/ 						if( !( nNext & 12 ) )
/*N*/ 							NoteLast();
/*N*/ 						break;
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			if( bMultiple && IsConcat() )
/*N*/ 			{
/*N*/ 				Add();
/*N*/ 				nAct = 0;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if( !bMultiple )
/*N*/ 	{
/*?*/ 		DBG_ASSERT( pLongArr->Count() == 0, "I said: Simple!" );
/*?*/ 		if( nAct )
/*?*/ 		{
/*?*/ 			if( bInner )
/*?*/ 			{
/*?*/ 				long nTmpMin, nTmpMax;
/*?*/ 				{
/*?*/ 					nTmpMin = nMin + 2 * nStart;
/*?*/ 					nTmpMax = nMax - 2 * nEnd;
/*?*/ 					if( nTmpMin <= nTmpMax )
/*?*/ 					{
/*?*/ 						pLongArr->Insert( nTmpMin, 0 );
/*?*/ 						pLongArr->Insert( nTmpMax, 1 );
/*?*/ 					}
/*?*/ 				}
/*?*/ 			}
/*?*/ 			else
/*?*/ 			{
/*?*/ 				pLongArr->Insert( nMin, 0 );
/*?*/ 				pLongArr->Insert( nMax, 1 );
/*?*/ 			}
/*?*/ 		}
/*N*/ 	}
/*N*/ 	else if( !IsConcat() )
/*N*/ 		Add();
/*N*/ }

/*N*/ void SvxBoundArgs::Add()
/*N*/ {
/*N*/ 	USHORT nLongIdx = 1;
/*N*/ 	USHORT nCount = aBoolArr.Count();
/*N*/ 	if( nCount && ( !bInner || !pTextRanger->IsSimple() ) )
/*N*/ 	{
/*N*/ 		BOOL bDelete = aBoolArr[ 0 ];
/*N*/ 		if( bInner )
/*N*/ 			bDelete = !bDelete;
/*N*/ 		for( USHORT nBoolIdx = 1; nBoolIdx < nCount; ++nBoolIdx )
/*N*/ 		{
/*N*/ 			if( bDelete )
/*N*/ 			{
/*N*/ 				USHORT nNext = 2;
/*N*/ 				while( nBoolIdx < nCount && !aBoolArr[ nBoolIdx++ ] &&
/*N*/ 					   (!bInner || nBoolIdx < nCount ) )
/*N*/ 					nNext += 2;
/*N*/ 				pLongArr->Remove( nLongIdx, nNext );
/*N*/ 				nNext /= 2;
/*N*/ 				nBoolIdx -= nNext;
/*N*/ 				nCount -= nNext;
/*N*/ 				aBoolArr.Remove( nBoolIdx, nNext );
/*N*/ 				if( nBoolIdx )
/*N*/ 					aBoolArr[ nBoolIdx - 1 ] = FALSE;
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*N*/ 				else
/*N*/ 					++nNext;
/*N*/ #endif
/*N*/ 			}
/*N*/ 			bDelete = nBoolIdx < nCount && aBoolArr[ nBoolIdx ];
/*N*/ 			nLongIdx += 2;
/*N*/ 			DBG_ASSERT( nLongIdx == 2*nBoolIdx+1, "BoundArgs: Array-Idx Confusion" );
/*N*/ 			DBG_ASSERT( aBoolArr.Count()*2 == pLongArr->Count(),
/*N*/ 						"BoundArgs: Array-Count: Confusion" );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if( 0 != ( nCount = pLongArr->Count() ) )
/*N*/ 	{
/*N*/ 		if( bInner )
/*N*/ 		{
/*?*/ 			pLongArr->Remove( 0, 1 );
/*?*/ 			pLongArr->Remove( pLongArr->Count() - 1, 1 );
/*?*/ 
/*?*/ 			// Hier wird die Zeile beim "einfachen" Konturumfluss im Innern
/*?*/ 			// in ein grosses Rechteck zusammengefasst.
/*?*/ 			// Zur Zeit (April 1999) wertet die EditEngine nur das erste Rechteck
/*?*/ 			// aus, falls sie eines Tages in der Lage ist, eine Zeile in mehreren
/*?*/ 			// Teilen auszugeben, kann es sinnvoll sein, die folgenden Zeilen
/*?*/ 			// zu loeschen.
/*?*/ 			if( pTextRanger->IsSimple() && pLongArr->Count() > 2 )
/*?*/ 				pLongArr->Remove( 1, pLongArr->Count() - 2 );
/*?*/ 
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void SvxBoundArgs::Concat( const PolyPolygon* pPoly )
/*N*/ {
/*N*/ 	SetConcat( TRUE );
/*N*/ 	DBG_ASSERT( pPoly, "Nothing to do?" );
/*N*/ 	SvLongs *pOld = pLongArr;
/*N*/ 	pLongArr = new SvLongs( 2, 8 );
/*N*/ 	aBoolArr.Remove( 0, aBoolArr.Count() );
/*N*/ 	bInner = FALSE;
/*N*/ 	Calc( *pPoly );
/*N*/ 	USHORT nCount = pLongArr->Count();
/*N*/ 	USHORT nIdx = 0;
/*N*/ 	USHORT i = 0;
/*N*/ 	BOOL bSubtract = pTextRanger->IsInner();
/*N*/ 	while( i < nCount )
/*N*/ 	{
/*N*/ 		USHORT nOldCount = pOld->Count();
/*N*/ 		if( nIdx == nOldCount )
/*N*/ 		{   // Am Ende des alten Arrays angelangt...
/*?*/ 			if( !bSubtract )
/*?*/ 				pOld->Insert( pLongArr, nIdx, i, USHRT_MAX );
/*?*/ 			break;
/*N*/ 		}
/*N*/ 		long nLeft = (*pLongArr)[ i++ ];
/*N*/ 		long nRight = (*pLongArr)[ i++ ];
/*N*/ 		USHORT nLeftPos = nIdx + 1;
/*N*/ 		while( nLeftPos < nOldCount && nLeft > (*pOld)[ nLeftPos ] )
/*N*/ 			nLeftPos += 2;
/*N*/ 		if( nLeftPos >= nOldCount )
/*N*/ 		{	// Das aktuelle Intervall gehoert ans Ende des alten Arrays...
/*?*/ 			if( !bSubtract )
/*?*/ 				pOld->Insert( pLongArr, nOldCount, i - 2, USHRT_MAX );
/*?*/ 			break;
/*N*/ 		}
/*N*/ 		USHORT nRightPos = nLeftPos - 1;
/*N*/ 		while( nRightPos < nOldCount && nRight >= (*pOld)[ nRightPos ] )
/*N*/ 			nRightPos += 2;
/*N*/ 		if( nRightPos < nLeftPos )
/*N*/ 		{   // Das aktuelle Intervall gehoert zwischen zwei alte Intervalle
/*?*/ 			if( !bSubtract )
/*?*/ 				pOld->Insert( pLongArr, nRightPos, i - 2, i );
/*?*/ 			nIdx = nRightPos + 2;
/*N*/ 		}
/*N*/ 		else if( bSubtract ) // Subtrahieren ggf. Trennen
/*N*/ 		{
/*?*/ 			long nOld;
/*?*/ 			if( nLeft > ( nOld = (*pOld)[ nLeftPos - 1 ] ) )
/*?*/ 			{   // Jetzt spalten wir den linken Teil ab...
/*?*/ 				if( nLeft - 1 > nOld )
/*?*/ 				{
/*?*/ 					pOld->Insert( nOld, nLeftPos - 1 );
/*?*/ 					pOld->Insert( nLeft - 1, nLeftPos );
/*?*/ 					nLeftPos += 2;
/*?*/ 					nRightPos += 2;
/*?*/ 				}
/*?*/ 			}
/*?*/ 			if( nRightPos - nLeftPos > 1 )
/*?*/ 				pOld->Remove( nLeftPos, nRightPos - nLeftPos - 1 );
/*?*/ 			if( ++nRight >= ( nOld = (*pOld)[ nLeftPos ] ) )
/*?*/ 				pOld->Remove( nLeftPos - 1, 2 );
/*?*/ 			else
/*?*/ 				(*pOld)[ nLeftPos - 1 ] = nRight;
/*N*/ 		}
/*N*/ 		else // Verschmelzen
/*N*/ 		{
/*N*/ 			if( nLeft < (*pOld)[ nLeftPos - 1 ] )
/*N*/ 				(*pOld)[ nLeftPos - 1 ] = nLeft;
/*N*/ 			if( nRight > (*pOld)[ nRightPos - 1 ] )
/*N*/ 				(*pOld)[ nRightPos - 1 ] = nRight;
/*N*/ 			if( nRightPos - nLeftPos > 1 )
/*?*/ 				pOld->Remove( nLeftPos, nRightPos - nLeftPos - 1 );
/*N*/ 
/*N*/ 		}
/*N*/ 		nIdx = nLeftPos - 1;
/*N*/ 	}
/*N*/ 	delete pLongArr;
/*N*/ }

/*************************************************************************
 * SvxBoundArgs::Area ermittelt den Bereich, in dem sich der Punkt befindet
 * 0 = innerhalb der Zeile
 * 1 = unterhalb, aber innerhalb der oberen Randes
 * 2 = oberhalb, aber innerhalb der unteren Randes
 * 5 = unterhalb des oberen Randes
 *10 = oberhalb des unteren Randes
 *************************************************************************/

/*N*/ USHORT SvxBoundArgs::Area( const Point& rPt )
/*N*/ {
/*N*/ 	long nB = B( rPt );
/*N*/ 	if( nB >= nBottom )
/*N*/ 	{
/*N*/ 		if( nB >= nLower )
/*N*/ 			return 5;
/*N*/ 		return 1;
/*N*/ 	}
/*N*/ 	if( nB <= nTop )
/*N*/ 	{
/*N*/ 		if( nB <= nUpper )
/*N*/ 			return 10;
/*N*/ 		return 2;
/*N*/ 	}
/*N*/ 	return 0;
/*N*/ }

/*************************************************************************
 * lcl_Cut berechnet die X-Koordinate der Strecke (Pt1-Pt2) auf der
 * Y-Koordinate nY.
 * Vorausgesetzt wird, dass einer der Punkte oberhalb und der andere
 * unterhalb der Y-Koordinate liegt.
 *************************************************************************/

/*N*/ long SvxBoundArgs::Cut( long nB, const Point& rPt1, const Point& rPt2 )
/*N*/ {
/*N*/ 	if( pTextRanger->IsVertical() )
/*N*/ 	{
/*?*/ 		double nQuot = nB - rPt1.X();
/*?*/ 		nQuot /= ( rPt2.X() - rPt1.X() );
/*?*/ 		nQuot *= ( rPt2.Y() - rPt1.Y() );
/*?*/ 		return long( rPt1.Y() + nQuot );
/*N*/ 	}
/*N*/ 	double nQuot = nB - rPt1.Y();
/*N*/ 	nQuot /= ( rPt2.Y() - rPt1.Y() );
/*N*/ 	nQuot *= ( rPt2.X() - rPt1.X() );
/*N*/ 	return long( rPt1.X() + nQuot );
/*N*/ }

/*N*/ void SvxBoundArgs::NoteUpLow( long nA, const BYTE nArea )
/*N*/ {
/*N*/ 	if( nAct )
/*N*/ 	{
/*N*/ 		NoteMargin( nA, nA );
/*N*/ 		if( bMultiple )
/*N*/ 		{
/*N*/ 			NoteRange( nArea != nAct );
/*N*/ 			nAct = 0;
/*N*/ 		}
/*N*/ 		if( !nFirst )
/*N*/ 			nFirst = nArea;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		nAct = nArea;
/*N*/ 		nMin = nA;
/*N*/ 		nMax = nA;
/*N*/ 	}
/*N*/ }

/*N*/ SvLongsPtr TextRanger::GetTextRanges( const Range& rRange )
/*N*/ {
/*N*/ 	DBG_ASSERT( rRange.Min() || rRange.Max(), "Zero-Range not allowed, Bye Bye" );
/*N*/ 	USHORT nIndex = 0;
/*N*/ 	while( nIndex < nCacheSize && rRange != pRangeArr[ nIndex ] )
/*N*/ 		++nIndex;
/*N*/ 	if( nIndex >= nCacheSize )
/*N*/ 	{
/*N*/ 		++nCacheIdx;
/*N*/ 		nCacheIdx %= nCacheSize;
/*N*/ 		pRangeArr[ nCacheIdx ] = rRange;
/*N*/ 		if( !pCache[ nCacheIdx ] )
/*N*/ 			pCache[ nCacheIdx ] = new SvLongs( 2, 8 );
/*N*/ 		nIndex = nCacheIdx;
/*N*/ 		SvxBoundArgs aArg( this, pCache[ nCacheIdx ], rRange );
/*N*/ 		aArg.Calc( *pPoly );
/*N*/ 		if( pLine )
/*N*/ 			aArg.Concat( pLine );
/*N*/ 	}
/*N*/ 	return pCache[ nIndex ];
/*N*/ }



}
