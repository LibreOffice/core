/*************************************************************************
 *
 *  $RCSfile: ChartTypeTemplate.hxx,v $
 *
 *  $Revision: 1.5 $
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
#ifndef CHART_CHARTTYPETEMPLATE_HXX
#define CHART_CHARTTYPETEMPLATE_HXX

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#include "ServiceMacros.hxx"

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XCHARTTYPETEMPLATE_HPP_
#include <drafts/com/sun/star/chart2/XChartTypeTemplate.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XAXISCONTAINER_HPP_
#include <drafts/com/sun/star/chart2/XAxisContainer.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_STACKMODE_HPP_
#include <drafts/com/sun/star/chart2/StackMode.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XGRIDCONTAINER_HPP_
#include <drafts/com/sun/star/chart2/XGridContainer.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XBOUNDEDCOORDINATESYSTEMCONTAINER_HPP_
#include <drafts/com/sun/star/chart2/XBoundedCoordinateSystemContainer.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XLEGEND_HPP_
#include <drafts/com/sun/star/chart2/XLegend.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICENAME_HPP_
#include <com/sun/star/lang/XServiceName.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XCHARTTYPE_HPP_
#include <drafts/com/sun/star/chart2/XChartType.hpp>
#endif

namespace chart
{

class ChartTypeTemplate : public ::cppu::WeakImplHelper2<
        ::drafts::com::sun::star::chart2::XChartTypeTemplate,
        ::com::sun::star::lang::XServiceName >
{
public:
    explicit ChartTypeTemplate(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > const & xContext,
        const ::rtl::OUString & rServiceName );
    virtual ~ChartTypeTemplate();

    APPHELPER_XSERVICEINFO_DECL()
    /// establish methods for factory instatiation
//  APPHELPER_SERVICE_FACTORY_HELPER( ChartTypeTemplate )

protected:
    // ____ XChartTypeTemplate ____
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::chart2::XDiagram > SAL_CALL
        createDiagram( const ::com::sun::star::uno::Sequence<
                           ::com::sun::star::uno::Reference<
                               ::drafts::com::sun::star::chart2::XDataSeries > >& aSeriesSeq )
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL matchesTemplate(
        const ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::chart2::XDiagram >& xDiagram )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XServiceName ____
    virtual ::rtl::OUString SAL_CALL getServiceName()
        throw (::com::sun::star::uno::RuntimeException);

    // Methods to overload for automatic creation
    // ------------------------------------------

    /// returns 2 by default.  Supported are 2 and 3
    virtual sal_Int32 getDimension() const;

    /** returns StackMode_NONE by default.  For a column/bar chart you would
        want to return StackMode_STACKED here.
     */
    virtual ::drafts::com::sun::star::chart2::StackMode getXStackMode() const;

    /** returns StackMode_NONE by default.  This is a global flag used for all series
        if createDataSeriesTree() is not overloaded
     */
    virtual ::drafts::com::sun::star::chart2::StackMode getYStackMode() const;

    /** returns StackMode_NONE by default.  For a column/bar chart you would
        want to return StackMode_STACKED here.
     */
    virtual ::drafts::com::sun::star::chart2::StackMode getZStackMode() const;

    virtual ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XChartType > getDefaultChartType()
        throw (::com::sun::star::uno::RuntimeException);

    // Methods for creating the diagram piecewise
    // ------------------------------------------

    /** Creates a 2d or 3d cartesian coordinate system with mathematically
        oriented, linear scales with auto-min/max.

        <p>The dimension depends on the property "ChartStyle".</p>

        @param xCoordSysCnt
            If this container is valid, the coordinate system is added to it.
     */
   virtual ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XBoundedCoordinateSystem >
       createCoordinateSystem(
           const ::com::sun::star::uno::Reference<
               ::drafts::com::sun::star::chart2::XBoundedCoordinateSystemContainer > & xCoordSysCnt );

    /** create axes and add them to the given container.

        <p>As default, this method creates as many axes as there are dimensions
        in the given coordinate system.  Each of the axis represents one of the
        dimensions of the coordinate system.</p>
     */
    virtual void createAndAddAxes(
        const ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::chart2::XBoundedCoordinateSystem > & rCoordSys,
        const ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::chart2::XAxisContainer > & rOutAxisCnt );

    /** create grids and add them to the given container.

        <p>As default, this method creates a major grid for the second
        coordinate of the coordinate system.</p>
     */
    virtual void createAndAddGrids(
        const ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::chart2::XBoundedCoordinateSystem > & rCoordSys,
        const ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::chart2::XGridContainer > & rOutGridCnt );

    /** create a data series tree, that fits the requirements of the chart type.

        <p>As default, this creates a tree with the following structure:</p>

        <pre>
          root
           |
           +-- chart type
                   |
                   +-- category (DiscreteStackableScaleGroup using scale 0)
                          |
                          +-- values (ContinuousStackableScaleGroup using scale 1)
                                |
                                +-- series 0
                                |
                                +-- series 1
                                |
                                ...
                                |
                                +.. series n-1
        </pre>

        <p>If there are less than two scales available the returned tree is
        empty.</p>
     */
    virtual ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XDataSeriesTreeParent > createDataSeriesTree(
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Reference<
                    ::drafts::com::sun::star::chart2::XDataSeries > >& aSeriesSeq,
            const ::com::sun::star::uno::Reference<
                ::drafts::com::sun::star::chart2::XBoundedCoordinateSystem > & rCoordSys
            );

    // helper methods
    // --------------
    ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XDataSeriesTreeParent > createRootNode();

    ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XDataSeriesTreeNode >
        createChartTypeGroup(
            const ::com::sun::star::uno::Reference<
                ::drafts::com::sun::star::chart2::XChartType > & xChartType );

    ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XDataSeriesTreeNode >
        createScaleGroup( bool bIsDiscrete,
                          bool bIsStackable,
                          ::com::sun::star::uno::Reference<
                              ::drafts::com::sun::star::chart2::XBoundedCoordinateSystem > xCoordSys,
                          sal_Int32 nRepresentedDimension,
                          ::drafts::com::sun::star::chart2::StackMode eStackMode =
                              ::drafts::com::sun::star::chart2::StackMode_NONE );

    void addDataSeriesToGroup(
        const ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::chart2::XDataSeriesTreeNode > & rParent,
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                ::drafts::com::sun::star::chart2::XDataSeries > > & rDataSeries );

    /** Finds the first ContinuousScaleGroup in the tree and sets the stacking
        mode there if it is a stackable group
     */
    void setStackModeAtTree(
        const  ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::chart2::XDataSeriesTreeParent > & rTree,
        ::drafts::com::sun::star::chart2::StackMode eMode );

    void attachNodeToNode(
        const ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::chart2::XDataSeriesTreeNode > & rParent,
        const ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::chart2::XDataSeriesTreeNode > & rChild );

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
        GetComponentContext() const;

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
        m_xContext;
    const ::rtl::OUString m_aServiceName;

    /** modifies the given diagram
     */
    void FillDiagram( const ::com::sun::star::uno::Reference<
                          ::drafts::com::sun::star::chart2::XDiagram > & xDiagram,
                      const ::com::sun::star::uno::Sequence<
                          ::com::sun::star::uno::Reference<
                              ::drafts::com::sun::star::chart2::XDataSeries > >& aSeriesSeq );
};

} //  namespace chart

// CHART_CHARTTYPETEMPLATE_HXX
#endif
