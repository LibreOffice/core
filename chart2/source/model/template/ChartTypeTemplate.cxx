/*************************************************************************
 *
 *  $RCSfile: ChartTypeTemplate.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: bm $ $Date: 2003-11-20 18:12:23 $
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
#include "ChartTypeTemplate.hxx"
#include "PropertyHelper.hxx"
#include "macros.hxx"
#include "ContextHelper.hxx"
#include "Scale.hxx"
#include "DataSeriesTreeHelper.hxx"

#include "Scaling.hxx"
#include "CartesianCoordinateSystem.hxx"
#include "BoundedCoordinateSystem.hxx"
#include "MeterHelper.hxx"
#include "LegendHelper.hxx"
#include "BarChartType.hxx"

#ifndef _CPPUHELPER_COMPONENT_CONTEXT_HXX_
#include <cppuhelper/component_context.hxx>
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

#include <algorithm>
#include <iterator>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans::PropertyAttribute;
using namespace ::drafts::com::sun::star;

using ::rtl::OUString;
using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;

// ======================================================================

namespace
{
#if OSL_DEBUG_LEVEL > 1

#include <drafts/com/sun/star/chart2/XChartTypeGroup.hpp>
#include <drafts/com/sun/star/chart2/XScaleGroup.hpp>

// forward declatation
void lcl_ShowTree( const Reference< chart2::XDataSeriesTreeParent > & xParent, sal_Int32 nLevel = 0 );

#endif

} // anonymous namespace

namespace chart
{

ChartTypeTemplate::ChartTypeTemplate(
    Reference< uno::XComponentContext > const & xContext,
    const ::rtl::OUString & rServiceName ) :
        m_xContext( xContext ),
        m_aServiceName( rServiceName )
{
}

ChartTypeTemplate::~ChartTypeTemplate()
{}

// ____ XChartTypeTemplate ____

Reference< chart2::XDiagram > SAL_CALL ChartTypeTemplate::createDiagram(
    const Sequence< Reference< chart2::XDataSeries > >& aSeriesSeq )
    throw (uno::RuntimeException)
{
    Reference< chart2::XDiagram > xDia;

    try
    {
        // create diagram
        ContextHelper::tContextEntryMapType aContextValues(
            ContextHelper::MakeContextEntryMap
            ( C2U( "TemplateServiceName" ),
              uno::makeAny( getServiceName() ))
            );

        xDia.set(
            m_xContext->getServiceManager()->createInstanceWithContext(
                C2U( "drafts.com.sun.star.chart2.Diagram" ),
                ContextHelper::createContext( aContextValues, m_xContext ) ),
            uno::UNO_QUERY_THROW );

        // modify diagram
        FillDiagram( xDia, aSeriesSeq );

        // create and attach legend
        Reference< chart2::XLegend > xLegend(
            m_xContext->getServiceManager()->createInstanceWithContext(
                C2U( "drafts.com.sun.star.chart2.Legend" ),
                m_xContext ),
            uno::UNO_QUERY_THROW );

        xDia->setLegend( xLegend );
        LegendHelper::defaultFillEmptyLegend( xLegend, xDia );
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return xDia;
}

sal_Bool SAL_CALL ChartTypeTemplate::matchesTemplate(
    const uno::Reference< chart2::XDiagram >& xDiagram )
    throw (uno::RuntimeException)
{
    sal_Bool bResult = sal_False;

    if( ! xDiagram.is())
        return bResult;

    try
    {
        uno::Reference< chart2::XDataSeriesTreeParent > xParent( xDiagram->getTree(), uno::UNO_QUERY_THROW );

        ::std::vector< uno::Reference< chart2::XChartTypeGroup > > aChartTypes(
            helper::DataSeriesTreeHelper::getChartTypes( xParent ));

        if( aChartTypes.size() == 1 &&
            aChartTypes[0].is() &&
            aChartTypes[0]->getChartType().is() )
        {
            if( aChartTypes[0]->getChartType()->getChartType().equals(
                    getDefaultChartType()->getChartType()))
            {
                try
                {
                    uno::Reference< beans::XPropertySet > xProp( aChartTypes[0]->getChartType(),
                                                                 uno::UNO_QUERY_THROW );
                    uno::Reference< beans::XPropertySetInfo > xInfo( xProp->getPropertySetInfo() );
                    sal_Int32 nDim;
                    if( xInfo.is() &&
                        xInfo->hasPropertyByName( C2U( "Dimension" )) &&
                        (xProp->getPropertyValue( C2U( "Dimension" )) >>= nDim ))
                    {
                        bResult = (nDim == getDimension());
                    }
                    else
                        // correct chart type, but without Dimension property
                        bResult = sal_True;
                }
                catch( uno::Exception & ex )
                {
                    ASSERT_EXCEPTION( ex );
                }

                if( bResult )
                    bResult = ( helper::DataSeriesTreeHelper::getStackMode( xParent ) ==
                                getStackMode() );
            }
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return bResult;
}

// ____ XServiceName ____
    ::rtl::OUString SAL_CALL ChartTypeTemplate::getServiceName()
    throw (uno::RuntimeException)
{
    return m_aServiceName;
}

// ________________________________________

sal_Int32 ChartTypeTemplate::getDimension() const
{
    return 2;
}

chart2::StackMode ChartTypeTemplate::getStackMode() const
{
    return chart2::StackMode_NONE;
}

uno::Reference< chart2::XChartType > ChartTypeTemplate::getDefaultChartType()
    throw (uno::RuntimeException)
{
    return new BarChartType( 2 );
}

// ________________________________________

Reference< chart2::XBoundedCoordinateSystem > ChartTypeTemplate::createCoordinateSystem(
    const Reference< chart2::XBoundedCoordinateSystemContainer > & xCoordSysCnt )
{
    Reference< chart2::XCoordinateSystem > xCoordSys( new CartesianCoordinateSystem( getDimension() ));
    Reference< chart2::XBoundedCoordinateSystem > xResult(
        new BoundedCoordinateSystem( xCoordSys ));

    chart2::ScaleData aScale;
    aScale.Orientation = chart2::AxisOrientation_MATHEMATICAL;
    aScale.Scaling = new LinearScaling( 1.0, 0.0 );

    for( sal_Int32 i = 0; i < getDimension(); ++i )
        xResult->setScaleByDimension(
            i, Reference< chart2::XScale >( new Scale( m_xContext, aScale ) ));

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

void ChartTypeTemplate::createAndAddAxes(
    const Reference< chart2::XBoundedCoordinateSystem > & rCoordSys,
    const Reference< chart2::XAxisContainer > & rOutAxisCnt )
{
    if( rCoordSys.is() && rOutAxisCnt.is())
    {
        // create axes
        Reference< chart2::XAxis > aAxis;
        sal_Int32 nDim = rCoordSys->getDimension(),
            i = 0;

        for( ; i < nDim; ++i )
            MeterHelper::createAxis( i, rCoordSys, rOutAxisCnt, m_xContext );
    }
}

void ChartTypeTemplate::createAndAddGrids(
    const Reference< chart2::XBoundedCoordinateSystem > & rCoordSys,
    const Reference< chart2::XGridContainer > & rOutGridCnt )
{
    // just add one y-grid
    static const sal_Int32 nGridDim = 1;

    if( rOutGridCnt.is() &&
        rCoordSys.is() &&
        rCoordSys->getDimension() >= 2 )
    {
        ContextHelper::tContextEntryMapType aContextValues(
            ContextHelper::MakeContextEntryMap
            ( C2U( "Identifier" ), uno::makeAny( MeterHelper::makeGridIdentifier( 1, 0, true ) ))
            );

        Reference< chart2::XGrid > xGrid(
            m_xContext->getServiceManager()->createInstanceWithContext(
                C2U( "drafts.com.sun.star.chart2.Grid" ),
                ContextHelper::createContext( aContextValues, m_xContext ) ),
            uno::UNO_QUERY );

        if( xGrid.is())
        {
            // try to share XIncrement with axis
            Reference< chart2::XAxisContainer > xAxisCnt( rOutGridCnt, uno::UNO_QUERY );
            if( xAxisCnt.is())
            {
                Sequence< Reference< chart2::XAxis > > aAxes( xAxisCnt->getAxes());
                for( sal_Int32 i = 0; i < aAxes.getLength(); ++i )
                {
                    if( aAxes[i].is() &&
                        aAxes[i]->getRepresentedDimension() == nGridDim &&
                        aAxes[i]->getCoordinateSystem() == rCoordSys )
                    {
                        xGrid->setIncrement( aAxes[i]->getIncrement());
                        break;
                    }
                }
            }

            xGrid->attachCoordinateSystem( rCoordSys, nGridDim );
            rOutGridCnt->addGrid( xGrid );
        }
    }
}

void ChartTypeTemplate::FillDiagram(
    const Reference< chart2::XDiagram >& xDiagram,
    const Sequence< Reference< chart2::XDataSeries > >& aSeriesSeq )
{
    Reference< chart2::XBoundedCoordinateSystemContainer > xCoordSysCnt( xDiagram, uno::UNO_QUERY );
    // scales
    Reference< chart2::XBoundedCoordinateSystem > xCoordSys( createCoordinateSystem( xCoordSysCnt ));

    // data series tree
    Reference< chart2::XDataSeriesTreeParent > xTree(
        createDataSeriesTree( aSeriesSeq, xCoordSys));
    xDiagram->setTree( xTree );

#if OSL_DEBUG_LEVEL > 1
    lcl_ShowTree( xTree );
#endif

    // set stacking mode
//     sal_Int32 nStyle = CHSTYLE_ADDIN;
//     getFastPropertyValue( PROP_CHARTTYPE_CHART_TEMPLATE_NAME ) >>= nStyle;
//     SetStackModeAtTree( xTree, lcl_GetStackModeForChartStyle(
//                             static_cast< ChartStyle >( nStyle ) ));

    // axes
    Reference< chart2::XAxisContainer > xAxisCnt( xDiagram, uno::UNO_QUERY );
    OSL_ASSERT( xAxisCnt.is());
    if( xAxisCnt.is())
    {
        // remove formerly existing axes
        Sequence< Reference< chart2::XAxis > > aAxes( xAxisCnt->getAxes() );
        sal_Int32 i = 0;
        for( ; i < aAxes.getLength(); ++i )
        {
            xAxisCnt->removeAxis( aAxes[ i ] );
        }

        createAndAddAxes( xCoordSys, xAxisCnt );
    }

    // grid
    Reference< chart2::XGridContainer > xGridCnt( xDiagram, uno::UNO_QUERY );
    if( xGridCnt.is())
    {
        Sequence< Reference< chart2::XGrid > > aGrids( xGridCnt->getGrids() );
        sal_Int32 i = 0;
        for( ; i < aGrids.getLength(); ++i )
        {
            xGridCnt->removeGrid( aGrids[ i ] );
        }

        createAndAddGrids( xCoordSys, xGridCnt );
    }
}

Reference< chart2::XDataSeriesTreeParent > ChartTypeTemplate::createRootNode()
{
    Reference< chart2::XDataSeriesTreeParent > aRoot(
        m_xContext->getServiceManager()->createInstanceWithContext(
            C2U( "drafts.com.sun.star.chart2.DataSeriesTree" ), m_xContext ),
        uno::UNO_QUERY );
    OSL_ASSERT( aRoot.is());

    return aRoot;
}

Reference< chart2::XDataSeriesTreeNode > ChartTypeTemplate::createChartTypeGroup(
    const Reference< chart2::XChartType > & xChartType )
{
    Reference< chart2::XDataSeriesTreeNode > aChartTypeNode(
        m_xContext->getServiceManager()->createInstanceWithContext(
            C2U( "drafts.com.sun.star.chart2.ChartTypeGroup" ), m_xContext ),
        uno::UNO_QUERY );
    OSL_ASSERT( aChartTypeNode.is());
    Reference< chart2::XChartTypeGroup > aChartTypeGroup( aChartTypeNode, uno::UNO_QUERY );
    OSL_ASSERT( aChartTypeGroup.is());
    aChartTypeGroup->setChartType( xChartType );

    return aChartTypeNode;
}

void ChartTypeTemplate::addDataSeriesToGroup(
    const Reference< chart2::XDataSeriesTreeNode > & rParent,
    const Sequence< Reference< chart2::XDataSeries > > & rDataSeries )
{
    Reference< chart2::XDataSeriesTreeParent > xParent( rParent, uno::UNO_QUERY );
    OSL_ASSERT( xParent.is());
    if( xParent.is())
    {
        const sal_Int32 nSeriesCount = rDataSeries.getLength();
        sal_Int32 i = 0;
        for( i = 0; i < nSeriesCount; ++i )
        {
            Reference< chart2::XDataSeriesTreeNode > xNode( rDataSeries[ i ], uno::UNO_QUERY );
            xParent->addChild( xNode );
        }
    }
}


Reference< chart2::XDataSeriesTreeNode > ChartTypeTemplate::createScaleGroup(
    bool bIsDiscrete,
    bool bIsStackable,
    Reference< chart2::XBoundedCoordinateSystem > xCoordSys,
    sal_Int32 nRepresentedDimension,
    chart2::StackMode eStackMode /* = chart2::StackMode_NONE */ )
{
    ContextHelper::tContextEntryMapType aContextValues(
        ContextHelper::MakeContextEntryMap
        ( C2U( "CoordinateSystem" ),      uno::makeAny( xCoordSys ) )
        ( C2U( "RepresentedDimension" ),  uno::makeAny( sal_Int32( nRepresentedDimension ) ) )
        );

    Reference< chart2::XDataSeriesTreeNode > aNode;
    ::rtl::OUString aServiceName;

    if( bIsDiscrete )
    {
        if( bIsStackable )
            aServiceName = C2U( "drafts.com.sun.star.chart2.DiscreteStackableScaleGroup" );
        else
            aServiceName = C2U( "drafts.com.sun.star.chart2.DiscreteScaleGroup" );
    }
    else
    {
        if( bIsStackable )
            aServiceName = C2U( "drafts.com.sun.star.chart2.ContinuousStackableScaleGroup" );
        else
            aServiceName = C2U( "drafts.com.sun.star.chart2.ContinuousScaleGroup" );
    }

    aNode.set(
        m_xContext->getServiceManager()->createInstanceWithContext(
            aServiceName,
            ContextHelper::createContext( aContextValues, m_xContext )),
        uno::UNO_QUERY );

    OSL_ASSERT( aNode.is());

    Reference< chart2::XStackableScaleGroup > aStackGroup( aNode, uno::UNO_QUERY );
    if( aStackGroup.is())
        aStackGroup->setStackMode( eStackMode );

    return aNode;
}

void ChartTypeTemplate::attachNodeToNode(
    const Reference< chart2::XDataSeriesTreeNode > & rParent,
    const Reference< chart2::XDataSeriesTreeNode > & rChild )
{
    Reference< chart2::XDataSeriesTreeParent > xParent( rParent, uno::UNO_QUERY );
    OSL_ASSERT( xParent.is());
    if( xParent.is())
        xParent->addChild( rChild );
}

Reference< chart2::XDataSeriesTreeParent > ChartTypeTemplate::createDataSeriesTree(
    const Sequence< Reference< chart2::XDataSeries > > & aSeriesSeq,
    const Reference< chart2::XBoundedCoordinateSystem > & rCoordSys )
{
    // create series tree nodes
    // root
    Reference< chart2::XDataSeriesTreeParent > aRoot( createRootNode());

    // chart type group
    Reference< chart2::XDataSeriesTreeNode > aChartTypeNode(
        createChartTypeGroup( getDefaultChartType()));

    // 'x-axis' group
    Reference< chart2::XDataSeriesTreeNode > aCategoryNode(
        createScaleGroup( true, true, rCoordSys, 0, chart2::StackMode_STACKED ));

    // 'y-axis' group
    Reference< chart2::XDataSeriesTreeNode > aValueNode(
        createScaleGroup( false, true, rCoordSys, 1, getStackMode() ));

    // build tree
    // add series node to value node
    addDataSeriesToGroup( aValueNode, aSeriesSeq );

    // add value node to category node
    attachNodeToNode( aCategoryNode, aValueNode );

    // add category node to chart type node
    attachNodeToNode( aChartTypeNode, aCategoryNode );

    // add chart type node to root of tree
    aRoot->addChild( aChartTypeNode );

    return aRoot;
}

void ChartTypeTemplate::setStackModeAtTree(
    const Reference< chart2::XDataSeriesTreeParent > & rTree,
    chart2::StackMode eMode )
{
    bool bResult = helper::DataSeriesTreeHelper::setStackMode( rTree, eMode );
    OSL_ENSURE( bResult, "stack-mode could not be set" );
}

// ________

Sequence< OUString > ChartTypeTemplate::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 3 );
    aServices[ 0 ] = C2U( "drafts.com.sun.star.chart2.ChartTypeTemplate" );
    aServices[ 1 ] = C2U( "drafts.com.sun.star.layout.LayoutElement" );
    aServices[ 2 ] = C2U( "com.sun.star.beans.PropertySet" );
    return aServices;
}

Reference< uno::XComponentContext > ChartTypeTemplate::GetComponentContext() const
{
    return m_xContext;
}

// ================================================================================

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( ChartTypeTemplate,
                             C2U( "com.sun.star.comp.chart.ChartTypeTemplate" ));
} //  namespace chart

// ____ DEBUG helper ____

#if OSL_DEBUG_LEVEL > 1
namespace
{

// prints the data series tree
void lcl_ShowTree( const Reference< chart2::XDataSeriesTreeParent > & xParent, sal_Int32 nLevel )
{
    if( nLevel == 0 )
    {
        OSL_TRACE( "DataSeriesTree" );
        OSL_TRACE( "--------------" );
    }

    if( xParent.is())
    {
//        char aIndent[ nLevel * 4  + 1];
        char aIndent[ 200 ];
        // avoids including any C-headers
        for( int j = 0; j < nLevel * 4; ++j )
            aIndent[j] = ' ';
        aIndent[j] = '\0';

        Sequence< Reference< chart2::XDataSeriesTreeNode > > aChildren( xParent->getChildren());
        for( sal_Int32 i = 0; i < aChildren.getLength(); ++i )
        {
            Reference< lang::XServiceInfo > xInfo( aChildren[ i ], uno::UNO_QUERY );
            if( xInfo.is())
            {
                Reference< chart2::XStackableScaleGroup > xStackableGroup( xInfo, uno::UNO_QUERY );
                Reference< chart2::XScaleGroup > xScaleGroup( xInfo, uno::UNO_QUERY );
                bool bIsDiscrete = ( xInfo->supportsService(
                                         C2U( "drafts.com.sun.star.chart2.DiscreteScaleGroup" )) ||
                                     xInfo->supportsService(
                                         C2U( "drafts.com.sun.star.chart2.DiscreteStackableScaleGroup" )));

                if( xStackableGroup.is())
                {
                    if( xStackableGroup.is())
                    {
                        OSL_TRACE( "%s<%s stackable scale group> (Dim %d, %s)",
                                   aIndent,
                                   bIsDiscrete ? "discrete" : "continuous",
                                   xStackableGroup->getRepresentedDimension(),
                                   ( xStackableGroup->getStackMode() == chart2::StackMode_NONE )
                                   ? "unstacked"
                                   : ( xStackableGroup->getStackMode() == chart2::StackMode_STACKED )
                                   ? "stacked"
                                   : "percent stacked" );
                    }
                    else
                    {
                        OSL_TRACE( "%s<stackable scale group>", aIndent );
                    }
                }
                else if( xScaleGroup.is())
                {
                    if( xStackableGroup.is())
                    {
                        OSL_TRACE( "%s<%s scale group> (Dim %d)",
                                   aIndent,
                                   bIsDiscrete ? "discrete" : "continuous",
                                   xScaleGroup->getRepresentedDimension() );
                    }
                    else
                    {
                        OSL_TRACE( "%s<scale group>", aIndent );
                    }
                }
                else if( xInfo->supportsService( C2U( "drafts.com.sun.star.chart2.ChartTypeGroup" )))
                {
                    Reference< chart2::XChartTypeGroup > xGroup( xInfo, uno::UNO_QUERY );
                    if( xGroup.is() )
                    {
                        OSL_TRACE( "%s<chart type group> (%s)", aIndent, U2C( xGroup->getChartType()->getChartType()) );
                    }
                    else
                    {
                        OSL_TRACE( "%s<chart type group>", aIndent );
                    }
                }
                else if( xInfo->supportsService( C2U( "drafts.com.sun.star.chart2.DataSeries" )))
                {
                    Reference< beans::XPropertySet > xProp( xInfo, uno::UNO_QUERY );
                    ::rtl::OUString aId;
                    if( xProp.is() &&
                        ( xProp->getPropertyValue( C2U( "Identifier" )) >>= aId ))
                    {
                        OSL_TRACE( "%s<data series> (%s)", aIndent, U2C( aId ));
                    }
                    else
                    {
                        OSL_TRACE( "%s<data series>", aIndent );
                    }
                }
                else
                {
                    OSL_TRACE( "%s<unknown Node>", aIndent );
                }
            }
            else
            {
                OSL_TRACE( "%s<unknown Node>", aIndent );
            }

            Reference< chart2::XDataSeriesTreeParent > xNewParent( aChildren[ i ], uno::UNO_QUERY );
            if( xNewParent.is())
                lcl_ShowTree( xNewParent, nLevel + 1 );
        }
    }
}
} // anonymous namespace
#endif
