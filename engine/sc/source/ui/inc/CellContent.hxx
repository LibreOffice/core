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

#include "CellMarkKind.hxx"
#include "CellTextFlags.hxx"
#include <address.hxx>
#include <basegfx/range/b2drange.hxx>
#include <editeng/svxenum.hxx>
#include <fillinfo.hxx>
#include <rtl/ustring.hxx>
#include <tools/color.hxx>
#include <vcl/font.hxx>

#include <memory>
#include <optional>
#include <vector>

namespace sc
{
class Sparkline;

/// Neighboring cells of one row with the same fill: a background color or a color of a color
/// scale. Hidden columns inside the run belong to it.
struct CellFill
{
    SCROW mnRow;
    SCCOL mnStartColumn;
    SCCOL mnEndColumn;
    Color maColor;
};

/// The data bar of a cell.
struct CellDataBar
{
    SCCOL mnColumn;
    SCROW mnRow;
    ScDataBarInfo maInfo;
};

/// The icon of an icon set in a cell.
struct CellIconSet
{
    SCCOL mnColumn;
    SCROW mnRow;
    ScIconSetInfo maInfo;
};

/// The part of a shadow that falls onto a cell. The shadow belongs to this cell or to a cell or
/// merged area next to it.
struct CellShadow
{
    SCCOL mnColumn;
    SCROW mnRow;
    ScShadowPart mePart;
    SvxShadowLocation meLocation;
    /// The width of the shadow in twips.
    tools::Long mnSize;
    Color maColor;
};

/// The sparkline of a cell, drawn into the cell or into the merged area that starts there.
struct CellSparkline
{
    SCCOL mnColumn;
    SCROW mnRow;
    SCCOL mnEndColumn;
    SCROW mnEndRow;
    std::shared_ptr<Sparkline> mpSparkline;
};

/// A formula mark, a formula error mark or a drop-down button of a cell, in the corner of the
/// cell or of the merged area that starts there.
struct CellMark
{
    SCCOL mnColumn;
    SCROW mnRow;
    SCCOL mnEndColumn;
    SCROW mnEndRow;
    CellMarkKind meKind;
    /// The filter of a drop-down button is active.
    bool mbFilterActive;
};

/// A merged area, which hides the grid lines inside it.
struct CellMergedArea
{
    SCCOL mnColumn;
    SCROW mnRow;
    SCCOL mnEndColumn;
    SCROW mnEndRow;
};

/// The columns that the text of a cell paints into and where it is cut. It holds no position: the
/// geometry makes the rectangles from it.
struct CellTextArea
{
    /// The first and the last column the text may paint into: the cell or the merged area, and
    /// the empty neighbors that overflowing text reaches into.
    SCCOL mnLeftColumn = 0;
    SCCOL mnRightColumn = 0;
    /// The text is aligned beside the drop-down button of an autofilter, which keeps its room.
    bool mbBesideButton = false;
    /// The text is cut off at the left or the right edge.
    bool mbLeftClip = false;
    bool mbRightClip = false;
    /// The cells the text paints into leave room for the arrow that marks the cut.
    bool mbLeftMarkRoom = false;
    bool mbRightMarkRoom = false;
};

/** The text of a cell, laid out in one run of one font by the plain text layouter, and not
    placed. The run starts at the left end of its baseline. The sizes are in twips.
 */
struct CellText
{
    SCCOL mnColumn;
    SCROW mnRow;
    /// The last cell of the merged area, or the cell itself.
    SCCOL mnEndColumn;
    SCROW mnEndRow;
    CellTextArea maArea;
    SvxCellHorJustify meHorJust;
    SvxCellVerJustify meVerJust;
    CellTextFlags meFlags;
    /// The arrows that mark text cut off at the left or the right.
    bool mbLeftClipMark;
    bool mbRightClipMark;
    /// The margins of the cell. The horizontal ones include the indent.
    double mfLeftIndent;
    double mfRightIndent;
    /// The right margin without the indent, which centered text uses.
    double mfRightMargin;
    double mfTopMargin;
    double mfBottomMargin;

    OUString maText;
    vcl::Font maFont;
    /// Where each character ends, from the start of the run.
    std::vector<double> maAdvances;
    double mfWidth;
    double mfAscent;
    double mfDescent;
    /// The text has lines, emphasis marks, relief or a shadow.
    bool mbDecorated;
    /// The ink of the glyphs from the start of the baseline. Empty for decorated text, whose
    /// lines reach beyond the glyphs.
    basegfx::B2DRange maInkBounds;
    /// The colors of the underline and the overline of decorated text.
    Color maUnderlineColor;
    Color maOverlineColor;
    /// The fill of the cell, when it has one, against which an automatic font color is chosen.
    /// Set when the font colors stay automatic.
    std::optional<Color> moSurface;
};

/** What the cells of a range show, by cell address and without positions. A CellGeometry places
    it. Each list is in the order of the rows, and of the columns within a row. The borders are not
    here: the border array of ScTableInfo (svx::frame::Array) holds the style of each cell edge by
    address already, after the conflicts between neighbors are resolved.
 */
struct CellContent
{
    std::vector<CellFill> maFills;
    std::vector<CellDataBar> maDataBars;
    std::vector<CellIconSet> maIconSets;
    /// The horizontal part of a cell comes before its vertical part.
    std::vector<CellShadow> maShadows;
    std::vector<CellSparkline> maSparklines;
    std::vector<CellMark> maMarks;
    std::vector<CellMergedArea> maMergedAreas;
    /// The text of the cells, in the order it is laid out.
    std::vector<CellText> maTexts;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
