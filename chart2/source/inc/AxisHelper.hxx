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
#ifndef INCLUDED_CHART2_SOURCE_INC_AXISHELPER_HXX
#define INCLUDED_CHART2_SOURCE_INC_AXISHELPER_HXX

#include "charttoolsdllapi.hxx"
#include <com/sun/star/chart2/ScaleData.hpp>

#include <vector>

namespace chart { class ChartModel; }
namespace chart { class ExplicitCategoriesProvider; }
namespace chart { class ReferenceSizeProvider; }
namespace com::sun::star::beans { class XPropertySet; }
namespace com::sun::star::chart2 { class XAxis; }
namespace com::sun::star::chart2 { class XChartDocument; }
namespace com::sun::star::chart2 { class XChartType; }
namespace com::sun::star::chart2 { class XCoordinateSystem; }
namespace com::sun::star::chart2 { class XDiagram; }
namespace com::sun::star::chart2 { class XScaling; }
namespace com::sun::star::uno { class XComponentContext; }

namespace chart
{

class OOO_DLLPUBLIC_CHARTTOOLS AxisHelper
{
public:
    static css::uno::Reference< css::chart2::XScaling > createLinearScaling();
    static css::uno::Reference< css::chart2::XScaling > createLogarithmicScaling( double fBase = 10.0 );

    static css::chart2::ScaleData createDefaultScale();

    static void removeExplicitScaling( css::chart2::ScaleData& rScaleData );

    static bool isLogarithmic( const css::uno::Reference< css::chart2::XScaling >& xScaling );

    static void checkDateAxis( css::chart2::ScaleData& rScale, ExplicitCategoriesProvider* pExplicitCategoriesProvider, bool bChartTypeAllowsDateAxis );
    static css::chart2::ScaleData getDateCheckedScale( const css::uno::Reference< css::chart2::XAxis >& xAxis, ChartModel& rModel );

    static sal_Int32 getExplicitNumberFormatKeyForAxis(
                  const css::uno::Reference< css::chart2::XAxis >& xAxis
                , const css::uno::Reference< css::chart2::XCoordinateSystem >& xCorrespondingCoordinateSystem
                , const css::uno::Reference< css::chart2::XChartDocument>& xChartDoc
                , bool bSearchForParallelAxisIfNothingIsFound );

    static css::uno::Reference<
           css::chart2::XAxis >
        createAxis( sal_Int32 nDimensionIndex, bool bMainAxis
                , const css::uno::Reference< css::chart2::XDiagram >& xDiagram
                , const css::uno::Reference< css::uno::XComponentContext >& xContext
                , ReferenceSizeProvider * pRefSizeProvider = nullptr );

    static css::uno::Reference< css::chart2::XAxis >
        createAxis(
            sal_Int32 nDimensionIndex
            , sal_Int32 nAxisIndex // 0==main or 1==secondary axis
            , const css::uno::Reference< css::chart2::XCoordinateSystem >& xCooSys
            , const css::uno::Reference< css::uno::XComponentContext > & xContext
            , ReferenceSizeProvider * pRefSizeProvider = nullptr );

    static void showAxis( sal_Int32 nDimensionIndex, bool bMainAxis
                , const css::uno::Reference< css::chart2::XDiagram >& xDiagram
                , const css::uno::Reference< css::uno::XComponentContext >& xContext
                , ReferenceSizeProvider * pRefSizeProvider = nullptr );

    static void showGrid( sal_Int32 nDimensionIndex, sal_Int32 nCooSysIndex, bool bMainGrid
                , const css::uno::Reference< css::chart2::XDiagram >& xDiagram );

    static void hideAxis( sal_Int32 nDimensionIndex, bool bMainAxis
                , const css::uno::Reference< css::chart2::XDiagram >& xDiagram );
    static void hideGrid( sal_Int32 nDimensionIndex, sal_Int32 nCooSysIndex, bool bMainGrid
                , const css::uno::Reference< css::chart2::XDiagram >& xDiagram );

    static bool isAxisShown( sal_Int32 nDimensionIndex, bool bMainAxis
                , const css::uno::Reference< css::chart2::XDiagram >& xDiagram );
    static bool isGridShown( sal_Int32 nDimensionIndex, sal_Int32 nCooSysIndex, bool bMainGrid
                , const css::uno::Reference< css::chart2::XDiagram >& xDiagram );

    static void makeAxisVisible( const css::uno::Reference< css::chart2::XAxis >& xAxis );
    static void makeGridVisible( const css::uno::Reference< css::beans::XPropertySet >& xGridProperties );

    static void makeAxisInvisible( const css::uno::Reference< css::chart2::XAxis >& xAxis );
    static void makeGridInvisible( const css::uno::Reference< css::beans::XPropertySet >& xGridProperties );

    static void hideAxisIfNoDataIsAttached( const css::uno::Reference< css::chart2::XAxis >& xAxis
                                          , const css::uno::Reference< css::chart2::XDiagram >& xDiagram);

    SAL_DLLPRIVATE static bool areAxisLabelsVisible( const css::uno::Reference< css::beans::XPropertySet >& xAxisProperties );
    static bool isAxisVisible( const css::uno::Reference< css::chart2::XAxis >& xAxis );
    static bool isGridVisible( const css::uno::Reference< css::beans::XPropertySet >& xGridProperties );

    static css::uno::Reference< css::chart2::XCoordinateSystem >
        getCoordinateSystemByIndex(
            const css::uno::Reference< css::chart2::XDiagram >& xDiagram
            , sal_Int32 nIndex );

    static css::uno::Reference< css::chart2::XCoordinateSystem >
        getCoordinateSystemOfAxis(
              const css::uno::Reference< css::chart2::XAxis >& xAxis
            , const css::uno::Reference< css::chart2::XDiagram >& xDiagram );

    static css::uno::Reference< css::chart2::XAxis >
        getAxis( sal_Int32 nDimensionIndex, bool bMainAxis
                , const css::uno::Reference< css::chart2::XDiagram >& xDiagram );
    static css::uno::Reference< css::chart2::XAxis >
        getAxis( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex
            , const css::uno::Reference< css::chart2::XCoordinateSystem >& xCooSys );

    static css::uno::Reference< css::chart2::XAxis >
        getCrossingMainAxis( const css::uno::Reference< css::chart2::XAxis >& xAxis
            , const css::uno::Reference< css::chart2::XCoordinateSystem >& xCooSys );

    static css::uno::Reference< css::chart2::XAxis >
        getParallelAxis( const css::uno::Reference< css::chart2::XAxis >& xAxis
            , const css::uno::Reference< css::chart2::XDiagram >& xDiagram );

    static css::uno::Reference< css::beans::XPropertySet >
        getGridProperties( const css::uno::Reference< css::chart2::XCoordinateSystem >& xCooSys
                , sal_Int32 nDimensionIndex
                , sal_Int32 nAxisIndex //0: Primary axis, 1: secondary axis
                , sal_Int32 nSubGridIndex //-1: Main Grid; 0: First SubGrid etc
                );

    static sal_Int32 getDimensionIndexOfAxis(
              const css::uno::Reference< css::chart2::XAxis >& xAxis
            , const css::uno::Reference< css::chart2::XDiagram >& xDiagram );

    static bool getIndicesForAxis(
              const css::uno::Reference< css::chart2::XAxis >& xAxis
            , const css::uno::Reference< css::chart2::XCoordinateSystem >& xCooSys
            , sal_Int32& rOutDimensionIndex, sal_Int32& rOutAxisIndex );

    static bool getIndicesForAxis(
              const css::uno::Reference< css::chart2::XAxis >& xAxis
            , const css::uno::Reference< css::chart2::XDiagram >& xDiagram
            , sal_Int32& rOutCooSysIndex, sal_Int32& rOutDimensionIndex, sal_Int32& rOutAxisIndex );

    /** @param bOnlyVisible if </TRUE>, only axes with property "Show" set to
               </sal_True> are returned
     */
    static css::uno::Sequence< css::uno::Reference< css::chart2::XAxis > >
            getAllAxesOfDiagram( const css::uno::Reference< css::chart2::XDiagram >& xDiagram
            , bool bOnlyVisible = false );

    /** @param bOnlyVisible if </TRUE>, only axes with property "Show" set to
               </sal_True> are returned
     */
    SAL_DLLPRIVATE static std::vector< css::uno::Reference< css::chart2::XAxis > >
            getAllAxesOfCoordinateSystem( const css::uno::Reference< css::chart2::XCoordinateSystem >& xCooSys
            , bool bOnlyVisible = false );

    static css::uno::Sequence< css::uno::Reference< css::beans::XPropertySet > >
            getAllGrids( const css::uno::Reference< css::chart2::XDiagram >& xDiagram );

    static void getAxisOrGridPossibilities( css::uno::Sequence< sal_Bool >& rPossibilityList
        , const css::uno::Reference< css::chart2::XDiagram>& xDiagram, bool bAxis=true );

    static void getAxisOrGridExistence( css::uno::Sequence< sal_Bool >& rExistenceList
        , const css::uno::Reference< css::chart2::XDiagram>& xDiagram, bool bAxis=true );

    static bool changeVisibilityOfGrids( const css::uno::Reference< css::chart2::XDiagram>& xDiagram
                        , const css::uno::Sequence< sal_Bool >& rOldExistenceList
                        , const css::uno::Sequence< sal_Bool >& rNewExistenceList );

    static bool changeVisibilityOfAxes( const css::uno::Reference< css::chart2::XDiagram>& xDiagram
                        , const css::uno::Sequence< sal_Bool >& rOldExistenceList
                        , const css::uno::Sequence< sal_Bool >& rNewExistenceList
                        , const css::uno::Reference< css::uno::XComponentContext >& xContext
                        , ReferenceSizeProvider * pRefSizeProvider );

    static bool shouldAxisBeDisplayed( const css::uno::Reference< css::chart2::XAxis >& xAxis
                                       , const css::uno::Reference< css::chart2::XCoordinateSystem >& xCooSys );
    static bool isSecondaryYAxisNeeded( const css::uno::Reference<
                    css::chart2::XCoordinateSystem >& xCooSys );

    static css::uno::Reference< css::chart2::XChartType >
        getChartTypeByIndex( const css::uno::Reference< css::chart2::XCoordinateSystem >& xCooSys,
                             sal_Int32 nIndex );

    static void setRTLAxisLayout( const css::uno::Reference< css::chart2::XCoordinateSystem >& xCooSys );

    static css::uno::Reference< css::chart2::XChartType >
        getFirstChartTypeWithSeriesAttachedToAxisIndex( const css::uno::Reference< css::chart2::XDiagram >& xDiagram, const sal_Int32 nAttachedAxisIndex );

    static bool isAxisPositioningEnabled();
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
