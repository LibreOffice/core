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

#include <rangecache.hxx>
#include <cellvalue.hxx>
#include <document.hxx>
#include <brdcst.hxx>
#include <queryevaluator.hxx>
#include <queryparam.hxx>

#include <sal/log.hxx>
#include <svl/numformat.hxx>
#include <unotools/collatorwrapper.hxx>

static bool needsDescending(ScQueryOp op)
{
    assert(op == SC_GREATER || op == SC_GREATER_EQUAL || op == SC_LESS || op == SC_LESS_EQUAL
           || op == SC_EQUAL);
    // We want all matching values to start in the sort order,
    // since the data is searched from start until the last matching one.
    return op == SC_GREATER || op == SC_GREATER_EQUAL;
}

static ScSortedRangeCache::ValueType toValueType(const ScQueryParam& param)
{
    assert(param.GetEntry(0).bDoQuery && !param.GetEntry(1).bDoQuery
           && param.GetEntry(0).GetQueryItems().size() == 1);
    assert(param.GetEntry(0).GetQueryItem().meType == ScQueryEntry::ByString
           || param.GetEntry(0).GetQueryItem().meType == ScQueryEntry::ByValue);
    if (param.GetEntry(0).GetQueryItem().meType == ScQueryEntry::ByValue)
        return ScSortedRangeCache::ValueType::Values;
    return param.bCaseSens ? ScSortedRangeCache::ValueType::StringsCaseSensitive
                           : ScSortedRangeCache::ValueType::StringsCaseInsensitive;
}

ScSortedRangeCache::ScSortedRangeCache(ScDocument* pDoc, const ScRange& rRange,
                                       const ScQueryParam& param, ScInterpreterContext* context,
                                       bool invalid, sal_uInt8 nSortedBinarySearch)
    : maRange(rRange)
    , mpDoc(pDoc)
    , mValid(false)
    , mValueType(toValueType(param))
{
    assert(maRange.aStart.Col() == maRange.aEnd.Col());
    assert(maRange.aStart.Tab() == maRange.aEnd.Tab());
    SCTAB nTab = maRange.aStart.Tab();
    SCTAB nCol = maRange.aStart.Col();
    assert(param.GetEntry(0).bDoQuery && !param.GetEntry(1).bDoQuery
           && param.GetEntry(0).GetQueryItems().size() == 1);
    const ScQueryEntry& entry = param.GetEntry(0);
    const ScQueryEntry::Item& item = entry.GetQueryItem();
    mQueryOp = entry.eOp;
    mQueryType = item.meType;

    if (invalid)
        return; // leave empty

    SCROW startRow = maRange.aStart.Row();
    SCROW endRow = maRange.aEnd.Row();
    SCCOL startCol = maRange.aStart.Col();
    SCCOL endCol = maRange.aEnd.Col();
    if (!item.mbMatchEmpty)
        if (!pDoc->ShrinkToDataArea(nTab, startCol, startRow, endCol, endRow))
            return; // no data cells, no need for a cache

    if (mValueType == ValueType::Values)
    {
        struct RowData
        {
            SCROW row;
            double value;
        };
        std::vector<RowData> rowData;
        for (SCROW nRow = startRow; nRow <= endRow; ++nRow)
        {
            ScRefCellValue cell(pDoc->GetRefCellValue(ScAddress(nCol, nRow, nTab)));
            if (ScQueryEvaluator::isQueryByValue(mQueryOp, mQueryType, cell))
                rowData.push_back(RowData{ nRow, cell.getValue() });
            else if (ScQueryEvaluator::isQueryByString(mQueryOp, mQueryType, cell))
            {
                // Make sure that other possibilities in the generic handling
                // in ScQueryEvaluator::processEntry() do not alter the results.
                // (ByTextColor/ByBackgroundColor are blocked by CanBeUsedForSorterCache(),
                // but isQueryByString() is possible if the cell content is a string.
                // And including strings here would be tricky, as the string comparison
                // may possibly(?) be different than a numeric one. So check if the string
                // may possibly match a number, by converting it to one. If it can't match,
                // then it's fine to ignore it (and it can happen e.g. if the query uses
                // the whole column which includes a textual header). But if it can possibly
                // match, then bail out and leave it to the unoptimized case.
                // TODO Maybe it would actually work to use the numeric value obtained here?
                if (!ScQueryEvaluator::isMatchWholeCell(*pDoc, mQueryOp))
                    return; // substring matching cannot be sorted
                sal_uInt32 format = 0;
                double value;
                if (context->NFIsNumberFormat(cell.getString(pDoc), format, value))
                    return;
            }
        }

        if (nSortedBinarySearch == 0x00) //nBinarySearchDisabled = 0x00
        {
            std::stable_sort(
                rowData.begin(), rowData.end(),
                [](const RowData& d1, const RowData& d2) { return d1.value < d2.value; });
        }
        else if (nSortedBinarySearch == 0x01) //nSearchbAscd
        {
            // expected it is already sorted properly in Ascd mode.
        }
        else /*(nSortedBinarySearch == 0x02) nSearchbDesc*/
        {
            // expected it is already sorted properly in Desc mode, just need to reverse.
            std::reverse(rowData.begin(), rowData.end());
        }

        if (needsDescending(entry.eOp))
            for (auto it = rowData.rbegin(); it != rowData.rend(); ++it)
                mSortedRows.emplace_back(it->row);
        else
            for (const RowData& d : rowData)
                mSortedRows.emplace_back(d.row);
    }
    else
    {
        struct RowData
        {
            SCROW row;
            OUString string;
        };
        std::vector<RowData> rowData;
        // Try to reuse as much ScQueryEvaluator code as possible, this should
        // basically do the same comparisons.
        assert(pDoc->FetchTable(nTab) != nullptr);
        ScQueryEvaluator evaluator(*pDoc, *pDoc->FetchTable(nTab), param, context);
        for (SCROW nRow = startRow; nRow <= endRow; ++nRow)
        {
            ScRefCellValue cell(pDoc->GetRefCellValue(ScAddress(nCol, nRow, nTab)));
            // This should be used only with ScQueryEntry::ByString, and that
            // means that ScQueryEvaluator::isQueryByString() should be the only
            // possibility in the generic handling in ScQueryEvaluator::processEntry()
            // (ByTextColor/ByBackgroundColor are blocked by CanBeUsedForSorterCache(),
            // and isQueryByValue() is blocked by ScQueryEntry::ByString).
            assert(mQueryType == ScQueryEntry::ByString);
            assert(!ScQueryEvaluator::isQueryByValue(mQueryOp, mQueryType, cell));
            if (ScQueryEvaluator::isQueryByString(mQueryOp, mQueryType, cell))
            {
                const svl::SharedString* sharedString = nullptr;
                OUString string = evaluator.getCellString(cell, nRow, nCol, &sharedString);
                if (sharedString)
                    string = sharedString->getString();
                rowData.push_back(RowData{ nRow, string });
            }
        }
        CollatorWrapper& collator
            = ScGlobal::GetCollator(mValueType == ValueType::StringsCaseSensitive);

        if (nSortedBinarySearch == 0x00) //nBinarySearchDisabled = 0x00
        {
            std::stable_sort(rowData.begin(), rowData.end(),
                             [&collator](const RowData& d1, const RowData& d2) {
                                 return collator.compareString(d1.string, d2.string) < 0;
                             });
        }
        else if (nSortedBinarySearch == 0x01) //nSearchbAscd
        {
            // expected it is already sorted properly in Asc mode.
        }
        else /*(nSortedBinarySearch == 0x02) nSearchbDesc*/
        {
            // expected it is already sorted properly in Desc mode, just need to reverse.
            std::reverse(rowData.begin(), rowData.end());
        }

        if (needsDescending(entry.eOp))
            for (auto it = rowData.rbegin(); it != rowData.rend(); ++it)
                mSortedRows.emplace_back(it->row);
        else
            for (const RowData& d : rowData)
                mSortedRows.emplace_back(d.row);
    }

    mRowToIndex.resize(maRange.aEnd.Row() - maRange.aStart.Row() + 1, mSortedRows.max_size());
    for (size_t i = 0; i < mSortedRows.size(); ++i)
        mRowToIndex[mSortedRows[i] - maRange.aStart.Row()] = i;
    mValid = true;
}

void ScSortedRangeCache::Notify(const SfxHint& rHint)
{
    if (!mpDoc->IsInDtorClear())
    {
        if (rHint.GetId() == SfxHintId::ScDataChanged || rHint.GetId() == SfxHintId::ScAreaChanged)
        {
            mpDoc->RemoveSortedRangeCache(*this);
            // this ScSortedRangeCache is deleted by RemoveSortedRangeCache
        }
    }
}

ScSortedRangeCache::HashKey ScSortedRangeCache::makeHashKey(const ScRange& range,
                                                            const ScQueryParam& param)
{
    assert(param.GetEntry(0).bDoQuery && !param.GetEntry(1).bDoQuery
           && param.GetEntry(0).GetQueryItems().size() == 1);
    const ScQueryEntry& entry = param.GetEntry(0);
    const ScQueryEntry::Item& item = entry.GetQueryItem();
    return { range, toValueType(param), entry.eOp, item.meType };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
