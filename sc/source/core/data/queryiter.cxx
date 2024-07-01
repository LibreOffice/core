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
#include <rtl/math.hxx>

#include <comphelper/flagguard.hxx>
#include <o3tl/safeint.hxx>
#include <svl/numformat.hxx>

#include <global.hxx>
#include <document.hxx>
#include <table.hxx>
#include <column.hxx>
#include <formulacell.hxx>
#include <cellform.hxx>
#include <queryparam.hxx>
#include <queryentry.hxx>
#include <cellvalue.hxx>
#include <queryevaluator.hxx>
#include <rangecache.hxx>
#include <refdata.hxx>

#include <svl/sharedstring.hxx>
#include <unotools/collatorwrapper.hxx>

#include <limits>
#include <vector>

template< ScQueryCellIteratorAccess accessType, ScQueryCellIteratorType queryType >
ScQueryCellIteratorBase< accessType, queryType >::ScQueryCellIteratorBase(ScDocument& rDocument,
    ScInterpreterContext& rContext, SCTAB nTable, const ScQueryParam& rParam, bool bMod, bool bReverse )
    : AccessBase( rDocument, rContext, rParam, bReverse )
    , nStopOnMismatch( nStopOnMismatchDisabled )
    , nTestEqualCondition( nTestEqualConditionDisabled )
    , nSortedBinarySearch( nBinarySearchDisabled )
    , bAdvanceQuery( false )
    , bIgnoreMismatchOnLeadingStrings( false )
    , nSearchOpCode( SC_OPCODE_NONE )
    , nBestFitCol(SCCOL_MAX)
    , nBestFitRow(SCROW_MAX)
{
    nTab = nTable;
    nCol = !bReverse ? maParam.nCol1 : maParam.nCol2;
    nRow = !bReverse ? maParam.nRow1 : maParam.nRow2;
    SCSIZE i;
    if (!bMod) // Or else it's already inserted
        return;

    SCSIZE nCount = maParam.GetEntryCount();
    for (i = 0; (i < nCount) && (maParam.GetEntry(i).bDoQuery); ++i)
    {
        ScQueryEntry& rEntry = maParam.GetEntry(i);
        ScQueryEntry::Item& rItem = rEntry.GetQueryItem();
        sal_uInt32 nIndex = 0;
        bool bNumber = mrContext.NFIsNumberFormat(
            rItem.maString.getString(), nIndex, rItem.mfVal);
        rItem.meType = bNumber ? ScQueryEntry::ByValue : ScQueryEntry::ByString;
    }
}

template< ScQueryCellIteratorAccess accessType, ScQueryCellIteratorType queryType >
void ScQueryCellIteratorBase< accessType, queryType >::PerformQuery()
{
    assert(nTab < rDoc.GetTableCount() && "index out of bounds, FIX IT");
    const ScQueryEntry& rEntry = maParam.GetEntry(0);
    const ScQueryEntry::Item& rItem = rEntry.GetQueryItem();

    const bool bSingleQueryItem = rEntry.GetQueryItems().size() == 1;
    SCCOLROW nFirstQueryField = rEntry.nField;
    bool bAllStringIgnore = bIgnoreMismatchOnLeadingStrings &&
        rItem.meType != ScQueryEntry::ByString;
    bool bFirstStringIgnore = bIgnoreMismatchOnLeadingStrings &&
        !maParam.bHasHeader && rItem.meType == ScQueryEntry::ByString &&
        ((maParam.bByRow && nRow == maParam.nRow1) ||
         (!maParam.bByRow && nCol == maParam.nCol1));
    bool bTestEqualCondition = false;
    const bool bNewSearchFunction = nSearchOpCode == SC_OPCODE_X_LOOKUP || nSearchOpCode == SC_OPCODE_X_MATCH;
    ScQueryEvaluator queryEvaluator(rDoc, *rDoc.maTabs[nTab], maParam, &mrContext,
        (nTestEqualCondition ? &bTestEqualCondition : nullptr), bNewSearchFunction);
    if( queryType == ScQueryCellIteratorType::CountIf )
    {
        // These are not used for COUNTIF, so should not be set, make the compiler
        // explicitly aware of it so that the relevant parts are optimized away.
        assert( !bAllStringIgnore );
        assert( !bIgnoreMismatchOnLeadingStrings );
        assert( nStopOnMismatch == nStopOnMismatchDisabled );
        assert( nTestEqualCondition == nTestEqualConditionDisabled );
        bAllStringIgnore = false;
        bIgnoreMismatchOnLeadingStrings = false;
        nStopOnMismatch = nStopOnMismatchDisabled;
        nTestEqualCondition = nTestEqualConditionDisabled;
        // This one is always set.
        assert( bAdvanceQuery );
        bAdvanceQuery = true;
    }

    const ScColumn* pCol = &(rDoc.maTabs[nTab])->aCol[nCol];
    while (true)
    {
        bool bNextColumn = maCurPos.first == pCol->maCells.end();
        if (!bNextColumn)
        {
            if ((!mbReverseSearch && nRow > maParam.nRow2) || (mbReverseSearch && nRow < maParam.nRow1))
                bNextColumn = true;
        }

        if (bNextColumn)
        {
            do
            {
                if (!mbReverseSearch)
                {
                    ++nCol;
                    SCCOL nAlloCols = rDoc.maTabs[nTab]->GetAllocatedColumnsCount();
                    if (nCol > maParam.nCol2 || nCol >= nAlloCols || (!maParam.bByRow && maParam.nCol2 >= nAlloCols))
                        return;
                }
                else
                {
                    --nCol;
                    if (nCol < maParam.nCol1 || nCol < static_cast<SCCOL>(0))
                        return;
                }
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
                if(HandleItemFound())
                    return;
                !mbReverseSearch ? IncPos() : DecPos();
                continue;
            }
            else
            {
                !mbReverseSearch ? IncBlock() : DecBlock();
                continue;
            }
        }

        ScRefCellValue aCell = sc::toRefCell(maCurPos.first, maCurPos.second);

        if (bAllStringIgnore && aCell.hasString())
            !mbReverseSearch ? IncPos() : DecPos();
        else
        {
            if ( queryEvaluator.ValidQuery( nRow,
                    (nCol == static_cast<SCCOL>(nFirstQueryField) ? &aCell : nullptr)))
            {
                if ( nTestEqualCondition && bTestEqualCondition )
                    nTestEqualCondition |= nTestEqualConditionMatched;
                if ( aCell.isEmpty())
                    return;

                // XLookUp/XMatch: Forward/asc/backward/desc search for best fit value, except if we have an exact match
                if (bNewSearchFunction &&
                    (rEntry.eOp == SC_LESS_EQUAL || rEntry.eOp == SC_GREATER_EQUAL) &&
                    (nBestFitCol != nCol || nBestFitRow != nRow))
                {
                    bool bNumSearch = rItem.meType == ScQueryEntry::ByValue && aCell.hasNumeric();
                    bool bStringSearch = rItem.meType == ScQueryEntry::ByString && aCell.hasString();
                    if (bNumSearch || bStringSearch)
                    {
                        if (nTestEqualCondition == nTestEqualConditionFulfilled || (nBestFitCol == SCCOL_MAX && nBestFitRow == SCROW_MAX))
                            HandleBestFitItemFound(nCol, nRow);
                        else
                        {
                            ScAddress aBFAddr(nBestFitCol, nBestFitRow, nTab);
                            ScRefCellValue aBFCell(rDoc, aBFAddr);
                            ScQueryParam aParamTmp(maParam);
                            ScQueryEntry& rEntryTmp = aParamTmp.GetEntry(0);

                            if (rEntry.eOp == SC_LESS_EQUAL)
                                rEntryTmp.eOp = SC_GREATER;
                            else if (rEntry.eOp == SC_GREATER_EQUAL)
                                rEntryTmp.eOp = SC_LESS;

                            ScQueryEntry::Item& rItemTmp = rEntryTmp.GetQueryItem();
                            if (bNumSearch)
                                rItemTmp.mfVal = aBFCell.getValue();
                            else if (bStringSearch)
                                rItemTmp.maString = svl::SharedString(aBFCell.getString(&rDoc));

                            ScQueryEvaluator queryEvaluatorTmp(rDoc, *rDoc.maTabs[nTab], aParamTmp, &mrContext, nullptr, bNewSearchFunction);
                            if (queryEvaluatorTmp.ValidQuery(nRow, (nCol == static_cast<SCCOL>(nFirstQueryField) ? &aCell : nullptr)))
                                HandleBestFitItemFound(nCol, nRow);
                            else
                            {
                                !mbReverseSearch ? IncPos() : DecPos();
                                continue;
                            }
                        }
                    }
                    else
                    {
                        !mbReverseSearch ? IncPos() : DecPos();
                        continue;
                    }
                }
                if (HandleItemFound())
                    return;
                !mbReverseSearch ? IncPos() : DecPos();
                continue;
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
                    return;
                }
                bool bStop;
                if (bFirstStringIgnore)
                {
                    if (aCell.hasString())
                    {
                        !mbReverseSearch ? IncPos() : DecPos();
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
                    return;
                }
            }
            else
                !mbReverseSearch ? IncPos() : DecPos();
        }
        bFirstStringIgnore = false;
    }
}

template< ScQueryCellIteratorAccess accessType, ScQueryCellIteratorType queryType >
void ScQueryCellIteratorBase< accessType, queryType >::InitPos()
{
    if constexpr( accessType != ScQueryCellIteratorAccess::SortedCache )
        AccessBase::InitPos();
    else
    {
        // This should be all in AccessBase::InitPos(), but that one can't call
        // BinarySearch(), so do it this way instead.
        bool bNewSearchFunction = nSearchOpCode == SC_OPCODE_X_LOOKUP || nSearchOpCode == SC_OPCODE_X_MATCH;
        AccessBase::InitPosStart(bNewSearchFunction, nSortedBinarySearch);
        ScQueryOp& op = maParam.GetEntry(0).eOp;
        SCCOLROW beforeColRow = -1;
        SCCOLROW lastColRow = -1;
        if( op == SC_EQUAL )
        {
            if( BinarySearch( maParam.bByRow ? nCol : nRow) )
            {
                // BinarySearch() searches for the last item that matches. Now we
                // also need to find the first item where to start. Find the last
                // non-matching position using SC_LESS and the start position
                // is the one after it.
                lastColRow = maParam.bByRow ? nRow : nCol;
                ScQueryOp saveOp = op;
                op = SC_LESS;
                if( BinarySearch(maParam.bByRow ? nCol : nRow, true) )
                    beforeColRow = maParam.bByRow ? nRow : nCol;
                // If BinarySearch() returns false, there was no match, which means
                // there's no value smaller. In that case BinarySearch() has set
                // the position to the first row in the range.
                op = saveOp; // back to SC_EQUAL
            }
            else if( maParam.GetEntry(0).GetQueryItem().mbMatchEmpty
                && rDoc.IsEmptyData(maParam.nCol1, maParam.nRow1, maParam.nCol2, maParam.nRow2, nTab))
            {
                // BinarySearch() returns false in case it's all empty data,
                // handle that specially.
                beforeColRow = -1;
                lastColRow = maParam.nRow2;
            }
            if (maParam.bByRow)
                AccessBase::InitPosFinish(beforeColRow, lastColRow, false/*bFirstMatch*/);
            else
                AccessBase::InitPosColFinish(beforeColRow, lastColRow, false/*bFirstMatch*/);
        }
        else
        {   // The range is from the start up to and including the last matching.
            if( BinarySearch( maParam.bByRow ? nCol : nRow) )
            {
                lastColRow = maParam.bByRow ? nRow : nCol;
                if (nSearchOpCode == SC_OPCODE_X_LOOKUP || nSearchOpCode == SC_OPCODE_X_MATCH)
                {
                    ScQueryOp saveOp = op;
                    op = SC_LESS;
                    if( BinarySearch(maParam.bByRow ? nCol : nRow, true) )
                        beforeColRow = maParam.bByRow ? nRow : nCol;
                    op = saveOp;
                }
            }
            if ((nSearchOpCode == SC_OPCODE_X_LOOKUP || nSearchOpCode == SC_OPCODE_X_MATCH) &&
                (lastColRow == beforeColRow || beforeColRow == -1))
            {
                beforeColRow = -1;
                if (maParam.bByRow)
                    AccessBase::InitPosFinish(beforeColRow, lastColRow, true/*bFirstMatch*/);
                else
                {
                    AccessBase::InitPosColFinish(beforeColRow, lastColRow, true/*bFirstMatch*/);
                    AdvanceQueryParamEntryFieldForBinarySearch();
                }
            }
            else
            {
                if (maParam.bByRow)
                    AccessBase::InitPosFinish(beforeColRow, lastColRow, false/*bFirstMatch*/);
                else
                {
                    AccessBase::InitPosColFinish(beforeColRow, lastColRow, false/*bFirstMatch*/);
                    AdvanceQueryParamEntryFieldForBinarySearch();
                }
            }
        }
    }
}

template< ScQueryCellIteratorAccess accessType, ScQueryCellIteratorType queryType >
void ScQueryCellIteratorBase< accessType, queryType >::AdvanceQueryParamEntryField()
{
    SCSIZE nEntries = maParam.GetEntryCount();
    for ( SCSIZE j = 0; j < nEntries; j++  )
    {
        ScQueryEntry& rEntry = maParam.GetEntry( j );
        if ( rEntry.bDoQuery )
        {
            if (!mbReverseSearch && rEntry.nField < rDoc.MaxCol())
                rEntry.nField++;
            else if (mbReverseSearch && rEntry.nField > static_cast<SCCOLROW>(0))
                rEntry.nField--;
            else
            {
                assert(!"AdvanceQueryParamEntryField: ++rEntry.nField > MAXCOL || --rEntry.nField < 0");
            }
        }
        else
            break;  // for
    }
}

template< ScQueryCellIteratorAccess accessType, ScQueryCellIteratorType queryType >
void ScQueryCellIteratorBase< accessType, queryType >::AdvanceQueryParamEntryFieldForBinarySearch()
{
    SCSIZE nEntries = maParam.GetEntryCount();
    for ( SCSIZE j = 0; j < nEntries; j++  )
    {
        ScQueryEntry& rEntry = maParam.GetEntry( j );
        if ( rEntry.bDoQuery )
        {
            if (rEntry.nField < rDoc.MaxCol())
                rEntry.nField = nCol;
            else
            {
                assert(!"AdvanceQueryParamEntryFieldForBinarySearch: rEntry.nField >= MAXCOL");
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

}

template< ScQueryCellIteratorAccess accessType, ScQueryCellIteratorType queryType >
bool ScQueryCellIteratorBase< accessType, queryType >::BinarySearch( SCCOLROW col_row, bool forEqual )
{
    assert(maParam.GetEntry(0).bDoQuery && !maParam.GetEntry(1).bDoQuery
        && maParam.GetEntry(0).GetQueryItems().size() == 1 );
    assert(maParam.eSearchType == utl::SearchParam::SearchType::Normal);
    assert(maParam.GetEntry(0).GetQueryItem().meType == ScQueryEntry::ByString
        || maParam.GetEntry(0).GetQueryItem().meType == ScQueryEntry::ByValue);
    assert(maParam.GetEntry(0).eOp == SC_LESS || maParam.GetEntry(0).eOp == SC_LESS_EQUAL
        || maParam.GetEntry(0).eOp == SC_GREATER || maParam.GetEntry(0).eOp == SC_GREATER_EQUAL
        || maParam.GetEntry(0).eOp == SC_EQUAL);

    // TODO: This will be extremely slow with mdds::multi_type_vector.

    assert(nTab < rDoc.GetTableCount() && "index out of bounds, FIX IT");
    nCol = maParam.bByRow ? col_row : maParam.nCol1;
    nRow = maParam.bByRow ? maParam.nRow1 : col_row;

    if (maParam.bByRow)
    {
        if (nCol >= rDoc.maTabs[nTab]->GetAllocatedColumnsCount())
            return false;
    }
    else
    {
        if (maParam.nCol2 >= rDoc.maTabs[nTab]->GetAllocatedColumnsCount())
            return false;
    }

    const ScColumn* pCol = nullptr;
    if (maParam.bByRow)
    {
        pCol = &(rDoc.maTabs[nTab])->aCol[nCol];
        if (pCol->IsEmptyData())
            return false;
    }
    else
    {
        pCol = &(rDoc.maTabs[nTab])->aCol[nCol];
    }

    CollatorWrapper& rCollator = ScGlobal::GetCollator(maParam.bCaseSens);
    const ScQueryEntry& rEntry = maParam.GetEntry(0);
    const ScQueryEntry::Item& rItem = rEntry.GetQueryItem();
    bool bAscending = rEntry.eOp == SC_LESS || rEntry.eOp == SC_LESS_EQUAL || rEntry.eOp == SC_EQUAL;
    bool bByString = rItem.meType == ScQueryEntry::ByString;
    bool bForceStr = bByString && ( rEntry.eOp == SC_EQUAL || forEqual );
    bool bAllStringIgnore = bIgnoreMismatchOnLeadingStrings && !bByString;
    bool bFirstStringIgnore = bIgnoreMismatchOnLeadingStrings &&
        !maParam.bHasHeader && bByString;

    if (maParam.bHasHeader)
        maParam.bByRow ? ++nRow : ++nCol;

    if (bFirstStringIgnore)
    {
        // move to next col if necessary
        if (!maParam.bByRow && maParam.nCol1 != nCol)
            pCol = &(rDoc.maTabs[nTab])->aCol[nCol];

        sc::CellStoreType::const_position_type aPos = pCol->maCells.position(nRow);
        if (aPos.first->type == sc::element_type_string || aPos.first->type == sc::element_type_edittext)
        {
            ScRefCellValue aCell = sc::toRefCell(aPos.first, aPos.second);
            sal_uInt32 nFormat = pCol->GetNumberFormat(mrContext, nRow);
            OUString aCellStr = ScCellFormat::GetInputString(aCell, nFormat, &mrContext, rDoc);
            sal_Int32 nTmp = rCollator.compareString(aCellStr, rEntry.GetQueryItem().maString.getString());
            if ((rEntry.eOp == SC_LESS_EQUAL && nTmp > 0) ||
                    (rEntry.eOp == SC_GREATER_EQUAL && nTmp < 0) ||
                    (rEntry.eOp == SC_EQUAL && nTmp != 0) ||
                    (rEntry.eOp == SC_LESS && nTmp >= 0) ||
                    (rEntry.eOp == SC_GREATER && nTmp <= 0))
                maParam.bByRow ? ++nRow : ++nCol;
        }
    }

    sc::CellStoreType::const_position_type startPos;
    // Skip leading empty block, if any.
    if (maParam.bByRow)
    {
        startPos = pCol->maCells.position(nRow);
        if (startPos.first->type == sc::element_type_empty)
            incBlock(startPos);
    }
    else
    {
        bool bNonEmpty = false;
        while (nCol != maParam.nCol2 && !bNonEmpty)
        {
            if (maParam.nCol1 != nCol)
                pCol = &(rDoc.maTabs[nTab])->aCol[nCol];
            startPos = pCol->maCells.position(nRow);
            if (startPos.first->type == sc::element_type_empty)
                ++nCol;
            else
                bNonEmpty = true;
        }
    }

    if(bAllStringIgnore)
    {
        // Skip all leading string or empty blocks.
        if (maParam.bByRow)
        {
            while (startPos.first != pCol->maCells.end()
                && (startPos.first->type == sc::element_type_string ||
                    startPos.first->type == sc::element_type_edittext ||
                    startPos.first->type == sc::element_type_empty))
            {
                incBlock(startPos);
            }
        }
        else
        {
            bool bSkipped = false;
            while (nCol != maParam.nCol2 && !bSkipped)
            {
                if (maParam.nCol1 != nCol)
                    pCol = &(rDoc.maTabs[nTab])->aCol[nCol];
                startPos = pCol->maCells.position(nRow);
                if (startPos.first->type == sc::element_type_string ||
                    startPos.first->type == sc::element_type_edittext ||
                    startPos.first->type == sc::element_type_empty)
                    ++nCol;
                else
                    bSkipped = true;
            }
        }
    }

    if (maParam.bByRow)
    {
        if (startPos.first == pCol->maCells.end())
            return false;
        nRow = startPos.first->position + startPos.second;
        if (nRow > maParam.nRow2)
            return false;
    }
    else
    {
        if (nCol > maParam.nCol2)
            return false;
    }

    auto aIndexer = maParam.bByRow ? MakeBinarySearchIndexer(&pCol->maCells, nRow, maParam.nRow2) :
        MakeBinarySearchIndexer(nullptr, nCol, maParam.nCol2);

    if (!aIndexer.isValid())
        return false;

    size_t nLo = aIndexer.getLowIndex();
    size_t nHi = aIndexer.getHighIndex();
    BinarySearchCellType aCellData;

    // Bookkeeping values for breaking up the binary search in case the data
    // range isn't strictly sorted.
    size_t nLastInRange = nLo;
    double fLastInRangeValue = bAscending ?
        -(::std::numeric_limits<double>::max()) :
            ::std::numeric_limits<double>::max();
    OUString aLastInRangeString;
    if (!bAscending)
        aLastInRangeString = OUString(u'\xFFFF');

    if (maParam.bByRow)
        aCellData = aIndexer.getCell(nLastInRange);
    else
        aCellData = aIndexer.getColCell(nLastInRange, nRow);

    ScRefCellValue aCell = aCellData.first;
    if (bForceStr || aCell.hasString())
    {
        sal_uInt32 nFormat = pCol->GetNumberFormat(mrContext, maParam.bByRow ? aCellData.second : nRow);
        aLastInRangeString = ScCellFormat::GetInputString(aCell, nFormat, &mrContext, rDoc);
    }
    else
    {
        switch (aCell.getType())
        {
            case CELLTYPE_VALUE :
                fLastInRangeValue = aCell.getDouble();
            break;
            case CELLTYPE_FORMULA :
                fLastInRangeValue = aCell.getFormula()->GetValue();
            break;
            default:
            {
                // added to avoid warnings
            }
        }
    }

    sal_Int32 nRes = 0;
    std::optional<size_t> found;
    bool bDone = false;
    bool orderBroken = false;
    while (nLo <= nHi && !bDone)
    {
        size_t nMid = (nLo+nHi)/2;
        size_t i = nMid;

        if (maParam.bByRow)
            aCellData = aIndexer.getCell(i);
        else
            aCellData = aIndexer.getColCell(i, nRow);
        aCell = aCellData.first;
        bool bStr = bForceStr || aCell.hasString();
        nRes = 0;

        // compares are content<query:-1, content>query:1
        // Cell value comparison similar to ScTable::ValidQuery()
        if (!bStr && !bByString)
        {
            double nCellVal;
            switch (aCell.getType())
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
                if (bAscending)
                {
                    if (fLastInRangeValue <= nCellVal)
                    {
                        fLastInRangeValue = nCellVal;
                        nLastInRange = i;
                    }
                    else if (fLastInRangeValue >= nCellVal)
                    {
                        // not strictly sorted, continue with GetThis()
                        orderBroken = true;
                        bDone = true;
                    }
                }
            }
            else if ((nCellVal > rItem.mfVal) && !::rtl::math::approxEqual(
                        nCellVal, rItem.mfVal))
            {
                nRes = 1;
                if (!bAscending)
                {
                    if (fLastInRangeValue >= nCellVal)
                    {
                        fLastInRangeValue = nCellVal;
                        nLastInRange = i;
                    }
                    else if (fLastInRangeValue <= nCellVal)
                    {
                        // not strictly sorted, continue with GetThis()
                        orderBroken = true;
                        bDone = true;
                    }
                }
            }
        }
        else if (bStr && bByString)
        {
            if (!maParam.bByRow)
                pCol = &(rDoc.maTabs[nTab])->aCol[aCellData.second];
            sal_uInt32 nFormat = pCol->GetNumberFormat(mrContext, maParam.bByRow ? aCellData.second : nRow);
            OUString aCellStr = ScCellFormat::GetInputString(aCell, nFormat, &mrContext, rDoc);

            nRes = rCollator.compareString(aCellStr, rEntry.GetQueryItem().maString.getString());
            if (nRes < 0 && bAscending)
            {
                sal_Int32 nTmp = rCollator.compareString( aLastInRangeString,
                        aCellStr);
                if (nTmp <= 0)
                {
                    aLastInRangeString = aCellStr;
                    nLastInRange = i;
                }
                else if (nTmp > 0)
                {
                    // not strictly sorted, continue with GetThis()
                    orderBroken = true;
                    bDone = true;
                }
            }
            else if (nRes > 0 && !bAscending)
            {
                sal_Int32 nTmp = rCollator.compareString( aLastInRangeString,
                        aCellStr);
                if (nTmp >= 0)
                {
                    aLastInRangeString = aCellStr;
                    nLastInRange = i;
                }
                else if (nTmp < 0)
                {
                    // not strictly sorted, continue with GetThis()
                    orderBroken = true;
                    bDone = true;
                }
            }
        }
        else if (!bStr && bByString)
        {
            nRes = -1; // numeric < string
            if (bAscending)
                nLastInRange = i;
        }
        else // if (bStr && !bByString)
        {
            nRes = 1; // string > numeric
            if (!bAscending)
                nLastInRange = i;
        }
        if (nRes < 0)
        {
            if (bAscending)
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
            if (bAscending)
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
            if(rEntry.eOp == SC_LESS_EQUAL || rEntry.eOp == SC_GREATER_EQUAL || rEntry.eOp == SC_EQUAL)
            {
                found = i;
                nLastInRange = i;
                nLo = nMid + 1;
            }
            else if (bAscending)
            {
                if (nMid > 0)
                    nHi = nMid - 1;
                else
                    bDone = true;
            }
            else
            {
                if (nMid > 0)
                    nHi = nMid - 1;
                else
                    bDone = true;
            }
        }
    }

    bool isInRange;
    if (orderBroken)
    {
        // Reset position to the first row in range and force caller
        // to search from start.
        nLo = aIndexer.getLowIndex();
        isInRange = false;
    }
    else if (found)
    {
        nLo = *found;
        isInRange = true;
    }
    else
    {
        // Not nothing was found and the search position is at the start,
        // then the possible match would need to be before the data range.
        // In that case return false to force the caller to search from the start
        // and detect this.
        isInRange = nLo != aIndexer.getLowIndex();
        // If nothing was found, that is either because there is no value
        // that would match exactly, or the data range is not properly sorted
        // and we failed to detect (doing so reliably would require a linear scan).
        // Set the position to the last one that was in matching range (i.e. before
        // where the exact match would be), and leave sorting it out to GetThis()
        // or whatever the caller uses.
        nLo = nLastInRange;
    }

    if (maParam.bByRow)
    {
        aCellData = aIndexer.getCell(nLo);
        if (nLo <= nHi && aCellData.second <= maParam.nRow2)
        {
            nRow = aCellData.second;
            maCurPos = aIndexer.getPosition(nLo);
            return isInRange;
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
    else
    {
        aCellData = aIndexer.getColCell(nLo, nRow);
        if (nLo <= nHi && aCellData.second <= maParam.nCol2)
        {
            nCol = aCellData.second;
            maCurPos = aIndexer.getColPosition(nLo, nRow);
            return isInRange;
        }
        else
        {
            nCol = maParam.nCol2 + 1;
            // Set current position to the last possible col.
            pCol = &(rDoc.maTabs[nTab])->aCol[maParam.nCol2];
            maCurPos.first = pCol->maCells.end();
            --maCurPos.first;
            maCurPos.second = maCurPos.first->size - 1;
            return false;
        }
    }
}


template< ScQueryCellIteratorAccess accessType >
bool ScQueryCellIterator< accessType >::FindEqualOrSortedLastInRange( SCCOL& nFoundCol,
        SCROW& nFoundRow )
{
    // Set and automatically reset mpParam->mbRangeLookup when returning.
    comphelper::FlagRestorationGuard aRangeLookupResetter( maParam.mbRangeLookup, true );

    nFoundCol = rDoc.MaxCol()+1;
    nFoundRow = rDoc.MaxRow()+1;

    if ((nSearchOpCode == SC_OPCODE_X_LOOKUP || nSearchOpCode == SC_OPCODE_X_MATCH) &&
        nSortedBinarySearch == nBinarySearchDisabled)
        SetStopOnMismatch( false ); // assume not sorted keys for XLookup/XMatch
    else
        SetStopOnMismatch( true ); // assume sorted keys

    SetTestEqualCondition( true );
    bIgnoreMismatchOnLeadingStrings = true;

    bool bLiteral = maParam.eSearchType == utl::SearchParam::SearchType::Normal &&
        maParam.GetEntry(0).GetQueryItem().meType == ScQueryEntry::ByString;
    bool bBinary = maParam.bByRow &&
        (bLiteral || maParam.GetEntry(0).GetQueryItem().meType == ScQueryEntry::ByValue) &&
        (maParam.GetEntry(0).eOp == SC_LESS_EQUAL || maParam.GetEntry(0).eOp == SC_GREATER_EQUAL);

    // assume not sorted properly if we are using XLookup/XMatch with forward or backward search
    if (bBinary && (nSearchOpCode == SC_OPCODE_X_LOOKUP || nSearchOpCode == SC_OPCODE_X_MATCH) &&
        nSortedBinarySearch == nBinarySearchDisabled)
        bBinary = false;

    bool bFound = false;
    if (bBinary)
    {
        if (BinarySearch( maParam.nCol1 ))
        {
            // BinarySearch() already positions correctly and only needs real
            // query comparisons afterwards, skip the verification check below.
            maParam.mbRangeLookup = false;
            bFound = GetThis();
        }
        else // Not sorted properly, or before the range (in which case GetFirst() will be simple).
            bFound = GetFirst();
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
        SCSIZE nEntries = maParam.GetEntryCount();
        std::vector<SCCOL> aFoundFieldPositions(nEntries);
        do
        {
            nFoundCol = GetCol();
            nFoundRow = GetRow();
            aPosSave = maCurPos;
            // If we might need to rewind below, save the position to rewind to
            // rather than calculate it as a diff between nCol and nFoundCol as
            // PerformQuery can return early if nCol is greater than
            // maParam.nCol2 or AllocatedColumns
            if (maParam.mbRangeLookup && bAdvanceQuery)
            {
                for (SCSIZE j=0; j < nEntries; ++j)
                {
                    ScQueryEntry& rEntry = maParam.GetEntry( j );
                    if (rEntry.bDoQuery)
                        aFoundFieldPositions[j] = maParam.GetEntry(j).nField;
                    else
                        break;  // for
                }
            }
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
            bool bColDiff = nCol != nFoundCol;
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
                if (bAdvanceQuery && bColDiff)
                {
                    for (SCSIZE j=0; j < nEntries; ++j)
                    {
                        ScQueryEntry& rEntry = maParam.GetEntry( j );
                        if (rEntry.bDoQuery)
                        {
                            rEntry.nField = aFoundFieldPositions[j];
                            assert(rEntry.nField >= 0);
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
    if (IsEqualConditionFulfilled() && (nSearchOpCode != SC_OPCODE_X_LOOKUP &&
        nSearchOpCode != SC_OPCODE_X_MATCH))
    {
        // Position on last equal entry, except for XLOOKUP,
        // which looking for the first equal entry
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

// Direct linear cell access using mdds.

ScQueryCellIteratorAccessSpecific< ScQueryCellIteratorAccess::Direct >
    ::ScQueryCellIteratorAccessSpecific( ScDocument& rDocument,
        ScInterpreterContext& rContext, const ScQueryParam& rParam, bool bReverseSearch )
    : maParam( rParam )
    , rDoc( rDocument )
    , mrContext( rContext )
    , mbReverseSearch( bReverseSearch )
{
    // coverity[uninit_member] - this just contains data, subclass will initialize some of it
}

void ScQueryCellIteratorAccessSpecific< ScQueryCellIteratorAccess::Direct >::InitPos()
{
    if (!mbReverseSearch)
    {
        nRow = maParam.nRow1;
        if (maParam.bHasHeader && maParam.bByRow)
            ++nRow;
    }
    else
    {
        nRow = maParam.nRow2;
    }
    const ScColumn& rCol = rDoc.maTabs[nTab]->CreateColumnIfNotExists(nCol);
    maCurPos = rCol.maCells.position(nRow);
}

void ScQueryCellIteratorAccessSpecific< ScQueryCellIteratorAccess::Direct >::IncPos()
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

void ScQueryCellIteratorAccessSpecific< ScQueryCellIteratorAccess::Direct >::DecPos()
{
    if (maCurPos.second > 0)
    {
        // Move within the same block.
        --maCurPos.second;
        --nRow;
    }
    else
        // Move to the prev block.
        DecBlock();
}

void ScQueryCellIteratorAccessSpecific< ScQueryCellIteratorAccess::Direct >::IncBlock()
{
    ++maCurPos.first;
    maCurPos.second = 0;

    nRow = maCurPos.first->position;
}

void ScQueryCellIteratorAccessSpecific< ScQueryCellIteratorAccess::Direct >::DecBlock()
{
    // Set current position to the last possible row.
    const ScColumn& rCol = rDoc.maTabs[nTab]->CreateColumnIfNotExists(nCol);
    if (maCurPos.first != rCol.maCells.begin())
    {
        --maCurPos.first;
        maCurPos.second = maCurPos.first->size - 1;

        nRow = maCurPos.first->position + maCurPos.second;
    }
    else
    {
        // No rows, set to end. This will make PerformQuery() go to next column.
        nRow = maParam.nRow1 - 1;
        maCurPos.first = rCol.maCells.end();
        maCurPos.second = 0;
    }
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
class ScQueryCellIteratorAccessSpecific< ScQueryCellIteratorAccess::Direct >::NonEmptyCellIndexer
{
    typedef std::map<size_t, sc::CellStoreType::const_iterator> BlockMapType;

    BlockMapType maBlockMap;

    const sc::CellStoreType* mpCells;

    size_t mnLowIndex;
    size_t mnHighIndex;

    bool mbValid;

public:
    /**
     * @param rCells cell storage container
     * @param nStartRow logical start row position
     * @param nEndRow logical end row position, inclusive.
     */
    NonEmptyCellIndexer(
        const sc::CellStoreType* pCells, SCROW nStartRow, SCROW nEndRow) :
        mpCells(pCells), mnLowIndex(0), mnHighIndex(0), mbValid(true)
    {
        // Find the low position.

        sc::CellStoreType::const_position_type aLoPos = mpCells->position(nStartRow);
        assert(aLoPos.first->type != sc::element_type_empty);
        assert(aLoPos.first != mpCells->end());

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

        sc::CellStoreType::const_position_type aHiPos = mpCells->position(aLoPos.first, nEndRow);
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

            assert(itBlk != mpCells->end());
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

        sc::CellStoreType::const_position_type aRet(mpCells->end(), 0);

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

    BinarySearchCellType getCell( size_t nIndex ) const
    {
        BinarySearchCellType aRet;
        aRet.second = -1;

        sc::CellStoreType::const_position_type aPos = getPosition(nIndex);
        if (aPos.first == mpCells->end())
            return aRet;

        aRet.first = sc::toRefCell(aPos.first, aPos.second);
        aRet.second = aPos.first->position + aPos.second;
        return aRet;
    }

    // TODO: NonEmptyCellIndexer for columns search
    static sc::CellStoreType::const_position_type getColPosition(size_t /*nIndex*/, SCROW /*nRow*/)
    {
        return sc::CellStoreType::const_position_type();
    }

    // TODO: NonEmptyCellIndexer for columns search
    static BinarySearchCellType getColCell(size_t /*nIndex*/, SCROW /*nRow*/)
    {
        BinarySearchCellType aRet;
        return aRet;
    }

    size_t getLowIndex() const { return mnLowIndex; }

    size_t getHighIndex() const { return mnHighIndex; }

    bool isValid() const { return mbValid; }
};

ScQueryCellIteratorAccessSpecific< ScQueryCellIteratorAccess::Direct >::NonEmptyCellIndexer
ScQueryCellIteratorAccessSpecific< ScQueryCellIteratorAccess::Direct >::MakeBinarySearchIndexer(
    const sc::CellStoreType* pCells, SCCOLROW nStartRow, SCCOLROW nEndRow)
{
    return NonEmptyCellIndexer(pCells, nStartRow, nEndRow);
}

// Sorted access using ScSortedRangeCache.

ScQueryCellIteratorAccessSpecific< ScQueryCellIteratorAccess::SortedCache >
    ::ScQueryCellIteratorAccessSpecific( ScDocument& rDocument,
        ScInterpreterContext& rContext, const ScQueryParam& rParam, bool bReverseSearch )
    : maParam( rParam )
    , rDoc( rDocument )
    , mrContext( rContext )
    , mbReverseSearch( bReverseSearch )
{
    // coverity[uninit_member] - this just contains data, subclass will initialize some of it
}

void ScQueryCellIteratorAccessSpecific< ScQueryCellIteratorAccess::SortedCache >::SetSortedRangeCache(
    const ScSortedRangeCache& cache)
{
    sortedCache = &cache;
}

// The idea in iterating using the sorted cache is that the iteration is instead done
// over indexes of the sorted cache (which is a stable sort of the cell contents) in the range
// that fits the query condition and then that is mapped to rows. This will result in iterating
// over only matching rows in their sorted order (and for equal rows in their row order).
void ScQueryCellIteratorAccessSpecific< ScQueryCellIteratorAccess::SortedCache >::InitPosStart(bool bNewSearchFunction, sal_uInt8 nSortedBinarySearch)
{
    ScRange aSortedRangeRange( maParam.nCol1, maParam.nRow1, nTab, maParam.nCol2, maParam.nRow2, nTab );
    // We want all matching values first in the sort order,
    SetSortedRangeCache( rDoc.GetSortedRangeCache( aSortedRangeRange, maParam, &mrContext, bNewSearchFunction, nSortedBinarySearch ));
    // InitPosFinish() needs to be called after this, ScQueryCellIteratorBase::InitPos()
    // will handle that
}

void ScQueryCellIteratorAccessSpecific< ScQueryCellIteratorAccess::SortedCache >::InitPosFinish(
    SCROW beforeRow, SCROW lastRow, bool bFirstMatch )
{
    pColumn = &rDoc.maTabs[nTab]->CreateColumnIfNotExists(nCol);
    if(lastRow >= 0)
    {
        sortedCachePos = beforeRow >= 0 ? sortedCache->indexForRow(beforeRow) + 1 : 0;
        sortedCachePosLast = sortedCache->indexForRow(lastRow);
        if(sortedCachePos <= sortedCachePosLast)
        {
            if (!bFirstMatch)
                nRow = sortedCache->rowForIndex(sortedCachePos);
            else
                nRow = sortedCache->rowForIndex(sortedCachePosLast);
            maCurPos = pColumn->maCells.position(nRow);
            return;
        }
    }
    // No rows, set to end.
    sortedCachePos = sortedCachePosLast = 0;
    maCurPos.first = pColumn->maCells.end();
    maCurPos.second = 0;
}

void ScQueryCellIteratorAccessSpecific< ScQueryCellIteratorAccess::SortedCache >::InitPosColFinish(
    SCCOL beforeCol, SCCOL lastCol, bool bFirstMatch)
{
    pColumn = &rDoc.maTabs[nTab]->CreateColumnIfNotExists(nCol);
    if (lastCol >= 0)
    {
        sortedCachePos = beforeCol >= 0 ? sortedCache->indexForCol(beforeCol) + 1 : 0;
        sortedCachePosLast = sortedCache->indexForCol(lastCol);
        if (sortedCachePos <= sortedCachePosLast)
        {
            if (!bFirstMatch)
                nCol = sortedCache->colForIndex(sortedCachePos);
            else
                nCol = sortedCache->colForIndex(sortedCachePosLast);
            pColumn = &rDoc.maTabs[nTab]->CreateColumnIfNotExists(nCol);
            maCurPos = pColumn->maCells.position(nRow);
            return;
        }
    }
    // No cols, set to end.
    sortedCachePos = sortedCachePosLast = 0;
    maCurPos.first = pColumn->maCells.end();
    maCurPos.second = 0;
}

template<bool fast>
bool ScQueryCellIteratorAccessSpecific< ScQueryCellIteratorAccess::SortedCache >::IncPosImpl()
{
    if(sortedCachePos < sortedCachePosLast)
    {
        ++sortedCachePos;
        if (maParam.bByRow)
            nRow = sortedCache->rowForIndex(sortedCachePos);
        else
            nCol = sortedCache->colForIndex(sortedCachePos);
#ifndef DBG_UTIL
        if constexpr (!fast)
#endif
        {
            if (maParam.bByRow)
            {
                // Avoid mdds position() call if row is in the same block.
                if (maCurPos.first != pColumn->maCells.end() && o3tl::make_unsigned(nRow) >= maCurPos.first->position
                    && o3tl::make_unsigned(nRow) < maCurPos.first->position + maCurPos.first->size)
                    maCurPos.second = nRow - maCurPos.first->position;
                else
                    maCurPos = pColumn->maCells.position(nRow);
            }
        }
        return true;
    }
    else
    {
        // This will make PerformQuery() go to next column.
        // Necessary even in fast mode, as GetNext() will call GetThis() in this case.
        if (!maParam.bByRow)
            ++nRow;
        maCurPos.first = pColumn->maCells.end();
        maCurPos.second = 0;
        return false;
    }
}

template
bool ScQueryCellIteratorAccessSpecific<ScQueryCellIteratorAccess::SortedCache>::IncPosImpl<false>();
template
bool ScQueryCellIteratorAccessSpecific<ScQueryCellIteratorAccess::SortedCache>::IncPosImpl<true>();

// Helper that allows binary search of unsorted cells using ScSortedRangeCache.
// Rows in the given range are kept in a sorted vector and that vector is binary-searched.
class ScQueryCellIteratorAccessSpecific< ScQueryCellIteratorAccess::SortedCache >::SortedCacheIndexer
{
    std::vector<SCCOLROW> mSortedColsRowsCopy;
    const std::vector<SCCOLROW>& mSortedColsRows;
    ScDocument& mrDoc;
    const sc::CellStoreType* mpCells;
    size_t mLowIndex;
    size_t mHighIndex;
    bool mValid;
    SCTAB mnTab;

    const std::vector<SCCOLROW>& makeSortedColsRows( const ScSortedRangeCache* cache, SCCOLROW startColRow, SCCOLROW endColRow )
    {
        // Keep a reference to cols/rows from the cache if equal, otherwise make a copy.
        if (cache->isRowSearch())
        {
            if (startColRow == cache->getRange().aStart.Row() && endColRow == cache->getRange().aEnd.Row())
                return cache->sortedRows();
            else
            {
                mSortedColsRowsCopy.reserve(cache->sortedRows().size());
                for (SCROW row : cache->sortedRows())
                    if (row >= startColRow && row <= endColRow)
                        mSortedColsRowsCopy.emplace_back(row);
                return mSortedColsRowsCopy;
            }
        }
        else
        {
            if (startColRow == cache->getRange().aStart.Col() && endColRow == cache->getRange().aEnd.Col())
                return cache->sortedCols();
            else
            {
                mSortedColsRowsCopy.reserve(cache->sortedCols().size());
                for (SCCOL col : cache->sortedCols())
                    if (col >= startColRow && col <= endColRow)
                        mSortedColsRowsCopy.emplace_back(col);
                return mSortedColsRowsCopy;
            }
        }
    }

public:
    SortedCacheIndexer( ScDocument& rDoc, const sc::CellStoreType* cells,
        SCCOLROW startColRow, SCCOLROW endColRow, SCTAB nTab,
        const ScSortedRangeCache* cache )
        : mSortedColsRows( makeSortedColsRows( cache, startColRow, endColRow))
        , mrDoc( rDoc )
        , mpCells( cells )
        , mValid( false )
        , mnTab( nTab )
    {
        if(mSortedColsRows.empty())
        {
            // coverity[uninit_member] - these are initialized only if valid
            return;
        }
        mLowIndex = 0;
        mHighIndex = mSortedColsRows.size() - 1;
        mValid = true;
    }

    sc::CellStoreType::const_position_type getPosition( size_t nIndex ) const
    {
        // TODO optimize?
        SCROW row = mSortedColsRows[ nIndex ];
        return mpCells->position(row);
    }

    BinarySearchCellType getCell( size_t nIndex ) const
    {
        BinarySearchCellType aRet;
        aRet.second = -1;

        sc::CellStoreType::const_position_type aPos = getPosition(nIndex);
        if (aPos.first == mpCells->end())
            return aRet;

        aRet.first = sc::toRefCell(aPos.first, aPos.second);
        aRet.second = aPos.first->position + aPos.second;
        return aRet;
    }

    sc::CellStoreType::const_position_type getColPosition( size_t nColIndex, SCROW nRowPos ) const
    {
        // TODO optimize?
        SCCOL col = mSortedColsRows[nColIndex];
        if (col >= mrDoc.maTabs[mnTab]->GetAllocatedColumnsCount())
            return sc::CellStoreType::const_position_type();

        const ScColumn& rCol = mrDoc.maTabs[mnTab]->aCol[col];
        return rCol.maCells.position(nRowPos);
    }

    BinarySearchCellType getColCell( size_t nColIndex, SCROW nRowPos) const
    {
        BinarySearchCellType aRet;
        aRet.second = -1;

        sc::CellStoreType::const_position_type aPos = getColPosition(nColIndex, nRowPos);

        aRet.first = sc::toRefCell(aPos.first, aPos.second);
        aRet.second = mSortedColsRows[nColIndex];

        return aRet;
    }

    size_t getLowIndex() const { return mLowIndex; }

    size_t getHighIndex() const { return mHighIndex; }

    bool isValid() const { return mValid; }
};

ScQueryCellIteratorAccessSpecific< ScQueryCellIteratorAccess::SortedCache >::SortedCacheIndexer
ScQueryCellIteratorAccessSpecific< ScQueryCellIteratorAccess::SortedCache >::MakeBinarySearchIndexer(
    const sc::CellStoreType* pCells, SCCOLROW nStartRow, SCCOLROW nEndRow)
{
    return SortedCacheIndexer(rDoc, pCells, nStartRow, nEndRow, nTab, sortedCache);
}

static bool CanBeUsedForSorterCache(ScDocument& /*rDoc*/, const ScQueryParam& /*rParam*/,
    SCTAB /*nTab*/, const ScFormulaCell* /*cell*/, const ScComplexRefData* /*refData*/,
    ScInterpreterContext& /*context*/)
{
#if 1
    /* TODO: tdf#151958 broken by string query of binary search on sorted
     * cache, use the direct query instead for releases and fix SortedCache
     * implementation after. Not only COUNTIF() is broken, but also COUNTIFS(),
     * and maybe lcl_LookupQuery() for VLOOKUP() etc. as well. Just disable
     * this for now.
     * Can't just return false because below would be unreachable code. Can't
     * just #if/#else/#endif either because parameters would be unused. Crap
     * this and comment out parameter names. */
    return false;
#else
    if(!rParam.GetEntry(0).bDoQuery || rParam.GetEntry(1).bDoQuery
        || rParam.GetEntry(0).GetQueryItems().size() != 1 )
        return false;
    if(rParam.eSearchType != utl::SearchParam::SearchType::Normal)
        return false;
    if(rParam.GetEntry(0).GetQueryItem().meType != ScQueryEntry::ByValue
        && rParam.GetEntry(0).GetQueryItem().meType != ScQueryEntry::ByString)
        return false;
    if(!rParam.bByRow)
        return false;
    if(rParam.bHasHeader)
        return false;
    if(rParam.mbRangeLookup)
        return false;
    const bool bNewSearchFunction = nSearchOpCode == SC_OPCODE_X_LOOKUP || nSearchOpCode == SC_OPCODE_X_MATCH;
    if(rParam.GetEntry(0).GetQueryItem().meType == ScQueryEntry::ByString && !bNewSearchFunction
        && !ScQueryEvaluator::isMatchWholeCell(rDoc, rParam.GetEntry(0).eOp))
        return false; // substring matching cannot be sorted
    if(rParam.GetEntry(0).eOp != SC_LESS && rParam.GetEntry(0).eOp != SC_LESS_EQUAL
        && rParam.GetEntry(0).eOp != SC_GREATER && rParam.GetEntry(0).eOp != SC_GREATER_EQUAL
        && rParam.GetEntry(0).eOp != SC_EQUAL)
        return false;
    // For unittests allow inefficient caching, in order for the code to be checked.
    static bool inUnitTest = getenv("LO_TESTNAME") != nullptr;
    if(refData == nullptr || refData->Ref1.IsRowRel() || refData->Ref2.IsRowRel())
    {
        // If this is not a range, then a cache is not worth it. If rows are relative, then each
        // computation will use a different area, so the cache wouldn't be reused. Tab/cols are
        // not a problem, because formula group computations are done for the same tab/col.
        if(!inUnitTest)
            return false;
    }
    if(rParam.nRow2 - rParam.nRow1 < 10)
    {
        if(!inUnitTest)
            return false;
    }
    if( !cell )
        return false;
    if( !cell->GetCellGroup() || cell->GetCellGroup()->mnLength < 10 )
    {
        if(!inUnitTest)
            return false;
    }
    // Check that all the relevant caches would be valid (may not be the case when mixing
    // numeric and string cells for ByValue lookups).
    for(SCCOL col : rDoc.GetAllocatedColumnsRange(nTab, rParam.nCol1, rParam.nCol2))
    {
        ScRange aSortedRangeRange( col, rParam.nRow1, nTab, col, rParam.nRow2, nTab);
        if( aSortedRangeRange.Contains( cell->aPos ))
            return false; // self-referencing, can't create cache
        ScSortedRangeCache& cache = rDoc.GetSortedRangeCache( aSortedRangeRange, rParam, &context );
        if(!cache.isValid())
            return false;
    }
    return true;
#endif
}

// Generic query implementation.

bool ScQueryCellIteratorTypeSpecific< ScQueryCellIteratorType::Generic >::HandleItemFound()
{
    getThisResult = true;
    return true; // Return from PerformQuery().
}

template< ScQueryCellIteratorAccess accessType >
bool ScQueryCellIterator< accessType >::GetThis()
{
    getThisResult = false;
    PerformQuery();
    return getThisResult;
}

template< ScQueryCellIteratorAccess accessType >
bool ScQueryCellIterator< accessType >::GetFirst()
{
    assert(nTab < rDoc.GetTableCount() && "index out of bounds, FIX IT");
    if (!mbReverseSearch)
        nCol = maParam.nCol1;
    else
        nCol = maParam.nCol2;
    InitPos();
    return GetThis();
}

template< ScQueryCellIteratorAccess accessType >
bool ScQueryCellIterator< accessType >::GetNext()
{
    if (!mbReverseSearch)
        IncPos();
    else
        DecPos();
    if ( nStopOnMismatch )
        nStopOnMismatch = nStopOnMismatchEnabled;
    if ( nTestEqualCondition )
        nTestEqualCondition = nTestEqualConditionEnabled;
    return GetThis();
}

template<>
bool ScQueryCellIterator< ScQueryCellIteratorAccess::SortedCache >::GetNext()
{
    assert( !nStopOnMismatch );
    assert( !nTestEqualCondition );
    // When searching using sorted cache, we should always find cells that match,
    // because InitPos()/IncPos() select only such rows, so skip GetThis() (and thus
    // the somewhat expensive PerformQuery) as long as we're not at the end
    // of a column. As an optimization IncPosFast() returns true if not at the end,
    // in which case in non-DBG_UTIL mode it doesn't even bother to set maCurPos.
    if( IncPosFast())
    {
#ifdef DBG_UTIL
        assert(GetThis());
#endif
        return true;
    }
    return GetThis();
}

bool ScQueryCellIteratorSortedCache::CanBeUsed(ScDocument& rDoc, const ScQueryParam& rParam,
    SCTAB nTab, const ScFormulaCell* cell, const ScComplexRefData* refData,
    ScInterpreterContext& context)
{
    return CanBeUsedForSorterCache(rDoc, rParam, nTab, cell, refData, context);
}

// Countifs implementation.

bool ScQueryCellIteratorTypeSpecific< ScQueryCellIteratorType::CountIf >::HandleItemFound()
{
    ++countIfCount;
    return false; // Continue searching.
}

template< ScQueryCellIteratorAccess accessType >
sal_uInt64 ScCountIfCellIterator< accessType >::GetCount()
{
    // Keep Entry.nField in iterator on column change
    SetAdvanceQueryParamEntryField( true );
    assert(nTab < rDoc.GetTableCount() && "try to access index out of bounds, FIX IT");
    maParam.nCol1 = rDoc.ClampToAllocatedColumns(nTab, maParam.nCol1);
    maParam.nCol2 = rDoc.ClampToAllocatedColumns(nTab, maParam.nCol2);
    nCol = maParam.nCol1;
    InitPos();
    countIfCount = 0;
    PerformQuery();
    return countIfCount;
}


bool ScCountIfCellIteratorSortedCache::CanBeUsed(ScDocument& rDoc, const ScQueryParam& rParam,
    SCTAB nTab, const ScFormulaCell* cell, const ScComplexRefData* refData,
    ScInterpreterContext& context)
{
    return CanBeUsedForSorterCache(rDoc, rParam, nTab, cell, refData, context);
}

template<>
sal_uInt64 ScCountIfCellIterator< ScQueryCellIteratorAccess::SortedCache >::GetCount()
{
    // Keep Entry.nField in iterator on column change
    SetAdvanceQueryParamEntryField( true );
    assert(nTab < rDoc.GetTableCount() && "try to access index out of bounds, FIX IT");
    sal_uInt64 count = 0;
    // Each column must be sorted separately.
    for(SCCOL col : rDoc.GetAllocatedColumnsRange(nTab, maParam.nCol1, maParam.nCol2))
    {
        nCol = col;
        nRow = maParam.nRow1;
        ScRange aSortedRangeRange( col, maParam.nRow1, nTab, col, maParam.nRow2, nTab);
        ScQueryOp& op = maParam.GetEntry(0).eOp;
        bool bNewSearchFunction = nSearchOpCode == SC_OPCODE_X_LOOKUP || nSearchOpCode == SC_OPCODE_X_MATCH;
        SetSortedRangeCache( rDoc.GetSortedRangeCache( aSortedRangeRange, maParam, &mrContext, bNewSearchFunction, nSearchOpCode ));
        if( op == SC_EQUAL )
        {
            // BinarySearch() searches for the last item that matches. Therefore first
            // find the last non-matching position using SC_LESS and then find the last
            // matching position using SC_EQUAL.
            ScQueryOp saveOp = op;
            op = SC_LESS;
            if( BinarySearch( nCol, true ))
            {
                op = saveOp; // back to SC_EQUAL
                size_t lastNonMatching = sortedCache->indexForRow(nRow);
                if( BinarySearch( nCol ))
                {
                    size_t lastMatching = sortedCache->indexForRow(nRow);
                    assert(lastMatching >= lastNonMatching);
                    count += lastMatching - lastNonMatching;
                }
                else
                {
                    // BinarySearch() should at least find the same result as the SC_LESS
                    // call, so this should not happen.
                    assert(false);
                }
            }
            else
            {
                // BinarySearch() returning false means that all values are larger,
                // so try to find matching ones and count those up to and including
                // the found one.
                op = saveOp; // back to SC_EQUAL
                if( BinarySearch( nCol ))
                {
                    size_t lastMatching = sortedCache->indexForRow(nRow) + 1;
                    count += lastMatching;
                }
                else if( maParam.GetEntry(0).GetQueryItem().mbMatchEmpty
                    && rDoc.IsEmptyData(col, maParam.nRow1, col, maParam.nRow2, nTab))
                {
                    // BinarySearch() returns false in case it's all empty data,
                    // handle that specially.
                    count += maParam.nRow2 - maParam.nRow1 + 1;
                }
            }
        }
        else
        {
            // BinarySearch() searches for the last item that matches. Therefore everything
            // up to and including the found row matches the condition.
            if( BinarySearch( nCol ))
                count += sortedCache->indexForRow(nRow) + 1;
        }
    }
    if( maParam.GetEntry(0).GetQueryItem().mbMatchEmpty
        && maParam.nCol2 >= rDoc.GetAllocatedColumnsCount( nTab ))
    {
        const sal_uInt64 nRows = maParam.nRow2 - maParam.nRow1 + 1;
        count += (maParam.nCol2 - rDoc.GetAllocatedColumnsCount(nTab)) * nRows;
    }
    return count;
}

template class ScQueryCellIterator< ScQueryCellIteratorAccess::Direct >;
template class ScQueryCellIterator< ScQueryCellIteratorAccess::SortedCache >;
template class ScCountIfCellIterator< ScQueryCellIteratorAccess::Direct >;
template class ScCountIfCellIterator< ScQueryCellIteratorAccess::SortedCache >;

// gcc for some reason needs these too
template class ScQueryCellIteratorBase< ScQueryCellIteratorAccess::Direct, ScQueryCellIteratorType::Generic >;
template class ScQueryCellIteratorBase< ScQueryCellIteratorAccess::SortedCache, ScQueryCellIteratorType::Generic >;
template class ScQueryCellIteratorBase< ScQueryCellIteratorAccess::Direct, ScQueryCellIteratorType::CountIf >;
template class ScQueryCellIteratorBase< ScQueryCellIteratorAccess::SortedCache, ScQueryCellIteratorType::CountIf >;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
