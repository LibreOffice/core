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

#include <ChartTypeManager.hxx>
#include <StackMode.hxx>

#include "LineChartTypeTemplate.hxx"
#include "BarChartTypeTemplate.hxx"
#include "ColumnLineChartTypeTemplate.hxx"
#include "AreaChartTypeTemplate.hxx"
#include "PieChartTypeTemplate.hxx"
#include "ScatterChartTypeTemplate.hxx"
#include "StockChartTypeTemplate.hxx"
#include "NetChartTypeTemplate.hxx"
#include "BubbleChartTypeTemplate.hxx"
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <tools/diagnose_ex.h>
#include <sal/log.hxx>

#include <algorithm>
#include <iterator>
#include <o3tl/functional.hxx>
#include <map>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Sequence;

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
//    TEMPLATE_SURFACE,
//     TEMPLATE_ADDIN,
    TEMPLATE_NOT_FOUND = 0xffff
};

typedef std::map< OUString, TemplateId > tTemplateMapType;

const tTemplateMapType & lcl_DefaultChartTypeMap()
{
    static const tTemplateMapType aMap{
        {"com.sun.star.chart2.template.Symbol",                         TEMPLATE_SYMBOL},
        {"com.sun.star.chart2.template.StackedSymbol",                  TEMPLATE_STACKEDSYMBOL},
        {"com.sun.star.chart2.template.PercentStackedSymbol",           TEMPLATE_PERCENTSTACKEDSYMBOL},
        {"com.sun.star.chart2.template.Line",                           TEMPLATE_LINE},
        {"com.sun.star.chart2.template.StackedLine",                    TEMPLATE_STACKEDLINE},
        {"com.sun.star.chart2.template.PercentStackedLine",             TEMPLATE_PERCENTSTACKEDLINE},
        {"com.sun.star.chart2.template.LineSymbol",                     TEMPLATE_LINESYMBOL},
        {"com.sun.star.chart2.template.StackedLineSymbol",              TEMPLATE_STACKEDLINESYMBOL},
        {"com.sun.star.chart2.template.PercentStackedLineSymbol",       TEMPLATE_PERCENTSTACKEDLINESYMBOL},
        {"com.sun.star.chart2.template.ThreeDLine",                     TEMPLATE_THREEDLINE},
        {"com.sun.star.chart2.template.StackedThreeDLine",              TEMPLATE_STACKEDTHREEDLINE},
        {"com.sun.star.chart2.template.PercentStackedThreeDLine",       TEMPLATE_PERCENTSTACKEDTHREEDLINE},
        {"com.sun.star.chart2.template.ThreeDLineDeep",                 TEMPLATE_THREEDLINEDEEP},
        {"com.sun.star.chart2.template.Column",                         TEMPLATE_COLUMN},
        {"com.sun.star.chart2.template.StackedColumn",                  TEMPLATE_STACKEDCOLUMN},
        {"com.sun.star.chart2.template.PercentStackedColumn",           TEMPLATE_PERCENTSTACKEDCOLUMN},
        {"com.sun.star.chart2.template.Bar",                            TEMPLATE_BAR},
        {"com.sun.star.chart2.template.StackedBar",                     TEMPLATE_STACKEDBAR},
        {"com.sun.star.chart2.template.PercentStackedBar",              TEMPLATE_PERCENTSTACKEDBAR},
        {"com.sun.star.chart2.template.ThreeDColumnDeep",               TEMPLATE_THREEDCOLUMNDEEP},
        {"com.sun.star.chart2.template.ThreeDColumnFlat",               TEMPLATE_THREEDCOLUMNFLAT},
        {"com.sun.star.chart2.template.StackedThreeDColumnFlat",        TEMPLATE_STACKEDTHREEDCOLUMNFLAT},
        {"com.sun.star.chart2.template.PercentStackedThreeDColumnFlat", TEMPLATE_PERCENTSTACKEDTHREEDCOLUMNFLAT},
        {"com.sun.star.chart2.template.ThreeDBarDeep",                  TEMPLATE_THREEDBARDEEP},
        {"com.sun.star.chart2.template.ThreeDBarFlat",                  TEMPLATE_THREEDBARFLAT},
        {"com.sun.star.chart2.template.StackedThreeDBarFlat",           TEMPLATE_STACKEDTHREEDBARFLAT},
        {"com.sun.star.chart2.template.PercentStackedThreeDBarFlat",    TEMPLATE_PERCENTSTACKEDTHREEDBARFLAT},
        {"com.sun.star.chart2.template.ColumnWithLine",                 TEMPLATE_COLUMNWITHLINE},
        {"com.sun.star.chart2.template.StackedColumnWithLine",          TEMPLATE_STACKEDCOLUMNWITHLINE},
        {"com.sun.star.chart2.template.Area",                           TEMPLATE_AREA},
        {"com.sun.star.chart2.template.StackedArea",                    TEMPLATE_STACKEDAREA},
        {"com.sun.star.chart2.template.PercentStackedArea",             TEMPLATE_PERCENTSTACKEDAREA},
        {"com.sun.star.chart2.template.ThreeDArea",                     TEMPLATE_THREEDAREA},
        {"com.sun.star.chart2.template.StackedThreeDArea",              TEMPLATE_STACKEDTHREEDAREA},
        {"com.sun.star.chart2.template.PercentStackedThreeDArea",       TEMPLATE_PERCENTSTACKEDTHREEDAREA},
        {"com.sun.star.chart2.template.Pie",                            TEMPLATE_PIE},
        {"com.sun.star.chart2.template.PieAllExploded",                 TEMPLATE_PIEALLEXPLODED},
        {"com.sun.star.chart2.template.Donut",                          TEMPLATE_DONUT},
        {"com.sun.star.chart2.template.DonutAllExploded",               TEMPLATE_DONUTALLEXPLODED},
        {"com.sun.star.chart2.template.ThreeDPie",                      TEMPLATE_THREEDPIE},
        {"com.sun.star.chart2.template.ThreeDPieAllExploded",           TEMPLATE_THREEDPIEALLEXPLODED},
        {"com.sun.star.chart2.template.ThreeDDonut",                    TEMPLATE_THREEDDONUT},
        {"com.sun.star.chart2.template.ThreeDDonutAllExploded",         TEMPLATE_THREEDDONUTALLEXPLODED},
        {"com.sun.star.chart2.template.ScatterLineSymbol",              TEMPLATE_SCATTERLINESYMBOL},
        {"com.sun.star.chart2.template.ScatterLine",                    TEMPLATE_SCATTERLINE},
        {"com.sun.star.chart2.template.ScatterSymbol",                  TEMPLATE_SCATTERSYMBOL},
        {"com.sun.star.chart2.template.ThreeDScatter",                  TEMPLATE_THREEDSCATTER},
        {"com.sun.star.chart2.template.Net",                            TEMPLATE_NET},
        {"com.sun.star.chart2.template.NetSymbol",                      TEMPLATE_NETSYMBOL},
        {"com.sun.star.chart2.template.NetLine",                        TEMPLATE_NETLINE},
        {"com.sun.star.chart2.template.StackedNet",                     TEMPLATE_STACKEDNET},
        {"com.sun.star.chart2.template.StackedNetSymbol",               TEMPLATE_STACKEDNETSYMBOL},
        {"com.sun.star.chart2.template.StackedNetLine",                 TEMPLATE_STACKEDNETLINE},
        {"com.sun.star.chart2.template.PercentStackedNet",              TEMPLATE_PERCENTSTACKEDNET},
        {"com.sun.star.chart2.template.PercentStackedNetSymbol",        TEMPLATE_PERCENTSTACKEDNETSYMBOL},
        {"com.sun.star.chart2.template.PercentStackedNetLine",          TEMPLATE_PERCENTSTACKEDNETLINE},
        {"com.sun.star.chart2.template.FilledNet",                      TEMPLATE_FILLEDNET},
        {"com.sun.star.chart2.template.StackedFilledNet",               TEMPLATE_STACKEDFILLEDNET},
        {"com.sun.star.chart2.template.PercentStackedFilledNet",        TEMPLATE_PERCENTSTACKEDFILLEDNET},
        {"com.sun.star.chart2.template.StockLowHighClose",              TEMPLATE_STOCKLOWHIGHCLOSE},
        {"com.sun.star.chart2.template.StockOpenLowHighClose",          TEMPLATE_STOCKOPENLOWHIGHCLOSE},
        {"com.sun.star.chart2.template.StockVolumeLowHighClose",        TEMPLATE_STOCKVOLUMELOWHIGHCLOSE},
        {"com.sun.star.chart2.template.StockVolumeOpenLowHighClose",    TEMPLATE_STOCKVOLUMEOPENLOWHIGHCLOSE},
        {"com.sun.star.chart2.template.Bubble",                         TEMPLATE_BUBBLE},
//      {"com.sun.star.chart2.template.Surface",                        TEMPLATE_SURFACE},
//      {"com.sun.star.chart2.template.Addin",                          TEMPLATE_ADDIN},
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
        uno::XComponentContext > const & xContext ) :
    m_xContext( xContext )
{}

ChartTypeManager::~ChartTypeManager()
{}

// ____ XMultiServiceFactory ____
uno::Reference< uno::XInterface > SAL_CALL ChartTypeManager::createInstance(
    const OUString& aServiceSpecifier )
{
    uno::Reference< uno::XInterface > xResult;
    TemplateId nId = lcl_GetTemplateIdForService( aServiceSpecifier );

    if( nId == TEMPLATE_NOT_FOUND )
    {
        try
        {
            xResult = m_xContext->getServiceManager()->createInstanceWithContext(
                aServiceSpecifier, m_xContext );
        }
//         catch( registry::InvalidValueException & ex )
        catch( const uno::Exception & )
        {
            // couldn't create service via factory

            // As XMultiServiceFactory does not specify, what to do in case
            // createInstance is called with an unknown service-name, this
            // function will just return an empty XInterface.
            DBG_UNHANDLED_EXCEPTION("chart2");
            SAL_WARN("chart2", "Couldn't instantiate service: "<< aServiceSpecifier );
            xResult.set( nullptr );
        }
    }
    else
    {
        uno::Reference< chart2::XChartTypeTemplate > xTemplate;
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
                    chart2::PieChartOffsetMode_NONE, false ));
                break;
            case TEMPLATE_PIEALLEXPLODED:
                xTemplate.set( new PieChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::PieChartOffsetMode_ALL_EXPLODED, false ));
                break;
            case TEMPLATE_DONUT:
                xTemplate.set( new PieChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::PieChartOffsetMode_NONE, true ));
                break;
            case TEMPLATE_DONUTALLEXPLODED:
                xTemplate.set( new PieChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::PieChartOffsetMode_ALL_EXPLODED, true ));
                break;
            case TEMPLATE_THREEDPIE:
                xTemplate.set( new PieChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::PieChartOffsetMode_NONE, false, 3 ));
                break;
            case TEMPLATE_THREEDPIEALLEXPLODED:
                xTemplate.set( new PieChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::PieChartOffsetMode_ALL_EXPLODED, false, 3 ));
                break;
            case TEMPLATE_THREEDDONUT:
                xTemplate.set( new PieChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::PieChartOffsetMode_NONE, true, 3 ));
                break;
            case TEMPLATE_THREEDDONUTALLEXPLODED:
                xTemplate.set( new PieChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::PieChartOffsetMode_ALL_EXPLODED, true, 3 ));
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
            default: break;
//            case TEMPLATE_SURFACE:
//            case TEMPLATE_ADDIN:
//               break;

            case TEMPLATE_NOT_FOUND:
                OSL_ASSERT( false );
                break;
        }
        xResult.set( xTemplate, uno::UNO_QUERY );
    }

    return xResult;
}

uno::Reference< uno::XInterface > SAL_CALL ChartTypeManager::createInstanceWithArguments(
    const OUString& ServiceSpecifier,
    const uno::Sequence< uno::Any >& /* Arguments */ )
{
    OSL_FAIL( "createInstanceWithArguments: No arguments supported" );
    return createInstance( ServiceSpecifier );
}

uno::Sequence< OUString > SAL_CALL ChartTypeManager::getAvailableServiceNames()
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
            xEnumAcc->createContentEnumeration( "com.sun.star.chart2.ChartTypeTemplate" ));
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
OUString SAL_CALL ChartTypeManager::getImplementationName()
{
    return OUString("com.sun.star.comp.chart.ChartTypeManager");
}

sal_Bool SAL_CALL ChartTypeManager::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL ChartTypeManager::getSupportedServiceNames()
{
    return {
        "com.sun.star.chart2.ChartTypeManager",
        "com.sun.star.lang.MultiServiceFactory" };
}

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart_ChartTypeManager_get_implementation(css::uno::XComponentContext *context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::ChartTypeManager(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
