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

#ifdef _MSC_VER
#pragma hdrstop
#endif

// INCLUDE ---------------------------------------------------------------

#include <bf_svtools/zforlist.hxx>
#include <bf_sch/schdll.hxx>
#include <bf_sch/memchrt.hxx>
#include <float.h>				// DBL_MIN

#include "chartarr.hxx"
#include "document.hxx"
#include "rechead.hxx"
#include "globstr.hrc"
#include "cell.hxx"
#include "docoptio.hxx"
namespace binfilter {


// -----------------------------------------------------------------------

// static
/*N*/ void ScChartArray::CopySettings( SchMemChart& rDest, const SchMemChart& rSource )
/*N*/ {
/*N*/ 	rDest.SetMainTitle( rSource.GetMainTitle() );
/*N*/ 	rDest.SetSubTitle( rSource.GetSubTitle() );
/*N*/ 	rDest.SetXAxisTitle( rSource.GetXAxisTitle() );
/*N*/ 	rDest.SetYAxisTitle( rSource.GetYAxisTitle() );
/*N*/ 	rDest.SetZAxisTitle( rSource.GetZAxisTitle() );
/*N*/ 
/*N*/ 	const sal_Int32* pArr;
/*N*/ 	if ( rSource.GetRowCount() == rDest.GetRowCount() &&
/*N*/ 		 rSource.GetColCount() == rDest.GetColCount() )
/*N*/ 	{
/*N*/ 		//	don't copy column/row number formats here (are set in new MemChart object)
/*N*/ 
/*N*/ 		if ( (pArr = rSource.GetRowTranslation()) ) rDest.SetRowTranslation( pArr );
/*N*/ 		if ( (pArr = rSource.GetColTranslation()) ) rDest.SetColTranslation( pArr );
/*N*/ 		rDest.SetTranslation( rSource.GetTranslation() );
/*N*/ 	}
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ ScChartArray::ScChartArray( ScDocument* pDoc, const ScRangeListRef& rRangeList,
/*N*/ 					const String& rChartName ) :
/*N*/ 		aRangeListRef( rRangeList ),
/*N*/ 		aName( rChartName ),
/*N*/ 		pDocument( pDoc ),
/*N*/ 		pPositionMap( NULL ),
/*N*/ 		eGlue( SC_CHARTGLUE_NA ),
/*N*/ 		nStartCol(0),
/*N*/ 		nStartRow(0),
/*N*/ 		bColHeaders( FALSE ),
/*N*/ 		bRowHeaders( FALSE ),
/*N*/ 		bDummyUpperLeft( FALSE ),
/*N*/ 		bValid( TRUE )
/*N*/ {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if ( aRangeListRef.Is() )
/*N*/ }
/*N*/ 
/*N*/ ScChartArray::ScChartArray( const ScChartArray& rArr ) :
/*N*/ 		aRangeListRef( rArr.aRangeListRef ),
/*N*/ 		aName(rArr.aName),
/*N*/ 		pDocument(rArr.pDocument),
/*N*/ 		pPositionMap( NULL ),
/*N*/ 		eGlue(rArr.eGlue),
/*N*/ 		nStartCol(rArr.nStartCol),
/*N*/ 		nStartRow(rArr.nStartRow),
/*N*/ 		bColHeaders(rArr.bColHeaders),
/*N*/ 		bRowHeaders(rArr.bRowHeaders),
/*N*/ 		bDummyUpperLeft( rArr.bDummyUpperLeft ),
/*N*/ 		bValid(rArr.bValid)
/*N*/ {
/*N*/ }
/*N*/ 
/*N*/ ScChartArray::ScChartArray( ScDocument* pDoc, SvStream& rStream, ScMultipleReadHeader& rHdr ) :
/*N*/ 		pDocument( pDoc ),
/*N*/ 		pPositionMap( NULL ),
/*N*/ 		eGlue( SC_CHARTGLUE_NONE ),
/*N*/ 		bDummyUpperLeft( FALSE ),
/*N*/ 		bValid( TRUE )
/*N*/ {
/*N*/ 	USHORT nCol2, nRow2, nTable;
/*N*/ 
/*N*/ 	rHdr.StartEntry();
/*N*/ 
/*N*/ 	rStream >> nTable;
/*N*/ 	rStream >> nStartCol;
/*N*/ 	rStream >> nStartRow;
/*N*/ 	rStream >> nCol2;
/*N*/ 	rStream >> nRow2;
/*N*/ 	rStream.ReadByteString( aName, rStream.GetStreamCharSet() );
/*N*/ 	rStream >> bColHeaders;
/*N*/ 	rStream >> bRowHeaders;
/*N*/ 
/*N*/ 	rHdr.EndEntry();
/*N*/ 
/*N*/ 	SetRangeList( ScRange( nStartCol, nStartRow, nTable, nCol2, nRow2, nTable ) );
/*N*/ }

/*N*/ ScChartArray::ScChartArray( ScDocument* pDoc, const SchMemChart& rData ) :
/*N*/ 		pDocument( pDoc ),
/*N*/ 		pPositionMap( NULL )
/*N*/ {
/*N*/ 	BOOL bInitOk = bValid = FALSE;
/*N*/     const SchChartRange& rChartRange = rData.GetChartRange();
/*N*/     ::std::vector< SchCellRangeAddress >::const_iterator iRange =
/*N*/         rChartRange.maRanges.begin();
/*N*/     if ( iRange != rChartRange.maRanges.end() )
/*N*/ 	{	// new SO6 chart format
/*N*/ 		bValid = TRUE;
/*N*/         bColHeaders = rChartRange.mbFirstRowContainsLabels;
/*N*/         bRowHeaders = rChartRange.mbFirstColumnContainsLabels;
/*N*/ 		aRangeListRef = new ScRangeList;
/*N*/         for ( ; iRange != rChartRange.maRanges.end(); ++iRange )
/*N*/         {
/*N*/             const SchSingleCell& rAddr1 = (*iRange).maUpperLeft.maCells[0];
/*N*/             const SchSingleCell& rAddr2 = (*iRange).maLowerRight.maCells[0];
/*N*/             USHORT nTab = (USHORT) (*iRange).mnTableNumber;
/*N*/             ScRange aRange(
/*N*/                 (USHORT) rAddr1.mnColumn, (USHORT) rAddr1.mnRow, nTab,
/*N*/                 (USHORT) rAddr2.mnColumn, (USHORT) rAddr2.mnRow, nTab );
/*N*/             aRangeListRef->Append( aRange );
/*N*/         }
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{	// old SO5 chart format
/*N*/         //! A similar routine is implemented in
/*N*/         //! SchMemChart::ConvertChartRangeForCalc() for OldToNew. If anything
/*N*/         //! is changed here it propably must be changed there too!
/*N*/ 		const sal_Unicode cTok = ';';
/*N*/ 		xub_StrLen nToken;
/*N*/ 		String aPos = ((SchMemChart&)rData).SomeData1();
/*N*/ 		if ( (nToken = aPos.GetTokenCount( cTok )) >= 5)
/*N*/ 		{
/*N*/ 			String aOpt = ((SchMemChart&)rData).SomeData2();
/*N*/ 			xub_StrLen nOptToken = aOpt.GetTokenCount( cTok );
/*N*/             BOOL bNewChart = (nOptToken >= 4);      // as of 341/342
/*N*/ 			USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
/*N*/ 			xub_StrLen nInd = 0;
/*N*/ 			for ( xub_StrLen j=0; j < nToken; j+=5 )
/*N*/ 			{
/*N*/ 				xub_StrLen nInd2 = nInd;
/*N*/ 				nTab1 = (USHORT) aPos.GetToken( 0, cTok, nInd ).ToInt32();
/*N*/                 // To make old versions (<341/342) skip it, the token separator
/*N*/                 // is a ','
/*N*/ 				if ( bNewChart )
/*?*/ 					nTab2 = (USHORT) aPos.GetToken( 1, ',', nInd2 ).ToInt32();
/*N*/ 				else
/*N*/ 					nTab2 = nTab1;
/*N*/ 				nCol1 = (USHORT) aPos.GetToken( 0, cTok, nInd ).ToInt32();
/*N*/ 				nRow1 = (USHORT) aPos.GetToken( 0, cTok, nInd ).ToInt32();
/*N*/ 				nCol2 = (USHORT) aPos.GetToken( 0, cTok, nInd ).ToInt32();
/*N*/ 				nRow2 = (USHORT) aPos.GetToken( 0, cTok, nInd ).ToInt32();
/*N*/ 				AddToRangeList( ScRange( nCol1, nRow1, nTab1,
/*N*/ 					nCol2, nRow2, nTab2 ) );
/*N*/ 			}
/*N*/ 			bValid = TRUE;
/*N*/ 
/*N*/ 			if (aOpt.Len() >= 2)
/*N*/ 			{
/*N*/ 				bColHeaders = ( aOpt.GetChar(0) != '0' );
/*N*/ 				bRowHeaders = ( aOpt.GetChar(1) != '0' );
/*N*/ 				if ( aOpt.Len() >= 3 )
/*N*/ 				{
/*N*/ 					if ( bNewChart )
/*N*/ 					{
/*N*/ 						bDummyUpperLeft = ( aOpt.GetChar(2) != '0' );
/*N*/ 						xub_StrLen nInd = 4;	// 111;
/*N*/ 						eGlue = (ScChartGlue) aOpt.GetToken( 0, cTok, nInd ).ToInt32();
/*N*/ 						nStartCol = (USHORT) aOpt.GetToken( 0, cTok, nInd ).ToInt32();
/*N*/ 						nStartRow = (USHORT) aOpt.GetToken( 0, cTok, nInd ).ToInt32();
/*N*/ 						bInitOk = TRUE;
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/ 				bColHeaders = bRowHeaders = FALSE;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*?*/ 			SetRangeList( ScRange() );
/*?*/ 			bColHeaders = bRowHeaders = bValid = FALSE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if ( !bInitOk )
/*N*/ 	{	// muessen in GlueState neu berechnet werden
/*N*/ 		InvalidateGlue();
/*N*/ 		nStartCol = nStartRow = 0;
/*N*/ 		bDummyUpperLeft = FALSE;
/*N*/ 	}
/*N*/ }

/*N*/ ScChartArray::~ScChartArray()
/*N*/ {
/*N*/ 	delete pPositionMap;
/*N*/ }




/*N*/ void ScChartArray::SetRangeList( const ScRange& rRange )
/*N*/ {
/*N*/ 	aRangeListRef = new ScRangeList;
/*N*/ 	aRangeListRef->Append( rRange );
/*N*/ 	InvalidateGlue();
/*N*/ }

/*N*/ void ScChartArray::AddToRangeList( const ScRange& rRange )
/*N*/ {
/*N*/ 	if ( aRangeListRef.Is() )
/*N*/ 		aRangeListRef->Append( rRange );
/*N*/ 	else
/*N*/ 		SetRangeList( rRange );
/*N*/ 	InvalidateGlue();
/*N*/ }

/*N*/ void ScChartArray::AddToRangeList( const ScRangeListRef& rAdd )
/*N*/ {
/*N*/ 	if ( aRangeListRef.Is() )
/*N*/ 	{
/*N*/ 		ULONG nCount = rAdd->Count();
/*N*/ 		for (ULONG i=0; i<nCount; i++)
/*N*/ 			aRangeListRef->Join( *rAdd->GetObject(i) );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		SetRangeList( rAdd );
/*N*/ 	InvalidateGlue();
/*N*/ }

/*N*/ void ScChartArray::GlueState()
/*N*/ {
/*N*/ 	if ( eGlue != SC_CHARTGLUE_NA )
/*N*/ 		return;
/*N*/ 	bDummyUpperLeft = FALSE;
/*N*/ 	ScRangePtr pR;
/*N*/ 	if ( aRangeListRef->Count() <= 1 )
/*N*/ 	{
/*?*/ 		if ( pR = aRangeListRef->First() )
/*?*/ 		{
/*?*/ 			if ( pR->aStart.Tab() == pR->aEnd.Tab() )
/*?*/ 				eGlue = SC_CHARTGLUE_NONE;
/*?*/ 			else
/*?*/ 				eGlue = SC_CHARTGLUE_COLS;	// mehrere Tabellen spaltenweise
/*?*/ 			nStartCol = pR->aStart.Col();
/*?*/ 			nStartRow = pR->aStart.Row();
/*?*/ 		}
/*?*/ 		else
/*?*/ 		{
/*?*/ 			InvalidateGlue();
/*?*/ 			nStartCol = nStartRow = 0;
/*?*/ 		}
/*?*/ 		return;
/*N*/ 	}
/*N*/ 	ULONG nOldPos = aRangeListRef->GetCurPos();
/*N*/ 
/*N*/ 	pR = aRangeListRef->First();
/*N*/ 	nStartCol = pR->aStart.Col();
/*N*/ 	nStartRow = pR->aStart.Row();
/*N*/ 	USHORT nMaxCols, nMaxRows, nEndCol, nEndRow;
/*N*/ 	nMaxCols = nMaxRows = nEndCol = nEndRow = 0;
/*N*/ 	do
/*N*/ 	{	// umspannenden Bereich etc. feststellen
/*N*/ 		USHORT nTmp, n1, n2;
/*N*/ 		if ( (n1 = pR->aStart.Col()) < nStartCol )
/*N*/ 			nStartCol = n1;
/*N*/ 		if ( (n2 = pR->aEnd.Col()) > nEndCol )
/*N*/ 			nEndCol = n2;
/*N*/ 		if ( (nTmp = n2 - n1 + 1) > nMaxCols )
/*N*/ 			nMaxCols = nTmp;
/*N*/ 		if ( (n1 = pR->aStart.Row()) < nStartRow )
/*N*/ 			nStartRow = n1;
/*N*/ 		if ( (n2 = pR->aEnd.Row()) > nEndRow )
/*N*/ 			nEndRow = n2;
/*N*/ 		if ( (nTmp = n2 - n1 + 1) > nMaxRows )
/*N*/ 			nMaxRows = nTmp;
/*N*/ 	} while ( pR = aRangeListRef->Next() );
/*N*/ 	USHORT nC = nEndCol - nStartCol + 1;
/*N*/ 	if ( nC == 1 )
/*N*/ 	{
/*N*/ 		eGlue = SC_CHARTGLUE_ROWS;
/*N*/ 		return;
/*N*/ 	}
/*N*/ 	USHORT nR = nEndRow - nStartRow + 1;
/*N*/ 	if ( nR == 1 )
/*N*/ 	{
/*N*/ 		eGlue = SC_CHARTGLUE_COLS;
/*N*/ 		return;
/*N*/ 	}
/*N*/ 	ULONG nCR = (ULONG)nC * nR;
/*N*/ //2do:
/*
    Erstmal simpel ohne Bitmaskiererei, maximal koennten so 8MB alloziert
    werden (256 Cols mal 32000 Rows), das liesse sich mit 2 Bit je Eintrag
    auf 2MB reduzieren, andererseits ist es so schneller.
    Weitere Platz-Optimierung waere, in dem Array nur die wirklich benutzten
    Zeilen/Spalten abzulegen, wuerde aber ein weiteres durchlaufen der
    RangeList und indirekten Zugriff auf das Array bedeuten.
 */
/*N*/ 	const BYTE nHole = 0;
/*N*/ 	const BYTE nOccu = 1;
/*N*/ 	const BYTE nFree = 2;
/*N*/ 	const BYTE nGlue = 3;
/*N*/ #ifdef WIN
/*?*/ 	// we hate 16bit, don't we?
/*?*/ 	BYTE huge* p;
/*?*/ 	BYTE huge* pA = (BYTE huge*) SvMemAlloc( nCR );
/*?*/ 	if ( nCR > (ULONG)((USHORT)~0) )
/*?*/ 	{	// in 32k Bloecken initialisieren
/*?*/ 		ULONG j;
/*?*/ 		for ( j=0; j<nCR; j+=0x8000 )
/*?*/ 		{
/*?*/ 			memset( pA+j, nHole, Min( (ULONG)0x8000, nCR-j ) );
/*?*/ 		}
/*?*/ 	}
/*?*/ 	else
/*?*/ 		memset( pA, nHole, nCR * sizeof(BYTE) );
/*N*/ #else
/*N*/ 	BYTE* p;
/*N*/ 	BYTE* pA = new BYTE[ nCR ];
/*N*/ 	memset( pA, 0, nCR * sizeof(BYTE) );
/*N*/ #endif
/*N*/ 
/*N*/ 	USHORT nCol, nRow, nCol1, nRow1, nCol2, nRow2;
/*N*/ 	for ( pR = aRangeListRef->First(); pR; pR = aRangeListRef->Next() )
/*N*/ 	{	// Selektionen 2D als belegt markieren
/*N*/ 		nCol1 = pR->aStart.Col() - nStartCol;
/*N*/ 		nCol2 = pR->aEnd.Col() - nStartCol;
/*N*/ 		nRow1 = pR->aStart.Row() - nStartRow;
/*N*/ 		nRow2 = pR->aEnd.Row() - nStartRow;
/*N*/ 		for ( nCol = nCol1; nCol <= nCol2; nCol++ )
/*N*/ 		{
/*N*/ 			p = pA + (ULONG)nCol * nR + nRow1;
/*N*/ 			for ( nRow = nRow1; nRow <= nRow2; nRow++, p++ )
/*N*/ 				*p = nOccu;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	BOOL bGlue = TRUE;
/*N*/ 
/*N*/ 	BOOL bGlueCols = FALSE;
/*N*/ 	for ( nCol = 0; bGlue && nCol < nC; nCol++ )
/*N*/ 	{	// Spalten probieren durchzugehen und als frei markieren
/*N*/ 		p = pA + (ULONG)nCol * nR;
/*N*/ 		for ( nRow = 0; bGlue && nRow < nR; nRow++, p++ )
/*N*/ 		{
/*N*/ 			if ( *p == nOccu )
/*N*/ 			{	// Wenn einer mittendrin liegt ist keine Zusammenfassung
/*N*/ 				// moeglich. Am Rand koennte ok sein, wenn in dieser Spalte
/*N*/ 				// in jeder belegten Zeile einer belegt ist.
/*N*/ 				if ( nRow > 0 && nCol > 0 )
/*N*/ 					bGlue = FALSE;		// nCol==0 kann DummyUpperLeft sein
/*N*/ 				else
/*N*/ 					nRow = nR;
/*N*/ 			}
/*N*/ 			else
/*N*/ 				*p = nFree;
/*N*/ 		}
/*N*/ 		if ( bGlue && *(p = (pA + ((((ULONG)nCol+1) * nR) - 1))) == nFree )
/*N*/ 		{	// Spalte als komplett frei markieren
/*N*/ 			*p = nGlue;
/*N*/ 			bGlueCols = TRUE;		// mindestens eine freie Spalte
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	BOOL bGlueRows = FALSE;
/*N*/ 	for ( nRow = 0; bGlue && nRow < nR; nRow++ )
/*N*/ 	{	// Zeilen probieren durchzugehen und als frei markieren
/*N*/ 		p = pA + nRow;
/*N*/ 		for ( nCol = 0; bGlue && nCol < nC; nCol++, p+=nR )
/*N*/ 		{
/*N*/ 			if ( *p == nOccu )
/*N*/ 			{
/*N*/ 				if ( nCol > 0 && nRow > 0 )
/*N*/ 					bGlue = FALSE;		// nRow==0 kann DummyUpperLeft sein
/*N*/ 				else
/*N*/ 					nCol = nC;
/*N*/ 			}
/*N*/ 			else
/*N*/ 				*p = nFree;
/*N*/ 		}
/*N*/ 		if ( bGlue && *(p = (pA + ((((ULONG)nC-1) * nR) + nRow))) == nFree )
/*N*/ 		{	// Zeile als komplett frei markieren
/*N*/ 			*p = nGlue;
/*N*/ 			bGlueRows = TRUE;		// mindestens eine freie Zeile
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// n=1: die linke obere Ecke koennte bei Beschriftung automagisch
/*N*/ 	// hinzugezogen werden
/*N*/ 	p = pA + 1;
/*N*/ 	for ( ULONG n = 1; bGlue && n < nCR; n++, p++ )
/*N*/ 	{	// ein unberuehrtes Feld heisst, dass es weder spaltenweise noch
/*N*/ 		// zeilenweise zu erreichen war, also nichts zusamenzufassen
/*N*/ 		if ( *p == nHole )
/*N*/ 			bGlue = FALSE;
/*N*/ 	}
/*N*/ 	if ( bGlue )
/*N*/ 	{
/*N*/ 		if ( bGlueCols && bGlueRows )
/*N*/ 			eGlue = SC_CHARTGLUE_BOTH;
/*N*/ 		else if ( bGlueRows )
/*N*/ 			eGlue = SC_CHARTGLUE_ROWS;
/*N*/ 		else
/*N*/ 			eGlue = SC_CHARTGLUE_COLS;
/*N*/ 		if ( *pA != nOccu )
/*N*/ 			bDummyUpperLeft = TRUE;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		eGlue = SC_CHARTGLUE_NONE;
/*N*/ 	}
/*N*/ 
/*N*/ #ifdef WIN
/*?*/ 	SvMemFree( pA );
/*N*/ #else
/*N*/ 	delete [] pA;
/*N*/ #endif
/*N*/ }


#ifdef _MSC_VER
#pragma optimize("",off)
#endif

/*N*/ SchMemChart* ScChartArray::CreateMemChart()
/*N*/ {
/*N*/ 	ULONG nCount = aRangeListRef->Count();
/*N*/ 	if ( nCount > 1 )
/*N*/ 		return CreateMemChartMulti();
/*N*/ 	else if ( nCount == 1 )
/*N*/ 	{
/*N*/ 		ScRange* pR = aRangeListRef->First();
/*N*/ 		if ( pR->aStart.Tab() != pR->aEnd.Tab() )
/*?*/ 			return CreateMemChartMulti();
/*N*/ 		else
/*N*/ 			return CreateMemChartSingle();
/*N*/ 	}
/*N*/ 	else
/*?*/ 		return CreateMemChartMulti();	// kann 0 Range besser ab als Single
/*N*/ }

/*N*/ SchMemChart* ScChartArray::CreateMemChartSingle()
/*N*/ {
/*N*/ 	USHORT i,nCol,nRow;
/*N*/ 
/*N*/ 		//
/*N*/ 		//	wirkliche Groesse (ohne versteckte Zeilen/Spalten)
/*N*/ 		//
/*N*/ 
/*N*/ 	USHORT nColAdd = bRowHeaders ? 1 : 0;
/*N*/ 	USHORT nRowAdd = bColHeaders ? 1 : 0;
/*N*/ 
/*N*/ 	USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
/*N*/ 	aRangeListRef->First()->GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
/*N*/ 
/*N*/ 	USHORT nStrCol = nCol1;		// fuer Beschriftung merken
/*N*/ 	USHORT nStrRow = nRow1;
/*N*/ 	// Beschriftungen auch nach HiddenCols finden
/*N*/ 	while ( (pDocument->GetColFlags( nCol1, nTab1) & CR_HIDDEN) != 0 )
/*N*/ 		nCol1++;
/*N*/ 	while ( (pDocument->GetRowFlags( nRow1, nTab1) & CR_HIDDEN) != 0 )
/*N*/ 		nRow1++;
/*N*/ 	// falls alles hidden ist, bleibt die Beschriftung am Anfang
/*N*/ 	if ( nCol1 <= nCol2 )
/*N*/ 	{
/*N*/ 		nStrCol = nCol1;
/*N*/ 		nCol1 += nColAdd;
/*N*/ 	}
/*N*/ 	if ( nRow1 <= nRow2 )
/*N*/ 	{
/*N*/ 		nStrRow = nRow1;
/*N*/ 		nRow1 += nRowAdd;
/*N*/ 	}
/*N*/ 
/*N*/ 	USHORT nTotalCols = ( nCol1 <= nCol2 ? nCol2 - nCol1 + 1 : 0 );
/*N*/ 	USHORT* pCols = new USHORT[nTotalCols ? nTotalCols : 1];
/*N*/ 	USHORT nColCount = 0;
/*N*/ 	for (i=0; i<nTotalCols; i++)
/*N*/ 		if ((pDocument->GetColFlags(nCol1+i,nTab1)&CR_HIDDEN)==0)
/*N*/ 			pCols[nColCount++] = nCol1+i;
/*N*/ 
/*N*/ 	USHORT nTotalRows = ( nRow1 <= nRow2 ? nRow2 - nRow1 + 1 : 0 );
/*N*/ 	USHORT* pRows = new USHORT[nTotalRows ? nTotalRows : 1];
/*N*/ 	USHORT nRowCount = 0;
/*N*/ 	for (i=0; i<nTotalRows; i++)
/*N*/ 		if ((pDocument->GetRowFlags(nRow1+i,nTab1)&CR_HIDDEN)==0)
/*N*/ 			pRows[nRowCount++] = nRow1+i;
/*N*/ 
/*N*/ 	BOOL bValidData = TRUE;
/*N*/ 	if ( !nColCount )
/*N*/ 	{
/*N*/ 		bValidData = FALSE;
/*N*/ 		nColCount = 1;
/*N*/ 		pCols[0] = nStrCol;
/*N*/ 	}
/*N*/ 	if ( !nRowCount )
/*N*/ 	{
/*N*/ 		bValidData = FALSE;
/*N*/ 		nRowCount = 1;
/*N*/ 		pRows[0] = nStrRow;
/*N*/ 	}
/*N*/ 
/*N*/ 		//
/*N*/ 		//	Daten
/*N*/ 		//
/*N*/ 
/*N*/ 	SchMemChart* pMemChart = SchDLL::NewMemChart( nColCount, nRowCount );
/*N*/ 	if (pMemChart)
/*N*/ 	{
/*N*/ 		SvNumberFormatter* pFormatter = pDocument->GetFormatTable();
/*N*/ 		pMemChart->SetNumberFormatter( pFormatter );
/*N*/ 		if ( bValidData )
/*N*/ 		{
/*N*/ 			BOOL bCalcAsShown = pDocument->GetDocOptions().IsCalcAsShown();
/*N*/ 			ScBaseCell* pCell;
/*N*/ 			for (nCol=0; nCol<nColCount; nCol++)
/*N*/ 			{
/*N*/ 				for (nRow=0; nRow<nRowCount; nRow++)
/*N*/ 				{
/*N*/ 					double nVal = DBL_MIN;		// Hack fuer Chart, um leere Zellen zu erkennen
/*N*/ 
/*N*/ 					pDocument->GetCell( pCols[nCol], pRows[nRow], nTab1, pCell );
/*N*/ 					if (pCell)
/*N*/ 					{
/*N*/ 						CellType eType = pCell->GetCellType();
/*N*/ 						if (eType == CELLTYPE_VALUE)
/*N*/ 						{
/*?*/ 							nVal = ((ScValueCell*)pCell)->GetValue();
/*?*/ 							if ( bCalcAsShown && nVal != 0.0 )
/*?*/ 							{
/*?*/ 								sal_uInt32 nFormat;
/*?*/ 								pDocument->GetNumberFormat( pCols[nCol],
/*?*/ 									pRows[nRow], nTab1, nFormat );
/*?*/ 								nVal = pDocument->RoundValueAsShown( nVal, nFormat );
/*?*/ 							}
/*N*/ 						}
/*N*/ 						else if (eType == CELLTYPE_FORMULA)
/*N*/ 						{
/*N*/ 							ScFormulaCell* pFCell = (ScFormulaCell*)pCell;
/*N*/ 							if ( (pFCell->GetErrCode() == 0) && pFCell->IsValue() )
/*N*/ 								nVal = pFCell->GetValue();
/*N*/ 						}
/*N*/ 					}
/*N*/ 					pMemChart->SetData(nCol, nRow, nVal);
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*?*/ 			//!	Flag, dass Daten ungueltig ??
/*?*/ 
/*?*/ 			for (nCol=0; nCol<nColCount; nCol++)
/*?*/ 				for (nRow=0; nRow<nRowCount; nRow++)
/*?*/ 					pMemChart->SetData( nCol, nRow, DBL_MIN );
/*N*/ 		}
/*N*/ 
/*N*/ 		//
/*N*/ 		//	Spalten-Header
/*N*/ 		//
/*N*/ 
/*N*/ 		for (nCol=0; nCol<nColCount; nCol++)
/*N*/ 		{
/*N*/             String aString, aColStr;
/*N*/ 			if (bColHeaders)
/*N*/ 				pDocument->GetString( pCols[nCol], nStrRow, nTab1, aString );
/*N*/ 			if ( !aString.Len() )
/*N*/ 			{
/*N*/ 				aString = ScGlobal::GetRscString(STR_COLUMN);
/*N*/ 				aString += ' ';
/*N*/ //                aString += String::CreateFromInt32( pCols[nCol]+1 );
/*N*/                 ScAddress aPos( pCols[ nCol ], 0, 0 );
/*N*/                 aPos.Format( aColStr, SCA_VALID_COL, NULL );
/*N*/                 aString += aColStr;
/*N*/ 			}
/*N*/ 			pMemChart->SetColText(nCol, aString);
/*N*/ 
/*N*/ 			ULONG nNumberAttr = pDocument->GetNumberFormat( ScAddress(
/*N*/ 											pCols[nCol], nRow1, nTab1 ) );
/*N*/ 			pMemChart->SetNumFormatIdCol( nCol, nNumberAttr );
/*N*/ 		}
/*N*/ 
/*N*/ 		//
/*N*/ 		//	Zeilen-Header
/*N*/ 		//
/*N*/ 
/*N*/ 		for (nRow=0; nRow<nRowCount; nRow++)
/*N*/ 		{
/*N*/ 			String aString;
/*N*/ 			if (bRowHeaders)
/*N*/ 			{
/*N*/ 				ScAddress aAddr( nStrCol, pRows[nRow], nTab1 );
/*N*/ 				pDocument->GetString( nStrCol, pRows[nRow], nTab1, aString );
/*N*/ 			}
/*N*/ 			if ( !aString.Len() )
/*N*/ 			{
/*N*/ 				aString = ScGlobal::GetRscString(STR_ROW);
/*N*/ 				aString += ' ';
/*N*/ 				aString += String::CreateFromInt32( pRows[nRow]+1 );
/*N*/ 			}
/*N*/ 			pMemChart->SetRowText(nRow, aString);
/*N*/ 
/*N*/ 			ULONG nNumberAttr = pDocument->GetNumberFormat( ScAddress(
/*N*/ 											nCol1, pRows[nRow], nTab1 ) );
/*N*/ 			pMemChart->SetNumFormatIdRow( nRow, nNumberAttr );
/*N*/ 		}
/*N*/ 
/*N*/ 		//
/*N*/ 		//  Titel
/*N*/ 		//
/*N*/ 
/*N*/ 		pMemChart->SetMainTitle(ScGlobal::GetRscString(STR_CHART_MAINTITLE));
/*N*/ 		pMemChart->SetSubTitle(ScGlobal::GetRscString(STR_CHART_SUBTITLE));
/*N*/ 		pMemChart->SetXAxisTitle(ScGlobal::GetRscString(STR_CHART_XTITLE));
/*N*/ 		pMemChart->SetYAxisTitle(ScGlobal::GetRscString(STR_CHART_YTITLE));
/*N*/ 		pMemChart->SetZAxisTitle(ScGlobal::GetRscString(STR_CHART_ZTITLE));
/*N*/ 
/*N*/ 		//
/*N*/ 		//	Zahlen-Typ
/*N*/ 		//
/*N*/ 
/*N*/ 		ULONG nNumberAttr = pDocument->GetNumberFormat( ScAddress(
/*N*/ 										nCol1, nRow1, nTab1 ) );
/*N*/ 		if (pFormatter)
/*N*/ 			pMemChart->SetDataType(pFormatter->GetType( nNumberAttr ));
/*N*/ 
/*N*/ 		//
/*N*/ 		//	Parameter-Strings
/*N*/ 		//
/*N*/ 
/*N*/         SetExtraStrings( *pMemChart );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		DBG_ERROR("SchDLL::NewMemChart gibt 0 zurueck!");
/*N*/ 
/*N*/ 		//	Aufraeumen
/*N*/ 
/*N*/ 	delete[] pRows;
/*N*/ 	delete[] pCols;
/*N*/ 
/*N*/ 	return pMemChart;
/*N*/ }

/*N*/ SchMemChart* ScChartArray::CreateMemChartMulti()
/*N*/ {
/*N*/ 	CreatePositionMap();
/*N*/ 	USHORT nColCount = pPositionMap->GetColCount();
/*N*/ 	USHORT nRowCount = pPositionMap->GetRowCount();
/*N*/ 
/*N*/ 	USHORT nCol, nRow;
/*N*/ 
/*N*/ 	//
/*N*/ 	//	Daten
/*N*/ 	//
/*N*/ 
/*N*/ 	SchMemChart* pMemChart = SchDLL::NewMemChart( nColCount, nRowCount );
/*N*/ 	if (pMemChart)
/*N*/ 	{
/*N*/ 		pMemChart->SetNumberFormatter( pDocument->GetFormatTable() );
/*N*/ 		BOOL bCalcAsShown = pDocument->GetDocOptions().IsCalcAsShown();
/*N*/ 		ULONG nIndex = 0;
/*N*/ 		for ( nCol = 0; nCol < nColCount; nCol++ )
/*N*/ 		{
/*N*/ 			for ( nRow = 0; nRow < nRowCount; nRow++, nIndex++ )
/*N*/ 			{
/*N*/ 				double nVal = DBL_MIN;		// Hack fuer Chart, um leere Zellen zu erkennen
/*N*/ 				const ScAddress* pPos = pPositionMap->GetPosition( nIndex );
/*N*/ 				if ( pPos )
/*N*/ 				{	// sonst: Luecke
/*N*/ 					ScBaseCell* pCell = pDocument->GetCell( *pPos );
/*N*/ 					if (pCell)
/*N*/ 					{
/*N*/ 						CellType eType = pCell->GetCellType();
/*N*/ 						if (eType == CELLTYPE_VALUE)
/*N*/ 						{
/*?*/ 							nVal = ((ScValueCell*)pCell)->GetValue();
/*?*/ 							if ( bCalcAsShown && nVal != 0.0 )
/*?*/ 							{
/*?*/ 								ULONG nFormat = pDocument->GetNumberFormat( *pPos );
/*?*/ 								nVal = pDocument->RoundValueAsShown( nVal, nFormat );
/*?*/ 							}
/*N*/ 						}
/*N*/ 						else if (eType == CELLTYPE_FORMULA)
/*N*/ 						{
/*N*/ 							ScFormulaCell* pFCell = (ScFormulaCell*)pCell;
/*N*/ 							if ( (pFCell->GetErrCode() == 0) && pFCell->IsValue() )
/*N*/ 								nVal = pFCell->GetValue();
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 				pMemChart->SetData(nCol, nRow, nVal);
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ //2do: Beschriftung bei Luecken
/*N*/ 
/*N*/ 		//
/*N*/ 		//	Spalten-Header
/*N*/ 		//
/*N*/ 
/*N*/ 		USHORT nPosCol = 0;
/*N*/ 		for ( nCol = 0; nCol < nColCount; nCol++ )
/*N*/ 		{
/*N*/             String aString, aColStr;
/*N*/ 			const ScAddress* pPos = pPositionMap->GetColHeaderPosition( nCol );
/*N*/ 			if ( bColHeaders && pPos )
/*N*/ 				pDocument->GetString(
/*N*/ 					pPos->Col(), pPos->Row(), pPos->Tab(), aString );
/*N*/ 			if ( !aString.Len() )
/*N*/ 			{
/*?*/ 				aString = ScGlobal::GetRscString(STR_COLUMN);
/*?*/ 				aString += ' ';
/*?*/ 				if ( pPos )
/*?*/ 					nPosCol = pPos->Col() + 1;
/*?*/ 				else
/*?*/ 					nPosCol++;
/*?*/                 ScAddress aPos( nPosCol - 1, 0, 0 );
/*?*/                 aPos.Format( aColStr, SCA_VALID_COL, NULL );
/*?*/ //                aString += String::CreateFromInt32( nPosCol );
/*?*/                 aString += aColStr;
/*N*/ 			}
/*N*/ 			pMemChart->SetColText(nCol, aString);
/*N*/ 
/*N*/ 			ULONG nNumberAttr = 0;
/*N*/ 			pPos = pPositionMap->GetPosition( nCol, 0 );
/*N*/ 			if ( pPos )
/*N*/ 				nNumberAttr = pDocument->GetNumberFormat( *pPos );
/*N*/ 			pMemChart->SetNumFormatIdCol( nCol, nNumberAttr );
/*N*/ 		}
/*N*/ 
/*N*/ 		//
/*N*/ 		//	Zeilen-Header
/*N*/ 		//
/*N*/ 
/*N*/ 		USHORT nPosRow = 0;
/*N*/ 		for ( nRow = 0; nRow < nRowCount; nRow++ )
/*N*/ 		{
/*N*/ 			String aString;
/*N*/ 			const ScAddress* pPos = pPositionMap->GetRowHeaderPosition( nRow );
/*N*/ 			if ( bRowHeaders && pPos )
/*N*/ 			{
/*N*/ 				pDocument->GetString(
/*N*/ 					pPos->Col(), pPos->Row(), pPos->Tab(), aString );
/*N*/ 			}
/*N*/ 			if ( !aString.Len() )
/*N*/ 			{
/*?*/ 				aString = ScGlobal::GetRscString(STR_ROW);
/*?*/ 				aString += ' ';
/*?*/ 				if ( pPos )
/*?*/ 					nPosRow = pPos->Row() + 1;
/*?*/ 				else
/*?*/ 					nPosRow++;
/*?*/ 				aString += String::CreateFromInt32( nPosRow );
/*N*/ 			}
/*N*/ 			pMemChart->SetRowText(nRow, aString);
/*N*/ 
/*N*/ 			ULONG nNumberAttr = 0;
/*N*/ 			pPos = pPositionMap->GetPosition( 0, nRow );
/*N*/ 			if ( pPos )
/*N*/ 				nNumberAttr = pDocument->GetNumberFormat( *pPos );
/*N*/ 			pMemChart->SetNumFormatIdRow( nRow, nNumberAttr );
/*N*/ 		}
/*N*/ 
/*N*/ 		//
/*N*/ 		//  Titel
/*N*/ 		//
/*N*/ 
/*N*/ 		pMemChart->SetMainTitle(ScGlobal::GetRscString(STR_CHART_MAINTITLE));
/*N*/ 		pMemChart->SetSubTitle(ScGlobal::GetRscString(STR_CHART_SUBTITLE));
/*N*/ 		pMemChart->SetXAxisTitle(ScGlobal::GetRscString(STR_CHART_XTITLE));
/*N*/ 		pMemChart->SetYAxisTitle(ScGlobal::GetRscString(STR_CHART_YTITLE));
/*N*/ 		pMemChart->SetZAxisTitle(ScGlobal::GetRscString(STR_CHART_ZTITLE));
/*N*/ 
/*N*/ 		//
/*N*/ 		//	Zahlen-Typ
/*N*/ 		//
/*N*/ 
/*N*/ 		SvNumberFormatter* pFormatter = pDocument->GetFormatTable();
/*N*/ 		if (pFormatter)
/*N*/ 		{
/*N*/ 			ULONG nIndex = 0;
/*N*/ 			ULONG nCount = pPositionMap->GetCount();
/*N*/ 			const ScAddress* pPos;
/*N*/ 			do
/*N*/ 			{
/*N*/ 				pPos = pPositionMap->GetPosition( nIndex );
/*N*/ 			} while ( !pPos && ++nIndex < nCount );
/*N*/ 			ULONG nFormat = ( pPos ? pDocument->GetNumberFormat( *pPos ) : 0 );
/*N*/ 			pMemChart->SetDataType( pFormatter->GetType( nFormat ) );
/*N*/ 		}
/*N*/ 
/*N*/ 		//
/*N*/ 		//	Parameter-Strings
/*N*/ 		//
/*N*/ 
/*N*/         SetExtraStrings( *pMemChart );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		DBG_ERROR("SchDLL::NewMemChart gibt 0 zurueck!");
/*N*/ 
/*N*/ 	return pMemChart;
/*N*/ }

/*N*/ void ScChartArray::SetExtraStrings( SchMemChart& rMem )
/*N*/ {
/*N*/     ScRangePtr pR;
/*N*/ 	USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
/*N*/ #if 0
/* now this is done in SchMemChart::ConvertChartRangeForCalc() for SO5 file format
    const sal_Unicode cTok = ';';
    String aRef;
    for ( pR = aRangeListRef->First(); pR; pR = aRangeListRef->Next() )
    {
        pR->GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
        if ( aRef.Len() )
            aRef += cTok;
        aRef += String::CreateFromInt32( nTab1 );
        // hier ',' als TokenSep damit alte Versionen (<341/342) das ueberlesen
        aRef += ',';  aRef += String::CreateFromInt32( nTab2 );
        aRef += cTok; aRef += String::CreateFromInt32( nCol1 );
        aRef += cTok; aRef += String::CreateFromInt32( nRow1 );
        aRef += cTok; aRef += String::CreateFromInt32( nCol2 );
        aRef += cTok; aRef += String::CreateFromInt32( nRow2 );
    }

    String aFlags = bColHeaders ? '1' : '0';
    aFlags += bRowHeaders ? '1' : '0';
    aFlags += bDummyUpperLeft ? '1' : '0';
    aFlags += cTok;
    aFlags += String::CreateFromInt32( eGlue );
    aFlags += cTok;
    aFlags += String::CreateFromInt32( nStartCol );
    aFlags += cTok;
    aFlags += String::CreateFromInt32( nStartRow );

    rMem.SomeData1() = aRef;
    rMem.SomeData2() = aFlags;
*/
/*N*/ #endif
/*N*/ 
/*N*/     String aSheetNames;
/*N*/     SchChartRange aChartRange;
/*N*/     aChartRange.mbFirstColumnContainsLabels = bRowHeaders;
/*N*/     aChartRange.mbFirstRowContainsLabels = bColHeaders;
/*N*/     aChartRange.mbKeepCopyOfData = sal_False;
/*N*/     for ( pR = aRangeListRef->First(); pR; pR = aRangeListRef->Next() )
/*N*/     {
/*N*/         pR->GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
/*N*/         for ( USHORT nTab = nTab1; nTab <= nTab2; ++nTab )
/*N*/         {
/*N*/             SchCellRangeAddress aCellRangeAddress;
/*N*/             SchSingleCell aCell;
/*N*/             aCell.mnColumn = nCol1;
/*N*/             aCell.mnRow = nRow1;
/*N*/             aCellRangeAddress.maUpperLeft.maCells.push_back( aCell );
/*N*/             aCell.mnColumn = nCol2;
/*N*/             aCell.mnRow = nRow2;
/*N*/             aCellRangeAddress.maLowerRight.maCells.push_back( aCell );
/*N*/             aCellRangeAddress.mnTableNumber = nTab;
/*N*/             String aName;
/*N*/             pDocument->GetName( nTab, aName );
/*N*/             aCellRangeAddress.msTableName = aName;
/*N*/             aChartRange.maRanges.push_back( aCellRangeAddress );
/*N*/             if ( aSheetNames.Len() )
/*N*/                 aSheetNames += ';';
/*N*/             aSheetNames += aName;
/*N*/         }
/*N*/     }
/*N*/     rMem.SetChartRange( aChartRange );
/*N*/ 
/*N*/     // #90896# need that for OLE and clipboard of old binary file format
/*N*/     rMem.SomeData3() = aSheetNames;
/*N*/ 
/*N*/ 	rMem.SetReadOnly( TRUE );	// Daten nicht im Chart per Daten-Fenster veraendern
/*N*/ }
/*N*/ 
/*N*/ #ifdef _MSC_VER
/*N*/ #pragma optimize("",on)
/*N*/ #endif


/*N*/ const ScChartPositionMap* ScChartArray::GetPositionMap()
/*N*/ {
/*N*/ 	if ( !pPositionMap )
/*N*/ 		CreatePositionMap();
/*N*/ 	return pPositionMap;
/*N*/ }


/*N*/ void ScChartArray::CreatePositionMap()
/*N*/ {
/*N*/ 	if ( eGlue == SC_CHARTGLUE_NA && pPositionMap )
/*N*/ 	{
/*?*/ 		delete pPositionMap;
/*?*/ 		pPositionMap = NULL;
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( pPositionMap )
/*N*/ 		return ;
/*N*/ 
/*N*/ 	USHORT nColAdd = bRowHeaders ? 1 : 0;
/*N*/ 	USHORT nRowAdd = bColHeaders ? 1 : 0;
/*N*/ 
/*N*/ 	USHORT nCol, nRow, nTab, nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
/*N*/ 
/*N*/ 	//
/*N*/ 	//	wirkliche Groesse (ohne versteckte Zeilen/Spalten)
/*N*/ 	//
/*N*/ 
/*N*/ 	USHORT nColCount, nRowCount;
/*N*/ 	nColCount = nRowCount = 0;
/*N*/ 
/*N*/ 	GlueState();
/*N*/ 
/*N*/ 	BOOL bNoGlue = (eGlue == SC_CHARTGLUE_NONE);
/*N*/ 	Table* pCols = new Table;
/*N*/ 	Table* pNewRowTable = new Table;
/*N*/ 	ScAddress* pNewAddress = new ScAddress;
/*N*/ 	ScRangePtr pR;
/*N*/ 	Table* pCol;
/*N*/ 	ScAddress* pPos;
/*N*/ 	USHORT nNoGlueRow = 0;
/*N*/ 	for ( pR = aRangeListRef->First(); pR; pR = aRangeListRef->Next() )
/*N*/ 	{
/*N*/ 		pR->GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
/*N*/ 		for ( nTab = nTab1; nTab <= nTab2; nTab++ )
/*N*/ 		{
/*N*/ 			// nTab im ColKey, um gleiche Col/Row in anderer Table haben zu koennen
/*N*/ 			ScAddress aInsCol( (bNoGlue ? 0 : nCol1), 0, nTab );
/*N*/ 			for ( nCol = nCol1; nCol <= nCol2; nCol++, aInsCol.IncCol() )
/*N*/ 			{
/*N*/ 				if ( (pDocument->GetColFlags( nCol, nTab) & CR_HIDDEN) == 0 )
/*N*/ 				{
/*N*/ 					ULONG nInsCol = (ULONG)(UINT32) aInsCol;
/*N*/ 					if ( bNoGlue || eGlue == SC_CHARTGLUE_ROWS )
/*N*/ 					{	// meistens gleiche Cols
/*N*/ 						if ( !(pCol = (Table*) pCols->Get( nInsCol )) )
/*N*/ 						{
/*N*/ 							pCols->Insert( nInsCol, pNewRowTable );
/*N*/ 							pCol = pNewRowTable;
/*N*/ 							pNewRowTable = new Table;
/*N*/ 						}
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{	// meistens neue Cols
/*?*/ 						if ( pCols->Insert( nInsCol, pNewRowTable ) )
/*?*/ 						{
/*?*/ 							pCol = pNewRowTable;
/*?*/ 							pNewRowTable = new Table;
/*?*/ 						}
/*?*/ 						else
/*?*/ 							pCol = (Table*) pCols->Get( nInsCol );
/*N*/ 					}
/*N*/ 					// bei anderer Tabelle wurde bereits neuer ColKey erzeugt,
/*N*/ 					// die Zeilen muessen fuer's Dummy fuellen gleich sein!
/*N*/ 					ULONG nInsRow = (bNoGlue ? nNoGlueRow : nRow1);
/*N*/ 					for ( nRow = nRow1; nRow <= nRow2; nRow++, nInsRow++ )
/*N*/ 					{
/*N*/ 						if ( (pDocument->GetRowFlags( nRow, nTab) & CR_HIDDEN) == 0 )
/*N*/ 						{
/*N*/ 							if ( pCol->Insert( nInsRow, pNewAddress ) )
/*N*/ 							{
/*N*/ 								pNewAddress->Set( nCol, nRow, nTab );
/*N*/ 								pNewAddress = new ScAddress;
/*N*/ 							}
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		// bei NoGlue werden zusammengehoerige Tabellen als ColGlue dargestellt
/*N*/ 		nNoGlueRow += nRow2 - nRow1 + 1;
/*N*/ 	}
/*N*/ 	delete pNewAddress;
/*N*/ 	delete pNewRowTable;
/*N*/ 
/*N*/ 	// Anzahl der Daten
/*N*/ 	nColCount = (USHORT) pCols->Count();
/*N*/ 	if ( pCol = (Table*) pCols->First() )
/*N*/ 	{
/*N*/ 		if ( bDummyUpperLeft )
/*?*/ 			pCol->Insert( 0, (void*)0 );		// Dummy fuer Beschriftung
/*N*/ 		nRowCount = (USHORT) pCol->Count();
/*N*/ 	}
/*N*/ 	else
/*N*/ 		nRowCount = 0;
/*N*/ 	if ( nColCount )
/*N*/ 		nColCount -= nColAdd;
/*N*/ 	if ( nRowCount )
/*N*/ 		nRowCount -= nRowAdd;
/*N*/ 
/*N*/ 	if ( nColCount==0 || nRowCount==0 )
/*N*/ 	{	// einen Eintrag ohne Daten erzeugen
/*?*/ 		pR = aRangeListRef->First();
/*?*/ 		if ( pCols->Count() > 0 )
/*?*/ 			pCol = (Table*) pCols->First();
/*?*/ 		else
/*?*/ 		{
/*?*/ 			pCol = new Table;
/*?*/ 			pCols->Insert( 0, pCol );
/*?*/ 		}
/*?*/ 		nColCount = 1;
/*?*/ 		if ( pCol->Count() > 0 )
/*?*/ 		{	// kann ja eigentlich nicht sein, wenn nColCount==0 || nRowCount==0
/*?*/ 			pPos = (ScAddress*) pCol->First();
/*?*/ 			if ( pPos )
/*?*/ 			{
/*?*/ 				delete pPos;
/*?*/ 				pCol->Replace( pCol->GetCurKey(), (void*)0 );
/*?*/ 			}
/*?*/ 		}
/*?*/ 		else
/*?*/ 			pCol->Insert( 0, (void*)0 );
/*?*/ 		nRowCount = 1;
/*?*/ 		nColAdd = 0;
/*?*/ 		nRowAdd = 0;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if ( bNoGlue )
/*N*/ 		{	// Luecken mit Dummies fuellen, erste Spalte ist Master
/*?*/ 			Table* pFirstCol = (Table*) pCols->First();
/*?*/ 			ULONG nCount = pFirstCol->Count();
/*?*/ 			pFirstCol->First();
/*?*/ 			for ( ULONG n = 0; n < nCount; n++, pFirstCol->Next() )
/*?*/ 			{
/*?*/ 				ULONG nKey = pFirstCol->GetCurKey();
/*?*/ 				pCols->First();
/*?*/ 				while ( pCol = (Table*) pCols->Next() )
/*?*/ 					pCol->Insert( nKey, (void*)0 );		// keine Daten
/*?*/ 			}
/*?*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	pPositionMap = new ScChartPositionMap( nColCount, nRowCount,
/*N*/ 		nColAdd, nRowAdd, *pCols );
/*N*/ 
/*N*/ 	//	Aufraeumen
/*N*/ 	for ( pCol = (Table*) pCols->First(); pCol; pCol = (Table*) pCols->Next() )
/*N*/ 	{	//! nur Tables loeschen, nicht die ScAddress*
/*N*/ 		delete pCol;
/*N*/ 	}
/*N*/ 	delete pCols;
/*N*/ }


/*N*/ ScChartPositionMap::ScChartPositionMap( USHORT nChartCols, USHORT nChartRows,
/*N*/ 			USHORT nColAdd, USHORT nRowAdd, Table& rCols ) :
/*N*/ 		nCount( (ULONG) nChartCols * nChartRows ),
/*N*/ 		nColCount( nChartCols ),
/*N*/ 		nRowCount( nChartRows ),
/*N*/ 		ppData( new ScAddress* [ nChartCols * nChartRows ] ),
/*N*/ 		ppColHeader( new ScAddress* [ nChartCols ] ),
/*N*/ 		ppRowHeader( new ScAddress* [ nChartRows ] )
/*N*/ {
/*N*/ 	DBG_ASSERT( nColCount && nRowCount, "ScChartPositionMap without dimension" );
/*N*/ #ifdef WIN
/*N*/ #error ScChartPositionMap not implemented for 16-bit dumdums
/*N*/ #endif
/*N*/ 
/*N*/ 	ScAddress* pPos;
/*N*/ 	USHORT nCol, nRow;
/*N*/ 
/*N*/ 	Table* pCol = (Table*) rCols.First();
/*N*/ 
/*N*/ 	// Zeilen-Header
/*N*/ 	pPos = (ScAddress*) pCol->First();
/*N*/ 	if ( nRowAdd )
/*N*/ 		pPos = (ScAddress*) pCol->Next();
/*N*/ 	if ( nColAdd )
/*N*/ 	{	// eigenstaendig
/*N*/ 		for ( nRow = 0; nRow < nRowCount; nRow++ )
/*N*/ 		{
/*N*/ 			ppRowHeader[ nRow ] = pPos;
/*N*/ 			pPos = (ScAddress*) pCol->Next();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{	// Kopie
/*?*/ 		for ( nRow = 0; nRow < nRowCount; nRow++ )
/*?*/ 		{
/*?*/ 			ppRowHeader[ nRow ] = ( pPos ? new ScAddress( *pPos ) : NULL );
/*?*/ 			pPos = (ScAddress*) pCol->Next();
/*?*/ 		}
/*N*/ 	}
/*N*/ 	if ( nColAdd )
/*N*/ 		pCol = (Table*) rCols.Next();
/*N*/ 
/*N*/ 	// Daten spaltenweise und Spalten-Header
/*N*/ 	ULONG nIndex = 0;
/*N*/ 	for ( nCol = 0; nCol < nColCount; nCol++ )
/*N*/ 	{
/*N*/ 		if ( pCol )
/*N*/ 		{
/*N*/ 			pPos = (ScAddress*) pCol->First();
/*N*/ 			if ( nRowAdd )
/*N*/ 			{
/*N*/ 				ppColHeader[ nCol ] = pPos;		// eigenstaendig
/*N*/ 				pPos = (ScAddress*) pCol->Next();
/*N*/ 			}
/*N*/ 			else
/*?*/ 				ppColHeader[ nCol ] = ( pPos ? new ScAddress( *pPos ) : NULL );
/*N*/ 			for ( USHORT nRow = 0; nRow < nRowCount; nRow++, nIndex++ )
/*N*/ 			{
/*N*/ 				ppData[ nIndex ] = pPos;
/*N*/ 				pPos = (ScAddress*) pCol->Next();
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*?*/ 			ppColHeader[ nCol ] = NULL;
/*?*/ 			for ( nRow = 0; nRow < nRowCount; nRow++, nIndex++ )
/*?*/ 			{
/*?*/ 				ppData[ nIndex ] = NULL;
/*?*/ 			}
/*N*/ 		}
/*N*/ 		pCol = (Table*) rCols.Next();
/*N*/ 	}
/*N*/ }


/*N*/ ScChartPositionMap::~ScChartPositionMap()
/*N*/ {
/*N*/ 	for ( ULONG nIndex=0; nIndex < nCount; nIndex++ )
/*N*/ 	{
/*N*/ 		delete ppData[nIndex];
/*N*/ 	}
/*N*/ 	delete [] ppData;
/*N*/ 
/*N*/ 	USHORT j;
/*N*/ 	for ( j=0; j < nColCount; j++ )
/*N*/ 	{
/*N*/ 		delete ppColHeader[j];
/*N*/ 	}
/*N*/ 	delete [] ppColHeader;
/*N*/ 	for ( j=0; j < nRowCount; j++ )
/*N*/ 	{
/*N*/ 		delete ppRowHeader[j];
/*N*/ 	}
/*N*/ 	delete [] ppRowHeader;
/*N*/ }






//
//				Collection
//



/*N*/ BOOL ScChartCollection::Load( ScDocument* pDoc, SvStream& rStream )
/*N*/ {
/*N*/ 	BOOL bSuccess = TRUE;
/*N*/ 	USHORT nNewCount;
/*N*/ 	FreeAll();
/*N*/ 
/*N*/ 	ScMultipleReadHeader aHdr( rStream );
/*N*/ 
/*N*/ 	rStream >> nNewCount;
/*N*/ 
/*N*/ 	for (USHORT i=0; i<nNewCount && bSuccess; i++)
/*N*/ 	{
/*?*/ 		ScChartArray* pObject = new ScChartArray( pDoc, rStream, aHdr );
/*?*/ 		bSuccess = Insert( pObject );
/*N*/ 	}
/*N*/ 	return bSuccess;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
