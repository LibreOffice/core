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

#include "CorrelationCircleDataInterpreter.hxx"
#include <DataSeries.hxx>
#include <DataSeriesHelper.hxx>
#include <CommonConverters.hxx>

#include <com/sun/star/util/XCloneable.hpp>
#include <comphelper/diagnose_ex.hxx>

#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace
{
/** The dimension columns belong to every series, so each series past the first
    gets a copy of them.
 */
uno::Reference<chart2::data::XLabeledDataSequence>
lcl_shareWithSeries(const uno::Reference<chart2::data::XLabeledDataSequence>& rShared,
                    const OUString& rRole, std::size_t nSeriesIndex)
{
    uno::Reference<chart2::data::XLabeledDataSequence> xResult(rShared);

    if (rShared.is() && nSeriesIndex > 0)
    {
        uno::Reference<util::XCloneable> xCloneable(rShared, uno::UNO_QUERY);
        if (xCloneable.is())
            xResult.set(xCloneable->createClone(), uno::UNO_QUERY);
    }

    if (xResult.is())
        ::chart::DataInterpreter::SetRole(xResult->getValues(), rRole);

    return xResult;
}

} // anonymous namespace

namespace chart
{
CorrelationCircleDataInterpreter::CorrelationCircleDataInterpreter() {}

CorrelationCircleDataInterpreter::~CorrelationCircleDataInterpreter() {}

// ____ XDataInterpreter ____
InterpretedData CorrelationCircleDataInterpreter::interpretDataSource(
    const Reference<chart2::data::XDataSource>& xSource,
    const Sequence<beans::PropertyValue>& aArguments,
    const std::vector<rtl::Reference<DataSeries>>& aSeriesToReUse)
{
    if (!xSource.is())
        return InterpretedData();

    std::vector<uno::Reference<chart2::data::XLabeledDataSequence>> aData
        = DataInterpreter::getDataSequences(xSource);

    uno::Reference<chart2::data::XLabeledDataSequence> xCategories;
    if (HasCategories(aArguments, aData) && !aData.empty())
    {
        xCategories = aData.front();
        aData.erase(aData.begin());
        if (xCategories.is())
            SetRole(xCategories->getValues(), u"categories"_ustr);
    }

    std::vector<rtl::Reference<DataSeries>> aSeriesVec;

    // Two dimension columns and one feature column are the smallest input that
    // yields a single point.
    if (aData.size() >= 3)
    {
        uno::Reference<chart2::data::XLabeledDataSequence> xValuesX(aData[aData.size() - 2]);
        uno::Reference<chart2::data::XLabeledDataSequence> xValuesY(aData[aData.size() - 1]);
        aData.resize(aData.size() - 2);

        aSeriesVec.reserve(aData.size());

        for (std::size_t nIndex = 0; nIndex < aData.size(); ++nIndex)
        {
            try
            {
                std::vector<uno::Reference<chart2::data::XLabeledDataSequence>> aNewData{
                    lcl_shareWithSeries(xValuesX, u"values-x"_ustr, nIndex),
                    lcl_shareWithSeries(xValuesY, u"values-y"_ustr, nIndex)
                };

                if (aData[nIndex].is())
                    SetRole(aData[nIndex]->getValues(), u"values-feature"_ustr);
                aNewData.push_back(aData[nIndex]);

                rtl::Reference<DataSeries> xSeries;
                if (nIndex < aSeriesToReUse.size())
                    xSeries = aSeriesToReUse[nIndex];
                else
                    xSeries = new DataSeries;
                assert(xSeries.is());
                xSeries->setData(aNewData);

                aSeriesVec.push_back(xSeries);
            }
            catch (const uno::Exception&)
            {
                DBG_UNHANDLED_EXCEPTION("chart2");
            }
        }
    }

    return { { std::move(aSeriesVec) }, xCategories };
}

InterpretedData
CorrelationCircleDataInterpreter::reinterpretDataSeries(const InterpretedData& aInterpretedData)
{
    InterpretedData aResult(aInterpretedData);

    std::vector<rtl::Reference<DataSeries>> aSeries(FlattenSequence(aInterpretedData.Series));
    for (auto const& xSeries : aSeries)
    {
        try
        {
            uno::Reference<chart2::data::XLabeledDataSequence> xValuesFeature(
                DataSeriesHelper::getDataSequenceByRole(xSeries, u"values-feature"_ustr));
            uno::Reference<chart2::data::XLabeledDataSequence> xValuesY(
                DataSeriesHelper::getDataSequenceByRole(xSeries, u"values-y"_ustr));
            uno::Reference<chart2::data::XLabeledDataSequence> xValuesX(
                DataSeriesHelper::getDataSequenceByRole(xSeries, u"values-x"_ustr));

            if (!xValuesFeature.is() || !xValuesY.is() || !xValuesX.is())
            {
                // Sequences the roles above did not claim fill the empty roles
                // in turn, so that a series coming from another chart type with
                // three sequences keeps all of them.
                std::vector<uno::Reference<chart2::data::XLabeledDataSequence>> aValueSeqVec(
                    DataSeriesHelper::getAllDataSequencesByRole(xSeries->getDataSequences2(),
                                                                u"values"_ustr));
                for (auto const& xUsed : { xValuesFeature, xValuesY, xValuesX })
                {
                    if (xUsed.is())
                    {
                        auto aIt = std::find(aValueSeqVec.begin(), aValueSeqVec.end(), xUsed);
                        if (aIt != aValueSeqVec.end())
                            aValueSeqVec.erase(aIt);
                    }
                }

                std::size_t nNextFree = 0;
                auto lclTakeNext
                    = [&aValueSeqVec,
                       &nNextFree](uno::Reference<chart2::data::XLabeledDataSequence>& rSequence,
                                   const OUString& rRole) {
                          if (!rSequence.is() && nNextFree < aValueSeqVec.size())
                          {
                              rSequence = aValueSeqVec[nNextFree++];
                              if (rSequence.is())
                                  SetRole(rSequence->getValues(), rRole);
                          }
                      };

                lclTakeNext(xValuesFeature, u"values-feature"_ustr);
                lclTakeNext(xValuesY, u"values-y"_ustr);
                lclTakeNext(xValuesX, u"values-x"_ustr);
            }

            if (xValuesFeature.is() && xValuesX.is() && xValuesY.is())
            {
                std::vector<uno::Reference<chart2::data::XLabeledDataSequence>> aNewSequences{
                    xValuesX, xValuesY, xValuesFeature
                };
                if (xSeries->getDataSequences2() != aNewSequences)
                    xSeries->setData(aNewSequences);
            }
        }
        catch (const uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }

    return aResult;
}

bool CorrelationCircleDataInterpreter::isDataCompatible(const InterpretedData& aInterpretedData)
{
    const std::vector<rtl::Reference<DataSeries>> aSeries(FlattenSequence(aInterpretedData.Series));
    for (rtl::Reference<DataSeries> const& xSeries : aSeries)
    {
        try
        {
            if (xSeries->getDataSequences2().size() != 3)
                return false;
        }
        catch (const uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }

    return true;
}

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
