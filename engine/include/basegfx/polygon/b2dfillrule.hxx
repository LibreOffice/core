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

namespace basegfx
{
/** The rule that decides which areas of a filled polypolygon are inside.

    EvenOdd: a point is inside when a ray from it crosses the outline an odd number of times.
    Nonzero: a point is inside when the crossings, counted +1 or -1 by the direction of the
    outline, do not add up to zero.
 */
enum class FillRule
{
    EvenOdd,
    Nonzero
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
