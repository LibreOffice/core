/*************************************************************************
 *
 *  $RCSfile: ChartTypeManager.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: bm $ $Date: 2003-11-19 16:50:12 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "ChartTypeManager.hxx"
#include "PropertyHelper.hxx"
#include "algohelper.hxx"
#include "macros.hxx"
#include "ContextHelper.hxx"

#include "Scaling.hxx"
#include "CartesianCoordinateSystem.hxx"
#include "DoubleStackedBarChartTypeTemplate.hxx"

#include "LineChartTypeTemplate.hxx"
#include "BarChartTypeTemplate.hxx"
#include "ColumnLineChartTypeTemplate.hxx"
#include "AreaChartTypeTemplate.hxx"
#include "PieChartTypeTemplate.hxx"

#ifndef _CPPUHELPER_COMPONENT_CONTEXT_HXX_
#include <cppuhelper/component_context.hxx>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XCONTENTENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICENAME_HPP_
#include <com/sun/star/lang/XServiceName.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTSOLIDTYPE_HPP_
#include <com/sun/star/chart/ChartSolidType.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XCHARTTYPEGROUP_HPP_
#include <drafts/com/sun/star/chart2/XChartTypeGroup.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XSTACKABLESCALEGROUP_HPP_
#include <drafts/com/sun/star/chart2/XStackableScaleGroup.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_STACKMODE_HPP_
#include <drafts/com/sun/star/chart2/StackMode.hpp>
#endif
#include <drafts/com/sun/star/chart2/CurveStyle.hpp>

#include <algorithm>
#include <iterator>
#include <functional>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans::PropertyAttribute;
using namespace ::drafts::com::sun::star;

using ::rtl::OUString;
using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::osl::MutexGuard;

// ======================================================================

namespace
{
enum
{
    PROP_TEMPLATE_MANAGER_CHART_TEMPLATE_NAME,
    PROP_TEMPLATE_MANAGER_SOLID_TYPE,
    PROP_TEMPLATE_MANAGER_NUMBER_OF_LINES,
    PROP_TEMPLATE_MANAGER_SPLINE_ORDER,
    PROP_TEMPLATE_MANAGER_SPLINE_RESOLUTION
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    // preliminary
    rOutProperties.push_back(
        Property( C2U( "ChartStyleTemplateServiceName" ),
                  PROP_TEMPLATE_MANAGER_CHART_TEMPLATE_NAME,
                  ::getCppuType( reinterpret_cast< const OUString * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    // properties for the chart type dialog
    rOutProperties.push_back(
        Property( C2U( "SolidType" ),
                  PROP_TEMPLATE_MANAGER_SOLID_TYPE,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "NumberOfLines" ),
                  PROP_TEMPLATE_MANAGER_NUMBER_OF_LINES,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "SplineOrder" ),
                  PROP_TEMPLATE_MANAGER_SPLINE_ORDER,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "SplineResolution" ),
                  PROP_TEMPLATE_MANAGER_SPLINE_RESOLUTION,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

void lcl_AddDefaultsToMap(
    ::chart::helper::tPropertyValueMap & rOutMap )
{
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_TEMPLATE_MANAGER_CHART_TEMPLATE_NAME ));
    rOutMap[ PROP_TEMPLATE_MANAGER_CHART_TEMPLATE_NAME ] =
        uno::makeAny(
            ::rtl::OUString( C2U( "com.sun.star.chart2.template.Column" )));

    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_TEMPLATE_MANAGER_SOLID_TYPE ));
    rOutMap[ PROP_TEMPLATE_MANAGER_SOLID_TYPE ] =
        uno::makeAny( ::com::sun::star::chart::ChartSolidType::RECTANGULAR_SOLID );

    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_TEMPLATE_MANAGER_NUMBER_OF_LINES ));
    rOutMap[ PROP_TEMPLATE_MANAGER_NUMBER_OF_LINES ] =
        uno::makeAny( sal_Int32( 0 ));

    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_TEMPLATE_MANAGER_SPLINE_ORDER ));
    rOutMap[ PROP_TEMPLATE_MANAGER_SPLINE_ORDER ] =
        uno::makeAny( sal_Int32( 3 ));

    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_TEMPLATE_MANAGER_SPLINE_RESOLUTION ));
    rOutMap[ PROP_TEMPLATE_MANAGER_SPLINE_RESOLUTION ] =
        uno::makeAny( sal_Int32( 20 ));
}

const Sequence< Property > & lcl_GetPropertySequence()
{
    static Sequence< Property > aPropSeq;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aPropSeq.getLength() )
    {
        // get properties
        ::std::vector< ::com::sun::star::beans::Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );

        // and sort them for access via bsearch
        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::helper::PropertyNameLess() );

        // transfer result to static Sequence
        aPropSeq = ::chart::helper::VectorToSequence( aProperties );
    }

    return aPropSeq;
}

enum TemplateId
{
    TEMPLATE_LINE,
    TEMPLATE_STACKEDLINE,
    TEMPLATE_PERCENTSTACKEDLINE,
    TEMPLATE_LINESYMBOL,
    TEMPLATE_STACKEDLINESYMBOL,
    TEMPLATE_PERCENTSTACKEDLINESYMBOL,
    TEMPLATE_CUBICSPLINE,
    TEMPLATE_CUBICSPLINESYMBOL,
    TEMPLATE_BSPLINE,
    TEMPLATE_BSPLINESYMBOL,
    TEMPLATE_THREEDLINE,
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
    TEMPLATE_PIEONEEXPLODED,
    TEMPLATE_PIEALLEXPLODED,
    TEMPLATE_RING,
    TEMPLATE_THREEDPIE,
    TEMPLATE_SCATTERLINESYMBOL,
    TEMPLATE_CUBICSPLINESCATTER,
    TEMPLATE_CUBICSPLINESCATTERSYMBOL,
    TEMPLATE_BSPLINESCATTER,
    TEMPLATE_BSPLINESCATTERSYMBOL,
    TEMPLATE_SCATTERLINE,
    TEMPLATE_SCATTERSYMBOL,
    TEMPLATE_NET,
    TEMPLATE_NETSYMBOL,
    TEMPLATE_STACKEDNET,
    TEMPLATE_STACKEDNETSYMBOL,
    TEMPLATE_PERCENTSTACKEDNET,
    TEMPLATE_PERCENTSTACKEDNETSYMBOL,
    TEMPLATE_STOCKLOWHIGHCLOSE,
    TEMPLATE_STOCKOPENLOWHIGHCLOSE,
    TEMPLATE_STOCKVOLUMELOWHIGHCLOSE,
    TEMPLATE_STOCKVOLUMEOPENLOWHIGHCLOSE,
    TEMPLATE_SURFACE,
    TEMPLATE_THREEDSCATTER,
    TEMPLATE_THREEDSCATTERSYMBOL,
    TEMPLATE_ADDIN,
    TEMPLATE_NOT_FOUND = 0xffff
};

typedef ::std::map< OUString, TemplateId > tTemplateMapType;

const tTemplateMapType & lcl_DefaultChartTypeMap()
{
    static const tTemplateMapType aMap = tTemplateMapType(
        ::comphelper::MakeMap< tTemplateMapType::key_type, tTemplateMapType::data_type >
        ( C2U( "com.sun.star.chart2.template.Line" ),                           TEMPLATE_LINE )
        ( C2U( "com.sun.star.chart2.template.StackedLine" ),                    TEMPLATE_STACKEDLINE )
        ( C2U( "com.sun.star.chart2.template.PercentStackedLine" ),             TEMPLATE_PERCENTSTACKEDLINE )
        ( C2U( "com.sun.star.chart2.template.LineSymbol" ),                     TEMPLATE_LINESYMBOL )
        ( C2U( "com.sun.star.chart2.template.StackedLineSymbol" ),              TEMPLATE_STACKEDLINESYMBOL )
        ( C2U( "com.sun.star.chart2.template.PercentStackedLineSymbol" ),       TEMPLATE_PERCENTSTACKEDLINESYMBOL )
        ( C2U( "com.sun.star.chart2.template.CubicSpline" ),                    TEMPLATE_CUBICSPLINE )
        ( C2U( "com.sun.star.chart2.template.CubicSplineSymbol" ),              TEMPLATE_CUBICSPLINESYMBOL )
        ( C2U( "com.sun.star.chart2.template.BSpline" ),                        TEMPLATE_BSPLINE )
        ( C2U( "com.sun.star.chart2.template.BSplineSymbol" ),                  TEMPLATE_BSPLINESYMBOL )
        ( C2U( "com.sun.star.chart2.template.ThreeDLine" ),                     TEMPLATE_THREEDLINE )
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
        ( C2U( "com.sun.star.chart2.template.PieOneExploded" ),                 TEMPLATE_PIEONEEXPLODED )
        ( C2U( "com.sun.star.chart2.template.PieAllExploded" ),                 TEMPLATE_PIEALLEXPLODED )
        ( C2U( "com.sun.star.chart2.template.Ring" ),                           TEMPLATE_RING )
        ( C2U( "com.sun.star.chart2.template.ThreeDPie" ),                      TEMPLATE_THREEDPIE )
        ( C2U( "com.sun.star.chart2.template.ScatterLineSymbol" ),              TEMPLATE_SCATTERLINESYMBOL )
        ( C2U( "com.sun.star.chart2.template.CubicSplineScatter" ),             TEMPLATE_CUBICSPLINESCATTER )
        ( C2U( "com.sun.star.chart2.template.CubicSplineScatterSymbol" ),       TEMPLATE_CUBICSPLINESCATTERSYMBOL )
        ( C2U( "com.sun.star.chart2.template.BSplineScatter" ),                 TEMPLATE_BSPLINESCATTER )
        ( C2U( "com.sun.star.chart2.template.BSplineScatterSymbol" ),           TEMPLATE_BSPLINESCATTERSYMBOL )
        ( C2U( "com.sun.star.chart2.template.ScatterLine" ),                    TEMPLATE_SCATTERLINE )
        ( C2U( "com.sun.star.chart2.template.ScatterSymbol" ),                  TEMPLATE_SCATTERSYMBOL )
        ( C2U( "com.sun.star.chart2.template.Net" ),                            TEMPLATE_NET )
        ( C2U( "com.sun.star.chart2.template.NetSymbol" ),                      TEMPLATE_NETSYMBOL )
        ( C2U( "com.sun.star.chart2.template.StackedNet" ),                     TEMPLATE_STACKEDNET )
        ( C2U( "com.sun.star.chart2.template.StackedNetSymbol" ),               TEMPLATE_STACKEDNETSYMBOL )
        ( C2U( "com.sun.star.chart2.template.PercentStackedNet" ),              TEMPLATE_PERCENTSTACKEDNET )
        ( C2U( "com.sun.star.chart2.template.PercentStackedNetSymbol" ),        TEMPLATE_PERCENTSTACKEDNETSYMBOL )
        ( C2U( "com.sun.star.chart2.template.StockLowHighClose" ),              TEMPLATE_STOCKLOWHIGHCLOSE )
        ( C2U( "com.sun.star.chart2.template.StockOpenLowHighClose" ),          TEMPLATE_STOCKOPENLOWHIGHCLOSE )
        ( C2U( "com.sun.star.chart2.template.StockVolumeLowHighClose" ),        TEMPLATE_STOCKVOLUMELOWHIGHCLOSE )
        ( C2U( "com.sun.star.chart2.template.StockVolumeOpenLowHighClose" ),    TEMPLATE_STOCKVOLUMEOPENLOWHIGHCLOSE )
        ( C2U( "com.sun.star.chart2.template.Surface" ),                        TEMPLATE_SURFACE )
        ( C2U( "com.sun.star.chart2.template.ThreeDScatter" ),                  TEMPLATE_THREEDSCATTER )
        ( C2U( "com.sun.star.chart2.template.ThreeDScatterSymbol" ),            TEMPLATE_THREEDSCATTERSYMBOL )
        ( C2U( "com.sun.star.chart2.template.Addin" ),                          TEMPLATE_ADDIN )
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
        ::property::OPropertySet( m_aMutex ),
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
        xResult = m_xContext->getServiceManager()->createInstanceWithContext(
            aServiceSpecifier, m_xContext );
    }
    else
    {
        uno::Reference< chart2::XChartTypeTemplate > xTemplate;
        switch( nId )
        {
            // Line
            case TEMPLATE_LINE:
                xTemplate.set( new LineChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::StackMode_NONE, chart2::CurveStyle_LINES, false ));
                break;
            case TEMPLATE_STACKEDLINE:
                xTemplate.set( new LineChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::StackMode_STACKED, chart2::CurveStyle_LINES, false ));
                break;
            case TEMPLATE_PERCENTSTACKEDLINE:
                xTemplate.set( new LineChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::StackMode_STACKED_PERCENT, chart2::CurveStyle_LINES, false ));
                break;
            case TEMPLATE_LINESYMBOL:
                xTemplate.set( new LineChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::StackMode_NONE, chart2::CurveStyle_LINES, true ));
                break;
            case TEMPLATE_STACKEDLINESYMBOL:
                xTemplate.set( new LineChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::StackMode_STACKED, chart2::CurveStyle_LINES, true ));
                break;
            case TEMPLATE_PERCENTSTACKEDLINESYMBOL:
                xTemplate.set( new LineChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::StackMode_STACKED_PERCENT, chart2::CurveStyle_LINES, true ));
                break;
            case TEMPLATE_CUBICSPLINE:
                xTemplate.set( new LineChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::StackMode_NONE, chart2::CurveStyle_CUBIC_SPLINES, false ));
                break;
            case TEMPLATE_CUBICSPLINESYMBOL:
                xTemplate.set( new LineChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::StackMode_NONE, chart2::CurveStyle_CUBIC_SPLINES, true ));
                break;
            case TEMPLATE_BSPLINE:
                xTemplate.set( new LineChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::StackMode_NONE, chart2::CurveStyle_B_SPLINES, false ));
                break;
            case TEMPLATE_BSPLINESYMBOL:
                xTemplate.set( new LineChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::StackMode_NONE, chart2::CurveStyle_B_SPLINES, true ));
                break;
            case TEMPLATE_THREEDLINE:
                xTemplate.set( new LineChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::StackMode_NONE, chart2::CurveStyle_LINES, false, 3 ));
                break;

            // Bar/Column
            case TEMPLATE_COLUMN:
                xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::StackMode_NONE, BarChartTypeTemplate::VERTICAL ));
                break;
            case TEMPLATE_STACKEDCOLUMN:
                xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::StackMode_STACKED, BarChartTypeTemplate::VERTICAL ));
                break;
            case TEMPLATE_PERCENTSTACKEDCOLUMN:
                xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::StackMode_STACKED_PERCENT, BarChartTypeTemplate::VERTICAL ));
                break;
            case TEMPLATE_BAR:
                xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::StackMode_NONE, BarChartTypeTemplate::HORIZONTAL ));
                break;
            case TEMPLATE_STACKEDBAR:
                xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::StackMode_STACKED, BarChartTypeTemplate::HORIZONTAL ));
                break;
            case TEMPLATE_PERCENTSTACKEDBAR:
                xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::StackMode_STACKED_PERCENT, BarChartTypeTemplate::HORIZONTAL ));
                break;
            case TEMPLATE_THREEDCOLUMNDEEP:
                xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::StackMode_NONE, BarChartTypeTemplate::VERTICAL,
                    3, BarChartTypeTemplate::THREE_D_DEEP ));
                break;
            case TEMPLATE_THREEDCOLUMNFLAT:
                xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::StackMode_NONE, BarChartTypeTemplate::VERTICAL,
                    3, BarChartTypeTemplate::THREE_D_FLAT ));
                break;
            case TEMPLATE_STACKEDTHREEDCOLUMNFLAT:
                xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::StackMode_STACKED, BarChartTypeTemplate::VERTICAL,
                    3, BarChartTypeTemplate::THREE_D_FLAT ));
            case TEMPLATE_PERCENTSTACKEDTHREEDCOLUMNFLAT:
                xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::StackMode_STACKED_PERCENT, BarChartTypeTemplate::VERTICAL,
                    3, BarChartTypeTemplate::THREE_D_FLAT ));
            case TEMPLATE_THREEDBARDEEP:
                xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::StackMode_NONE, BarChartTypeTemplate::HORIZONTAL,
                    3, BarChartTypeTemplate::THREE_D_DEEP ));
                break;
            case TEMPLATE_THREEDBARFLAT:
                xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::StackMode_NONE, BarChartTypeTemplate::HORIZONTAL,
                    3, BarChartTypeTemplate::THREE_D_FLAT ));
                break;
            case TEMPLATE_STACKEDTHREEDBARFLAT:
                xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::StackMode_STACKED, BarChartTypeTemplate::HORIZONTAL,
                    3, BarChartTypeTemplate::THREE_D_FLAT ));
                break;
            case TEMPLATE_PERCENTSTACKEDTHREEDBARFLAT:
                xTemplate.set( new BarChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::StackMode_STACKED_PERCENT, BarChartTypeTemplate::HORIZONTAL,
                    3, BarChartTypeTemplate::THREE_D_FLAT ));
                break;

            // Combi-Chart Line/Column
            case TEMPLATE_COLUMNWITHLINE:
            case TEMPLATE_STACKEDCOLUMNWITHLINE:
            {
                chart2::StackMode eMode = ( nId == TEMPLATE_COLUMNWITHLINE )
                    ? chart2::StackMode_NONE
                    : chart2::StackMode_STACKED;
                sal_Int32 nNumOfLines = 1;
                getFastPropertyValue( PROP_TEMPLATE_MANAGER_NUMBER_OF_LINES ) >>= nNumOfLines;

                xTemplate.set( new ColumnLineChartTypeTemplate( m_xContext, aServiceSpecifier, eMode, nNumOfLines ));
            }
            break;

            // Area
            case TEMPLATE_AREA:
                xTemplate.set( new AreaChartTypeTemplate( m_xContext, aServiceSpecifier, chart2::StackMode_NONE ));
                break;
            case TEMPLATE_STACKEDAREA:
                xTemplate.set( new AreaChartTypeTemplate( m_xContext, aServiceSpecifier, chart2::StackMode_STACKED ));
                break;
            case TEMPLATE_PERCENTSTACKEDAREA:
                xTemplate.set( new AreaChartTypeTemplate( m_xContext, aServiceSpecifier, chart2::StackMode_STACKED_PERCENT ));
                break;
            case TEMPLATE_THREEDAREA:
                xTemplate.set( new AreaChartTypeTemplate( m_xContext, aServiceSpecifier, chart2::StackMode_NONE, 3 ));
                break;
            case TEMPLATE_STACKEDTHREEDAREA:
                xTemplate.set( new AreaChartTypeTemplate( m_xContext, aServiceSpecifier, chart2::StackMode_STACKED, 3 ));
                break;
            case TEMPLATE_PERCENTSTACKEDTHREEDAREA:
                xTemplate.set( new AreaChartTypeTemplate( m_xContext, aServiceSpecifier, chart2::StackMode_STACKED_PERCENT, 3 ));
                break;

            case TEMPLATE_PIE:
                xTemplate.set( new PieChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::PieChartOffsetMode_NONE, false ));
                break;
            case TEMPLATE_PIEONEEXPLODED:
                xTemplate.set( new PieChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::PieChartOffsetMode_FIRST_EXPLODED, false ));
                break;
            case TEMPLATE_PIEALLEXPLODED:
                xTemplate.set( new PieChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::PieChartOffsetMode_ALL_EXPLODED, false ));
                break;
            case TEMPLATE_RING:
                xTemplate.set( new PieChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::PieChartOffsetMode_NONE, true ));
                break;
            case TEMPLATE_THREEDPIE:
                xTemplate.set( new PieChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::PieChartOffsetMode_NONE, false, 3 ));
                break;

            case TEMPLATE_SCATTERLINESYMBOL:
            case TEMPLATE_CUBICSPLINESCATTER:
            case TEMPLATE_CUBICSPLINESCATTERSYMBOL:
            case TEMPLATE_BSPLINESCATTER:
            case TEMPLATE_BSPLINESCATTERSYMBOL:
            case TEMPLATE_SCATTERLINE:
            case TEMPLATE_SCATTERSYMBOL:

            // TEST !!!
            case TEMPLATE_NET:
                xTemplate.set( new DoubleStackedBarChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::StackMode_NONE, chart2::StackMode_STACKED ));
                break;

            // TEST !!!
            case TEMPLATE_NETSYMBOL:
            case TEMPLATE_STACKEDNET:
            case TEMPLATE_STACKEDNETSYMBOL:
                xTemplate.set( new DoubleStackedBarChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::StackMode_STACKED, chart2::StackMode_STACKED ));
                break;

            // TEST !!!
            case TEMPLATE_PERCENTSTACKEDNET:
            case TEMPLATE_PERCENTSTACKEDNETSYMBOL:
                xTemplate.set( new DoubleStackedBarChartTypeTemplate( m_xContext, aServiceSpecifier,
                    chart2::StackMode_STACKED_PERCENT, chart2::StackMode_STACKED ));
                break;

            case TEMPLATE_STOCKLOWHIGHCLOSE:
            case TEMPLATE_STOCKOPENLOWHIGHCLOSE:
            case TEMPLATE_STOCKVOLUMELOWHIGHCLOSE:
            case TEMPLATE_STOCKVOLUMEOPENLOWHIGHCLOSE:
            case TEMPLATE_SURFACE:
            case TEMPLATE_THREEDSCATTER:
            case TEMPLATE_THREEDSCATTERSYMBOL:
            case TEMPLATE_ADDIN:
                break;

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
    const uno::Sequence< uno::Any >& Arguments )
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
                      ::std::select1st< tTemplateMapType::value_type >());

    // add components that were registered in the context's factory
    uno::Reference< container::XContentEnumerationAccess > xEnumAcc(
        m_xContext->getServiceManager(), uno::UNO_QUERY );
    if( xEnumAcc.is())
    {
        uno::Reference< container::XEnumeration > xEnum(
            xEnumAcc->createContentEnumeration( C2U( "drafts.com.sun.star.chart2.ChartTypeTemplate" ) ));
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

    return helper::VectorToSequence( aServices );
}

// ____ XServiceInfo ____
Sequence< OUString > ChartTypeManager::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 3 );
    aServices[ 0 ] = C2U( "drafts.com.sun.star.chart2.ChartTypeManager" );
    aServices[ 1 ] = C2U( "drafts.com.sun.star.lang.MultiServiceFactory" );
    aServices[ 2 ] = C2U( "com.sun.star.beans.PropertySet" );
    return aServices;
}

// ____ OPropertySet ____
uno::Any ChartTypeManager::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    static helper::tPropertyValueMap aStaticDefaults;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aStaticDefaults.size() )
    {
        // initialize defaults
        lcl_AddDefaultsToMap( aStaticDefaults );
    }

    helper::tPropertyValueMap::const_iterator aFound(
        aStaticDefaults.find( nHandle ));

    if( aFound == aStaticDefaults.end())
        return uno::Any();

    return (*aFound).second;
    // \--
}

// ____ OPropertySet ____
::cppu::IPropertyArrayHelper & SAL_CALL ChartTypeManager::getInfoHelper()
{
    static ::cppu::OPropertyArrayHelper aArrayHelper( lcl_GetPropertySequence(),
                                                      /* bSorted = */ sal_True );

    return aArrayHelper;
}


// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL
    ChartTypeManager::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( !xInfo.is())
    {
        xInfo = ::cppu::OPropertySetHelper::createPropertySetInfo(
            getInfoHelper());
    }

    return xInfo;
    // \--
}

// ================================================================================

using impl::ChartTypeManager_Base;

IMPLEMENT_FORWARD_XINTERFACE2( ChartTypeManager, ChartTypeManager_Base, ::property::OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( ChartTypeManager, ChartTypeManager_Base, ::property::OPropertySet )

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( ChartTypeManager,
                             C2U( "com.sun.star.comp.chart.ChartTypeManager" ));
} //  namespace chart
