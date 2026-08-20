/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <FillProperties.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/RectanglePoint.hpp>
#include <com/sun/star/util/XComplexColor.hpp>

#include <tools/color.hxx>

using namespace css;
namespace chart
{

namespace
{

void lcl_AddPropertiesToVector_without_BitmapProperties( std::vector<beans::Property> & rOutProperties )
{
    rOutProperties.emplace_back( u"FillStyle"_ustr,
                  FillProperties::PROP_FILL_STYLE,
                  cppu::UnoType<drawing::FillStyle>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( u"FillColor"_ustr,
                  FillProperties::PROP_FILL_COLOR,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID         // "maybe auto"
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back(u"FillComplexColor"_ustr,
                  FillProperties::PROP_FILL_COMPLEX_COLOR,
                  cppu::UnoType<util::XComplexColor>::get(),
                  beans::PropertyAttribute::BOUND | beans::PropertyAttribute::MAYBEVOID | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( u"GlowEffectColor"_ustr,
          FillProperties::PROP_FILL_GLOW_COLOR,
          cppu::UnoType<sal_Int32>::get(),
          beans::PropertyAttribute::BOUND
          | beans::PropertyAttribute::MAYBEVOID         // "maybe auto"
          | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( u"GlowEffectRadius"_ustr,
          FillProperties::PROP_FILL_GLOW_RADIUS,
          cppu::UnoType<sal_Int32>::get(),
          beans::PropertyAttribute::BOUND
          | beans::PropertyAttribute::MAYBEVOID         // "maybe auto"
          | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( u"GlowEffectTransparency"_ustr,
      FillProperties::PROP_FILL_GLOW_TRANSPARENCY,
      cppu::UnoType<sal_Int16>::get(),
      beans::PropertyAttribute::BOUND
      | beans::PropertyAttribute::MAYBEVOID         // "maybe auto"
      | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( u"SoftEdgeRadius"_ustr,
      FillProperties::PROP_FILL_SOFTEDGE_RADIUS,
      cppu::UnoType<sal_Int32>::get(),
      beans::PropertyAttribute::BOUND
      | beans::PropertyAttribute::MAYBEVOID         // "maybe auto"
      | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( u"FillTransparence"_ustr,
                  FillProperties::PROP_FILL_TRANSPARENCE,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( u"FillTransparenceGradientName"_ustr,
                  FillProperties::PROP_FILL_TRANSPARENCE_GRADIENT_NAME,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( u"FillGradientName"_ustr,
                  FillProperties::PROP_FILL_GRADIENT_NAME,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( u"FillGradientStepCount"_ustr,
                  FillProperties::PROP_FILL_GRADIENT_STEPCOUNT,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( u"FillHatchName"_ustr,
                  FillProperties::PROP_FILL_HATCH_NAME,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    //bitmap properties see lcl_AddPropertiesToVector_only_BitmapProperties()

    rOutProperties.emplace_back( u"FillBackground"_ustr,
                  FillProperties::PROP_FILL_BACKGROUND,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
}

void lcl_AddPropertiesToVector_only_BitmapProperties(std::vector<beans::Property> & rOutProperties)
{
    rOutProperties.emplace_back( u"FillBitmapName"_ustr,
                  FillProperties::PROP_FILL_BITMAP_NAME,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( u"FillBitmapOffsetX"_ustr,
                  FillProperties::PROP_FILL_BITMAP_OFFSETX,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( u"FillBitmapOffsetY"_ustr,
                  FillProperties::PROP_FILL_BITMAP_OFFSETY,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( u"FillBitmapPositionOffsetX"_ustr,
                  FillProperties::PROP_FILL_BITMAP_POSITION_OFFSETX,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( u"FillBitmapPositionOffsetY"_ustr,
                  FillProperties::PROP_FILL_BITMAP_POSITION_OFFSETY,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( u"FillBitmapRectanglePoint"_ustr,
                  FillProperties::PROP_FILL_BITMAP_RECTANGLEPOINT,
                  cppu::UnoType<drawing::RectanglePoint>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( u"FillBitmapLogicalSize"_ustr,
                  FillProperties::PROP_FILL_BITMAP_LOGICALSIZE,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( u"FillBitmapSizeX"_ustr,
                  FillProperties::PROP_FILL_BITMAP_SIZEX,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( u"FillBitmapSizeY"_ustr,
                  FillProperties::PROP_FILL_BITMAP_SIZEY,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( u"FillBitmapMode"_ustr,
                  FillProperties::PROP_FILL_BITMAP_MODE,
                  cppu::UnoType<drawing::BitmapMode>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
}

void lcl_AddDefaultsToMap_without_BitmapProperties(
    ::chart::tPropertyValueMap & rOutMap )
{
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, FillProperties::PROP_FILL_STYLE, drawing::FillStyle_SOLID );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, FillProperties::PROP_FILL_COLOR, Color(0xd9, 0xd9, 0xd9) ); // gray85
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, FillProperties::PROP_FILL_TRANSPARENCE, sal_Int16(0) );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, FillProperties::PROP_FILL_BACKGROUND, false );
}

void lcl_AddDefaultsToMap_only_BitmapProperties(
    ::chart::tPropertyValueMap & rOutMap )
{
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, FillProperties::PROP_FILL_BITMAP_OFFSETX, sal_Int16(0) );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, FillProperties::PROP_FILL_BITMAP_OFFSETY, sal_Int16(0) );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, FillProperties::PROP_FILL_BITMAP_POSITION_OFFSETX, sal_Int16(0) );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, FillProperties::PROP_FILL_BITMAP_POSITION_OFFSETY, sal_Int16(0) );

    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, FillProperties::PROP_FILL_BITMAP_RECTANGLEPOINT, drawing::RectanglePoint_MIDDLE_MIDDLE );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, FillProperties::PROP_FILL_BITMAP_LOGICALSIZE, true );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, FillProperties::PROP_FILL_BITMAP_SIZEX, sal_Int32(0) );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, FillProperties::PROP_FILL_BITMAP_SIZEY, sal_Int32(0) );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, FillProperties::PROP_FILL_BITMAP_MODE, drawing::BitmapMode_REPEAT );
}

}//end anonymous namespace

void FillProperties::AddPropertiesToVector(
    std::vector<beans::Property> & rOutProperties )
{
    // Fill Properties see service drawing::FillProperties
    lcl_AddPropertiesToVector_without_BitmapProperties( rOutProperties );
    lcl_AddPropertiesToVector_only_BitmapProperties( rOutProperties );
}

void FillProperties::AddDefaultsToMap(
    ::chart::tPropertyValueMap & rOutMap )
{
    lcl_AddDefaultsToMap_without_BitmapProperties( rOutMap );
    lcl_AddDefaultsToMap_only_BitmapProperties( rOutMap );
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
