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
        bColHeaders( false ),
        bRowHeaders( false ),
        bDummyUpperLeft( false )
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
        bColHeaders( false ),
        bRowHeaders( false ),
        bDummyUpperLeft( false )
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
    bDummyUpperLeft = false;
    ScRange* pR;
    if ( aRangeListRef->size() <= 1 )
    {
        if (  !aRangeListRef->empty() )
        {
            pR = aRangeListRef->front();
            if ( pR->aStart.Tab() == pR->aEnd.Tab() )
                eGlue = SC_CHARTGLUE_NONE;
            else
                eGlue = SC_CHARTGLUE_COLS;  // several tables column by column
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

    pR = aRangeListRef->front();
    nStartCol = pR->aStart.Col();
    nStartRow = pR->aStart.Row();
    SCCOL nMaxCols, nEndCol;
    SCROW nMaxRows, nEndRow;
    nMaxCols = nEndCol = 0;
    nMaxRows = nEndRow = 0;
    for ( size_t i = 1, nRanges = aRangeListRef->size(); i <= nRanges; ++i )     // <= so 1 extra pass after last item
    {   // detect spanning/surrounding area etc.
        SCCOLROW nTmp, n1, n2;
        if ( (n1 = pR->aStart.Col()) < nStartCol ) nStartCol = static_cast<SCCOL>(n1  );
        if ( (n2 = pR->aEnd.Col()  ) > nEndCol   ) nEndCol   = static_cast<SCCOL>(n2  );
        if ( (nTmp = n2 - n1 + 1   ) > nMaxCols  ) nMaxCols  = static_cast<SCCOL>(nTmp);
        if ( (n1 = pR->aStart.Row()) < nStartRow ) nStartRow = static_cast<SCROW>(n1  );
        if ( (n2 = pR->aEnd.Row()  ) > nEndRow   ) nEndRow   = static_cast<SCROW>(n2  );
        if ( (nTmp = n2 - n1 + 1   ) > nMaxRows  ) nMaxRows  = static_cast<SCROW>(nTmp);
        if ( i < nRanges )                      // in last pass; i = nRanges so don't use at()
            pR = (*aRangeListRef)[i];
    }
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
    First do it simple without bit masking. A maximum of 8MB could be allocated
    this way (256 Cols x 32000 Rows). That could be reduced to 2MB by
    using 2 Bits per entry, but it is faster this way.
    Another optimizing would be to store only used rows/columns in the array, but
    would mean another iteration of the RangeList indirect access to the array.
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
    for ( size_t i = 0, nRanges = aRangeListRef->size(); i < nRanges; ++i )
    {   // mark selections as used in 2D
        pR = (*aRangeListRef)[i];
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

    sal_Bool bGlueCols = false;
    for ( nCol = 0; bGlue && nCol < nC; nCol++ )
    {   // iterate columns and try to mark as unused
        p = pA + (sal_uLong)nCol * nR;
        for ( nRow = 0; bGlue && nRow < nR; nRow++, p++ )
        {
            if ( *p == nOccu )
            {   // Wenn einer mittendrin liegt ist keine Zusammenfassung
                // moeglich. Am Rand koennte ok sein, wenn in dieser Spalte
                // in jeder belegten Zeile einer belegt ist.
                if ( nRow > 0 && nCol > 0 )
                    bGlue = false;      // nCol==0 can be DummyUpperLeft
                else
                    nRow = nR;
            }
            else
                *p = nFree;
        }
        if ( bGlue && *(p = (pA + ((((sal_uLong)nCol+1) * nR) - 1))) == nFree )
        {   // mark column as totally unused
            *p = nGlue;
            bGlueCols = sal_True;       // one unused column at least
        }
    }

    sal_Bool bGlueRows = false;
    for ( nRow = 0; bGlue && nRow < nR; nRow++ )
    {   // iterate rows and try to mark as unused
        p = pA + nRow;
        for ( nCol = 0; bGlue && nCol < nC; nCol++, p+=nR )
        {
            if ( *p == nOccu )
            {
                if ( nCol > 0 && nRow > 0 )
                    bGlue = false;      // nRow==0 can be DummyUpperLeft
                else
                    nCol = nC;
            }
            else
                *p = nFree;
        }
        if ( bGlue && *(p = (pA + ((((sal_uLong)nC-1) * nR) + nRow))) == nFree )
        {   // mark row as totally unused
            *p = nGlue;
            bGlueRows = sal_True;       // one unused row at least
        }
    }

    // n=1: die linke obere Ecke koennte bei Beschriftung automagisch
    // hinzugezogen werden
    p = pA + 1;
    for ( sal_uLong n = 1; bGlue && n < nCR; n++, p++ )
    {   // ein unberuehrtes Feld heisst, dass es weder spaltenweise noch
        // zeilenweise zu erreichen war, also nichts zusamenzufassen
        if ( *p == nHole )
            bGlue = false;
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
    if ( aRangeListRef->size() == 1 )
    {
        aRangeListRef->front()->GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
        if ( nCol1 > nCol2 || nRow1 > nRow2 )
            bColStrings = bRowStrings = false;
        else
        {
            for (iCol=nCol1; iCol<=nCol2 && bColStrings; iCol++)
            {
                if (lcl_hasValueDataButNoDates( pDocument, iCol, nRow1, nTab1 ))
                        bColStrings = false;
            }
            for (iRow=nRow1; iRow<=nRow2 && bRowStrings; iRow++)
            {
                if (lcl_hasValueDataButNoDates( pDocument, nCol1, iRow, nTab1 ))
                        bRowStrings = false;
            }
        }
    }
    else
    {
        sal_Bool bVert = (eGlue == SC_CHARTGLUE_NONE || eGlue == SC_CHARTGLUE_ROWS);
        for ( size_t i = 0, nRanges = aRangeListRef->size();
              (i < nRanges) && (bColStrings || bRowStrings);
              ++i
            )
        {
            ScRange* pR = (*aRangeListRef)[i];
            pR->GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
            sal_Bool bTopRow = (nRow1 == nStartRow);
            if ( bRowStrings && (bVert || nCol1 == nStartCol) )
            {   // NONE or ROWS: RowStrings in every selection possible
                // COLS or BOTH: only from first column
                if ( nCol1 <= nCol2 )
                    for (iRow=nRow1; iRow<=nRow2 && bRowStrings; iRow++)
                    {
                        if (lcl_hasValueDataButNoDates( pDocument, nCol1, iRow, nTab1 ))
                                bRowStrings = false;
                    }
            }
            if ( bColStrings && bTopRow )
            {   // ColStrings only from first row
                if ( nRow1 <= nRow2 )
                    for (iCol=nCol1; iCol<=nCol2 && bColStrings; iCol++)
                    {
                        if (lcl_hasValueDataButNoDates( pDocument, iCol, nRow1, nTab1 ))
                                bColStrings = false;
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
    //  real size (without hidden rows/columns)
    //

    SCSIZE nColCount = 0;
    SCSIZE nRowCount = 0;

    GlueState();

    const sal_Bool bNoGlue = (eGlue == SC_CHARTGLUE_NONE);
    ColumnMap* pCols = new ColumnMap;
    SCROW nNoGlueRow = 0;
    for ( size_t i = 0, nRanges = aRangeListRef->size(); i < nRanges; ++i )
    {
        ScRange* pR = (*aRangeListRef)[i];
        pR->GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
        for ( nTab = nTab1; nTab <= nTab2; nTab++ )
        {
            // nTab in ColKey to allow to have the same col/row in another tabe
            sal_uLong nInsCol = (static_cast<sal_uLong>(nTab) << 16) | (bNoGlue ? 0 :
                    static_cast<sal_uLong>(nCol1));
            for ( nCol = nCol1; nCol <= nCol2; ++nCol, ++nInsCol )
            {
                RowMap* pCol = NULL;
                ColumnMap::const_iterator it = pCols->find( nInsCol );
                if ( it == pCols->end() )
                {
                    pCol = new RowMap;
                    pCols->insert( ColumnMap::value_type( nInsCol, pCol ) );
                }
                else
                    pCol = it->second;

                // in other table a new ColKey already was created,
                // the rows must be equal to be filled with Dummy
                sal_uLong nInsRow = (bNoGlue ? nNoGlueRow : nRow1);
                for ( nRow = nRow1; nRow <= nRow2; nRow++, nInsRow++ )
                {
                    if ( pCol->find( nInsRow ) == pCol->end() )
                    {
                        pCol->insert( RowMap::value_type( nInsRow, new ScAddress( nCol, nRow, nTab ) ) );
                    }
                }
            }
        }
        // bei NoGlue werden zusammengehoerige Tabellen als ColGlue dargestellt
        nNoGlueRow += nRow2 - nRow1 + 1;
    }

    // count of data
    nColCount = static_cast< SCSIZE >( pCols->size());
    if ( !pCols->empty() )
    {
        RowMap* pCol = pCols->begin()->second;
        if ( bDummyUpperLeft )
            (*pCol)[ 0 ] = NULL;        // Dummy for labeling
        nRowCount = static_cast< SCSIZE >( pCol->size());
    }
    else
        nRowCount = 0;
    if ( nColCount > 0 )
        nColCount -= nColAdd;
    if ( nRowCount > 0 )
        nRowCount -= nRowAdd;

    if ( nColCount==0 || nRowCount==0 )
    {   // create an entry without data
        RowMap* pCol;
        if ( !pCols->empty() )
            pCol = pCols->begin()->second;
        else
        {
            pCol = new RowMap;
            (*pCols)[ 0 ] = pCol;
        }
        nColCount = 1;
        if ( !pCol->empty() )
        {   // cannot be if nColCount==0 || nRowCount==0
            ScAddress* pPos = pCol->begin()->second;
            if ( pPos )
            {
                sal_uLong nCurrentKey = pCol->begin()->first;
                delete pPos;
                (*pCol)[ nCurrentKey ] = NULL;
            }
        }
        else
            (*pCol)[ 0 ] = NULL;
        nRowCount = 1;
        nColAdd = 0;
        nRowAdd = 0;
    }
    else
    {
        if ( bNoGlue )
        {   // fill gaps with Dummies, first column is master
            RowMap* pFirstCol = pCols->begin()->second;
            sal_uLong nCount = pFirstCol->size();
            RowMap::const_iterator it1 = pFirstCol->begin();
            for ( sal_uLong n = 0; n < nCount; n++, ++it1 )
            {
                sal_uLong nKey = it1->first;
                for (ColumnMap::const_iterator it2 = ++pCols->begin(); it2 != pCols->end(); ++it2 )
                    it2->second->insert( RowMap::value_type( nKey, NULL )); // no data
            }
        }
    }

    pPositionMap = new ScChartPositionMap( static_cast<SCCOL>(nColCount), static_cast<SCROW>(nRowCount),
        static_cast<SCCOL>(nColAdd), static_cast<SCROW>(nRowAdd), *pCols );

    //  cleanup
    for (ColumnMap::const_iterator it = pCols->begin(); it != pCols->end(); ++it )
    {   //! nur Tables loeschen, nicht die ScAddress*
        delete it->second;
    }
    delete pCols;
}


ScChartPositionMap::ScChartPositionMap( SCCOL nChartCols, SCROW nChartRows,
            SCCOL nColAdd, SCROW nRowAdd, ColumnMap& rCols ) :
        ppData( new ScAddress* [ nChartCols * nChartRows ] ),
        ppColHeader( new ScAddress* [ nChartCols ] ),
        ppRowHeader( new ScAddress* [ nChartRows ] ),
        nCount( (sal_uLong) nChartCols * nChartRows ),
        nColCount( nChartCols ),
        nRowCount( nChartRows )
{
    OSL_ENSURE( nColCount && nRowCount, "ScChartPositionMap without dimension" );

    ColumnMap::const_iterator pColIter = rCols.begin();
    RowMap* pCol1 = pColIter->second;
    RowMap::const_iterator pPos1Iter;

    // row header
    pPos1Iter = pCol1->begin();
    if ( nRowAdd )
        ++pPos1Iter;
    if ( nColAdd )
    {   // independent
        SCROW nRow = 0;
        for ( ; nRow < nRowCount && pPos1Iter != pCol1->end(); nRow++ )
        {
            ppRowHeader[ nRow ] = pPos1Iter->second;
            ++pPos1Iter;
        }
        for ( ; nRow < nRowCount; nRow++ )
            ppRowHeader[ nRow ] = NULL;
    }
    else
    {   // copy
        SCROW nRow = 0;
        for ( ; nRow < nRowCount && pPos1Iter != pCol1->end(); nRow++ )
        {
            ppRowHeader[ nRow ] = pPos1Iter->second ?
                new ScAddress( *pPos1Iter->second ) : NULL;
            ++pPos1Iter;
        }
        for ( ; nRow < nRowCount; nRow++ )
            ppRowHeader[ nRow ] = NULL;
    }
    if ( nColAdd )
    {
        ++pColIter;
    }

    // data column by column and column-header
    sal_uLong nIndex = 0;
    for ( SCCOL nCol = 0; nCol < nColCount; nCol++ )
    {
        if ( pColIter != rCols.end() )
        {
            RowMap* pCol2 = pColIter->second;
            RowMap::const_iterator pPosIter = pCol2->begin();
            if ( pPosIter != pCol2->end() )
            {
                if ( nRowAdd )
                {
                    ppColHeader[ nCol ] = pPosIter->second;     // independent
                    ++pPosIter;
                }
                else
                    ppColHeader[ nCol ] = pPosIter->second ?
                        new ScAddress( *pPosIter->second ) : NULL;
            }

            SCROW nRow = 0;
            for ( ; nRow < nRowCount && pPosIter != pCol2->end(); nRow++, nIndex++ )
            {
                ppData[ nIndex ] = pPosIter->second;
                ++pPosIter;
            }
            for ( ; nRow < nRowCount; nRow++, nIndex++ )
                ppData[ nIndex ] = NULL;

            ++pColIter;
        }
        else
        {
            ppColHeader[ nCol ] = NULL;
            for ( SCROW nRow = 0; nRow < nRowCount; nRow++, nIndex++ )
            {
                ppData[ nIndex ] = NULL;
            }
        }
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
