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

#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

//------------------------------------------------------------------------

#define SC_RANGELST_CXX			//fuer ICC

#include <stdlib.h>				// qsort

#include "document.hxx"
#include "refupdat.hxx"
#include "rechead.hxx"
namespace binfilter {


// === ScRangeList ====================================================

/*N*/ ScRangeList::~ScRangeList()
/*N*/ {
/*N*/ 	for ( ScRangePtr pR = First(); pR; pR = Next() )
/*N*/ 		delete pR;
/*N*/ }

/*N*/ void ScRangeList::RemoveAll()
/*N*/ {
/*N*/ 	for ( ScRangePtr pR = First(); pR; pR = Next() )
/*N*/ 		delete pR;
/*N*/ 	Clear();
/*N*/ }

/*N*/ USHORT ScRangeList::Parse( const String& rStr, ScDocument* pDoc, USHORT nMask )
/*N*/ {
/*N*/ 	if ( rStr.Len() )
/*N*/ 	{
/*N*/ 		nMask |= SCA_VALID;				// falls das jemand vergessen sollte
/*N*/ 		USHORT nResult = (USHORT)~0;	// alle Bits setzen
/*N*/ 		ScRange	aRange;
/*N*/ 		String aOne;
/*N*/ 		USHORT nTab = 0;
/*N*/ 		if ( pDoc )
/*N*/ 		{
/*N*/ 			//!	erste markierte Tabelle gibts nicht mehr am Dokument
/*N*/ 			//!	-> uebergeben? oder spaeter an den Ranges setzen
/*N*/ 		}
/*N*/ 		else
/*N*/ 			nTab = 0;
/*N*/ 		USHORT nTCount = rStr.GetTokenCount();
/*N*/ 		for ( USHORT i=0; i<nTCount; i++ )
/*N*/ 		{
/*N*/ 			aOne = rStr.GetToken(i);
/*N*/ 			if ( aOne.Search( ':' ) == STRING_NOTFOUND )
/*N*/ 			{	// Range muss es sein
/*N*/ 				String aStrTmp( aOne );
/*N*/ 				aOne += ':';
/*N*/ 				aOne += aStrTmp;
/*N*/ 			}
/*N*/ 			aRange.aStart.SetTab( nTab );	// Default Tab wenn nicht angegeben
/*N*/ 			USHORT nRes = aRange.Parse( aOne, pDoc );
/*N*/ 			if ( (nRes & nMask) == nMask )
/*N*/ 				Append( aRange );
/*N*/ 			nResult &= nRes;		// alle gemeinsamen Bits bleiben erhalten
/*N*/ 		}
/*N*/ 		return nResult;				// SCA_VALID gesetzt wenn alle ok
/*N*/ 	}
/*N*/ 	else
/*N*/ 		return 0;
/*N*/ }


/*N*/ void ScRangeList::Format( String& rStr, USHORT nFlags, ScDocument* pDoc ) const
/*N*/ {
/*N*/ 	rStr.Erase();
/*N*/ 	ULONG nCnt = Count();
/*N*/ 	for ( ULONG nIdx = 0; nIdx < nCnt; nIdx++ )
/*N*/ 	{
/*N*/ 		String aStr;
/*N*/ 		GetObject( nIdx )->Format( aStr, nFlags, pDoc );
/*N*/ 		if ( nIdx )
/*N*/ 			rStr += ';';
/*N*/ 		rStr += aStr;
/*N*/ 	}
/*N*/ }


/*N*/ void ScRangeList::Join( const ScRange& r, BOOL bIsInList )
/*N*/ {
/*N*/ 	if ( !Count() )
/*N*/ 	{
/*N*/ 		Append( r );
/*N*/ 		return ;
/*N*/ 	}
/*N*/ 	USHORT nCol1 = r.aStart.Col();
/*N*/ 	USHORT nRow1 = r.aStart.Row();
/*N*/ 	USHORT nTab1 = r.aStart.Tab();
/*N*/ 	USHORT nCol2 = r.aEnd.Col();
/*N*/ 	USHORT nRow2 = r.aEnd.Row();
/*N*/ 	USHORT nTab2 = r.aEnd.Tab();
/*N*/ 	ScRangePtr pOver = (ScRangePtr) &r;		// fies aber wahr wenn bInList
/*N*/ 	ULONG nOldPos;
/*N*/ 	if ( bIsInList )
/*N*/ 	{	// merken um ggbf. zu loeschen bzw. wiederherzustellen
/*N*/ 		nOldPos = GetPos( pOver );
/*N*/ 	}
/*N*/ 	BOOL bJoinedInput = FALSE;
/*N*/ 	for ( ScRangePtr p = First(); p && pOver; p = Next() )
/*N*/ 	{
/*N*/ 		if ( p == pOver )
/*N*/ 			continue;			// derselbe, weiter mit dem naechsten
/*N*/ 		BOOL bJoined = FALSE;
/*N*/ 		if ( p->In( r ) )
/*N*/ 		{	// Range r in Range p enthalten oder identisch
/*N*/ 			if ( bIsInList )
/*N*/ 				bJoined = TRUE;		// weg mit Range r
/*N*/ 			else
/*N*/ 			{	// das war's dann
/*N*/ 				bJoinedInput = TRUE;	// nicht anhaengen
/*N*/ 				break;	// for
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else if ( r.In( *p ) )
/*N*/ 		{	// Range p in Range r enthalten, r zum neuen Range machen
/*N*/ 			*p = r;
/*N*/ 			bJoined = TRUE;
/*N*/ 		}
/*N*/ 		if ( !bJoined && p->aStart.Tab() == nTab1 && p->aEnd.Tab() == nTab2 )
/*N*/ 		{	// 2D
/*N*/ 			if ( p->aStart.Col() == nCol1 && p->aEnd.Col() == nCol2 )
/*N*/ 			{
/*N*/ 				if ( p->aStart.Row() == nRow2+1 )
/*N*/ 				{	// oben
/*?*/ 					p->aStart.SetRow( nRow1 );
/*?*/ 					bJoined = TRUE;
/*N*/ 				}
/*N*/ 				else if ( p->aEnd.Row() == nRow1-1 )
/*N*/ 				{	// unten
/*N*/ 					p->aEnd.SetRow( nRow2 );
/*N*/ 					bJoined = TRUE;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else if ( p->aStart.Row() == nRow1 && p->aEnd.Row() == nRow2 )
/*N*/ 			{
/*N*/ 				if ( p->aStart.Col() == nCol2+1 )
/*N*/ 				{	// links
/*?*/ 					p->aStart.SetCol( nCol1 );
/*?*/ 					bJoined = TRUE;
/*N*/ 				}
/*N*/ 				else if ( p->aEnd.Col() == nCol1-1 )
/*N*/ 				{	// rechts
/*N*/ 					p->aEnd.SetCol( nCol2 );
/*N*/ 					bJoined = TRUE;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if ( bJoined )
/*N*/ 		{
/*N*/ 			if ( bIsInList )
/*N*/ 			{	// innerhalb der Liste Range loeschen
/*N*/ 				Remove( nOldPos );
/*N*/ 				delete pOver;
/*N*/ 				pOver = NULL;
/*N*/ 				if ( nOldPos )
/*N*/ 					nOldPos--;			// Seek richtig aufsetzen
/*N*/ 			}
/*N*/ 			bJoinedInput = TRUE;
/*N*/ 			Join( *p, TRUE );			// rekursiv!
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if ( bIsInList )
/*N*/ 		Seek( nOldPos );
/*N*/ 	else if ( !bJoinedInput )
/*N*/ 		Append( r );
/*N*/ }




/*N*/ BOOL ScRangeList::Store( SvStream& rStream ) const
/*N*/ {
/*N*/ 	BOOL bOk = TRUE;
/*N*/ 	ULONG nCount = Count();
/*N*/ 	ULONG nBytes = sizeof(UINT32) + nCount * sizeof(ScRange);
/*N*/ 	ScWriteHeader aHdr( rStream, nBytes );
/*N*/ 	rStream << (UINT32) nCount;
/*N*/ 	for ( ULONG j = 0; j < nCount && bOk; j++ )
/*N*/ 	{
/*N*/ 		rStream << *GetObject( j );
/*N*/ 		if( rStream.GetError() != SVSTREAM_OK )
/*N*/ 			bOk = FALSE;
/*N*/ 	}
/*N*/ 	return bOk;
/*N*/ }


/*N*/ BOOL ScRangeList::Load( SvStream& rStream, USHORT nVer )
/*N*/ {
/*N*/ 	BOOL bOk = TRUE;
/*N*/ 	ScReadHeader aHdr( rStream );
/*N*/ 	ScRange aRange;
/*N*/ 	UINT32 n;
/*N*/ 	rStream >> n;
/*N*/ 	ULONG nCount = n;
/*N*/ 	for ( ULONG j = 0; j < nCount && bOk; j++ )
/*N*/ 	{
/*N*/ 		rStream >> aRange;
/*N*/ 		Append( aRange );
/*N*/ 		if( rStream.GetError() != SVSTREAM_OK )
/*N*/ 			bOk = FALSE;
/*N*/ 	}
/*N*/ 	return bOk;
/*N*/ }


/*N*/ BOOL ScRangeList::UpdateReference( UpdateRefMode eUpdateRefMode,
/*N*/ 									ScDocument* pDoc, const ScRange& rWhere,
/*N*/ 									short nDx, short nDy, short nDz )
/*N*/ {
/*N*/ 	BOOL bChanged = FALSE;
/*N*/ 	if ( Count() )
/*N*/ 	{
/*N*/ 		USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
/*N*/ 		rWhere.GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
/*N*/ 		for ( ScRange* pR = First(); pR; pR = Next() )
/*N*/ 		{
/*N*/ 			USHORT theCol1, theRow1, theTab1, theCol2, theRow2, theTab2;
/*N*/ 			pR->GetVars( theCol1, theRow1, theTab1, theCol2, theRow2, theTab2 );
/*N*/ 			if ( ScRefUpdate::Update( pDoc, eUpdateRefMode,
/*N*/ 					nCol1, nRow1, nTab1, nCol2, nRow2, nTab2,
/*N*/ 					nDx, nDy, nDz,
/*N*/ 					theCol1, theRow1, theTab1, theCol2, theRow2, theTab2 )
/*N*/ 					!= UR_NOTHING )
/*N*/ 			{
/*?*/ 				bChanged = TRUE;
/*?*/ 				pR->aStart.Set( theCol1, theRow1, theTab1 );
/*?*/ 				pR->aEnd.Set( theCol2, theRow2, theTab2 );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bChanged;
/*N*/ }






/*N*/ ScRangeList::ScRangeList( const ScRangeList& rList )
/*N*/ {
/*N*/ 	ULONG nCount = rList.Count();
/*N*/ 	for ( ULONG j = 0; j < nCount; j++ )
/*N*/ 		Append( *rList.GetObject( j ) );
/*N*/ }




/*N*/ ScRangeList& ScRangeList::operator=(const ScRangeList& rList)
/*N*/ {
/*N*/ 	RemoveAll();
/*N*/ 
/*N*/ 	ULONG nCount = rList.Count();
/*N*/ 	for ( ULONG j = 0; j < nCount; j++ )
/*N*/ 		Append( *rList.GetObject( j ) );
/*N*/ 
/*N*/ 	return *this;
/*N*/ }








// === ScRangePairList ====================================================

/*N*/ ScRangePairList::~ScRangePairList()
/*N*/ {
/*N*/ 	for ( ScRangePair* pR = First(); pR; pR = Next() )
/*?*/ 		delete pR;
/*N*/ }


/*N*/ void ScRangePairList::Join( const ScRangePair& r, BOOL bIsInList )
/*N*/ {
/*N*/ 	if ( !Count() )
/*N*/ 	{
/*N*/ 		Append( r );
/*N*/ 		return ;
/*N*/ 	}
/*N*/ 	const ScRange& r1 = r.GetRange(0);
/*N*/ 	const ScRange& r2 = r.GetRange(1);
/*N*/ 	USHORT nCol1 = r1.aStart.Col();
/*N*/ 	USHORT nRow1 = r1.aStart.Row();
/*N*/ 	USHORT nTab1 = r1.aStart.Tab();
/*N*/ 	USHORT nCol2 = r1.aEnd.Col();
/*N*/ 	USHORT nRow2 = r1.aEnd.Row();
/*N*/ 	USHORT nTab2 = r1.aEnd.Tab();
/*N*/ 	ScRangePair* pOver = (ScRangePair*) &r;		// fies aber wahr wenn bInList
/*N*/ 	ULONG nOldPos;
/*N*/ 	if ( bIsInList )
/*N*/ 	{	// merken um ggbf. zu loeschen bzw. wiederherzustellen
/*N*/ 		nOldPos = GetPos( pOver );
/*N*/ 	}
/*N*/ 	BOOL bJoinedInput = FALSE;
/*N*/ 	for ( ScRangePair* p = First(); p && pOver; p = Next() )
/*N*/ 	{
/*N*/ 		if ( p == pOver )
/*N*/ 			continue;			// derselbe, weiter mit dem naechsten
/*N*/ 		BOOL bJoined = FALSE;
/*N*/ 		ScRange& rp1 = p->GetRange(0);
/*N*/ 		ScRange& rp2 = p->GetRange(1);
/*N*/ 		if ( rp2 == r2 )
/*N*/ 		{	// nur wenn Range2 gleich ist
/*N*/ 			if ( rp1.In( r1 ) )
/*N*/ 			{	// RangePair r in RangePair p enthalten oder identisch
/*N*/ 				if ( bIsInList )
/*N*/ 					bJoined = TRUE;		// weg mit RangePair r
/*N*/ 				else
/*N*/ 				{	// das war's dann
/*N*/ 					bJoinedInput = TRUE;	// nicht anhaengen
/*N*/ 					break;	// for
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else if ( r1.In( rp1 ) )
/*N*/ 			{	// RangePair p in RangePair r enthalten, r zum neuen RangePair machen
/*N*/ 				*p = r;
/*N*/ 				bJoined = TRUE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if ( !bJoined && rp1.aStart.Tab() == nTab1 && rp1.aEnd.Tab() == nTab2
/*N*/ 		  && rp2.aStart.Tab() == r2.aStart.Tab()
/*N*/ 		  && rp2.aEnd.Tab() == r2.aEnd.Tab() )
/*N*/ 		{	// 2D, Range2 muss genauso nebeneinander liegen wie Range1
/*N*/ 			if ( rp1.aStart.Col() == nCol1 && rp1.aEnd.Col() == nCol2
/*N*/ 			  && rp2.aStart.Col() == r2.aStart.Col()
/*N*/ 			  && rp2.aEnd.Col() == r2.aEnd.Col() )
/*N*/ 			{
/*N*/ 				if ( rp1.aStart.Row() == nRow2+1
/*N*/ 				  && rp2.aStart.Row() == r2.aEnd.Row()+1 )
/*N*/ 				{	// oben
/*N*/ 					rp1.aStart.SetRow( nRow1 );
/*N*/ 					rp2.aStart.SetRow( r2.aStart.Row() );
/*N*/ 					bJoined = TRUE;
/*N*/ 				}
/*N*/ 				else if ( rp1.aEnd.Row() == nRow1-1
/*N*/ 				  && rp2.aEnd.Row() == r2.aStart.Row()-1 )
/*N*/ 				{	// unten
/*N*/ 					rp1.aEnd.SetRow( nRow2 );
/*N*/ 					rp2.aEnd.SetRow( r2.aEnd.Row() );
/*N*/ 					bJoined = TRUE;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else if ( rp1.aStart.Row() == nRow1 && rp1.aEnd.Row() == nRow2
/*N*/ 			  && rp2.aStart.Row() == r2.aStart.Row()
/*N*/ 			  && rp2.aEnd.Row() == r2.aEnd.Row() )
/*N*/ 			{
/*N*/ 				if ( rp1.aStart.Col() == nCol2+1
/*N*/ 				  && rp2.aStart.Col() == r2.aEnd.Col()+1 )
/*N*/ 				{	// links
/*N*/ 					rp1.aStart.SetCol( nCol1 );
/*N*/ 					rp2.aStart.SetCol( r2.aStart.Col() );
/*N*/ 					bJoined = TRUE;
/*N*/ 				}
/*N*/ 				else if ( rp1.aEnd.Col() == nCol1-1
/*N*/ 				  && rp2.aEnd.Col() == r2.aEnd.Col()-1 )
/*N*/ 				{	// rechts
/*N*/ 					rp1.aEnd.SetCol( nCol2 );
/*N*/ 					rp2.aEnd.SetCol( r2.aEnd.Col() );
/*N*/ 					bJoined = TRUE;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if ( bJoined )
/*N*/ 		{
/*N*/ 			if ( bIsInList )
/*N*/ 			{	// innerhalb der Liste RangePair loeschen
/*N*/ 				Remove( nOldPos );
/*N*/ 				delete pOver;
/*N*/ 				pOver = NULL;
/*N*/ 				if ( nOldPos )
/*N*/ 					nOldPos--;			// Seek richtig aufsetzen
/*N*/ 			}
/*N*/ 			bJoinedInput = TRUE;
/*N*/ 			Join( *p, TRUE );			// rekursiv!
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if ( bIsInList )
/*N*/ 		Seek( nOldPos );
/*N*/ 	else if ( !bJoinedInput )
/*N*/ 		Append( r );
/*N*/ }




/*N*/ BOOL ScRangePairList::Store( SvStream& rStream ) const
/*N*/ {
/*N*/ 	BOOL bOk = TRUE;
/*N*/ 	ULONG nCount = Count();
/*N*/ 	ULONG nBytes = sizeof(UINT32) + nCount * sizeof(ScRangePair);
/*N*/ 	ScWriteHeader aHdr( rStream, nBytes );
/*N*/ 	rStream << (UINT32) nCount;
/*N*/ 	for ( ULONG j = 0; j < nCount && bOk; j++ )
/*N*/ 	{
/*N*/ 		rStream << *GetObject( j );
/*N*/ 		if( rStream.GetError() != SVSTREAM_OK )
/*N*/ 			bOk = FALSE;
/*N*/ 	}
/*N*/ 	return bOk;
/*N*/ }


/*N*/ BOOL ScRangePairList::Load( SvStream& rStream, USHORT nVer )
/*N*/ {
/*N*/ 	BOOL bOk = TRUE;
/*N*/ 	ScReadHeader aHdr( rStream );
/*N*/ 	ScRangePair aRangePair;
/*N*/ 	ScRange aRange;
/*N*/ 	UINT32 n;
/*N*/ 	rStream >> n;
/*N*/ 	ULONG nCount = n;
/*N*/ 	for ( ULONG j = 0; j < nCount && bOk; j++ )
/*N*/ 	{
/*N*/ 		if ( nVer < SC_COLROWNAME_RANGEPAIR )
/*N*/ 		{	// aus technical Beta 4.0 versuchen mit altem Verhalten zu uebernehmen
/*N*/ 			rStream >> aRange;
/*N*/ 			aRangePair.GetRange(0) = aRange;
/*N*/ 			ScRange& r = aRangePair.GetRange(1);
/*N*/ 			r = aRange;
/*N*/ 			USHORT nCol2 = aRange.aEnd.Col();
/*N*/ 			USHORT nRow2 = aRange.aEnd.Row();
/*N*/ 			if ( nCol2 - aRange.aStart.Col() >= nRow2 - aRange.aStart.Row() )
/*N*/ 			{	// ColNames
/*N*/ 				r.aStart.SetRow( (USHORT) Min( (ULONG)nRow2 + 1, (ULONG)MAXROW ) );
/*N*/ 				r.aEnd.SetRow( MAXROW );
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{	// RowNames
/*N*/ 				r.aStart.SetCol( (USHORT) Min( (ULONG)(nCol2 + 1), (ULONG)MAXCOL ) );
/*N*/ 				r.aEnd.SetCol( MAXCOL );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 			rStream >> aRangePair;
/*N*/ 		Append( aRangePair );
/*N*/ 		if( rStream.GetError() != SVSTREAM_OK )
/*N*/ 			bOk = FALSE;
/*N*/ 	}
/*N*/ 	return bOk;
/*N*/ }


/*N*/ BOOL ScRangePairList::UpdateReference( UpdateRefMode eUpdateRefMode,
/*N*/ 									ScDocument* pDoc, const ScRange& rWhere,
/*N*/ 									short nDx, short nDy, short nDz )
/*N*/ {
/*N*/ 	BOOL bChanged = FALSE;
/*N*/ 	if ( Count() )
/*N*/ 	{
/*?*/ 		USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
/*?*/ 		rWhere.GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
/*?*/ 		for ( ScRangePair* pR = First(); pR; pR = Next() )
/*?*/ 		{
/*?*/ 			for ( USHORT j=0; j<2; j++ )
/*?*/ 			{
/*?*/ 				ScRange& rRange = pR->GetRange(j);
/*?*/ 				USHORT theCol1, theRow1, theTab1, theCol2, theRow2, theTab2;
/*?*/ 				rRange.GetVars( theCol1, theRow1, theTab1, theCol2, theRow2, theTab2 );
/*?*/ 				if ( ScRefUpdate::Update( pDoc, eUpdateRefMode,
/*?*/ 						nCol1, nRow1, nTab1, nCol2, nRow2, nTab2,
/*?*/ 						nDx, nDy, nDz,
/*?*/ 						theCol1, theRow1, theTab1, theCol2, theRow2, theTab2 )
/*?*/ 						!= UR_NOTHING )
/*?*/ 				{
/*?*/ 					bChanged = TRUE;
/*?*/ 					rRange.aStart.Set( theCol1, theRow1, theTab1 );
/*?*/ 					rRange.aEnd.Set( theCol2, theRow2, theTab2 );
/*?*/ 				}
/*?*/ 			}
/*?*/ 		}
/*N*/ 	}
/*N*/ 	return bChanged;
/*N*/ }


/*N*/ ScRangePair* ScRangePairList::Find( const ScRange& rRange ) const
/*N*/ {
/*N*/ 	ULONG nCount = Count();
/*N*/ 	for ( ULONG j = 0; j < nCount; j++ )
/*N*/ 	{
/*N*/ 		ScRangePair* pR = GetObject( j );
/*N*/ 		if ( pR->GetRange(0) == rRange )
/*N*/ 			return pR;
/*N*/ 	}
/*N*/ 	return NULL;
/*N*/ }


/*N*/ ScRangePairList* ScRangePairList::Clone() const
/*N*/ {
/*N*/ 	ScRangePairList* pNew = new ScRangePairList;
/*N*/ 	ULONG nCount = Count();
/*N*/ 	for ( ULONG j = 0; j < nCount; j++ )
/*N*/ 	{
/*N*/ 		pNew->Append( *GetObject( j ) );
/*N*/ 	}
/*N*/ 	return pNew;
/*N*/ }


struct ScRangePairNameSort
{
    ScRangePair*	pPair;
    ScDocument*		pDoc;
};








}
