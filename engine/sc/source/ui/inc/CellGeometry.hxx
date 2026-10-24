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

#pragma once

#include <address.hxx>
#include <basegfx/range/b2drange.hxx>

#include <unordered_map>

class ScDocument;

namespace sc
{
/** Where the columns and rows of one sheet are, in twips from the top-left corner of an origin
    cell. Positions left of or above the origin are negative. Hidden columns and rows have size 0.

    The exact geometry uses the widths and heights of the document as they are. The scaled geometry
    rounds each down to whole pixels at the given scale, and to at least one pixel when not hidden,
    the way a window or a tile lays out cells. Its positions stay in twips, so a position times the
    scale is a whole pixel.
 */
class CellGeometry
{
public:
    /// The exact geometry.
    CellGeometry(const ScDocument& rDocument, SCTAB nTab, SCCOL nOriginColumn, SCROW nOriginRow);

    /// The scaled geometry, for fScaleX and fScaleY pixels per twip.
    CellGeometry(const ScDocument& rDocument, SCTAB nTab, SCCOL nOriginColumn, SCROW nOriginRow,
                 double fScaleX, double fScaleY);

    SCCOL getOriginColumn() const { return mnOriginColumn; }
    SCROW getOriginRow() const { return mnOriginRow; }
    bool isScaled() const { return mbScaled; }
    /// Pixels per twip of a scaled geometry.
    double getScaleX() const { return mfScaleX; }
    double getScaleY() const { return mfScaleY; }

    double getColumnStart(SCCOL nColumn) const;
    double getColumnWidth(SCCOL nColumn) const;
    double getRowStart(SCROW nRow) const;
    double getRowHeight(SCROW nRow) const;

    /// The rectangle of the cells from the start cell to the end cell, both included.
    basegfx::B2DRange getRange(SCCOL nStartColumn, SCROW nStartRow, SCCOL nEndColumn,
                               SCROW nEndRow) const;

private:
    double scaleWidth(sal_uInt16 nTwips) const;
    double scaleHeight(sal_uInt16 nTwips) const;
    /// The width of the columns from nStartColumn to nEndColumn, both included.
    double sumColumnWidths(SCCOL nStartColumn, SCCOL nEndColumn) const;
    /// The height of the rows from nStartRow to nEndRow, both included.
    double sumRowHeights(SCROW nStartRow, SCROW nEndRow) const;

    const ScDocument& mrDocument;
    SCTAB mnTab;
    SCCOL mnOriginColumn;
    SCROW mnOriginRow;
    bool mbScaled;
    double mfScaleX;
    double mfScaleY;

    /// Column starts that were asked for already, measured from the origin column.
    mutable std::unordered_map<SCCOL, double> maColumnStarts;
    /// Row starts that were asked for already, measured from the origin row.
    mutable std::unordered_map<SCROW, double> maRowStarts;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
