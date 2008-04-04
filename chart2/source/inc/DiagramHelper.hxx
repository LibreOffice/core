/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DiagramHelper.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 10:59:16 $
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
#ifndef CHART2_DIAGRAMHELPER_HXX
#define CHART2_DIAGRAMHELPER_HXX

#include "StackMode.hxx"

#ifndef _COM_SUN_STAR_CHART2_XAXIS_HPP_
#include <com/sun/star/chart2/XAxis.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDIAGRAM_HPP_
#include <com/sun/star/chart2/XDiagram.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XCHARTTYPETEMPLATE_HPP_
#include <com/sun/star/chart2/XChartTypeTemplate.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XCOORDINATESYSTEM_HPP_
#include <com/sun/star/chart2/XCoordinateSystem.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_INTERPRETEDDATA_HPP_
#include <com/sun/star/chart2/InterpretedData.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_STACKINGDIRECTION_HPP_
#include <com/sun/star/chart2/StackingDirection.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart2/XChartDocument.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#include <utility>
#include <vector>


namespace chart
{

class DiagramHelper
{
public:
    typedef ::std::pair<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XChartTypeTemplate >,
            ::rtl::OUString >
        tTemplateWithServiceName;

    /** tries to find a template in the chart-type manager that matches the
        given diagram.

        @param rPreferredTemplateName
            Check this template first.  This may speed up searching, if the
            caller assumes a certain template as most likely to be the one that
            matches.

        @return
            A pair containing a template with the correct properties set as
            first entry and the service name of the templateas second entry.  If
            no template was found both elements are empty.
     */
    static tTemplateWithServiceName
        getTemplateForDiagram(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDiagram > & xDiagram,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XMultiServiceFactory > & xChartTypeManager,
            const ::rtl::OUString & rPreferredTemplateName = ::rtl::OUString());

    /** Sets the "SwapXAndYAxis" property at all coordinate systems found in the
        given diagram.

        "vertical==true" for bar charts, "vertical==false" for column charts
     */
    static void setVertical( const ::com::sun::star::uno::Reference<
                                 ::com::sun::star::chart2::XDiagram > & xDiagram,
                             bool bVertical = true );

    /** Gets the "SwapXAndYAxis" property at all coordinate systems found in the
        given diagram.

        "vertical==true" for bar charts, "vertical==false" for column charts
    */
    static bool getVertical( const ::com::sun::star::uno::Reference<
                                 ::com::sun::star::chart2::XDiagram > & xDiagram,
                             bool& rbOutFoundResult, bool& rbOutAmbiguousResult );

    static StackMode getStackMode(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDiagram > & xDiagram,
        bool& rbFound, bool& rbAmbiguous
        );

    /** @param bOnlyAtFirstChartType
            If </TRUE>, the stacking mode is only set at the series found inside
            the first chart type.  This is the standard for all current
            templates (the only template that has more than one chart-type and
            allows stacking is bar/line combi, and for this the stacking only
            applies to the first chart type/the bars)
     */
    static void setStackMode(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDiagram > & xDiagram,
        StackMode eStackMode,
        bool bOnlyAtFirstChartType = false
        );

    /** Retrieves the stackmode of the first DataSeries or none. If the series have differing stack
        modes, rbAmbiguous is set to true. If no series is there rbFound is set to false.

        @param xCorrespondingCoordinateSystem
            The coordinate system in which the given chart type xChartType is
            located.  (This is needed for determining percent stacking.  If
            omitted, the result will just indicate "not stacked", "stacked" or
            "ambiguous")
     */
    static StackMode getStackModeFromChartType(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartType > & xChartType,
        bool& rbFound, bool& rbAmbiguous,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XCoordinateSystem > & xCorrespondingCoordinateSystem =
                ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XCoordinateSystem >()
        );

    /** Returns the dimension found for all chart types in the tree.  If the
        dimension is not unique, 0 is returned.
     */
    static sal_Int32 getDimension(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDiagram > & xDiagram );

    /** Sets the dimension of the diagram given.

        1. Sets the dimension of all used ChartTypes
        2. Adapts the DataSeriesTree to reflect the new dimension
        3. If new coordinate-systems have to be created, adapts the
           XCoordinateSystemContainer of the diagram.
     */
    static void setDimension(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDiagram > & xDiagram,
        sal_Int32 nNewDimensionCount );

    /** Replaces all occurences of xCooSysToReplace in the tree with
        xReplacement in the diagram's tree
     */
    static void replaceCoordinateSystem(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDiagram > & xDiagram,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XCoordinateSystem > & xCooSysToReplace,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XCoordinateSystem > & xReplacement );

    static bool isSeriesAttachedToMainAxis(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeries >& xDataSeries );

    static bool attachSeriesToAxis( bool bMainAxis,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeries >& xSeries,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDiagram >& xDiagram,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );

    static ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XAxis > getAttachedAxis(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeries >& xSeries,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDiagram >& xDiagram );

    static ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartType >
        getChartTypeOfSeries(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDiagram >& xDiagram,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDataSeries >& xSeries );

    static ::com::sun::star::uno::Reference<
    ::com::sun::star::chart2::XCoordinateSystem >
        getCoordinateSystemOfChartType(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDiagram >& xDiagram,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XChartType >& xChartType );

    static ::std::vector<
            ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeries > >
        getDataSeriesFromDiagram(
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDiagram > & xDiagram );

    /** return all data series in this diagram grouped by chart-types
     */
    static ::com::sun::star::uno::Sequence<
               ::com::sun::star::uno::Sequence<
                   ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries > > >
        getDataSeriesGroups(
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDiagram > & xDiagram );

    static bool isCategoryDiagram(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDiagram >& xDiagram );

    static void setCategoriesToDiagram(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::data::XLabeledDataSequence >& xCategories,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDiagram >& xDiagram,
            bool bSetAxisType = false, // when this flag is true ...
            bool bCategoryAxis = true);// set the AxisType to CATEGORY or back to REALNUMBER

    static ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XLabeledDataSequence >
        getCategoriesFromDiagram(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDiagram > & xDiagram );

    static ::com::sun::star::uno::Sequence< rtl::OUString >
        generateAutomaticCategories(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XChartDocument > & xChartDoc );

    static ::com::sun::star::uno::Sequence< rtl::OUString >
        generateAutomaticCategories(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XCoordinateSystem > & xCooSys );

    static void generateAutomaticCategoriesFromChartType(
            ::com::sun::star::uno::Sequence< rtl::OUString >& rRet,
            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType );

    static ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartType >
        getChartTypeByIndex( const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDiagram >& xDiagram, sal_Int32 nIndex );

    static ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XChartType > >
        getChartTypesFromDiagram(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDiagram > & xDiagram );

    static bool areChartTypesCompatible( const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XChartType >& xFirstType,
                const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XChartType >& xSecondType );


    /**
        * Test if a series can be moved.
        *
        * @param xDiagram
        *  Reference to the diagram that contains the series.
        *
        * @param xGivenDataSeries
        *  Reference to the series that should be tested for moving.
        *
        * @param bForward
        *  Direction of the move to be checked.
        *
        * @returns </TRUE> if the series can be moved.
        *
        */
    static bool isSeriesMoveable(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDiagram >& xDiagram,
            const ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XDataSeries >& xGivenDataSeries,
            bool bForward );

    /**
        * Move a series forward or backward.
        *
        * @param xDiagram
        *  Reference to the diagram that contains the series.
        *
        * @param xGivenDataSeries
        *  Reference to the series that should be moved.
        *
        * @param bForward
        *  Direction in which the series should be moved.
        *
        * @returns </TRUE> if the series was moved successfully.
        *
        */
    static bool moveSeries(
                const ::com::sun::star::uno::Reference<
                  ::com::sun::star::chart2::XDiagram >& xDiagram,
                const ::com::sun::star::uno::Reference<
          ::com::sun::star::chart2::XDataSeries >& xGivenDataSeries,
                bool bForward );

    static sal_Int32 getIndexOfSeriesWithinChartType(
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XDataSeries >& xDataSeries,
               const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XChartType >& xChartType );

    static bool isSupportingFloorAndWall( const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDiagram > & xDiagram );

    static bool isPieOrDonutChart( const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDiagram >& xDiagram );

    static sal_Int32 getGeometry3D(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDiagram > & xDiagram,
        bool& rbFound, bool& rbAmbiguous );

    static void setGeometry3D(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDiagram > & xDiagram,
        sal_Int32 nNewGeometry );

private:
    // not implemented
    DiagramHelper();

};

} //  namespace chart

// CHART2_DIAGRAMHELPER_HXX
#endif
