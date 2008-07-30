/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ChartTypeHelper.hxx,v $
 * $Revision: 1.16 $
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

#ifndef _CHART2_CHARTTYPEHELPER_HXX
#define _CHART2_CHARTTYPEHELPER_HXX

#include <com/sun/star/chart2/XChartType.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/drawing/Direction3D.hpp>

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
    static bool     shiftTicksAtXAxisPerDefault(     const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType );

    //returns sequence of ::com::sun::star::chart::DataLabelPlacement
    static ::com::sun::star::uno::Sequence < sal_Int32 > getSupportedLabelPlacements(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType, sal_Int32 nDimensionCount, sal_Bool bSwapXAndY
        , const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries >& xSeries );

    //returns sequence of ::com::sun::star::chart::MissingValueTreatment
    static ::com::sun::star::uno::Sequence < sal_Int32 > getSupportedMissingValueTreatments(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType );

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
