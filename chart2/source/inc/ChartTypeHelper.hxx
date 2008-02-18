/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartTypeHelper.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-18 15:59:02 $
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

#ifndef _CHART2_CHARTTYPEHELPER_HXX
#define _CHART2_CHARTTYPEHELPER_HXX

#ifndef _COM_SUN_STAR_CHART2_XCHARTTYPE_HPP_
#include <com/sun/star/chart2/XChartType.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_AXISTYPE_HPP_
#include <com/sun/star/chart2/AxisType.hpp>
#endif
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>

#ifndef _COM_SUN_STAR_DRAWING_DIRECTION3D_HPP_
#include <com/sun/star/drawing/Direction3D.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class ChartTypeHelper
{
public:
    static sal_Bool isSupportingGeometryProperties(  const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType, sal_Int32 nDimensionCount );
    static sal_Bool isSupportingStatisticProperties( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType, sal_Int32 nDimensionCount );
    static sal_Bool isSupportingRegressionProperties(const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType, sal_Int32 nDimensionCount );
    static sal_Bool isSupportingMainAxis(            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType, sal_Int32 nDimensionCount, sal_Int32 nDimensionIndex );
    static sal_Bool isSupportingSecondaryAxis(       const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType, sal_Int32 nDimensionCount, sal_Int32 nDimensionIndex );
    static sal_Bool isSupportingAreaProperties(      const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType, sal_Int32 nDimensionCount );
    static sal_Bool isSupportingSymbolProperties(    const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType, sal_Int32 nDimensionCount );
    static sal_Bool isSupportingOverlapAndGapWidthProperties( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType, sal_Int32 nDimensionCount );
    static sal_Bool isSupportingBarConnectors(       const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType, sal_Int32 nDimensionCount );
    static sal_Bool isSupportingRightAngledAxes(     const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType );
    static bool     isSupportingAxisSideBySide(      const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType, sal_Int32 nDimensionCount );
    static bool     isSupportingStartingAngle(       const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType );

    //returns sequence of ::com::sun::star::chart::DataLabelPlacement
    static ::com::sun::star::uno::Sequence < sal_Int32 > getSupportedLabelPlacements(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType, sal_Int32 nDimensionCount, sal_Bool bSwapXAndY
        , const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries >& xSeries );

    static ::com::sun::star::drawing::Direction3D getDefaultSimpleLightDirection( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType );
    static ::com::sun::star::drawing::Direction3D getDefaultRealisticLightDirection( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType );
    static sal_Int32 getDefaultDirectLightColor( bool bSimple, const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType );
    static sal_Int32 getDefaultAmbientLightColor( bool bSimple, const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType );
    static sal_Int32 getNumberOfDisplayedSeries( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType, sal_Int32 nNumberOfSeries );
    static bool noBordersForSimpleScheme( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType );

    static sal_Int32 //one of ::com::sun::star::chart2::AxisType
                    getAxisType( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType
                                , sal_Int32 nDimensionIndex );

    /** Determines if all data series of a chart type are attached to the same
        axis.

        @param rOutAxisIndex If, and only if, </TRUE> is returned this
               out-parameter is filled with the index (0 or 1) of the axis to
               which all series are attached.
     */
    static bool allSeriesAttachedToSameAxis(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType,
        sal_Int32 & rOutAxisIndex );
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
