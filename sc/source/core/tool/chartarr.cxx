/*************************************************************************
 *
 *  $RCSfile: chartarr.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 10:33:19 $
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

ScChartArray::ScChartArray( ScDocument* pDoc, SCTAB nTab,
                    SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
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
#if SC_ROWLIMIT_STREAM_ACCESS
#error address types changed!
    SCCOL nCol2;
    SCROW nRow2;
    SCTAB nTable;

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
#endif // SC_ROWLIMIT_STREAM_ACCESS
}

ScChartArray::ScChartArray( ScDocument* pDoc, const SchMemChart& rData ) :
        pDocument( pDoc ),
        pPositionMap( NULL )
{
    BOOL bInitOk = bValid = FALSE;
    const SchChartRange& rChartRange = rData.GetChartRange();
    ::std::vector< SchCellRangeAddress >::const_iterator iRange =
        rChartRange.maRanges.begin();
    if ( iRange != rChartRange.maRanges.end() )
    {   // new SO6 chart format
        bValid = TRUE;
        bColHeaders = rChartRange.mbFirstRowContainsLabels;
        bRowHeaders = rChartRange.mbFirstColumnContainsLabels;
        aRangeListRef = new ScRangeList;
        for ( ; iRange != rChartRange.maRanges.end(); ++iRange )
        {
            const SchSingleCell& rAddr1 = (*iRange).maUpperLeft.maCells[0];
            const SchSingleCell& rAddr2 = (*iRange).maLowerRight.maCells[0];
            SCTAB nTab = (SCTAB) (*iRange).mnTableNumber;
            ScRange aRange(
                (SCCOL) rAddr1.mnColumn, (SCROW) rAddr1.mnRow, nTab,
                (SCCOL) rAddr2.mnColumn, (SCROW) rAddr2.mnRow, nTab );
            aRangeListRef->Append( aRange );
        }
    }
    else
    {   // old SO5 chart format
        //! A similar routine is implemented in
        //! SchMemChart::ConvertChartRangeForCalc() for OldToNew. If anything
        //! is changed here it propably must be changed there too!
        const sal_Unicode cTok = ';';
        xub_StrLen nToken;
        String aPos = ((SchMemChart&)rData).SomeData1();
        if ( (nToken = aPos.GetTokenCount( cTok )) >= 5)
        {
            String aOpt = ((SchMemChart&)rData).SomeData2();
            xub_StrLen nOptToken = aOpt.GetTokenCount( cTok );
            BOOL bNewChart = (nOptToken >= 4);      // as of 341/342
            SCCOL nCol1;
            SCROW nRow1;
            SCTAB nTab1;
            SCCOL nCol2;
            SCROW nRow2;
            SCTAB nTab2;
            xub_StrLen nInd = 0;
            for ( xub_StrLen j=0; j < nToken; j+=5 )
            {
                xub_StrLen nInd2 = nInd;
                nTab1 = (SCTAB) aPos.GetToken( 0, cTok, nInd ).ToInt32();
                // To make old versions (<341/342) skip it, the token separator
                // is a ','
                if ( bNewChart )
                    nTab2 = (SCTAB) aPos.GetToken( 1, ',', nInd2 ).ToInt32();
                else
                    nTab2 = nTab1;
                nCol1 = (SCCOL) aPos.GetToken( 0, cTok, nInd ).ToInt32();
                nRow1 = (SCROW) aPos.GetToken( 0, cTok, nInd ).ToInt32();
                nCol2 = (SCCOL) aPos.GetToken( 0, cTok, nInd ).ToInt32();
                nRow2 = (SCROW) aPos.GetToken( 0, cTok, nInd ).ToInt32();
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
                        nStartCol = (SCCOL) aOpt.GetToken( 0, cTok, nInd ).ToInt32();
                        nStartRow = (SCROW) aOpt.GetToken( 0, cTok, nInd ).ToInt32();
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
    }
    if ( !bInitOk )
    {   // muessen in GlueState neu berechnet werden
        InvalidateGlue();
        nStartCol = 0;
        nStartRow = 0;
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
    aRangeListRef = new ScRangeList;
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
            nStartCol = 0;
            nStartRow = 0;
        }
        return;
    }
    ULONG nOldPos = aRangeListRef->GetCurPos();

    pR = aRangeListRef->First();
    nStartCol = pR->aStart.Col();
    nStartRow = pR->aStart.Row();
    SCCOL nMaxCols, nEndCol;
    SCROW nMaxRows, nEndRow;
    nMaxCols = nEndCol = 0;
    nMaxRows = nEndRow = 0;
    do
    {   // umspannenden Bereich etc. feststellen
        SCCOL nColTmp, nCol1, nCol2;
        if ( (nCol1 = pR->aStart.Col()) < nStartCol )
            nStartCol = nCol1;
        if ( (nCol2 = pR->aEnd.Col()) > nEndCol )
            nEndCol = nCol2;
        if ( (nColTmp = nCol2 - nCol1 + 1) > nMaxCols )
            nMaxCols = nColTmp;
        SCROW nRowTmp, nRow1, nRow2;
        if ( (nRow1 = pR->aStart.Row()) < nStartRow )
            nStartRow = nRow1;
        if ( (nRow2 = pR->aEnd.Row()) > nEndRow )
            nEndRow = nRow2;
        if ( (nRowTmp = nRow2 - nRow1 + 1) > nMaxRows )
            nMaxRows = nRowTmp;
    } while ( pR = aRangeListRef->Next() );
    SCCOL nC = nEndCol - nStartCol + 1;
    if ( nC == 1 )
    {
        eGlue = SC_CHARTGLUE_ROWS;
        return;
    }
    SCROW nR = nEndRow - nStartRow + 1;
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

    SCCOL nCol;
    SCROW  nRow;
    SCCOL  nCol1;
    SCROW  nRow1;
    SCCOL  nCol2;
    SCROW  nRow2;
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
    SCCOL nCol1;
    SCROW nRow1;
    SCTAB nTab1;
    SCCOL nCol2;
    SCROW nRow2;
    SCTAB nTab2;

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
            for (SCCOL i=nCol1; i<=nCol2 && bColStrings; i++)
            {
                if ( i==nCol1 || (pDocument->GetColFlags( i, nTab1) & CR_HIDDEN) == 0 )
                    if (pDocument->HasValueData( i, nRow1, nTab1 ))
                        bColStrings = FALSE;
            }
            for (SCROW j=nRow1; j<=nRow2 && bRowStrings; j++)
            {
                if ( j==nRow1 || (pDocument->GetRowFlags( j, nTab1) & CR_HIDDEN) == 0 )
                    if (pDocument->HasValueData( nCol1, j, nTab1 ))
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
                    for (SCROW i=nRow1; i<=nRow2 && bRowStrings; i++)
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
                    for (SCCOL i=nCol1; i<=nCol2 && bColStrings; i++)
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

SchMemChart* ScChartArray::CreateMemChartSingle()
{
    SCSIZE nCol;
    SCSIZE nRow;

        //
        //  wirkliche Groesse (ohne versteckte Zeilen/Spalten)
        //

    SCCOL nColAdd = bRowHeaders ? 1 : 0;
    SCROW nRowAdd = bColHeaders ? 1 : 0;

    SCCOL nCol1;
    SCROW nRow1;
    SCTAB nTab1;
    SCCOL nCol2;
    SCROW nRow2;
    SCTAB nTab2;
    aRangeListRef->First()->GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );

    SCCOL nStrCol = nCol1;      // fuer Beschriftung merken
    SCROW nStrRow = nRow1;
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

    SCSIZE nTotalCols = ( nCol1 <= nCol2 ? nCol2 - nCol1 + 1 : 0 );
    SCCOL* pCols = new SCCOL[nTotalCols > 0 ? nTotalCols : 1];
    SCSIZE nColCount = 0;
    for (SCSIZE i=0; i<nTotalCols; i++)
        if ((pDocument->GetColFlags(nCol1+i,nTab1)&CR_HIDDEN)==0)
            pCols[nColCount++] = nCol1+i;

    SCSIZE nTotalRows = ( nRow1 <= nRow2 ? nRow2 - nRow1 + 1 : 0 );
    SCROW* pRows = new SCROW[nTotalRows > 0 ? nTotalRows : 1];
    SCSIZE nRowCount = 0;
    for (SCSIZE j=0; j<nTotalRows; j++)
        if ((pDocument->GetRowFlags(nRow1+j,nTab1)&CR_HIDDEN)==0)
            pRows[nRowCount++] = nRow1+j;

    // May happen at least with more than 32k rows.
    if (nColCount > SHRT_MAX || nRowCount > SHRT_MAX)
    {
        nColCount = 0;
        nRowCount = 0;
    }

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

    SchMemChart* pMemChart = SchDLL::NewMemChart(
            static_cast<short>(nColCount), static_cast<short>(nRowCount) );
    if (pMemChart)
    {
        SvNumberFormatter* pFormatter = pDocument->GetFormatTable();
        pMemChart->SetNumberFormatter( pFormatter );
        if ( bValidData )
        {
            BOOL bCalcAsShown = pDocument->GetDocOptions().IsCalcAsShown();
            ScBaseCell* pCell;
            for (nCol=0; nCol<nColCount; nCol++)
            {
                for (nRow=0; nRow<nRowCount; nRow++)
                {
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
                    pMemChart->SetData(static_cast<short>(nCol), static_cast<short>(nRow), nVal);
                }
            }
        }
        else
        {
            //! Flag, dass Daten ungueltig ??

            for (nCol=0; nCol<nColCount; nCol++)
                for (nRow=0; nRow<nRowCount; nRow++)
                    pMemChart->SetData( static_cast<short>(nCol), static_cast<short>(nRow), DBL_MIN );
        }

        //
        //  Spalten-Header
        //

        for (nCol=0; nCol<nColCount; nCol++)
        {
            String aString, aColStr;
            if (bColHeaders)
                pDocument->GetString( pCols[nCol], nStrRow, nTab1, aString );
            if ( !aString.Len() )
            {
                aString = ScGlobal::GetRscString(STR_COLUMN);
                aString += ' ';
//                aString += String::CreateFromInt32( pCols[nCol]+1 );
                ScAddress aPos( pCols[ nCol ], 0, 0 );
                aPos.Format( aColStr, SCA_VALID_COL, NULL );
                aString += aColStr;
            }
            pMemChart->SetColText( static_cast<short>(nCol), aString);

            ULONG nNumberAttr = pDocument->GetNumberFormat( ScAddress(
                                            pCols[nCol], nRow1, nTab1 ) );
            pMemChart->SetNumFormatIdCol( static_cast<long>(nCol), nNumberAttr );
        }

        //
        //  Zeilen-Header
        //

        for (nRow=0; nRow<nRowCount; nRow++)
        {
            String aString;
            if (bRowHeaders)
            {
                ScAddress aAddr( nStrCol, pRows[nRow], nTab1 );
                pDocument->GetString( nStrCol, pRows[nRow], nTab1, aString );
            }
            if ( !aString.Len() )
            {
                aString = ScGlobal::GetRscString(STR_ROW);
                aString += ' ';
                aString += String::CreateFromInt32( pRows[nRow]+1 );
            }
            pMemChart->SetRowText( static_cast<short>(nRow), aString);

            ULONG nNumberAttr = pDocument->GetNumberFormat( ScAddress(
                                            nCol1, pRows[nRow], nTab1 ) );
            pMemChart->SetNumFormatIdRow( static_cast<long>(nRow), nNumberAttr );
        }

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

        SetExtraStrings( *pMemChart );
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
    SCSIZE nColCount = pPositionMap->GetColCount();
    SCSIZE nRowCount = pPositionMap->GetRowCount();

    SCSIZE nCol;
    SCSIZE nRow;

    // May happen at least with more than 32k rows.
    if (nColCount > SHRT_MAX || nRowCount > SHRT_MAX)
    {
        nColCount = 0;
        nRowCount = 0;
    }

    BOOL bValidData = TRUE;
    if ( !nColCount )
    {
        bValidData = FALSE;
        nColCount = 1;
    }
    if ( !nRowCount )
    {
        bValidData = FALSE;
        nRowCount = 1;
    }

    //
    //  Daten
    //

    SchMemChart* pMemChart = SchDLL::NewMemChart(
            static_cast<short>(nColCount), static_cast<short>(nRowCount) );
    if (pMemChart)
    {
        pMemChart->SetNumberFormatter( pDocument->GetFormatTable() );
        if (bValidData)
        {
            BOOL bCalcAsShown = pDocument->GetDocOptions().IsCalcAsShown();
            ULONG nIndex = 0;
            for ( nCol = 0; nCol < nColCount; nCol++ )
            {
                for ( nRow = 0; nRow < nRowCount; nRow++, nIndex++ )
                {
                    double nVal = DBL_MIN;      // Hack fuer Chart, um leere Zellen zu erkennen
                    const ScAddress* pPos = pPositionMap->GetPosition( nIndex );
                    if ( pPos )
                    {   // sonst: Luecke
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
                    pMemChart->SetData(static_cast<short>(nCol), static_cast<short>(nRow), nVal);
                }
            }
        }
        else
        {
            //! Flag, dass Daten ungueltig ??

            for (nCol=0; nCol<nColCount; nCol++)
                for (nRow=0; nRow<nRowCount; nRow++)
                    pMemChart->SetData( static_cast<short>(nCol), static_cast<short>(nRow), DBL_MIN );
        }

//2do: Beschriftung bei Luecken

        //
        //  Spalten-Header
        //

        SCCOL nPosCol = 0;
        for ( nCol = 0; nCol < nColCount; nCol++ )
        {
            String aString, aColStr;
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
                ScAddress aPos( nPosCol - 1, 0, 0 );
                aPos.Format( aColStr, SCA_VALID_COL, NULL );
//                aString += String::CreateFromInt32( nPosCol );
                aString += aColStr;
            }
            pMemChart->SetColText( static_cast<short>(nCol), aString);

            ULONG nNumberAttr = 0;
            pPos = pPositionMap->GetPosition( nCol, 0 );
            if ( pPos )
                nNumberAttr = pDocument->GetNumberFormat( *pPos );
            pMemChart->SetNumFormatIdCol( static_cast<long>(nCol), nNumberAttr );
        }

        //
        //  Zeilen-Header
        //

        SCROW nPosRow = 0;
        for ( nRow = 0; nRow < nRowCount; nRow++ )
        {
            String aString;
            const ScAddress* pPos = pPositionMap->GetRowHeaderPosition( nRow );
            if ( bRowHeaders && pPos )
            {
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
            pMemChart->SetRowText( static_cast<short>(nRow), aString);

            ULONG nNumberAttr = 0;
            pPos = pPositionMap->GetPosition( 0, nRow );
            if ( pPos )
                nNumberAttr = pDocument->GetNumberFormat( *pPos );
            pMemChart->SetNumFormatIdRow( static_cast<long>(nRow), nNumberAttr );
        }

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

        SetExtraStrings( *pMemChart );
    }
    else
        DBG_ERROR("SchDLL::NewMemChart gibt 0 zurueck!");

    return pMemChart;
}

void ScChartArray::SetExtraStrings( SchMemChart& rMem )
{
    ScRangePtr pR;
    SCCOL nCol1;
    SCROW nRow1;
    SCTAB nTab1;
    SCCOL nCol2;
    SCROW nRow2;
    SCTAB nTab2;
#if 0
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
#endif

    String aSheetNames;
    SchChartRange aChartRange;
    aChartRange.mbFirstColumnContainsLabels = bRowHeaders;
    aChartRange.mbFirstRowContainsLabels = bColHeaders;
    aChartRange.mbKeepCopyOfData = sal_False;
    for ( pR = aRangeListRef->First(); pR; pR = aRangeListRef->Next() )
    {
        pR->GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
        for ( SCTAB nTab = nTab1; nTab <= nTab2; ++nTab )
        {
            SchCellRangeAddress aCellRangeAddress;
            SchSingleCell aCell;
            aCell.mnColumn = nCol1;
            aCell.mnRow = nRow1;
            aCellRangeAddress.maUpperLeft.maCells.push_back( aCell );
            aCell.mnColumn = nCol2;
            aCell.mnRow = nRow2;
            aCellRangeAddress.maLowerRight.maCells.push_back( aCell );
            aCellRangeAddress.mnTableNumber = nTab;
            String aName;
            pDocument->GetName( nTab, aName );
            aCellRangeAddress.msTableName = aName;
            aChartRange.maRanges.push_back( aCellRangeAddress );
            if ( aSheetNames.Len() )
                aSheetNames += ';';
            aSheetNames += aName;
        }
    }
    rMem.SetChartRange( aChartRange );

    // #90896# need that for OLE and clipboard of old binary file format
    rMem.SomeData3() = aSheetNames;

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

    SCSIZE nColAdd = bRowHeaders ? 1 : 0;
    SCSIZE nRowAdd = bColHeaders ? 1 : 0;

    SCCOL nCol;
    SCROW nRow;
    SCTAB nTab;
    SCCOL nCol1;
    SCROW nRow1;
    SCTAB nTab1;
    SCCOL nCol2;
    SCROW nRow2;
    SCTAB nTab2;

    //
    //  wirkliche Groesse (ohne versteckte Zeilen/Spalten)
    //

    SCSIZE nColCount = 0;
    SCSIZE nRowCount = 0;

    GlueState();

    BOOL bNoGlue = (eGlue == SC_CHARTGLUE_NONE);
    Table* pCols = new Table;
    Table* pNewRowTable = new Table;
    ScAddress* pNewAddress = new ScAddress;
    ScRangePtr pR;
    Table* pCol;
    ScAddress* pPos;
    SCROW nNoGlueRow = 0;
    for ( pR = aRangeListRef->First(); pR; pR = aRangeListRef->Next() )
    {
        pR->GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
        for ( nTab = nTab1; nTab <= nTab2; nTab++ )
        {
            // nTab im ColKey, um gleiche Col/Row in anderer Table haben zu koennen
            ULONG nInsCol = (static_cast<ULONG>(nTab) << 16) | (bNoGlue ? 0 :
                    static_cast<ULONG>(nCol1));
            for ( nCol = nCol1; nCol <= nCol2; ++nCol, ++nInsCol )
            {
                if ( (pDocument->GetColFlags( nCol, nTab) & CR_HIDDEN) == 0 )
                {
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
    nColCount = pCols->Count();
    if ( pCol = (Table*) pCols->First() )
    {
        if ( bDummyUpperLeft )
            pCol->Insert( 0, (void*)0 );        // Dummy fuer Beschriftung
        nRowCount = pCol->Count();
    }
    else
        nRowCount = 0;
    if ( nColCount > 0 )
        nColCount -= nColAdd;
    if ( nRowCount > 0 )
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


ScChartPositionMap::ScChartPositionMap( SCSIZE nChartCols, SCSIZE nChartRows,
            SCSIZE nColAdd, SCSIZE nRowAdd, Table& rCols ) :
        nCount( nChartCols * nChartRows ),
        nColCount( nChartCols ),
        nRowCount( nChartRows ),
        ppData( new ScAddress* [ nChartCols * nChartRows ] ),
        ppColHeader( new ScAddress* [ nChartCols ] ),
        ppRowHeader( new ScAddress* [ nChartRows ] )
{
    DBG_ASSERT( nColCount > 0 && nRowCount > 0, "ScChartPositionMap without dimension" );
#ifdef WIN
#error ScChartPositionMap not implemented for 16-bit dumdums
#endif

    ScAddress* pPos;
    SCSIZE nCol;
    SCSIZE nRow;

    Table* pCol = (Table*) rCols.First();

    // Zeilen-Header
    pPos = (ScAddress*) pCol->First();
    if ( nRowAdd > 0 )
        pPos = (ScAddress*) pCol->Next();
    if ( nColAdd > 0 )
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
    if ( nColAdd > 0 )
        pCol = (Table*) rCols.Next();

    // Daten spaltenweise und Spalten-Header
    SCSIZE nIndex = 0;
    for ( nCol = 0; nCol < nColCount; nCol++ )
    {
        if ( pCol )
        {
            pPos = (ScAddress*) pCol->First();
            if ( nRowAdd > 0 )
            {
                ppColHeader[ nCol ] = pPos;     // eigenstaendig
                pPos = (ScAddress*) pCol->Next();
            }
            else
                ppColHeader[ nCol ] = ( pPos ? new ScAddress( *pPos ) : NULL );
            for ( nRow = 0; nRow < nRowCount; nRow++, nIndex++ )
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
    for ( ULONG nIndex=0; nIndex < nCount; nIndex++ )
    {
        delete ppData[nIndex];
    }
    delete [] ppData;

    USHORT j;
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


ScRangeListRef ScChartPositionMap::GetColRanges( SCSIZE nChartCol ) const
{
    ScRangeListRef xRangeList = new ScRangeList;
    if ( nChartCol < nColCount )
    {
        SCSIZE nStop = GetIndex( nChartCol, nRowCount );
        for ( SCSIZE nIndex = GetIndex( nChartCol, 0 ); nIndex < nStop; nIndex++ )
        {
            if ( ppData[ nIndex ] )
                xRangeList->Join( *ppData[ nIndex ] );
        }
    }
    return xRangeList;
}


ScRangeListRef ScChartPositionMap::GetRowRanges( SCSIZE nChartRow ) const
{
    ScRangeListRef xRangeList = new ScRangeList;
    if ( nChartRow < nRowCount )
    {
        SCSIZE nStop = GetIndex( nColCount, nChartRow );
        for ( SCSIZE nIndex = GetIndex( 0, nChartRow ); nIndex < nStop;
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

