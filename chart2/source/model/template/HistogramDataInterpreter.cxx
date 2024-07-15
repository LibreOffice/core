/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include "HistogramDataInterpreter.hxx"
#include <HistogramDataSequence.hxx>
#include "HistogramCalculator.hxx"
#include <LabeledDataSequence.hxx>

#include <DataSeries.hxx>
#include <DataSeriesHelper.hxx>
#include <CommonConverters.hxx>
#include <com/sun/star/util/XCloneable.hpp>
#include <comphelper/diagnose_ex.hxx>
#include <sal/log.hxx>

using namespace css;

namespace chart
{
InterpretedData HistogramDataInterpreter::interpretDataSource(
    const uno::Reference<chart2::data::XDataSource>& xSource,
    const uno::Sequence<beans::PropertyValue>& /*aArguments*/,
    const std::vector<rtl::Reference<DataSeries>>& /*aSeriesToReUse*/)
{
    if (!xSource.is())
        return InterpretedData();

    InterpretedData aInterpretedData;

    std::vector<uno::Reference<chart2::data::XLabeledDataSequence>> aData
        = DataInterpreter::getDataSequences(xSource);

    if (aData.empty() || !aData[0].is())
        return InterpretedData();

    SetRole(aData[0]->getValues(), u"values-y-original"_ustr);

    // Extract raw data from the spreadsheet
    std::vector<double> rawData;
    uno::Reference<chart2::data::XDataSequence> xValues = aData[0]->getValues();
    if (!xValues.is())
        return InterpretedData();

    uno::Sequence<uno::Any> aRawAnyValues = xValues->getData();

    for (const auto& aAny : aRawAnyValues)
    {
        double fValue = 0.0;
        if (aAny >>= fValue) // Extract double from Any
        {
            rawData.push_back(fValue);
        }
    }

    // Perform histogram calculations
    HistogramCalculator aHistogramCalculator;
    aHistogramCalculator.computeBinFrequencyHistogram(rawData);

    // Get bin ranges and frequencies
    const auto& binRanges = aHistogramCalculator.getBinRanges();
    const auto& binFrequencies = aHistogramCalculator.getBinFrequencies();

    // Create labels and values for HistogramDataSequence
    std::vector<OUString> aLabels;
    std::vector<double> aValues;
    for (size_t i = 0; i < binRanges.size(); ++i)
    {
        OUString aLabel;
        if (i == 0)
        {
            aLabel = u"["_ustr + OUString::number(binRanges[i].first) + u"-"_ustr
                     + OUString::number(binRanges[i].second) + u"]"_ustr;
        }
        else
        {
            aLabel = u"("_ustr + OUString::number(binRanges[i].first) + u"-"_ustr
                     + OUString::number(binRanges[i].second) + u"]"_ustr;
        }
        aLabels.push_back(aLabel);
        aValues.push_back(static_cast<double>(binFrequencies[i]));
    }

    rtl::Reference<DataSeries> xSeries = new DataSeries;
    std::vector<uno::Reference<chart2::data::XLabeledDataSequence>> aNewData;
    aNewData.push_back(aData[0]);

    rtl::Reference<HistogramDataSequence> aValuesDataSequence = new HistogramDataSequence();
    aValuesDataSequence->setValues(comphelper::containerToSequence(aValues));
    aValuesDataSequence->setLabels(comphelper::containerToSequence(aLabels));

    uno::Reference<chart2::data::XDataSequence> aDataSequence = aValuesDataSequence;
    SetRole(aDataSequence, u"values-y"_ustr);
    aNewData.push_back(new LabeledDataSequence(aDataSequence));

    xSeries->setData(aNewData);
    std::vector<rtl::Reference<DataSeries>> aSeriesVec;
    aSeriesVec.push_back(xSeries);

    aInterpretedData.Series.push_back(aSeriesVec);
    aInterpretedData.Categories = nullptr;

    return aInterpretedData;
}

InterpretedData
HistogramDataInterpreter::reinterpretDataSeries(const InterpretedData& rInterpretedData)
{
    return rInterpretedData;
}

bool HistogramDataInterpreter::isDataCompatible(const InterpretedData& /*aInterpretedData*/)
{
    return false;
}

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
