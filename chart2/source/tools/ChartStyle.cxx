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
#include <iostream>
#include <ChartStyle.hxx>
#include <com/sun/star/chart2/ChartObjectType.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/instance.hxx>
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

namespace chart
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

ChartObjectStyle::ChartObjectStyle( const ChartObjectStyle & rOther )
    : OPropertySet(rOther.m_aMutex)
    , mrArrayHelper(rOther.mrArrayHelper)
    , mrPropertyMap(rOther.mrPropertyMap)
    , mxPropSetInfo(rOther.mxPropSetInfo)
{}

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

ChartStyle::ChartStyle( const ChartStyle & rOther )
    : m_nNumObjects(rOther.m_nNumObjects)
{
    for ( sal_Int16 nIdx = 0; nIdx < m_nNumObjects; nIdx++ )
    {
        auto It = rOther.m_xChartStyle.find( nIdx );

        if (It != rOther.m_xChartStyle.end())
            m_xChartStyle[ nIdx ] = It->second;
    }
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

    css::uno::Reference<css::beans::XPropertySet> xWallStyle
        = m_xChartStyle[css::chart2::ChartObjectType::WALL];

    xWallStyle->setPropertyValue("LineStyle", css::uno::Any(css::drawing::LineStyle_SOLID));
    xWallStyle->setPropertyValue("FillStyle", css::uno::Any(css::drawing::FillStyle_NONE));
    xWallStyle->setPropertyValue("LineColor",
                                 css::uno::Any(static_cast<sal_Int32>(0xb3b3b3))); // gray30
    xWallStyle->setPropertyValue("FillColor",
                                 css::uno::Any(static_cast<sal_Int32>(0xe6e6e6))); // gray10

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

void SAL_CALL ChartStyle::applyStyleToTitle(const css::uno::Reference<css::chart2::XTitle>& xTitle)
{
    css::uno::Reference<css::style::XStyleSupplier> xTitleStyle(xTitle, css::uno::UNO_QUERY);
    if (xTitleStyle.is())
    {
        xTitleStyle->setStyle(css::uno::Reference<css::style::XStyle>(
            m_xChartStyle.find(css::chart2::ChartObjectType::TITLE)->second,
            css::uno::UNO_QUERY_THROW));
    }
}

void ChartStyle::applyStyleToAxis(const css::uno::Reference<css::chart2::XAxis>& xAxis)
{
    css::uno::Reference<css::style::XStyleSupplier> xAxisStyle(xAxis, css::uno::UNO_QUERY);
    if (xAxisStyle.is())
    {
        xAxisStyle->setStyle(css::uno::Reference<css::style::XStyle>(
            m_xChartStyle.find(css::chart2::ChartObjectType::AXIS)->second,
            css::uno::UNO_QUERY_THROW));
    }

    css::uno::Reference<css::chart2::XTitled> xTitled(xAxis, css::uno::UNO_QUERY);
    if (xTitled.is())
    {
        css::uno::Reference<css::chart2::XTitle> xTitle = xTitled->getTitleObject();
        if (xTitle.is())
            applyStyleToTitle(xTitle);
    }
}

void ChartStyle::applyStyleToCoordinates(
    const css::uno::Reference<css::chart2::XCoordinateSystemContainer>& xCooSysCont)
{
    css::uno::Sequence<css::uno::Reference<css::chart2::XCoordinateSystem>> aCooSysSeq(
        xCooSysCont->getCoordinateSystems());

    for (sal_Int32 nCooSysIdx = 0; nCooSysIdx < aCooSysSeq.getLength(); ++nCooSysIdx)
    {
        css::uno::Reference<css::chart2::XCoordinateSystem> xCooSys = aCooSysSeq[nCooSysIdx];

        sal_Int16 nDimCount = xCooSys->getDimension();
        for (sal_Int16 nDimIdx = 0; nDimIdx < nDimCount; nDimIdx++)
        {
            applyStyleToAxis(xCooSys->getAxisByDimension(nDimIdx, 0));
            if (xCooSys->getMaximumAxisIndexByDimension(nDimIdx))
                applyStyleToAxis(xCooSys->getAxisByDimension(nDimIdx, 1));
        }
    }
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

    css::uno::Reference<css::style::XStyleSupplier> xWallStyle(xDiagram->getWall(),
                                                               css::uno::UNO_QUERY);
    if (xWallStyle.is())
    {
        xWallStyle->setStyle(css::uno::Reference<css::style::XStyle>(
            m_xChartStyle.find(css::chart2::ChartObjectType::WALL)->second,
            css::uno::UNO_QUERY_THROW));
    }

    css::uno::Reference<css::chart2::XTitled> xTitled(xDiagram, css::uno::UNO_QUERY);
    if (xTitled.is())
    {
        css::uno::Reference<css::chart2::XTitle> xTitle = xTitled->getTitleObject();
        if (xTitle.is())
            applyStyleToTitle(xTitle);
    }

    css::uno::Reference<css::chart2::XCoordinateSystemContainer> xCooSysCont(xDiagram,
                                                                             css::uno::UNO_QUERY);
    if (xCooSysCont.is())
        applyStyleToCoordinates(xCooSysCont);
}

void SAL_CALL
ChartStyle::applyStyleToBackground(const css::uno::Reference<css::beans::XPropertySet>& xBackground)
{
    css::uno::Reference<css::style::XStyleSupplier> xBackgroundStyle(xBackground,
                                                                     css::uno::UNO_QUERY);
    if (xBackgroundStyle.is())
    {
        xBackgroundStyle->setStyle(css::uno::Reference<css::style::XStyle>(
            m_xChartStyle.find(css::chart2::ChartObjectType::PAGE)->second,
            css::uno::UNO_QUERY_THROW));
    }
}

void SAL_CALL
ChartStyle::updateStyleElement(const sal_Int16 nChartObjectType, const css::uno::Sequence<css::beans::PropertyValue>& rProperties)
{
    css::uno::Reference<css::beans::XPropertyAccess>
        xPropertyAccess(getStyleForObject(nChartObjectType), css::uno::UNO_QUERY_THROW);

    if (xPropertyAccess.is())
    {
        xPropertyAccess->setPropertyValues( rProperties );
    }
}

void SAL_CALL
ChartStyle::updateChartStyle(const css::uno::Reference<css::chart2::XChartDocument>& rxModel)
{
    if (!rxModel.is())
        return;

    css::uno::Reference<css::beans::XPropertyAccess> xPropAccess;
    css::uno::Reference<css::chart2::XTitled> xMainTitled(rxModel, css::uno::UNO_QUERY);

    css::uno::Reference<css::chart2::XTitle> xMainTitle = xMainTitled->getTitleObject();
    if (xMainTitle.is())
    {
        xPropAccess = css::uno::Reference<css::beans::XPropertyAccess>(xMainTitle, css::uno::UNO_QUERY);
        updateStyleElement(css::chart2::ChartObjectType::TITLE, xPropAccess->getPropertyValues());
    }

    xPropAccess = css::uno::Reference<css::beans::XPropertyAccess>(rxModel->getPageBackground(), css::uno::UNO_QUERY);
    if (xPropAccess.is())
        updateStyleElement(css::chart2::ChartObjectType::PAGE, xPropAccess->getPropertyValues());

    xPropAccess = css::uno::Reference<css::beans::XPropertyAccess>(rxModel->getFirstDiagram(), css::uno::UNO_QUERY);
    if (xPropAccess.is())
        updateStyleElement(css::chart2::ChartObjectType::DIAGRAM, xPropAccess->getPropertyValues());

    css::uno::Reference<css::chart2::XDiagram> xDiagram = rxModel->getFirstDiagram();

    xPropAccess = css::uno::Reference<css::beans::XPropertyAccess>( xDiagram->getWall(), css::uno::UNO_QUERY);
    if (xPropAccess.is())
        updateStyleElement(css::chart2::ChartObjectType::WALL, xPropAccess->getPropertyValues());

    xPropAccess = css::uno::Reference<css::beans::XPropertyAccess>( xDiagram->getLegend(), css::uno::UNO_QUERY);
    if (xPropAccess.is())
        updateStyleElement(css::chart2::ChartObjectType::LEGEND, xPropAccess->getPropertyValues());

    css::uno::Reference<css::chart2::XTitled> xTitled( xDiagram, css::uno::UNO_QUERY);
    css::uno::Reference<css::chart2::XTitle> xSubTitle = xTitled->getTitleObject();
    if (xSubTitle.is())
    {
        xPropAccess = css::uno::Reference<css::beans::XPropertyAccess>( xSubTitle, css::uno::UNO_QUERY);
        if (xPropAccess.is())
            updateStyleElement(css::chart2::ChartObjectType::TITLE, xPropAccess->getPropertyValues());
    }

    css::uno::Reference<css::chart2::XCoordinateSystemContainer> xCooSysCont( xDiagram, css::uno::UNO_QUERY);

    css::uno::Sequence<css::uno::Reference<css::chart2::XCoordinateSystem>> aCooSysSeq(
        xCooSysCont->getCoordinateSystems());

    for (sal_Int32 nCooSysIdx = 0; nCooSysIdx < aCooSysSeq.getLength(); ++nCooSysIdx)
    {
        css::uno::Reference<css::chart2::XCoordinateSystem> xCooSys = aCooSysSeq[nCooSysIdx];

        sal_Int16 nDimCount = xCooSys->getDimension();
        for (sal_Int16 nDimIdx = 0; nDimIdx < nDimCount; nDimIdx++)
        {
            css::uno::Reference<css::chart2::XAxis> xAxis = xCooSys->getAxisByDimension(nDimIdx, 0);
            xPropAccess = css::uno::Reference<css::beans::XPropertyAccess>( xAxis, css::uno::UNO_QUERY);
            if (xPropAccess.is())
                updateStyleElement(css::chart2::ChartObjectType::AXIS, xPropAccess->getPropertyValues());

            if (xCooSys->getMaximumAxisIndexByDimension(nDimIdx))
            {
                xAxis = xCooSys->getAxisByDimension(nDimIdx, 1);
                xPropAccess = css::uno::Reference<css::beans::XPropertyAccess>( xAxis, css::uno::UNO_QUERY);
                if (xPropAccess.is())
                    updateStyleElement(css::chart2::ChartObjectType::AXIS, xPropAccess->getPropertyValues());

            }
        }
    }
}

sal_Bool ChartStyle::isUserDefined() { return false; }

sal_Bool ChartStyle::isInUse() { return true; }

OUString ChartStyle::getParentStyle() { return ""; }

void ChartStyle::setParentStyle(const OUString&) {}

void ChartStyle::setName(const OUString& rName) { maName = rName; }

OUString ChartStyle::getName() { return maName; }

class ChartStyles : public cppu::WeakImplHelper<css::container::XNameContainer>
{
public:
    ChartStyles();
    virtual ~ChartStyles();

    // XNameContainer
    virtual void SAL_CALL insertByName(const OUString& rName, const css::uno::Any& rStyle);
    virtual void SAL_CALL removeByName(const OUString& rName);
    virtual void SAL_CALL replaceByName(const OUString& rName, const css::uno::Any& rStyle);

    virtual css::uno::Any SAL_CALL getByName(const OUString& rName);
    virtual css::uno::Sequence<OUString> SAL_CALL getElementNames();
    virtual sal_Bool SAL_CALL hasByName(const OUString& rName);
    virtual sal_Bool SAL_CALL hasElements();
    virtual css::uno::Type SAL_CALL getElementType();

private:
    void addInitialStyles();

    std::map<OUString, css::uno::Reference<css::chart2::XChartStyle>> maChartStyles;
};

ChartStyles::ChartStyles() { addInitialStyles(); }

ChartStyles::~ChartStyles() {}

void ChartStyles::addInitialStyles()
{
    css::uno::Any aDefaultStyle;
    css::uno::Reference<css::chart2::XChartStyle> xChartStyle = new ChartStyle;
    css::uno::Reference<css::style::XStyle> xStyle(xChartStyle, css::uno::UNO_QUERY_THROW);
    xStyle->setName("LibreOffice");

    aDefaultStyle <<= xChartStyle;
    insertByName("LibreOffice", aDefaultStyle);
    insertByName("Default", aDefaultStyle);

}

void ChartStyles::insertByName(const OUString& rName, const css::uno::Any& rStyle)
{
    css::uno::Reference<css::chart2::XChartStyle> xChartStyle;
    if (!(rStyle >>= xChartStyle))
        throw css::lang::IllegalArgumentException();

    maChartStyles[rName] = xChartStyle;
}

void ChartStyles::removeByName(const OUString& rName)
{
    auto itr = maChartStyles.find(rName);
    if (itr == maChartStyles.end())
        throw css::lang::IllegalArgumentException();

    maChartStyles.erase(itr);
}

void ChartStyles::replaceByName(const OUString& rName, const css::uno::Any& rStyle)
{
    css::uno::Reference<css::chart2::XChartStyle> xChartStyle;
    if (!(rStyle >>= xChartStyle))
        throw css::lang::IllegalArgumentException();

    auto itr = maChartStyles.find(rName);
    if (itr == maChartStyles.end())
        throw css::lang::IllegalArgumentException();

    maChartStyles[rName] = xChartStyle;
}

css::uno::Any ChartStyles::getByName(const OUString& rName)
{
    auto itr = maChartStyles.find(rName);
    if (itr == maChartStyles.end())
        throw css::lang::IllegalArgumentException();

    css::uno::Any aRet;
    aRet <<= itr->second;

    return aRet;
}

css::uno::Sequence<OUString> ChartStyles::getElementNames()
{
    return comphelper::mapKeysToSequence(maChartStyles);
}

sal_Bool ChartStyles::hasByName(const OUString& rName)
{
    auto itr = maChartStyles.find(rName);
    if (itr == maChartStyles.end())
        return false;

    return true;
}

sal_Bool ChartStyles::hasElements() { return !maChartStyles.empty(); }

css::uno::Type ChartStyles::getElementType() { return css::uno::Type(); }

//
// needed by MSC compiler
using impl::ChartObjectStyle_Base;

IMPLEMENT_FORWARD_XINTERFACE2(ChartObjectStyle, ChartObjectStyle_Base, ::property::OPropertySet)

namespace
{
struct theChartStyles
    : public rtl::StaticWithInit<css::uno::Reference<css::container::XNameContainer>,
                                 theChartStyles>
{
    css::uno::Reference<css::container::XNameContainer> operator()() { return new ChartStyles; }
};
}

css::uno::Reference<css::container::XNameContainer> getChartStyles()
{
    return theChartStyles::get();
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
