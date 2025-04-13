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
#include "ChartType.hxx"
#include <rtl/ref.hxx>

namespace chart
{
class ChartType;
class DataSeries;

class ChartTypeHelper
{
public:
    //starting value for bars or baseline for areas for example
    static bool shiftCategoryPosAtXAxisPerDefault(     const rtl::Reference< ::chart::ChartType >& xChartType );

    //returns sequence of css::chart::DataLabelPlacement
    static css::uno::Sequence < sal_Int32 > getSupportedLabelPlacements(
        const rtl::Reference< ::chart::ChartType >& xChartType, bool bSwapXAndY
        , const rtl::Reference< ::chart::DataSeries >& xSeries );

    //returns sequence of css::chart::MissingValueTreatment
    static css::uno::Sequence < sal_Int32 > getSupportedMissingValueTreatments(
        const rtl::Reference< ::chart::ChartType >& xChartType );

    static css::drawing::Direction3D getDefaultSimpleLightDirection( const rtl::Reference< ::chart::ChartType >& xChartType );
    static css::drawing::Direction3D getDefaultRealisticLightDirection( const rtl::Reference< ::chart::ChartType >& xChartType );
    static sal_Int32 getDefaultDirectLightColor( bool bSimple, const rtl::Reference< ::chart::ChartType >& xChartType );
    static sal_Int32 getDefaultAmbientLightColor( bool bSimple, const rtl::Reference< ::chart::ChartType >& xChartType );
    static sal_Int32 getNumberOfDisplayedSeries( const rtl::Reference< ::chart::ChartType >& xChartType, sal_Int32 nNumberOfSeries );
    static bool noBordersForSimpleScheme( const rtl::Reference< ::chart::ChartType >& xChartType );

    static OUString getRoleOfSequenceForYAxisNumberFormatDetection( const rtl::Reference<
        ::chart::ChartType >& xChartType );

    static OUString getRoleOfSequenceForDataLabelNumberFormatDetection( const rtl::Reference<
        ::chart::ChartType >& xChartType );
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
