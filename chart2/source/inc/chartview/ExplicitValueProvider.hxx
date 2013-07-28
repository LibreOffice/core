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
#ifndef _CHART2_EXPLICITVALUEPROVIDER_HXX
#define _CHART2_EXPLICITVALUEPROVIDER_HXX

#include "ExplicitScaleValues.hxx"

#include <boost/shared_ptr.hpp>
#include <com/sun/star/chart2/XAxis.hpp>
#include <com/sun/star/chart2/XCoordinateSystem.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include "chartviewdllapi.hxx"

namespace chart
{

/**
*/

class DrawModelWrapper;
class OOO_DLLPUBLIC_CHARTVIEW ExplicitValueProvider
{
public:
    /** Gives calculated scale and increment values for a given xAxis in the current view.
        In contrast to the model data these explicit values are always complete as misssing auto properties are calculated.
        If the given Axis could not be found or for another reason no correct output can be given false is returned.
     */
    virtual sal_Bool getExplicitValuesForAxis(
        ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XAxis > xAxis
        , ExplicitScaleData&  rExplicitScale
        , ExplicitIncrementData& rExplicitIncrement )=0;

    /** for rotated objects the shape size and position differs from the visible rectangle
        if bSnapRect is set to true you get the resulting visible position (left-top) and size
    */
    virtual ::com::sun::star::awt::Rectangle
        getRectangleOfObject( const OUString& rObjectCID, bool bSnapRect=false )=0;

    virtual ::com::sun::star::awt::Rectangle getDiagramRectangleExcludingAxes()=0;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        getShapeForCID( const OUString& rObjectCID )=0;

    virtual ::boost::shared_ptr< DrawModelWrapper > getDrawModelWrapper() = 0;

    static const com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ExplicitValueProvider* getExplicitValueProvider( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xChartView );

    static ::com::sun::star::awt::Rectangle
        addAxisTitleSizes(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::frame::XModel >& xChartModel
            , const ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XInterface >& xChartView
            , const ::com::sun::star::awt::Rectangle& rExcludingPositionAndSize );

    static ::com::sun::star::awt::Rectangle
        substractAxisTitleSizes(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::frame::XModel >& xChartModel
            , const ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XInterface >& xChartView
            , const ::com::sun::star::awt::Rectangle& rPositionAndSizeIncludingTitles );

    static sal_Int32 getExplicitNumberFormatKeyForAxis(
              const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XAxis >& xAxis
            , const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XCoordinateSystem > & xCorrespondingCoordinateSystem
            , const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >& xNumberFormatsSupplier );

    static sal_Int32 getExplicitNumberFormatKeyForDataLabel(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xSeriesOrPointProp
            , const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries >& xSeries
            , sal_Int32 nPointIndex /*-1 for whole series*/
            , const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram >& xDiagram );

    static sal_Int32 getExplicitPercentageNumberFormatKeyForDataLabel(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xSeriesOrPointProp
            , const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >& xNumberFormatsSupplier );

protected:
    ~ExplicitValueProvider() {}
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
