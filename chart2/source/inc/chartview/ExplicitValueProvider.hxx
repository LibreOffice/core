/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
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
        getRectangleOfObject( const rtl::OUString& rObjectCID, bool bSnapRect=false )=0;

    virtual ::com::sun::star::awt::Rectangle getDiagramRectangleExcludingAxes()=0;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        getShapeForCID( const rtl::OUString& rObjectCID )=0;

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
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
