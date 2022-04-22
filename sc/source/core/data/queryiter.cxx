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

#include <queryiter.hxx>

#include <svl/numformat.hxx>
#include <svl/zforlist.hxx>

#include <global.hxx>
#include <dociter.hxx>
#include <document.hxx>
#include <table.hxx>
#include <column.hxx>
#include <formulacell.hxx>
#include <attarray.hxx>
#include <patattr.hxx>
#include <docoptio.hxx>
#include <cellform.hxx>
#include <segmenttree.hxx>
#include <progress.hxx>
#include <queryparam.hxx>
#include <queryentry.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <cellvalue.hxx>
#include <scmatrix.hxx>
#include <rowheightcontext.hxx>
#include <queryevaluator.hxx>

#include <o3tl/safeint.hxx>
#include <tools/fract.hxx>
#include <editeng/editobj.hxx>
#include <svl/sharedstring.hxx>
#include <unotools/collatorwrapper.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>

#include <algorithm>
#include <limits>
#include <vector>

ScQueryCellIterator::ScQueryCellIterator(ScDocument& rDocument, const ScInterpreterContext& rContext, SCTAB nTable,
             const ScQueryParam& rParam, bool bMod ) :
    maParam(rParam),
    rDoc( rDocument ),
    mrContext( rContext ),
    nTab( nTable),
    nStopOnMismatch( nStopOnMismatchDisabled ),
    nTestEqualCondition( nTestEqualConditionDisabled ),
    bAdvanceQuery( false ),
    bIgnoreMismatchOnLeadingStrings( false )
{
    nCol = maParam.nCol1;
    nRow = maParam.nRow1;
    SCSIZE i;
    if (!bMod) // Or else it's already inserted
        return;

    SCSIZE nCount = maParam.GetEntryCount();
    for (i = 0; (i < nCount) && (maParam.GetEntry(i).bDoQuery); ++i)
    {
        ScQueryEntry& rEntry = maParam.GetEntry(i);
        ScQueryEntry::Item& rItem = rEntry.GetQueryItem();
        sal_uInt32 nIndex = 0;
        bool bNumber = mrContext.GetFormatTable()->IsNumberFormat(
            rItem.maString.getString(), nIndex, rItem.mfVal);
        rItem.meType = bNumber ? ScQueryEntry::ByValue : ScQueryEntry::ByString;
    }
}

void ScQueryCellIterator::InitPos()
{
    nRow = maParam.nRow1;
    if (maParam.bHasHeader && maParam.bByRow)
        ++nRow;
    const ScColumn& rCol = rDoc.maTabs[nTab]->CreateColumnIfNotExists(nCol);
    maCurPos = rCol.maCells.position(nRow);
}

void ScQueryCellIterator::IncPos()
{
    if (maCurPos.second + 1 < maCurPos.first->size)
    {
        // Move within the same block.
        ++maCurPos.second;
        ++nRow;
    }
    else
        // Move to the next block.
        IncBlock();
}

void ScQueryCellIterator::IncBlock()
{
    ++maCurPos.first;
    maCurPos.second = 0;

    nRow = maCurPos.first->position;
}

bool ScQueryCellIterator::GetThis()
{
    assert(nTab < rDoc.GetTableCount() && "index out of bounds, FIX IT");
    const ScQueryEntry& rEntry = maParam.GetEntry(0);
    const ScQueryEntry::Item& rItem = rEntry.GetQueryItem();

    SCCOLROW nFirstQueryField = rEntry.nField;
    bool bAllStringIgnore = bIgnoreMismatchOnLeadingStrings &&
        rItem.meType != ScQueryEntry::ByString;
    bool bFirstStringIgnore = bIgnoreMismatchOnLeadingStrings &&
        !maParam.bHasHeader && rItem.meType == ScQueryEntry::ByString &&
        ((maParam.bByRow && nRow == maParam.nRow1) ||
         (!maParam.bByRow && nCol == maParam.nCol1));
    bool bTestEqualCondition = false;
    ScQueryEvaluator queryEvaluator(rDoc, *rDoc.maTabs[nTab], maParam, &mrContext,
        (nTestEqualCondition ? &bTestEqualCondition : nullptr));

    ScColumn* pCol = &(rDoc.maTabs[nTab])->aCol[nCol];
    while (true)
    {
        bool bNextColumn = maCurPos.first == pCol->maCells.end();
        if (!bNextColumn)
        {
            if (nRow > maParam.nRow2)
                bNextColumn = true;
        }

        if (bNextColumn)
        {
            do
            {
                ++nCol;
                if (nCol > maParam.nCol2 || nCol >= rDoc.maTabs[nTab]->GetAllocatedColumnsCount())
                    return false;
                if ( bAdvanceQuery )
                {
                    AdvanceQueryParamEntryField();
                    nFirstQueryField = rEntry.nField;
                }
                pCol = &(rDoc.maTabs[nTab])->aCol[nCol];
            }
            while (!rItem.mbMatchEmpty && pCol->IsEmptyData());

            InitPos();

            bFirstStringIgnore = bIgnoreMismatchOnLeadingStrings &&
                !maParam.bHasHeader && rItem.meType == ScQueryEntry::ByString &&
                maParam.bByRow;
        }

        if (maCurPos.first->type == sc::element_type_empty)
        {
            if (rItem.mbMatchEmpty && rEntry.GetQueryItems().size() == 1)
            {
                // This shortcut, instead of determining if any SC_OR query
                // exists or this query is SC_AND'ed (which wouldn't make
                // sense, but..) and evaluating them in ValidQuery(), is
                // possible only because the interpreter is the only caller
                // that sets mbMatchEmpty and there is only one item in those
                // cases.
                // XXX this would have to be reworked if other filters used it
                // in different manners and evaluation would have to be done in
                // ValidQuery().
                return true;
            }
            else
            {
                IncBlock();
                continue;
            }
        }

        ScRefCellValue aCell = sc::toRefCell(maCurPos.first, maCurPos.second);

        if (bAllStringIgnore && aCell.hasString())
            IncPos();
        else
        {
            if ( queryEvaluator.ValidQuery( nRow,
                    (nCol == static_cast<SCCOL>(nFirstQueryField) ? &aCell : nullptr)))
            {
                if ( nTestEqualCondition && bTestEqualCondition )
                    nTestEqualCondition |= nTestEqualConditionMatched;
                return !aCell.isEmpty(); // Found it!
            }
            else if ( nStopOnMismatch )
            {
                // Yes, even a mismatch may have a fulfilled equal
                // condition if regular expressions were involved and
                // SC_LESS_EQUAL or SC_GREATER_EQUAL were queried.
                if ( nTestEqualCondition && bTestEqualCondition )
                {
                    nTestEqualCondition |= nTestEqualConditionMatched;
                    nStopOnMismatch |= nStopOnMismatchOccurred;
                    return false;
                }
                bool bStop;
                if (bFirstStringIgnore)
                {
                    if (aCell.hasString())
                    {
                        IncPos();
                        bStop = false;
                    }
                    else
                        bStop = true;
                }
                else
                    bStop = true;
                if (bStop)
                {
                    nStopOnMismatch |= nStopOnMismatchOccurred;
                    return false;
                }
            }
            else
                IncPos();
        }
        bFirstStringIgnore = false;
    }
}

bool ScQueryCellIterator::GetFirst()
{
    assert(nTab < rDoc.GetTableCount() && "index out of bounds, FIX IT");
    nCol = maParam.nCol1;
    InitPos();
    return GetThis();
}

bool ScQueryCellIterator::GetNext()
{
    IncPos();
    if ( nStopOnMismatch )
        nStopOnMismatch = nStopOnMismatchEnabled;
    if ( nTestEqualCondition )
        nTestEqualCondition = nTestEqualConditionEnabled;
    return GetThis();
}

void ScQueryCellIterator::AdvanceQueryParamEntryField()
{
    SCSIZE nEntries = maParam.GetEntryCount();
    for ( SCSIZE j = 0; j < nEntries; j++  )
    {
        ScQueryEntry& rEntry = maParam.GetEntry( j );
        if ( rEntry.bDoQuery )
        {
            if ( rEntry.nField < rDoc.MaxCol() )
                rEntry.nField++;
            else
            {
                assert(!"AdvanceQueryParamEntryField: ++rEntry.nField > MAXCOL");
            }
        }
        else
            break;  // for
    }
}

bool ScQueryCellIterator::FindEqualOrSortedLastInRange( SCCOL& nFoundCol,
        SCROW& nFoundRow )
{
    // Set and automatically reset mpParam->mbRangeLookup when returning. We
    // could use comphelper::FlagRestorationGuard, but really, that one is
    // overengineered for this simple purpose here.
    struct BoolResetter
    {
        bool& mr;
        bool  mb;
        BoolResetter( bool& r, bool b ) : mr(r), mb(r) { r = b; }
        ~BoolResetter() { mr = mb; }
    } aRangeLookupResetter( maParam.mbRangeLookup, true);

    nFoundCol = rDoc.MaxCol()+1;
    nFoundRow = rDoc.MaxRow()+1;
    SetStopOnMismatch( true ); // assume sorted keys
    SetTestEqualCondition( true );
    bIgnoreMismatchOnLeadingStrings = true;
    bool bLiteral = maParam.eSearchType == utl::SearchParam::SearchType::Normal &&
        maParam.GetEntry(0).GetQueryItem().meType == ScQueryEntry::ByString;
    bool bBinary = maParam.bByRow &&
        (bLiteral || maParam.GetEntry(0).GetQueryItem().meType == ScQueryEntry::ByValue) &&
        (maParam.GetEntry(0).eOp == SC_LESS_EQUAL || maParam.GetEntry(0).eOp == SC_GREATER_EQUAL);
    bool bFound = false;
    if (bBinary)
    {
        if (BinarySearch())
        {
            // BinarySearch() already positions correctly and only needs real
            // query comparisons afterwards, skip the verification check below.
            maParam.mbRangeLookup = false;
            bFound = GetThis();
        }
    }
    else
    {
        bFound = GetFirst();
    }
    if (bFound)
    {
        // First equal entry or last smaller than (greater than) entry.
        PositionType aPosSave;
        bool bNext = false;
        do
        {
            nFoundCol = GetCol();
            nFoundRow = GetRow();
            aPosSave = maCurPos;
            if (IsEqualConditionFulfilled())
                break;
            bNext = GetNext();
        }
        while (bNext);

        // There may be no pNext but equal condition fulfilled if regular
        // expressions are involved. Keep the found entry and proceed.
        if (!bNext && !IsEqualConditionFulfilled())
        {
            // Step back to last in range and adjust position markers for
            // GetNumberFormat() or similar.
            SCCOL nColDiff = nCol - nFoundCol;
            nCol = nFoundCol;
            nRow = nFoundRow;
            maCurPos = aPosSave;
            if (maParam.mbRangeLookup)
            {
                // Verify that the found entry does not only fulfill the range
                // lookup but also the real query, i.e. not numeric was found
                // if query is ByString and vice versa.
                maParam.mbRangeLookup = false;
                // Step back the last field advance if GetNext() did one.
                if (bAdvanceQuery && nColDiff)
                {
                    SCSIZE nEntries = maParam.GetEntryCount();
                    for (SCSIZE j=0; j < nEntries; ++j)
                    {
                        ScQueryEntry& rEntry = maParam.GetEntry( j );
                        if (rEntry.bDoQuery)
                        {
                            if (rEntry.nField - nColDiff >= 0)
                                rEntry.nField -= nColDiff;
                            else
                            {
                                assert(!"FindEqualOrSortedLastInRange: rEntry.nField -= nColDiff < 0");
                            }
                        }
                        else
                            break;  // for
                    }
                }
                // Check it.
                if (!GetThis())
                {
                    nFoundCol = rDoc.MaxCol()+1;
                    nFoundRow = rDoc.MaxRow()+1;
                }
            }
        }
    }
    if ( IsEqualConditionFulfilled() )
    {
        // Position on last equal entry.
        SCSIZE nEntries = maParam.GetEntryCount();
        for ( SCSIZE j = 0; j < nEntries; j++  )
        {
            ScQueryEntry& rEntry = maParam.GetEntry( j );
            if ( rEntry.bDoQuery )
            {
                switch ( rEntry.eOp )
                {
                    case SC_LESS_EQUAL :
                    case SC_GREATER_EQUAL :
                        rEntry.eOp = SC_EQUAL;
                    break;
                    default:
                    {
                        // added to avoid warnings
                    }
                }
            }
            else
                break;  // for
        }
        PositionType aPosSave;
        bIgnoreMismatchOnLeadingStrings = false;
        SetTestEqualCondition( false );
        do
        {
            nFoundCol = GetCol();
            nFoundRow = GetRow();
            aPosSave = maCurPos;
        } while (GetNext());

        // Step back conditions are the same as above
        nCol = nFoundCol;
        nRow = nFoundRow;
        maCurPos = aPosSave;
        return true;
    }
    if ( (maParam.eSearchType != utl::SearchParam::SearchType::Normal) &&
            StoppedOnMismatch() )
    {
        // Assume found entry to be the last value less than respectively
        // greater than the query. But keep on searching for an equal match.
        SCSIZE nEntries = maParam.GetEntryCount();
        for ( SCSIZE j = 0; j < nEntries; j++  )
        {
            ScQueryEntry& rEntry = maParam.GetEntry( j );
            if ( rEntry.bDoQuery )
            {
                switch ( rEntry.eOp )
                {
                    case SC_LESS_EQUAL :
                    case SC_GREATER_EQUAL :
                        rEntry.eOp = SC_EQUAL;
                    break;
                    default:
                    {
                        // added to avoid warnings
                    }
                }
            }
            else
                break;  // for
        }
        SetStopOnMismatch( false );
        SetTestEqualCondition( false );
        if (GetNext())
        {
            // Last of a consecutive area, avoid searching the entire parameter
            // range as it is a real performance bottleneck in case of regular
            // expressions.
            PositionType aPosSave;
            do
            {
                nFoundCol = GetCol();
                nFoundRow = GetRow();
                aPosSave = maCurPos;
                SetStopOnMismatch( true );
            } while (GetNext());
            nCol = nFoundCol;
            nRow = nFoundRow;
            maCurPos = aPosSave;
        }
    }
    return (nFoundCol <= rDoc.MaxCol()) && (nFoundRow <= rDoc.MaxRow());
}

ScCountIfCellIterator::ScCountIfCellIterator(ScDocument& rDocument, const ScInterpreterContext& rContext, SCTAB nTable,
             const ScQueryParam& rParam ) :
    maParam(rParam),
    rDoc( rDocument ),
    mrContext( rContext ),
    nTab( nTable)
{
    maParam.nCol1 = rDoc.maTabs[nTable]->ClampToAllocatedColumns(maParam.nCol1);
    maParam.nCol2 = rDoc.maTabs[nTable]->ClampToAllocatedColumns(maParam.nCol2);
    nCol = maParam.nCol1;
    nRow = maParam.nRow1;
}

void ScCountIfCellIterator::InitPos()
{
    nRow = maParam.nRow1;
    if (maParam.bHasHeader && maParam.bByRow)
        ++nRow;
    ScColumn* pCol = &(rDoc.maTabs[nTab])->aCol[nCol];
    maCurPos = pCol->maCells.position(nRow);
}

void ScCountIfCellIterator::IncPos()
{
    if (maCurPos.second + 1 < maCurPos.first->size)
    {
        // Move within the same block.
        ++maCurPos.second;
        ++nRow;
    }
    else
        // Move to the next block.
        IncBlock();
}

void ScCountIfCellIterator::IncBlock()
{
    ++maCurPos.first;
    maCurPos.second = 0;

    nRow = maCurPos.first->position;
}

int ScCountIfCellIterator::GetCount()
{
    assert(nTab < rDoc.GetTableCount() && "try to access index out of bounds, FIX IT");
    nCol = maParam.nCol1;
    InitPos();

    const ScQueryEntry& rEntry = maParam.GetEntry(0);
    const ScQueryEntry::Item& rItem = rEntry.GetQueryItem();
    const bool bSingleQueryItem = rEntry.GetQueryItems().size() == 1;
    int count = 0;
    ScQueryEvaluator queryEvaluator(rDoc, *rDoc.maTabs[nTab], maParam, &mrContext);

    ScColumn* pCol = &(rDoc.maTabs[nTab])->aCol[nCol];
    while (true)
    {
        bool bNextColumn = maCurPos.first == pCol->maCells.end();
        if (!bNextColumn)
        {
            if (nRow > maParam.nRow2)
                bNextColumn = true;
        }

        if (bNextColumn)
        {
            do
            {
                ++nCol;
                if (nCol > maParam.nCol2 || nCol >= rDoc.maTabs[nTab]->GetAllocatedColumnsCount())
                    return count;
                AdvanceQueryParamEntryField();
                pCol = &(rDoc.maTabs[nTab])->aCol[nCol];
            }
            while (!rItem.mbMatchEmpty && pCol->IsEmptyData());

            InitPos();
        }

        if (maCurPos.first->type == sc::element_type_empty)
        {
            if (rItem.mbMatchEmpty && bSingleQueryItem)
            {
                // This shortcut, instead of determining if any SC_OR query
                // exists or this query is SC_AND'ed (which wouldn't make
                // sense, but..) and evaluating them in ValidQuery(), is
                // possible only because the interpreter is the only caller
                // that sets mbMatchEmpty and there is only one item in those
                // cases.
                // XXX this would have to be reworked if other filters used it
                // in different manners and evaluation would have to be done in
                // ValidQuery().
                count++;
                IncPos();
                continue;
            }
            else
            {
                IncBlock();
                continue;
            }
        }

        ScRefCellValue aCell = sc::toRefCell(maCurPos.first, maCurPos.second);

        if ( queryEvaluator.ValidQuery( nRow,
                (nCol == static_cast<SCCOL>(rEntry.nField) ? &aCell : nullptr)))
        {
            if (aCell.isEmpty())
                return count;
            count++;
            IncPos();
            continue;
        }
        else
            IncPos();
    }
    return count;
}

void ScCountIfCellIterator::AdvanceQueryParamEntryField()
{
    SCSIZE nEntries = maParam.GetEntryCount();
    for ( SCSIZE j = 0; j < nEntries; j++  )
    {
        ScQueryEntry& rEntry = maParam.GetEntry( j );
        if ( rEntry.bDoQuery )
        {
            if ( rEntry.nField < rDoc.MaxCol() )
                rEntry.nField++;
            else
            {
                OSL_FAIL( "AdvanceQueryParamEntryField: ++rEntry.nField > MAXCOL" );
            }
        }
        else
            break;  // for
    }
}

namespace {

template<typename Iter>
void incBlock(std::pair<Iter, size_t>& rPos)
{
    // Move to the next block.
    ++rPos.first;
    rPos.second = 0;
}

template<typename Iter>
void decBlock(std::pair<Iter, size_t>& rPos)
{
    // Move to the last element of the previous block.
    --rPos.first;
    rPos.second = rPos.first->size - 1;
}

/**
 * This class sequentially indexes non-empty cells in order, from the top of
 * the block where the start row position is, to the bottom of the block
 * where the end row position is.  It skips all empty blocks that may be
 * present in between.
 *
 * The index value is an offset from the first element of the first block
 * disregarding all empty cell blocks.
 */
class NonEmptyCellIndexer
{
    typedef std::map<size_t, sc::CellStoreType::const_iterator> BlockMapType;

    BlockMapType maBlockMap;

    const sc::CellStoreType& mrCells;

    size_t mnLowIndex;
    size_t mnHighIndex;

    bool mbValid;

public:

    typedef std::pair<ScRefCellValue, SCROW> CellType;

    /**
     * @param rCells cell storage container
     * @param nStartRow logical start row position
     * @param nEndRow logical end row position, inclusive.
     * @param bSkipTopStrBlock when true, skip all leading string cells.
     */
    NonEmptyCellIndexer(
        const sc::CellStoreType& rCells, SCROW nStartRow, SCROW nEndRow, bool bSkipTopStrBlock ) :
        mrCells(rCells), mnLowIndex(0), mnHighIndex(0), mbValid(true)
    {
        if (nEndRow < nStartRow)
        {
            mbValid = false;
            return;
        }

        // Find the low position.

        sc::CellStoreType::const_position_type aLoPos = mrCells.position(nStartRow);
        if (aLoPos.first->type == sc::element_type_empty)
            incBlock(aLoPos);

        if (aLoPos.first == rCells.end())
        {
            mbValid = false;
            return;
        }

        if (bSkipTopStrBlock)
        {
            // Skip all leading string or empty blocks.
            while (aLoPos.first->type == sc::element_type_string ||
                   aLoPos.first->type == sc::element_type_edittext ||
                   aLoPos.first->type == sc::element_type_empty)
            {
                incBlock(aLoPos);
                if (aLoPos.first == rCells.end())
                {
                    mbValid = false;
                    return;
                }
            }
        }

        SCROW nFirstRow = aLoPos.first->position;
        SCROW nLastRow = aLoPos.first->position + aLoPos.first->size - 1;

        if (nFirstRow > nEndRow)
        {
            // Both start and end row positions are within the leading skipped
            // blocks.
            mbValid = false;
            return;
        }

        // Calculate the index of the low position.
        if (nFirstRow < nStartRow)
            mnLowIndex = nStartRow - nFirstRow;
        else
        {
            // Start row is within the skipped block(s). Set it to the first
            // element of the low block.
            mnLowIndex = 0;
        }

        if (nEndRow < nLastRow)
        {
            assert(nEndRow >= nFirstRow);
            mnHighIndex = nEndRow - nFirstRow;

            maBlockMap.emplace(aLoPos.first->size, aLoPos.first);
            return;
        }

        // Find the high position.

        sc::CellStoreType::const_position_type aHiPos = mrCells.position(aLoPos.first, nEndRow);
        if (aHiPos.first->type == sc::element_type_empty)
        {
            // Move to the last position of the previous block.
            decBlock(aHiPos);

            // Check the row position of the end of the previous block, and make sure it's valid.
            SCROW nBlockEndRow = aHiPos.first->position + aHiPos.first->size - 1;
            if (nBlockEndRow < nStartRow)
            {
                mbValid = false;
                return;
            }
        }

        // Tag the start and end blocks, and all blocks in between in order
        // but skip all empty blocks.

        size_t nPos = 0;
        sc::CellStoreType::const_iterator itBlk = aLoPos.first;
        while (itBlk != aHiPos.first)
        {
            if (itBlk->type == sc::element_type_empty)
            {
                ++itBlk;
                continue;
            }

            nPos += itBlk->size;
            maBlockMap.emplace(nPos, itBlk);
            ++itBlk;

            if (itBlk->type == sc::element_type_empty)
                ++itBlk;

            assert(itBlk != mrCells.end());
        }

        assert(itBlk == aHiPos.first);
        nPos += itBlk->size;
        maBlockMap.emplace(nPos, itBlk);

        // Calculate the high index.
        BlockMapType::const_reverse_iterator ri = maBlockMap.rbegin();
        mnHighIndex = ri->first;
        mnHighIndex -= ri->second->size;
        mnHighIndex += aHiPos.second;
    }

    sc::CellStoreType::const_position_type getPosition( size_t nIndex ) const
    {
        assert(mbValid);
        assert(mnLowIndex <= nIndex);
        assert(nIndex <= mnHighIndex);

        sc::CellStoreType::const_position_type aRet(mrCells.end(), 0);

        BlockMapType::const_iterator it = maBlockMap.upper_bound(nIndex);
        if (it == maBlockMap.end())
            return aRet;

        sc::CellStoreType::const_iterator itBlk = it->second;
        size_t nBlkIndex = it->first - itBlk->size; // index of the first element of the block.
        assert(nBlkIndex <= nIndex);
        assert(nIndex < it->first);

        size_t nOffset = nIndex - nBlkIndex;
        aRet.first = itBlk;
        aRet.second = nOffset;
        return aRet;
    }

    CellType getCell( size_t nIndex ) const
    {
        std::pair<ScRefCellValue, SCROW> aRet;
        aRet.second = -1;

        sc::CellStoreType::const_position_type aPos = getPosition(nIndex);
        if (aPos.first == mrCells.end())
            return aRet;

        aRet.first = sc::toRefCell(aPos.first, aPos.second);
        aRet.second = aPos.first->position + aPos.second;
        return aRet;
    }

    size_t getLowIndex() const { return mnLowIndex; }

    size_t getHighIndex() const { return mnHighIndex; }

    bool isValid() const { return mbValid; }
};

}

bool ScQueryCellIterator::BinarySearch()
{
    // TODO: This will be extremely slow with mdds::multi_type_vector.

    assert(nTab < rDoc.GetTableCount() && "index out of bounds, FIX IT");
    nCol = maParam.nCol1;

    if (nCol >= rDoc.maTabs[nTab]->GetAllocatedColumnsCount())
        return false;

    ScColumn* pCol = &(rDoc.maTabs[nTab])->aCol[nCol];
    if (pCol->IsEmptyData())
        return false;

    CollatorWrapper& rCollator = ScGlobal::GetCollator(maParam.bCaseSens);
    SvNumberFormatter& rFormatter = *(mrContext.GetFormatTable());
    const ScQueryEntry& rEntry = maParam.GetEntry(0);
    const ScQueryEntry::Item& rItem = rEntry.GetQueryItem();
    bool bLessEqual = rEntry.eOp == SC_LESS_EQUAL;
    bool bByString = rItem.meType == ScQueryEntry::ByString;
    bool bAllStringIgnore = bIgnoreMismatchOnLeadingStrings && !bByString;
    bool bFirstStringIgnore = bIgnoreMismatchOnLeadingStrings &&
        !maParam.bHasHeader && bByString;

    nRow = maParam.nRow1;
    if (maParam.bHasHeader)
        ++nRow;

    ScRefCellValue aCell;
    if (bFirstStringIgnore)
    {
        sc::CellStoreType::const_position_type aPos = pCol->maCells.position(nRow);
        if (aPos.first->type == sc::element_type_string || aPos.first->type == sc::element_type_edittext)
        {
            aCell = sc::toRefCell(aPos.first, aPos.second);
            sal_uInt32 nFormat = pCol->GetNumberFormat(mrContext, nRow);
            OUString aCellStr = ScCellFormat::GetInputString(aCell, nFormat, rFormatter, rDoc);
            sal_Int32 nTmp = rCollator.compareString(aCellStr, rEntry.GetQueryItem().maString.getString());
            if ((rEntry.eOp == SC_LESS_EQUAL && nTmp > 0) ||
                    (rEntry.eOp == SC_GREATER_EQUAL && nTmp < 0) ||
                    (rEntry.eOp == SC_EQUAL && nTmp != 0))
                ++nRow;
        }
    }

    NonEmptyCellIndexer aIndexer(pCol->maCells, nRow, maParam.nRow2, bAllStringIgnore);
    if (!aIndexer.isValid())
        return false;

    size_t nLo = aIndexer.getLowIndex();
    size_t nHi = aIndexer.getHighIndex();
    NonEmptyCellIndexer::CellType aCellData;

    // Bookkeeping values for breaking up the binary search in case the data
    // range isn't strictly sorted.
    size_t nLastInRange = nLo;
    size_t nFirstLastInRange = nLastInRange;
    double fLastInRangeValue = bLessEqual ?
        -(::std::numeric_limits<double>::max()) :
            ::std::numeric_limits<double>::max();
    OUString aLastInRangeString;
    if (!bLessEqual)
        aLastInRangeString = OUString(u'\xFFFF');

    aCellData = aIndexer.getCell(nLastInRange);
    aCell = aCellData.first;
    if (aCell.hasString())
    {
        sal_uInt32 nFormat = pCol->GetNumberFormat(mrContext, aCellData.second);
        OUString aStr = ScCellFormat::GetInputString(aCell, nFormat, rFormatter, rDoc);
        aLastInRangeString = aStr;
    }
    else
    {
        switch (aCell.meType)
        {
            case CELLTYPE_VALUE :
                fLastInRangeValue = aCell.mfValue;
            break;
            case CELLTYPE_FORMULA :
                fLastInRangeValue = aCell.mpFormula->GetValue();
            break;
            default:
            {
                // added to avoid warnings
            }
        }
    }

    sal_Int32 nRes = 0;
    bool bFound = false;
    bool bDone = false;
    while (nLo <= nHi && !bDone)
    {
        size_t nMid = (nLo+nHi)/2;
        size_t i = nMid;

        aCellData = aIndexer.getCell(i);
        aCell = aCellData.first;
        bool bStr = aCell.hasString();
        nRes = 0;

        // compares are content<query:-1, content>query:1
        // Cell value comparison similar to ScTable::ValidQuery()
        if (!bStr && !bByString)
        {
            double nCellVal;
            switch (aCell.meType)
            {
                case CELLTYPE_VALUE :
                case CELLTYPE_FORMULA :
                    nCellVal = aCell.getValue();
                break;
                default:
                    nCellVal = 0.0;
            }
            if ((nCellVal < rItem.mfVal) && !::rtl::math::approxEqual(
                        nCellVal, rItem.mfVal))
            {
                nRes = -1;
                if (bLessEqual)
                {
                    if (fLastInRangeValue < nCellVal)
                    {
                        fLastInRangeValue = nCellVal;
                        nLastInRange = i;
                    }
                    else if (fLastInRangeValue > nCellVal)
                    {
                        // not strictly sorted, continue with GetThis()
                        nLastInRange = nFirstLastInRange;
                        bDone = true;
                    }
                }
            }
            else if ((nCellVal > rItem.mfVal) && !::rtl::math::approxEqual(
                        nCellVal, rItem.mfVal))
            {
                nRes = 1;
                if (!bLessEqual)
                {
                    if (fLastInRangeValue > nCellVal)
                    {
                        fLastInRangeValue = nCellVal;
                        nLastInRange = i;
                    }
                    else if (fLastInRangeValue < nCellVal)
                    {
                        // not strictly sorted, continue with GetThis()
                        nLastInRange = nFirstLastInRange;
                        bDone = true;
                    }
                }
            }
        }
        else if (bStr && bByString)
        {
            sal_uInt32 nFormat = pCol->GetNumberFormat(mrContext, aCellData.second);
            OUString aCellStr = ScCellFormat::GetInputString(aCell, nFormat, rFormatter, rDoc);

            nRes = rCollator.compareString(aCellStr, rEntry.GetQueryItem().maString.getString());
            if (nRes < 0 && bLessEqual)
            {
                sal_Int32 nTmp = rCollator.compareString( aLastInRangeString,
                        aCellStr);
                if (nTmp < 0)
                {
                    aLastInRangeString = aCellStr;
                    nLastInRange = i;
                }
                else if (nTmp > 0)
                {
                    // not strictly sorted, continue with GetThis()
                    nLastInRange = nFirstLastInRange;
                    bDone = true;
                }
            }
            else if (nRes > 0 && !bLessEqual)
            {
                sal_Int32 nTmp = rCollator.compareString( aLastInRangeString,
                        aCellStr);
                if (nTmp > 0)
                {
                    aLastInRangeString = aCellStr;
                    nLastInRange = i;
                }
                else if (nTmp < 0)
                {
                    // not strictly sorted, continue with GetThis()
                    nLastInRange = nFirstLastInRange;
                    bDone = true;
                }
            }
        }
        else if (!bStr && bByString)
        {
            nRes = -1; // numeric < string
            if (bLessEqual)
                nLastInRange = i;
        }
        else // if (bStr && !bByString)
        {
            nRes = 1; // string > numeric
            if (!bLessEqual)
                nLastInRange = i;
        }
        if (nRes < 0)
        {
            if (bLessEqual)
                nLo = nMid + 1;
            else // assumed to be SC_GREATER_EQUAL
            {
                if (nMid > 0)
                    nHi = nMid - 1;
                else
                    bDone = true;
            }
        }
        else if (nRes > 0)
        {
            if (bLessEqual)
            {
                if (nMid > 0)
                    nHi = nMid - 1;
                else
                    bDone = true;
            }
            else // assumed to be SC_GREATER_EQUAL
                nLo = nMid + 1;
        }
        else
        {
            nLo = i;
            bDone = bFound = true;
        }
    }

    if (!bFound)
    {
        // If all hits didn't result in a moving limit there's something
        // strange, e.g. data range not properly sorted, or only identical
        // values encountered, which doesn't mean there aren't any others in
        // between... leave it to GetThis(). The condition for this would be
        // if (nLastInRange == nFirstLastInRange) nLo = nFirstLastInRange;
        // Else, in case no exact match was found, we step back for a
        // subsequent GetThis() to find the last in range. Effectively this is
        // --nLo with nLastInRange == nLo-1. Both conditions combined yield:
        nLo = nLastInRange;
    }

    aCellData = aIndexer.getCell(nLo);
    if (nLo <= nHi && aCellData.second <= maParam.nRow2)
    {
        nRow = aCellData.second;
        maCurPos = aIndexer.getPosition(nLo);
        return true;
    }
    else
    {
        nRow = maParam.nRow2 + 1;
        // Set current position to the last possible row.
        maCurPos.first = pCol->maCells.end();
        --maCurPos.first;
        maCurPos.second = maCurPos.first->size - 1;
        return false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
