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

#include <sal/log.hxx>

ScSortedRangeCache::ScSortedRangeCache(ScDocument* pDoc, const ScRange& rRange, bool bDescending,
                                       ScSortedRangeCacheMap& cacheMap)
    : maRange(rRange)
    , mpDoc(pDoc)
    , mCacheMap(cacheMap)
    , mDescending(bDescending)
{
    assert(maRange.aStart.Col() == maRange.aEnd.Col());
    assert(maRange.aStart.Tab() == maRange.aEnd.Tab());
    SCTAB nTab = maRange.aStart.Tab();
    SCTAB nCol = maRange.aStart.Col();
    mSortedRows.reserve(maRange.aEnd.Row() - maRange.aStart.Row() + 1);
    for (SCROW nRow = maRange.aStart.Row(); nRow <= maRange.aEnd.Row(); ++nRow)
    {
        ScRefCellValue cell(pDoc->GetRefCellValue(ScAddress(nCol, nRow, nTab)));
        if (!cell.hasError() && cell.hasNumeric())
            mSortedRows.push_back(nRow);
    }
    std::stable_sort(
        mSortedRows.begin(), mSortedRows.end(), [pDoc, nCol, nTab](SCROW nRow1, SCROW nRow2) {
            return pDoc->GetValue(nCol, nRow1, nTab) < pDoc->GetValue(nCol, nRow2, nTab);
        });
    if (mDescending)
        std::reverse(mSortedRows.begin(), mSortedRows.end());
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
