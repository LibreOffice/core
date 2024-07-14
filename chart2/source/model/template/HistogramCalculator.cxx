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

    // Set min, max to the first value
    double fMinValue = rDataPoints[0];
    double fMaxValue = rDataPoints[0];

    // Compute min and max values, ignoring non-finite values
    for (auto const& rValue : rDataPoints)
    {
        if (std::isfinite(rValue))
        {
            fMinValue = std::min(fMinValue, rValue);
            fMaxValue = std::max(fMaxValue, rValue);
        }
    }

    // Round min and max to 6 decimal places
    // Not sure this is needed or desired
    fMinValue = std::round(fMinValue * 1e6) / 1e6;
    fMaxValue = std::round(fMaxValue * 1e6) / 1e6;

    // Handle the case where all values are the same
    if (fMinValue == fMaxValue)
    {
        maBinRanges = { { fMinValue, fMinValue } };
        maBinFrequencies = { sal_Int32(rDataPoints.size()) };
        return;
    }

    mnBins = sal_Int32(std::sqrt(rDataPoints.size()));

    // Calculate bin width, ensuring it's not zero and rounding to 6 decimal places
    mfBinWidth = std::round((fMaxValue - fMinValue) / mnBins * 1e6) / 1e6;

    if (mfBinWidth <= 0.0)
    {
        mfBinWidth = 0.000001; //minimum bin width of 0.000001
        mnBins = sal_Int32(std::ceil((fMaxValue - fMinValue) / mfBinWidth));
    }

    //recalculate maxValue to ensure it's included in the last bin
    fMaxValue = fMinValue + mfBinWidth * mnBins;

    // Initialize bin ranges and frequencies
    maBinRanges.resize(mnBins);
    maBinFrequencies.resize(mnBins, 0);

    // Calculate bin ranges
    for (sal_Int32 nBin = 0; nBin < mnBins; ++nBin)
    {
        double fBinStart = fMinValue + nBin * mfBinWidth;
        double fBinEnd = (nBin == mnBins - 1) ? fMaxValue : (fBinStart + mfBinWidth);
        maBinRanges[nBin] = { std::round(fBinStart * 1e6) / 1e6, std::round(fBinEnd * 1e6) / 1e6 };
    }

    // Calculate frequencies
    for (double fValue : rDataPoints)
    {
        if (std::isfinite(fValue))
        {
            // Calculate into which bin the value falls into
            sal_Int32 nBinIndex = sal_Int32((fValue - fMinValue) / mfBinWidth);
            // Sanitize
            nBinIndex = std::clamp(nBinIndex, sal_Int32(0), mnBins - 1);

            maBinFrequencies[nBinIndex]++;
        }
    }
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
