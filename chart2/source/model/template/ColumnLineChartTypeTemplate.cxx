/*************************************************************************
 *
 *  $RCSfile: ColumnLineChartTypeTemplate.cxx,v $
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
#include "ColumnLineChartTypeTemplate.hxx"
#include "macros.hxx"
#include "LineChartType.hxx"
#include "BarChartType.hxx"

#include <algorithm>

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace chart
{

ColumnLineChartTypeTemplate::ColumnLineChartTypeTemplate(
    Reference<
        uno::XComponentContext > const & xContext,
    chart2::StackMode eStackMode,
    sal_Int32 nNumberOfLines ) :
        ChartTypeTemplate( xContext ),
        m_eStackMode( eStackMode ),
        m_nNumberOfLines( nNumberOfLines )
{}

ColumnLineChartTypeTemplate::~ColumnLineChartTypeTemplate()
{}

Reference< chart2::XDataSeriesTreeParent > ColumnLineChartTypeTemplate::createDataSeriesTree(
    const Sequence< Reference< chart2::XDataSeries > > & aSeriesSeq,
    const Reference< chart2::XBoundedCoordinateSystem > & rCoordSys )
{
    // create series tree nodes
    // root
    Reference< chart2::XDataSeriesTreeParent > aRoot( createRootNode());

    sal_Int32 nNumberOfBars = aSeriesSeq.getLength();
    sal_Int32 nNumberOfLines = m_nNumberOfLines;
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

// ____ XChartTypeTemplate ____
uno::Reference< chart2::XChartType > SAL_CALL ColumnLineChartTypeTemplate::getChartTypeForAdditionalSeries()
    throw (uno::RuntimeException)
{
    return new LineChartType();
}

} //  namespace chart
