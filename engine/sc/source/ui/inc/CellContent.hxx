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
#include <address.hxx>
#include <editeng/svxenum.hxx>
#include <fillinfo.hxx>
#include <tools/color.hxx>

#include <memory>
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
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
