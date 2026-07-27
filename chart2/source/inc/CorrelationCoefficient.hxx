/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the LibreOffice contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <optional>
#include <span>

namespace chart
{
/** The Pearson correlation coefficient of two runs of values, paired up by
    position.

    A position where either side is not a finite number counts for nothing, and
    so does a position past the end of the shorter run. The answer is empty when
    fewer than two positions are left, or when one of the two runs holds the
    same value throughout, because then there is no coefficient to give. The
    answer otherwise lies between minus one and one.
 */
inline std::optional<double> calculateCorrelationCoefficient(std::span<const double> aFirst,
                                                             std::span<const double> aSecond)
{
    const std::size_t nLength = std::min(aFirst.size(), aSecond.size());

    auto lclIsPair = [&aFirst, &aSecond](std::size_t nIndex) {
        return std::isfinite(aFirst[nIndex]) && std::isfinite(aSecond[nIndex]);
    };

    std::size_t nCount = 0;
    double fSumFirst = 0.0;
    double fSumSecond = 0.0;
    for (std::size_t nIndex = 0; nIndex < nLength; ++nIndex)
    {
        if (lclIsPair(nIndex))
        {
            ++nCount;
            fSumFirst += aFirst[nIndex];
            fSumSecond += aSecond[nIndex];
        }
    }

    if (nCount < 2)
        return {};

    const double fMeanFirst = fSumFirst / nCount;
    const double fMeanSecond = fSumSecond / nCount;

    // Subtracting the means before multiplying keeps the answer accurate for
    // values that are large next to how far they spread.
    double fProductSum = 0.0;
    double fSquareSumFirst = 0.0;
    double fSquareSumSecond = 0.0;
    for (std::size_t nIndex = 0; nIndex < nLength; ++nIndex)
    {
        if (lclIsPair(nIndex))
        {
            const double fDeltaFirst = aFirst[nIndex] - fMeanFirst;
            const double fDeltaSecond = aSecond[nIndex] - fMeanSecond;
            fProductSum += fDeltaFirst * fDeltaSecond;
            fSquareSumFirst += fDeltaFirst * fDeltaFirst;
            fSquareSumSecond += fDeltaSecond * fDeltaSecond;
        }
    }

    if (fSquareSumFirst <= 0.0 || fSquareSumSecond <= 0.0)
        return {};

    const double fCoefficient = fProductSum / std::sqrt(fSquareSumFirst * fSquareSumSecond);

    // Rounding can carry the division a little past one, so hold the answer
    // inside the range a coefficient has.
    return std::clamp(fCoefficient, -1.0, 1.0);
}

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
