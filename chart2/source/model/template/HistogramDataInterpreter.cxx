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
    const uno::Sequence<beans::PropertyValue>& aArguments,
    const std::vector<rtl::Reference<DataSeries>>& aSeriesToReUse)
{
    if (!xSource.is())
        return InterpretedData();

    InterpretedData aInterpretedData(
        DataInterpreter::interpretDataSource(xSource, aArguments, aSeriesToReUse));

    std::vector<uno::Reference<chart2::data::XLabeledDataSequence>> aData
        = DataInterpreter::getDataSequences(xSource);

    if (aData.empty() || !aData[0].is())
        return InterpretedData();

    SetRole(aData[0]->getValues(), u"values-y-original"_ustr);

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
