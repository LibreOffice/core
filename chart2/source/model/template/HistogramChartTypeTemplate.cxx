/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "HistogramChartTypeTemplate.hxx"
#include "HistogramChartType.hxx"
#include "HistogramDataInterpreter.hxx"
#include <Diagram.hxx>
#include <DataSeries.hxx>
#include <DataSeriesHelper.hxx>
#include <PropertyHelper.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/chart2/DataPointGeometry3D.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/diagnose_ex.hxx>

#include <algorithm>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::Property;

namespace
{
enum
{
    PROP_HISTOGRAM_TEMPLATE_DIMENSION,
};

void lcl_AddPropertiesToVector(std::vector<Property>& rOutProperties)
{
    rOutProperties.emplace_back(
        "Dimension", PROP_HISTOGRAM_TEMPLATE_DIMENSION, cppu::UnoType<sal_Int32>::get(),
        beans::PropertyAttribute::BOUND | beans::PropertyAttribute::MAYBEDEFAULT);
}

::cppu::OPropertyArrayHelper& StaticHistogramChartTypeTemplateInfoHelper()
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
HistogramChartTypeTemplate::HistogramChartTypeTemplate(
    Reference<uno::XComponentContext> const& xContext, const OUString& rServiceName,
    StackMode eStackMode)
    : ChartTypeTemplate(xContext, rServiceName)
    , m_eStackMode(eStackMode)
    , m_nDim(2)
{
}

sal_Int32 HistogramChartTypeTemplate::getDimension() const { return m_nDim; }

StackMode HistogramChartTypeTemplate::getStackMode(sal_Int32 /* nChartTypeIndex */) const
{
    return m_eStackMode;
}

rtl::Reference<ChartType>
    HistogramChartTypeTemplate::getChartTypeForIndex(sal_Int32 /*nChartTypeIndex*/)
{
    return new HistogramChartType();
}

rtl::Reference<ChartType> HistogramChartTypeTemplate::getChartTypeForNewSeries2(
    const std::vector<rtl::Reference<ChartType>>& aFormerlyUsedChartTypes)
{
    rtl::Reference<ChartType> xResult(getChartTypeForIndex(0));
    ChartTypeTemplate::copyPropertiesFromOldToNewCoordinateSystem(aFormerlyUsedChartTypes, xResult);
    return xResult;
}

// ____ OPropertySet ____
void HistogramChartTypeTemplate::GetDefaultValue(sal_Int32 nHandle, uno::Any& rAny) const
{
    static ::chart::tPropertyValueMap aStaticDefaults = []() {
        ::chart::tPropertyValueMap aTmp;
        ::chart::PropertyHelper::setPropertyValueDefault<sal_Int32>(
            aTmp, PROP_HISTOGRAM_TEMPLATE_DIMENSION, 2);
        return aTmp;
    }();
    tPropertyValueMap::const_iterator aFound(aStaticDefaults.find(nHandle));
    if (aFound == aStaticDefaults.end())
        rAny.clear();
    else
        rAny = (*aFound).second;
}

::cppu::IPropertyArrayHelper& SAL_CALL HistogramChartTypeTemplate::getInfoHelper()
{
    return StaticHistogramChartTypeTemplateInfoHelper();
}

// ____ XPropertySet ____
Reference<beans::XPropertySetInfo> SAL_CALL HistogramChartTypeTemplate::getPropertySetInfo()
{
    static uno::Reference<beans::XPropertySetInfo> xPropertySetInfo(
        ::cppu::OPropertySetHelper::createPropertySetInfo(
            StaticHistogramChartTypeTemplateInfoHelper()));
    return xPropertySetInfo;
}

void HistogramChartTypeTemplate::applyStyle2(const rtl::Reference<DataSeries>& xSeries,
                                             ::sal_Int32 nChartTypeIndex, ::sal_Int32 nSeriesIndex,
                                             ::sal_Int32 nSeriesCount)
{
    ChartTypeTemplate::applyStyle2(xSeries, nChartTypeIndex, nSeriesIndex, nSeriesCount);
    xSeries->setPropertyAlsoToAllAttributedDataPoints("BorderStyle",
                                                      uno::Any(drawing::LineStyle_NONE));
}

void HistogramChartTypeTemplate::resetStyles2(const rtl::Reference<::chart::Diagram>& xDiagram)
{
    ChartTypeTemplate::resetStyles2(xDiagram);
    std::vector<rtl::Reference<DataSeries>> aSeriesVec(xDiagram->getDataSeries());
    uno::Any aLineStyleAny(drawing::LineStyle_NONE);
    for (auto const& series : aSeriesVec)
    {
        if (series->getPropertyValue("BorderStyle") == aLineStyleAny)
        {
            series->setPropertyToDefault("BorderStyle");
        }
    }

    xDiagram->setVertical(false);
}

rtl::Reference<DataInterpreter> HistogramChartTypeTemplate::getDataInterpreter2()
{
    if (!m_xDataInterpreter.is())
        m_xDataInterpreter.set(new HistogramDataInterpreter);

    return m_xDataInterpreter;
}

IMPLEMENT_FORWARD_XINTERFACE2(HistogramChartTypeTemplate, ChartTypeTemplate, OPropertySet)
IMPLEMENT_FORWARD_XTYPEPROVIDER2(HistogramChartTypeTemplate, ChartTypeTemplate, OPropertySet)

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
