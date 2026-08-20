/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <ChartTypeManager.hxx>
#include <StackMode.hxx>

#include "LineChartTypeTemplate.hxx"
#include "BarChartTypeTemplate.hxx"
#include "ColumnLineChartTypeTemplate.hxx"
#include "HistogramChartTypeTemplate.hxx"
#include "AreaChartTypeTemplate.hxx"
#include "PieChartTypeTemplate.hxx"
#include "ScatterChartTypeTemplate.hxx"
#include "StockChartTypeTemplate.hxx"
#include "NetChartTypeTemplate.hxx"
#include "BubbleChartTypeTemplate.hxx"
#include "CorrelationCircleChartTypeTemplate.hxx"
#include "FunnelChartTypeTemplate.hxx"
#include "BoxWhiskerChartTypeTemplate.hxx"
#include "ClusteredColumnChartTypeTemplate.hxx"
#include "ParetoLineChartTypeTemplate.hxx"
#include "RegionMapChartTypeTemplate.hxx"
#include "SunburstChartTypeTemplate.hxx"
#include "TreemapChartTypeTemplate.hxx"
#include "WaterfallChartTypeTemplate.hxx"
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <sal/log.hxx>

#include <algorithm>
#include <iterator>
#include <o3tl/functional.hxx>
#include <map>
#include <utility>

using namespace ::com::sun::star;

using ::cpo::uno::Sequence;

namespace
{

enum TemplateId
{
    TEMPLATE_SYMBOL,
    TEMPLATE_STACKEDSYMBOL,
    TEMPLATE_PERCENTSTACKEDSYMBOL,
    TEMPLATE_LINE,
    TEMPLATE_STACKEDLINE,
    TEMPLATE_PERCENTSTACKEDLINE,
    TEMPLATE_LINESYMBOL,
    TEMPLATE_STACKEDLINESYMBOL,
    TEMPLATE_PERCENTSTACKEDLINESYMBOL,
    TEMPLATE_THREEDLINE,
    TEMPLATE_STACKEDTHREEDLINE,
    TEMPLATE_PERCENTSTACKEDTHREEDLINE,
    TEMPLATE_THREEDLINEDEEP,
    TEMPLATE_COLUMN,
    TEMPLATE_STACKEDCOLUMN,
    TEMPLATE_PERCENTSTACKEDCOLUMN,
    TEMPLATE_BAR,
    TEMPLATE_STACKEDBAR,
    TEMPLATE_PERCENTSTACKEDBAR,
    TEMPLATE_THREEDCOLUMNDEEP,
    TEMPLATE_THREEDCOLUMNFLAT,
    TEMPLATE_STACKEDTHREEDCOLUMNFLAT,
    TEMPLATE_PERCENTSTACKEDTHREEDCOLUMNFLAT,
    TEMPLATE_THREEDBARDEEP,
    TEMPLATE_THREEDBARFLAT,
    TEMPLATE_STACKEDTHREEDBARFLAT,
    TEMPLATE_PERCENTSTACKEDTHREEDBARFLAT,
    TEMPLATE_COLUMNWITHLINE,
    TEMPLATE_STACKEDCOLUMNWITHLINE,
    TEMPLATE_HISTOGRAM,
    TEMPLATE_AREA,
    TEMPLATE_STACKEDAREA,
    TEMPLATE_PERCENTSTACKEDAREA,
    TEMPLATE_THREEDAREA,
    TEMPLATE_STACKEDTHREEDAREA,
    TEMPLATE_PERCENTSTACKEDTHREEDAREA,
    TEMPLATE_PIE,
    TEMPLATE_PIEALLEXPLODED,
    TEMPLATE_DONUT,
    TEMPLATE_DONUTALLEXPLODED,
    TEMPLATE_BAROFPIE,
    TEMPLATE_PIEOFPIE,
    TEMPLATE_THREEDPIE,
    TEMPLATE_THREEDPIEALLEXPLODED,
    TEMPLATE_THREEDDONUT,
    TEMPLATE_THREEDDONUTALLEXPLODED,
    TEMPLATE_SCATTERLINESYMBOL,
    TEMPLATE_SCATTERLINE,
    TEMPLATE_SCATTERSYMBOL,
    TEMPLATE_THREEDSCATTER,
    TEMPLATE_NET,
    TEMPLATE_NETSYMBOL,
    TEMPLATE_NETLINE,
    TEMPLATE_STACKEDNET,
    TEMPLATE_STACKEDNETSYMBOL,
    TEMPLATE_STACKEDNETLINE,
    TEMPLATE_PERCENTSTACKEDNET,
    TEMPLATE_PERCENTSTACKEDNETSYMBOL,
    TEMPLATE_PERCENTSTACKEDNETLINE,
    TEMPLATE_FILLEDNET,
    TEMPLATE_STACKEDFILLEDNET,
    TEMPLATE_PERCENTSTACKEDFILLEDNET,
    TEMPLATE_STOCKLOWHIGHCLOSE,
    TEMPLATE_STOCKOPENLOWHIGHCLOSE,
    TEMPLATE_STOCKVOLUMELOWHIGHCLOSE,
    TEMPLATE_STOCKVOLUMEOPENLOWHIGHCLOSE,
    TEMPLATE_BUBBLE,
    TEMPLATE_CORRELATIONCIRCLE,
    TEMPLATE_FUNNEL,
    TEMPLATE_BOXWHISKER,
    TEMPLATE_CLUSTEREDCOLUMN,
    TEMPLATE_PARETOLINE,
    TEMPLATE_REGIONMAP,
    TEMPLATE_SUNBURST,
    TEMPLATE_TREEMAP,
    TEMPLATE_WATERFALL,
//    TEMPLATE_SURFACE,
//     TEMPLATE_ADDIN,
    TEMPLATE_NOT_FOUND = 0xffff
};

typedef std::map< OUString, TemplateId > tTemplateMapType;

const tTemplateMapType & lcl_DefaultChartTypeMap()
{
    static const tTemplateMapType aMap{
        {u"com.sun.star.chart2.template.Symbol"_ustr,                         TEMPLATE_SYMBOL},
        {u"com.sun.star.chart2.template.StackedSymbol"_ustr,                  TEMPLATE_STACKEDSYMBOL},
        {u"com.sun.star.chart2.template.PercentStackedSymbol"_ustr,           TEMPLATE_PERCENTSTACKEDSYMBOL},
        {u"com.sun.star.chart2.template.Line"_ustr,                           TEMPLATE_LINE},
        {u"com.sun.star.chart2.template.StackedLine"_ustr,                    TEMPLATE_STACKEDLINE},
        {u"com.sun.star.chart2.template.PercentStackedLine"_ustr,             TEMPLATE_PERCENTSTACKEDLINE},
        {u"com.sun.star.chart2.template.LineSymbol"_ustr,                     TEMPLATE_LINESYMBOL},
        {u"com.sun.star.chart2.template.StackedLineSymbol"_ustr,              TEMPLATE_STACKEDLINESYMBOL},
        {u"com.sun.star.chart2.template.PercentStackedLineSymbol"_ustr,       TEMPLATE_PERCENTSTACKEDLINESYMBOL},
        {u"com.sun.star.chart2.template.ThreeDLine"_ustr,                     TEMPLATE_THREEDLINE},
        {u"com.sun.star.chart2.template.StackedThreeDLine"_ustr,              TEMPLATE_STACKEDTHREEDLINE},
        {u"com.sun.star.chart2.template.PercentStackedThreeDLine"_ustr,       TEMPLATE_PERCENTSTACKEDTHREEDLINE},
        {u"com.sun.star.chart2.template.ThreeDLineDeep"_ustr,                 TEMPLATE_THREEDLINEDEEP},
        {u"com.sun.star.chart2.template.Column"_ustr,                         TEMPLATE_COLUMN},
        {u"com.sun.star.chart2.template.StackedColumn"_ustr,                  TEMPLATE_STACKEDCOLUMN},
        {u"com.sun.star.chart2.template.PercentStackedColumn"_ustr,           TEMPLATE_PERCENTSTACKEDCOLUMN},
        {u"com.sun.star.chart2.template.Bar"_ustr,                            TEMPLATE_BAR},
        {u"com.sun.star.chart2.template.StackedBar"_ustr,                     TEMPLATE_STACKEDBAR},
        {u"com.sun.star.chart2.template.PercentStackedBar"_ustr,              TEMPLATE_PERCENTSTACKEDBAR},
        {u"com.sun.star.chart2.template.ThreeDColumnDeep"_ustr,               TEMPLATE_THREEDCOLUMNDEEP},
        {u"com.sun.star.chart2.template.ThreeDColumnFlat"_ustr,               TEMPLATE_THREEDCOLUMNFLAT},
        {u"com.sun.star.chart2.template.StackedThreeDColumnFlat"_ustr,        TEMPLATE_STACKEDTHREEDCOLUMNFLAT},
        {u"com.sun.star.chart2.template.PercentStackedThreeDColumnFlat"_ustr, TEMPLATE_PERCENTSTACKEDTHREEDCOLUMNFLAT},
        {u"com.sun.star.chart2.template.ThreeDBarDeep"_ustr,                  TEMPLATE_THREEDBARDEEP},
        {u"com.sun.star.chart2.template.ThreeDBarFlat"_ustr,                  TEMPLATE_THREEDBARFLAT},
        {u"com.sun.star.chart2.template.StackedThreeDBarFlat"_ustr,           TEMPLATE_STACKEDTHREEDBARFLAT},
        {u"com.sun.star.chart2.template.PercentStackedThreeDBarFlat"_ustr,    TEMPLATE_PERCENTSTACKEDTHREEDBARFLAT},
        {u"com.sun.star.chart2.template.ColumnWithLine"_ustr,                 TEMPLATE_COLUMNWITHLINE},
        {u"com.sun.star.chart2.template.StackedColumnWithLine"_ustr,          TEMPLATE_STACKEDCOLUMNWITHLINE},
        {u"com.sun.star.chart2.template.Histogram"_ustr,                      TEMPLATE_HISTOGRAM},
        {u"com.sun.star.chart2.template.Area"_ustr,                           TEMPLATE_AREA},
        {u"com.sun.star.chart2.template.StackedArea"_ustr,                    TEMPLATE_STACKEDAREA},
        {u"com.sun.star.chart2.template.PercentStackedArea"_ustr,             TEMPLATE_PERCENTSTACKEDAREA},
        {u"com.sun.star.chart2.template.ThreeDArea"_ustr,                     TEMPLATE_THREEDAREA},
        {u"com.sun.star.chart2.template.StackedThreeDArea"_ustr,              TEMPLATE_STACKEDTHREEDAREA},
        {u"com.sun.star.chart2.template.PercentStackedThreeDArea"_ustr,       TEMPLATE_PERCENTSTACKEDTHREEDAREA},
        {u"com.sun.star.chart2.template.Pie"_ustr,                            TEMPLATE_PIE},
        {u"com.sun.star.chart2.template.PieAllExploded"_ustr,                 TEMPLATE_PIEALLEXPLODED},
        {u"com.sun.star.chart2.template.Donut"_ustr,                          TEMPLATE_DONUT},
        {u"com.sun.star.chart2.template.DonutAllExploded"_ustr,               TEMPLATE_DONUTALLEXPLODED},
        {u"com.sun.star.chart2.template.BarOfPie"_ustr,                       TEMPLATE_BAROFPIE},
        {u"com.sun.star.chart2.template.PieOfPie"_ustr,                       TEMPLATE_PIEOFPIE},
        {u"com.sun.star.chart2.template.DonutAllExploded"_ustr,               TEMPLATE_DONUTALLEXPLODED},
        {u"com.sun.star.chart2.template.ThreeDPie"_ustr,                      TEMPLATE_THREEDPIE},
        {u"com.sun.star.chart2.template.ThreeDPieAllExploded"_ustr,           TEMPLATE_THREEDPIEALLEXPLODED},
        {u"com.sun.star.chart2.template.ThreeDDonut"_ustr,                    TEMPLATE_THREEDDONUT},
        {u"com.sun.star.chart2.template.ThreeDDonutAllExploded"_ustr,         TEMPLATE_THREEDDONUTALLEXPLODED},
        {u"com.sun.star.chart2.template.ScatterLineSymbol"_ustr,              TEMPLATE_SCATTERLINESYMBOL},
        {u"com.sun.star.chart2.template.ScatterLine"_ustr,                    TEMPLATE_SCATTERLINE},
        {u"com.sun.star.chart2.template.ScatterSymbol"_ustr,                  TEMPLATE_SCATTERSYMBOL},
        {u"com.sun.star.chart2.template.ThreeDScatter"_ustr,                  TEMPLATE_THREEDSCATTER},
        {u"com.sun.star.chart2.template.Net"_ustr,                            TEMPLATE_NET},
        {u"com.sun.star.chart2.template.NetSymbol"_ustr,                      TEMPLATE_NETSYMBOL},
        {u"com.sun.star.chart2.template.NetLine"_ustr,                        TEMPLATE_NETLINE},
        {u"com.sun.star.chart2.template.StackedNet"_ustr,                     TEMPLATE_STACKEDNET},
        {u"com.sun.star.chart2.template.StackedNetSymbol"_ustr,               TEMPLATE_STACKEDNETSYMBOL},
        {u"com.sun.star.chart2.template.StackedNetLine"_ustr,                 TEMPLATE_STACKEDNETLINE},
        {u"com.sun.star.chart2.template.PercentStackedNet"_ustr,              TEMPLATE_PERCENTSTACKEDNET},
        {u"com.sun.star.chart2.template.PercentStackedNetSymbol"_ustr,        TEMPLATE_PERCENTSTACKEDNETSYMBOL},
        {u"com.sun.star.chart2.template.PercentStackedNetLine"_ustr,          TEMPLATE_PERCENTSTACKEDNETLINE},
        {u"com.sun.star.chart2.template.FilledNet"_ustr,                      TEMPLATE_FILLEDNET},
        {u"com.sun.star.chart2.template.StackedFilledNet"_ustr,               TEMPLATE_STACKEDFILLEDNET},
        {u"com.sun.star.chart2.template.PercentStackedFilledNet"_ustr,        TEMPLATE_PERCENTSTACKEDFILLEDNET},
        {u"com.sun.star.chart2.template.StockLowHighClose"_ustr,              TEMPLATE_STOCKLOWHIGHCLOSE},
        {u"com.sun.star.chart2.template.StockOpenLowHighClose"_ustr,          TEMPLATE_STOCKOPENLOWHIGHCLOSE},
        {u"com.sun.star.chart2.template.StockVolumeLowHighClose"_ustr,        TEMPLATE_STOCKVOLUMELOWHIGHCLOSE},
        {u"com.sun.star.chart2.template.StockVolumeOpenLowHighClose"_ustr,    TEMPLATE_STOCKVOLUMEOPENLOWHIGHCLOSE},
        {u"com.sun.star.chart2.template.Bubble"_ustr,                         TEMPLATE_BUBBLE},
        {u"com.sun.star.chart2.template.CorrelationCircle"_ustr,              TEMPLATE_CORRELATIONCIRCLE},
        {u"com.sun.star.chart2.template.Funnel"_ustr,                         TEMPLATE_FUNNEL},
        {u"com.sun.star.chart2.template.BoxWhisker"_ustr,                     TEMPLATE_BOXWHISKER},
        {u"com.sun.star.chart2.template.ClusteredColumn"_ustr,                TEMPLATE_CLUSTEREDCOLUMN},
        {u"com.sun.star.chart2.template.ParetoLine"_ustr,                     TEMPLATE_PARETOLINE},
        {u"com.sun.star.chart2.template.RegionMap"_ustr,                      TEMPLATE_REGIONMAP},
        {u"com.sun.star.chart2.template.Sunburst"_ustr,                       TEMPLATE_SUNBURST},
        {u"com.sun.star.chart2.template.Treemap"_ustr,                        TEMPLATE_TREEMAP},
        {u"com.sun.star.chart2.template.Waterfall"_ustr,                      TEMPLATE_WATERFALL},
//      {u"com.sun.star.chart2.template.Surface"_ustr,                        TEMPLATE_SURFACE},
//      {u"com.sun.star.chart2.template.Addin"_ustr,                          TEMPLATE_ADDIN},
        };
    return aMap;
}

TemplateId lcl_GetTemplateIdForService( const OUString & rServiceName )
{
    TemplateId eResult = TEMPLATE_NOT_FOUND;
    const tTemplateMapType & rMap = lcl_DefaultChartTypeMap();
    tTemplateMapType::const_iterator aIt( rMap.find( rServiceName ));

    if( aIt != rMap.end())
        eResult = (*aIt).second;

    return eResult;
}

} // anonymous namespace

namespace chart
{

ChartTypeManager::ChartTypeManager(
    uno::Reference<
        uno::XComponentContext > xContext ) :
    m_xContext(std::move( xContext ))
{}

ChartTypeManager::~ChartTypeManager()
{}

// ____ XMultiServiceFactory ____
uno::Reference< uno::XInterface > ChartTypeManager::createInstance(
    const OUString& aServiceSpecifier )
{
    return static_cast<cppu::OWeakObject*>(createTemplate(aServiceSpecifier).get());
}

rtl::Reference< ::chart::ChartTypeTemplate > ChartTypeManager::createTemplate(
    const OUString& aServiceSpecifier )
{
    TemplateId nId = lcl_GetTemplateIdForService( aServiceSpecifier );

    rtl::Reference< ::chart::ChartTypeTemplate > xTemplate;
    switch( nId )
    {
        // Point (category x axis)
        case TEMPLATE_SYMBOL:
            xTemplate.set( new LineChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::NONE, true, false ));
            break;
        case TEMPLATE_STACKEDSYMBOL:
            xTemplate.set( new LineChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::YStacked, true, false ));
            break;
        case TEMPLATE_PERCENTSTACKEDSYMBOL:
            xTemplate.set( new LineChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::YStackedPercent, true, false ));
            break;
        // Line (category x axis)
        case TEMPLATE_LINE:
            xTemplate.set( new LineChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::NONE, false ));
            break;
        case TEMPLATE_STACKEDLINE:
            xTemplate.set( new LineChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::YStacked, false ));
            break;
        case TEMPLATE_PERCENTSTACKEDLINE:
            xTemplate.set( new LineChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::YStackedPercent, false ));
            break;
        case TEMPLATE_LINESYMBOL:
            xTemplate.set( new LineChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::NONE, true ));
            break;
        case TEMPLATE_STACKEDLINESYMBOL:
            xTemplate.set( new LineChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::YStacked, true ));
            break;
        case TEMPLATE_PERCENTSTACKEDLINESYMBOL:
            xTemplate.set( new LineChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::YStackedPercent, true ));
            break;
        case TEMPLATE_THREEDLINE:
            xTemplate.set( new LineChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::NONE, false, true, 3 ));
            break;
        case TEMPLATE_STACKEDTHREEDLINE:
            xTemplate.set( new LineChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::YStacked, false, true, 3 ));
            break;
        case TEMPLATE_PERCENTSTACKEDTHREEDLINE:
            xTemplate.set( new LineChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::YStackedPercent, false, true, 3 ));
            break;
        case TEMPLATE_THREEDLINEDEEP:
            xTemplate.set( new LineChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::ZStacked, false, true, 3 ));
            break;

        // Bar/Column
        case TEMPLATE_COLUMN:
            xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::NONE, BarChartTypeTemplate::VERTICAL ));
            break;
        case TEMPLATE_STACKEDCOLUMN:
            xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::YStacked, BarChartTypeTemplate::VERTICAL ));
            break;
        case TEMPLATE_PERCENTSTACKEDCOLUMN:
            xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::YStackedPercent, BarChartTypeTemplate::VERTICAL ));
            break;
        case TEMPLATE_BAR:
            xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::NONE, BarChartTypeTemplate::HORIZONTAL ));
            break;
        case TEMPLATE_STACKEDBAR:
            xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::YStacked, BarChartTypeTemplate::HORIZONTAL ));
            break;
        case TEMPLATE_PERCENTSTACKEDBAR:
            xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::YStackedPercent, BarChartTypeTemplate::HORIZONTAL ));
            break;
        case TEMPLATE_THREEDCOLUMNDEEP:
            xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::ZStacked, BarChartTypeTemplate::VERTICAL, 3 ));
            break;
        case TEMPLATE_THREEDCOLUMNFLAT:
            xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::NONE, BarChartTypeTemplate::VERTICAL, 3 ));
            break;
        case TEMPLATE_STACKEDTHREEDCOLUMNFLAT:
            xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::YStacked, BarChartTypeTemplate::VERTICAL, 3 ));
            break;
        case TEMPLATE_PERCENTSTACKEDTHREEDCOLUMNFLAT:
            xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::YStackedPercent, BarChartTypeTemplate::VERTICAL, 3 ));
            break;
        case TEMPLATE_THREEDBARDEEP:
            xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::ZStacked, BarChartTypeTemplate::HORIZONTAL, 3 ));
            break;
        case TEMPLATE_THREEDBARFLAT:
            xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::NONE, BarChartTypeTemplate::HORIZONTAL, 3 ));
            break;
        case TEMPLATE_STACKEDTHREEDBARFLAT:
            xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::YStacked, BarChartTypeTemplate::HORIZONTAL, 3 ));
            break;
        case TEMPLATE_PERCENTSTACKEDTHREEDBARFLAT:
            xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::YStackedPercent, BarChartTypeTemplate::HORIZONTAL, 3 ));
            break;

        // Combi-Chart Line/Column
        case TEMPLATE_COLUMNWITHLINE:
        case TEMPLATE_STACKEDCOLUMNWITHLINE:
        {
            StackMode eMode = ( nId == TEMPLATE_COLUMNWITHLINE )
                ? StackMode::NONE
                : StackMode::YStacked;

            xTemplate.set( new ColumnLineChartTypeTemplate( m_xContext, aServiceSpecifier, eMode, 1 ));
        }
        break;

        // Histogram
        case TEMPLATE_HISTOGRAM:
            xTemplate.set(new HistogramChartTypeTemplate(m_xContext, aServiceSpecifier, StackMode::NONE));
            break;

        // Area
        case TEMPLATE_AREA:
            xTemplate.set( new AreaChartTypeTemplate( m_xContext, aServiceSpecifier, StackMode::NONE ));
            break;
        case TEMPLATE_STACKEDAREA:
            xTemplate.set( new AreaChartTypeTemplate( m_xContext, aServiceSpecifier, StackMode::YStacked ));
            break;
        case TEMPLATE_PERCENTSTACKEDAREA:
            xTemplate.set( new AreaChartTypeTemplate( m_xContext, aServiceSpecifier, StackMode::YStackedPercent ));
            break;
        case TEMPLATE_THREEDAREA:
            xTemplate.set( new AreaChartTypeTemplate( m_xContext, aServiceSpecifier, StackMode::ZStacked, 3 ));
            break;
        case TEMPLATE_STACKEDTHREEDAREA:
            xTemplate.set( new AreaChartTypeTemplate( m_xContext, aServiceSpecifier, StackMode::YStacked, 3 ));
            break;
        case TEMPLATE_PERCENTSTACKEDTHREEDAREA:
            xTemplate.set( new AreaChartTypeTemplate( m_xContext, aServiceSpecifier, StackMode::YStackedPercent, 3 ));
            break;

        case TEMPLATE_PIE:
            xTemplate.set( new PieChartTypeTemplate( m_xContext, aServiceSpecifier,
                chart2::PieChartOffsetMode_NONE, false,
                chart2::PieChartSubType_NONE, 3, 2));
            break;
        case TEMPLATE_PIEALLEXPLODED:
            xTemplate.set( new PieChartTypeTemplate( m_xContext, aServiceSpecifier,
                chart2::PieChartOffsetMode_ALL_EXPLODED, false,
                chart2::PieChartSubType_NONE, 3, 2 ));
            break;
        case TEMPLATE_DONUT:
            xTemplate.set( new PieChartTypeTemplate( m_xContext, aServiceSpecifier,
                chart2::PieChartOffsetMode_NONE, true,
                chart2::PieChartSubType_NONE, 3, 2 ));
            break;
        case TEMPLATE_DONUTALLEXPLODED:
            xTemplate.set( new PieChartTypeTemplate( m_xContext, aServiceSpecifier,
                chart2::PieChartOffsetMode_ALL_EXPLODED, true,
                chart2::PieChartSubType_NONE, 3, 2 ));
            break;
        case TEMPLATE_BAROFPIE:
            xTemplate.set( new PieChartTypeTemplate( m_xContext, aServiceSpecifier,
                chart2::PieChartOffsetMode_NONE, false,
                chart2::PieChartSubType_BAR, 3, 2 ));
            break;
        case TEMPLATE_PIEOFPIE:
            xTemplate.set( new PieChartTypeTemplate( m_xContext, aServiceSpecifier,
                chart2::PieChartOffsetMode_NONE, false,
                chart2::PieChartSubType_PIE, 3, 2 ));
            break;
        case TEMPLATE_THREEDPIE:
            xTemplate.set( new PieChartTypeTemplate( m_xContext, aServiceSpecifier,
                chart2::PieChartOffsetMode_NONE, false,
                chart2::PieChartSubType_NONE, 3, 3 ));
            break;
        case TEMPLATE_THREEDPIEALLEXPLODED:
            xTemplate.set( new PieChartTypeTemplate( m_xContext, aServiceSpecifier,
                chart2::PieChartOffsetMode_ALL_EXPLODED, false,
                chart2::PieChartSubType_NONE, 3, 3 ));
            break;
        case TEMPLATE_THREEDDONUT:
            xTemplate.set( new PieChartTypeTemplate( m_xContext, aServiceSpecifier,
                chart2::PieChartOffsetMode_NONE, true,
                chart2::PieChartSubType_NONE, 3, 3 ));
            break;
        case TEMPLATE_THREEDDONUTALLEXPLODED:
            xTemplate.set( new PieChartTypeTemplate( m_xContext, aServiceSpecifier,
                chart2::PieChartOffsetMode_ALL_EXPLODED, true,
                chart2::PieChartSubType_NONE, 3, 3 ));
            break;

        case TEMPLATE_SCATTERLINESYMBOL:
            xTemplate.set( new ScatterChartTypeTemplate( m_xContext, aServiceSpecifier, /* bSymbols */ true ));
            break;
        case TEMPLATE_SCATTERLINE:
            xTemplate.set( new ScatterChartTypeTemplate( m_xContext, aServiceSpecifier, /* bSymbols */ false ));
            break;
        case TEMPLATE_SCATTERSYMBOL:
            xTemplate.set( new ScatterChartTypeTemplate( m_xContext, aServiceSpecifier, /* bSymbols */ true, /* bHasLines */ false ));
            break;
        case TEMPLATE_THREEDSCATTER:
            xTemplate.set( new ScatterChartTypeTemplate( m_xContext, aServiceSpecifier, /* bSymbols */ false, /* bHasLines */ true, 3 ));
            break;

        // NetChart
        case TEMPLATE_NET:
            xTemplate.set( new NetChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::NONE, true ));
            break;
        case TEMPLATE_NETSYMBOL:
            xTemplate.set( new NetChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::NONE, true, false ));
            break;
        case TEMPLATE_NETLINE:
            xTemplate.set( new NetChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::NONE, false ));
            break;

        case TEMPLATE_STACKEDNET:
            xTemplate.set( new NetChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::YStacked, true ));
            break;
        case TEMPLATE_STACKEDNETSYMBOL:
            xTemplate.set( new NetChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::YStacked, true, false ));
            break;
        case TEMPLATE_STACKEDNETLINE:
            xTemplate.set( new NetChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::YStacked, false, true ));
            break;

        case TEMPLATE_PERCENTSTACKEDNET:
            xTemplate.set( new NetChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::YStackedPercent, true ));
            break;
        case TEMPLATE_PERCENTSTACKEDNETSYMBOL:
            xTemplate.set( new NetChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::YStackedPercent, true, false ));
            break;
        case TEMPLATE_PERCENTSTACKEDNETLINE:
            xTemplate.set( new NetChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::YStackedPercent, false, true ));
            break;

        case TEMPLATE_FILLEDNET:
            xTemplate.set( new NetChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::NONE, false, false, true ));
            break;
        case TEMPLATE_STACKEDFILLEDNET:
            xTemplate.set( new NetChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::YStacked, false, false, true ));
            break;
        case TEMPLATE_PERCENTSTACKEDFILLEDNET:
            xTemplate.set( new NetChartTypeTemplate( m_xContext, aServiceSpecifier,
                StackMode::YStackedPercent, false, false, true ));
            break;

        case TEMPLATE_STOCKLOWHIGHCLOSE:
            xTemplate.set( new StockChartTypeTemplate( m_xContext, aServiceSpecifier,
                StockChartTypeTemplate::StockVariant::NONE, false ));
            break;
        case TEMPLATE_STOCKOPENLOWHIGHCLOSE:
            xTemplate.set( new StockChartTypeTemplate( m_xContext, aServiceSpecifier,
                StockChartTypeTemplate::StockVariant::Open, true ));
            break;
        case TEMPLATE_STOCKVOLUMELOWHIGHCLOSE:
            xTemplate.set( new StockChartTypeTemplate( m_xContext, aServiceSpecifier,
                StockChartTypeTemplate::StockVariant::Volume, false ));
            break;
        case TEMPLATE_STOCKVOLUMEOPENLOWHIGHCLOSE:
            xTemplate.set( new StockChartTypeTemplate( m_xContext, aServiceSpecifier,
                StockChartTypeTemplate::StockVariant::VolumeOpen, true ));
            break;

        //BubbleChart
        case TEMPLATE_BUBBLE:
            xTemplate.set( new BubbleChartTypeTemplate( m_xContext, aServiceSpecifier ));
            break;

        // Correlation circle plot
        case TEMPLATE_CORRELATIONCIRCLE:
            xTemplate.set( new CorrelationCircleChartTypeTemplate( m_xContext, aServiceSpecifier ));
            break;

        // Funnel chart
        case TEMPLATE_FUNNEL:
            xTemplate.set( new FunnelChartTypeTemplate( m_xContext,
                        aServiceSpecifier));
            break;
        case TEMPLATE_BOXWHISKER:
            xTemplate.set( new BoxWhiskerChartTypeTemplate( m_xContext,
                        aServiceSpecifier));
            break;
        case TEMPLATE_CLUSTEREDCOLUMN:
            xTemplate.set( new ClusteredColumnChartTypeTemplate( m_xContext,
                        aServiceSpecifier));
            break;
        case TEMPLATE_PARETOLINE:
            xTemplate.set( new ParetoLineChartTypeTemplate( m_xContext,
                        aServiceSpecifier));
            break;
        case TEMPLATE_REGIONMAP:
            xTemplate.set( new RegionMapChartTypeTemplate( m_xContext,
                        aServiceSpecifier));
            break;
        case TEMPLATE_SUNBURST:
            xTemplate.set( new SunburstChartTypeTemplate( m_xContext,
                        aServiceSpecifier));
            break;
        case TEMPLATE_TREEMAP:
            xTemplate.set( new TreemapChartTypeTemplate( m_xContext,
                        aServiceSpecifier));
            break;
        case TEMPLATE_WATERFALL:
            xTemplate.set( new WaterfallChartTypeTemplate( m_xContext,
                        aServiceSpecifier));
            break;
        case TEMPLATE_NOT_FOUND:
            SAL_WARN("chart2", "Couldn't instantiate service: "<< aServiceSpecifier );
            assert(false);
            break;

        default: break;
//            case TEMPLATE_SURFACE:
//            case TEMPLATE_ADDIN:
//               break;
    }

    return xTemplate;
}

uno::Reference< uno::XInterface > ChartTypeManager::createInstanceWithArguments(
    const OUString& ServiceSpecifier,
    const cpo::uno::Sequence< cpo::uno::Any >& /* Arguments */ )
{
    OSL_FAIL( "createInstanceWithArguments: No arguments supported" );
    return createInstance( ServiceSpecifier );
}

cpo::uno::Sequence< OUString > ChartTypeManager::getAvailableServiceNames()
{
    std::vector< OUString > aServices;
    const tTemplateMapType & rMap = lcl_DefaultChartTypeMap();
    aServices.reserve( rMap.size());

    // get own default templates
    std::transform( rMap.begin(), rMap.end(), std::back_inserter( aServices ),
            ::o3tl::select1st< tTemplateMapType::value_type >() );

    // add components that were registered in the context's factory
    uno::Reference< container::XContentEnumerationAccess > xEnumAcc(
        m_xContext->getServiceManager(), uno::UNO_QUERY );
    if( xEnumAcc.is())
    {
        uno::Reference< container::XEnumeration > xEnum(
            xEnumAcc->createContentEnumeration( u"com.sun.star.chart2.ChartTypeTemplate"_ustr ));
        if( xEnum.is())
        {
            uno::Reference< uno::XInterface > xFactIntf;

            while( xEnum->hasMoreElements())
            {
                if( xEnum->nextElement() >>= xFactIntf )
                {
                    uno::Reference< lang::XServiceName > xServiceName( xFactIntf, uno::UNO_QUERY );
                    if( xServiceName.is())
                        aServices.push_back( xServiceName->getServiceName());
                }
            }
        }
    }

    return comphelper::containerToSequence( aServices );
}

// ____ XServiceInfo ____
OUString ChartTypeManager::getImplementationName()
{
    return u"com.sun.star.comp.chart.ChartTypeManager"_ustr;
}

bool ChartTypeManager::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

cpo::uno::Sequence< OUString > ChartTypeManager::getSupportedServiceNames()
{
    return {
        u"com.sun.star.chart2.ChartTypeManager"_ustr,
        u"com.sun.star.lang.MultiServiceFactory"_ustr };
}

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart_ChartTypeManager_get_implementation(css::uno::XComponentContext *context,
        cpo::uno::Sequence<cpo::uno::Any> const &)
{
    return cppu::acquire(new ::chart::ChartTypeManager(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
