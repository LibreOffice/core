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

ScMultiSel::ScMultiSel():
    aMultiSelContainer(),
    aRowSel()
{
}

ScMultiSel::ScMultiSel( const ScMultiSel& rMultiSel )
{
    MapType::iterator aDestEnd = aMultiSelContainer.end();
    MapType::iterator aDestIter = aDestEnd;
    for ( const auto& aSourcePair : rMultiSel.aMultiSelContainer )
    {
        // correct hint is always aDestEnd as keys come in ascending order
        // Amortized constant time operation as we always give the correct hint
        aDestIter = aMultiSelContainer.emplace_hint( aDestEnd, aSourcePair.first, ScMarkArray() );
        aSourcePair.second.CopyMarksTo( aDestIter->second );
    }
    rMultiSel.aRowSel.CopyMarksTo( aRowSel );
}

ScMultiSel::~ScMultiSel()
{
}

ScMultiSel& ScMultiSel::operator=(const ScMultiSel& rMultiSel)
{
    Clear();
    MapType::iterator aDestEnd = aMultiSelContainer.end();
    MapType::iterator aDestIter = aDestEnd;
    for ( const auto& aSourcePair : rMultiSel.aMultiSelContainer )
    {
        // correct hint is always aDestEnd as keys come in ascending order
        // Amortized constant time operation as we always give the correct hint
        aDestIter = aMultiSelContainer.emplace_hint( aDestEnd, aSourcePair.first, ScMarkArray() );
        aSourcePair.second.CopyMarksTo( aDestIter->second );
    }
    rMultiSel.aRowSel.CopyMarksTo( aRowSel );
    return *this;
}

void ScMultiSel::Clear()
{
    aMultiSelContainer.clear();
    aRowSel.Reset();
}

bool ScMultiSel::HasMarks( SCCOL nCol ) const
{
    if ( aRowSel.HasMarks() )
        return true;
    MapType::const_iterator aIter = aMultiSelContainer.find( nCol );
    if ( aIter == aMultiSelContainer.end() )
        return false;
    return aIter->second.HasMarks();
}

bool ScMultiSel::HasOneMark( SCCOL nCol, SCROW& rStartRow, SCROW& rEndRow ) const
{
    bool aResult1 = false, aResult2 = false;
    SCROW nRow1 = -1, nRow2 = -1, nRow3 = -1, nRow4 = -1;
    aResult1 = aRowSel.HasOneMark( nRow1, nRow2 );
    MapType::const_iterator aIter = aMultiSelContainer.find( nCol );
    if ( aIter != aMultiSelContainer.end() )
        aResult2 = aIter->second.HasOneMark( nRow3, nRow4 );

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
    MapType::const_iterator aIter = aMultiSelContainer.find( nCol );
    if ( aIter != aMultiSelContainer.end() )
        return aIter->second.GetMark( nRow );
    return false;
}

bool ScMultiSel::IsAllMarked( SCCOL nCol, SCROW nStartRow, SCROW nEndRow ) const
{
    bool bHasMarks1 = aRowSel.HasMarks();
    MapType::const_iterator aIter = aMultiSelContainer.find( nCol );
    bool bHasMarks2 = ( aIter != aMultiSelContainer.end() && aIter->second.HasMarks() );

    if ( !bHasMarks1 && !bHasMarks2 )
        return false;

    if ( bHasMarks1 && bHasMarks2 )
    {
        if ( aRowSel.IsAllMarked( nStartRow, nEndRow ) ||
             aIter->second.IsAllMarked( nStartRow, nEndRow ) )
            return true;
        ScMultiSelIter aMultiIter( *this, nCol );
        ScFlatBoolRowSegments::RangeData aRowRange;
        bool bRet = aMultiIter.GetRangeData( nStartRow, aRowRange );
        return bRet && aRowRange.mbValue && aRowRange.mnRow2 >= nEndRow;
    }

    if ( bHasMarks1 )
        return aRowSel.IsAllMarked( nStartRow, nEndRow );

    return aIter->second.IsAllMarked( nStartRow, nEndRow );
}

bool ScMultiSel::HasEqualRowsMarked( SCCOL nCol1, SCCOL nCol2 ) const
{
    MapType::const_iterator aIter1 = aMultiSelContainer.find( nCol1 );
    MapType::const_iterator aIter2 = aMultiSelContainer.find( nCol2 );
    MapType::const_iterator aEnd = aMultiSelContainer.end();
    bool bCol1Exists = ( aIter1 != aEnd );
    bool bCol2Exists = ( aIter2 != aEnd );
    if ( bCol1Exists || bCol2Exists )
    {
        if ( bCol1Exists && bCol2Exists )
            return aIter1->second.HasEqualRowsMarked( aIter2->second );
        else if ( bCol1Exists )
            return !aIter1->second.HasMarks();
        else
            return !aIter2->second.HasMarks();
    }

    return true;
}

SCROW ScMultiSel::GetNextMarked( SCCOL nCol, SCROW nRow, bool bUp ) const
{
    MapType::const_iterator aIter = aMultiSelContainer.find( nCol );
    if ( aIter == aMultiSelContainer.end() )
        return aRowSel.GetNextMarked( nRow, bUp );

    SCROW nRow1, nRow2;
    nRow1 = aRowSel.GetNextMarked( nRow, bUp );
    nRow2 = aIter->second.GetNextMarked( nRow, bUp );
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
    MapType::iterator aIter = aMultiSelContainer.end();
    for ( SCCOL nCol = MAXCOL; nCol >= 0; --nCol )
    {
        aIter = aMultiSelContainer.emplace_hint( aIter, nCol, ScMarkArray() );
        aIter->second.SetMarkArea( nStartRow, nEndRow, true );
    }
}

void ScMultiSel::SetMarkArea( SCCOL nStartCol, SCCOL nEndCol, SCROW nStartRow, SCROW nEndRow, bool bMark )
{
    if ( nStartCol == 0 && nEndCol == MAXCOL )
    {
        aRowSel.SetMarkArea( nStartRow, nEndRow, bMark );
        if ( !bMark )
        {
            // Remove any per column marks for the row range.
            for ( auto& aIter : aMultiSelContainer )
                if ( aIter.second.HasMarks() )
                    aIter.second.SetMarkArea( nStartRow, nEndRow, false );
        }
        return;
    }

    // Bad case - we need to extend aMultiSelContainer size to MAXCOL
    // and move row marks from aRowSel to aMultiSelContainer
    if ( !bMark && aRowSel.HasMarks() )
    {
        SCROW nBeg, nLast = nEndRow;
        if ( aRowSel.GetMark( nStartRow ) )
        {
            nBeg = nStartRow;
            nLast = aRowSel.GetMarkEnd( nStartRow, false );
        }
        else
        {
            nBeg = aRowSel.GetNextMarked( nStartRow, false );
            if ( nBeg != MAXROWCOUNT )
                nLast = aRowSel.GetMarkEnd( nBeg, false );
        }

        if ( nBeg != MAXROWCOUNT && nLast >= nEndRow )
            MarkAllCols( nBeg, nEndRow );
        else
        {
            while ( nBeg != MAXROWCOUNT && nLast < nEndRow )
            {
                MarkAllCols( nBeg, nLast );
                nBeg = aRowSel.GetNextMarked( nLast + 1, false );
                if ( nBeg != MAXROWCOUNT )
                    nLast = aRowSel.GetMarkEnd( nBeg, false );
            }
            if ( nBeg != MAXROWCOUNT && nLast >= nEndRow )
                MarkAllCols( nBeg, nEndRow );
        }

        aRowSel.SetMarkArea( nStartRow, nEndRow, false );
    }

    MapType::iterator aIter = aMultiSelContainer.end();
    for ( SCCOL nColIter = nEndCol; nColIter >= nStartCol; --nColIter )
    {
        // First hint is usually off, so the first emplace operation will take up to
        // logarithmic in map size, all other iterations will take only constant time.
        aIter = aMultiSelContainer.emplace_hint( aIter, nColIter, ScMarkArray() );
        aIter->second.SetMarkArea( nStartRow, nEndRow, bMark );
    }
}

bool ScMultiSel::IsRowMarked( SCROW nRow ) const
{
    return aRowSel.GetMark( nRow );
}

bool ScMultiSel::IsRowRangeMarked( SCROW nStartRow, SCROW nEndRow ) const
{
    if ( !aRowSel.GetMark( nStartRow ) )
        return false;
    SCROW nLast = aRowSel.GetMarkEnd( nStartRow, false );
    return ( nLast >= nEndRow );
}

ScMarkArray ScMultiSel::GetMarkArray( SCCOL nCol ) const
{
    ScMultiSelIter aMultiIter( *this, nCol );
    ScMarkArray aMarkArray;
    SCROW nTop, nBottom;
    while( aMultiIter.Next( nTop, nBottom ) )
        aMarkArray.SetMarkArea( nTop, nBottom, true );
    return aMarkArray;
}

bool ScMultiSel::HasAnyMarks() const
{
    if ( aRowSel.HasMarks() )
        return true;
    for ( const auto& aPair : aMultiSelContainer )
        if ( aPair.second.HasMarks() )
            return true;
    return false;
}

void ScMultiSel::ShiftCols(SCCOL nStartCol, long nColOffset)
{
    if (nStartCol > MAXCOL)
        return;

    ScMultiSel aNewMultiSel(*this);
    Clear();

    if (nColOffset < 0)
    {
        // columns that would be moved on the left of nStartCol must be removed
        const SCCOL nEndPos = nStartCol - nColOffset;
        for (SCCOL nSearchPos = nStartCol; nSearchPos < nEndPos; ++nSearchPos)
        {
            const auto& aColIt = aNewMultiSel.aMultiSelContainer.find(nSearchPos);
            if (aColIt != aNewMultiSel.aMultiSelContainer.end())
            {
                aNewMultiSel.aMultiSelContainer.erase(aColIt);
            }
        }
    }

    MapType::iterator aDestEnd = aMultiSelContainer.end();
    MapType::iterator aDestIter = aDestEnd;
    for (const auto& aSourcePair : aNewMultiSel.aMultiSelContainer)
    {
        SCCOL nCol = aSourcePair.first;
        if (aSourcePair.first >= nStartCol)
        {
            nCol += nColOffset;
            if (nCol < 0)
                nCol = 0;
            else if (nCol > MAXCOL)
                nCol = MAXCOL;
        }
        // correct hint is always aDestEnd as keys come in ascending order
        // Amortized constant time operation as we always give the correct hint
        aDestIter = aMultiSelContainer.emplace_hint( aDestEnd, nCol, ScMarkArray() );
        aSourcePair.second.CopyMarksTo(aDestIter->second);
    }
    aNewMultiSel.aRowSel.CopyMarksTo(aRowSel);

    if (nColOffset > 0 && nStartCol > 0)
    {
        // insert nColOffset new columns, and select their cells if they are selected
        // both in the old column at nStartPos and in the previous column
        const auto& aPrevPosIt = aNewMultiSel.aMultiSelContainer.find(nStartCol - 1);
        if (aPrevPosIt != aNewMultiSel.aMultiSelContainer.end())
        {
            const auto& aStartPosIt = aNewMultiSel.aMultiSelContainer.find(nStartCol);
            if (aStartPosIt != aNewMultiSel.aMultiSelContainer.end())
            {
                MapType::iterator aNewColIt = aMultiSelContainer.emplace_hint(aDestEnd, nStartCol, ScMarkArray());
                aStartPosIt->second.CopyMarksTo(aNewColIt->second);
                aNewColIt->second.Intersect(aPrevPosIt->second);
                for (long i = 1; i < nColOffset; ++i)
                {
                    aDestIter = aMultiSelContainer.emplace_hint(aDestEnd, nStartCol + i, ScMarkArray());
                    aNewColIt->second.CopyMarksTo(aDestIter->second);
                }
            }
        }
    }
}

void ScMultiSel::ShiftRows(SCROW nStartRow, long nRowOffset)
{
    for (auto& aPair: aMultiSelContainer)
    {
        aPair.second.Shift(nStartRow, nRowOffset);
    }
    aRowSel.Shift(nStartRow, nRowOffset);
}

const ScMarkArray& ScMultiSel::GetRowSelArray() const
{
    return aRowSel;
}

const ScMarkArray* ScMultiSel::GetMultiSelArray( SCCOL nCol ) const
{
    ScMultiSel::MapType::const_iterator aIter = aMultiSelContainer.find( nCol );
    return (aIter != aMultiSelContainer.end()) ? &aIter->second : nullptr;
}

ScMultiSelIter::ScMultiSelIter( const ScMultiSel& rMultiSel, SCCOL nCol ) :
    aMarkArrayIter(nullptr),
    nNextSegmentStart(0)
{
    bool bHasMarks1 = rMultiSel.aRowSel.HasMarks();
    ScMultiSel::MapType::const_iterator aIter = rMultiSel.aMultiSelContainer.find( nCol );
    bool bHasMarks2 = ( ( aIter != rMultiSel.aMultiSelContainer.end() ) && aIter->second.HasMarks() );

    if (bHasMarks1 && bHasMarks2)
    {
        pRowSegs.reset( new ScFlatBoolRowSegments);
        pRowSegs->setFalse( 0, MAXROW );
        if ( bHasMarks1 )
        {
            ScMarkArrayIter aMarkIter( &rMultiSel.aRowSel );
            SCROW nTop, nBottom;
            while ( aMarkIter.Next( nTop, nBottom ) )
                pRowSegs->setTrue( nTop, nBottom );
        }

        if ( bHasMarks2 )
        {
            ScMarkArrayIter aMarkIter( &aIter->second );
            SCROW nTop, nBottom;
            while ( aMarkIter.Next( nTop, nBottom ) )
                pRowSegs->setTrue( nTop, nBottom );
        }
    }
    else if (bHasMarks1)
    {
        aMarkArrayIter.reset( &rMultiSel.aRowSel);
    }
    else if (bHasMarks2)
    {
        aMarkArrayIter.reset( &aIter->second);
    }
}

ScMultiSelIter::~ScMultiSelIter()
{
}

bool ScMultiSelIter::Next( SCROW& rTop, SCROW& rBottom )
{
    if (pRowSegs)
    {
        ScFlatBoolRowSegments::RangeData aRowRange;
        bool bRet = pRowSegs->getRangeData( nNextSegmentStart, aRowRange );
        if ( bRet && !aRowRange.mbValue )
        {
            nNextSegmentStart = aRowRange.mnRow2 + 1;
            bRet = pRowSegs->getRangeData( nNextSegmentStart, aRowRange );
        }
        if ( bRet )
        {
            rTop = aRowRange.mnRow1;
            rBottom = aRowRange.mnRow2;
            nNextSegmentStart = rBottom + 1;
        }
        return bRet;
    }

    return aMarkArrayIter.Next( rTop, rBottom);
}

bool ScMultiSelIter::GetRangeData( SCROW nRow, ScFlatBoolRowSegments::RangeData& rRowRange ) const
{
    assert(pRowSegs);
    return pRowSegs->getRangeData( nRow, rRowRange);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
