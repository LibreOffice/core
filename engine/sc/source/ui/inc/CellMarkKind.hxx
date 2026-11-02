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

#include <sal/types.h>

namespace sc
{
/// The kinds of small marks that Calc draws on a cell.
enum class CellMarkKind : sal_uInt8
{
    /// The cell holds a formula. Drawn in the bottom-left corner.
    Formula,
    /// The formula of the cell gives an error. Drawn in the top-left corner.
    FormulaError,
    /// Text of the cell is cut off at the left edge.
    ClipLeft,
    /// Text of the cell is cut off at the right edge.
    ClipRight,
    /// The cell holds the drop-down button of an autofilter.
    AutoFilter,
    /// The cell holds the drop-down button of a pivot table field.
    PivotButton,
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
