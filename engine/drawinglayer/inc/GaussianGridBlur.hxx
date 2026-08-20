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
/** Gaussian blur of a float grid, approximated by three box blurs.

    A box of radius r has a variance of (r * r + r) / 3, so whole boxes only reach discrete widths.
    A fractional weight at the pixel beyond each box fills the gaps, which keeps the width
    continuous and still softens a sigma of under a pixel. A sigma of zero leaves the grid alone.
*/
class GaussianGridBlur final
{
    tools::Long mnWidth;
    tools::Long mnHeight;
    tools::Long mnRadius;
    float mfTailWeight;

    void boxBlur(float* pGrid, float* pScratch) const;

public:
    GaussianGridBlur(tools::Long nWidth, tools::Long nHeight, double fSigma);

    void execute(float* pGrid) const;
};

} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
