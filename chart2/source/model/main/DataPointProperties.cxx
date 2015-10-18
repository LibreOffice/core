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

#include "DataPointProperties.hxx"
#include "macros.hxx"
#include "LinePropertiesHelper.hxx"
#include "FillProperties.hxx"
#include <unonames.hxx>

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

    // Common

    rOutProperties.push_back(
        Property( "Color",
                  PROP_DATAPOINT_COLOR,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID         // "maybe auto"
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "FillColor",
                  PROP_DATAPOINT_COLOR,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID         // "maybe auto"
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "Transparency",
                  PROP_DATAPOINT_TRANSPARENCY,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "FillTransparence",
                  PROP_DATAPOINT_TRANSPARENCY,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    // Fill Properties
    rOutProperties.push_back(
        Property( "FillStyle",
                  PROP_DATAPOINT_FILL_STYLE,
                  cppu::UnoType<drawing::FillStyle>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "TransparencyGradientName",
                  PROP_DATAPOINT_TRANSPARENCY_GRADIENT_NAME,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( "FillTransparenceGradientName",
                  PROP_DATAPOINT_TRANSPARENCY_GRADIENT_NAME,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( "GradientName",
                  PROP_DATAPOINT_GRADIENT_NAME,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( "FillGradientName",
                  PROP_DATAPOINT_GRADIENT_NAME,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        beans::Property( "GradientStepCount",
                  PROP_DATAPOINT_GRADIENT_STEPCOUNT,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        beans::Property( "FillGradientStepCount",
                  PROP_DATAPOINT_GRADIENT_STEPCOUNT,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( "HatchName",
                  PROP_DATAPOINT_HATCH_NAME,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( "FillHatchName",
                  PROP_DATAPOINT_HATCH_NAME,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( "FillBitmapName",
                  PROP_DATAPOINT_FILL_BITMAP_NAME,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID  ));

    rOutProperties.push_back(
        Property( "FillBackground",
                  PROP_DATAPOINT_FILL_BACKGROUND,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));

    // border for filled objects
    rOutProperties.push_back(
        Property( "BorderColor",
                  PROP_DATAPOINT_BORDER_COLOR,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID         // "maybe auto"
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "BorderStyle",
                  PROP_DATAPOINT_BORDER_STYLE,
                  cppu::UnoType<drawing::LineStyle>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "BorderWidth",
                  PROP_DATAPOINT_BORDER_WIDTH,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "BorderDashName",
                  PROP_DATAPOINT_BORDER_DASH_NAME,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( "BorderTransparency",
                  PROP_DATAPOINT_BORDER_TRANSPARENCY,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    // Line Properties

    rOutProperties.push_back(
        Property( "LineColor",
                  PROP_DATAPOINT_BORDER_COLOR,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "LineStyle",
                  LinePropertiesHelper::PROP_LINE_STYLE,
                  cppu::UnoType<drawing::LineStyle>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "LineWidth",
                  LinePropertiesHelper::PROP_LINE_WIDTH,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
         Property( "LineDash",
                   LinePropertiesHelper::PROP_LINE_DASH,
                   cppu::UnoType<drawing::LineDash>::get(),
                   beans::PropertyAttribute::BOUND
                   | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( "LineDashName",
                  LinePropertiesHelper::PROP_LINE_DASH_NAME,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( "LineTransparence",
                  PROP_DATAPOINT_BORDER_TRANSPARENCY,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    // FillProperties
    // bitmap properties
    rOutProperties.push_back(
        Property( "FillBitmapOffsetX",
                  FillProperties::PROP_FILL_BITMAP_OFFSETX,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "FillBitmapOffsetY",
                  FillProperties::PROP_FILL_BITMAP_OFFSETY,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "FillBitmapPositionOffsetX",
                  FillProperties::PROP_FILL_BITMAP_POSITION_OFFSETX,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "FillBitmapPositionOffsetY",
                  FillProperties::PROP_FILL_BITMAP_POSITION_OFFSETY,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "FillBitmapRectanglePoint",
                  FillProperties::PROP_FILL_BITMAP_RECTANGLEPOINT,
                  cppu::UnoType<drawing::RectanglePoint>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "FillBitmapLogicalSize",
                  FillProperties::PROP_FILL_BITMAP_LOGICALSIZE,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "FillBitmapSizeX",
                  FillProperties::PROP_FILL_BITMAP_SIZEX,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "FillBitmapSizeY",
                  FillProperties::PROP_FILL_BITMAP_SIZEY,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "FillBitmapMode",
                  FillProperties::PROP_FILL_BITMAP_MODE,
                  cppu::UnoType<drawing::BitmapMode>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    // others
    rOutProperties.push_back(
        Property( "Symbol",
                  PROP_DATAPOINT_SYMBOL_PROP,
                  cppu::UnoType<chart2::Symbol>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( "Offset",
                  PROP_DATAPOINT_OFFSET,
                  cppu::UnoType<double>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( "Geometry3D",
                  PROP_DATAPOINT_GEOMETRY3D,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( CHART_UNONAME_NUMFMT,
                  PROP_DATAPOINT_NUMBER_FORMAT,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( CHART_UNONAME_LINK_TO_SRC_NUMFMT,
                  PROP_DATAPOINT_LINK_NUMBERFORMAT_TO_SOURCE,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    //additional 'PercentageNumberFormat'
    rOutProperties.push_back(
        Property( "PercentageNumberFormat",
                  PROP_DATAPOINT_PERCENTAGE_NUMBER_FORMAT,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( "LabelPlacement",
                  PROP_DATAPOINT_LABEL_PLACEMENT,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( "ReferencePageSize",
                  PROP_DATAPOINT_REFERENCE_DIAGRAM_SIZE,
                  cppu::UnoType<awt::Size>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( "TextRotation",
                  PROP_DATAPOINT_TEXT_ROTATION,
                  cppu::UnoType<double>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    // statistics
    rOutProperties.push_back(
        Property( CHART_UNONAME_ERRORBAR_X,
                  PROP_DATAPOINT_ERROR_BAR_X,
                  // XPropertySet supporting service ErrorBar
                  cppu::UnoType<beans::XPropertySet>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));
    rOutProperties.push_back(
        Property( CHART_UNONAME_ERRORBAR_Y,
                  PROP_DATAPOINT_ERROR_BAR_Y,
                  // XPropertySet supporting service ErrorBar
                  cppu::UnoType<beans::XPropertySet>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));
    rOutProperties.push_back(
        Property( "ShowErrorBox",
                  PROP_DATAPOINT_SHOW_ERROR_BOX,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));
    rOutProperties.push_back(
        Property( "PercentDiagonal",
                  PROP_DATAPOINT_PERCENT_DIAGONAL,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    // Properties specific to data label.

    rOutProperties.push_back(
        Property( CHART_UNONAME_LABEL,
                  PROP_DATAPOINT_LABEL,
                  cppu::UnoType<chart2::DataPointLabel>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "TextWordWrap",
                  PROP_DATAPOINT_TEXT_WORD_WRAP,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( CHART_UNONAME_LABEL_SEP,
                  PROP_DATAPOINT_LABEL_SEPARATOR,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( CHART_UNONAME_LABEL_BORDER_STYLE,
                  PROP_DATAPOINT_LABEL_BORDER_STYLE,
                  cppu::UnoType<drawing::LineStyle>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( CHART_UNONAME_LABEL_BORDER_COLOR,
                  PROP_DATAPOINT_LABEL_BORDER_COLOR,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID         // "maybe auto"
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( CHART_UNONAME_LABEL_BORDER_WIDTH,
                  PROP_DATAPOINT_LABEL_BORDER_WIDTH,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
         Property( CHART_UNONAME_LABEL_BORDER_DASH,
                   PROP_DATAPOINT_LABEL_BORDER_DASH,
                   cppu::UnoType<drawing::LineDash>::get(),
                   beans::PropertyAttribute::BOUND
                   | beans::PropertyAttribute::MAYBEVOID ));
    rOutProperties.push_back(
        Property( CHART_UNONAME_LABEL_BORDER_DASHNAME,
                  PROP_DATAPOINT_LABEL_BORDER_DASH_NAME,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));
    rOutProperties.push_back(
        Property( CHART_UNONAME_LABEL_BORDER_TRANS,
                  PROP_DATAPOINT_LABEL_BORDER_TRANS,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
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
    PropertyHelper::setPropertyValueDefault( rOutMap, LinePropertiesHelper::PROP_LINE_STYLE, drawing::LineStyle_SOLID );
    PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, LinePropertiesHelper::PROP_LINE_WIDTH, 0 );
    PropertyHelper::setPropertyValueDefault( rOutMap, LinePropertiesHelper::PROP_LINE_DASH, drawing::LineDash());
    PropertyHelper::setEmptyPropertyValueDefault( rOutMap, LinePropertiesHelper::PROP_LINE_DASH_NAME );

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

    //@todo maybe choose a different one here -> should be dynamically that of the attached axis
    PropertyHelper::setPropertyValueDefault( rOutMap, PROP_DATAPOINT_ERROR_BAR_X, uno::Reference< beans::XPropertySet >());
    PropertyHelper::setPropertyValueDefault( rOutMap, PROP_DATAPOINT_ERROR_BAR_Y, uno::Reference< beans::XPropertySet >());
    PropertyHelper::setPropertyValueDefault< sal_Int16 >( rOutMap, PROP_DATAPOINT_PERCENT_DIAGONAL, 0 );

    PropertyHelper::setPropertyValueDefault< double >( rOutMap, PROP_DATAPOINT_TEXT_ROTATION, 0.0 );

    PropertyHelper::setPropertyValueDefault(rOutMap, PROP_DATAPOINT_LINK_NUMBERFORMAT_TO_SOURCE, true);

    // data label
    PropertyHelper::setPropertyValueDefault(
        rOutMap, PROP_DATAPOINT_LABEL,
        chart2::DataPointLabel(
            sal_False, // ShowNumber
            sal_False, // ShowNumberInPercent
            sal_False, // ShowCategoryName
            sal_False  // ShowLegendSymbol
            ));

    PropertyHelper::setPropertyValueDefault( rOutMap, PROP_DATAPOINT_TEXT_WORD_WRAP, false );
    PropertyHelper::setPropertyValueDefault< OUString >( rOutMap, PROP_DATAPOINT_LABEL_SEPARATOR, " " );
    PropertyHelper::setPropertyValueDefault<sal_Int32>(rOutMap, PROP_DATAPOINT_LABEL_BORDER_STYLE, drawing::LineStyle_NONE);
    PropertyHelper::setEmptyPropertyValueDefault(rOutMap, PROP_DATAPOINT_LABEL_BORDER_COLOR);
    PropertyHelper::setPropertyValueDefault<sal_Int32>(rOutMap, PROP_DATAPOINT_LABEL_BORDER_WIDTH, 0);
    PropertyHelper::setPropertyValueDefault(rOutMap, PROP_DATAPOINT_LABEL_BORDER_DASH, drawing::LineDash());
    PropertyHelper::setEmptyPropertyValueDefault(rOutMap, PROP_DATAPOINT_LABEL_BORDER_DASH_NAME);
    PropertyHelper::setPropertyValueDefault<sal_Int16>(rOutMap, PROP_DATAPOINT_LABEL_BORDER_TRANS, 0);
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
