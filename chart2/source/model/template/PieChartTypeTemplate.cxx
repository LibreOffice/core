/*************************************************************************
 *
 *  $RCSfile: PieChartTypeTemplate.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: bm $ $Date: 2003-11-21 14:20:13 $
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
#include "PieChartTypeTemplate.hxx"
#include "PolarCoordinateSystem.hxx"
#include "BoundedCoordinateSystem.hxx"
#include "Scaling.hxx"
#include "PieChartType.hxx"
#include "Scale.hxx"
#include "macros.hxx"
#include "algohelper.hxx"

#ifndef CHART_PROPERTYHELPER_HXX
#include "PropertyHelper.hxx"
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

#include <algorithm>

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star;

using ::rtl::OUString;
using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::osl::MutexGuard;

namespace
{

static const ::rtl::OUString lcl_aServiceName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.PieChartTypeTemplate" ));

enum
{
    PROP_PIE_TEMPLATE_DEFAULT_OFFSET,
    PROP_PIE_TEMPLATE_OFFSET_MODE,
    PROP_PIE_TEMPLATE_DIMENSION,
    PROP_PIE_TEMPLATE_USE_RINGS
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( C2U( "OffsetMode" ),
                  PROP_PIE_TEMPLATE_OFFSET_MODE,
                  ::getCppuType( reinterpret_cast< const chart2::PieChartOffsetMode * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "DefaultOffset" ),
                  PROP_PIE_TEMPLATE_DEFAULT_OFFSET,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "Dimension" ),
                  PROP_PIE_TEMPLATE_DIMENSION,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "UseRings" ),
                  PROP_PIE_TEMPLATE_USE_RINGS,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

void lcl_AddDefaultsToMap(
    ::chart::helper::tPropertyValueMap & rOutMap )
{
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_PIE_TEMPLATE_OFFSET_MODE ));
    rOutMap[ PROP_PIE_TEMPLATE_OFFSET_MODE ] =
        uno::makeAny( chart2::PieChartOffsetMode_NONE );
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_PIE_TEMPLATE_DEFAULT_OFFSET ));
    rOutMap[ PROP_PIE_TEMPLATE_DEFAULT_OFFSET ] =
        uno::makeAny( sal_Int32( 10 ) );
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_PIE_TEMPLATE_DIMENSION ));
    rOutMap[ PROP_PIE_TEMPLATE_DIMENSION ] =
        uno::makeAny( sal_Int32( 2 ) );
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_PIE_TEMPLATE_USE_RINGS ));
    rOutMap[ PROP_PIE_TEMPLATE_USE_RINGS ] =
        uno::makeAny( sal_False );
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

PieChartTypeTemplate::PieChartTypeTemplate(
    uno::Reference<
        uno::XComponentContext > const & xContext,
    const ::rtl::OUString & rServiceName,
    chart2::PieChartOffsetMode eMode,
    bool bRings            /* = false */,
    sal_Int32 nDim         /* = 2 */    ) :
        ChartTypeTemplate( xContext, rServiceName ),
        ::property::OPropertySet( m_aMutex ),
        m_ePieOffsetMode( eMode ),
        m_bIsRingChart( bRings )
{
    setFastPropertyValue_NoBroadcast( PROP_PIE_TEMPLATE_OFFSET_MODE,    uno::makeAny( eMode ));
    setFastPropertyValue_NoBroadcast( PROP_PIE_TEMPLATE_DIMENSION,      uno::makeAny( nDim ));
    setFastPropertyValue_NoBroadcast( PROP_PIE_TEMPLATE_USE_RINGS,      uno::makeAny( sal_Bool( bRings )));
}

PieChartTypeTemplate::~PieChartTypeTemplate()
{}

// ____ OPropertySet ____
uno::Any PieChartTypeTemplate::GetDefaultValue( sal_Int32 nHandle ) const
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

::cppu::IPropertyArrayHelper & SAL_CALL PieChartTypeTemplate::getInfoHelper()
{
    return lcl_getInfoHelper();
}


// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL
    PieChartTypeTemplate::getPropertySetInfo()
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


// ____ ChartTypeTemplate ____
sal_Int32 PieChartTypeTemplate::getDimension() const
{
    sal_Int32 nDim = 2;
    try
    {
        // note: UNO-methods are never const
        const_cast< PieChartTypeTemplate * >( this )->
            getFastPropertyValue( PROP_PIE_TEMPLATE_DIMENSION ) >>= nDim;
    }
    catch( beans::UnknownPropertyException & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return nDim;
}

uno::Reference< chart2::XBoundedCoordinateSystem > PieChartTypeTemplate::createCoordinateSystem(
    const uno::Reference< chart2::XBoundedCoordinateSystemContainer > & xCoordSysCnt )
{
    uno::Reference< chart2::XCoordinateSystem > xCoordSys(
        new PolarCoordinateSystem( getDimension() ));
    uno::Reference< chart2::XBoundedCoordinateSystem > xResult(
        new BoundedCoordinateSystem( xCoordSys ));

    chart2::ScaleData aScale;
    aScale.Orientation = chart2::AxisOrientation_MATHEMATICAL;
    aScale.Scaling = new LinearScaling( 1.0, 0.0 );

    for( sal_Int32 i = 0; i < getDimension(); ++i )
        xResult->setScaleByDimension(
            i, uno::Reference< chart2::XScale >( new Scale( GetComponentContext(), aScale ) ));

    try
    {
        if( xCoordSys.is())
            xCoordSysCnt->addCoordinateSystem( xResult );
    }
    catch( lang::IllegalArgumentException ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return xResult;
}

void PieChartTypeTemplate::createAndAddAxes(
    const uno::Reference< chart2::XBoundedCoordinateSystem > & rCoordSys,
    const uno::Reference< chart2::XAxisContainer > & rOutAxisCnt )
{
    // do nothing -- there are no axes in a pie chart
}

Reference< chart2::XDataSeriesTreeParent > PieChartTypeTemplate::createDataSeriesTree(
    const Sequence< Reference< chart2::XDataSeries > > & aSeriesSeq,
    const Reference< chart2::XBoundedCoordinateSystem > & rCoordSys )
{
    sal_Int32 nDefaultOffset = 10;
    sal_Int32 nDim = getDimension();
    getFastPropertyValue( PROP_PIE_TEMPLATE_DEFAULT_OFFSET ) >>= nDefaultOffset;
    sal_Int32 nOffset = (m_ePieOffsetMode == chart2::PieChartOffsetMode_NONE)
        ? 0
        : nDefaultOffset;

    // create series tree nodes
    // root
    Reference< chart2::XDataSeriesTreeParent > aRoot( createRootNode());

    // chart type group
    Reference< chart2::XDataSeriesTreeNode > aChartTypeNode(
        createChartTypeGroup( new PieChartType( nDim, nOffset )));

    // 'x-axis' group
    Reference< chart2::XDataSeriesTreeNode > aCategoryNode(
        createScaleGroup( true, true, rCoordSys, 0, chart2::StackMode_STACKED ));

    // 'y-axis' group
    Reference< chart2::XDataSeriesTreeNode > aValueNode(
        createScaleGroup( false, true, rCoordSys, 1, getYStackMode() ));

    // add chart type node to root of tree
    aRoot->addChild( aChartTypeNode );

    // build tree
    // add series node to value node
    if( m_ePieOffsetMode == chart2::PieChartOffsetMode_FIRST_EXPLODED )
    {
        if( aSeriesSeq.getLength() >= 1 )
        {
            Sequence< Reference< chart2::XDataSeries > > aFirst( aSeriesSeq.getConstArray(), 1 );
            addDataSeriesToGroup( aValueNode, aFirst );
        }

        if( aSeriesSeq.getLength() > 1 )
        {
            // chart type group
            Reference< chart2::XDataSeriesTreeNode > aChartTypeNode2(
                createChartTypeGroup( new PieChartType( nDim, 0 )));

            // 'x-axis' group
            Reference< chart2::XDataSeriesTreeNode > aCategoryNode2(
                createScaleGroup( true, true, rCoordSys, 0, chart2::StackMode_STACKED ));

            // 'y-axis' group
            Reference< chart2::XDataSeriesTreeNode > aValueNode2(
                createScaleGroup( false, true, rCoordSys, 1, getYStackMode() ));

            Sequence< Reference< chart2::XDataSeries > > aRest( aSeriesSeq.getConstArray() + 1,
                                                                aSeriesSeq.getLength() - 1 );
            addDataSeriesToGroup( aValueNode2, aRest );

            attachNodeToNode( aCategoryNode2, aValueNode2 );
            attachNodeToNode( aChartTypeNode2, aCategoryNode2 );

            aRoot->addChild( aChartTypeNode2 );
        }
    }
    else
        addDataSeriesToGroup( aValueNode, aSeriesSeq );

    // add value node to category node
    attachNodeToNode( aCategoryNode, aValueNode );

    // add category node to chart type node
    attachNodeToNode( aChartTypeNode, aCategoryNode );

    return aRoot;
}

uno::Reference< chart2::XChartType > PieChartTypeTemplate::getDefaultChartType()
    throw (uno::RuntimeException)
{
    sal_Int32 nOffset = 0;
    if( m_ePieOffsetMode == chart2::PieChartOffsetMode_ALL_EXPLODED )
    {
        nOffset = 10;
        getFastPropertyValue( PROP_PIE_TEMPLATE_DEFAULT_OFFSET ) >>= nOffset;
    }

    return new PieChartType( getDimension(), nOffset );
}

// ----------------------------------------

uno::Sequence< ::rtl::OUString > PieChartTypeTemplate::getSupportedServiceNames_Static()
{
    uno::Sequence< ::rtl::OUString > aServices( 2 );
    aServices[ 0 ] = lcl_aServiceName;
    aServices[ 1 ] = C2U( "drafts.com.sun.star.chart2.ChartTypeTemplate" );
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( PieChartTypeTemplate, lcl_aServiceName );

IMPLEMENT_FORWARD_XINTERFACE2( PieChartTypeTemplate, ChartTypeTemplate, OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( PieChartTypeTemplate, ChartTypeTemplate, OPropertySet )

} //  namespace chart
