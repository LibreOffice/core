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
class HistogramCalculator
{
    sal_Int32 mnBins = 1;
    double mfBinWidth = 1.0;

    std::vector<std::pair<double, double>> maBinRanges;
    std::vector<sal_Int32> maBinFrequencies;

public:
    HistogramCalculator();

    void computeBinFrequencyHistogram(const std::vector<double>& dataPoints);

    const std::vector<std::pair<double, double>>& getBinRanges() const { return maBinRanges; }
    const std::vector<sal_Int32>& getBinFrequencies() const { return maBinFrequencies; }
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
