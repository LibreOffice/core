/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "HistogramChartType.hxx"

#include <algorithm>
#include <cmath>
#include <vector>

#include <Axis.hxx>
#include <AxisHelper.hxx>
#include <AxisIndexDefines.hxx>
#include <CartesianCoordinateSystem.hxx>
#include <CommonFunctors.hxx>
#include <DataSeries.hxx>
#include <DataSeriesHelper.hxx>
#include <Diagram.hxx>
#include <HistogramDataSequence.hxx>
#include <LabeledDataSequence.hxx>
#include <ModifyListenerHelper.hxx>
#include <PropertyHelper.hxx>
#include <servicenames_charttypes.hxx>

#include <comphelper/diagnose_ex.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <sal/log.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/uno/Any.hxx>

namespace com::sun::star::uno
{
class XComponentContext;
}

using namespace ::com::sun::star;

namespace
{
enum
{
    PROP_HISTOGRAMCHARTTYPE_BINWIDTH,
    PROP_HISTOGRAMCHARTTYPE_BINCOUNT,
    PROP_HISTOGRAMCHARTTYPE_FREQUENCYTYPE,
    PROP_HISTOGRAMCHARTTYPE_OVERLAP_SEQUENCE,
    PROP_HISTOGRAMCHARTTYPE_GAPWIDTH_SEQUENCE
};

void lcl_AddPropertiesToVector(std::vector<beans::Property>& rOutProperties)
{
    rOutProperties.emplace_back(
        "BinWidth", PROP_HISTOGRAMCHARTTYPE_BINWIDTH, cppu::UnoType<double>::get(),
        beans::PropertyAttribute::BOUND | beans::PropertyAttribute::MAYBEDEFAULT);

    rOutProperties.emplace_back(
        "BinCount", PROP_HISTOGRAMCHARTTYPE_BINCOUNT, cppu::UnoType<sal_Int32>::get(),
        beans::PropertyAttribute::BOUND | beans::PropertyAttribute::MAYBEDEFAULT);

    rOutProperties.emplace_back(
        "FrequencyType", PROP_HISTOGRAMCHARTTYPE_FREQUENCYTYPE, cppu::UnoType<sal_Int32>::get(),
        beans::PropertyAttribute::BOUND | beans::PropertyAttribute::MAYBEDEFAULT);

    rOutProperties.emplace_back("OverlapSequence", PROP_HISTOGRAMCHARTTYPE_OVERLAP_SEQUENCE,
                                cppu::UnoType<uno::Sequence<sal_Int32>>::get(),
                                beans::PropertyAttribute::BOUND
                                    | beans::PropertyAttribute::MAYBEDEFAULT);

    rOutProperties.emplace_back("GapwidthSequence", PROP_HISTOGRAMCHARTTYPE_GAPWIDTH_SEQUENCE,
                                cppu::UnoType<uno::Sequence<sal_Int32>>::get(),
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
        if (!xAxis.is())
            continue;

        chart2::ScaleData aScaleData = xAxis->getScaleData();

        if (i == 0) // X-axis
        {
            // Set up X-axis specifically for histogram bins
            aScaleData.AxisType = chart2::AxisType::CATEGORY;
            aScaleData.AutoDateAxis = false;
            aScaleData.ShiftedCategoryPosition = true;
            aScaleData.Orientation = chart2::AxisOrientation_MATHEMATICAL;

            // Clear any existing scaling/categories
            AxisHelper::removeExplicitScaling(aScaleData);
            aScaleData.Categories.clear();
        }
        else if (i == 1) // Y-axis
        {
            aScaleData.AxisType = chart2::AxisType::REALNUMBER;
            aScaleData.ShiftedCategoryPosition = false;
            aScaleData.Orientation = chart2::AxisOrientation_MATHEMATICAL;
            aScaleData.Scaling = AxisHelper::createLinearScaling();
        }

        xAxis->setScaleData(aScaleData);
    }

    return xResult;
}

void HistogramChartType::createCalculatedDataSeries()
{
    if (m_aDataSeries.empty())
    {
        return;
    }

    // Snapshot the current binning parameters so each new HistogramDataSequence
    // runs the calculator in the correct mode (auto / fixed width / fixed count).
    sal_Int32 nFrequencyType = 0;
    double fBinWidth = 0.0;
    sal_Int32 nBinCount = 0;
    try
    {
        getPropertyValue(u"FrequencyType"_ustr) >>= nFrequencyType;
        getPropertyValue(u"BinWidth"_ustr) >>= fBinWidth;
        getPropertyValue(u"BinCount"_ustr) >>= nBinCount;
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    for (const auto& xSeries : m_aDataSeries)
    {
        if (!xSeries.is())
        {
            continue;
        }

        // 1. Find the raw values-y
        uno::Reference<chart2::data::XDataSequence> xValuesY;
        auto aSeqs = xSeries->getDataSequences2();
        for (const auto& seq : aSeqs)
        {
            if (seq.is() && seq->getValues().is() && DataSeriesHelper::getRole(seq) == "values-y")
            {
                xValuesY = seq->getValues();
                break;
            }
        }

        if (!xValuesY.is())
        {
            continue;
        }

        // 2. Regenerate the calculated-y frequencies
        rtl::Reference<HistogramDataSequence> xCalcSeq
            = new HistogramDataSequence(xValuesY, false, nFrequencyType, fBinWidth, nBinCount);
        uno::Reference<chart2::data::XLabeledDataSequence> xLabeledCalc
            = new LabeledDataSequence(xCalcSeq);
        xSeries->setCalculatedYSequence(xLabeledCalc);

        // 3. Regenerate the categories (bins)
        rtl::Reference<HistogramDataSequence> xCatSeq
            = new HistogramDataSequence(xValuesY, true, nFrequencyType, fBinWidth, nBinCount);

        uno::Reference<chart2::data::XDataSequence> xCatDataSeq(xCatSeq);
        uno::Reference<beans::XPropertySet> xCatProp(xCatDataSeq, uno::UNO_QUERY);
        if (xCatProp.is())
        {
            xCatProp->setPropertyValue(u"Role"_ustr, uno::Any(u"categories"_ustr));
        }

        uno::Reference<chart2::data::XLabeledDataSequence> xLabeledCat
            = new LabeledDataSequence(xCatSeq);

        // 4. Attach the categories to the DataSeries so the View can find them
        bool bHasCategories = false;
        for (auto& seq : aSeqs)
        {
            if (seq.is() && DataSeriesHelper::getRole(seq) == "categories")
            {
                seq = xLabeledCat;
                bHasCategories = true;
                break;
            }
        }

        if (!bHasCategories)
        {
            aSeqs.push_back(xLabeledCat);
        }

        xSeries->setData(aSeqs);
    }
}

OUString SAL_CALL HistogramChartType::getChartType()
{
    return CHART2_SERVICE_NAME_CHARTTYPE_HISTOGRAM;
}

uno::Sequence<OUString> SAL_CALL HistogramChartType::getSupportedMandatoryRoles()
{
    return { u"label"_ustr, u"values-y"_ustr };
}

uno::Sequence<OUString> SAL_CALL HistogramChartType::getSupportedOptionalRoles() { return {}; }

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
        uno::Sequence<sal_Int32> aSeq{ 0 }; // No gap for histogram

        ::chart::PropertyHelper::setPropertyValueDefault(aTmp, PROP_HISTOGRAMCHARTTYPE_BINWIDTH,
                                                         2.0);
        ::chart::PropertyHelper::setPropertyValueDefault(aTmp, PROP_HISTOGRAMCHARTTYPE_BINCOUNT,
                                                         sal_Int32(10));
        ::chart::PropertyHelper::setPropertyValueDefault(
            aTmp, PROP_HISTOGRAMCHARTTYPE_FREQUENCYTYPE, sal_Int32(0));
        ::chart::PropertyHelper::setPropertyValueDefault(
            aTmp, PROP_HISTOGRAMCHARTTYPE_OVERLAP_SEQUENCE, aSeq);
        ::chart::PropertyHelper::setPropertyValueDefault(
            aTmp, PROP_HISTOGRAMCHARTTYPE_GAPWIDTH_SEQUENCE, aSeq);

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

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_chart_HistogramChartType_get_implementation(
    css::uno::XComponentContext* /*context*/, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ::chart::HistogramChartType());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */