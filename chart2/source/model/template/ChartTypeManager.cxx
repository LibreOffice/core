/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "ChartTypeManager.hxx"
#include "macros.hxx"
#include "StackMode.hxx"
#include "ContainerHelper.hxx"

#include "CartesianCoordinateSystem.hxx"

#include "LineChartTypeTemplate.hxx"
#include "BarChartTypeTemplate.hxx"
#include "ColumnLineChartTypeTemplate.hxx"
#include "AreaChartTypeTemplate.hxx"
#include "PieChartTypeTemplate.hxx"
#include "ScatterChartTypeTemplate.hxx"
#include "StockChartTypeTemplate.hxx"
#include "NetChartTypeTemplate.hxx"
#include "BubbleChartTypeTemplate.hxx"
#include <cppuhelper/component_context.hxx>
#include <comphelper/InlineContainer.hxx>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/chart/ChartSolidType.hpp>
#include <com/sun/star/chart2/CurveStyle.hpp>

#include <algorithm>
#include <iterator>
#include <functional>
#include <o3tl/compat_functional.hxx>

using namespace ::com::sun::star;

using ::rtl::OUString;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::osl::MutexGuard;

// ======================================================================

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

typedef ::std::map< OUString, TemplateId > tTemplateMapType;

const tTemplateMapType & lcl_DefaultChartTypeMap()
{
    static const tTemplateMapType aMap = tTemplateMapType(
        ::comphelper::MakeMap< tTemplateMapType::key_type, tTemplateMapType::mapped_type >
        ( C2U( "com.sun.star.chart2.template.Symbol" ),                         TEMPLATE_SYMBOL )
        ( C2U( "com.sun.star.chart2.template.StackedSymbol" ),                  TEMPLATE_STACKEDSYMBOL )
        ( C2U( "com.sun.star.chart2.template.PercentStackedSymbol" ),           TEMPLATE_PERCENTSTACKEDSYMBOL )
        ( C2U( "com.sun.star.chart2.template.Line" ),                           TEMPLATE_LINE )
        ( C2U( "com.sun.star.chart2.template.StackedLine" ),                    TEMPLATE_STACKEDLINE )
        ( C2U( "com.sun.star.chart2.template.PercentStackedLine" ),             TEMPLATE_PERCENTSTACKEDLINE )
        ( C2U( "com.sun.star.chart2.template.LineSymbol" ),                     TEMPLATE_LINESYMBOL )
        ( C2U( "com.sun.star.chart2.template.StackedLineSymbol" ),              TEMPLATE_STACKEDLINESYMBOL )
        ( C2U( "com.sun.star.chart2.template.PercentStackedLineSymbol" ),       TEMPLATE_PERCENTSTACKEDLINESYMBOL )
        ( C2U( "com.sun.star.chart2.template.ThreeDLine" ),                     TEMPLATE_THREEDLINE )
        ( C2U( "com.sun.star.chart2.template.StackedThreeDLine" ),              TEMPLATE_STACKEDTHREEDLINE )
        ( C2U( "com.sun.star.chart2.template.PercentStackedThreeDLine" ),       TEMPLATE_PERCENTSTACKEDTHREEDLINE )
        ( C2U( "com.sun.star.chart2.template.ThreeDLineDeep" ),                 TEMPLATE_THREEDLINEDEEP )
        ( C2U( "com.sun.star.chart2.template.Column" ),                         TEMPLATE_COLUMN )
        ( C2U( "com.sun.star.chart2.template.StackedColumn" ),                  TEMPLATE_STACKEDCOLUMN )
        ( C2U( "com.sun.star.chart2.template.PercentStackedColumn" ),           TEMPLATE_PERCENTSTACKEDCOLUMN )
        ( C2U( "com.sun.star.chart2.template.Bar" ),                            TEMPLATE_BAR )
        ( C2U( "com.sun.star.chart2.template.StackedBar" ),                     TEMPLATE_STACKEDBAR )
        ( C2U( "com.sun.star.chart2.template.PercentStackedBar" ),              TEMPLATE_PERCENTSTACKEDBAR )
        ( C2U( "com.sun.star.chart2.template.ThreeDColumnDeep" ),               TEMPLATE_THREEDCOLUMNDEEP )
        ( C2U( "com.sun.star.chart2.template.ThreeDColumnFlat" ),               TEMPLATE_THREEDCOLUMNFLAT )
        ( C2U( "com.sun.star.chart2.template.StackedThreeDColumnFlat" ),        TEMPLATE_STACKEDTHREEDCOLUMNFLAT )
        ( C2U( "com.sun.star.chart2.template.PercentStackedThreeDColumnFlat" ), TEMPLATE_PERCENTSTACKEDTHREEDCOLUMNFLAT )
        ( C2U( "com.sun.star.chart2.template.ThreeDBarDeep" ),                  TEMPLATE_THREEDBARDEEP )
        ( C2U( "com.sun.star.chart2.template.ThreeDBarFlat" ),                  TEMPLATE_THREEDBARFLAT )
        ( C2U( "com.sun.star.chart2.template.StackedThreeDBarFlat" ),           TEMPLATE_STACKEDTHREEDBARFLAT )
        ( C2U( "com.sun.star.chart2.template.PercentStackedThreeDBarFlat" ),    TEMPLATE_PERCENTSTACKEDTHREEDBARFLAT )
        ( C2U( "com.sun.star.chart2.template.ColumnWithLine" ),                 TEMPLATE_COLUMNWITHLINE )
        ( C2U( "com.sun.star.chart2.template.StackedColumnWithLine" ),          TEMPLATE_STACKEDCOLUMNWITHLINE )
        ( C2U( "com.sun.star.chart2.template.Area" ),                           TEMPLATE_AREA )
        ( C2U( "com.sun.star.chart2.template.StackedArea" ),                    TEMPLATE_STACKEDAREA )
        ( C2U( "com.sun.star.chart2.template.PercentStackedArea" ),             TEMPLATE_PERCENTSTACKEDAREA )
        ( C2U( "com.sun.star.chart2.template.ThreeDArea" ),                     TEMPLATE_THREEDAREA )
        ( C2U( "com.sun.star.chart2.template.StackedThreeDArea" ),              TEMPLATE_STACKEDTHREEDAREA )
        ( C2U( "com.sun.star.chart2.template.PercentStackedThreeDArea" ),       TEMPLATE_PERCENTSTACKEDTHREEDAREA )
        ( C2U( "com.sun.star.chart2.template.Pie" ),                            TEMPLATE_PIE )
        ( C2U( "com.sun.star.chart2.template.PieAllExploded" ),                 TEMPLATE_PIEALLEXPLODED )
        ( C2U( "com.sun.star.chart2.template.Donut" ),                          TEMPLATE_DONUT )
        ( C2U( "com.sun.star.chart2.template.DonutAllExploded" ),               TEMPLATE_DONUTALLEXPLODED )
        ( C2U( "com.sun.star.chart2.template.ThreeDPie" ),                      TEMPLATE_THREEDPIE )
        ( C2U( "com.sun.star.chart2.template.ThreeDPieAllExploded" ),           TEMPLATE_THREEDPIEALLEXPLODED )
        ( C2U( "com.sun.star.chart2.template.ThreeDDonut" ),                    TEMPLATE_THREEDDONUT )
        ( C2U( "com.sun.star.chart2.template.ThreeDDonutAllExploded" ),         TEMPLATE_THREEDDONUTALLEXPLODED )
        ( C2U( "com.sun.star.chart2.template.ScatterLineSymbol" ),              TEMPLATE_SCATTERLINESYMBOL )
        ( C2U( "com.sun.star.chart2.template.ScatterLine" ),                    TEMPLATE_SCATTERLINE )
        ( C2U( "com.sun.star.chart2.template.ScatterSymbol" ),                  TEMPLATE_SCATTERSYMBOL )
        ( C2U( "com.sun.star.chart2.template.ThreeDScatter" ),                  TEMPLATE_THREEDSCATTER )
        ( C2U( "com.sun.star.chart2.template.Net" ),                            TEMPLATE_NET )
        ( C2U( "com.sun.star.chart2.template.NetSymbol" ),                      TEMPLATE_NETSYMBOL )
        ( C2U( "com.sun.star.chart2.template.NetLine" ),                        TEMPLATE_NETLINE )
        ( C2U( "com.sun.star.chart2.template.StackedNet" ),                     TEMPLATE_STACKEDNET )
        ( C2U( "com.sun.star.chart2.template.StackedNetSymbol" ),               TEMPLATE_STACKEDNETSYMBOL )
        ( C2U( "com.sun.star.chart2.template.StackedNetLine" ),                 TEMPLATE_STACKEDNETLINE )
        ( C2U( "com.sun.star.chart2.template.PercentStackedNet" ),              TEMPLATE_PERCENTSTACKEDNET )
        ( C2U( "com.sun.star.chart2.template.PercentStackedNetSymbol" ),        TEMPLATE_PERCENTSTACKEDNETSYMBOL )
        ( C2U( "com.sun.star.chart2.template.PercentStackedNetLine" ),          TEMPLATE_PERCENTSTACKEDNETLINE )
        ( C2U( "com.sun.star.chart2.template.FilledNet" ),                      TEMPLATE_FILLEDNET )
        ( C2U( "com.sun.star.chart2.template.StackedFilledNet" ),               TEMPLATE_STACKEDFILLEDNET )
        ( C2U( "com.sun.star.chart2.template.PercentStackedFilledNet" ),        TEMPLATE_PERCENTSTACKEDFILLEDNET )
        ( C2U( "com.sun.star.chart2.template.StockLowHighClose" ),              TEMPLATE_STOCKLOWHIGHCLOSE )
        ( C2U( "com.sun.star.chart2.template.StockOpenLowHighClose" ),          TEMPLATE_STOCKOPENLOWHIGHCLOSE )
        ( C2U( "com.sun.star.chart2.template.StockVolumeLowHighClose" ),        TEMPLATE_STOCKVOLUMELOWHIGHCLOSE )
        ( C2U( "com.sun.star.chart2.template.StockVolumeOpenLowHighClose" ),    TEMPLATE_STOCKVOLUMEOPENLOWHIGHCLOSE )
        ( C2U( "com.sun.star.chart2.template.Bubble" ),                         TEMPLATE_BUBBLE )
//      ( C2U( "com.sun.star.chart2.template.Surface" ),                        TEMPLATE_SURFACE )
//      ( C2U( "com.sun.star.chart2.template.Addin" ),                          TEMPLATE_ADDIN )
        );

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
    throw (uno::Exception,
           uno::RuntimeException)
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
        catch( uno::Exception & ex )
        {
            // couldn't create service via factory

            // As XMultiServiceFactory does not specify, what to do in case
            // createInstance is called with an unknown service-name, this
            // function will just return an empty XInterface.
            ASSERT_EXCEPTION( ex );
            OSL_TRACE( "Couldn't instantiate service \"%s\"", U2C( aServiceSpecifier ));
            xResult.set( 0 );
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
                    StackMode_NONE, true, false ));
                break;
            case TEMPLATE_STACKEDSYMBOL:
                xTemplate.set( new LineChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_Y_STACKED, true, false ));
                break;
            case TEMPLATE_PERCENTSTACKEDSYMBOL:
                xTemplate.set( new LineChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_Y_STACKED_PERCENT, true, false ));
                break;
            // Line (category x axis)
            case TEMPLATE_LINE:
                xTemplate.set( new LineChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_NONE, false ));
                break;
            case TEMPLATE_STACKEDLINE:
                xTemplate.set( new LineChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_Y_STACKED, false ));
                break;
            case TEMPLATE_PERCENTSTACKEDLINE:
                xTemplate.set( new LineChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_Y_STACKED_PERCENT, false ));
                break;
            case TEMPLATE_LINESYMBOL:
                xTemplate.set( new LineChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_NONE, true ));
                break;
            case TEMPLATE_STACKEDLINESYMBOL:
                xTemplate.set( new LineChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_Y_STACKED, true ));
                break;
            case TEMPLATE_PERCENTSTACKEDLINESYMBOL:
                xTemplate.set( new LineChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_Y_STACKED_PERCENT, true ));
                break;
            case TEMPLATE_THREEDLINE:
                xTemplate.set( new LineChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_NONE, false, true, 3 ));
                break;
            case TEMPLATE_STACKEDTHREEDLINE:
                xTemplate.set( new LineChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_Y_STACKED, false, true, 3 ));
                break;
            case TEMPLATE_PERCENTSTACKEDTHREEDLINE:
                xTemplate.set( new LineChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_Y_STACKED_PERCENT, false, true, 3 ));
                break;
            case TEMPLATE_THREEDLINEDEEP:
                xTemplate.set( new LineChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_Z_STACKED, false, true, 3 ));
                break;

            // Bar/Column
            case TEMPLATE_COLUMN:
                xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_NONE, BarChartTypeTemplate::VERTICAL ));
                break;
            case TEMPLATE_STACKEDCOLUMN:
                xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_Y_STACKED, BarChartTypeTemplate::VERTICAL ));
                break;
            case TEMPLATE_PERCENTSTACKEDCOLUMN:
                xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_Y_STACKED_PERCENT, BarChartTypeTemplate::VERTICAL ));
                break;
            case TEMPLATE_BAR:
                xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_NONE, BarChartTypeTemplate::HORIZONTAL ));
                break;
            case TEMPLATE_STACKEDBAR:
                xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_Y_STACKED, BarChartTypeTemplate::HORIZONTAL ));
                break;
            case TEMPLATE_PERCENTSTACKEDBAR:
                xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_Y_STACKED_PERCENT, BarChartTypeTemplate::HORIZONTAL ));
                break;
            case TEMPLATE_THREEDCOLUMNDEEP:
                xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_Z_STACKED, BarChartTypeTemplate::VERTICAL, 3 ));
                break;
            case TEMPLATE_THREEDCOLUMNFLAT:
                xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_NONE, BarChartTypeTemplate::VERTICAL, 3 ));
                break;
            case TEMPLATE_STACKEDTHREEDCOLUMNFLAT:
                xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_Y_STACKED, BarChartTypeTemplate::VERTICAL, 3 ));
                break;
            case TEMPLATE_PERCENTSTACKEDTHREEDCOLUMNFLAT:
                xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_Y_STACKED_PERCENT, BarChartTypeTemplate::VERTICAL, 3 ));
                break;
            case TEMPLATE_THREEDBARDEEP:
                xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_Z_STACKED, BarChartTypeTemplate::HORIZONTAL, 3 ));
                break;
            case TEMPLATE_THREEDBARFLAT:
                xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_NONE, BarChartTypeTemplate::HORIZONTAL, 3 ));
                break;
            case TEMPLATE_STACKEDTHREEDBARFLAT:
                xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_Y_STACKED, BarChartTypeTemplate::HORIZONTAL, 3 ));
                break;
            case TEMPLATE_PERCENTSTACKEDTHREEDBARFLAT:
                xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_Y_STACKED_PERCENT, BarChartTypeTemplate::HORIZONTAL, 3 ));
                break;

            // Combi-Chart Line/Column
            case TEMPLATE_COLUMNWITHLINE:
            case TEMPLATE_STACKEDCOLUMNWITHLINE:
            {
                StackMode eMode = ( nId == TEMPLATE_COLUMNWITHLINE )
                    ? StackMode_NONE
                    : StackMode_Y_STACKED;

                xTemplate.set( new ColumnLineChartTypeTemplate( m_xContext, aServiceSpecifier, eMode, 1 ));
            }
            break;

            // Area
            case TEMPLATE_AREA:
                xTemplate.set( new AreaChartTypeTemplate( m_xContext, aServiceSpecifier, StackMode_NONE ));
                break;
            case TEMPLATE_STACKEDAREA:
                xTemplate.set( new AreaChartTypeTemplate( m_xContext, aServiceSpecifier, StackMode_Y_STACKED ));
                break;
            case TEMPLATE_PERCENTSTACKEDAREA:
                xTemplate.set( new AreaChartTypeTemplate( m_xContext, aServiceSpecifier, StackMode_Y_STACKED_PERCENT ));
                break;
            case TEMPLATE_THREEDAREA:
                xTemplate.set( new AreaChartTypeTemplate( m_xContext, aServiceSpecifier, StackMode_Z_STACKED, 3 ));
                break;
            case TEMPLATE_STACKEDTHREEDAREA:
                xTemplate.set( new AreaChartTypeTemplate( m_xContext, aServiceSpecifier, StackMode_Y_STACKED, 3 ));
                break;
            case TEMPLATE_PERCENTSTACKEDTHREEDAREA:
                xTemplate.set( new AreaChartTypeTemplate( m_xContext, aServiceSpecifier, StackMode_Y_STACKED_PERCENT, 3 ));
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
                    StackMode_NONE, true ));
                break;
            case TEMPLATE_NETSYMBOL:
                xTemplate.set( new NetChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_NONE, true, false ));
                break;
            case TEMPLATE_NETLINE:
                xTemplate.set( new NetChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_NONE, false ));
                break;

            case TEMPLATE_STACKEDNET:
                xTemplate.set( new NetChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_Y_STACKED, true ));
                break;
            case TEMPLATE_STACKEDNETSYMBOL:
                xTemplate.set( new NetChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_Y_STACKED, true, false ));
                break;
            case TEMPLATE_STACKEDNETLINE:
                xTemplate.set( new NetChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_Y_STACKED, false, true ));
                break;

            case TEMPLATE_PERCENTSTACKEDNET:
                xTemplate.set( new NetChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_Y_STACKED_PERCENT, true ));
                break;
            case TEMPLATE_PERCENTSTACKEDNETSYMBOL:
                xTemplate.set( new NetChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_Y_STACKED_PERCENT, true, false ));
                break;
            case TEMPLATE_PERCENTSTACKEDNETLINE:
                xTemplate.set( new NetChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_Y_STACKED_PERCENT, false, true ));
                break;

            case TEMPLATE_FILLEDNET:
                xTemplate.set( new NetChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_NONE, false, false, true ));
                break;
            case TEMPLATE_STACKEDFILLEDNET:
                xTemplate.set( new NetChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_Y_STACKED, false, false, true ));
                break;
            case TEMPLATE_PERCENTSTACKEDFILLEDNET:
                xTemplate.set( new NetChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StackMode_Y_STACKED_PERCENT, false, false, true ));
                break;

            case TEMPLATE_STOCKLOWHIGHCLOSE:
                xTemplate.set( new StockChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StockChartTypeTemplate::LOW_HI_CLOSE, false ));
                break;
            case TEMPLATE_STOCKOPENLOWHIGHCLOSE:
                xTemplate.set( new StockChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StockChartTypeTemplate::OPEN_LOW_HI_CLOSE, true ));
                break;
            case TEMPLATE_STOCKVOLUMELOWHIGHCLOSE:
                xTemplate.set( new StockChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StockChartTypeTemplate::VOL_LOW_HI_CLOSE, false ));
                break;
            case TEMPLATE_STOCKVOLUMEOPENLOWHIGHCLOSE:
                xTemplate.set( new StockChartTypeTemplate( m_xContext, aServiceSpecifier,
                    StockChartTypeTemplate::VOL_OPEN_LOW_HI_CLOSE, true ));
                break;

            //BubbleChart
            case TEMPLATE_BUBBLE:
                xTemplate.set( new BubbleChartTypeTemplate( m_xContext, aServiceSpecifier ));
                break;

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
    throw (uno::Exception,
           uno::RuntimeException)
{
    OSL_ENSURE( false, "createInstanceWithArguments: No arguments supported" );
    return createInstance( ServiceSpecifier );
}

uno::Sequence< OUString > SAL_CALL ChartTypeManager::getAvailableServiceNames()
    throw (uno::RuntimeException)
{
    ::std::vector< OUString > aServices;
    const tTemplateMapType & rMap = lcl_DefaultChartTypeMap();
    aServices.reserve( rMap.size());

    // get own default templates
    ::std::transform( rMap.begin(), rMap.end(), ::std::back_inserter( aServices ),
                      ::o3tl::select1st< tTemplateMapType::value_type >());

    // add components that were registered in the context's factory
    uno::Reference< container::XContentEnumerationAccess > xEnumAcc(
        m_xContext->getServiceManager(), uno::UNO_QUERY );
    if( xEnumAcc.is())
    {
        uno::Reference< container::XEnumeration > xEnum(
            xEnumAcc->createContentEnumeration( C2U( "com.sun.star.chart2.ChartTypeTemplate" ) ));
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

    return ContainerHelper::ContainerToSequence( aServices );
}

// ____ XServiceInfo ____
Sequence< OUString > ChartTypeManager::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 2 );
    aServices[ 0 ] = C2U( "com.sun.star.chart2.ChartTypeManager" );
    aServices[ 1 ] = C2U( "com.sun.star.lang.MultiServiceFactory" );
    return aServices;
}

// ================================================================================

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( ChartTypeManager,
                             C2U( "com.sun.star.comp.chart.ChartTypeManager" ));
} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
