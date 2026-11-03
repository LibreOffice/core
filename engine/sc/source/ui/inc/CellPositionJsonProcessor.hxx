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
#include <drawinglayer/processor2d/Primitive2dJsonProcessor.hxx>

#include <utility>
#include <vector>

class ScDocument;

namespace sc
{
/** The columns or the rows of a sheet along one axis, which turn a position in twips from the
    start of an origin cell into the cell it falls in and the offset into that cell. A position on
    the edge between two cells belongs to the second, and a cell of size 0 has no positions of its
    own. A position before the sheet belongs to its first cell, with a negative offset, and one
    after it to its last cell, past its end. The sizes are those of the exact geometry: hidden
    cells have size 0.
 */
class CellAxis
{
public:
    CellAxis(const ScDocument& rDocument, SCTAB nTab, bool bRows, SCCOLROW nOrigin);

    /// The cell and the offset into it.
    std::pair<SCCOLROW, double> map(double fPosition);

private:
    double sizeOf(SCCOLROW nIndex) const;

    const ScDocument& mrDocument;
    SCTAB mnTab;
    bool mbRows;
    SCCOLROW mnOrigin;
    SCCOLROW mnLast;
    /// Where the sheet ends and starts, from the start of the origin cell.
    double mfEnd;
    double mfStart;
    /// The starts of the cells from the origin on, and of the cells before it going back, as far
    /// as positions were asked for.
    std::vector<double> maForward;
    std::vector<double> maBackward;
};

/** Writes the JSON of cell content with its positions relative to the cells of the sheet: an x as
    [column, offset] and a y as [row, offset], in twips from the start of that column or row.
    Positions are those of the content as the builder places it with the exact geometry, from the
    start of the origin cell. The rules follow the browser, which places the content:

    - Paths take the points as "column:offset,row:offset", absolute, with M, L, C and Z.
    - Ranges take each edge as a position.
    - A text portion has its origin as a position; its glyphs keep their size.
    - An image that is not turned has its corners as positions, so it stretches with the cells.
    - A transformation that only scales and moves passes its children on as positions. One that
      turns or shears has its origin as a position, and its children stay as they are.
    - A value outside the sheet, such as the edge of a clip that does not clip, is a position in
      the first or the last cell, so the browser needs no other geometry.
 */
class CellPositionJsonProcessor final : public drawinglayer::Primitive2dJsonProcessor
{
public:
    CellPositionJsonProcessor(tools::JsonWriter& rWriter, const ScDocument& rDocument, SCTAB nTab,
                              SCCOL nOriginColumn, SCROW nOriginRow);

    /// Write a range of a Calc primitive or a block as an object of four positions: x, y, right
    /// and bottom. Calc primitives are written outside any transformation.
    void writeCellRange(tools::JsonWriter& rWriter, std::string_view sName,
                        const basegfx::B2DRange& rRange);
    /// The same, as fields of the open node.
    void writeCellRangeFields(tools::JsonWriter& rWriter, const basegfx::B2DRange& rRange);

protected:
    void putPosition(std::string_view sName, double fValue, bool bVertical) override;
    void putTextOrigin(double fX, double fY) override;
    void putRange(std::string_view sName, const basegfx::B2DRange& rRange, double fScale) override;
    void putPath(std::string_view sName, const basegfx::B2DPolyPolygon& rPolyPolygon) override;
    void putMatrix(std::string_view sName, const basegfx::B2DHomMatrix& rMatrix,
                   MatrixKind eKind) override;
    void beginChildren(const basegfx::B2DHomMatrix& rTransform) override;
    void endChildren() override;

private:
    /// How a position written inside the current transformation maps to the content: content
    /// x = a * x + e, content y = d * y + f. Without mapping, positions are written as numbers.
    struct Axes
    {
        double a = 1.0;
        double d = 1.0;
        double e = 0.0;
        double f = 0.0;
        bool bMapping = true;
    };

    /// The position as JSON: "[cell, offset]", or the number inside a turned transformation.
    OString position(double fValue, bool bVertical);
    /// The position in a path: "cell:offset", or the number.
    OString pathPosition(double fValue, bool bVertical);
    void writePositionValue(tools::JsonWriter& rWriter, double fValue, bool bVertical);

    CellAxis maColumns;
    CellAxis maRows;
    std::vector<Axes> maAxes;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
