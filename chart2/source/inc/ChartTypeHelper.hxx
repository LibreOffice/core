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

#include <com/sun/star/drawing/Direction3D.hpp>
#include "charttoolsdllapi.hxx"

namespace com::sun::star::chart2 { class XChartType; }
namespace com::sun::star::chart2 { class XDataSeries; }

namespace chart
{

class OOO_DLLPUBLIC_CHARTTOOLS ChartTypeHelper
{
public:
    static bool isSupportingGeometryProperties(  const css::uno::Reference< css::chart2::XChartType >& xChartType, sal_Int32 nDimensionCount );
    static bool isSupportingStatisticProperties( const css::uno::Reference< css::chart2::XChartType >& xChartType, sal_Int32 nDimensionCount );
    static bool isSupportingRegressionProperties(const css::uno::Reference< css::chart2::XChartType >& xChartType, sal_Int32 nDimensionCount );
    static bool isSupportingMainAxis(            const css::uno::Reference< css::chart2::XChartType >& xChartType, sal_Int32 nDimensionCount, sal_Int32 nDimensionIndex );
    static bool isSupportingSecondaryAxis(       const css::uno::Reference< css::chart2::XChartType >& xChartType, sal_Int32 nDimensionCount );
    static bool isSupportingAreaProperties(      const css::uno::Reference< css::chart2::XChartType >& xChartType, sal_Int32 nDimensionCount );
    static bool isSupportingSymbolProperties(    const css::uno::Reference< css::chart2::XChartType >& xChartType, sal_Int32 nDimensionCount );
    static bool isSupportingOverlapAndGapWidthProperties( const css::uno::Reference< css::chart2::XChartType >& xChartType, sal_Int32 nDimensionCount );
    static bool isSupportingBarConnectors(       const css::uno::Reference< css::chart2::XChartType >& xChartType, sal_Int32 nDimensionCount );
    static bool isSupportingRightAngledAxes(     const css::uno::Reference< css::chart2::XChartType >& xChartType );
    static bool isSupportingAxisSideBySide(      const css::uno::Reference< css::chart2::XChartType >& xChartType, sal_Int32 nDimensionCount );
    static bool isSupportingStartingAngle(       const css::uno::Reference< css::chart2::XChartType >& xChartType );
    //starting value for bars or baseline for areas for example
    static bool isSupportingBaseValue(           const css::uno::Reference< css::chart2::XChartType >& xChartType );
    static bool isSupportingAxisPositioning(     const css::uno::Reference< css::chart2::XChartType >& xChartType, sal_Int32 nDimensionCount, sal_Int32 nDimensionIndex );
    static bool isSupportingDateAxis( const css::uno::Reference< css::chart2::XChartType >& xChartType, sal_Int32 nDimensionIndex );
    static bool isSupportingComplexCategory( const css::uno::Reference< css::chart2::XChartType >& xChartType );
    static bool isSupportingCategoryPositioning( const css::uno::Reference< css::chart2::XChartType >& xChartType, sal_Int32 nDimensionCount );

    //returns sequence of css::chart::DataLabelPlacement
    static css::uno::Sequence < sal_Int32 > getSupportedLabelPlacements(
        const css::uno::Reference< css::chart2::XChartType >& xChartType, bool bSwapXAndY
        , const css::uno::Reference< css::chart2::XDataSeries >& xSeries );

    //returns sequence of css::chart::MissingValueTreatment
    static css::uno::Sequence < sal_Int32 > getSupportedMissingValueTreatments(
        const css::uno::Reference< css::chart2::XChartType >& xChartType );

    SAL_DLLPRIVATE static css::drawing::Direction3D getDefaultSimpleLightDirection( const css::uno::Reference< css::chart2::XChartType >& xChartType );
    SAL_DLLPRIVATE static css::drawing::Direction3D getDefaultRealisticLightDirection( const css::uno::Reference< css::chart2::XChartType >& xChartType );
    SAL_DLLPRIVATE static sal_Int32 getDefaultDirectLightColor( bool bSimple, const css::uno::Reference< css::chart2::XChartType >& xChartType );
    SAL_DLLPRIVATE static sal_Int32 getDefaultAmbientLightColor( bool bSimple, const css::uno::Reference< css::chart2::XChartType >& xChartType );
    static sal_Int32 getNumberOfDisplayedSeries( const css::uno::Reference< css::chart2::XChartType >& xChartType, sal_Int32 nNumberOfSeries );
    SAL_DLLPRIVATE static bool noBordersForSimpleScheme( const css::uno::Reference< css::chart2::XChartType >& xChartType );

    static bool isSeriesInFrontOfAxisLine( const css::uno::Reference< css::chart2::XChartType >& xChartType );

    static sal_Int32 //one of css::chart2::AxisType
                    getAxisType( const css::uno::Reference< css::chart2::XChartType >& xChartType
                                , sal_Int32 nDimensionIndex );

    static OUString getRoleOfSequenceForYAxisNumberFormatDetection( const css::uno::Reference<
        css::chart2::XChartType >& xChartType );

    static OUString getRoleOfSequenceForDataLabelNumberFormatDetection( const css::uno::Reference<
        css::chart2::XChartType >& xChartType );

    static bool isSupportingOnlyDeepStackingFor3D( const css::uno::Reference< css::chart2::XChartType >& xChartType );
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
