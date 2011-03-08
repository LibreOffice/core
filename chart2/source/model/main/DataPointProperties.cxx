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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "DataPointProperties.hxx"
#include "macros.hxx"
#include "LineProperties.hxx"
#include "FillProperties.hxx"

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/drawing/RectanglePoint.hpp>

#include <com/sun/star/chart2/DataPointGeometry3D.hpp>
#include <com/sun/star/chart2/DataPointLabel.hpp>
#include <com/sun/star/chart2/Symbol.hpp>

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;

namespace chart
{

void DataPointProperties::AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    // DataPointProperties
    // ===================

    // Common
    // ------
    rOutProperties.push_back(
        Property( C2U( "Color" ),
                  PROP_DATAPOINT_COLOR,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID         // "maybe auto"
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "Transparency" ),
                  PROP_DATAPOINT_TRANSPARENCY,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    // Fill Properties
    // ---------------
    rOutProperties.push_back(
        Property( C2U( "FillStyle" ),
                  PROP_DATAPOINT_FILL_STYLE,
                  ::getCppuType( reinterpret_cast< const drawing::FillStyle * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "TransparencyGradientName" ),
                  PROP_DATAPOINT_TRANSPARENCY_GRADIENT_NAME,
                  ::getCppuType( reinterpret_cast< const ::rtl::OUString * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "GradientName" ),
                  PROP_DATAPOINT_GRADIENT_NAME,
                  ::getCppuType( reinterpret_cast< const ::rtl::OUString * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));


    rOutProperties.push_back(
        beans::Property( C2U( "GradientStepCount" ),
                  PROP_DATAPOINT_GRADIENT_STEPCOUNT,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "HatchName" ),
                  PROP_DATAPOINT_HATCH_NAME,
                  ::getCppuType( reinterpret_cast< const ::rtl::OUString * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "FillBitmapName" ),
                  PROP_DATAPOINT_FILL_BITMAP_NAME,
                  ::getCppuType( reinterpret_cast< const ::rtl::OUString * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID  ));
    rOutProperties.push_back(
        Property( C2U( "FillBackground" ),
                  PROP_DATAPOINT_FILL_BACKGROUND,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));

    // border for filled objects
    rOutProperties.push_back(
        Property( C2U( "BorderColor" ),
                  PROP_DATAPOINT_BORDER_COLOR,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID         // "maybe auto"
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "BorderStyle" ),
                  PROP_DATAPOINT_BORDER_STYLE,
                  ::getCppuType( reinterpret_cast< const drawing::LineStyle * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "BorderWidth" ),
                  PROP_DATAPOINT_BORDER_WIDTH,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "BorderDashName" ),
                  PROP_DATAPOINT_BORDER_DASH_NAME,
                  ::getCppuType( reinterpret_cast< const ::rtl::OUString * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));
    rOutProperties.push_back(
        Property( C2U( "BorderTransparency" ),
                  PROP_DATAPOINT_BORDER_TRANSPARENCY,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    // Line Properties
    // ---------------
    rOutProperties.push_back(
        Property( C2U( "LineStyle" ),
                  LineProperties::PROP_LINE_STYLE,
                  ::getCppuType( reinterpret_cast< const drawing::LineStyle * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "LineWidth" ),
                  LineProperties::PROP_LINE_WIDTH,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
         Property( C2U( "LineDash" ),
                   LineProperties::PROP_LINE_DASH,
                   ::getCppuType( reinterpret_cast< const drawing::LineDash * >(0)),
                   beans::PropertyAttribute::BOUND
                   | beans::PropertyAttribute::MAYBEVOID ));
    rOutProperties.push_back(
        Property( C2U( "LineDashName" ),
                  LineProperties::PROP_LINE_DASH_NAME,
                  ::getCppuType( reinterpret_cast< const ::rtl::OUString * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    // FillProperties
    // bitmap properties
    rOutProperties.push_back(
        Property( C2U( "FillBitmapOffsetX" ),
                  FillProperties::PROP_FILL_BITMAP_OFFSETX,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "FillBitmapOffsetY" ),
                  FillProperties::PROP_FILL_BITMAP_OFFSETY,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "FillBitmapPositionOffsetX" ),
                  FillProperties::PROP_FILL_BITMAP_POSITION_OFFSETX,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "FillBitmapPositionOffsetY" ),
                  FillProperties::PROP_FILL_BITMAP_POSITION_OFFSETY,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));


    rOutProperties.push_back(
        Property( C2U( "FillBitmapRectanglePoint" ),
                  FillProperties::PROP_FILL_BITMAP_RECTANGLEPOINT,
                  ::getCppuType( reinterpret_cast< const drawing::RectanglePoint * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "FillBitmapLogicalSize" ),
                  FillProperties::PROP_FILL_BITMAP_LOGICALSIZE,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "FillBitmapSizeX" ),
                  FillProperties::PROP_FILL_BITMAP_SIZEX,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "FillBitmapSizeY" ),
                  FillProperties::PROP_FILL_BITMAP_SIZEY,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "FillBitmapMode" ),
                  FillProperties::PROP_FILL_BITMAP_MODE,
                  ::getCppuType( reinterpret_cast< const drawing::BitmapMode * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    // others
    rOutProperties.push_back(
        Property( C2U( "Symbol" ),
                  PROP_DATAPOINT_SYMBOL_PROP,
                  ::getCppuType( reinterpret_cast< const chart2::Symbol * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "Offset" ),
                  PROP_DATAPOINT_OFFSET,
                  ::getCppuType( reinterpret_cast< const double * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "Geometry3D" ),
                  PROP_DATAPOINT_GEOMETRY3D,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "Label" ),
                  PROP_DATAPOINT_LABEL,
                  ::getCppuType( reinterpret_cast< const chart2::DataPointLabel * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "LabelSeparator" ),
                  PROP_DATAPOINT_LABEL_SEPARATOR,
                  ::getCppuType( reinterpret_cast< const ::rtl::OUString * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "NumberFormat" ),
                  PROP_DATAPOINT_NUMBER_FORMAT,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    //additional 'PercentageNumberFormat'
    rOutProperties.push_back(
        Property( C2U( "PercentageNumberFormat" ),
                  PROP_DATAPOINT_PERCENTAGE_NUMBER_FORMAT,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "LabelPlacement" ),
                  PROP_DATAPOINT_LABEL_PLACEMENT,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "ReferencePageSize" ),
                  PROP_DATAPOINT_REFERENCE_DIAGRAM_SIZE,
                  ::getCppuType( reinterpret_cast< const awt::Size * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

        rOutProperties.push_back(
        Property( C2U( "TextRotation" ),
                  PROP_DATAPOINT_TEXT_ROTATION,
                  ::getCppuType( reinterpret_cast< const double * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    // statistics
    rOutProperties.push_back(
        Property( C2U( "ErrorBarX" ),
                  PROP_DATAPOINT_ERROR_BAR_X,
                  // XPropertySet supporting service ErrorBar
                  ::getCppuType( reinterpret_cast< const uno::Reference< beans::XPropertySet > * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));
    rOutProperties.push_back(
        Property( C2U( "ErrorBarY" ),
                  PROP_DATAPOINT_ERROR_BAR_Y,
                  // XPropertySet supporting service ErrorBar
                  ::getCppuType( reinterpret_cast< const uno::Reference< beans::XPropertySet > * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));
    rOutProperties.push_back(
        Property( C2U( "ShowErrorBox" ),
                  PROP_DATAPOINT_SHOW_ERROR_BOX,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));
    rOutProperties.push_back(
        Property( C2U( "PercentDiagonal" ),
                  PROP_DATAPOINT_PERCENT_DIAGONAL,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));
}

void DataPointProperties::AddDefaultsToMap(
    ::chart::tPropertyValueMap & rOutMap )
{
    PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, PROP_DATAPOINT_COLOR, 0x0099ccff ); // blue 8
    PropertyHelper::setPropertyValueDefault< sal_Int16 >( rOutMap, PROP_DATAPOINT_TRANSPARENCY, 0 );

    //fill
    PropertyHelper::setPropertyValueDefault( rOutMap, PROP_DATAPOINT_FILL_STYLE, drawing::FillStyle_SOLID );
    PropertyHelper::setEmptyPropertyValueDefault( rOutMap, PROP_DATAPOINT_TRANSPARENCY_GRADIENT_NAME );
    PropertyHelper::setEmptyPropertyValueDefault( rOutMap, PROP_DATAPOINT_GRADIENT_NAME );
    PropertyHelper::setEmptyPropertyValueDefault( rOutMap, PROP_DATAPOINT_HATCH_NAME );
    PropertyHelper::setEmptyPropertyValueDefault( rOutMap, PROP_DATAPOINT_FILL_BITMAP_NAME );
    PropertyHelper::setPropertyValueDefault( rOutMap, PROP_DATAPOINT_FILL_BACKGROUND, false );

    //border
    PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, PROP_DATAPOINT_BORDER_COLOR, 0x000000 ); // black
    PropertyHelper::setPropertyValueDefault( rOutMap, PROP_DATAPOINT_BORDER_STYLE, drawing::LineStyle_SOLID ); // drawing::LineStyle_NONE
    PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, PROP_DATAPOINT_BORDER_WIDTH, 0 );
    PropertyHelper::setEmptyPropertyValueDefault( rOutMap, PROP_DATAPOINT_BORDER_DASH_NAME );
    PropertyHelper::setPropertyValueDefault< sal_Int16 >( rOutMap, PROP_DATAPOINT_BORDER_TRANSPARENCY, 0 );

    //line
    PropertyHelper::setPropertyValueDefault( rOutMap, LineProperties::PROP_LINE_STYLE, drawing::LineStyle_SOLID );
    PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, LineProperties::PROP_LINE_WIDTH, 0 );
    PropertyHelper::setPropertyValueDefault( rOutMap, LineProperties::PROP_LINE_DASH, drawing::LineDash());
    PropertyHelper::setEmptyPropertyValueDefault( rOutMap, LineProperties::PROP_LINE_DASH_NAME );

    //fill bitmap
    PropertyHelper::setPropertyValueDefault< sal_Int16 >( rOutMap, FillProperties::PROP_FILL_BITMAP_OFFSETX, 0 );
    PropertyHelper::setPropertyValueDefault< sal_Int16 >( rOutMap, FillProperties::PROP_FILL_BITMAP_OFFSETY, 0 );
    PropertyHelper::setPropertyValueDefault< sal_Int16 >( rOutMap, FillProperties::PROP_FILL_BITMAP_POSITION_OFFSETX, 0 );
    PropertyHelper::setPropertyValueDefault< sal_Int16 >( rOutMap, FillProperties::PROP_FILL_BITMAP_POSITION_OFFSETY, 0 );
    PropertyHelper::setPropertyValueDefault( rOutMap, FillProperties::PROP_FILL_BITMAP_RECTANGLEPOINT, drawing::RectanglePoint_MIDDLE_MIDDLE );
    PropertyHelper::setPropertyValueDefault( rOutMap, FillProperties::PROP_FILL_BITMAP_LOGICALSIZE, true );

    PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, FillProperties::PROP_FILL_BITMAP_SIZEX, 0 );
    PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, FillProperties::PROP_FILL_BITMAP_SIZEY, 0 );
    PropertyHelper::setPropertyValueDefault( rOutMap, FillProperties::PROP_FILL_BITMAP_MODE, drawing::BitmapMode_REPEAT );

    //others
    chart2::Symbol aSymbProp;
    aSymbProp.Style = chart2::SymbolStyle_NONE;
    aSymbProp.StandardSymbol = 0;
    aSymbProp.Size = awt::Size( 250, 250 ); // ca. 7pt x 7pt (7pt=246.94)
    aSymbProp.BorderColor = 0x000000;       // Black
    aSymbProp.FillColor = 0xee4000;         // OrangeRed2
    PropertyHelper::setPropertyValueDefault( rOutMap, PROP_DATAPOINT_SYMBOL_PROP, aSymbProp );

    PropertyHelper::setPropertyValueDefault< double >( rOutMap, PROP_DATAPOINT_OFFSET, 0.0 );
    PropertyHelper::setPropertyValueDefault( rOutMap, PROP_DATAPOINT_GEOMETRY3D, chart2::DataPointGeometry3D::CUBOID );

    PropertyHelper::setPropertyValueDefault(
        rOutMap, PROP_DATAPOINT_LABEL,
        chart2::DataPointLabel(
            sal_False, // ShowNumber
            sal_False, // ShowNumberInPercent
            sal_False, // ShowCategoryName
            sal_False  // ShowLegendSymbol
            ));

    PropertyHelper::setPropertyValueDefault< rtl::OUString >( rOutMap, PROP_DATAPOINT_LABEL_SEPARATOR, C2U(" ") );

    //@todo maybe choose a different one here -> should be dynamically that of the attached axis
    PropertyHelper::setPropertyValueDefault( rOutMap, PROP_DATAPOINT_ERROR_BAR_X, uno::Reference< beans::XPropertySet >());
    PropertyHelper::setPropertyValueDefault( rOutMap, PROP_DATAPOINT_ERROR_BAR_Y, uno::Reference< beans::XPropertySet >());
    PropertyHelper::setPropertyValueDefault< sal_Int16 >( rOutMap, PROP_DATAPOINT_PERCENT_DIAGONAL, 0 );

    PropertyHelper::setPropertyValueDefault< double >( rOutMap, PROP_DATAPOINT_TEXT_ROTATION, 0.0 );
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
