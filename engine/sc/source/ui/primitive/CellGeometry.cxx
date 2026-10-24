/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <CellGeometry.hxx>

#include <document.hxx>

#include <algorithm>
#include <cmath>

namespace sc
{
CellGeometry::CellGeometry(const ScDocument& rDocument, SCTAB nTab, SCCOL nOriginColumn,
                           SCROW nOriginRow)
    : CellGeometry(rDocument, nTab, nOriginColumn, nOriginRow, 0.0, 0.0)
{
}

CellGeometry::CellGeometry(const ScDocument& rDocument, SCTAB nTab, SCCOL nOriginColumn,
                           SCROW nOriginRow, double fScaleX, double fScaleY)
    : mrDocument(rDocument)
    , mnTab(nTab)
    , mnOriginColumn(nOriginColumn)
    , mnOriginRow(nOriginRow)
    , mbScaled(fScaleX > 0.0 && fScaleY > 0.0)
    , mfScaleX(fScaleX)
    , mfScaleY(fScaleY)
{
}

double CellGeometry::scaleWidth(sal_uInt16 nTwips) const
{
    if (!mbScaled || nTwips == 0)
        return nTwips;
    // A column that is not hidden takes at least one pixel.
    const double fPixels = std::max(1.0, std::trunc(nTwips * mfScaleX));
    return fPixels / mfScaleX;
}

double CellGeometry::scaleHeight(sal_uInt16 nTwips) const
{
    if (!mbScaled || nTwips == 0)
        return nTwips;
    // A row that is not hidden takes at least one pixel.
    const double fPixels = std::max(1.0, std::trunc(nTwips * mfScaleY));
    return fPixels / mfScaleY;
}

double CellGeometry::getColumnWidth(SCCOL nColumn) const
{
    if (!mrDocument.ValidCol(nColumn))
        return 0.0;
    return scaleWidth(mrDocument.GetColWidth(nColumn, mnTab, /*bHiddenAsZero*/ true));
}

double CellGeometry::sumColumnWidths(SCCOL nStartColumn, SCCOL nEndColumn) const
{
    if (nEndColumn < nStartColumn)
        return 0.0;

    if (!mbScaled)
        return mrDocument.GetColWidth(nStartColumn, nEndColumn, mnTab);

    // Each column is rounded on its own.
    double fWidth = 0.0;
    for (SCCOL nColumn = nStartColumn; nColumn <= nEndColumn; ++nColumn)
        fWidth += getColumnWidth(nColumn);
    return fWidth;
}

double CellGeometry::getColumnStart(SCCOL nColumn) const
{
    nColumn = std::clamp<SCCOL>(nColumn, 0, mrDocument.MaxCol() + 1);
    if (nColumn == mnOriginColumn)
        return 0.0;

    auto aIterator = maColumnStarts.find(nColumn);
    if (aIterator != maColumnStarts.end())
        return aIterator->second;

    double fStart;
    if (nColumn > mnOriginColumn)
        fStart = sumColumnWidths(mnOriginColumn, std::min<SCCOL>(nColumn - 1, mrDocument.MaxCol()));
    else
        fStart = -sumColumnWidths(nColumn, mnOriginColumn - 1);

    maColumnStarts.emplace(nColumn, fStart);
    return fStart;
}

double CellGeometry::getRowHeight(SCROW nRow) const
{
    if (!mrDocument.ValidRow(nRow))
        return 0.0;
    return scaleHeight(mrDocument.GetRowHeight(nRow, mnTab, /*bHiddenAsZero*/ true));
}

double CellGeometry::sumRowHeights(SCROW nStartRow, SCROW nEndRow) const
{
    if (nEndRow < nStartRow)
        return 0.0;

    if (!mbScaled)
        return mrDocument.GetRowHeight(nStartRow, nEndRow, mnTab, /*bHiddenAsZero*/ true);

    // Each row is rounded on its own, so rows are added up per run of equal height.
    double fHeight = 0.0;
    SCROW nRow = nStartRow;
    while (nRow <= nEndRow)
    {
        SCROW nSpanStart = nRow;
        SCROW nSpanEnd = nRow;
        const sal_uInt16 nTwips
            = mrDocument.GetRowHeight(nRow, mnTab, &nSpanStart, &nSpanEnd, /*bHiddenAsZero*/ true);
        nSpanEnd = std::min(nSpanEnd, nEndRow);
        fHeight += scaleHeight(nTwips) * (nSpanEnd - nRow + 1);
        nRow = nSpanEnd + 1;
    }
    return fHeight;
}

double CellGeometry::getRowStart(SCROW nRow) const
{
    nRow = std::clamp<SCROW>(nRow, 0, mrDocument.MaxRow() + 1);
    if (nRow == mnOriginRow)
        return 0.0;

    auto aIterator = maRowStarts.find(nRow);
    if (aIterator != maRowStarts.end())
        return aIterator->second;

    double fStart;
    if (nRow > mnOriginRow)
        fStart = sumRowHeights(mnOriginRow, std::min(nRow - 1, mrDocument.MaxRow()));
    else
        fStart = -sumRowHeights(nRow, mnOriginRow - 1);

    maRowStarts.emplace(nRow, fStart);
    return fStart;
}

basegfx::B2DRange CellGeometry::getRange(SCCOL nStartColumn, SCROW nStartRow, SCCOL nEndColumn,
                                         SCROW nEndRow) const
{
    return basegfx::B2DRange(getColumnStart(nStartColumn), getRowStart(nStartRow),
                             getColumnStart(nEndColumn + 1), getRowStart(nEndRow + 1));
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
