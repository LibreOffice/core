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

#include "FillProperties.hxx"
#include "macros.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/RectanglePoint.hpp>

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;

namespace chart
{

namespace
{

void lcl_AddPropertiesToVector_without_BitmapProperties( ::std::vector< css::beans::Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( "FillStyle",
                  FillProperties::PROP_FILL_STYLE,
                  cppu::UnoType<drawing::FillStyle>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "FillColor",
                  FillProperties::PROP_FILL_COLOR,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID         // "maybe auto"
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "FillTransparence",
                  FillProperties::PROP_FILL_TRANSPARENCE,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "FillTransparenceGradientName",
                  FillProperties::PROP_FILL_TRANSPARENCE_GRADIENT_NAME,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "FillGradientName",
                  FillProperties::PROP_FILL_GRADIENT_NAME,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        beans::Property( "FillGradientStepCount",
                  FillProperties::PROP_FILL_GRADIENT_STEPCOUNT,
                  cppu::UnoType<sal_Int16>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( "FillHatchName",
                  FillProperties::PROP_FILL_HATCH_NAME,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    //bitmap properties see lcl_AddPropertiesToVector_only_BitmapProperties()

    rOutProperties.push_back(
        Property( "FillBackground",
                  FillProperties::PROP_FILL_BACKGROUND,
                  cppu::UnoType<sal_Bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

void lcl_AddPropertiesToVector_only_BitmapProperties( ::std::vector< css::beans::Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( "FillBitmapName",
                  FillProperties::PROP_FILL_BITMAP_NAME,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

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
                  cppu::UnoType<sal_Bool>::get(),
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
}

void lcl_AddDefaultsToMap_without_BitmapProperties(
    ::chart::tPropertyValueMap & rOutMap )
{
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, FillProperties::PROP_FILL_STYLE, drawing::FillStyle_SOLID );
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, FillProperties::PROP_FILL_COLOR, 0xd9d9d9 ); // gray85
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int16 >( rOutMap, FillProperties::PROP_FILL_TRANSPARENCE, 0 );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, FillProperties::PROP_FILL_BACKGROUND, false );
}

void lcl_AddDefaultsToMap_only_BitmapProperties(
    ::chart::tPropertyValueMap & rOutMap )
{
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int16 >( rOutMap, FillProperties::PROP_FILL_BITMAP_OFFSETX, 0 );
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int16 >( rOutMap, FillProperties::PROP_FILL_BITMAP_OFFSETY, 0 );
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int16 >( rOutMap, FillProperties::PROP_FILL_BITMAP_POSITION_OFFSETX, 0 );
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int16 >( rOutMap, FillProperties::PROP_FILL_BITMAP_POSITION_OFFSETY, 0 );

    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, FillProperties::PROP_FILL_BITMAP_RECTANGLEPOINT, drawing::RectanglePoint_MIDDLE_MIDDLE );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, FillProperties::PROP_FILL_BITMAP_LOGICALSIZE, true );
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, FillProperties::PROP_FILL_BITMAP_SIZEX, 0 );
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, FillProperties::PROP_FILL_BITMAP_SIZEY, 0 );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, FillProperties::PROP_FILL_BITMAP_MODE, drawing::BitmapMode_REPEAT );
}

}//end anonymous namespace

void FillProperties::AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
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
