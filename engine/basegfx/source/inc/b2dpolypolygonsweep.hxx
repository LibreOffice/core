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

#include <basegfx/polygon/b2dpolypolygon.hxx>

namespace basegfx::utils
{
// which areas of a polypolygon are inside: EvenOdd when a ray from the point crosses the outline
// an odd number of times, Nonzero when the crossings, counted +1 or -1 by the direction of the
// outline, do not add up to zero
enum class SweepFillRule
{
    EvenOdd,
    Nonzero
};

// Fill: the area of the first operand alone, the others combine both operands
enum class SweepOperation
{
    Fill,
    Or,
    And,
    Xor,
    Diff
};

/** the area of eOperation on rFirst and rSecond, each filled by its own rule, as polygons that
    cross neither themselves nor each other, so even-odd and nonzero give the same area. Outer
    polygons and holes have opposite orientation, curves stay curves and a segment that is not
    cut keeps its control points

    @param pSplitLimit
    the most split points one round of cutting may find, set to 0 when there are more or a
    coordinate does not fit the grid; the result then ignores the crossings
*/
B2DPolyPolygon solvePolygonsBySweep(const B2DPolyPolygon& rFirst, SweepFillRule eFirstRule,
                                    const B2DPolyPolygon& rSecond, SweepFillRule eSecondRule,
                                    SweepOperation eOperation, size_t* pSplitLimit = nullptr);

/** the parts of the lines rStroke inside the area rClip, filled by eClipRule, or with bInside
    false the parts outside of it. A part on the border of rClip counts as inside when rClip is on
    one side of it. A closed line stays closed when all of it is kept, else it gives open lines,
    one per run, and a run over its start is one line. Curves stay curves

    @param pSplitLimit
    the most split points one round of cutting may find, set to 0 when there are more or a
    coordinate does not fit the grid; the result is then empty for inside and rStroke for
    outside
*/
B2DPolyPolygon clipStrokeBySweep(const B2DPolyPolygon& rStroke, const B2DPolyPolygon& rClip,
                                 SweepFillRule eClipRule, bool bInside,
                                 size_t* pSplitLimit = nullptr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
