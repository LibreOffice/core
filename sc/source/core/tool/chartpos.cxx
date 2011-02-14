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
#include "precompiled_sc.hxx"

// INCLUDE ---------------------------------------------------------------

#include <tools/table.hxx>

#include "chartpos.hxx"
#include "document.hxx"
#include "rechead.hxx"

namespace
{
    bool lcl_hasValueDataButNoDates( ScDocument* pDocument, SCCOL nCol, SCROW nRow, SCTAB nTab )
    {
        bool bReturn = false;
        if (pDocument->HasValueData( nCol, nRow, nTab ))
        {
            //treat dates like text #i25706#
            sal_uInt32 nNumberFormat = pDocument->GetNumberFormat( ScAddress( nCol, nRow, nTab ) );
            short nType = pDocument->GetFormatTable()->GetType(nNumberFormat);
            bool bIsDate = (nType & NUMBERFORMAT_DATE);
            bReturn = !bIsDate;
        }
        return bReturn;
    }
}

ScChartPositioner::ScChartPositioner( ScDocument* pDoc, SCTAB nTab,
                    SCCOL nStartColP, SCROW nStartRowP, SCCOL nEndColP, SCROW nEndRowP) :
        pDocument( pDoc ),
        pPositionMap( NULL ),
        eGlue( SC_CHARTGLUE_NA ),
        nStartCol(0),
        nStartRow(0),
        bColHeaders( sal_False ),
        bRowHeaders( sal_False ),
        bDummyUpperLeft( sal_False )
{
    SetRangeList( ScRange( nStartColP, nStartRowP, nTab, nEndColP, nEndRowP, nTab ) );
    CheckColRowHeaders();
}

ScChartPositioner::ScChartPositioner( ScDocument* pDoc, const ScRangeListRef& rRangeList ) :
        aRangeListRef( rRangeList ),
        pDocument( pDoc ),
        pPositionMap( NULL ),
        eGlue( SC_CHARTGLUE_NA ),
        nStartCol(0),
        nStartRow(0),
        bColHeaders( sal_False ),
        bRowHeaders( sal_False ),
        bDummyUpperLeft( sal_False )
{
    if ( aRangeListRef.Is() )
        CheckColRowHeaders();
}

ScChartPositioner::ScChartPositioner( const ScChartPositioner& rPositioner ) :
        aRangeListRef( rPositioner.aRangeListRef ),
        pDocument(rPositioner.pDocument),
        pPositionMap( NULL ),
        eGlue(rPositioner.eGlue),
        nStartCol(rPositioner.nStartCol),
        nStartRow(rPositioner.nStartRow),
        bColHeaders(rPositioner.bColHeaders),
        bRowHeaders(rPositioner.bRowHeaders),
        bDummyUpperLeft( rPositioner.bDummyUpperLeft )
{
}

ScChartPositioner::~ScChartPositioner()
{
    delete pPositionMap;
}

sal_Bool ScChartPositioner::operator==(const ScChartPositioner& rCmp) const
{
    return bColHeaders == rCmp.bColHeaders
        && bRowHeaders == rCmp.bRowHeaders
        && *aRangeListRef == *rCmp.aRangeListRef;
}

void ScChartPositioner::SetRangeList( const ScRange& rRange )
{
    aRangeListRef = new ScRangeList;
    aRangeListRef->Append( rRange );
    InvalidateGlue();
}

void ScChartPositioner::GlueState()
{
    if ( eGlue != SC_CHARTGLUE_NA )
        return;
    bDummyUpperLeft = sal_False;
    ScRangePtr pR;
    if ( aRangeListRef->Count() <= 1 )
    {
        if ( (pR = aRangeListRef->First())!=NULL )
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
//  sal_uLong nOldPos = aRangeListRef->GetCurPos();

    pR = aRangeListRef->First();
    nStartCol = pR->aStart.Col();
    nStartRow = pR->aStart.Row();
    SCCOL nMaxCols, nEndCol;
    SCROW nMaxRows, nEndRow;
    nMaxCols = nEndCol = 0;
    nMaxRows = nEndRow = 0;
    do
    {   // umspannenden Bereich etc. feststellen
        SCCOLROW nTmp, n1, n2;
        if ( (n1 = pR->aStart.Col()) < nStartCol )
            nStartCol = static_cast<SCCOL>(n1);
        if ( (n2 = pR->aEnd.Col()) > nEndCol )
            nEndCol = static_cast<SCCOL>(n2);
        if ( (nTmp = n2 - n1 + 1) > nMaxCols )
            nMaxCols = static_cast<SCCOL>(nTmp);
        if ( (n1 = pR->aStart.Row()) < nStartRow )
            nStartRow = static_cast<SCROW>(n1);
        if ( (n2 = pR->aEnd.Row()) > nEndRow )
            nEndRow = static_cast<SCROW>(n2);
        if ( (nTmp = n2 - n1 + 1) > nMaxRows )
            nMaxRows = static_cast<SCROW>(nTmp);
    } while ( (pR = aRangeListRef->Next())!=NULL );
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
    sal_uLong nCR = (sal_uLong)nC * nR;
//2do:
/*
    Erstmal simpel ohne Bitmaskiererei, maximal koennten so 8MB alloziert
    werden (256 Cols mal 32000 Rows), das liesse sich mit 2 Bit je Eintrag
    auf 2MB reduzieren, andererseits ist es so schneller.
    Weitere Platz-Optimierung waere, in dem Array nur die wirklich benutzten
    Zeilen/Spalten abzulegen, wuerde aber ein weiteres durchlaufen der
    RangeList und indirekten Zugriff auf das Array bedeuten.
 */
    const sal_uInt8 nHole = 0;
    const sal_uInt8 nOccu = 1;
    const sal_uInt8 nFree = 2;
    const sal_uInt8 nGlue = 3;
    sal_uInt8* p;
    sal_uInt8* pA = new sal_uInt8[ nCR ];
    memset( pA, 0, nCR * sizeof(sal_uInt8) );

    SCCOL nCol, nCol1, nCol2;
    SCROW nRow, nRow1, nRow2;
    for ( pR = aRangeListRef->First(); pR; pR = aRangeListRef->Next() )
    {   // Selektionen 2D als belegt markieren
        nCol1 = pR->aStart.Col() - nStartCol;
        nCol2 = pR->aEnd.Col() - nStartCol;
        nRow1 = pR->aStart.Row() - nStartRow;
        nRow2 = pR->aEnd.Row() - nStartRow;
        for ( nCol = nCol1; nCol <= nCol2; nCol++ )
        {
            p = pA + (sal_uLong)nCol * nR + nRow1;
            for ( nRow = nRow1; nRow <= nRow2; nRow++, p++ )
                *p = nOccu;
        }
    }
    sal_Bool bGlue = sal_True;

    sal_Bool bGlueCols = sal_False;
    for ( nCol = 0; bGlue && nCol < nC; nCol++ )
    {   // Spalten probieren durchzugehen und als frei markieren
        p = pA + (sal_uLong)nCol * nR;
        for ( nRow = 0; bGlue && nRow < nR; nRow++, p++ )
        {
            if ( *p == nOccu )
            {   // Wenn einer mittendrin liegt ist keine Zusammenfassung
                // moeglich. Am Rand koennte ok sein, wenn in dieser Spalte
                // in jeder belegten Zeile einer belegt ist.
                if ( nRow > 0 && nCol > 0 )
                    bGlue = sal_False;      // nCol==0 kann DummyUpperLeft sein
                else
                    nRow = nR;
            }
            else
                *p = nFree;
        }
        if ( bGlue && *(p = (pA + ((((sal_uLong)nCol+1) * nR) - 1))) == nFree )
        {   // Spalte als komplett frei markieren
            *p = nGlue;
            bGlueCols = sal_True;       // mindestens eine freie Spalte
        }
    }

    sal_Bool bGlueRows = sal_False;
    for ( nRow = 0; bGlue && nRow < nR; nRow++ )
    {   // Zeilen probieren durchzugehen und als frei markieren
        p = pA + nRow;
        for ( nCol = 0; bGlue && nCol < nC; nCol++, p+=nR )
        {
            if ( *p == nOccu )
            {
                if ( nCol > 0 && nRow > 0 )
                    bGlue = sal_False;      // nRow==0 kann DummyUpperLeft sein
                else
                    nCol = nC;
            }
            else
                *p = nFree;
        }
        if ( bGlue && *(p = (pA + ((((sal_uLong)nC-1) * nR) + nRow))) == nFree )
        {   // Zeile als komplett frei markieren
            *p = nGlue;
            bGlueRows = sal_True;       // mindestens eine freie Zeile
        }
    }

    // n=1: die linke obere Ecke koennte bei Beschriftung automagisch
    // hinzugezogen werden
    p = pA + 1;
    for ( sal_uLong n = 1; bGlue && n < nCR; n++, p++ )
    {   // ein unberuehrtes Feld heisst, dass es weder spaltenweise noch
        // zeilenweise zu erreichen war, also nichts zusamenzufassen
        if ( *p == nHole )
            bGlue = sal_False;
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
            bDummyUpperLeft = sal_True;
    }
    else
    {
        eGlue = SC_CHARTGLUE_NONE;
    }

    delete [] pA;
}

void ScChartPositioner::CheckColRowHeaders()
{
    SCCOL nCol1, nCol2, iCol;
    SCROW nRow1, nRow2, iRow;
    SCTAB nTab1, nTab2;

    sal_Bool bColStrings = sal_True;
    sal_Bool bRowStrings = sal_True;
    GlueState();
    if ( aRangeListRef->Count() == 1 )
    {
        aRangeListRef->First()->GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
        if ( nCol1 > nCol2 || nRow1 > nRow2 )
            bColStrings = bRowStrings = sal_False;
        else
        {
            for (iCol=nCol1; iCol<=nCol2 && bColStrings; iCol++)
            {
                if (lcl_hasValueDataButNoDates( pDocument, iCol, nRow1, nTab1 ))
                        bColStrings = sal_False;
            }
            for (iRow=nRow1; iRow<=nRow2 && bRowStrings; iRow++)
            {
                if (lcl_hasValueDataButNoDates( pDocument, nCol1, iRow, nTab1 ))
                        bRowStrings = sal_False;
            }
        }
    }
    else
    {
        sal_Bool bVert = (eGlue == SC_CHARTGLUE_NONE || eGlue == SC_CHARTGLUE_ROWS);
        for ( ScRangePtr pR = aRangeListRef->First();
                pR && (bColStrings || bRowStrings);
                pR = aRangeListRef->Next() )
        {
            pR->GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
            sal_Bool bTopRow = (nRow1 == nStartRow);
            if ( bRowStrings && (bVert || nCol1 == nStartCol) )
            {   // NONE oder ROWS: RowStrings in jeder Selektion moeglich
                // COLS oder BOTH: nur aus der ersten Spalte
                if ( nCol1 <= nCol2 )
                    for (iRow=nRow1; iRow<=nRow2 && bRowStrings; iRow++)
                    {
                        if (lcl_hasValueDataButNoDates( pDocument, nCol1, iRow, nTab1 ))
                                bRowStrings = sal_False;
                    }
            }
            if ( bColStrings && bTopRow )
            {   // ColStrings nur aus der ersten Zeile
                if ( nRow1 <= nRow2 )
                    for (iCol=nCol1; iCol<=nCol2 && bColStrings; iCol++)
                    {
                        if (lcl_hasValueDataButNoDates( pDocument, iCol, nRow1, nTab1 ))
                                bColStrings = sal_False;
                    }
            }
        }
    }
    bColHeaders = bColStrings;
    bRowHeaders = bRowStrings;
}

const ScChartPositionMap* ScChartPositioner::GetPositionMap()
{
    CreatePositionMap();
    return pPositionMap;
}


void ScChartPositioner::CreatePositionMap()
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

    SCCOL nCol, nCol1, nCol2;
    SCROW nRow, nRow1, nRow2;
    SCTAB nTab, nTab1, nTab2;

    //
    //  wirkliche Groesse (ohne versteckte Zeilen/Spalten)
    //

    SCSIZE nColCount = 0;
    SCSIZE nRowCount = 0;

    GlueState();

    sal_Bool bNoGlue = (eGlue == SC_CHARTGLUE_NONE);
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
            sal_uLong nInsCol = (static_cast<sal_uLong>(nTab) << 16) | (bNoGlue ? 0 :
                    static_cast<sal_uLong>(nCol1));
            for ( nCol = nCol1; nCol <= nCol2; ++nCol, ++nInsCol )
            {
                if ( bNoGlue || eGlue == SC_CHARTGLUE_ROWS )
                {   // meistens gleiche Cols
                    if ( (pCol = (Table*) pCols->Get( nInsCol ))==NULL )
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
                sal_uLong nInsRow = (bNoGlue ? nNoGlueRow : nRow1);
                for ( nRow = nRow1; nRow <= nRow2; nRow++, nInsRow++ )
                {
                    if ( pCol->Insert( nInsRow, pNewAddress ) )
                    {
                        pNewAddress->Set( nCol, nRow, nTab );
                        pNewAddress = new ScAddress;
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
    nColCount = static_cast< SCSIZE >( pCols->Count());
    if ( (pCol = (Table*) pCols->First())!=NULL )
    {
        if ( bDummyUpperLeft )
            pCol->Insert( 0, (void*)0 );        // Dummy fuer Beschriftung
        nRowCount = static_cast< SCSIZE >( pCol->Count());
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
            sal_uLong nCount = pFirstCol->Count();
            pFirstCol->First();
            for ( sal_uLong n = 0; n < nCount; n++, pFirstCol->Next() )
            {
                sal_uLong nKey = pFirstCol->GetCurKey();
                pCols->First();
                while ( (pCol = (Table*) pCols->Next())!=NULL )
                    pCol->Insert( nKey, (void*)0 );     // keine Daten
            }
        }
    }

    pPositionMap = new ScChartPositionMap( static_cast<SCCOL>(nColCount), static_cast<SCROW>(nRowCount),
        static_cast<SCCOL>(nColAdd), static_cast<SCROW>(nRowAdd), *pCols );

    //  Aufraeumen
    for ( pCol = (Table*) pCols->First(); pCol; pCol = (Table*) pCols->Next() )
    {   //! nur Tables loeschen, nicht die ScAddress*
        delete pCol;
    }
    delete pCols;
}


ScChartPositionMap::ScChartPositionMap( SCCOL nChartCols, SCROW nChartRows,
            SCCOL nColAdd, SCROW nRowAdd, Table& rCols ) :
        ppData( new ScAddress* [ nChartCols * nChartRows ] ),
        ppColHeader( new ScAddress* [ nChartCols ] ),
        ppRowHeader( new ScAddress* [ nChartRows ] ),
        nCount( (sal_uLong) nChartCols * nChartRows ),
        nColCount( nChartCols ),
        nRowCount( nChartRows )
{
    DBG_ASSERT( nColCount && nRowCount, "ScChartPositionMap without dimension" );

    ScAddress* pPos;
    SCCOL nCol;
    SCROW nRow;

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
    sal_uLong nIndex = 0;
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
    for ( sal_uLong nIndex=0; nIndex < nCount; nIndex++ )
    {
        delete ppData[nIndex];
    }
    delete [] ppData;

    SCCOL j;
    for ( j=0; j < nColCount; j++ )
    {
        delete ppColHeader[j];
    }
    delete [] ppColHeader;
    SCROW i;
    for ( i=0; i < nRowCount; i++ )
    {
        delete ppRowHeader[i];
    }
    delete [] ppRowHeader;
}


//UNUSED2009-05 ScRangeListRef ScChartPositionMap::GetColRanges( SCCOL nChartCol ) const
//UNUSED2009-05 {
//UNUSED2009-05     ScRangeListRef xRangeList = new ScRangeList;
//UNUSED2009-05     if ( nChartCol < nColCount )
//UNUSED2009-05     {
//UNUSED2009-05         sal_uLong nStop = GetIndex( nChartCol, nRowCount );
//UNUSED2009-05         for ( sal_uLong nIndex = GetIndex( nChartCol, 0 ); nIndex < nStop; nIndex++ )
//UNUSED2009-05         {
//UNUSED2009-05             if ( ppData[ nIndex ] )
//UNUSED2009-05                 xRangeList->Join( *ppData[ nIndex ] );
//UNUSED2009-05         }
//UNUSED2009-05     }
//UNUSED2009-05     return xRangeList;
//UNUSED2009-05 }


//UNUSED2009-05 ScRangeListRef ScChartPositionMap::GetRowRanges( SCROW nChartRow ) const
//UNUSED2009-05 {
//UNUSED2009-05     ScRangeListRef xRangeList = new ScRangeList;
//UNUSED2009-05     if ( nChartRow < nRowCount )
//UNUSED2009-05     {
//UNUSED2009-05         sal_uLong nStop = GetIndex( nColCount, nChartRow );
//UNUSED2009-05         for ( sal_uLong nIndex = GetIndex( 0, nChartRow ); nIndex < nStop;
//UNUSED2009-05                 nIndex += nRowCount )
//UNUSED2009-05         {
//UNUSED2009-05             if ( ppData[ nIndex ] )
//UNUSED2009-05                 xRangeList->Join( *ppData[ nIndex ] );
//UNUSED2009-05         }
//UNUSED2009-05     }
//UNUSED2009-05     return xRangeList;
//UNUSED2009-05 }
