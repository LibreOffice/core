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

#include <config_options.h>
#include "charttoolsdllapi.hxx"
#include <com/sun/star/chart2/ScaleData.hpp>
#include <rtl/ref.hxx>
#include "GridProperties.hxx"

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
class Axis;
class BaseCoordinateSystem;
class ChartType;
class Diagram;

class UNLESS_MERGELIBS(OOO_DLLPUBLIC_CHARTTOOLS) AxisHelper
{
public:
    static css::uno::Reference< css::chart2::XScaling > createLinearScaling();
    static css::uno::Reference< css::chart2::XScaling > createLogarithmicScaling( double fBase = 10.0 );

    static css::chart2::ScaleData createDefaultScale();

    static void removeExplicitScaling( css::chart2::ScaleData& rScaleData );

    static bool isLogarithmic( const css::uno::Reference< css::chart2::XScaling >& xScaling );

    static void checkDateAxis( css::chart2::ScaleData& rScale, ExplicitCategoriesProvider* pExplicitCategoriesProvider, bool bChartTypeAllowsDateAxis );
    static css::chart2::ScaleData getDateCheckedScale( const rtl::Reference< ::chart::Axis >& xAxis, ChartModel& rModel );

    static sal_Int32 getExplicitNumberFormatKeyForAxis(
                  const rtl::Reference< ::chart::Axis >& xAxis
                , const rtl::Reference< ::chart::BaseCoordinateSystem >& xCorrespondingCoordinateSystem
                , const rtl::Reference< ::chart::ChartModel>& xChartDoc
                , bool bSearchForParallelAxisIfNothingIsFound );

    static rtl::Reference< ::chart::Axis >
        createAxis( sal_Int32 nDimensionIndex, bool bMainAxis
                , const rtl::Reference< ::chart::Diagram >& xDiagram
                , const css::uno::Reference< css::uno::XComponentContext >& xContext
                , ReferenceSizeProvider * pRefSizeProvider = nullptr );

    static rtl::Reference< ::chart::Axis >
        createAxis(
            sal_Int32 nDimensionIndex
            , sal_Int32 nAxisIndex // 0==main or 1==secondary axis
            , const rtl::Reference< ::chart::BaseCoordinateSystem >& xCooSys
            , const css::uno::Reference< css::uno::XComponentContext > & xContext
            , ReferenceSizeProvider * pRefSizeProvider = nullptr );

    static void showAxis( sal_Int32 nDimensionIndex, bool bMainAxis
                , const rtl::Reference< ::chart::Diagram >& xDiagram
                , const css::uno::Reference< css::uno::XComponentContext >& xContext
                , ReferenceSizeProvider * pRefSizeProvider = nullptr );

    static void showGrid( sal_Int32 nDimensionIndex, sal_Int32 nCooSysIndex, bool bMainGrid
                , const rtl::Reference< ::chart::Diagram >& xDiagram );

    static void hideAxis( sal_Int32 nDimensionIndex, bool bMainAxis
                , const rtl::Reference< ::chart::Diagram >& xDiagram );
    static void hideGrid( sal_Int32 nDimensionIndex, sal_Int32 nCooSysIndex, bool bMainGrid
                , const rtl::Reference< ::chart::Diagram >& xDiagram );

    static bool isAxisShown( sal_Int32 nDimensionIndex, bool bMainAxis
                , const rtl::Reference< ::chart::Diagram >& xDiagram );
    static bool isGridShown( sal_Int32 nDimensionIndex, sal_Int32 nCooSysIndex, bool bMainGrid
                , const rtl::Reference< ::chart::Diagram >& xDiagram );

    static void makeAxisVisible( const rtl::Reference< ::chart::Axis >& xAxis );
    static void makeGridVisible( const rtl::Reference< ::chart::GridProperties >& xGridProperties );

    static void makeAxisInvisible( const rtl::Reference< ::chart::Axis >& xAxis );
    static void makeGridInvisible( const rtl::Reference< ::chart::GridProperties >& xGridProperties );

    static void hideAxisIfNoDataIsAttached( const rtl::Reference< ::chart::Axis >& xAxis
                                          , const rtl::Reference< ::chart::Diagram >& xDiagram);

    SAL_DLLPRIVATE static bool areAxisLabelsVisible( const rtl::Reference< ::chart::Axis >& xAxisProperties );
    static bool isAxisVisible( const rtl::Reference< ::chart::Axis >& xAxis );
    static bool isGridVisible( const rtl::Reference< ::chart::GridProperties >& xGridProperties );

    static rtl::Reference< ::chart::BaseCoordinateSystem >
        getCoordinateSystemByIndex(
            const rtl::Reference< ::chart::Diagram >& xDiagram
            , sal_Int32 nIndex );

    static rtl::Reference< ::chart::BaseCoordinateSystem >
        getCoordinateSystemOfAxis(
              const rtl::Reference< ::chart::Axis >& xAxis
            , const rtl::Reference< ::chart::Diagram >& xDiagram );

    static rtl::Reference< ::chart::Axis >
        getAxis( sal_Int32 nDimensionIndex, bool bMainAxis
                , const rtl::Reference< ::chart::Diagram >& xDiagram );
    static rtl::Reference< ::chart::Axis >
        getAxis( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex
            , const rtl::Reference< ::chart::BaseCoordinateSystem >& xCooSys );

    static rtl::Reference< ::chart::Axis >
        getCrossingMainAxis( const rtl::Reference< ::chart::Axis >& xAxis
            , const rtl::Reference< ::chart::BaseCoordinateSystem >& xCooSys );

    static rtl::Reference< ::chart::Axis >
        getParallelAxis( const rtl::Reference< ::chart::Axis >& xAxis
            , const rtl::Reference< ::chart::Diagram >& xDiagram );

    static rtl::Reference< ::chart::GridProperties >
        getGridProperties( const rtl::Reference< ::chart::BaseCoordinateSystem >& xCooSys
                , sal_Int32 nDimensionIndex
                , sal_Int32 nAxisIndex //0: Primary axis, 1: secondary axis
                , sal_Int32 nSubGridIndex //-1: Main Grid; 0: First SubGrid etc
                );

    static sal_Int32 getDimensionIndexOfAxis(
              const rtl::Reference< ::chart::Axis >& xAxis
            , const rtl::Reference< ::chart::Diagram >& xDiagram );

    static bool getIndicesForAxis(
              const rtl::Reference< ::chart::Axis >& xAxis
            , const rtl::Reference< ::chart::BaseCoordinateSystem >& xCooSys
            , sal_Int32& rOutDimensionIndex, sal_Int32& rOutAxisIndex );

    static bool getIndicesForAxis(
              const rtl::Reference< ::chart::Axis >& xAxis
            , const rtl::Reference< ::chart::Diagram >& xDiagram
            , sal_Int32& rOutCooSysIndex, sal_Int32& rOutDimensionIndex, sal_Int32& rOutAxisIndex );

    /** @param bOnlyVisible if </TRUE>, only axes with property "Show" set to
               </sal_True> are returned
     */
    static std::vector< rtl::Reference< ::chart::Axis > >
            getAllAxesOfDiagram( const rtl::Reference< ::chart::Diagram >& xDiagram
            , bool bOnlyVisible = false );

    /** @param bOnlyVisible if </TRUE>, only axes with property "Show" set to
               </sal_True> are returned
     */
    SAL_DLLPRIVATE static std::vector< rtl::Reference< ::chart::Axis > >
            getAllAxesOfCoordinateSystem( const rtl::Reference< ::chart::BaseCoordinateSystem >& xCooSys
            , bool bOnlyVisible = false );

    static std::vector< rtl::Reference< ::chart::GridProperties > >
            getAllGrids( const rtl::Reference< ::chart::Diagram >& xDiagram );

    static void getAxisOrGridPossibilities( css::uno::Sequence< sal_Bool >& rPossibilityList
        , const rtl::Reference< ::chart::Diagram>& xDiagram, bool bAxis=true );

    static void getAxisOrGridExistence( css::uno::Sequence< sal_Bool >& rExistenceList
        , const rtl::Reference< ::chart::Diagram>& xDiagram, bool bAxis=true );

    static bool changeVisibilityOfGrids( const rtl::Reference< ::chart::Diagram>& xDiagram
                        , const css::uno::Sequence< sal_Bool >& rOldExistenceList
                        , const css::uno::Sequence< sal_Bool >& rNewExistenceList );

    static bool changeVisibilityOfAxes( const rtl::Reference< ::chart::Diagram>& xDiagram
                        , const css::uno::Sequence< sal_Bool >& rOldExistenceList
                        , const css::uno::Sequence< sal_Bool >& rNewExistenceList
                        , const css::uno::Reference< css::uno::XComponentContext >& xContext
                        , ReferenceSizeProvider * pRefSizeProvider );

    static bool shouldAxisBeDisplayed( const rtl::Reference< ::chart::Axis >& xAxis
                                       , const rtl::Reference< ::chart::BaseCoordinateSystem >& xCooSys );
    static bool isSecondaryYAxisNeeded( const rtl::Reference<
                    ::chart::BaseCoordinateSystem >& xCooSys );

    static rtl::Reference< ::chart::ChartType >
        getChartTypeByIndex( const rtl::Reference< ::chart::BaseCoordinateSystem >& xCooSys,
                             sal_Int32 nIndex );

    static void setRTLAxisLayout( const rtl::Reference< ::chart::BaseCoordinateSystem >& xCooSys );

    static rtl::Reference< ::chart::ChartType >
        getFirstChartTypeWithSeriesAttachedToAxisIndex( const rtl::Reference< ::chart::Diagram >& xDiagram, const sal_Int32 nAttachedAxisIndex );

    static bool isAxisPositioningEnabled();
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
