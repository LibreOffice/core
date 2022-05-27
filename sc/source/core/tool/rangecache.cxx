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
#include <unotools/collatorwrapper.hxx>

static bool needsDescending(const ScQueryParam& param)
{
    assert(param.GetEntry(0).bDoQuery && !param.GetEntry(1).bDoQuery
           && param.GetEntry(0).GetQueryItems().size() == 1);
    assert(param.GetEntry(0).eOp == SC_GREATER || param.GetEntry(0).eOp == SC_GREATER_EQUAL
           || param.GetEntry(0).eOp == SC_LESS || param.GetEntry(0).eOp == SC_LESS_EQUAL
           || param.GetEntry(0).eOp == SC_EQUAL);
    // We want all matching values to start in the sort order,
    // since the data is searched from start until the last matching one.
    return param.GetEntry(0).eOp == SC_GREATER || param.GetEntry(0).eOp == SC_GREATER_EQUAL;
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
                                       const ScQueryParam& param, ScSortedRangeCacheMap& cacheMap,
                                       ScInterpreterContext* context)
    : maRange(rRange)
    , mpDoc(pDoc)
    , mCacheMap(cacheMap)
    , mDescending(needsDescending(param))
    , mValues(toValueType(param))
{
    assert(maRange.aStart.Col() == maRange.aEnd.Col());
    assert(maRange.aStart.Tab() == maRange.aEnd.Tab());
    SCTAB nTab = maRange.aStart.Tab();
    SCTAB nCol = maRange.aStart.Col();
    assert(param.GetEntry(0).bDoQuery && !param.GetEntry(1).bDoQuery
           && param.GetEntry(0).GetQueryItems().size() == 1);
    const ScQueryEntry& entry = param.GetEntry(0);
    const ScQueryEntry::Item& item = entry.GetQueryItem();

    SCROW startRow = maRange.aStart.Row();
    SCROW endRow = maRange.aEnd.Row();
    SCCOL startCol = maRange.aStart.Col();
    SCCOL endCol = maRange.aEnd.Col();
    if (!item.mbMatchEmpty)
        if (!pDoc->ShrinkToDataArea(nTab, startCol, startRow, endCol, endRow))
            return;

    if (mValues == ValueType::Values)
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
            if (ScQueryEvaluator::isQueryByValue(entry, item, cell))
                rowData.push_back(RowData{ nRow, cell.getValue() });
        }
        std::stable_sort(rowData.begin(), rowData.end(),
                         [](const RowData& d1, const RowData& d2) { return d1.value < d2.value; });
        if (mDescending)
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
            if (ScQueryEvaluator::isQueryByString(entry, item, cell))
            {
                const svl::SharedString* sharedString = nullptr;
                OUString string = evaluator.getCellString(cell, nRow, entry, &sharedString);
                if (sharedString)
                    string = sharedString->getString();
                rowData.push_back(RowData{ nRow, string });
            }
        }
        CollatorWrapper& collator
            = ScGlobal::GetCollator(mValues == ValueType::StringsCaseSensitive);
        std::stable_sort(rowData.begin(), rowData.end(),
                         [&collator](const RowData& d1, const RowData& d2) {
                             return collator.compareString(d1.string, d2.string) < 0;
                         });
        if (mDescending)
            for (auto it = rowData.rbegin(); it != rowData.rend(); ++it)
                mSortedRows.emplace_back(it->row);
        else
            for (const RowData& d : rowData)
                mSortedRows.emplace_back(d.row);
    }
    mRowToIndex.resize(maRange.aEnd.Row() - maRange.aStart.Row() + 1, mSortedRows.max_size());
    for (size_t i = 0; i < mSortedRows.size(); ++i)
        mRowToIndex[mSortedRows[i] - maRange.aStart.Row()] = i;
}

void ScSortedRangeCache::Notify(const SfxHint& rHint)
{
    if (!mpDoc->IsInDtorClear())
    {
        const ScHint* p = dynamic_cast<const ScHint*>(&rHint);
        if ((p && (p->GetId() == SfxHintId::ScDataChanged))
            || dynamic_cast<const ScAreaChangedHint*>(&rHint))
        {
            mpDoc->RemoveSortedRangeCache(*this);
            delete this;
        }
    }
}

ScSortedRangeCache::HashKey ScSortedRangeCache::makeHashKey(const ScRange& range,
                                                            const ScQueryParam& param)
{
    return { range, needsDescending(param), toValueType(param) };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
