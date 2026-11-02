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
#include <drawinglayer/primitive2d/Primitive2DContainer.hxx>
#include <tools/color.hxx>

#include <memory>
#include <optional>
#include <vector>

class ScDocument;
class ScFieldEditEngine;
struct ScTableInfo;

namespace sc
{
class CellGeometry;

/// What a view shows in its cells besides the content.
struct CellPrimitiveSettings
{
    /// Formulas are shown in place of their results.
    bool mbShowFormulas = false;
    /// Zero values are shown. Without it a cell with a zero value is empty.
    bool mbShowNullValues = true;
    /// Cells with a formula get the formula mark.
    bool mbFormulaMarks = false;
    /// Cells whose formula gives an error get the error mark.
    bool mbFormulaErrorMarks = true;
    /// Text that is cut off gets the clip arrows.
    bool mbClipMarks = true;
    /// Autofilter and pivot table cells get their drop-down buttons.
    bool mbButtons = true;
    /// Text of cells outside the range that overflows into it, or whose merged area reaches into
    /// it, is built too.
    bool mbTextFromOutside = false;
    /// The size of one device pixel in twips. Marks and buttons keep a size in pixels.
    double mfPixelSize = 15.0;
    /// The background of the document, against which an automatic font color is chosen.
    Color maDocumentColor = COL_WHITE;
    /// The automatic font color.
    Color maFontColor = COL_BLACK;
    /// The color of the arrows on text that is cut off.
    Color maClipMarkColor = COL_LIGHTRED;
    /// A cell that is not drawn, because a view edits it in place.
    std::optional<ScAddress> moSkipCell;
};

/** The content of a range of cells as primitives, in layers drawn in this order. Positions are in
    twips, in the coordinates of the geometry the content was built with.
 */
struct CellPrimitiveLayers
{
    /// Cell fills, color scales, data bars, icon sets.
    drawinglayer::primitive2d::Primitive2DContainer maBackground;
    /// Cell shadows and borders.
    drawinglayer::primitive2d::Primitive2DContainer maBorders;
    /// The text of the cells.
    drawinglayer::primitive2d::Primitive2DContainer maText;
    /// Sparklines.
    drawinglayer::primitive2d::Primitive2DContainer maExtras;
    /// Formula marks, clip arrows, drop-down buttons.
    drawinglayer::primitive2d::Primitive2DContainer maMarks;
    /// The areas where grid lines are hidden, as one GridCoverPrimitive2D.
    drawinglayer::primitive2d::Primitive2DContainer maGridCover;
    /// The area the layers paint into, larger than the range where text or a merge reaches out.
    basegfx::B2DRange maInkRange;

    bool isEmpty() const;
};

/** The content of a range of cells as primitives, cut into rows. The shadows and borders cover the
    whole range, because a border between two rows belongs to both. Every other layer is cut by the
    row of its cell, and text of a merged area or overflowing text by the row of its first cell.
 */
struct CellPrimitiveRows
{
    /// The shadows and borders of the whole range, in maBorders. The other layers are empty.
    CellPrimitiveLayers maRange;
    /// The other layers of each row, from the first row of the range on. Their maBorders is empty.
    std::vector<CellPrimitiveLayers> maRows;
};

/** Builds the content of a range of cells of one sheet as primitives, without drawing anything.

    The geometry places the cells, so one builder serves both the exact layout of the document and
    the rounded layout of a window. Text is measured at document resolution, so line breaks and
    overflow stay the same at every zoom.
 */
class CellPrimitiveBuilder
{
public:
    CellPrimitiveBuilder(ScDocument& rDocument, SCTAB nTab, const CellGeometry& rGeometry,
                         CellPrimitiveSettings aSettings);
    ~CellPrimitiveBuilder();

    CellPrimitiveLayers build(SCCOL nStartColumn, SCROW nStartRow, SCCOL nEndColumn, SCROW nEndRow);
    /// The same content, read from rInfo, which the caller filled for this range with
    /// ScDocument::FillInfo at the scale of the geometry, so the range is not read twice. The
    /// builder sets the column and row sizes of the border array of rInfo. A right-to-left sheet
    /// is read again, because its border array would have to be mirrored.
    CellPrimitiveLayers build(SCCOL nStartColumn, SCROW nStartRow, SCCOL nEndColumn, SCROW nEndRow,
                              ScTableInfo& rInfo);
    /// The same content, cut into rows. Text from cells outside the range lands only in its rows.
    CellPrimitiveRows buildRows(SCCOL nStartColumn, SCROW nStartRow, SCCOL nEndColumn,
                                SCROW nEndRow);
    /// The borders of the whole range, and the other layers of the rows from nFirstContentRow to
    /// nLastContentRow only, both included. The other rows of the result stay empty.
    CellPrimitiveRows buildRows(SCCOL nStartColumn, SCROW nStartRow, SCCOL nEndColumn,
                                SCROW nEndRow, SCROW nFirstContentRow, SCROW nLastContentRow);

private:
    class Impl;
    std::unique_ptr<Impl> mpImpl;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
