/*************************************************************************
 *
 *  $RCSfile: chartarr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:17 $
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

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <svtools/intitem.hxx>
#include <svtools/zforlist.hxx>
#include <sch/schdll.hxx>
#include <sch/memchrt.hxx>
#include <float.h>              // DBL_MIN

#include "chartarr.hxx"
#include "document.hxx"
#include "rechead.hxx"
#include "globstr.hrc"
#include "cell.hxx"
#include "docoptio.hxx"

#ifndef _COM_SUN_STAR_CHART_CHARTSERIESADDRESS_HPP_
#include <com/sun/star/chart/ChartSeriesAddress.hpp>
#endif


// -----------------------------------------------------------------------

// static
void ScChartArray::CopySettings( SchMemChart& rDest, const SchMemChart& rSource )
{
    rDest.SetMainTitle( rSource.GetMainTitle() );
    rDest.SetSubTitle( rSource.GetSubTitle() );
    rDest.SetXAxisTitle( rSource.GetXAxisTitle() );
    rDest.SetYAxisTitle( rSource.GetYAxisTitle() );
    rDest.SetZAxisTitle( rSource.GetZAxisTitle() );

    const long* pArr;
    if ( rSource.GetRowCount() == rDest.GetRowCount() &&
         rSource.GetColCount() == rDest.GetColCount() )
    {
        //  don't copy column/row number formats here (are set in new MemChart object)

        if ( (pArr = rSource.GetRowTranslation()) ) rDest.SetRowTranslation( pArr );
        if ( (pArr = rSource.GetColTranslation()) ) rDest.SetColTranslation( pArr );
        rDest.SetTranslation( rSource.GetTranslation() );
    }
}

// -----------------------------------------------------------------------

ScChartArray::ScChartArray( ScDocument* pDoc, USHORT nTab,
                    USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
                    const String& rChartName ) :
        aName( rChartName ),
        pDocument( pDoc ),
        pPositionMap( NULL ),
        eGlue( SC_CHARTGLUE_NA ),
        nStartCol(0),
        nStartRow(0),
        bColHeaders( FALSE ),
        bRowHeaders( FALSE ),
        bDummyUpperLeft( FALSE ),
        bValid( TRUE )
{
    SetRangeList( ScRange( nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab ) );
    CheckColRowHeaders();
}

ScChartArray::ScChartArray( ScDocument* pDoc, const ScRangeListRef& rRangeList,
                    const String& rChartName ) :
        aRangeListRef( rRangeList ),
        aName( rChartName ),
        pDocument( pDoc ),
        pPositionMap( NULL ),
        eGlue( SC_CHARTGLUE_NA ),
        nStartCol(0),
        nStartRow(0),
        bColHeaders( FALSE ),
        bRowHeaders( FALSE ),
        bDummyUpperLeft( FALSE ),
        bValid( TRUE )
{
    if ( aRangeListRef.Is() )
        CheckColRowHeaders();
}

ScChartArray::ScChartArray( const ScChartArray& rArr ) :
        aRangeListRef( rArr.aRangeListRef ),
        aName(rArr.aName),
        pDocument(rArr.pDocument),
        pPositionMap( NULL ),
        eGlue(rArr.eGlue),
        nStartCol(rArr.nStartCol),
        nStartRow(rArr.nStartRow),
        bColHeaders(rArr.bColHeaders),
        bRowHeaders(rArr.bRowHeaders),
        bDummyUpperLeft( rArr.bDummyUpperLeft ),
        bValid(rArr.bValid)
{
}

ScChartArray::ScChartArray( ScDocument* pDoc, SvStream& rStream, ScMultipleReadHeader& rHdr ) :
        pDocument( pDoc ),
        pPositionMap( NULL ),
        eGlue( SC_CHARTGLUE_NONE ),
        bDummyUpperLeft( FALSE ),
        bValid( TRUE )
{
    USHORT nCol2, nRow2, nTable;

    rHdr.StartEntry();

    rStream >> nTable;
    rStream >> nStartCol;
    rStream >> nStartRow;
    rStream >> nCol2;
    rStream >> nRow2;
    rStream.ReadByteString( aName, rStream.GetStreamCharSet() );
    rStream >> bColHeaders;
    rStream >> bRowHeaders;

    rHdr.EndEntry();

    SetRangeList( ScRange( nStartCol, nStartRow, nTable, nCol2, nRow2, nTable ) );
}

ScChartArray::ScChartArray( ScDocument* pDoc, const SchMemChart& rData ) :
        pDocument( pDoc ),
        pPositionMap( NULL )
{
    const sal_Unicode cTok = ';';
    BOOL bInitOk = FALSE;
    xub_StrLen nToken;
    String aPos = ((SchMemChart&)rData).SomeData1();
    if ( (nToken = aPos.GetTokenCount( cTok )) >= 5)
    {
        String aOpt = ((SchMemChart&)rData).SomeData2();
        xub_StrLen nOptToken = aOpt.GetTokenCount( cTok );
        BOOL bNewChart = (nOptToken >= 4);      // ab 341/342
        aRangeListRef.Clear();
        USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
        xub_StrLen nInd = 0;
        for ( xub_StrLen j=0; j < nToken; j+=5 )
        {
            xub_StrLen nInd2 = nInd;
            nTab1 = (USHORT) aPos.GetToken( 0, cTok, nInd ).ToInt32();
            // damit alte Versionen (<341/342) das ueberlesen ist der nTab2
            // Token Separator ein ','
            if ( bNewChart )
                nTab2 = (USHORT) aPos.GetToken( 1, ',', nInd2 ).ToInt32();
            else
                nTab2 = nTab1;
            nCol1 = (USHORT) aPos.GetToken( 0, cTok, nInd ).ToInt32();
            nRow1 = (USHORT) aPos.GetToken( 0, cTok, nInd ).ToInt32();
            nCol2 = (USHORT) aPos.GetToken( 0, cTok, nInd ).ToInt32();
            nRow2 = (USHORT) aPos.GetToken( 0, cTok, nInd ).ToInt32();
            AddToRangeList( ScRange( nCol1, nRow1, nTab1,
                nCol2, nRow2, nTab2 ) );
        }
        bValid = TRUE;

        if (aOpt.Len() >= 2)
        {
            bColHeaders = ( aOpt.GetChar(0) != '0' );
            bRowHeaders = ( aOpt.GetChar(1) != '0' );
            if ( aOpt.Len() >= 3 )
            {
                if ( bNewChart )
                {
                    bDummyUpperLeft = ( aOpt.GetChar(2) != '0' );
                    xub_StrLen nInd = 4;    // 111;
                    eGlue = (ScChartGlue) aOpt.GetToken( 0, cTok, nInd ).ToInt32();
                    nStartCol = (USHORT) aOpt.GetToken( 0, cTok, nInd ).ToInt32();
                    nStartRow = (USHORT) aOpt.GetToken( 0, cTok, nInd ).ToInt32();
                    bInitOk = TRUE;
                }
            }
        }
        else
            bColHeaders = bRowHeaders = FALSE;
    }
    else
    {
        SetRangeList( ScRange() );
        bColHeaders = bRowHeaders = bValid = FALSE;
    }
    if ( !bInitOk )
    {   // muessen in GlueState neu berechnet werden
        InvalidateGlue();
        nStartCol = nStartRow = 0;
        bDummyUpperLeft = FALSE;
    }
}

ScChartArray::~ScChartArray()
{
    delete pPositionMap;
}

DataObject* ScChartArray::Clone() const
{
    return new ScChartArray(*this);
}

BOOL ScChartArray::operator==(const ScChartArray& rCmp) const
{
    return bColHeaders == rCmp.bColHeaders
        && bRowHeaders == rCmp.bRowHeaders
        && aName == rCmp.aName
        && *aRangeListRef == *rCmp.aRangeListRef;
}

BOOL ScChartArray::IsAtCursor(const ScAddress& rPos) const
{
    for ( ScRangePtr pR = aRangeListRef->First(); pR;
                     pR = aRangeListRef->Next() )
    {
        if ( pR->In( rPos ) )
            return TRUE;
    }
    return FALSE;
}

void ScChartArray::SetRangeList( const ScRange& rRange )
{
    aRangeListRef = new ScRangeList();      // handelt auch ReleaseRef / AddRef
    aRangeListRef->Append( rRange );
    InvalidateGlue();
}

void ScChartArray::AddToRangeList( const ScRange& rRange )
{
    if ( aRangeListRef.Is() )
        aRangeListRef->Append( rRange );
    else
        SetRangeList( rRange );
    InvalidateGlue();
}

void ScChartArray::AddToRangeList( const ScRangeListRef& rAdd )
{
    if ( aRangeListRef.Is() )
    {
        ULONG nCount = rAdd->Count();
        for (ULONG i=0; i<nCount; i++)
            aRangeListRef->Join( *rAdd->GetObject(i) );
    }
    else
        SetRangeList( rAdd );
    InvalidateGlue();
}

void ScChartArray::GlueState()
{
    if ( eGlue != SC_CHARTGLUE_NA )
        return;
    bDummyUpperLeft = FALSE;
    ScRangePtr pR;
    if ( aRangeListRef->Count() <= 1 )
    {
        if ( pR = aRangeListRef->First() )
        {
            if ( pR->aStart.Tab() == pR->aEnd.Tab() )
                eGlue = SC_CHARTGLUE_NONE;
            else
                eGlue = SC_CHARTGLUE_COLS;  // mehrere Tabellen spaltenweise
            nStartCol = pR->aStart.Col();
            nStartRow = pR->aStart.Row();
        }
        else
        {
            InvalidateGlue();
            nStartCol = nStartRow = 0;
        }
        return;
    }
    ULONG nOldPos = aRangeListRef->GetCurPos();

    pR = aRangeListRef->First();
    nStartCol = pR->aStart.Col();
    nStartRow = pR->aStart.Row();
    USHORT nMaxCols, nMaxRows, nEndCol, nEndRow;
    nMaxCols = nMaxRows = nEndCol = nEndRow = 0;
    do
    {   // umspannenden Bereich etc. feststellen
        USHORT nTmp, n1, n2;
        if ( (n1 = pR->aStart.Col()) < nStartCol )
            nStartCol = n1;
        if ( (n2 = pR->aEnd.Col()) > nEndCol )
            nEndCol = n2;
        if ( (nTmp = n2 - n1 + 1) > nMaxCols )
            nMaxCols = nTmp;
        if ( (n1 = pR->aStart.Row()) < nStartRow )
            nStartRow = n1;
        if ( (n2 = pR->aEnd.Row()) > nEndRow )
            nEndRow = n2;
        if ( (nTmp = n2 - n1 + 1) > nMaxRows )
            nMaxRows = nTmp;
    } while ( pR = aRangeListRef->Next() );
    USHORT nC = nEndCol - nStartCol + 1;
    if ( nC == 1 )
    {
        eGlue = SC_CHARTGLUE_ROWS;
        return;
    }
    USHORT nR = nEndRow - nStartRow + 1;
    if ( nR == 1 )
    {
        eGlue = SC_CHARTGLUE_COLS;
        return;
    }
    ULONG nCR = (ULONG)nC * nR;
//2do:
/*
    Erstmal simpel ohne Bitmaskiererei, maximal koennten so 8MB alloziert
    werden (256 Cols mal 32000 Rows), das liesse sich mit 2 Bit je Eintrag
    auf 2MB reduzieren, andererseits ist es so schneller.
    Weitere Platz-Optimierung waere, in dem Array nur die wirklich benutzten
    Zeilen/Spalten abzulegen, wuerde aber ein weiteres durchlaufen der
    RangeList und indirekten Zugriff auf das Array bedeuten.
 */
    const BYTE nHole = 0;
    const BYTE nOccu = 1;
    const BYTE nFree = 2;
    const BYTE nGlue = 3;
#ifdef WIN
    // we hate 16bit, don't we?
    BYTE huge* p;
    BYTE huge* pA = (BYTE huge*) SvMemAlloc( nCR );
    if ( nCR > (ULONG)((USHORT)~0) )
    {   // in 32k Bloecken initialisieren
        ULONG j;
        for ( j=0; j<nCR; j+=0x8000 )
        {
            memset( pA+j, nHole, Min( (ULONG)0x8000, nCR-j ) );
        }
    }
    else
        memset( pA, nHole, nCR * sizeof(BYTE) );
#else
    BYTE* p;
    BYTE* pA = new BYTE[ nCR ];
    memset( pA, 0, nCR * sizeof(BYTE) );
#endif

    USHORT nCol, nRow, nCol1, nRow1, nCol2, nRow2;
    for ( pR = aRangeListRef->First(); pR; pR = aRangeListRef->Next() )
    {   // Selektionen 2D als belegt markieren
        nCol1 = pR->aStart.Col() - nStartCol;
        nCol2 = pR->aEnd.Col() - nStartCol;
        nRow1 = pR->aStart.Row() - nStartRow;
        nRow2 = pR->aEnd.Row() - nStartRow;
        for ( nCol = nCol1; nCol <= nCol2; nCol++ )
        {
            p = pA + (ULONG)nCol * nR + nRow1;
            for ( nRow = nRow1; nRow <= nRow2; nRow++, p++ )
                *p = nOccu;
        }
    }
    BOOL bGlue = TRUE;

    BOOL bGlueCols = FALSE;
    for ( nCol = 0; bGlue && nCol < nC; nCol++ )
    {   // Spalten probieren durchzugehen und als frei markieren
        p = pA + (ULONG)nCol * nR;
        for ( nRow = 0; bGlue && nRow < nR; nRow++, p++ )
        {
            if ( *p == nOccu )
            {   // Wenn einer mittendrin liegt ist keine Zusammenfassung
                // moeglich. Am Rand koennte ok sein, wenn in dieser Spalte
                // in jeder belegten Zeile einer belegt ist.
                if ( nRow > 0 && nCol > 0 )
                    bGlue = FALSE;      // nCol==0 kann DummyUpperLeft sein
                else
                    nRow = nR;
            }
            else
                *p = nFree;
        }
        if ( bGlue && *(p = (pA + ((((ULONG)nCol+1) * nR) - 1))) == nFree )
        {   // Spalte als komplett frei markieren
            *p = nGlue;
            bGlueCols = TRUE;       // mindestens eine freie Spalte
        }
    }

    BOOL bGlueRows = FALSE;
    for ( nRow = 0; bGlue && nRow < nR; nRow++ )
    {   // Zeilen probieren durchzugehen und als frei markieren
        p = pA + nRow;
        for ( nCol = 0; bGlue && nCol < nC; nCol++, p+=nR )
        {
            if ( *p == nOccu )
            {
                if ( nCol > 0 && nRow > 0 )
                    bGlue = FALSE;      // nRow==0 kann DummyUpperLeft sein
                else
                    nCol = nC;
            }
            else
                *p = nFree;
        }
        if ( bGlue && *(p = (pA + ((((ULONG)nC-1) * nR) + nRow))) == nFree )
        {   // Zeile als komplett frei markieren
            *p = nGlue;
            bGlueRows = TRUE;       // mindestens eine freie Zeile
        }
    }

    // n=1: die linke obere Ecke koennte bei Beschriftung automagisch
    // hinzugezogen werden
    p = pA + 1;
    for ( ULONG n = 1; bGlue && n < nCR; n++, p++ )
    {   // ein unberuehrtes Feld heisst, dass es weder spaltenweise noch
        // zeilenweise zu erreichen war, also nichts zusamenzufassen
        if ( *p == nHole )
            bGlue = FALSE;
    }
    if ( bGlue )
    {
        if ( bGlueCols && bGlueRows )
            eGlue = SC_CHARTGLUE_BOTH;
        else if ( bGlueRows )
            eGlue = SC_CHARTGLUE_ROWS;
        else
            eGlue = SC_CHARTGLUE_COLS;
        if ( *pA != nOccu )
            bDummyUpperLeft = TRUE;
    }
    else
    {
        eGlue = SC_CHARTGLUE_NONE;
    }

#ifdef WIN
    SvMemFree( pA );
#else
    delete [] pA;
#endif
}

void ScChartArray::CheckColRowHeaders()
{
    USHORT i, nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;

    BOOL bColStrings = TRUE;
    BOOL bRowStrings = TRUE;
    GlueState();
    if ( aRangeListRef->Count() == 1 )
    {
        aRangeListRef->First()->GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
        // Beschriftungen auch nach hidden Cols/Rows finden
        while ( nCol1 <= nCol2 && (pDocument->GetColFlags(
                nCol1, nTab1) & CR_HIDDEN) != 0 )
            nCol1++;
        while ( nRow1 <= nRow2 && (pDocument->GetRowFlags(
                nRow1, nTab1) & CR_HIDDEN) != 0 )
            nRow1++;
        if ( nCol1 > nCol2 || nRow1 > nRow2 )
            bColStrings = bRowStrings = FALSE;
        else
        {
            for (i=nCol1; i<=nCol2 && bColStrings; i++)
            {
                if ( i==nCol1 || (pDocument->GetColFlags( i, nTab1) & CR_HIDDEN) == 0 )
                    if (pDocument->HasValueData( i, nRow1, nTab1 ))
                        bColStrings = FALSE;
            }
            for (i=nRow1; i<=nRow2 && bRowStrings; i++)
            {
                if ( i==nRow1 || (pDocument->GetRowFlags( i, nTab1) & CR_HIDDEN) == 0 )
                    if (pDocument->HasValueData( nCol1, i, nTab1 ))
                        bRowStrings = FALSE;
            }
        }
    }
    else
    {
        BOOL bVert = (eGlue == SC_CHARTGLUE_NONE || eGlue == SC_CHARTGLUE_ROWS);
        for ( ScRangePtr pR = aRangeListRef->First();
                pR && (bColStrings || bRowStrings);
                pR = aRangeListRef->Next() )
        {
            pR->GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
            BOOL bTopRow = (nRow1 == nStartRow);
            BOOL bHidOk;
            if ( bRowStrings && (bVert || nCol1 == nStartCol) )
            {   // NONE oder ROWS: RowStrings in jeder Selektion moeglich
                // COLS oder BOTH: nur aus der ersten Spalte
                while ( nCol1 <= nCol2 && (pDocument->GetColFlags(
                        nCol1, nTab1) & CR_HIDDEN) != 0 )
                    nCol1++;
                while ( nRow1 <= nRow2 && (pDocument->GetRowFlags(
                        nRow1, nTab1) & CR_HIDDEN) != 0 )
                    nRow1++;
                if ( nCol1 <= nCol2 )
                    for (i=nRow1; i<=nRow2 && bRowStrings; i++)
                    {
                        if ( i==nRow1 || (pDocument->GetRowFlags( i, nTab1) & CR_HIDDEN) == 0 )
                            if (pDocument->HasValueData( nCol1, i, nTab1 ))
                                bRowStrings = FALSE;
                    }
                bHidOk = TRUE;
            }
            else
                bHidOk = FALSE;
            if ( bColStrings && bTopRow )
            {   // ColStrings nur aus der ersten Zeile
                if ( !bHidOk )
                {
                    while ( nCol1 <= nCol2 && (pDocument->GetColFlags(
                            nCol1, nTab1) & CR_HIDDEN) != 0 )
                        nCol1++;
                    while ( nRow1 <= nRow2 && (pDocument->GetRowFlags(
                            nRow1, nTab1) & CR_HIDDEN) != 0 )
                        nRow1++;
                }
                if ( nRow1 <= nRow2 )
                    for (i=nCol1; i<=nCol2 && bColStrings; i++)
                    {
                        if ( i==nCol1 || (pDocument->GetColFlags( i, nTab1) & CR_HIDDEN) == 0 )
                            if (pDocument->HasValueData( i, nRow1, nTab1 ))
                                bColStrings = FALSE;
                    }
            }
        }
    }
    bColHeaders = bColStrings;
    bRowHeaders = bRowStrings;
}

#ifdef WNT
#pragma optimize("",off)
#endif

SchMemChart* ScChartArray::CreateMemChart()
{
    ULONG nCount = aRangeListRef->Count();
    if ( nCount > 1 )
        return CreateMemChartMulti();
    else if ( nCount == 1 )
    {
        ScRange* pR = aRangeListRef->First();
        if ( pR->aStart.Tab() != pR->aEnd.Tab() )
            return CreateMemChartMulti();
        else
            return CreateMemChartSingle();
    }
    else
        return CreateMemChartMulti();   // kann 0 Range besser ab als Single
}

SchMemChart* ScChartArray::CreateMemChartSingle() const
{
    USHORT i,nCol,nRow;

        //
        //  wirkliche Groesse (ohne versteckte Zeilen/Spalten)
        //

    USHORT nColAdd = bRowHeaders ? 1 : 0;
    USHORT nRowAdd = bColHeaders ? 1 : 0;

    USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
    aRangeListRef->First()->GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );

    USHORT nStrCol = nCol1;     // fuer Beschriftung merken
    USHORT nStrRow = nRow1;
    // Beschriftungen auch nach HiddenCols finden
    while ( (pDocument->GetColFlags( nCol1, nTab1) & CR_HIDDEN) != 0 )
        nCol1++;
    while ( (pDocument->GetRowFlags( nRow1, nTab1) & CR_HIDDEN) != 0 )
        nRow1++;
    // falls alles hidden ist, bleibt die Beschriftung am Anfang
    if ( nCol1 <= nCol2 )
    {
        nStrCol = nCol1;
        nCol1 += nColAdd;
    }
    if ( nRow1 <= nRow2 )
    {
        nStrRow = nRow1;
        nRow1 += nRowAdd;
    }

    USHORT nTotalCols = ( nCol1 <= nCol2 ? nCol2 - nCol1 + 1 : 0 );
    USHORT* pCols = new USHORT[nTotalCols ? nTotalCols : 1];
    USHORT nColCount = 0;
    for (i=0; i<nTotalCols; i++)
        if ((pDocument->GetColFlags(nCol1+i,nTab1)&CR_HIDDEN)==0)
            pCols[nColCount++] = nCol1+i;

    USHORT nTotalRows = ( nRow1 <= nRow2 ? nRow2 - nRow1 + 1 : 0 );
    USHORT* pRows = new USHORT[nTotalRows ? nTotalRows : 1];
    USHORT nRowCount = 0;
    for (i=0; i<nTotalRows; i++)
        if ((pDocument->GetRowFlags(nRow1+i,nTab1)&CR_HIDDEN)==0)
            pRows[nRowCount++] = nRow1+i;

    BOOL bValidData = TRUE;
    if ( !nColCount )
    {
        bValidData = FALSE;
        nColCount = 1;
        pCols[0] = nStrCol;
    }
    if ( !nRowCount )
    {
        bValidData = FALSE;
        nRowCount = 1;
        pRows[0] = nStrRow;
    }

        //
        //  Daten
        //

    SchMemChart* pMemChart = SchDLL::NewMemChart( nColCount, nRowCount );
    if (pMemChart)
    {
        ScRangeListRef xRL = new ScRangeList;
        SvNumberFormatter* pFormatter = pDocument->GetFormatTable();
        pMemChart->SetNumberFormatter( pFormatter );
        if ( bValidData )
        {
            com::sun::star::uno::Sequence<
                com::sun::star::chart::ChartSeriesAddress > aSeriesSeq( nColCount );
            BOOL bCalcAsShown = pDocument->GetDocOptions().IsCalcAsShown();
            ScBaseCell* pCell;
            for (nCol=0; nCol<nColCount; nCol++)
            {
                xRL->RemoveAll();
                for (nRow=0; nRow<nRowCount; nRow++)
                {
                    xRL->Join( ScAddress( pCols[nCol], pRows[nRow], nTab1 ) );

                    double nVal = DBL_MIN;      // Hack fuer Chart, um leere Zellen zu erkennen

                    pDocument->GetCell( pCols[nCol], pRows[nRow], nTab1, pCell );
                    if (pCell)
                    {
                        CellType eType = pCell->GetCellType();
                        if (eType == CELLTYPE_VALUE)
                        {
                            nVal = ((ScValueCell*)pCell)->GetValue();
                            if ( bCalcAsShown && nVal != 0.0 )
                            {
                                ULONG nFormat;
                                pDocument->GetNumberFormat( pCols[nCol],
                                    pRows[nRow], nTab1, nFormat );
                                nVal = pDocument->RoundValueAsShown( nVal, nFormat );
                            }
                        }
                        else if (eType == CELLTYPE_FORMULA)
                        {
                            ScFormulaCell* pFCell = (ScFormulaCell*)pCell;
                            if ( (pFCell->GetErrCode() == 0) && pFCell->IsValue() )
                                nVal = pFCell->GetValue();
                        }
                    }
                    pMemChart->SetData(nCol, nRow, nVal);
                }
                com::sun::star::chart::ChartSeriesAddress aSeries;
                String aStr;
                xRL->Format( aStr, SCR_ABS_3D, pDocument );
                aSeries.DataRangeAddress = aStr;
                if ( bColHeaders )
                {
                    ScAddress aAddr( pCols[nCol], nStrRow, nTab1 );
                    aAddr.Format( aStr, SCR_ABS_3D, pDocument );
                    aSeries.LabelAddress = aStr;
                }
                aSeriesSeq[nCol] = aSeries;
            }
            pMemChart->SetSeriesAddresses( aSeriesSeq );
        }
        else
        {
            //! Flag, dass Daten ungueltig ??

            for (nCol=0; nCol<nColCount; nCol++)
                for (nRow=0; nRow<nRowCount; nRow++)
                    pMemChart->SetData( nCol, nRow, DBL_MIN );
        }

        //
        //  Spalten-Header
        //

        for (nCol=0; nCol<nColCount; nCol++)
        {
            String aString;
            if (bColHeaders)
                pDocument->GetString( pCols[nCol], nStrRow, nTab1, aString );
            if ( !aString.Len() )
            {
                aString = ScGlobal::GetRscString(STR_COLUMN);
                aString += ' ';
                aString += String::CreateFromInt32( pCols[nCol]+1 );
            }
            pMemChart->SetColText(nCol, aString);

            ULONG nNumberAttr = pDocument->GetNumberFormat( ScAddress(
                                            pCols[nCol], nRow1, nTab1 ) );
            pMemChart->SetNumFormatIdCol( nCol, nNumberAttr );
        }

        //
        //  Zeilen-Header
        //

        xRL->RemoveAll();
        for (nRow=0; nRow<nRowCount; nRow++)
        {
            String aString;
            if (bRowHeaders)
            {
                ScAddress aAddr( nStrCol, pRows[nRow], nTab1 );
                xRL->Join( aAddr );
                pDocument->GetString( nStrCol, pRows[nRow], nTab1, aString );
            }
            if ( !aString.Len() )
            {
                aString = ScGlobal::GetRscString(STR_ROW);
                aString += ' ';
                aString += String::CreateFromInt32( pRows[nRow]+1 );
            }
            pMemChart->SetRowText(nRow, aString);

            ULONG nNumberAttr = pDocument->GetNumberFormat( ScAddress(
                                            nCol1, pRows[nRow], nTab1 ) );
            pMemChart->SetNumFormatIdRow( nRow, nNumberAttr );
        }
        String aCategoriesStr;
        xRL->Format( aCategoriesStr, SCR_ABS_3D, pDocument );
        pMemChart->SetCategoriesRangeAddress( aCategoriesStr );

        //
        //  Titel
        //

        pMemChart->SetMainTitle(ScGlobal::GetRscString(STR_CHART_MAINTITLE));
        pMemChart->SetSubTitle(ScGlobal::GetRscString(STR_CHART_SUBTITLE));
        pMemChart->SetXAxisTitle(ScGlobal::GetRscString(STR_CHART_XTITLE));
        pMemChart->SetYAxisTitle(ScGlobal::GetRscString(STR_CHART_YTITLE));
        pMemChart->SetZAxisTitle(ScGlobal::GetRscString(STR_CHART_ZTITLE));

        //
        //  Zahlen-Typ
        //

        ULONG nNumberAttr = pDocument->GetNumberFormat( ScAddress(
                                        nCol1, nRow1, nTab1 ) );
        if (pFormatter)
            pMemChart->SetDataType(pFormatter->GetType( nNumberAttr ));

        //
        //  Parameter-Strings
        //

        SetExtraStrings(*pMemChart);
    }
    else
        DBG_ERROR("SchDLL::NewMemChart gibt 0 zurueck!");

        //  Aufraeumen

    delete[] pRows;
    delete[] pCols;

    return pMemChart;
}

SchMemChart* ScChartArray::CreateMemChartMulti()
{
    CreatePositionMap();
    USHORT nColCount = pPositionMap->GetColCount();
    USHORT nRowCount = pPositionMap->GetRowCount();

    USHORT nCol, nRow;

    //
    //  Daten
    //

    SchMemChart* pMemChart = SchDLL::NewMemChart( nColCount, nRowCount );
    if (pMemChart)
    {
        ScRangeListRef xRL = new ScRangeList;
        com::sun::star::uno::Sequence<
            com::sun::star::chart::ChartSeriesAddress > aSeriesSeq( nColCount );
        pMemChart->SetNumberFormatter( pDocument->GetFormatTable() );
        BOOL bCalcAsShown = pDocument->GetDocOptions().IsCalcAsShown();
        ULONG nIndex = 0;
        for ( nCol = 0; nCol < nColCount; nCol++ )
        {
            xRL->RemoveAll();
            for ( nRow = 0; nRow < nRowCount; nRow++, nIndex++ )
            {
                double nVal = DBL_MIN;      // Hack fuer Chart, um leere Zellen zu erkennen
                const ScAddress* pPos = pPositionMap->GetPosition( nIndex );
                if ( pPos )
                {   // sonst: Luecke
                    xRL->Join( *pPos );
                    ScBaseCell* pCell = pDocument->GetCell( *pPos );
                    if (pCell)
                    {
                        CellType eType = pCell->GetCellType();
                        if (eType == CELLTYPE_VALUE)
                        {
                            nVal = ((ScValueCell*)pCell)->GetValue();
                            if ( bCalcAsShown && nVal != 0.0 )
                            {
                                ULONG nFormat = pDocument->GetNumberFormat( *pPos );
                                nVal = pDocument->RoundValueAsShown( nVal, nFormat );
                            }
                        }
                        else if (eType == CELLTYPE_FORMULA)
                        {
                            ScFormulaCell* pFCell = (ScFormulaCell*)pCell;
                            if ( (pFCell->GetErrCode() == 0) && pFCell->IsValue() )
                                nVal = pFCell->GetValue();
                        }
                    }
                }
                pMemChart->SetData(nCol, nRow, nVal);
            }
            com::sun::star::chart::ChartSeriesAddress aSeries;
            String aStr;
            xRL->Format( aStr, SCR_ABS_3D, pDocument );
            aSeries.DataRangeAddress = aStr;
            if ( bColHeaders )
            {
                const ScAddress* pPos = pPositionMap->GetColHeaderPosition( nCol );
                if ( pPos )
                {
                    pPos->Format( aStr, SCR_ABS_3D, pDocument );
                    aSeries.LabelAddress = aStr;
                }
            }
            aSeriesSeq[nCol] = aSeries;
        }
        pMemChart->SetSeriesAddresses( aSeriesSeq );

//2do: Beschriftung bei Luecken

        //
        //  Spalten-Header
        //

        USHORT nPosCol = 0;
        for ( nCol = 0; nCol < nColCount; nCol++ )
        {
            String aString;
            const ScAddress* pPos = pPositionMap->GetColHeaderPosition( nCol );
            if ( bColHeaders && pPos )
                pDocument->GetString(
                    pPos->Col(), pPos->Row(), pPos->Tab(), aString );
            if ( !aString.Len() )
            {
                aString = ScGlobal::GetRscString(STR_COLUMN);
                aString += ' ';
                if ( pPos )
                    nPosCol = pPos->Col() + 1;
                else
                    nPosCol++;
                aString += String::CreateFromInt32( nPosCol );
            }
            pMemChart->SetColText(nCol, aString);

            ULONG nNumberAttr = 0;
            pPos = pPositionMap->GetPosition( nCol, 0 );
            if ( pPos )
                nNumberAttr = pDocument->GetNumberFormat( *pPos );
            pMemChart->SetNumFormatIdCol( nCol, nNumberAttr );
        }

        //
        //  Zeilen-Header
        //

        xRL->RemoveAll();
        USHORT nPosRow = 0;
        for ( nRow = 0; nRow < nRowCount; nRow++ )
        {
            String aString;
            const ScAddress* pPos = pPositionMap->GetRowHeaderPosition( nRow );
            if ( bRowHeaders && pPos )
            {
                xRL->Join( *pPos );
                pDocument->GetString(
                    pPos->Col(), pPos->Row(), pPos->Tab(), aString );
            }
            if ( !aString.Len() )
            {
                aString = ScGlobal::GetRscString(STR_ROW);
                aString += ' ';
                if ( pPos )
                    nPosRow = pPos->Row() + 1;
                else
                    nPosRow++;
                aString += String::CreateFromInt32( nPosRow );
            }
            pMemChart->SetRowText(nRow, aString);

            ULONG nNumberAttr = 0;
            pPos = pPositionMap->GetPosition( 0, nRow );
            if ( pPos )
                nNumberAttr = pDocument->GetNumberFormat( *pPos );
            pMemChart->SetNumFormatIdRow( nRow, nNumberAttr );
        }
        String aCategoriesStr;
        xRL->Format( aCategoriesStr, SCR_ABS_3D, pDocument );
        pMemChart->SetCategoriesRangeAddress( aCategoriesStr );

        //
        //  Titel
        //

        pMemChart->SetMainTitle(ScGlobal::GetRscString(STR_CHART_MAINTITLE));
        pMemChart->SetSubTitle(ScGlobal::GetRscString(STR_CHART_SUBTITLE));
        pMemChart->SetXAxisTitle(ScGlobal::GetRscString(STR_CHART_XTITLE));
        pMemChart->SetYAxisTitle(ScGlobal::GetRscString(STR_CHART_YTITLE));
        pMemChart->SetZAxisTitle(ScGlobal::GetRscString(STR_CHART_ZTITLE));

        //
        //  Zahlen-Typ
        //

        SvNumberFormatter* pFormatter = pDocument->GetFormatTable();
        if (pFormatter)
        {
            ULONG nIndex = 0;
            ULONG nCount = pPositionMap->GetCount();
            const ScAddress* pPos;
            do
            {
                pPos = pPositionMap->GetPosition( nIndex );
            } while ( !pPos && ++nIndex < nCount );
            ULONG nFormat = ( pPos ? pDocument->GetNumberFormat( *pPos ) : 0 );
            pMemChart->SetDataType( pFormatter->GetType( nFormat ) );
        }

        //
        //  Parameter-Strings
        //

        SetExtraStrings(*pMemChart);
    }
    else
        DBG_ERROR("SchDLL::NewMemChart gibt 0 zurueck!");

    return pMemChart;
}

void ScChartArray::SetExtraStrings(SchMemChart& rMem) const
{
    const sal_Unicode cTok = ';';
    String aRef;
    USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
    BOOL bFirst = TRUE;
    for ( ScRangePtr pR = aRangeListRef->First(); pR; pR = aRangeListRef->Next() )
    {
        pR->GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
        if ( bFirst )
            bFirst = FALSE;
        else
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

    rMem.SetReadOnly( TRUE );   // Daten nicht im Chart per Daten-Fenster veraendern
}

#ifdef WNT
#pragma optimize("",on)
#endif


const ScChartPositionMap* ScChartArray::GetPositionMap()
{
    if ( !pPositionMap )
        CreatePositionMap();
    return pPositionMap;
}


void ScChartArray::CreatePositionMap()
{
    if ( eGlue == SC_CHARTGLUE_NA && pPositionMap )
    {
        delete pPositionMap;
        pPositionMap = NULL;
    }

    if ( pPositionMap )
        return ;

    USHORT nColAdd = bRowHeaders ? 1 : 0;
    USHORT nRowAdd = bColHeaders ? 1 : 0;

    USHORT nCol, nRow, nTab, nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;

    //
    //  wirkliche Groesse (ohne versteckte Zeilen/Spalten)
    //

    USHORT nColCount, nRowCount;
    nColCount = nRowCount = 0;

    GlueState();

    BOOL bNoGlue = (eGlue == SC_CHARTGLUE_NONE);
    Table* pCols = new Table;
    Table* pNewRowTable = new Table;
    ScAddress* pNewAddress = new ScAddress;
    ScRangePtr pR;
    Table* pCol;
    ScAddress* pPos;
    USHORT nNoGlueRow = 0;
    for ( pR = aRangeListRef->First(); pR; pR = aRangeListRef->Next() )
    {
        pR->GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
        for ( nTab = nTab1; nTab <= nTab2; nTab++ )
        {
            // nTab im ColKey, um gleiche Col/Row in anderer Table haben zu koennen
            ScAddress aInsCol( (bNoGlue ? 0 : nCol1), 0, nTab );
            for ( nCol = nCol1; nCol <= nCol2; nCol++, aInsCol.IncCol() )
            {
                if ( (pDocument->GetColFlags( nCol, nTab) & CR_HIDDEN) == 0 )
                {
                    ULONG nInsCol = (ULONG)(UINT32) aInsCol;
                    if ( bNoGlue || eGlue == SC_CHARTGLUE_ROWS )
                    {   // meistens gleiche Cols
                        if ( !(pCol = (Table*) pCols->Get( nInsCol )) )
                        {
                            pCols->Insert( nInsCol, pNewRowTable );
                            pCol = pNewRowTable;
                            pNewRowTable = new Table;
                        }
                    }
                    else
                    {   // meistens neue Cols
                        if ( pCols->Insert( nInsCol, pNewRowTable ) )
                        {
                            pCol = pNewRowTable;
                            pNewRowTable = new Table;
                        }
                        else
                            pCol = (Table*) pCols->Get( nInsCol );
                    }
                    // bei anderer Tabelle wurde bereits neuer ColKey erzeugt,
                    // die Zeilen muessen fuer's Dummy fuellen gleich sein!
                    ULONG nInsRow = (bNoGlue ? nNoGlueRow : nRow1);
                    for ( nRow = nRow1; nRow <= nRow2; nRow++, nInsRow++ )
                    {
                        if ( (pDocument->GetRowFlags( nRow, nTab) & CR_HIDDEN) == 0 )
                        {
                            if ( pCol->Insert( nInsRow, pNewAddress ) )
                            {
                                pNewAddress->Set( nCol, nRow, nTab );
                                pNewAddress = new ScAddress;
                            }
                        }
                    }
                }
            }
        }
        // bei NoGlue werden zusammengehoerige Tabellen als ColGlue dargestellt
        nNoGlueRow += nRow2 - nRow1 + 1;
    }
    delete pNewAddress;
    delete pNewRowTable;

    // Anzahl der Daten
    nColCount = (USHORT) pCols->Count();
    if ( pCol = (Table*) pCols->First() )
    {
        if ( bDummyUpperLeft )
            pCol->Insert( 0, (void*)0 );        // Dummy fuer Beschriftung
        nRowCount = (USHORT) pCol->Count();
    }
    else
        nRowCount = 0;
    if ( nColCount )
        nColCount -= nColAdd;
    if ( nRowCount )
        nRowCount -= nRowAdd;

    if ( nColCount==0 || nRowCount==0 )
    {   // einen Eintrag ohne Daten erzeugen
        pR = aRangeListRef->First();
        if ( pCols->Count() > 0 )
            pCol = (Table*) pCols->First();
        else
        {
            pCol = new Table;
            pCols->Insert( 0, pCol );
        }
        nColCount = 1;
        if ( pCol->Count() > 0 )
        {   // kann ja eigentlich nicht sein, wenn nColCount==0 || nRowCount==0
            pPos = (ScAddress*) pCol->First();
            if ( pPos )
            {
                delete pPos;
                pCol->Replace( pCol->GetCurKey(), (void*)0 );
            }
        }
        else
            pCol->Insert( 0, (void*)0 );
        nRowCount = 1;
        nColAdd = 0;
        nRowAdd = 0;
    }
    else
    {
        if ( bNoGlue )
        {   // Luecken mit Dummies fuellen, erste Spalte ist Master
            Table* pFirstCol = (Table*) pCols->First();
            ULONG nCount = pFirstCol->Count();
            pFirstCol->First();
            for ( ULONG n = 0; n < nCount; n++, pFirstCol->Next() )
            {
                ULONG nKey = pFirstCol->GetCurKey();
                pCols->First();
                while ( pCol = (Table*) pCols->Next() )
                    pCol->Insert( nKey, (void*)0 );     // keine Daten
            }
        }
    }

    pPositionMap = new ScChartPositionMap( nColCount, nRowCount,
        nColAdd, nRowAdd, *pCols );

    //  Aufraeumen
    for ( pCol = (Table*) pCols->First(); pCol; pCol = (Table*) pCols->Next() )
    {   //! nur Tables loeschen, nicht die ScAddress*
        delete pCol;
    }
    delete pCols;
}


ScChartPositionMap::ScChartPositionMap( USHORT nChartCols, USHORT nChartRows,
            USHORT nColAdd, USHORT nRowAdd, Table& rCols ) :
        nCount( (ULONG) nChartCols * nChartRows ),
        nColCount( nChartCols ),
        nRowCount( nChartRows ),
        ppData( new ScAddress* [ nChartCols * nChartRows ] ),
        ppColHeader( new ScAddress* [ nChartCols ] ),
        ppRowHeader( new ScAddress* [ nChartRows ] )
{
    DBG_ASSERT( nColCount && nRowCount, "ScChartPositionMap without dimension" );
#ifdef WIN
#error ScChartPositionMap not implemented for 16-bit dumdums
#endif

    ScAddress* pPos;
    USHORT nCol, nRow;

    Table* pCol = (Table*) rCols.First();

    // Zeilen-Header
    pPos = (ScAddress*) pCol->First();
    if ( nRowAdd )
        pPos = (ScAddress*) pCol->Next();
    if ( nColAdd )
    {   // eigenstaendig
        for ( nRow = 0; nRow < nRowCount; nRow++ )
        {
            ppRowHeader[ nRow ] = pPos;
            pPos = (ScAddress*) pCol->Next();
        }
    }
    else
    {   // Kopie
        for ( nRow = 0; nRow < nRowCount; nRow++ )
        {
            ppRowHeader[ nRow ] = ( pPos ? new ScAddress( *pPos ) : NULL );
            pPos = (ScAddress*) pCol->Next();
        }
    }
    if ( nColAdd )
        pCol = (Table*) rCols.Next();

    // Daten spaltenweise und Spalten-Header
    ULONG nIndex = 0;
    for ( nCol = 0; nCol < nColCount; nCol++ )
    {
        if ( pCol )
        {
            pPos = (ScAddress*) pCol->First();
            if ( nRowAdd )
            {
                ppColHeader[ nCol ] = pPos;     // eigenstaendig
                pPos = (ScAddress*) pCol->Next();
            }
            else
                ppColHeader[ nCol ] = ( pPos ? new ScAddress( *pPos ) : NULL );
            for ( USHORT nRow = 0; nRow < nRowCount; nRow++, nIndex++ )
            {
                ppData[ nIndex ] = pPos;
                pPos = (ScAddress*) pCol->Next();
            }
        }
        else
        {
            ppColHeader[ nCol ] = NULL;
            for ( nRow = 0; nRow < nRowCount; nRow++, nIndex++ )
            {
                ppData[ nIndex ] = NULL;
            }
        }
        pCol = (Table*) rCols.Next();
    }
}


ScChartPositionMap::~ScChartPositionMap()
{
    USHORT j;
    for ( j=0; j < nCount; j++ )
    {
        delete ppData[j];
    }
    delete [] ppData;
    for ( j=0; j < nColCount; j++ )
    {
        delete ppColHeader[j];
    }
    delete [] ppColHeader;
    for ( j=0; j < nRowCount; j++ )
    {
        delete ppRowHeader[j];
    }
    delete [] ppRowHeader;
}


ScRangeListRef ScChartPositionMap::GetColRanges( USHORT nChartCol ) const
{
    ScRangeListRef xRangeList = new ScRangeList;
    if ( nChartCol < nColCount )
    {
        ULONG nStop = GetIndex( nChartCol, nRowCount );
        for ( ULONG nIndex = GetIndex( nChartCol, 0 ); nIndex < nStop; nIndex++ )
        {
            if ( ppData[ nIndex ] )
                xRangeList->Join( *ppData[ nIndex ] );
        }
    }
    return xRangeList;
}


ScRangeListRef ScChartPositionMap::GetRowRanges( USHORT nChartRow ) const
{
    ScRangeListRef xRangeList = new ScRangeList;
    if ( nChartRow < nRowCount )
    {
        ULONG nStop = GetIndex( nColCount, nChartRow );
        for ( ULONG nIndex = GetIndex( 0, nChartRow ); nIndex < nStop;
                nIndex += nRowCount )
        {
            if ( ppData[ nIndex ] )
                xRangeList->Join( *ppData[ nIndex ] );
        }
    }
    return xRangeList;
}


//
//              Collection
//

DataObject* ScChartCollection::Clone() const
{
    return new ScChartCollection(*this);
}

BOOL ScChartCollection::operator==(const ScChartCollection& rCmp) const
{
    if (nCount != rCmp.nCount)
        return FALSE;

    for (USHORT i=0; i<nCount; i++)
        if (!((*(const ScChartArray*)pItems[i]) == (*(const ScChartArray*)rCmp.pItems[i])))
            return FALSE;

    return TRUE;
}

BOOL ScChartCollection::Load( ScDocument* pDoc, SvStream& rStream )
{
    BOOL bSuccess = TRUE;
    USHORT nNewCount;
    FreeAll();

    ScMultipleReadHeader aHdr( rStream );

    rStream >> nNewCount;

    for (USHORT i=0; i<nNewCount && bSuccess; i++)
    {
        ScChartArray* pObject = new ScChartArray( pDoc, rStream, aHdr );
        bSuccess = Insert( pObject );
    }
    return bSuccess;
}

