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

#include "simplerangelist.hxx"
#include "rangelst.hxx"

using ::std::list;
using ::std::pair;
using ::std::max;

ScSimpleRangeList::Range::Range(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2) :
    mnCol1(nCol1), mnRow1(nRow1), mnCol2(nCol2), mnRow2(nRow2) {}

ScSimpleRangeList::ScSimpleRangeList()
{
}

namespace {

bool maybeJoin(ScSimpleRangeList::Range& rOld, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2)
{
    if (rOld.mnRow1 == nRow1 && rOld.mnRow2 == nRow2)
    {
        // Check their column spans to see if they overlap.
        if (rOld.mnCol1 == nCol1)
        {
            // Their share the start column position.
            rOld.mnCol2 = max(rOld.mnCol2, nCol2);
            return true;
        }
        else if (rOld.mnCol1 < nCol1)
        {
            // Old range sits on the left.
            if (nCol1 - rOld.mnCol2 <= 1)
            {
                rOld.mnCol2 = max(rOld.mnCol2, nCol2);
                return true;
            }
        }
        else if (nCol1 < rOld.mnCol1)
        {
            // New range sits on the left.
            if (nCol1 - rOld.mnCol2 <= 1)
            {
                rOld.mnCol1 = nCol1;
                rOld.mnCol2 = max(rOld.mnCol2, nCol2);
                return true;
            }
        }
    }

    if (rOld.mnCol1 == nCol1 && rOld.mnCol2 == nCol2)
    {
        if (rOld.mnRow1 == nRow1)
        {
            // Their share the start row position.
            rOld.mnRow2 = max(rOld.mnRow2, nRow2);
            return true;
        }
        else if (rOld.mnRow1 < nRow1)
        {
            // Old range sits above.
            if (nRow1 - rOld.mnRow2 <= 1)
            {
                rOld.mnRow2 = max(rOld.mnRow2, nRow2);
                return true;
            }
        }
        else if (nRow1 < rOld.mnRow1)
        {
            // New range sits above.
            if (nRow1 - rOld.mnRow2 <= 1)
            {
                rOld.mnRow1 = nRow1;
                rOld.mnRow2 = max(rOld.mnRow2, nRow2);
                return true;
            }
        }
    }

    return false;
}

}

void ScSimpleRangeList::addRange(const ScRange& rRange)
{
    SCCOL nCol1 = rRange.aStart.Col();
    SCROW nRow1 = rRange.aStart.Row();
    SCTAB nTab1 = rRange.aStart.Tab();
    SCCOL nCol2 = rRange.aEnd.Col();
    SCROW nRow2 = rRange.aEnd.Row();
    SCTAB nTab2 = rRange.aEnd.Tab();

    for (SCTAB nTab = nTab1; nTab <= nTab2; ++nTab)
    {
        RangeListRef pRef = findTab(nTab);
        if (!pRef)
            // This should never happen!
            return;

        if (pRef->empty() || !maybeJoin(pRef->back(), nCol1, nRow1, nCol2, nRow2))
            // Not joinable.  Append it to the list.
            pRef->push_back(Range(nCol1, nRow1, nCol2, nRow2));
    }
}

void ScSimpleRangeList::insertCol(SCCOL nCol, SCTAB nTab)
{
    RangeListRef pRef = findTab(nTab);
    if (!pRef)
        // This should never happen!
        return;

    list<Range>::iterator itr = pRef->begin(), itrEnd = pRef->end();
    for (; itr != itrEnd; ++itr)
    {
        Range& r = *itr;
        if (r.mnCol2 < nCol)
            // insertion point to the right of the range.
            continue;

        if (nCol <= r.mnCol1)
        {
            // insertion point to the left of the range.
            ++r.mnCol1;
            ++r.mnCol2;
        }
        else if (nCol <= r.mnCol2)
        {
            // insertion point cuts through the range.
            ++r.mnCol2;
        }
    }
}

void ScSimpleRangeList::getRangeList(list<ScRange>& rList) const
{
    list<ScRange> aList;
    for (TabType::const_iterator itrTab = maTabs.begin(), itrTabEnd = maTabs.end(); itrTab != itrTabEnd; ++itrTab)
    {
        SCTAB nTab = itrTab->first;
        const RangeListRef& pRanges = itrTab->second;
        list<Range>::const_iterator itr = pRanges->begin(), itrEnd = pRanges->end();
        for (; itr != itrEnd; ++itr)
        {
            const Range& r = *itr;
            aList.push_back(ScRange(r.mnCol1, r.mnRow1, nTab, r.mnCol2, r.mnRow2, nTab));
        }
    }
    rList.swap(aList);
}

void ScSimpleRangeList::clear()
{
    maTabs.clear();
}

ScSimpleRangeList::RangeListRef ScSimpleRangeList::findTab(SCTAB nTab)
{
    TabType::iterator itr = maTabs.find(nTab);
    if (itr == maTabs.end())
    {
        RangeListRef p(new list<Range>);
        pair<TabType::iterator, bool> r = maTabs.insert(TabType::value_type(nTab, p));
        if (!r.second)
            return RangeListRef();
        itr = r.first;
    }

    return itr->second;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
