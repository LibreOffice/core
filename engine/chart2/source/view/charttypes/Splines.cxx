/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "Splines.hxx"

#include <basegfx/curve/BSpline.hxx>
#include <basegfx/curve/CubicSpline.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <com/sun/star/drawing/Position3D.hpp>
#include <osl/diagnose.h>

#include <vector>

namespace chart
{
using namespace ::com::sun::star;

// Calculates uniform parametric splines with subinterval length 1,
// according ODF1.2 part 1, chapter 'chart interpolation'.
void SplineCalculator::CalculateCubicSplines(
    const std::vector<std::vector<css::drawing::Position3D>>& rInput,
    std::vector<std::vector<css::drawing::Position3D>>& rResult, sal_uInt32 nGranularity)
{
    OSL_PRECOND(nGranularity > 0, "Granularity is invalid");

    sal_uInt32 nOuterCount = rInput.size();

    rResult.resize(nOuterCount);
    if (!nOuterCount)
        return;

    for (sal_uInt32 nOuter = 0; nOuter < nOuterCount; ++nOuter)
    {
        const auto& rSeries = rInput[nOuter];
        if (rSeries.size() <= 1)
            continue;

        const sal_uInt32 nMaxIndex = rSeries.size() - 1;

        std::vector<basegfx::B2DPoint> aPoints;
        aPoints.reserve(rSeries.size());
        for (const auto& rPoint : rSeries)
            aPoints.emplace_back(rPoint.PositionX, rPoint.PositionY);

        // A series whose first and last point coincide is a closed loop
        // and gets a periodic spline.
        const bool bPeriodic = nMaxIndex >= 2
                               && rSeries.front().PositionX == rSeries.back().PositionX
                               && rSeries.front().PositionY == rSeries.back().PositionY
                               && rSeries.front().PositionZ == rSeries.back().PositionZ;

        basegfx::CubicSpline aSpline(aPoints,
                                     bPeriodic ? basegfx::CubicSpline::BoundaryCondition::Periodic
                                               : basegfx::CubicSpline::BoundaryCondition::Natural);
        if (!aSpline.isValid())
            continue;

        std::vector<css::drawing::Position3D>& rOut = rResult[nOuter];
        rOut.resize(nMaxIndex * nGranularity + 1);

        // One sample lands at each input point, plus nGranularity - 1
        // evenly spaced samples between consecutive input points. The Z
        // coordinate stays with the left input point of each segment.
        sal_uInt32 nOutIndex = 0;
        const double fInputCount = double(nMaxIndex);
        for (sal_uInt32 nK = 0; nK < nMaxIndex; ++nK)
        {
            rOut[nOutIndex].PositionX = rSeries[nK].PositionX;
            rOut[nOutIndex].PositionY = rSeries[nK].PositionY;
            rOut[nOutIndex].PositionZ = rSeries[nK].PositionZ;
            ++nOutIndex;

            for (sal_uInt32 nJ = 1; nJ < nGranularity; ++nJ)
            {
                double fT = (double(nK) + double(nJ) / nGranularity) / fInputCount;
                basegfx::B2DPoint aSample = aSpline.getPosition(fT);
                rOut[nOutIndex].PositionX = aSample.getX();
                rOut[nOutIndex].PositionY = aSample.getY();
                rOut[nOutIndex].PositionZ = rSeries[nK].PositionZ;
                ++nOutIndex;
            }
        }
        // Last point comes straight from the input.
        rOut[nOutIndex] = rSeries[nMaxIndex];
    }
}

void SplineCalculator::CalculateBSplines(
    const std::vector<std::vector<css::drawing::Position3D>>& rInput,
    std::vector<std::vector<css::drawing::Position3D>>& rResult, sal_uInt32 nResolution,
    sal_uInt32 nDegree)
{
    OSL_ASSERT(nResolution > 1);
    OSL_ASSERT(nDegree >= 1);

    sal_uInt32 nOuterCount = rInput.size();

    rResult.resize(nOuterCount);

    if (!nOuterCount)
        return;

    for (sal_uInt32 nOuter = 0; nOuter < nOuterCount; ++nOuter)
    {
        const auto& rSeries = rInput[nOuter];
        if (rSeries.size() <= 1)
            continue;

        // Z is uniform across the series, so the first point's value
        // applies to every sample.
        const double fZCoordinate = rSeries[0].PositionZ;

        std::vector<basegfx::B2DPoint> aPoints;
        aPoints.reserve(rSeries.size());
        for (const auto& rPoint : rSeries)
            aPoints.emplace_back(rPoint.PositionX, rPoint.PositionY);

        basegfx::BSpline aSpline(aPoints, nDegree);
        if (!aSpline.isValid())
            continue;

        // After construction adjacent duplicate inputs are gone, so the
        // parameter vector reflects how many distinct points the spline
        // is built on. Output size scales with that count.
        const std::vector<double>& rT = aSpline.getInputParameters();
        const size_t nN = rT.size() - 1;
        const size_t nNewSize = nResolution * nN + 1;

        std::vector<css::drawing::Position3D>& rOut = rResult[nOuter];
        rOut.resize(nNewSize);

        // First and last samples come straight from the input so the
        // endpoints stay bit-exact.
        rOut[0] = rSeries.front();
        rOut[0].PositionZ = fZCoordinate;
        rOut[nNewSize - 1] = rSeries.back();
        rOut[nNewSize - 1].PositionZ = fZCoordinate;

        for (size_t nK = 0; nK < nN; ++nK)
        {
            for (sal_uInt32 nStep = 1;
                 nStep <= nResolution && (nK != nN - 1 || nStep != nResolution); ++nStep)
            {
                double fT = rT[nK] + nStep * (rT[nK + 1] - rT[nK]) / nResolution;
                basegfx::B2DPoint aSample = aSpline.getPosition(fT);
                rOut[nK * nResolution + nStep] = { aSample.getX(), aSample.getY(), fZCoordinate };
            }
        }
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
