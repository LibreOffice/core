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

#include "markmulti.hxx"
#include "markarr.hxx"
#include "segmenttree.hxx"

#include <algorithm>

ScMultiSel::ScMultiSel( SCCOL nCols ):
    aMultiSelContainer( static_cast<size_t>( nCols ), nullptr ),
    aRowSel()
{
    for ( size_t nColIter = 0, nSize = aMultiSelContainer.size(); nColIter < nSize; ++nColIter )
        aMultiSelContainer[nColIter] = new ScMarkArray;
}

ScMultiSel::ScMultiSel( const ScMultiSel& rMultiSel ):
    aMultiSelContainer( rMultiSel.aMultiSelContainer.size() )
{
    for ( size_t nColIter = 0, nSize = aMultiSelContainer.size(); nColIter < nSize; ++nColIter )
    {
        aMultiSelContainer[nColIter] = new ScMarkArray;
        rMultiSel.aMultiSelContainer[nColIter]->CopyMarksTo( *aMultiSelContainer[nColIter] );
    }
    rMultiSel.aRowSel.CopyMarksTo( aRowSel );
}

ScMultiSel::~ScMultiSel()
{
    Clear();
}

ScMultiSel& ScMultiSel::operator=(const ScMultiSel& rMultiSel)
{
    Clear();
    aMultiSelContainer.resize( rMultiSel.aMultiSelContainer.size() );
    for ( size_t nColIter = 0, nSize = aMultiSelContainer.size(); nColIter < nSize; ++nColIter )
    {
        aMultiSelContainer[nColIter] = new ScMarkArray;
        rMultiSel.aMultiSelContainer[nColIter]->CopyMarksTo( *aMultiSelContainer[nColIter] );
    }
    rMultiSel.aRowSel.CopyMarksTo( aRowSel );
    return *this;
}

void ScMultiSel::CreateCol( SCCOL nCol )
{
    if ( nCol < size() )
        return;

    size_t nNewSize = static_cast<size_t>( nCol ) + 1;
    size_t nOldSize = aMultiSelContainer.size();
    aMultiSelContainer.resize( nNewSize );
    for ( size_t nColIter = nOldSize; nColIter < nNewSize; ++nColIter )
        aMultiSelContainer[nColIter] = new ScMarkArray;
}

void ScMultiSel::Clear()
{
    for ( size_t nColIter = 0, nSize = aMultiSelContainer.size(); nColIter < nSize; ++nColIter )
        delete aMultiSelContainer[nColIter];
    aMultiSelContainer.clear();
    aRowSel.Reset( false );
}

bool ScMultiSel::HasMarks( SCCOL nCol ) const
{
    if ( aRowSel.HasMarks() )
        return true;
    if ( nCol >= size() )
        return false;
    return aMultiSelContainer[nCol]->HasMarks();
}

bool ScMultiSel::HasOneMark( SCCOL nCol, SCROW& rStartRow, SCROW& rEndRow ) const
{
    bool aResult1 = false, aResult2 = false;
    SCROW nRow1, nRow2, nRow3, nRow4;
    aResult1 = aRowSel.HasOneMark( nRow1, nRow2 );
    if ( nCol < size() )
        aResult2 = aMultiSelContainer[nCol]->HasOneMark( nRow3, nRow4 );

    if ( aResult1 || aResult2 )
    {
        if ( aResult1 && aResult2 )
        {
            if ( ( nRow2 + 1 ) < nRow3 )
                return false;
            if ( ( nRow4 + 1 ) < nRow1 )
                return false;

            auto aRows = std::minmax( { nRow1, nRow2, nRow3, nRow4 } );
            rStartRow = aRows.first;
            rEndRow = aRows.second;
            return true;
        }
        if ( aResult1 )
        {
            rStartRow = nRow1;
            rEndRow = nRow2;
            return true;
        }

        rStartRow = nRow3;
        rEndRow = nRow4;
        return true;
    }

    return false;
}

bool ScMultiSel::GetMark( SCCOL nCol, SCROW nRow ) const
{
    if ( aRowSel.GetMark( nRow ) )
        return true;
    if ( nCol < size() )
        return aMultiSelContainer[nCol]->GetMark( nRow );
    return false;
}

bool ScMultiSel::IsAllMarked( SCCOL nCol, SCROW nStartRow, SCROW nEndRow ) const
{
    bool bHasMarks1 = aRowSel.HasMarks();
    bool bHasMarks2 = ( nCol < size() && aMultiSelContainer[nCol]->HasMarks() );

    if ( !bHasMarks1 && !bHasMarks2 )
        return false;

    if ( bHasMarks1 && bHasMarks2 )
    {
        if ( aRowSel.IsAllMarked( nStartRow, nEndRow ) ||
             aMultiSelContainer[nCol]->IsAllMarked( nStartRow, nEndRow ) )
            return true;
        ScMultiSelIter aMultiIter( *this, nCol );
        ScFlatBoolRowSegments::RangeData aRowRange;
        aMultiIter.GetRowSegments().getRangeData( nStartRow, aRowRange );
        return ( aRowRange.mbValue && aRowRange.mnRow2 >= nEndRow );
    }

    if ( bHasMarks1 )
        return aRowSel.IsAllMarked( nStartRow, nEndRow );

    return aMultiSelContainer[nCol]->IsAllMarked( nStartRow, nEndRow );
}

bool ScMultiSel::HasEqualRowsMarked( SCCOL nCol1, SCCOL nCol2 ) const
{
    bool bCol1Exists = ( nCol1 < size() );
    bool bCol2Exists = ( nCol2 < size() );
    if ( bCol1Exists || bCol2Exists )
    {
        if ( bCol1Exists && bCol2Exists )
            return aMultiSelContainer[nCol1]->HasEqualRowsMarked( *aMultiSelContainer[nCol2] );
        else if ( bCol1Exists )
            return !aMultiSelContainer[nCol1]->HasMarks();
        else
            return !aMultiSelContainer[nCol2]->HasMarks();
    }

    return true;
}

SCsROW ScMultiSel::GetNextMarked( SCCOL nCol, SCsROW nRow, bool bUp ) const
{
    if ( nCol >= size() )
        return aRowSel.GetNextMarked( nRow, bUp );

    SCsROW nRow1, nRow2;
    nRow1 = aRowSel.GetNextMarked( nRow, bUp );
    nRow2 = aMultiSelContainer[nCol]->GetNextMarked( nRow, bUp );
    if ( nRow1 == nRow2 )
        return nRow1;
    if ( nRow1 == -1 )
        return nRow2;
    if ( nRow2 == -1 )
        return nRow1;

    PutInOrder( nRow1, nRow2 );
    return ( bUp ? nRow2 : nRow1 );
}

void ScMultiSel::MarkAllCols( SCROW nStartRow, SCROW nEndRow )
{
    if ( size() <= MAXCOL )
        CreateCol( MAXCOL );
    for ( SCCOL nCol = 0; nCol <= MAXCOL; ++nCol )
        aMultiSelContainer[nCol]->SetMarkArea( nStartRow, nEndRow, true );
}

void ScMultiSel::SetMarkArea( SCCOL nStartCol, SCCOL nEndCol, SCROW nStartRow, SCROW nEndRow, bool bMark )
{
    if ( nStartCol == 0 && nEndCol == MAXCOL )
    {
        aRowSel.SetMarkArea( nStartRow, nEndRow, bMark );
        if ( !bMark )
        {
            // Remove any per column marks for the row range.
            for ( SCCOL nCol = 0, nSize = size(); nCol < nSize; ++nCol )
                if ( aMultiSelContainer[nCol]->HasMarks() )
                    aMultiSelContainer[nCol]->SetMarkArea( nStartRow, nEndRow, false );
        }
        return;
    }

    // Bad case - we need to extend aMultiSelContainer size to MAXCOL
    // and move row marks from aRowSel to aMultiSelContainer
    if ( !bMark && aRowSel.HasMarks() )
    {
        SCROW nBeg, nLast = nEndRow + 1;
        if ( aRowSel.GetMark( nStartRow ) )
        {
            nBeg = nStartRow;
            nLast = aRowSel.GetMarkEnd( nStartRow, false );
        }
        else
        {
            nBeg = aRowSel.GetNextMarked( nStartRow, false );
            if ( nBeg != -1 )
                nLast = aRowSel.GetMarkEnd( nBeg, false );
        }

        if ( nBeg != -1 && nLast >= nEndRow )
            MarkAllCols( nBeg, nEndRow );
        else
        {
            while ( nBeg != -1 && nLast < nEndRow )
            {
                MarkAllCols( nBeg, nLast );
                nBeg = aRowSel.GetNextMarked( nLast + 1, false );
                if ( nBeg != -1 )
                    nLast = aRowSel.GetMarkEnd( nBeg, false );
            }
            if ( nBeg != -1 && nLast >= nEndRow )
                MarkAllCols( nBeg, nEndRow );
        }

        aRowSel.SetMarkArea( nStartRow, nEndRow, false );
    }

    if ( nEndCol >= size() )
        CreateCol( nEndCol );

    for ( SCCOL nColIter = nStartCol; nColIter <= nEndCol; ++nColIter )
        aMultiSelContainer[nColIter]->SetMarkArea( nStartRow, nEndRow, bMark );
}

bool ScMultiSel::IsRowMarked( SCROW nRow ) const
{
    return aRowSel.GetMark( nRow );
}

bool ScMultiSel::IsRowRangeMarked( SCROW nStartRow, SCROW nEndRow ) const
{
    bool bStartRowMarked = aRowSel.GetMark( nStartRow );
    if ( !bStartRowMarked )
        return false;
    SCROW nLast = aRowSel.GetMarkEnd( nStartRow, false );
    return ( nLast >= nEndRow );
}

ScMarkArray* ScMultiSel::GetMarkArray( SCCOL nCol ) const
{
    ScMultiSelIter aMultiIter( *this, nCol );
    ScMarkArray* pMarkArray = new ScMarkArray;
    SCROW nTop, nBottom;
    while( aMultiIter.Next( nTop, nBottom ) )
        pMarkArray->SetMarkArea( nTop, nBottom, true );
    return pMarkArray;
}



ScMultiSelIter::ScMultiSelIter( const ScMultiSel& rMultiSel, SCCOL nCol ) :
    aRowSegs(),
    nNextSegmentStart(0)
{
    aRowSegs.setFalse( 0, MAXROW );
    bool bHasMarks1 = rMultiSel.aRowSel.HasMarks();
    bool bHasMarks2 = ( nCol < rMultiSel.size() && rMultiSel.aMultiSelContainer[nCol]->HasMarks() );

    if ( bHasMarks1 )
    {
        ScMarkArrayIter aMarkIter( &rMultiSel.aRowSel );
        SCROW nTop, nBottom;
        while ( aMarkIter.Next( nTop, nBottom ) )
            aRowSegs.setTrue( nTop, nBottom );
    }

    if ( bHasMarks2 )
    {
        ScMarkArrayIter aMarkIter( rMultiSel.aMultiSelContainer[nCol] );
        SCROW nTop, nBottom;
        while ( aMarkIter.Next( nTop, nBottom ) )
            aRowSegs.setTrue( nTop, nBottom );
    }

}

ScMultiSelIter::~ScMultiSelIter()
{
}

bool ScMultiSelIter::Next( SCROW& rTop, SCROW& rBottom )
{
    ScFlatBoolRowSegments::RangeData aRowRange;
    bool bRet = aRowSegs.getRangeData( nNextSegmentStart, aRowRange );
    if ( bRet && !aRowRange.mbValue )
    {
        nNextSegmentStart = aRowRange.mnRow2 + 1;
        bRet = aRowSegs.getRangeData( nNextSegmentStart, aRowRange );
    }
    if ( bRet )
    {
        rTop = aRowRange.mnRow1;
        rBottom = aRowRange.mnRow2;
        nNextSegmentStart = rBottom + 1;
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
