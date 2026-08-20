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

#include <EuclideanDistanceTransform.hxx>

#include <algorithm>
#include <limits>
#include <vector>

namespace drawinglayer
{
namespace
{
// Where the parabola rooted at nRight drops below the one at nLeft. Both have the same shape, each
// lifted by its own input value.
float parabolaCrossing(const float* pInput, tools::Long nLeft, tools::Long nRight)
{
    return ((pInput[nRight] + float(nRight * nRight)) - (pInput[nLeft] + float(nLeft * nLeft)))
           / float(2 * nRight - 2 * nLeft);
}

// One dimensional squared distance transform along nCount positions. pHullPositions and
// pHullBoundaries are scratch, nCount and nCount + 1 entries.
void distanceTransform1D(const float* pInput, float* pOutput, tools::Long nCount,
                         tools::Long* pHullPositions, float* pHullBoundaries)
{
    const float fInfinity = std::numeric_limits<float>::infinity();

    // Lower envelope: the winning roots, and where each one takes over.
    tools::Long nHull = 0;
    pHullPositions[0] = 0;
    pHullBoundaries[0] = -fInfinity;
    pHullBoundaries[1] = fInfinity;

    for (tools::Long nPosition = 1; nPosition < nCount; ++nPosition)
    {
        float fCrossing = parabolaCrossing(pInput, pHullPositions[nHull], nPosition);

        // The new parabola takes over before the last one did, so the last one leaves it.
        while (fCrossing <= pHullBoundaries[nHull])
        {
            --nHull;
            fCrossing = parabolaCrossing(pInput, pHullPositions[nHull], nPosition);
        }

        ++nHull;
        pHullPositions[nHull] = nPosition;
        pHullBoundaries[nHull] = fCrossing;
        pHullBoundaries[nHull + 1] = fInfinity;
    }

    // Sample the winning parabola at each position.
    nHull = 0;

    for (tools::Long nPosition = 0; nPosition < nCount; ++nPosition)
    {
        while (pHullBoundaries[nHull + 1] < float(nPosition))
            ++nHull;

        const tools::Long nRoot = pHullPositions[nHull];
        const tools::Long nDelta = nPosition - nRoot;
        pOutput[nPosition] = float(nDelta * nDelta) + pInput[nRoot];
    }
}
}

EuclideanDistanceTransform::EuclideanDistanceTransform(tools::Long nWidth, tools::Long nHeight)
    : mnWidth(nWidth)
    , mnHeight(nHeight)
{
}

void EuclideanDistanceTransform::execute(float* pGrid) const
{
    const tools::Long nLongestSide = std::max(mnWidth, mnHeight);
    std::vector<float> aInput(nLongestSide);
    std::vector<float> aOutput(nLongestSide);
    std::vector<tools::Long> aHullPositions(nLongestSide);
    std::vector<float> aHullBoundaries(nLongestSide + 1);
    float* pInput = aInput.data();
    float* pOutput = aOutput.data();
    tools::Long* pHullPositions = aHullPositions.data();
    float* pHullBoundaries = aHullBoundaries.data();

    for (tools::Long nY = 0; nY < mnHeight; ++nY)
    {
        float* pRow = pGrid + nY * mnWidth;

        std::copy_n(pRow, mnWidth, pInput);
        distanceTransform1D(pInput, pOutput, mnWidth, pHullPositions, pHullBoundaries);
        std::copy_n(pOutput, mnWidth, pRow);
    }

    for (tools::Long nX = 0; nX < mnWidth; ++nX)
    {
        const float* pSource = pGrid + nX;

        for (tools::Long nY = 0; nY < mnHeight; ++nY, pSource += mnWidth)
        {
            pInput[nY] = *pSource;
        }

        distanceTransform1D(pInput, pOutput, mnHeight, pHullPositions, pHullBoundaries);

        float* pTarget = pGrid + nX;

        for (tools::Long nY = 0; nY < mnHeight; ++nY, pTarget += mnWidth)
        {
            *pTarget = pOutput[nY];
        }
    }
}

} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
