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
#include <sal/types.h>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>

#include <memory>

namespace com::sun::star::beans { class XPropertySet; }
namespace com::sun::star::uno { template <typename > class Reference; }
namespace com::sun::star::util { class XNumberFormatsSupplier; }
namespace com::sun::star::awt { struct Rectangle; }
class SvxShape;

namespace chart
{
class Axis;
class BaseCoordinateSystem;
class DrawModelWrapper;
class ChartModel;
struct ExplicitIncrementData;
struct ExplicitScaleData;

class SAL_LOPLUGIN_ANNOTATE("crosscast") ExplicitValueProvider
{
public:
    /** Gives calculated scale and increment values for a given xAxis in the current view.
        In contrast to the model data these explicit values are always complete as missing auto properties are calculated.
        If the given Axis could not be found or for another reason no correct output can be given false is returned.
     */
    virtual bool getExplicitValuesForAxis(
        rtl::Reference< ::chart::Axis > xAxis
        , ExplicitScaleData&  rExplicitScale
        , ExplicitIncrementData& rExplicitIncrement )=0;

    /** for rotated objects the shape size and position differs from the visible rectangle
        if bSnapRect is set to true you get the resulting visible position (left-top) and size
    */
    virtual css::awt::Rectangle
        getRectangleOfObject( const OUString& rObjectCID, bool bSnapRect=false )=0;

    virtual css::awt::Rectangle getDiagramRectangleExcludingAxes()=0;

    virtual rtl::Reference< SvxShape >
        getShapeForCID( const OUString& rObjectCID )=0;

    virtual std::shared_ptr< DrawModelWrapper > getDrawModelWrapper() = 0;

    static css::awt::Rectangle
        AddSubtractAxisTitleSizes(
                ChartModel& rModel
            , ExplicitValueProvider* pChartView
            , const css::awt::Rectangle& rPositionAndSize, bool bSubtract );

    static sal_Int32 getExplicitNumberFormatKeyForAxis(
              const rtl::Reference< ::chart::Axis >& xAxis
            , const rtl::Reference< ::chart::BaseCoordinateSystem > & xCorrespondingCoordinateSystem
            , const rtl::Reference<::chart::ChartModel>& xChartDoc);

    static sal_Int32 getExplicitNumberFormatKeyForDataLabel(
            const css::uno::Reference< css::beans::XPropertySet >& xSeriesOrPointProp );

    static sal_Int32 getExplicitPercentageNumberFormatKeyForDataLabel(
            const css::uno::Reference< css::beans::XPropertySet >& xSeriesOrPointProp
            , const css::uno::Reference< css::util::XNumberFormatsSupplier >& xNumberFormatsSupplier );

protected:
    ~ExplicitValueProvider() = default;
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
