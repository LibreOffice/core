/*************************************************************************
 *
 *  $RCSfile: ColumnLineChartTypeTemplate.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: bm $ $Date: 2003-11-21 14:20:12 $
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
#include "ColumnLineChartTypeTemplate.hxx"
#include "macros.hxx"
#include "LineChartType.hxx"
#include "BarChartType.hxx"
#include "algohelper.hxx"
#include "DataSeriesTreeHelper.hxx"

#ifndef CHART_PROPERTYHELPER_HXX
#include "PropertyHelper.hxx"
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

#include <algorithm>

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;
using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::osl::MutexGuard;

namespace
{

static const ::rtl::OUString lcl_aServiceName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.ColumnLineChartTypeTemplate" ));

enum
{
    PROP_COL_LINE_NUMBER_OF_LINES
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( C2U( "NumberOfLines" ),
                  PROP_COL_LINE_NUMBER_OF_LINES,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

void lcl_AddDefaultsToMap(
    ::chart::helper::tPropertyValueMap & rOutMap )
{
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_COL_LINE_NUMBER_OF_LINES ));
    rOutMap[ PROP_COL_LINE_NUMBER_OF_LINES ] =
        uno::makeAny( sal_Int32( 1 ) );
}

const uno::Sequence< Property > & lcl_GetPropertySequence()
{
    static uno::Sequence< Property > aPropSeq;

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

namespace chart
{

ColumnLineChartTypeTemplate::ColumnLineChartTypeTemplate(
    Reference<
        uno::XComponentContext > const & xContext,
    const ::rtl::OUString & rServiceName,
    chart2::StackMode eStackMode,
    sal_Int32 nNumberOfLines ) :
        ChartTypeTemplate( xContext, rServiceName ),
        ::property::OPropertySet( m_aMutex ),
        m_eStackMode( eStackMode )
{
    setFastPropertyValue_NoBroadcast( PROP_COL_LINE_NUMBER_OF_LINES, uno::makeAny( nNumberOfLines ));
}

ColumnLineChartTypeTemplate::~ColumnLineChartTypeTemplate()
{}

// ____ OPropertySet ____
uno::Any ColumnLineChartTypeTemplate::GetDefaultValue( sal_Int32 nHandle ) const
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

::cppu::IPropertyArrayHelper & SAL_CALL ColumnLineChartTypeTemplate::getInfoHelper()
{
    return lcl_getInfoHelper();
}


// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL
    ColumnLineChartTypeTemplate::getPropertySetInfo()
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

Reference< chart2::XDataSeriesTreeParent > ColumnLineChartTypeTemplate::createDataSeriesTree(
    const Sequence< Reference< chart2::XDataSeries > > & aSeriesSeq,
    const Reference< chart2::XBoundedCoordinateSystem > & rCoordSys )
{
    // create series tree nodes
    // root
    Reference< chart2::XDataSeriesTreeParent > aRoot( createRootNode());

    sal_Int32 nNumberOfBars = aSeriesSeq.getLength();
    sal_Int32 nNumberOfLines = 1;
    getFastPropertyValue( PROP_COL_LINE_NUMBER_OF_LINES ) >>= nNumberOfLines;

    if( nNumberOfLines >= nNumberOfBars )
    {
        nNumberOfLines = 1;
        nNumberOfBars -= 1;
    }
    else
        nNumberOfBars -= nNumberOfLines;

    // Bars
    // ----
    // chart type group
    Reference< chart2::XDataSeriesTreeNode > aBarNode(
        createChartTypeGroup( new BarChartType() ));

    // 'x-axis' group
    Reference< chart2::XDataSeriesTreeNode > aBarCategoryNode(
        createScaleGroup( true, true, rCoordSys, 0, chart2::StackMode_STACKED ));

    // 'y-axis' group
    Reference< chart2::XDataSeriesTreeNode > aBarValueNode(
        createScaleGroup( false, true, rCoordSys, 1, m_eStackMode ));

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
    Sequence< Reference< chart2::XDataSeries > > aBarSeq( nNumberOfBars );
    ::std::copy( aSeriesSeq.getConstArray(),
                 aSeriesSeq.getConstArray() + nNumberOfBars,
                 aBarSeq.getArray());
    addDataSeriesToGroup( aBarValueNode, aBarSeq );

    Sequence< Reference< chart2::XDataSeries > > aLineSeq( nNumberOfLines );
    ::std::copy( aSeriesSeq.getConstArray() + nNumberOfBars,
                 aSeriesSeq.getConstArray() + aSeriesSeq.getLength(),
                 aLineSeq.getArray());
    addDataSeriesToGroup( aLineValueNode, aLineSeq );

    // add value nodes to category nodes
    attachNodeToNode( aBarCategoryNode, aBarValueNode );
    attachNodeToNode( aLineCategoryNode, aLineValueNode );

    // add category node to chart type node
    attachNodeToNode( aBarNode, aBarCategoryNode );
    attachNodeToNode( aLineNode, aLineCategoryNode );

    // add chart type nodes to root of tree
    aRoot->addChild( aBarNode );
    aRoot->addChild( aLineNode );

    return aRoot;
}

chart2::StackMode ColumnLineChartTypeTemplate::getYStackMode() const
{
    return m_eStackMode;
}

uno::Reference< chart2::XChartType > ColumnLineChartTypeTemplate::getDefaultChartType()
    throw (uno::RuntimeException)
{
    return new LineChartType();
}


// ____ XChartTypeTemplate ____
sal_Bool SAL_CALL ColumnLineChartTypeTemplate::matchesTemplate(
    const uno::Reference< chart2::XDiagram >& xDiagram )
    throw (uno::RuntimeException)
{
    sal_Bool bResult = sal_False;

    if( ! xDiagram.is())
        return bResult;

    try
    {
        uno::Reference< chart2::XDataSeriesTreeParent > xParent( xDiagram->getTree(), uno::UNO_QUERY_THROW );
        ::std::vector< uno::Reference< chart2::XChartTypeGroup > > aChartTypeGroups(
            helper::DataSeriesTreeHelper::getChartTypes( xParent ));

        if( aChartTypeGroups.size() == 2 &&
            aChartTypeGroups[0].is() &&
            aChartTypeGroups[1].is() &&
            aChartTypeGroups[0]->getChartType().is() &&
            aChartTypeGroups[1]->getChartType().is() &&
            aChartTypeGroups[0]->getChartType()->getChartType().equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart2.BarChart" )) &&
            aChartTypeGroups[1]->getChartType()->getChartType().equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart2.LineChart" )) )
        {
            bResult = ( helper::DataSeriesTreeHelper::getStackMode(
                            uno::Reference< chart2::XDataSeriesTreeParent >(
                                aChartTypeGroups[0], uno::UNO_QUERY )) ==
                        getYStackMode() );
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return bResult;
}

// ----------------------------------------

uno::Sequence< ::rtl::OUString > ColumnLineChartTypeTemplate::getSupportedServiceNames_Static()
{
    uno::Sequence< ::rtl::OUString > aServices( 2 );
    aServices[ 0 ] = lcl_aServiceName;
    aServices[ 1 ] = C2U( "drafts.com.sun.star.chart2.ChartTypeTemplate" );
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( ColumnLineChartTypeTemplate, lcl_aServiceName );

IMPLEMENT_FORWARD_XINTERFACE2( ColumnLineChartTypeTemplate, ChartTypeTemplate, OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( ColumnLineChartTypeTemplate, ChartTypeTemplate, OPropertySet )

} //  namespace chart
