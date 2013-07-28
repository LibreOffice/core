/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef CHART2_DIAGRAMHELPER_HXX
#define CHART2_DIAGRAMHELPER_HXX

#include "StackMode.hxx"
#include "charttoolsdllapi.hxx"
#include <com/sun/star/chart2/XAxis.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/XChartTypeTemplate.hpp>
#include <com/sun/star/chart2/XCoordinateSystem.hpp>
#include <com/sun/star/chart2/InterpretedData.hpp>
#include <com/sun/star/chart2/StackingDirection.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XNumberFormats.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>

#include <utility>
#include <vector>

namespace chart
{

enum DiagramPositioningMode
{
    DiagramPositioningMode_AUTO,
    DiagramPositioningMode_EXCLUDING,
    DiagramPositioningMode_INCLUDING
};

class OOO_DLLPUBLIC_CHARTTOOLS DiagramHelper
{
public:
    typedef ::std::pair<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XChartTypeTemplate >,
            OUString >
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
            const OUString & rPreferredTemplateName = OUString());

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
            If </sal_True>, the stacking mode is only set at the series found inside
            the first chart type.  This is the standard for all current
            templates (the only template that has more than one chart-type and
            allows stacking is bar/line combi, and for this the stacking only
            applies to the first chart type/the bars)
     */
    static void setStackMode(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDiagram > & xDiagram,
        StackMode eStackMode,
        bool bOnlyAtFirstChartType = true
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

    /** Replaces all occurrences of xCooSysToReplace in the tree with
        xReplacement in the diagram's tree
     */
    SAL_DLLPRIVATE static void replaceCoordinateSystem(
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
            ::com::sun::star::uno::XComponentContext > & xContext,
        bool bAdaptAxes=true );

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

    static ::com::sun::star::uno::Sequence< OUString >
        getExplicitSimpleCategories(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XChartDocument > & xChartDoc );

    SAL_DLLPRIVATE static ::com::sun::star::uno::Sequence< OUString >
        generateAutomaticCategoriesFromCooSys(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XCoordinateSystem > & xCooSys );

    static void switchToDateCategories(
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XChartDocument > & xChartDoc );

    static void switchToTextCategories(
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XChartDocument > & xChartDoc );

    static bool isSupportingDateAxis( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram >& xDiagram );
    static bool isDateNumberFormat( sal_Int32 nNumberFormat, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormats >& xNumberFormats );
    static sal_Int32 getDateNumberFormat( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >& xNumberFormatsSupplier );

    static sal_Int32 getPercentNumberFormat( const ::com::sun::star::uno::Reference<
                ::com::sun::star::util::XNumberFormatsSupplier >& xNumberFormatsSupplier );

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

    SAL_DLLPRIVATE static bool areChartTypesCompatible( const ::com::sun::star::uno::Reference<
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
        * @returns </sal_True> if the series can be moved.
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
        * @returns </sal_True> if the series was moved successfully.
        *
        */
    static bool moveSeries(
                const ::com::sun::star::uno::Reference<
                  ::com::sun::star::chart2::XDiagram >& xDiagram,
                const ::com::sun::star::uno::Reference<
          ::com::sun::star::chart2::XDataSeries >& xGivenDataSeries,
                bool bForward );

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

    //returns integer from constant group ::com::sun::star::chart::MissingValueTreatment
    static sal_Int32 getCorrectedMissingValueTreatment(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDiagram > & xDiagram,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XChartType >& xChartType );

    static DiagramPositioningMode getDiagramPositioningMode( const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDiagram > & xDiagram );

    static bool setDiagramPositioning( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xChartModel,
        const ::com::sun::star::awt::Rectangle& rPosRect /*100th mm*/ );

    static ::com::sun::star::awt::Rectangle getDiagramRectangleFromModel( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xChartModel );

    static bool switchDiagramPositioningToExcludingPositioning( const ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XModel >& xChartModel
        , bool bResetModifiedState //set model back to unchanged if it was unchanged before
        , bool bConvertAlsoFromAutoPositioning );

private:
    // not implemented
    DiagramHelper();

};

} //  namespace chart

// CHART2_DIAGRAMHELPER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
