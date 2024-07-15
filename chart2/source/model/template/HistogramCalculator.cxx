/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "HistogramCalculator.hxx"

#include <algorithm>
#include <cmath>

namespace chart
{
HistogramCalculator::HistogramCalculator() = default;

void HistogramCalculator::computeBinFrequencyHistogram(const std::vector<double>& rDataPoints)
{
    if (rDataPoints.empty())
        return;

    mnBins = 1;
    mfBinWidth = 1.0;
    maBinRanges.clear();
    maBinFrequencies.clear();

    // Calculate statistics
    double fSum = 0.0;
    double fSquareSum = 0.0;
    double fMinValue = rDataPoints[0];
    double fMaxValue = rDataPoints[0];
    sal_Int32 nValidCount = 0;

    // Compute min and max values, ignoring non-finite values
    for (const auto& rValue : rDataPoints)
    {
        if (std::isfinite(rValue))
        {
            fSum += rValue;
            fSquareSum += rValue * rValue;
            fMinValue = std::min(fMinValue, rValue);
            fMaxValue = std::max(fMaxValue, rValue);
            ++nValidCount;
        }
    }

    if (nValidCount < 2 || fMinValue == fMaxValue) // Need at least two points for variance
    {
        mnBins = 1;
        mfBinWidth = 1.0;
        maBinRanges = { { std::floor(fMinValue), std::ceil(fMinValue + 1.0) } };
        maBinFrequencies = { nValidCount };
        return;
    }

    double fMean = fSum / nValidCount;
    double fVariance = (fSquareSum - fSum * fMean) / (nValidCount - 1);
    double fStdDev = std::sqrt(fVariance);

    // Apply Scott's rule for bin width
    mfBinWidth = (3.5 * fStdDev) / std::cbrt(nValidCount);

    // Calculate number of bins
    mnBins = static_cast<sal_Int32>(std::ceil((fMaxValue - fMinValue) / mfBinWidth));
    mnBins = std::max<sal_Int32>(mnBins, 1); // Ensure at least one bin

    // Set up bin ranges
    maBinRanges.reserve(mnBins);
    double fBinStart = fMinValue;

    for (sal_Int32 i = 0; i < mnBins; ++i)
    {
        double fBinEnd = fBinStart + mfBinWidth;

        // Correct rounding to avoid discrepancies
        fBinStart = std::round(fBinStart * 100.0) / 100.0;
        fBinEnd = std::round(fBinEnd * 100.0) / 100.0;

        if (i == 0)
        {
            // First bin includes the minimum value, so use closed interval [fMinValue, fBinEnd]
            maBinRanges.emplace_back(fMinValue, fBinEnd);
        }
        else
        {
            // Subsequent bins use half-open interval (fBinStart, fBinEnd]
            maBinRanges.emplace_back(fBinStart, fBinEnd);
        }
        fBinStart = fBinEnd;
    }

    // Adjust the last bin end to be inclusive
    maBinRanges.back().second = std::max(maBinRanges.back().second, fMaxValue);

    // Calculate frequencies
    maBinFrequencies.assign(mnBins, 0);
    for (double fValue : rDataPoints)
    {
        if (std::isfinite(fValue))
        {
            for (size_t i = 0; i < maBinRanges.size(); ++i)
            {
                if (i == 0 && fValue >= maBinRanges[i].first && fValue <= maBinRanges[i].second)
                {
                    maBinFrequencies[i]++;
                    break;
                }
                else if (i > 0 && fValue > maBinRanges[i].first && fValue <= maBinRanges[i].second)
                {
                    maBinFrequencies[i]++;
                    break;
                }
            }
        }
    }
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
