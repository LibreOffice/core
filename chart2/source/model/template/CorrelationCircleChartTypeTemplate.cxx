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

#include "CorrelationCircleChartTypeTemplate.hxx"
#include "CorrelationCircleChartType.hxx"
#include "CorrelationCircleDataInterpreter.hxx"
#include <Axis.hxx>
#include <AxisHelper.hxx>
#include <AxisIndexDefines.hxx>
#include <BaseCoordinateSystem.hxx>
#include <DataSeries.hxx>
#include <PropertyHelper.hxx>
#include <unonames.hxx>

#include <com/sun/star/chart2/DataPointLabel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/diagnose_ex.hxx>

#include <algorithm>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;

namespace
{
::cppu::OPropertyArrayHelper& StaticCorrelationCircleChartTypeTemplateInfoHelper()
{
    static ::cppu::OPropertyArrayHelper aPropHelper = []() {
        std::vector<css::beans::Property> aProperties;
        std::sort(aProperties.begin(), aProperties.end(), ::chart::PropertyNameLess());
        return comphelper::containerToSequence(aProperties);
    }();
    return aPropHelper;
};

} // anonymous namespace

namespace chart
{
CorrelationCircleChartTypeTemplate::CorrelationCircleChartTypeTemplate(
    Reference<uno::XComponentContext> const& xContext, const OUString& rServiceName)
    : ChartTypeTemplate(xContext, rServiceName)
{
}

CorrelationCircleChartTypeTemplate::~CorrelationCircleChartTypeTemplate() {}

// ____ OPropertySet ____
void CorrelationCircleChartTypeTemplate::GetDefaultValue(sal_Int32 nHandle,
                                                         css::uno::Any& rAny) const
{
    static ::chart::tPropertyValueMap aStaticDefaults;
    tPropertyValueMap::const_iterator aFound(aStaticDefaults.find(nHandle));
    if (aFound == aStaticDefaults.end())
        rAny.clear();
    else
        rAny = (*aFound).second;
}

::cppu::IPropertyArrayHelper& CorrelationCircleChartTypeTemplate::getInfoHelper()
{
    return StaticCorrelationCircleChartTypeTemplateInfoHelper();
}

// ____ XPropertySet ____
uno::Reference<beans::XPropertySetInfo> CorrelationCircleChartTypeTemplate::getPropertySetInfo()
{
    static const uno::Reference<beans::XPropertySetInfo> xPropertySetInfo(
        ::cppu::OPropertySetHelper::createPropertySetInfo(
            StaticCorrelationCircleChartTypeTemplateInfoHelper()));
    return xPropertySetInfo;
}

sal_Int32 CorrelationCircleChartTypeTemplate::getDimension() const { return 2; }

StackMode CorrelationCircleChartTypeTemplate::getStackMode(sal_Int32 /* nChartTypeIndex */) const
{
    return StackMode::NONE;
}

void CorrelationCircleChartTypeTemplate::adaptAxes(
    const std::vector<rtl::Reference<BaseCoordinateSystem>>& rCoordSys)
{
    ChartTypeTemplate::adaptAxes(rCoordSys);

    // The circle and the two lines through the origin are the frame the arrows
    // are read against, so the plot starts with neither grid.
    for (rtl::Reference<BaseCoordinateSystem> const& xCooSys : rCoordSys)
    {
        if (!xCooSys.is())
            continue;

        for (sal_Int32 nDimension = 0; nDimension < xCooSys->getDimension(); ++nDimension)
        {
            rtl::Reference<Axis> xAxis = AxisHelper::getAxis(nDimension, MAIN_AXIS_INDEX, xCooSys);
            if (xAxis.is())
                AxisHelper::makeGridInvisible(xAxis->getGridProperties2());
        }
    }
}

sal_Bool CorrelationCircleChartTypeTemplate::supportsCategories() { return false; }

void CorrelationCircleChartTypeTemplate::applyStyle2(const rtl::Reference<DataSeries>& xSeries,
                                                     ::sal_Int32 nChartTypeIndex,
                                                     ::sal_Int32 nSeriesIndex,
                                                     ::sal_Int32 nSeriesCount)
{
    ChartTypeTemplate::applyStyle2(xSeries, nChartTypeIndex, nSeriesIndex, nSeriesCount);

    try
    {
        // The line is the arrow from the origin and the symbol marks its tip. A
        // thin line keeps a crowd of arrows apart.
        xSeries->switchSymbolsOnOrOff(true, nSeriesIndex);
        xSeries->switchLinesOnOrOff(true);
        xSeries->makeLinesThickOrThin(false);

        // One point stands for a whole feature, so its name says more than the
        // number behind it.
        chart2::DataPointLabel aLabel(false, false, false, false, false, true);
        xSeries->setPropertyAlsoToAllAttributedDataPoints(CHART_UNONAME_LABEL,
                                                          css::uno::Any(aLabel));
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

rtl::Reference<ChartType>
    CorrelationCircleChartTypeTemplate::getChartTypeForIndex(sal_Int32 /*nChartTypeIndex*/)
{
    return new CorrelationCircleChartType();
}

rtl::Reference<ChartType> CorrelationCircleChartTypeTemplate::getChartTypeForNewSeries2(
    const std::vector<rtl::Reference<ChartType>>& aFormerlyUsedChartTypes)
{
    rtl::Reference<ChartType> xResult(getChartTypeForIndex(0));
    ChartTypeTemplate::copyPropertiesFromOldToNewCoordinateSystem(aFormerlyUsedChartTypes, xResult);
    return xResult;
}

rtl::Reference<DataInterpreter> CorrelationCircleChartTypeTemplate::getDataInterpreter2()
{
    if (!m_xDataInterpreter.is())
        m_xDataInterpreter.set(new CorrelationCircleDataInterpreter);

    return m_xDataInterpreter;
}

IMPLEMENT_FORWARD_XINTERFACE2(CorrelationCircleChartTypeTemplate, ChartTypeTemplate, OPropertySet)
IMPLEMENT_FORWARD_XTYPEPROVIDER2(CorrelationCircleChartTypeTemplate, ChartTypeTemplate,
                                 OPropertySet)

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
