/*************************************************************************
 *
 *  $RCSfile: PieChartTypeTemplate.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:32 $
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

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star;

using ::rtl::OUString;
using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::osl::MutexGuard;

namespace chart
{

PieChartTypeTemplate::PieChartTypeTemplate(
    uno::Reference<
        uno::XComponentContext > const & xContext,
    PieOffsetMode eMode,
    bool bRings            /* = false */,
    sal_Int32 nDim         /* = 2 */    ) :
        ChartTypeTemplate( xContext ),
        m_ePieOffsetMode( eMode ),
        m_nDim( nDim ),
        m_bIsRingChart( bRings )
{}

PieChartTypeTemplate::~PieChartTypeTemplate()
{}

sal_Int32 PieChartTypeTemplate::getDimension() const
{
    return m_nDim;
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
    sal_Int32 nOffset = (m_ePieOffsetMode == NO_OFFSET)
        ? 0
        : m_nDefaultOffset;

    // create series tree nodes
    // root
    Reference< chart2::XDataSeriesTreeParent > aRoot( createRootNode());

    // chart type group
    Reference< chart2::XDataSeriesTreeNode > aChartTypeNode(
        createChartTypeGroup( new PieChartType( m_nDim, nOffset )));

    // 'x-axis' group
    Reference< chart2::XDataSeriesTreeNode > aCategoryNode(
        createScaleGroup( true, true, rCoordSys, 0, chart2::StackMode_STACKED ));

    // 'y-axis' group
    Reference< chart2::XDataSeriesTreeNode > aValueNode(
        createScaleGroup( false, true, rCoordSys, 1, getStackMode() ));

    // add chart type node to root of tree
    aRoot->addChild( aChartTypeNode );

    // build tree
    // add series node to value node
    if( m_ePieOffsetMode == FIRST_EXPLODED )
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
                createChartTypeGroup( new PieChartType( m_nDim, 0 )));

            // 'x-axis' group
            Reference< chart2::XDataSeriesTreeNode > aCategoryNode2(
                createScaleGroup( true, true, rCoordSys, 0, chart2::StackMode_STACKED ));

            // 'y-axis' group
            Reference< chart2::XDataSeriesTreeNode > aValueNode2(
                createScaleGroup( false, true, rCoordSys, 1, getStackMode() ));

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

// ____ XChartTypeTemplate ____
uno::Reference< chart2::XChartType > SAL_CALL PieChartTypeTemplate::getChartTypeForAdditionalSeries()
    throw (uno::RuntimeException)
{
    sal_Int32 nOffset = 0;
    if( m_ePieOffsetMode == ALL_EXPLODED )
        nOffset = m_nDefaultOffset;

    return new PieChartType( m_nDim, nOffset );
}

} //  namespace chart
