/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StockChartTypeTemplate.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:24:53 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include "StockChartTypeTemplate.hxx"
#include "LineChartType.hxx"
#include "macros.hxx"
#include "algohelper.hxx"
#include "DataSeriesTreeHelper.hxx"
#include "DataSeriesHelper.hxx"
#include "BarChartType.hxx"

#ifndef _COM_SUN_STAR_CHART2_SYMBOLSTYLE_HPP_
#include <com/sun/star/chart2/SymbolStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDATASEQUENCE_HPP_
#include <com/sun/star/chart2/XDataSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDATASOURCE_HPP_
#include <com/sun/star/chart2/XDataSource.hpp>
#endif

#ifndef CHART_PROPERTYHELPER_HXX
#include "PropertyHelper.hxx"
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

#include <algorithm>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;
using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::osl::MutexGuard;

// ----------------------------------------
namespace
{

static const OUString lcl_aServiceName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.StockChartTypeTemplate" ));

enum
{
    PROP_STOCKCHARTTYPE_TEMPLATE_VOLUME,
    PROP_STOCKCHARTTYPE_TEMPLATE_OPEN,
    PROP_STOCKCHARTTYPE_TEMPLATE_LOW_HIGH,
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( C2U( "Volume" ),
                  PROP_STOCKCHARTTYPE_TEMPLATE_VOLUME,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "Open" ),
                  PROP_STOCKCHARTTYPE_TEMPLATE_OPEN,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "LowHigh" ),
                  PROP_STOCKCHARTTYPE_TEMPLATE_LOW_HIGH,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

void lcl_AddDefaultsToMap(
    ::chart::helper::tPropertyValueMap & rOutMap )
{
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_STOCKCHARTTYPE_TEMPLATE_VOLUME ));
    rOutMap[ PROP_STOCKCHARTTYPE_TEMPLATE_VOLUME ] =
        uno::makeAny( sal_False );
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_STOCKCHARTTYPE_TEMPLATE_OPEN ));
    rOutMap[ PROP_STOCKCHARTTYPE_TEMPLATE_OPEN ] =
        uno::makeAny( sal_False );
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_STOCKCHARTTYPE_TEMPLATE_LOW_HIGH ));
    rOutMap[ PROP_STOCKCHARTTYPE_TEMPLATE_LOW_HIGH ] =
        uno::makeAny( sal_True );
}

const Sequence< Property > & lcl_GetPropertySequence()
{
    static Sequence< Property > aPropSeq;

    // /--
    MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
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

::cppu::IPropertyArrayHelper & lcl_getInfoHelper()
{
    static ::cppu::OPropertyArrayHelper aArrayHelper(
        lcl_GetPropertySequence(),
        /* bSorted = */ sal_True );

    return aArrayHelper;
}

} // anonymous namespace
// ----------------------------------------

namespace chart
{

StockChartTypeTemplate::StockChartTypeTemplate(
    uno::Reference<
        uno::XComponentContext > const & xContext,
    const ::rtl::OUString & rServiceName,
    StockVariant eVariant ) :
        ChartTypeTemplate( xContext, rServiceName ),
        ::property::OPropertySet( m_aMutex )
{
    setFastPropertyValue_NoBroadcast(
        PROP_STOCKCHARTTYPE_TEMPLATE_OPEN,
        uno::makeAny( sal_Bool( eVariant == OPEN_LOW_HI_CLOSE ||
                                eVariant == VOL_OPEN_LOW_HI_CLOSE )));
    setFastPropertyValue_NoBroadcast(
        PROP_STOCKCHARTTYPE_TEMPLATE_VOLUME,
        uno::makeAny( sal_Bool( eVariant == VOL_LOW_HI_CLOSE ||
                                eVariant == VOL_OPEN_LOW_HI_CLOSE )));
}

StockChartTypeTemplate::~StockChartTypeTemplate()
{}
// ____ OPropertySet ____
uno::Any StockChartTypeTemplate::GetDefaultValue( sal_Int32 nHandle ) const
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

::cppu::IPropertyArrayHelper & SAL_CALL StockChartTypeTemplate::getInfoHelper()
{
    return lcl_getInfoHelper();
}


// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL
    StockChartTypeTemplate::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo;

    // /--
    MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( !xInfo.is())
    {
        xInfo = ::cppu::OPropertySetHelper::createPropertySetInfo(
            getInfoHelper());
    }

    return xInfo;
    // \--
}

uno::Reference< chart2::XChartType > StockChartTypeTemplate::getDefaultChartType()
    throw (uno::RuntimeException)
{
    return new LineChartType( 2, chart2::CurveStyle_LINES );
}

Reference< chart2::XDataSeriesTreeParent > StockChartTypeTemplate::createDataSeriesTree(
    const Sequence< Reference< chart2::XDataSeries > > & aSeriesSeq,
    const Reference< chart2::XBoundedCoordinateSystem > & rCoordSys )
{
    // create series tree nodes
    // root
    Reference< chart2::XDataSeriesTreeParent > aRoot( createRootNode());

    bool bHasVolume, bHasOpenValue, bHasLowHighValues;

    getFastPropertyValue( PROP_STOCKCHARTTYPE_TEMPLATE_VOLUME ) >>= bHasVolume;
    getFastPropertyValue( PROP_STOCKCHARTTYPE_TEMPLATE_OPEN ) >>= bHasOpenValue;
    getFastPropertyValue( PROP_STOCKCHARTTYPE_TEMPLATE_LOW_HIGH ) >>= bHasLowHighValues;

    // Bars (Volume)
    // -------------
    // chart type group
    if( bHasVolume )
    {
        Reference< chart2::XDataSeries > xVolumeSeries;
        for( sal_Int32 i = 0; i < aSeriesSeq.getLength(); ++i )
        {
            Reference< chart2::XDataSequence > xMatch(
                DataSeriesHelper::getDataSequenceByRole(
                    uno::Reference< chart2::XDataSource >( aSeriesSeq[i], uno::UNO_QUERY ),
                    C2U( "volume" )));
            if( xMatch.is() )
            {
                xVolumeSeries = aSeriesSeq[i];
                break;
            }
        }
        Reference< chart2::XDataSeriesTreeNode > aBarNode(
            createChartTypeGroup( new BarChartType() ));

        // 'x-axis' group
        Reference< chart2::XDataSeriesTreeNode > aBarCategoryNode(
            createScaleGroup( true /* bIsDiscrete */,
                              true /* bIsStackable */,
                              rCoordSys, 0, chart2::StackMode_STACKED ));

        // 'y-axis' group
        Reference< chart2::XDataSeriesTreeNode > aBarValueNode(
            createScaleGroup( false /* bIsDiscrete */,
                              true  /* bIsStackable */,
                              rCoordSys, 1, chart2::StackMode_NONE ));

        Sequence< Reference< chart2::XDataSeries > > aBarSeq( 1 );
        aBarSeq[0] = xVolumeSeries;
        addDataSeriesToGroup( aBarValueNode, aBarSeq );

        // add value nodes to category nodes
        attachNodeToNode( aBarCategoryNode, aBarValueNode );

        // add category node to chart type node
        attachNodeToNode( aBarNode, aBarCategoryNode );

        // add chart type nodes to root of tree
        aRoot->addChild( aBarNode );
    }

    // Lines
    // -----
    // chart type group
    Reference< chart2::XDataSeriesTreeNode > aLineNode(
        createChartTypeGroup( new LineChartType() ));

    // 'x-axis' group
    Reference< chart2::XDataSeriesTreeNode > aLineCategoryNode(
        createScaleGroup( true, false, rCoordSys, 0, chart2::StackMode_STACKED ));

    // 'y-axis' group
    Reference< chart2::XDataSeriesTreeNode > aLineValueNode(
        createScaleGroup( false, false, rCoordSys, 1, chart2::StackMode_NONE ));

    // Build Tree
    // ----------

    // add series node to value nodes
//     Sequence< Reference< chart2::XDataSeries > > aLineSeq( nNumberOfLines );
//     ::std::copy( aSeriesSeq.getConstArray() + nNumberOfBars,
//                  aSeriesSeq.getConstArray() + aSeriesSeq.getLength(),
//                  aLineSeq.getArray());
//     addDataSeriesToGroup( aLineValueNode, aLineSeq );

    // add value nodes to category nodes
//     attachNodeToNode( aLineCategoryNode, aLineValueNode );

    // add category node to chart type node
//     attachNodeToNode( aLineNode, aLineCategoryNode );

    // add chart type nodes to root of tree
//     aRoot->addChild( aLineNode );

    return aRoot;
}

// ____ XChartTypeTemplate ____
sal_Bool SAL_CALL StockChartTypeTemplate::matchesTemplate(
    const uno::Reference< chart2::XDiagram >& xDiagram )
    throw (uno::RuntimeException)
{
    sal_Bool bResult = ChartTypeTemplate::matchesTemplate( xDiagram );
    // todo: check for stock chart

    return bResult;
}

// ----------------------------------------

Sequence< OUString > StockChartTypeTemplate::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 2 );
    aServices[ 0 ] = lcl_aServiceName;
    aServices[ 1 ] = C2U( "com.sun.star.chart2.ChartTypeTemplate" );
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( StockChartTypeTemplate, lcl_aServiceName );

IMPLEMENT_FORWARD_XINTERFACE2( StockChartTypeTemplate, ChartTypeTemplate, OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( StockChartTypeTemplate, ChartTypeTemplate, OPropertySet )

} //  namespace chart
