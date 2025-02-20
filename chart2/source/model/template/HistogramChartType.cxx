/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "HistogramChartType.hxx"
#include <servicenames_charttypes.hxx>
#include <CartesianCoordinateSystem.hxx>
#include <PropertyHelper.hxx>
#include <Axis.hxx>
#include <AxisHelper.hxx>
#include <AxisIndexDefines.hxx>
#include <DataSeries.hxx>
#include <LabeledDataSequence.hxx>
#include <HistogramDataSequence.hxx>
#include "HistogramCalculator.hxx"
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <cppuhelper/supportsservice.hxx>

#include <algorithm>
#include <cmath>
#include <iostream>
namespace com::sun::star::uno
{
class XComponentContext;
}

using namespace ::com::sun::star;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::Property;

namespace
{
enum
{
    PROP_HISTOGRAMCHARTTYPE_BINWIDTH,
    PROP_HISTOGRAMCHARTTYPE_BINRANGE,
    PROP_HISTOGRAMCHARTTYPE_FREQUENCYTYPE,
    PROP_HISTOGRAMCHARTTYPE_OVERLAP_SEQUENCE,
    PROP_HISTOGRAMCHARTTYPE_GAPWIDTH_SEQUENCE
};

void lcl_AddPropertiesToVector(std::vector<Property>& rOutProperties)
{
    rOutProperties.emplace_back(
        "BinWidth", PROP_HISTOGRAMCHARTTYPE_BINWIDTH, cppu::UnoType<double>::get(),
        beans::PropertyAttribute::BOUND | beans::PropertyAttribute::MAYBEDEFAULT);

    rOutProperties.emplace_back(
        "BinRange", PROP_HISTOGRAMCHARTTYPE_BINRANGE, cppu::UnoType<double>::get(),
        beans::PropertyAttribute::BOUND | beans::PropertyAttribute::MAYBEDEFAULT);

    rOutProperties.emplace_back(
        "FrequencyType", PROP_HISTOGRAMCHARTTYPE_FREQUENCYTYPE, cppu::UnoType<sal_Int32>::get(),
        beans::PropertyAttribute::BOUND | beans::PropertyAttribute::MAYBEDEFAULT);

    rOutProperties.emplace_back("OverlapSequence", PROP_HISTOGRAMCHARTTYPE_OVERLAP_SEQUENCE,
                                cppu::UnoType<Sequence<sal_Int32>>::get(),
                                beans::PropertyAttribute::BOUND
                                    | beans::PropertyAttribute::MAYBEDEFAULT);

    rOutProperties.emplace_back("GapwidthSequence", PROP_HISTOGRAMCHARTTYPE_GAPWIDTH_SEQUENCE,
                                cppu::UnoType<sal_Int32>::get(),
                                beans::PropertyAttribute::BOUND
                                    | beans::PropertyAttribute::MAYBEDEFAULT);
}

::cppu::OPropertyArrayHelper& StaticHistogramChartTypeInfoHelper()
{
    static ::cppu::OPropertyArrayHelper aPropHelper = []() {
        std::vector<css::beans::Property> aProperties;
        lcl_AddPropertiesToVector(aProperties);

        std::sort(aProperties.begin(), aProperties.end(), ::chart::PropertyNameLess());

        return comphelper::containerToSequence(aProperties);
    }();
    return aPropHelper;
};

} // anonymous namespace

namespace chart
{
HistogramChartType::HistogramChartType() {}

HistogramChartType::HistogramChartType(const HistogramChartType& rOther)
    : ChartType(rOther)
{
}

HistogramChartType::~HistogramChartType() {}

// ____ XCloneable ____
uno::Reference<util::XCloneable> SAL_CALL HistogramChartType::createClone()
{
    return uno::Reference<util::XCloneable>(new HistogramChartType(*this));
}

rtl::Reference<ChartType> HistogramChartType::cloneChartType() const
{
    return new HistogramChartType(*this);
}

// ____ XChartType ____
rtl::Reference<::chart::BaseCoordinateSystem>
HistogramChartType::createCoordinateSystem2(sal_Int32 DimensionCount)
{
    rtl::Reference<CartesianCoordinateSystem> xResult
        = new CartesianCoordinateSystem(DimensionCount);

    for (sal_Int32 i = 0; i < DimensionCount; i++)
    {
        rtl::Reference<Axis> xAxis = xResult->getAxisByDimension2(i, MAIN_AXIS_INDEX);
        if (xAxis.is())
        {
            chart2::ScaleData aScaleData = xAxis->getScaleData();
            aScaleData.Orientation = chart2::AxisOrientation_MATHEMATICAL;
            aScaleData.Scaling = AxisHelper::createLinearScaling();

            if (i == 0) //X-axis
            {
                aScaleData.AxisType = chart2::AxisType::REALNUMBER;
                aScaleData.AutoDateAxis = false;
            }
            else if (i == 1) //Y-axis
            {
                aScaleData.AxisType = chart2::AxisType::REALNUMBER;
            }

            xAxis->setScaleData(aScaleData);
        }
    }

    return xResult;
}

OUString SAL_CALL HistogramChartType::getChartType()
{
    return CHART2_SERVICE_NAME_CHARTTYPE_HISTOGRAM;
}

uno::Sequence<OUString> HistogramChartType::getSupportedPropertyRoles()
{
    return { u"FillColor"_ustr, u"BorderColor"_ustr };
}

::cppu::IPropertyArrayHelper& SAL_CALL HistogramChartType::getInfoHelper()
{
    return StaticHistogramChartTypeInfoHelper();
}

// ____ OPropertySet ____
void HistogramChartType::GetDefaultValue(sal_Int32 nHandle, uno::Any& rAny) const
{
    static const ::chart::tPropertyValueMap aStaticDefaults = []() {
        ::chart::tPropertyValueMap aTmp;
        Sequence<sal_Int32> aSeq{ 0 }; // No gap for histogram
        ::chart::PropertyHelper::setPropertyValueDefault(
            aTmp, PROP_HISTOGRAMCHARTTYPE_GAPWIDTH_SEQUENCE, aSeq);
        ::chart::PropertyHelper::setPropertyValueDefault(
            aTmp, PROP_HISTOGRAMCHARTTYPE_OVERLAP_SEQUENCE, aSeq);
        ::chart::PropertyHelper::setPropertyValueDefault(aTmp, PROP_HISTOGRAMCHARTTYPE_BINWIDTH,
                                                         2.0);
        ::chart::PropertyHelper::setPropertyValueDefault(aTmp, PROP_HISTOGRAMCHARTTYPE_BINRANGE,
                                                         1.0);
        ::chart::PropertyHelper::setPropertyValueDefault(
            aTmp, PROP_HISTOGRAMCHARTTYPE_FREQUENCYTYPE, sal_Int32(0));
        return aTmp;
    }();

    tPropertyValueMap::const_iterator aFound(aStaticDefaults.find(nHandle));
    if (aFound == aStaticDefaults.end())
        rAny.clear();
    else
        rAny = (*aFound).second;
}
// ____ XPropertySet ____
uno::Reference<beans::XPropertySetInfo> SAL_CALL HistogramChartType::getPropertySetInfo()
{
    static uno::Reference<beans::XPropertySetInfo> xPropertySetInfo(
        ::cppu::OPropertySetHelper::createPropertySetInfo(StaticHistogramChartTypeInfoHelper()));
    return xPropertySetInfo;
}

OUString SAL_CALL HistogramChartType::getImplementationName()
{
    return "com.sun.star.comp.chart.HistogramChartType";
}

sal_Bool SAL_CALL HistogramChartType::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> SAL_CALL HistogramChartType::getSupportedServiceNames()
{
    return { CHART2_SERVICE_NAME_CHARTTYPE_HISTOGRAM, "com.sun.star.chart2.ChartType" };
}

namespace
{
void setRoleToTheSequence(uno::Reference<chart2::data::XDataSequence> const& xSequence,
                          OUString const& rRole)
{
    if (!xSequence.is())
        return;
    try
    {
        uno::Reference<beans::XPropertySet> xProperty(xSequence, uno::UNO_QUERY_THROW);
        xProperty->setPropertyValue(u"Role"_ustr, uno::Any(rRole));
    }
    catch (const uno::Exception&)
    {
    }
}
}

void HistogramChartType::createCalculatedDataSeries()
{
    if (m_aDataSeries.empty())
        return;

    std::vector<uno::Reference<chart2::data::XLabeledDataSequence>> const& aDataSequences
        = m_aDataSeries[0]->getDataSequences2();

    if (aDataSequences.empty() || !aDataSequences[0].is())
        return;

    // Extract raw data from the spreadsheet
    std::vector<double> rawData;
    uno::Reference<chart2::data::XDataSequence> xValues = aDataSequences[0]->getValues();

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

    rtl::Reference<HistogramDataSequence> aValuesDataSequence = new HistogramDataSequence();
    aValuesDataSequence->setValues(comphelper::containerToSequence(aValues));
    aValuesDataSequence->setLabels(comphelper::containerToSequence(aLabels));

    setRoleToTheSequence(aValuesDataSequence, u"values-y"_ustr);

    m_aDataSeries[0]->addDataSequence(new LabeledDataSequence(aValuesDataSequence));
}

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_chart_HistogramChartType_get_implementation(
    css::uno::XComponentContext* /*context*/, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ::chart::HistogramChartType());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
