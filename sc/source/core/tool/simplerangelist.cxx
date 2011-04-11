/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"


//------------------------------------------------------------------------

#include "simplerangelist.hxx"
#include "rangelst.hxx"

using ::std::list;
using ::std::pair;
using ::std::max;

// ============================================================================

ScSimpleRangeList::Range::Range(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2) :
    mnCol1(nCol1), mnRow1(nRow1), mnCol2(nCol2), mnRow2(nRow2) {}

bool ScSimpleRangeList::Range::contains(const Range& r) const
{
    return mnCol1 <= r.mnCol1 && mnRow1 <= r.mnRow1 && r.mnCol2 <= mnCol2 && r.mnRow2 <= mnRow2;
}

// ----------------------------------------------------------------------------

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

void ScSimpleRangeList::insertRow(SCROW nRow, SCTAB nTab)
{
    RangeListRef pRef = findTab(nTab);
    if (!pRef)
        // This should never happen!
        return;

    list<Range>::iterator itr = pRef->begin(), itrEnd = pRef->end();
    for (; itr != itrEnd; ++itr)
    {
        Range& r = *itr;
        if (r.mnRow2 < nRow)
            // insertion point is below the range.
            continue;

        if (nRow <= r.mnRow1)
        {
            // insertion point is above the range.
            ++r.mnRow1;
            ++r.mnRow2;
        }
        else if (nRow <= r.mnRow2)
        {
            // insertion point cuts through the range.
            ++r.mnRow2;
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
