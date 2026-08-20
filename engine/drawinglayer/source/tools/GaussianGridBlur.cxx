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

#include <GaussianGridBlur.hxx>

#include <algorithm>
#include <cmath>
#include <vector>

namespace drawinglayer
{
namespace
{
// Largest whole box whose variance does not overshoot what one pass has to reach.
tools::Long wholeBoxRadius(double fVariance)
{
    return static_cast<tools::Long>(
        std::max(0.0, std::floor((-1.0 + std::sqrt(1.0 + 12.0 * fVariance)) * 0.5)));
}

// Weight for the pixel at nRadius + 1, carrying the variance the whole boxes leave. Its square
// always exceeds fVariance, so the division is safe.
double tailWeight(double fVariance, tools::Long nRadius)
{
    const double fWholeVariance = double(nRadius) * double(nRadius + 1) / 3.0;
    const double fNextSquared = double(nRadius + 1) * double(nRadius + 1);

    return std::clamp((double(2 * nRadius + 1) * (fVariance - fWholeVariance))
                          / (2.0 * (fNextSquared - fVariance)),
                      0.0, 1.0);
}
}

GaussianGridBlur::GaussianGridBlur(tools::Long nWidth, tools::Long nHeight, double fSigma)
    : mnWidth(nWidth)
    , mnHeight(nHeight)
    , mnRadius(wholeBoxRadius(fSigma * fSigma / 3.0))
    , mfTailWeight(float(tailWeight(fSigma * fSigma / 3.0, mnRadius)))
{
}

// Mean over a square of mnRadius pixels each side, along the rows then the columns, repeating the
// edge values outside the grid. The pixel one step beyond each side counts mfTailWeight rather than
// a whole one. A running sum makes the cost independent of the radius. pScratch takes mnWidth times
// mnHeight entries.
void GaussianGridBlur::boxBlur(float* pGrid, float* pScratch) const
{
    const float fInverseKernelSize = 1.0f / (float(2 * mnRadius + 1) + 2.0f * mfTailWeight);

    for (tools::Long nY = 0; nY < mnHeight; ++nY)
    {
        const float* pRow = pGrid + nY * mnWidth;
        float* pTarget = pScratch + nY * mnWidth;
        float fSum = 0.0f;

        for (tools::Long nOffset = -mnRadius; nOffset <= mnRadius; ++nOffset)
        {
            fSum += pRow[std::clamp(nOffset, tools::Long(0), mnWidth - 1)];
        }

        pTarget[0] = (fSum + mfTailWeight * (pRow[0] + pRow[std::min(mnRadius + 1, mnWidth - 1)]))
                     * fInverseKernelSize;

        for (tools::Long nX = 1; nX < mnWidth; ++nX)
        {
            const tools::Long nEntering = std::min(nX + mnRadius, mnWidth - 1);
            // The near tail is the pixel the running sum has just dropped.
            const tools::Long nLeaving = std::max(nX - mnRadius - 1, tools::Long(0));
            const tools::Long nFarSide = std::min(nX + mnRadius + 1, mnWidth - 1);
            fSum += pRow[nEntering] - pRow[nLeaving];
            pTarget[nX]
                = (fSum + mfTailWeight * (pRow[nLeaving] + pRow[nFarSide])) * fInverseKernelSize;
        }
    }

    // Eight columns at a time, so every load is contiguous and the eight sums are independent.
    constexpr tools::Long nColumnBlock = 8;

    for (tools::Long nFirstColumn = 0; nFirstColumn < mnWidth; nFirstColumn += nColumnBlock)
    {
        const tools::Long nColumns = std::min(nColumnBlock, mnWidth - nFirstColumn);
        float aSums[nColumnBlock] = {};

        for (tools::Long nOffset = -mnRadius; nOffset <= mnRadius; ++nOffset)
        {
            const float* pRow = pScratch
                                + std::clamp(nOffset, tools::Long(0), mnHeight - 1) * mnWidth
                                + nFirstColumn;

            for (tools::Long nColumn = 0; nColumn < nColumns; ++nColumn)
                aSums[nColumn] += pRow[nColumn];
        }

        {
            const float* pNearTail = pScratch + nFirstColumn;
            const float* pFarTail
                = pScratch + std::min(mnRadius + 1, mnHeight - 1) * mnWidth + nFirstColumn;

            for (tools::Long nColumn = 0; nColumn < nColumns; ++nColumn)
            {
                pGrid[nFirstColumn + nColumn]
                    = (aSums[nColumn] + mfTailWeight * (pNearTail[nColumn] + pFarTail[nColumn]))
                      * fInverseKernelSize;
            }
        }

        for (tools::Long nY = 1; nY < mnHeight; ++nY)
        {
            const float* pEntering
                = pScratch + std::min(nY + mnRadius, mnHeight - 1) * mnWidth + nFirstColumn;
            const float* pLeaving
                = pScratch + std::max(nY - mnRadius - 1, tools::Long(0)) * mnWidth + nFirstColumn;
            const float* pFarSide
                = pScratch + std::min(nY + mnRadius + 1, mnHeight - 1) * mnWidth + nFirstColumn;
            float* pTarget = pGrid + nY * mnWidth + nFirstColumn;

            for (tools::Long nColumn = 0; nColumn < nColumns; ++nColumn)
            {
                aSums[nColumn] += pEntering[nColumn] - pLeaving[nColumn];
                pTarget[nColumn]
                    = (aSums[nColumn] + mfTailWeight * (pLeaving[nColumn] + pFarSide[nColumn]))
                      * fInverseKernelSize;
            }
        }
    }
}

void GaussianGridBlur::execute(float* pGrid) const
{
    if (mnRadius == 0 && mfTailWeight <= 0.0f)
        return;

    std::vector<float> aScratch(mnWidth * mnHeight);

    for (int nPass = 0; nPass < 3; ++nPass)
    {
        boxBlur(pGrid, aScratch.data());
    }
}

} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
