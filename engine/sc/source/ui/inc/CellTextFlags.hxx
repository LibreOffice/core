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

#include <o3tl/typed_flags_set.hxx>
#include <sal/types.h>

namespace sc
{
/// How the text of a cell was laid out and cut.
enum class CellTextFlags : sal_uInt16
{
    NONE = 0x0000,
    /// The text was laid out by the EditEngine: rich text, wrapped or rotated text and the like.
    EditEngine = 0x0001,
    /// A number that does not fit is shown as "###".
    HashFill = 0x0002,
    /// The font was made smaller so the text fits the cell.
    Shrink = 0x0004,
    /// The text is repeated to fill the cell.
    Repeat = 0x0008,
    /// Text is cut off at the left edge of the clip range.
    ClippedLeft = 0x0010,
    /// Text is cut off at the right edge of the clip range.
    ClippedRight = 0x0020,
    /// Text is cut off at the bottom of the cell.
    ClippedBottom = 0x0040,
    /// The text wraps into lines at the width of the cell.
    Wrap = 0x0080,
};
}

namespace o3tl
{
template <> struct typed_flags<sc::CellTextFlags> : is_typed_flags<sc::CellTextFlags, 0x00ff>
{
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
