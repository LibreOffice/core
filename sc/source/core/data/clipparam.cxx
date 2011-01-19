/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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

// INCLUDE ---------------------------------------------------------------

#include "clipparam.hxx"

using ::std::vector;

ScClipParam::ScClipParam() :
    meDirection(Unspecified),
    mbCutMode(false),
    mnSourceDocID(0)
{
}

ScClipParam::ScClipParam(const ScRange& rRange, bool bCutMode) :
    meDirection(Unspecified),
    mbCutMode(bCutMode),
    mnSourceDocID(0)
{
    maRanges.Append(rRange);
}

ScClipParam::ScClipParam(const ScClipParam& r) :
    maRanges(r.maRanges),
    meDirection(r.meDirection),
    mbCutMode(r.mbCutMode),
    mnSourceDocID(r.mnSourceDocID),
    maProtectedChartRangesVector(r.maProtectedChartRangesVector)
{
}

bool ScClipParam::isMultiRange() const
{
    return maRanges.Count() > 1;
}

SCCOL ScClipParam::getPasteColSize()
{
    if (!maRanges.Count())
        return 0;

    switch (meDirection)
    {
        case ScClipParam::Column:
        {
            SCCOL nColSize = 0;
            for (ScRangePtr p = maRanges.First(); p; p = maRanges.Next())
                nColSize += p->aEnd.Col() - p->aStart.Col() + 1;
            return nColSize;
        }
        case ScClipParam::Row:
        {
            // We assume that all ranges have identical column size.
            const ScRange& rRange = *maRanges.First();
            return rRange.aEnd.Col() - rRange.aStart.Col() + 1;
        }
        case ScClipParam::Unspecified:
        default:
            ;
    }
    return 0;
}

SCROW ScClipParam::getPasteRowSize()
{
    if (!maRanges.Count())
        return 0;

    switch (meDirection)
    {
        case ScClipParam::Column:
        {
            // We assume that all ranges have identical row size.
            const ScRange& rRange = *maRanges.First();
            return rRange.aEnd.Row() - rRange.aStart.Row() + 1;
        }
        case ScClipParam::Row:
        {
            SCROW nRowSize = 0;
            for (ScRangePtr p = maRanges.First(); p; p = maRanges.Next())
                nRowSize += p->aEnd.Row() - p->aStart.Row() + 1;
            return nRowSize;
        }
        case ScClipParam::Unspecified:
        default:
            ;
    }
    return 0;
}

ScRange ScClipParam::getWholeRange() const
{
    ScRange aWhole;
    bool bFirst = true;
    ScRangeList aRanges = maRanges;
    for (ScRange* p = aRanges.First(); p; p = aRanges.Next())
    {
        if (bFirst)
        {
            aWhole = *p;
            bFirst = false;
            continue;
        }

        if (aWhole.aStart.Col() > p->aStart.Col())
            aWhole.aStart.SetCol(p->aStart.Col());

        if (aWhole.aStart.Row() > p->aStart.Row())
            aWhole.aStart.SetRow(p->aStart.Row());

        if (aWhole.aEnd.Col() < p->aEnd.Col())
            aWhole.aEnd.SetCol(p->aEnd.Col());

        if (aWhole.aEnd.Row() < p->aEnd.Row())
            aWhole.aEnd.SetRow(p->aEnd.Row());
    }
    return aWhole;
}

void ScClipParam::transpose()
{
    switch (meDirection)
    {
        case Column:
            meDirection = ScClipParam::Row;
        break;
        case Row:
            meDirection = ScClipParam::Column;
        break;
        case Unspecified:
        default:
            ;
    }

    ScRangeList aNewRanges;
    if (maRanges.Count())
    {
        ScRange* p = maRanges.First();
        SCCOL nColOrigin = p->aStart.Col();
        SCROW nRowOrigin = p->aStart.Row();
        for (; p; p = maRanges.Next())
        {
            SCCOL nColDelta = p->aStart.Col() - nColOrigin;
            SCROW nRowDelta = p->aStart.Row() - nRowOrigin;
            SCCOL nCol1 = 0;
            SCCOL nCol2 = static_cast<SCCOL>(p->aEnd.Row() - p->aStart.Row());
            SCROW nRow1 = 0;
            SCROW nRow2 = static_cast<SCROW>(p->aEnd.Col() - p->aStart.Col());
            nCol1 += static_cast<SCCOL>(nRowDelta);
            nCol2 += static_cast<SCCOL>(nRowDelta);
            nRow1 += static_cast<SCROW>(nColDelta);
            nRow2 += static_cast<SCROW>(nColDelta);
            ScRange aNew(nCol1, nRow1, p->aStart.Tab(), nCol2, nRow2, p->aStart.Tab());
            aNewRanges.Append(aNew);
        }
    }
    maRanges = aNewRanges;
}

// ============================================================================

ScClipRangeNameData::ScClipRangeNameData() :
    mbReplace(false)
{
}

ScClipRangeNameData::~ScClipRangeNameData()
{
}

void ScClipRangeNameData::insert(sal_uInt16 nOldIndex, sal_uInt16 nNewIndex)
{
    maRangeMap.insert(
        ScRangeData::IndexMap::value_type(nOldIndex, nNewIndex));
}
