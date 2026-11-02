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
#include <fillinfo.hxx>
#include <tools/color.hxx>

#include <vector>

namespace sc
{
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

/** What the cells of a range show, by cell address and without positions. A CellGeometry places
    it. Each list is in the order of the rows, and of the columns within a row.
 */
struct CellContent
{
    std::vector<CellFill> maFills;
    std::vector<CellDataBar> maDataBars;
    std::vector<CellIconSet> maIconSets;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
