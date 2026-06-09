/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <HistogramCalculator.hxx>

#include <algorithm>
#include <cmath>

namespace chart
{
HistogramCalculator::HistogramCalculator() = default;

void HistogramCalculator::computeBinFrequencyHistogram(
    const std::vector<double>& rDataPoints, sal_Int32 nFrequencyType, double fFixedBinWidth,
    sal_Int32 nFixedBinCount, bool bUseUnderflowBin, double fUnderflowBinValue,
    bool bUseOverflowBin, double fOverflowBinValue)
{
    if (rDataPoints.empty())
        return;

    mnBins = 1;
    mfBinWidth = 1.0;
    maBinRanges.clear();
    maBinFrequencies.clear();
    maBinTypes.clear();

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
            if (nValidCount == 0)
            {
                fMinValue = rValue;
                fMaxValue = rValue;
            }
            else
            {
                fMinValue = std::min(fMinValue, rValue);
                fMaxValue = std::max(fMaxValue, rValue);
            }

            fSum += rValue;
            fSquareSum += rValue * rValue;
            ++nValidCount;
        }
    }

    if (nValidCount == 0)
        return;

    const bool bHasUnderflow = bUseUnderflowBin && std::isfinite(fUnderflowBinValue);
    const bool bHasOverflow = bUseOverflowBin && std::isfinite(fOverflowBinValue);

    // Ignore both special bins if their boundaries cross; otherwise normal bins
    // would have an invalid range.
    const bool bUseSpecialBins
        = !(bHasUnderflow && bHasOverflow && fUnderflowBinValue >= fOverflowBinValue);

    const bool bEffectiveUnderflow = bHasUnderflow && bUseSpecialBins;
    const bool bEffectiveOverflow = bHasOverflow && bUseSpecialBins;

    double fEffectiveMin = bEffectiveUnderflow ? fUnderflowBinValue : fMinValue;
    double fEffectiveMax = bEffectiveOverflow ? fOverflowBinValue : fMaxValue;

    if (fEffectiveMin >= fEffectiveMax)
    {
        fEffectiveMin = fMinValue;
        fEffectiveMax = fMaxValue;
    }

    if (nValidCount < 2 || fEffectiveMin == fEffectiveMax) // Need at least two points for variance
    {
        mnBins = 1;
        mfBinWidth = 1.0;

        if (bEffectiveUnderflow)
        {
            maBinTypes.push_back(HistogramBinType::Underflow);
            maBinRanges.emplace_back(fUnderflowBinValue, fUnderflowBinValue);
        }

        maBinTypes.push_back(HistogramBinType::Normal);
        maBinRanges.emplace_back(std::floor(fEffectiveMin), std::ceil(fEffectiveMin + 1.0));

        if (bEffectiveOverflow)
        {
            maBinTypes.push_back(HistogramBinType::Overflow);
            maBinRanges.emplace_back(fOverflowBinValue, fOverflowBinValue);
        }

        maBinFrequencies.assign(maBinRanges.size(), 0);
    }
    else
    {
        // Pick bin width / count based on frequency-type mode. Invalid fixed values
        // fall back to auto so stale or unset properties cannot produce a degenerate histogram.
        bool bResolved = false;
        if (nFrequencyType == 1 && fFixedBinWidth > 0.0)
        {
            mfBinWidth = fFixedBinWidth;
            mnBins
                = static_cast<sal_Int32>(std::ceil((fEffectiveMax - fEffectiveMin) / mfBinWidth));
            bResolved = true;
        }
        else if (nFrequencyType == 2 && nFixedBinCount > 0)
        {
            mnBins = nFixedBinCount;
            mfBinWidth = (fEffectiveMax - fEffectiveMin) / mnBins;
            bResolved = true;
        }

        if (!bResolved)
        {
            // Auto: Scott's rule.
            double fMean = fSum / nValidCount;
            double fVariance = (fSquareSum - fSum * fMean) / (nValidCount - 1);
            double fStdDev = std::sqrt(fVariance);

            mfBinWidth = (3.5 * fStdDev) / std::cbrt(nValidCount);
            mnBins
                = static_cast<sal_Int32>(std::ceil((fEffectiveMax - fEffectiveMin) / mfBinWidth));
        }

        mnBins = std::max<sal_Int32>(mnBins, 1); // Ensure at least one bin

        maBinRanges.reserve(mnBins + (bEffectiveUnderflow ? 1 : 0) + (bEffectiveOverflow ? 1 : 0));
        maBinTypes.reserve(mnBins + (bEffectiveUnderflow ? 1 : 0) + (bEffectiveOverflow ? 1 : 0));

        if (bEffectiveUnderflow)
        {
            maBinTypes.push_back(HistogramBinType::Underflow);
            maBinRanges.emplace_back(fUnderflowBinValue, fUnderflowBinValue);
        }

        double fBinStart = fEffectiveMin;
        for (sal_Int32 i = 0; i < mnBins; ++i)
        {
            double fBinEnd = fBinStart + mfBinWidth;

            maBinTypes.push_back(HistogramBinType::Normal);
            maBinRanges.emplace_back(fBinStart, fBinEnd);

            fBinStart = fBinEnd;
        }

        // With an overflow bin values above the overflow boundary belong to the overflow bin
        if (!maBinRanges.empty())
        {
            size_t nLastNormal = maBinRanges.size() - 1;
            if (bEffectiveOverflow)
                maBinRanges[nLastNormal].second = fEffectiveMax;
            else
                maBinRanges[nLastNormal].second
                    = std::max(maBinRanges[nLastNormal].second, fEffectiveMax);
        }

        if (bEffectiveOverflow)
        {
            maBinTypes.push_back(HistogramBinType::Overflow);
            maBinRanges.emplace_back(fOverflowBinValue, fOverflowBinValue);
        }

        maBinFrequencies.assign(maBinRanges.size(), 0);
    }

    // Calculate frequencies. Underflow is <= boundary, overflow is > boundary.
    // Normal bins are [start,end] for the first bin unless an underflow bin exists;
    // otherwise they are (start,end].
    for (double fValue : rDataPoints)
    {
        if (!std::isfinite(fValue))
            continue;

        for (size_t i = 0; i < maBinRanges.size(); ++i)
        {
            if (maBinTypes[i] == HistogramBinType::Underflow)
            {
                if (fValue <= maBinRanges[i].second)
                {
                    maBinFrequencies[i]++;
                    break;
                }
                continue;
            }

            if (maBinTypes[i] == HistogramBinType::Overflow)
            {
                if (fValue > maBinRanges[i].first)
                {
                    maBinFrequencies[i]++;
                    break;
                }
                continue;
            }

            const bool bPreviousIsUnderflow
                = i > 0 && maBinTypes[i - 1] == HistogramBinType::Underflow;
            const bool bFirstNormalBin = i == 0 || bPreviousIsUnderflow;

            const bool bInBin
                = bFirstNormalBin && !bPreviousIsUnderflow
                      ? fValue >= maBinRanges[i].first && fValue <= maBinRanges[i].second
                      : fValue > maBinRanges[i].first && fValue <= maBinRanges[i].second;

            if (bInBin)
            {
                maBinFrequencies[i]++;
                break;
            }
        }
    }
}

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */