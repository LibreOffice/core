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

#ifndef _CHART2_CHARTTYPEHELPER_HXX
#define _CHART2_CHARTTYPEHELPER_HXX

#include <com/sun/star/chart2/XChartType.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/drawing/Direction3D.hpp>
#include "charttoolsdllapi.hxx"

namespace chart
{

/**
*/

class OOO_DLLPUBLIC_CHARTTOOLS ChartTypeHelper
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
    //starting value for bars or baseline for areas for example
    static bool     isSupportingBaseValue(           const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType );
    static bool     shiftCategoryPosAtXAxisPerDefault(     const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType );
    static bool     isSupportingAxisPositioning(     const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType, sal_Int32 nDimensionCount, sal_Int32 nDimensionIndex );
    static bool     isSupportingDateAxis( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType, sal_Int32 nDimensionCount, sal_Int32 nDimensionIndex );

    //returns sequence of ::com::sun::star::chart::DataLabelPlacement
    static ::com::sun::star::uno::Sequence < sal_Int32 > getSupportedLabelPlacements(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType, sal_Int32 nDimensionCount, sal_Bool bSwapXAndY
        , const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries >& xSeries );

    //returns sequence of ::com::sun::star::chart::MissingValueTreatment
    static ::com::sun::star::uno::Sequence < sal_Int32 > getSupportedMissingValueTreatments(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType );

    SAL_DLLPRIVATE static ::com::sun::star::drawing::Direction3D getDefaultSimpleLightDirection( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType );
    SAL_DLLPRIVATE static ::com::sun::star::drawing::Direction3D getDefaultRealisticLightDirection( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType );
    SAL_DLLPRIVATE static sal_Int32 getDefaultDirectLightColor( bool bSimple, const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType );
    SAL_DLLPRIVATE static sal_Int32 getDefaultAmbientLightColor( bool bSimple, const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType );
    static sal_Int32 getNumberOfDisplayedSeries( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType, sal_Int32 nNumberOfSeries );
    SAL_DLLPRIVATE static bool noBordersForSimpleScheme( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType );

    static bool isSeriesInFrontOfAxisLine( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType );

    static sal_Int32 //one of ::com::sun::star::chart2::AxisType
                    getAxisType( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType
                                , sal_Int32 nDimensionIndex );

    static OUString getRoleOfSequenceForYAxisNumberFormatDetection( const ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XChartType >& xChartType );

    static OUString getRoleOfSequenceForDataLabelNumberFormatDetection( const ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XChartType >& xChartType );

    static bool shouldLabelNumberFormatKeyBeDetectedFromYAxis( const ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XChartType >& xChartType );

    static bool isSupportingOnlyDeepStackingFor3D( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >& xChartType );
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
