/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _CHART2_TOOLS_AXISHELPER_HXX
#define _CHART2_TOOLS_AXISHELPER_HXX

#include "charttoolsdllapi.hxx"
#include "ReferenceSizeProvider.hxx"
#include "ExplicitCategoriesProvider.hxx"
#include <com/sun/star/chart2/XChartType.hpp>
#include <com/sun/star/chart2/XCoordinateSystem.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>

#include <vector>

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class OOO_DLLPUBLIC_CHARTTOOLS AxisHelper
{
public:
    static ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XScaling > createLinearScaling();
    static ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XScaling > createLogarithmicScaling( double fBase = 10.0 );

    static ::com::sun::star::chart2::ScaleData createDefaultScale();

    static void removeExplicitScaling( ::com::sun::star::chart2::ScaleData& rScaleData );

    static bool isLogarithmic( const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XScaling >& xScaling );

    static void checkDateAxis( ::com::sun::star::chart2::ScaleData& rScale, ExplicitCategoriesProvider* pExplicitCategoriesProvider, bool bChartTypeAllowsDateAxis );
    static ::com::sun::star::chart2::ScaleData getDateCheckedScale( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XAxis >& xAxis, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xChartModel );

    static sal_Int32 getExplicitNumberFormatKeyForAxis(
                  const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XAxis >& xAxis
                , const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XCoordinateSystem >& xCorrespondingCoordinateSystem
                , const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >& xNumberFormatsSupplier
                , bool bSearchForParallelAxisIfNothingIsFound );

    static ::com::sun::star::uno::Reference<
           ::com::sun::star::chart2::XAxis >
        createAxis( sal_Int32 nDimensionIndex, bool bMainAxis
                , const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram >& xDiagram
                , const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext
                , ReferenceSizeProvider * pRefSizeProvider = 0 );

    static ::com::sun::star::uno::Reference<
           ::com::sun::star::chart2::XAxis >
        createAxis(
            sal_Int32 nDimensionIndex
            , sal_Int32 nAxisIndex // 0==main or 1==secondary axis
            , const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XCoordinateSystem >& xCooSys
            , const ::com::sun::star::uno::Reference<
                    ::com::sun::star::uno::XComponentContext > & xContext
            , ReferenceSizeProvider * pRefSizeProvider = 0 );

    static void showAxis( sal_Int32 nDimensionIndex, bool bMainAxis
                , const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram >& xDiagram
                , const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext
                , ReferenceSizeProvider * pRefSizeProvider = 0 );

    static void showGrid( sal_Int32 nDimensionIndex, sal_Int32 nCooSysIndex, bool bMainGrid
                , const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram >& xDiagram
                , const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext );

    static void hideAxis( sal_Int32 nDimensionIndex, bool bMainAxis
                , const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram >& xDiagram );
    static void hideGrid( sal_Int32 nDimensionIndex, sal_Int32 nCooSysIndex, bool bMainGrid
                , const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram >& xDiagram );

    static sal_Bool isAxisShown( sal_Int32 nDimensionIndex, bool bMainAxis
                , const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram >& xDiagram );
    static sal_Bool isGridShown( sal_Int32 nDimensionIndex, sal_Int32 nCooSysIndex, bool bMainGrid
                , const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram >& xDiagram );

    static void makeAxisVisible( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XAxis >& xAxis );
    static void makeGridVisible( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xGridProperties );

    static void makeAxisInvisible( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XAxis >& xAxis );
    static void makeGridInvisible( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xGridProperties );

    static void hideAxisIfNoDataIsAttached( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XAxis >& xAxis
                                          , const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram >& xDiagram);

    SAL_DLLPRIVATE static sal_Bool areAxisLabelsVisible( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xAxisProperties );
    static sal_Bool isAxisVisible( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XAxis >& xAxis );
    static sal_Bool isGridVisible( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xGridProperties );

    static ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XCoordinateSystem >
        getCoordinateSystemByIndex(
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDiagram >& xDiagram
            , sal_Int32 nIndex );

    static ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XCoordinateSystem >
        getCoordinateSystemOfAxis(
              const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XAxis >& xAxis
            , const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDiagram >& xDiagram );

    static ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XAxis >
        getAxis( sal_Int32 nDimensionIndex, bool bMainAxis
                , const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XDiagram >& xDiagram );
    static ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XAxis >
        getAxis( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex
            , const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XCoordinateSystem >& xCooSys );

    static ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XAxis >
        getCrossingMainAxis( const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XAxis >& xAxis
            , const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XCoordinateSystem >& xCooSys );

    static ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XAxis >
        getParallelAxis( const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XAxis >& xAxis
            , const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XDiagram >& xDiagram );

    static ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet >
        getGridProperties( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XCoordinateSystem >& xCooSys
                , sal_Int32 nDimensionIndex
                , sal_Int32 nAxisIndex //0: Primary axies, 1: secondary axis
                , sal_Int32 nSubGridIndex = -1 //-1: Main Grid; 0: First SubGrid etc
                );

    static sal_Int32 getDimensionIndexOfAxis(
              const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XAxis >& xAxis
            , const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XDiagram >& xDiagram );

    static bool getIndicesForAxis(
              const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XAxis >& xAxis
            , const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XCoordinateSystem >& xCooSys
            , sal_Int32& rOutDimensionIndex, sal_Int32& rOutAxisIndex );

    static bool getIndicesForAxis(
              const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XAxis >& xAxis
            , const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XDiagram >& xDiagram
            , sal_Int32& rOutCooSysIndex, sal_Int32& rOutDimensionIndex, sal_Int32& rOutAxisIndex );

    /** @param bOnlyVisible if </TRUE>, only axes with property "Show" set to
               </sal_True> are returned
     */
    static ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XAxis > >
            getAllAxesOfDiagram( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XDiagram >& xDiagram
            , bool bOnlyVisible = false );

    /** @param bOnlyVisible if </TRUE>, only axes with property "Show" set to
               </sal_True> are returned
     */
    SAL_DLLPRIVATE static std::vector<
                ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XAxis > >
            getAllAxesOfCoordinateSystem( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XCoordinateSystem >& xCooSys
            , bool bOnlyVisible = false );

    static ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Reference<
                    ::com::sun::star::beans::XPropertySet > >
            getAllGrids( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XDiagram >& xDiagram );

    static void getAxisOrGridPossibilities( ::com::sun::star::uno::Sequence< sal_Bool >& rPossibilityList
        , const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram>& xDiagram, sal_Bool bAxis=sal_True );

    static void getAxisOrGridExcistence( ::com::sun::star::uno::Sequence< sal_Bool >& rExcistenceList
        , const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram>& xDiagram, sal_Bool bAxis=sal_True );

    static bool changeVisibilityOfGrids( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram>& xDiagram
                        , const ::com::sun::star::uno::Sequence< sal_Bool >& rOldExistenceList
                        , const ::com::sun::star::uno::Sequence< sal_Bool >& rNewExistenceList
                        , const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext );

    static bool changeVisibilityOfAxes( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram>& xDiagram
                        , const ::com::sun::star::uno::Sequence< sal_Bool >& rOldExistenceList
                        , const ::com::sun::star::uno::Sequence< sal_Bool >& rNewExistenceList
                        , const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext
                        , ReferenceSizeProvider * pRefSizeProvider = 0 );

    static bool shouldAxisBeDisplayed( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XAxis >& xAxis
                                       , const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XCoordinateSystem >& xCooSys );
    static bool isSecondaryYAxisNeeded( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XCoordinateSystem >& xCooSys );

    static ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartType >
        getChartTypeByIndex( const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XCoordinateSystem >& xCooSys, sal_Int32 nIndex );

    static void setRTLAxisLayout( const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XCoordinateSystem >& xCooSys );

    static ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartType >
        getFirstChartTypeWithSeriesAttachedToAxisIndex( const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDiagram >& xDiagram, const sal_Int32 nAttachedAxisIndex );

    static bool isAxisPositioningEnabled();
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
