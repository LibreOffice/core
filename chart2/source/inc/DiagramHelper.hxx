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
#pragma once

#include "StackMode.hxx"
#include "charttoolsdllapi.hxx"
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/chart2/XChartTypeTemplate.hpp>

#include <utility>
#include <vector>

namespace chart { class ChartModel; }
namespace com::sun::star::chart2 { class XAxis; }
namespace com::sun::star::chart2 { class XChartDocument; }
namespace com::sun::star::chart2 { class XCoordinateSystem; }
namespace com::sun::star::chart2 { class XDiagram; }
namespace com::sun::star::frame { class XModel; }
namespace com::sun::star::lang { class XMultiServiceFactory; }
namespace com::sun::star::uno { class XComponentContext; }
namespace com::sun::star::util { class XNumberFormats; }
namespace com::sun::star::util { class XNumberFormatsSupplier; }

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
    typedef std::pair<
            css::uno::Reference< css::chart2::XChartTypeTemplate >,
            OUString >
        tTemplateWithServiceName;

    /** tries to find a template in the chart-type manager that matches the
        given diagram.

        @return
            A pair containing a template with the correct properties set as
            first entry and the service name of the templates second entry.  If
            no template was found both elements are empty.
     */
    static tTemplateWithServiceName
        getTemplateForDiagram(
            const css::uno::Reference< css::chart2::XDiagram > & xDiagram,
            const css::uno::Reference< css::lang::XMultiServiceFactory > & xChartTypeManager);

    /** Sets the "SwapXAndYAxis" property at all coordinate systems found in the
        given diagram.

        "vertical==true" for bar charts, "vertical==false" for column charts
     */
    static void setVertical( const css::uno::Reference< css::chart2::XDiagram > & xDiagram,
                             bool bVertical );

    /** Gets the "SwapXAndYAxis" property at all coordinate systems found in the
        given diagram.

        "vertical==true" for bar charts, "vertical==false" for column charts
    */
    static bool getVertical( const css::uno::Reference< css::chart2::XDiagram > & xDiagram,
                             bool& rbOutFoundResult, bool& rbOutAmbiguousResult );

    static StackMode getStackMode(
        const css::uno::Reference< css::chart2::XDiagram > & xDiagram,
        bool& rbFound, bool& rbAmbiguous
        );

    /** The stacking mode is only set at the series found inside
        the first chart type.  This is the standard for all current
        templates (the only template that has more than one chart-type and
        allows stacking is bar/line combi, and for this the stacking only
        applies to the first chart type/the bars)
     */
    static void setStackMode(
        const css::uno::Reference< css::chart2::XDiagram > & xDiagram,
        StackMode eStackMode
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
        const css::uno::Reference< css::chart2::XChartType > & xChartType,
        bool& rbFound, bool& rbAmbiguous,
        const css::uno::Reference< css::chart2::XCoordinateSystem > & xCorrespondingCoordinateSystem
        );

    /** Returns the dimension found for all chart types in the tree.  If the
        dimension is not unique, 0 is returned.
     */
    static sal_Int32 getDimension(
        const css::uno::Reference< css::chart2::XDiagram > & xDiagram );

    /** Sets the dimension of the diagram given.

        1. Sets the dimension of all used ChartTypes
        2. Adapts the DataSeriesTree to reflect the new dimension
        3. If new coordinate-systems have to be created, adapts the
           XCoordinateSystemContainer of the diagram.
     */
    static void setDimension(
        const css::uno::Reference< css::chart2::XDiagram > & xDiagram,
        sal_Int32 nNewDimensionCount );

    /** Replaces all occurrences of xCooSysToReplace in the tree with
        xReplacement in the diagram's tree
     */
    SAL_DLLPRIVATE static void replaceCoordinateSystem(
        const css::uno::Reference< css::chart2::XDiagram > & xDiagram,
        const css::uno::Reference< css::chart2::XCoordinateSystem > & xCooSysToReplace,
        const css::uno::Reference< css::chart2::XCoordinateSystem > & xReplacement );

    static bool isSeriesAttachedToMainAxis(
        const css::uno::Reference< css::chart2::XDataSeries >& xDataSeries );

    static bool attachSeriesToAxis( bool bMainAxis,
        const css::uno::Reference< css::chart2::XDataSeries >& xSeries,
        const css::uno::Reference< css::chart2::XDiagram >& xDiagram,
        const css::uno::Reference< css::uno::XComponentContext > & xContext,
        bool bAdaptAxes=true );

    static css::uno::Reference< css::chart2::XAxis > getAttachedAxis(
        const css::uno::Reference< css::chart2::XDataSeries >& xSeries,
        const css::uno::Reference< css::chart2::XDiagram >& xDiagram );

    static css::uno::Reference< css::chart2::XChartType >
        getChartTypeOfSeries(
            const css::uno::Reference< css::chart2::XDiagram >& xDiagram,
            const css::uno::Reference< css::chart2::XDataSeries >& xSeries );

    static std::vector< css::uno::Reference< css::chart2::XDataSeries > >
        getDataSeriesFromDiagram(
            const css::uno::Reference< css::chart2::XDiagram > & xDiagram );

    /** return all data series in this diagram grouped by chart-types
     */
    static css::uno::Sequence<
               css::uno::Sequence<
                   css::uno::Reference< css::chart2::XDataSeries > > >
        getDataSeriesGroups(
            const css::uno::Reference< css::chart2::XDiagram > & xDiagram );

    static bool isCategoryDiagram(
            const css::uno::Reference< css::chart2::XDiagram >& xDiagram );

    static void setCategoriesToDiagram(
            const css::uno::Reference< css::chart2::data::XLabeledDataSequence >& xCategories,
            const css::uno::Reference< css::chart2::XDiagram >& xDiagram,
            bool bSetAxisType = false, // when this flag is true ...
            bool bCategoryAxis = true);// set the AxisType to CATEGORY or back to REALNUMBER

    static css::uno::Reference< css::chart2::data::XLabeledDataSequence >
        getCategoriesFromDiagram(
            const css::uno::Reference< css::chart2::XDiagram > & xDiagram );

    static css::uno::Sequence< OUString >
        getExplicitSimpleCategories( ChartModel& rModel );

    SAL_DLLPRIVATE static css::uno::Sequence< OUString >
        generateAutomaticCategoriesFromCooSys(
            const css::uno::Reference< css::chart2::XCoordinateSystem > & xCooSys );

    static void switchToDateCategories(
        const css::uno::Reference< css::chart2::XChartDocument > & xChartDoc );

    static void switchToTextCategories(
        const css::uno::Reference< css::chart2::XChartDocument > & xChartDoc );

    static bool isSupportingDateAxis( const css::uno::Reference< css::chart2::XDiagram >& xDiagram );
    static bool isDateNumberFormat( sal_Int32 nNumberFormat, const css::uno::Reference< css::util::XNumberFormats >& xNumberFormats );
    static sal_Int32 getDateNumberFormat( const css::uno::Reference< css::util::XNumberFormatsSupplier >& xNumberFormatsSupplier );
    static sal_Int32 getDateTimeInputNumberFormat( const css::uno::Reference< css::util::XNumberFormatsSupplier >& xNumberFormatsSupplier, double fNumber );

    static sal_Int32 getPercentNumberFormat( const css::uno::Reference<
                css::util::XNumberFormatsSupplier >& xNumberFormatsSupplier );

    static css::uno::Reference< css::chart2::XChartType >
        getChartTypeByIndex( const css::uno::Reference< css::chart2::XDiagram >& xDiagram, sal_Int32 nIndex );

    static css::uno::Sequence<
            css::uno::Reference< css::chart2::XChartType > >
        getChartTypesFromDiagram(
            const css::uno::Reference< css::chart2::XDiagram > & xDiagram );

    SAL_DLLPRIVATE static bool areChartTypesCompatible( const css::uno::Reference<
                css::chart2::XChartType >& xFirstType,
                const css::uno::Reference< css::chart2::XChartType >& xSecondType );

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
            const css::uno::Reference< css::chart2::XDiagram >& xDiagram,
            const css::uno::Reference< css::chart2::XDataSeries >& xGivenDataSeries,
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
                const css::uno::Reference< css::chart2::XDiagram >& xDiagram,
                const css::uno::Reference< css::chart2::XDataSeries >& xGivenDataSeries,
                bool bForward );

    static bool isSupportingFloorAndWall( const css::uno::Reference< css::chart2::XDiagram > & xDiagram );

    static bool isPieOrDonutChart( const css::uno::Reference< css::chart2::XDiagram >& xDiagram );

    static sal_Int32 getGeometry3D(
        const css::uno::Reference< css::chart2::XDiagram > & xDiagram,
        bool& rbFound, bool& rbAmbiguous );

    static void setGeometry3D(
        const css::uno::Reference< css::chart2::XDiagram > & xDiagram,
        sal_Int32 nNewGeometry );

    //returns integer from constant group css::chart::MissingValueTreatment
    static sal_Int32 getCorrectedMissingValueTreatment(
            const css::uno::Reference< css::chart2::XDiagram > & xDiagram,
            const css::uno::Reference< css::chart2::XChartType >& xChartType );

    static DiagramPositioningMode getDiagramPositioningMode( const css::uno::Reference< css::chart2::XDiagram > & xDiagram );

    static bool setDiagramPositioning( const css::uno::Reference< css::frame::XModel >& xChartModel,
        const css::awt::Rectangle& rPosRect /*100th mm*/ );

    static css::awt::Rectangle getDiagramRectangleFromModel( const css::uno::Reference< css::frame::XModel >& xChartModel );

    static bool switchDiagramPositioningToExcludingPositioning( ChartModel& rModel
        , bool bResetModifiedState //set model back to unchanged if it was unchanged before
        , bool bConvertAlsoFromAutoPositioning );

private:
    DiagramHelper() = delete;

};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
