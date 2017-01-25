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

#include "chartpos.hxx"
#include "document.hxx"
#include "rechead.hxx"
#include <osl/diagnose.h>

#include <memory>

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
            bool bIsDate = (nType & css::util::NumberFormat::DATE);
            bReturn = !bIsDate;
        }
        return bReturn;
    }
}

ScChartPositioner::ScChartPositioner( ScDocument* pDoc, SCTAB nTab,
                    SCCOL nStartColP, SCROW nStartRowP, SCCOL nEndColP, SCROW nEndRowP) :
        pDocument( pDoc ),
        pPositionMap( nullptr ),
        eGlue( ScChartGlue::NA ),
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
        pPositionMap( nullptr ),
        eGlue( ScChartGlue::NA ),
        nStartCol(0),
        nStartRow(0),
        bColHeaders( false ),
        bRowHeaders( false ),
        bDummyUpperLeft( false )
{
    if ( aRangeListRef.is() )
        CheckColRowHeaders();
}

ScChartPositioner::ScChartPositioner( const ScChartPositioner& rPositioner ) :
        aRangeListRef( rPositioner.aRangeListRef ),
        pDocument(rPositioner.pDocument),
        pPositionMap( nullptr ),
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
}

void ScChartPositioner::SetRangeList( const ScRange& rRange )
{
    aRangeListRef = new ScRangeList;
    aRangeListRef->Append( rRange );
    InvalidateGlue();
}

void ScChartPositioner::GlueState()
{
    if ( eGlue != ScChartGlue::NA )
        return;
    bDummyUpperLeft = false;
    ScRange* pR;
    if ( aRangeListRef->size() <= 1 )
    {
        if (  !aRangeListRef->empty() )
        {
            pR = aRangeListRef->front();
            if ( pR->aStart.Tab() == pR->aEnd.Tab() )
                eGlue = ScChartGlue::NONE;
            else
                eGlue = ScChartGlue::Cols;  // several tables column by column
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

    // <= so 1 extra pass after last item
    for ( size_t i = 1, nRanges = aRangeListRef->size(); i <= nRanges; ++i )
    {   // detect spanning/surrounding area etc.
        SCCOLROW nTmp, n1, n2;
        if ( (n1 = pR->aStart.Col()) < nStartCol ) nStartCol = static_cast<SCCOL>(n1  );
        if ( (n2 = pR->aEnd.Col()  ) > nEndCol   ) nEndCol   = static_cast<SCCOL>(n2  );
        if ( (nTmp = n2 - n1 + 1   ) > nMaxCols  ) nMaxCols  = static_cast<SCCOL>(nTmp);
        if ( (n1 = pR->aStart.Row()) < nStartRow ) nStartRow = static_cast<SCROW>(n1  );
        if ( (n2 = pR->aEnd.Row()  ) > nEndRow   ) nEndRow   = static_cast<SCROW>(n2  );
        if ( (nTmp = n2 - n1 + 1   ) > nMaxRows  ) nMaxRows  = static_cast<SCROW>(nTmp);

        // in last pass; i = nRanges so don't use at()
        if ( i < nRanges )
            pR = (*aRangeListRef)[i];
    }
    SCCOL nC = nEndCol - nStartCol + 1;
    if ( nC == 1 )
    {
        eGlue = ScChartGlue::Rows;
        return;
    }
    SCROW nR = nEndRow - nStartRow + 1;
    if ( nR == 1 )
    {
        eGlue = ScChartGlue::Cols;
        return;
    }
    sal_uLong nCR = (sal_uLong)nC * nR;

    /*
    TODO:
    First do it simple without bit masking. A maximum of 8MB could be allocated
    this way (256 Cols x 32000 Rows). That could be reduced to 2MB by
    using 2 Bits per entry, but it is faster this way.
    Another optimization would be to store only used rows/columns in the array, but
    would mean another iteration of the RangeList indirect access to the array. */

    enum class CellState : sal_uInt8 { Hole, Occupied, Free, Glue };
    CellState* p;
    std::unique_ptr<CellState[]> pA(new CellState[ nCR ]);
    memset( pA.get(), 0, nCR * sizeof(CellState) );

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
            p = pA.get() + (sal_uLong)nCol * nR + nRow1;
            for ( nRow = nRow1; nRow <= nRow2; nRow++, p++ )
                *p = CellState::Occupied;
        }
    }
    bool bGlue = true;

    bool bGlueCols = false;
    for ( nCol = 0; bGlue && nCol < nC; nCol++ )
    {   // iterate columns and try to mark as unused
        p = pA.get() + (sal_uLong)nCol * nR;
        for ( nRow = 0; bGlue && nRow < nR; nRow++, p++ )
        {
            if ( *p == CellState::Occupied )
            {   // If there's one right in the middle, we can't combine.
                // If it were at the edge, we could combine, if in this Column
                // in every set line, one is set.
                if ( nRow > 0 && nCol > 0 )
                    bGlue = false; // nCol==0 can be DummyUpperLeft
                else
                    nRow = nR;
            }
            else
                *p = CellState::Free;
        }
        if ( bGlue && *(p = (pA.get() + ((((sal_uLong)nCol+1) * nR) - 1))) == CellState::Free )
        {   // mark column as totally unused
            *p = CellState::Glue;
            bGlueCols = true; // one unused column at least
        }
    }

    bool bGlueRows = false;
    for ( nRow = 0; bGlue && nRow < nR; nRow++ )
    {   // iterate rows and try to mark as unused
        p = pA.get() + nRow;
        for ( nCol = 0; bGlue && nCol < nC; nCol++, p+=nR )
        {
            if ( *p == CellState::Occupied )
            {
                if ( nCol > 0 && nRow > 0 )
                    bGlue = false; // nRow==0 can be DummyUpperLeft
                else
                    nCol = nC;
            }
            else
                *p = CellState::Free;
        }
        if ( bGlue && *(p = (pA.get() + ((((sal_uLong)nC-1) * nR) + nRow))) == CellState::Free )
        {   // mark row as totally unused
            *p = CellState::Glue;
            bGlueRows = true; // one unused row at least
        }
    }

    // If n=1: The upper left corner could be automagically pulled in for labeling
    p = pA.get() + 1;
    for ( sal_uLong n = 1; bGlue && n < nCR; n++, p++ )
    {   // An untouched field means we could neither reach it through rows nor columns,
        // thus we can't combine anything
        if ( *p == CellState::Hole )
            bGlue = false;
    }
    if ( bGlue )
    {
        if ( bGlueCols && bGlueRows )
            eGlue = ScChartGlue::Both;
        else if ( bGlueRows )
            eGlue = ScChartGlue::Rows;
        else
            eGlue = ScChartGlue::Cols;
        if ( pA[0] != CellState::Occupied )
            bDummyUpperLeft = true;
    }
    else
    {
        eGlue = ScChartGlue::NONE;
    }
}

void ScChartPositioner::CheckColRowHeaders()
{
    SCCOL nCol1, nCol2, iCol;
    SCROW nRow1, nRow2, iRow;
    SCTAB nTab1, nTab2;

    bool bColStrings = true;
    bool bRowStrings = true;
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
        bool bVert = (eGlue == ScChartGlue::NONE || eGlue == ScChartGlue::Rows);
        for ( size_t i = 0, nRanges = aRangeListRef->size();
              (i < nRanges) && (bColStrings || bRowStrings);
              ++i
            )
        {
            ScRange* pR = (*aRangeListRef)[i];
            pR->GetVars( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
            bool bTopRow = (nRow1 == nStartRow);
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
    return pPositionMap.get();
}

void ScChartPositioner::CreatePositionMap()
{
    if ( eGlue == ScChartGlue::NA && pPositionMap )
    {
        pPositionMap.reset();
    }

    if ( pPositionMap )
        return ;

    SCSIZE nColAdd = bRowHeaders ? 1 : 0;
    SCSIZE nRowAdd = bColHeaders ? 1 : 0;

    SCCOL nCol, nCol1, nCol2;
    SCROW nRow, nRow1, nRow2;
    SCTAB nTab, nTab1, nTab2;

    //  real size (without hidden rows/columns)

    SCSIZE nColCount = 0;
    SCSIZE nRowCount = 0;

    GlueState();

    const bool bNoGlue = (eGlue == ScChartGlue::NONE);
    std::unique_ptr<ColumnMap> pCols( new ColumnMap );
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
                RowMap* pCol = nullptr;
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
        // For NoGlue: associated tables will be rendered as ColGlue
        nNoGlueRow += nRow2 - nRow1 + 1;
    }

    // count of data
    nColCount = static_cast< SCSIZE >( pCols->size());
    if ( !pCols->empty() )
    {
        RowMap* pCol = pCols->begin()->second;
        if ( bDummyUpperLeft )
            (*pCol)[ 0 ] = nullptr; // Dummy for labeling
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
                (*pCol)[ nCurrentKey ] = nullptr;
            }
        }
        else
            (*pCol)[ 0 ] = nullptr;
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
                    it2->second->insert( RowMap::value_type( nKey, nullptr )); // no data
            }
        }
    }

    pPositionMap.reset( new ScChartPositionMap( static_cast<SCCOL>(nColCount), static_cast<SCROW>(nRowCount),
        static_cast<SCCOL>(nColAdd), static_cast<SCROW>(nRowAdd), *pCols ) );

    //  cleanup
    for (ColumnMap::const_iterator it = pCols->begin(); it != pCols->end(); ++it )
    {   // Only delete tables, not the ScAddress*!
        delete it->second;
    }
}

void ScChartPositioner::InvalidateGlue()
{
    eGlue = ScChartGlue::NA;
    pPositionMap.reset();
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
            ppRowHeader[ nRow ] = nullptr;
    }
    else
    {   // copy
        SCROW nRow = 0;
        for ( ; nRow < nRowCount && pPos1Iter != pCol1->end(); nRow++ )
        {
            ppRowHeader[ nRow ] = pPos1Iter->second ?
                new ScAddress( *pPos1Iter->second ) : nullptr;
            ++pPos1Iter;
        }
        for ( ; nRow < nRowCount; nRow++ )
            ppRowHeader[ nRow ] = nullptr;
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
                    ppColHeader[ nCol ] = pPosIter->second; // independent
                    ++pPosIter;
                }
                else
                    ppColHeader[ nCol ] = pPosIter->second ?
                        new ScAddress( *pPosIter->second ) : nullptr;
            }

            SCROW nRow = 0;
            for ( ; nRow < nRowCount && pPosIter != pCol2->end(); nRow++, nIndex++ )
            {
                ppData[ nIndex ] = pPosIter->second;
                ++pPosIter;
            }
            for ( ; nRow < nRowCount; nRow++, nIndex++ )
                ppData[ nIndex ] = nullptr;

            ++pColIter;
        }
        else
        {
            ppColHeader[ nCol ] = nullptr;
            for ( SCROW nRow = 0; nRow < nRowCount; nRow++, nIndex++ )
            {
                ppData[ nIndex ] = nullptr;
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
