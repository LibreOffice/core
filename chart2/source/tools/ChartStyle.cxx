/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <ChartStyle.hxx>
#include <com/sun/star/chart2/ChartObjectType.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <vector>

#include <Legend.hxx>
#include <Axis.hxx>
#include <DataPoint.hxx>
#include <DataSeries.hxx>
#include <Diagram.hxx>
#include <GridProperties.hxx>
#include <PageBackground.hxx>
#include <StockBar.hxx>
#include <Title.hxx>
#include <Wall.hxx>

namespace com
{
namespace sun
{
namespace star
{
namespace uno
{
class XComponentContext;
}
}
}
}

namespace chart2
{
ChartObjectStyle::ChartObjectStyle(css::uno::Reference<css::beans::XPropertySetInfo> xPropSetInfo,
                                   ::cppu::IPropertyArrayHelper& rArrayHelper,
                                   const chart::tPropertyValueMap& rPropertyMap)
    : OPropertySet(m_aMutex)
    , mrArrayHelper(rArrayHelper)
    , mrPropertyMap(rPropertyMap)
    , mxPropSetInfo(xPropSetInfo)
{
}

ChartObjectStyle::~ChartObjectStyle() {}

sal_Bool SAL_CALL ChartObjectStyle::isInUse() { return true; }

sal_Bool SAL_CALL ChartObjectStyle::isUserDefined() { return true; }

OUString SAL_CALL ChartObjectStyle::getParentStyle() { return ""; }

void SAL_CALL ChartObjectStyle::setParentStyle(const OUString&) {}

css::uno::Reference<css::beans::XPropertySetInfo> SAL_CALL ChartObjectStyle::getPropertySetInfo()
{
    return mxPropSetInfo;
}

OUString SAL_CALL ChartObjectStyle::getName() { return OUString(); }

void SAL_CALL ChartObjectStyle::setName(const OUString&) {}

// ____ OPropertySet ____
css::uno::Any ChartObjectStyle::GetDefaultValue(sal_Int32 nHandle) const
{
    chart::tPropertyValueMap::const_iterator aFound(mrPropertyMap.find(nHandle));
    if (aFound == mrPropertyMap.end())
        return css::uno::Any();
    return (*aFound).second;
}

::cppu::IPropertyArrayHelper& SAL_CALL ChartObjectStyle::getInfoHelper() { return mrArrayHelper; }

ChartStyle::ChartStyle()
    : m_nNumObjects(css::chart2::ChartObjectType::UNKNOWN)
{
    register_styles();
}

ChartStyle::~ChartStyle() {}

css::uno::Reference<css::beans::XPropertySet>
ChartStyle::getStyleForObject(const sal_Int16 nChartObjectType)
{
    auto itr = m_xChartStyle.find(nChartObjectType);
    if (itr == m_xChartStyle.end())
        throw css::lang::IllegalArgumentException("Unknown Chart Object Style requested", nullptr,
                                                  0);

    return itr->second;
}

void ChartStyle::register_styles()
{
    m_xChartStyle[css::chart2::ChartObjectType::LEGEND] = new ChartObjectStyle(
        *chart::legend::StaticLegendInfo::get(), *chart::legend::StaticLegendInfoHelper::get(),
        *chart::legend::StaticLegendDefaults::get());

    css::uno::Reference<css::beans::XPropertySet> xLegendStyle
        = m_xChartStyle[css::chart2::ChartObjectType::LEGEND];
    xLegendStyle->setPropertyValue("FillStyle", css::uno::Any(css::drawing::FillStyle_NONE));
    xLegendStyle->setPropertyValue("LineStyle", css::uno::Any(css::drawing::LineStyle_NONE));
    xLegendStyle->setPropertyValue("LineColor",
                                   css::uno::Any(static_cast<sal_Int32>(0xb3b3b3))); // gray30
    xLegendStyle->setPropertyValue("FillColor",
                                   css::uno::Any(static_cast<sal_Int32>(0xe6e6e6))); // gray10

    m_xChartStyle[css::chart2::ChartObjectType::AXIS] = new ChartObjectStyle(
        *chart::axis::StaticAxisInfo::get(), *chart::axis::StaticAxisInfoHelper::get(),
        *chart::axis::StaticAxisDefaults::get());

    m_xChartStyle[css::chart2::ChartObjectType::DATA_SERIES]
        = new ChartObjectStyle(*chart::dataSeries::StaticDataSeriesInfo::get(),
                               *chart::dataSeries::StaticDataSeriesInfoHelper::get(),
                               *chart::dataSeries::StaticDataSeriesDefaults::get());

    m_xChartStyle[css::chart2::ChartObjectType::TITLE] = new ChartObjectStyle(
        *chart::title::StaticTitleInfo::get(), *chart::title::StaticTitleInfoHelper::get(),
        *chart::title::StaticTitleDefaults::get());

    m_xChartStyle[css::chart2::ChartObjectType::WALL] = new ChartObjectStyle(
        *chart::wall::StaticWallInfo::get(), *chart::wall::StaticWallInfoHelper::get(),
        *chart::wall::StaticWallDefaults::get());

    m_xChartStyle[css::chart2::ChartObjectType::GRID] = new ChartObjectStyle(
        *chart::grid::StaticGridInfo::get(), *chart::grid::StaticGridInfoHelper::get(),
        *chart::grid::StaticGridDefaults::get());

    m_xChartStyle[css::chart2::ChartObjectType::PAGE]
        = new ChartObjectStyle(*chart::page::StaticPageBackgroundInfo::get(),
                               *chart::page::StaticPageBackgroundInfoHelper::get(),
                               *chart::page::StaticPageBackgroundDefaults::get());

    m_xChartStyle[css::chart2::ChartObjectType::DIAGRAM] = new ChartObjectStyle(
        *chart::diagram::StaticDiagramInfo::get(), *chart::diagram::StaticDiagramInfoHelper::get(),
        *chart::diagram::StaticDiagramDefaults::get());

    m_xChartStyle[css::chart2::ChartObjectType::STOCK] = new ChartObjectStyle(
        *chart::stock::StaticStockBarInfo::get(), *chart::stock::StaticStockBarInfoHelper::get(),
        *chart::stock::StaticStockBarDefaults::get());
}

// _____ XServiceInfo _____
OUString SAL_CALL ChartStyle::getImplementationName()
{
    return OUString("com.sun.star.comp.chart2.ChartStyle");
}

sal_Bool SAL_CALL ChartStyle::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> SAL_CALL ChartStyle::getSupportedServiceNames()
{
    return { "com.sun.star.chart2.ChartStyle" };
}

void SAL_CALL
ChartStyle::applyStyleToDiagram(const css::uno::Reference<css::chart2::XDiagram>& xDiagram)
{
    css::uno::Reference<css::style::XStyleSupplier> xLegendStyle(xDiagram->getLegend(),
                                                                 css::uno::UNO_QUERY);
    if (xLegendStyle.is())
    {
        xLegendStyle->setStyle(css::uno::Reference<css::style::XStyle>(
            m_xChartStyle.find(css::chart2::ChartObjectType::LEGEND)->second,
            css::uno::UNO_QUERY_THROW));
    }
}

//
// needed by MSC compiler
using impl::ChartObjectStyle_Base;

IMPLEMENT_FORWARD_XINTERFACE2(ChartObjectStyle, ChartObjectStyle_Base, ::property::OPropertySet)

} //  namespace chart2

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_chart2_ChartStyle_get_implementation(css::uno::XComponentContext*,
                                                       css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new chart2::ChartStyle);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
