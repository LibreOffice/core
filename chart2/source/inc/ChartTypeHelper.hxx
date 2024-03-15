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
#include <com/sun/star/drawing/Direction3D.hpp>
#include "charttoolsdllapi.hxx"
#include "ChartType.hxx"
#include <rtl/ref.hxx>

namespace com::sun::star::chart2 { class XChartType; }
namespace com::sun::star::chart2 { class XDataSeries; }

namespace chart
{
class ChartType;
class DataSeries;

class UNLESS_MERGELIBS(OOO_DLLPUBLIC_CHARTTOOLS) ChartTypeHelper
{
public:
    static bool isSupportingGeometryProperties(  const rtl::Reference< ::chart::ChartType >& xChartType, sal_Int32 nDimensionCount );
    static bool isSupportingStatisticProperties( const rtl::Reference< ::chart::ChartType >& xChartType, sal_Int32 nDimensionCount );
    static bool isSupportingRegressionProperties(const rtl::Reference< ::chart::ChartType >& xChartType, sal_Int32 nDimensionCount );
    static bool isSupportingMainAxis(            const rtl::Reference< ::chart::ChartType >& xChartType, sal_Int32 nDimensionCount, sal_Int32 nDimensionIndex );
    static bool isSupportingSecondaryAxis(       const rtl::Reference< ::chart::ChartType >& xChartType, sal_Int32 nDimensionCount );
    static bool isSupportingAreaProperties(      const rtl::Reference< ::chart::ChartType >& xChartType, sal_Int32 nDimensionCount );
    static bool isSupportingSymbolProperties(    const rtl::Reference< ::chart::ChartType >& xChartType, sal_Int32 nDimensionCount );
    static bool isSupportingOverlapAndGapWidthProperties( const rtl::Reference< ::chart::ChartType >& xChartType, sal_Int32 nDimensionCount );
    static bool isSupportingBarConnectors(       const rtl::Reference< ::chart::ChartType >& xChartType, sal_Int32 nDimensionCount );
    static bool isSupportingRightAngledAxes(     const rtl::Reference< ::chart::ChartType >& xChartType );
    static bool isSupportingAxisSideBySide(      const rtl::Reference< ::chart::ChartType >& xChartType, sal_Int32 nDimensionCount );
    static bool isSupportingStartingAngle(       const rtl::Reference< ::chart::ChartType >& xChartType );
    //starting value for bars or baseline for areas for example
    static bool isSupportingBaseValue(           const rtl::Reference< ::chart::ChartType >& xChartType );
    static bool isSupportingAxisPositioning(     const rtl::Reference< ::chart::ChartType >& xChartType, sal_Int32 nDimensionCount, sal_Int32 nDimensionIndex );
    static bool isSupportingDateAxis( const rtl::Reference< ::chart::ChartType >& xChartType, sal_Int32 nDimensionIndex );
    static bool isSupportingComplexCategory( const rtl::Reference< ::chart::ChartType >& xChartType );
    static bool isSupportingCategoryPositioning( const rtl::Reference< ::chart::ChartType >& xChartType, sal_Int32 nDimensionCount );
    static bool shiftCategoryPosAtXAxisPerDefault(     const rtl::Reference< ::chart::ChartType >& xChartType );

    //returns sequence of css::chart::DataLabelPlacement
    static css::uno::Sequence < sal_Int32 > getSupportedLabelPlacements(
        const rtl::Reference< ::chart::ChartType >& xChartType, bool bSwapXAndY
        , const rtl::Reference< ::chart::DataSeries >& xSeries );

    //returns sequence of css::chart::MissingValueTreatment
    static css::uno::Sequence < sal_Int32 > getSupportedMissingValueTreatments(
        const rtl::Reference< ::chart::ChartType >& xChartType );

    SAL_DLLPRIVATE static css::drawing::Direction3D getDefaultSimpleLightDirection( const rtl::Reference< ::chart::ChartType >& xChartType );
    SAL_DLLPRIVATE static css::drawing::Direction3D getDefaultRealisticLightDirection( const rtl::Reference< ::chart::ChartType >& xChartType );
    SAL_DLLPRIVATE static sal_Int32 getDefaultDirectLightColor( bool bSimple, const rtl::Reference< ::chart::ChartType >& xChartType );
    SAL_DLLPRIVATE static sal_Int32 getDefaultAmbientLightColor( bool bSimple, const rtl::Reference< ::chart::ChartType >& xChartType );
    static sal_Int32 getNumberOfDisplayedSeries( const rtl::Reference< ::chart::ChartType >& xChartType, sal_Int32 nNumberOfSeries );
    SAL_DLLPRIVATE static bool noBordersForSimpleScheme( const rtl::Reference< ::chart::ChartType >& xChartType );

    static bool isSeriesInFrontOfAxisLine( const rtl::Reference< ::chart::ChartType >& xChartType );

    static sal_Int32 //one of css::chart2::AxisType
                    getAxisType( const rtl::Reference< ::chart::ChartType >& xChartType
                                , sal_Int32 nDimensionIndex );

    static OUString getRoleOfSequenceForYAxisNumberFormatDetection( const rtl::Reference<
        ::chart::ChartType >& xChartType );

    static OUString getRoleOfSequenceForDataLabelNumberFormatDetection( const rtl::Reference<
        ::chart::ChartType >& xChartType );

    static bool isSupportingOnlyDeepStackingFor3D( const rtl::Reference< ::chart::ChartType >& xChartType );
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
