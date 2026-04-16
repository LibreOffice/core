/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vector>

#include <HistogramCalculator.hxx>
#include "HistogramDataInterpreter.hxx"
#include <HistogramDataSequence.hxx>

#include <CommonConverters.hxx>
#include <DataSeries.hxx>
#include <DataSource.hxx>
#include <DataSeriesHelper.hxx>
#include <LabeledDataSequence.hxx>

#include <comphelper/container.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/config.h>

#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/uno/Exception.hpp>

using namespace css;

namespace chart
{
InterpretedData HistogramDataInterpreter::interpretDataSource(
    const uno::Reference<chart2::data::XDataSource>& xSource,
    const uno::Sequence<beans::PropertyValue>& /*aArguments*/,
    const std::vector<rtl::Reference<DataSeries>>& aSeriesToReUse)
{
    if (!xSource.is())
    {
        return InterpretedData();
    }

    std::vector<uno::Reference<chart2::data::XLabeledDataSequence>> sourceSequences
        = DataInterpreter::getDataSequences(xSource);

    uno::Reference<chart2::data::XLabeledDataSequence> valuesYSeqLabeled;
    uno::Reference<chart2::data::XDataSequence> valuesYSeqData;

    for (const auto& seq : sourceSequences)
    {
        if (seq.is() && seq->getValues().is() && DataSeriesHelper::getRole(seq) == u"values-y")
        {
            valuesYSeqLabeled = seq;
            valuesYSeqData = seq->getValues();
            break;
        }
    }

    if (!valuesYSeqData.is())
        return InterpretedData();

    // True = Categories (Bins), False = Frequencies (Heights)
    rtl::Reference<HistogramDataSequence> xCategoryValuesSeq
        = new HistogramDataSequence(valuesYSeqData, true);
    rtl::Reference<HistogramDataSequence> xCalcSequenceImpl
        = new HistogramDataSequence(valuesYSeqData, false);

    uno::Reference<chart2::data::XDataSequence> xCategoryDataSeq(xCategoryValuesSeq);
    uno::Reference<beans::XPropertySet> xCategoryProp(xCategoryDataSeq, uno::UNO_QUERY);
    if (xCategoryProp.is())
        xCategoryProp->setPropertyValue(u"Role"_ustr, uno::Any(u"categories"_ustr));

    uno::Reference<chart2::data::XLabeledDataSequence> xLabeledCategorySeq
        = new LabeledDataSequence(xCategoryValuesSeq);
    uno::Reference<chart2::data::XLabeledDataSequence> xLabeledFreqSeq
        = new LabeledDataSequence(xCalcSequenceImpl);

    InterpretedData result;
    result.Categories = xLabeledCategorySeq;

    rtl::Reference<DataSeries> resultSeries;
    if (!aSeriesToReUse.empty() && aSeriesToReUse[0].is())
    {
        resultSeries = aSeriesToReUse[0];
        resultSeries->setData(DataSeries::tDataSequenceContainer{});
    }
    else
    {
        resultSeries = new DataSeries();
    }

    resultSeries->setData(DataSeries::tDataSequenceContainer{ valuesYSeqLabeled });
    resultSeries->setCalculatedYSequence(xLabeledFreqSeq);

    // Structure the result: one group, one series for a standard histogram
    // When multiple data columns are selected, only the first values-y series is used to build the histogram.
    result.Series.resize(1);
    result.Series[0].resize(1);
    result.Series[0][0] = resultSeries;

    return result;
}

InterpretedData
HistogramDataInterpreter::reinterpretDataSeries(const InterpretedData& rInterpretedData)
{
    rtl::Reference<DataSource> xSource = DataInterpreter::mergeInterpretedData(rInterpretedData);
    std::vector<rtl::Reference<DataSeries>> aSeriesToReUse
        = FlattenSequence(rInterpretedData.Series);

    uno::Sequence<beans::PropertyValue> aArguments;
    return interpretDataSource(xSource, aArguments, aSeriesToReUse);
}

bool HistogramDataInterpreter::isDataCompatible(const InterpretedData& aInterpretedData)
{
    // We need at least one numerical data series for histogram
    const std::vector<rtl::Reference<DataSeries>> aSeries(FlattenSequence(aInterpretedData.Series));

    if (aSeries.empty())
        return false;

    // Check if we have valid numerical data
    for (const auto& xSeries : aSeries)
    {
        if (!xSeries.is())
            continue;

        bool bFoundValidData = false;
        const auto& sequences = xSeries->getDataSequences2();

        for (const auto& seq : sequences)
        {
            if (!seq.is() || !seq->getValues().is())
                continue;

            OUString role = DataSeriesHelper::getRole(seq);
            if (role == u"values-y")
            {
                try
                {
                    uno::Sequence<uno::Any> data = seq->getValues()->getData();
                    if (!data.hasElements())
                        continue;

                    // Check first element is numerical
                    double dummy;
                    if (data[0] >>= dummy)
                    {
                        bFoundValidData = true;
                        break;
                    }
                }
                catch (const uno::Exception&)
                {
                    DBG_UNHANDLED_EXCEPTION("chart2");
                }
            }
        }

        if (!bFoundValidData)
            return false;
    }

    return true;
}

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
