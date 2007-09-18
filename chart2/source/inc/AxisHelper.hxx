/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AxisHelper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-18 14:57:04 $
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
#ifndef _CHART2_TOOLS_AXISHELPER_HXX
#define _CHART2_TOOLS_AXISHELPER_HXX

#include "ReferenceSizeProvider.hxx"

#ifndef _COM_SUN_STAR_CHART2_XCHARTTYPE_HPP_
#include <com/sun/star/chart2/XChartType.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XCOORDINATESYSTEM_HPP_
#include <com/sun/star/chart2/XCoordinateSystem.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDIAGRAM_HPP_
#include <com/sun/star/chart2/XDiagram.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#include <vector>

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class AxisHelper
{
public:
    static ::com::sun::star::chart2::ScaleData createDefaultScale();

    static void removeExplicitScaling( ::com::sun::star::chart2::ScaleData& rScaleData );

    static bool isLogarithmic( const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XScaling >& xScaling );

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

    static sal_Bool areAxisLabelsVisible( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xAxisProperties );
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
               </TRUE> are returned
     */
    static ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XAxis > >
            getAllAxesOfDiagram( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XDiagram >& xDiagram
            , bool bOnlyVisible = false );

    /** @param bOnlyVisible if </TRUE>, only axes with property "Show" set to
               </TRUE> are returned
     */
    static std::vector<
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
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
