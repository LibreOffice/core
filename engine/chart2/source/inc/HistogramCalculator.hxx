/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/types.h>
#include <vector>

namespace chart
{
/** Partitions a data set into histogram bins and counts per-bin frequencies.
    Binning can be automatic, by fixed bin width, or by fixed bin count. */
class HistogramCalculator
{
    sal_Int32 mnBins = 1;
    double mfBinWidth = 1.0;

    std::vector<std::pair<double, double>> maBinRanges;
    std::vector<sal_Int32> maBinFrequencies;

public:
    HistogramCalculator();

    // nFrequencyType: 0 = auto (Scott's Rule), 1 = fixed bin width, 2 = fixed bin count.
    // fFixedBinWidth / nFixedBinCount are consulted only when their mode is selected;
    // invalid values (<= 0) fall back to auto.
    void computeBinFrequencyHistogram(const std::vector<double>& dataPoints,
                                      sal_Int32 nFrequencyType = 0, double fFixedBinWidth = 0.0,
                                      sal_Int32 nFixedBinCount = 0);

    const std::vector<std::pair<double, double>>& getBinRanges() const { return maBinRanges; }
    const std::vector<sal_Int32>& getBinFrequencies() const { return maBinFrequencies; }
};

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
