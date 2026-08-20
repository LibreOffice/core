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

#include <tools/long.hxx>

namespace drawinglayer
{
/** Squared Euclidean distance from every pixel of a grid to the nearest object pixel, in linear
    time. The grid holds 0.0 at the object and a larger value elsewhere on entry, the squared
    distances on exit.

    The lower envelope of parabolas from "Distance Transforms of Sampled Functions" by Pedro
    Felzenszwalb and Daniel Huttenlocher.
*/
class EuclideanDistanceTransform final
{
    tools::Long mnWidth;
    tools::Long mnHeight;

public:
    EuclideanDistanceTransform(tools::Long nWidth, tools::Long nHeight);

    void execute(float* pGrid) const;
};

} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
