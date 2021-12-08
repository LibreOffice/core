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

#include <SceneProperties.hxx>
#include <ChartTypeHelper.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>
#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <com/sun/star/drawing/CameraGeometry.hpp>

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;

namespace chart
{

void SceneProperties::AddPropertiesToVector(
    std::vector< Property > & rOutProperties )
{
    // transformation matrix
    rOutProperties.emplace_back( "D3DTransformMatrix",
                  PROP_SCENE_TRANSF_MATRIX,
                  cppu::UnoType<drawing::HomogenMatrix>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    // distance: deprecated ( this is not used by the chart view; it's only here for compatibility with old chart  )
    rOutProperties.emplace_back( "D3DSceneDistance",
                  PROP_SCENE_DISTANCE,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    // focalLength: deprecated ( this is not used by the chart view; it's only here for compatibility with old chart  )
    rOutProperties.emplace_back( "D3DSceneFocalLength",
                  PROP_SCENE_FOCAL_LENGTH,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    // shadowSlant
    rOutProperties.emplace_back( "D3DSceneShadowSlant",
                  PROP_SCENE_SHADOW_SLANT,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    // shadeMode
    rOutProperties.emplace_back( "D3DSceneShadeMode",
                  PROP_SCENE_SHADE_MODE,
                  cppu::UnoType<drawing::ShadeMode>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    // ambientColor
    rOutProperties.emplace_back( "D3DSceneAmbientColor",
                  PROP_SCENE_AMBIENT_COLOR,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    // lightingMode
    rOutProperties.emplace_back( "D3DSceneTwoSidedLighting",
                  PROP_SCENE_TWO_SIDED_LIGHTING,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    // camera geometry
    rOutProperties.emplace_back( "D3DCameraGeometry",
                  PROP_SCENE_CAMERA_GEOMETRY,
                  cppu::UnoType<drawing::CameraGeometry>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    // perspective
    rOutProperties.emplace_back( "D3DScenePerspective",
                  PROP_SCENE_PERSPECTIVE,
                  cppu::UnoType<drawing::ProjectionMode>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    // Light Sources
    // light source 1
    rOutProperties.emplace_back( "D3DSceneLightColor1",
                  PROP_SCENE_LIGHT_COLOR_1,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "D3DSceneLightDirection1",
                  PROP_SCENE_LIGHT_DIRECTION_1,
                  cppu::UnoType<drawing::Direction3D>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "D3DSceneLightOn1",
                  PROP_SCENE_LIGHT_ON_1,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    // light source 2
    rOutProperties.emplace_back( "D3DSceneLightColor2",
                  PROP_SCENE_LIGHT_COLOR_2,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "D3DSceneLightDirection2",
                  PROP_SCENE_LIGHT_DIRECTION_2,
                  cppu::UnoType<drawing::Direction3D>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "D3DSceneLightOn2",
                  PROP_SCENE_LIGHT_ON_2,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    // light source 3
    rOutProperties.emplace_back( "D3DSceneLightColor3",
                  PROP_SCENE_LIGHT_COLOR_3,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "D3DSceneLightDirection3",
                  PROP_SCENE_LIGHT_DIRECTION_3,
                  cppu::UnoType<drawing::Direction3D>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "D3DSceneLightOn3",
                  PROP_SCENE_LIGHT_ON_3,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    // light source 4
    rOutProperties.emplace_back( "D3DSceneLightColor4",
                  PROP_SCENE_LIGHT_COLOR_4,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "D3DSceneLightDirection4",
                  PROP_SCENE_LIGHT_DIRECTION_4,
                  cppu::UnoType<drawing::Direction3D>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "D3DSceneLightOn4",
                  PROP_SCENE_LIGHT_ON_4,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    // light source 5
    rOutProperties.emplace_back( "D3DSceneLightColor5",
                  PROP_SCENE_LIGHT_COLOR_5,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "D3DSceneLightDirection5",
                  PROP_SCENE_LIGHT_DIRECTION_5,
                  cppu::UnoType<drawing::Direction3D>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "D3DSceneLightOn5",
                  PROP_SCENE_LIGHT_ON_5,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    // light source 6
    rOutProperties.emplace_back( "D3DSceneLightColor6",
                  PROP_SCENE_LIGHT_COLOR_6,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "D3DSceneLightDirection6",
                  PROP_SCENE_LIGHT_DIRECTION_6,
                  cppu::UnoType<drawing::Direction3D>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "D3DSceneLightOn6",
                  PROP_SCENE_LIGHT_ON_6,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    // light source 7
    rOutProperties.emplace_back( "D3DSceneLightColor7",
                  PROP_SCENE_LIGHT_COLOR_7,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "D3DSceneLightDirection7",
                  PROP_SCENE_LIGHT_DIRECTION_7,
                  cppu::UnoType<drawing::Direction3D>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "D3DSceneLightOn7",
                  PROP_SCENE_LIGHT_ON_7,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    // light source 8
    rOutProperties.emplace_back( "D3DSceneLightColor8",
                  PROP_SCENE_LIGHT_COLOR_8,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "D3DSceneLightDirection8",
                  PROP_SCENE_LIGHT_DIRECTION_8,
                  cppu::UnoType<drawing::Direction3D>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );
    rOutProperties.emplace_back( "D3DSceneLightOn8",
                  PROP_SCENE_LIGHT_ON_8,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT );
}

void SceneProperties::AddDefaultsToMap(
    ::chart::tPropertyValueMap & rOutMap )
{
    // Identity Matrix
    drawing::HomogenMatrix aMtx;
    aMtx.Line1.Column1 = aMtx.Line2.Column2 =
        aMtx.Line3.Column3 = aMtx.Line4.Column4 = 1.0;
    aMtx.Line1.Column2 = aMtx.Line1.Column3 = aMtx.Line1.Column4 =
        aMtx.Line2.Column1 = aMtx.Line2.Column3 = aMtx.Line2.Column4 =
        aMtx.Line3.Column1 = aMtx.Line3.Column2 = aMtx.Line3.Column4 =
        aMtx.Line4.Column1 = aMtx.Line4.Column2 = aMtx.Line4.Column3 = 0.0;

    rOutMap.setPropertyValueDefault( PROP_SCENE_TRANSF_MATRIX, aMtx );
    rOutMap.setPropertyValueDefault< sal_Int32 >( PROP_SCENE_DISTANCE, 4200 );
    rOutMap.setPropertyValueDefault< sal_Int32 >( PROP_SCENE_FOCAL_LENGTH, 8000 );

//     PROP_SCENE_SHADOW_SLANT;
    rOutMap.setPropertyValueDefault( PROP_SCENE_SHADE_MODE, drawing::ShadeMode_SMOOTH );

    rOutMap.setPropertyValueDefault< sal_Int32 >(
        PROP_SCENE_AMBIENT_COLOR, ChartTypeHelper::getDefaultAmbientLightColor(false,nullptr));

    rOutMap.setPropertyValueDefault( PROP_SCENE_TWO_SIDED_LIGHTING, true );

    drawing::Position3D vrp( 0.0, 0.0, 1.0 );
    drawing::Direction3D vpn( 0.0, 0.0, 1.0 );
    drawing::Direction3D vup( 0.0, 1.0, 0.0 );
    drawing::CameraGeometry aDefaultCameraGeometry( vrp, vpn, vup );

    rOutMap.setPropertyValueDefault( PROP_SCENE_CAMERA_GEOMETRY, aDefaultCameraGeometry );
    rOutMap.setPropertyValueDefault( PROP_SCENE_PERSPECTIVE, drawing::ProjectionMode_PERSPECTIVE );

    // Light Sources
    rOutMap.setPropertyValueDefault( PROP_SCENE_LIGHT_ON_1, false );
    rOutMap.setPropertyValueDefault( PROP_SCENE_LIGHT_ON_2, true );
    rOutMap.setPropertyValueDefault( PROP_SCENE_LIGHT_ON_3, false );
    rOutMap.setPropertyValueDefault( PROP_SCENE_LIGHT_ON_4, false );
    rOutMap.setPropertyValueDefault( PROP_SCENE_LIGHT_ON_5, false );
    rOutMap.setPropertyValueDefault( PROP_SCENE_LIGHT_ON_6, false );
    rOutMap.setPropertyValueDefault( PROP_SCENE_LIGHT_ON_7, false );
    rOutMap.setPropertyValueDefault( PROP_SCENE_LIGHT_ON_8, false );

    uno::Any aDefaultLightDirection( drawing::Direction3D( 0.0, 0.0, 1.0 ) );
    rOutMap.setPropertyValueDefault( PROP_SCENE_LIGHT_DIRECTION_1, aDefaultLightDirection );
    rOutMap.setPropertyValueDefault( PROP_SCENE_LIGHT_DIRECTION_2, ChartTypeHelper::getDefaultSimpleLightDirection(nullptr));
    rOutMap.setPropertyValueDefault( PROP_SCENE_LIGHT_DIRECTION_3, aDefaultLightDirection );
    rOutMap.setPropertyValueDefault( PROP_SCENE_LIGHT_DIRECTION_4, aDefaultLightDirection );
    rOutMap.setPropertyValueDefault( PROP_SCENE_LIGHT_DIRECTION_5, aDefaultLightDirection );
    rOutMap.setPropertyValueDefault( PROP_SCENE_LIGHT_DIRECTION_6, aDefaultLightDirection );
    rOutMap.setPropertyValueDefault( PROP_SCENE_LIGHT_DIRECTION_7, aDefaultLightDirection );
    rOutMap.setPropertyValueDefault( PROP_SCENE_LIGHT_DIRECTION_8, aDefaultLightDirection );

    uno::Any aDefaultLightColor;
    aDefaultLightColor <<= ChartTypeHelper::getDefaultDirectLightColor(false,nullptr);
    rOutMap.setPropertyValueDefault( PROP_SCENE_LIGHT_COLOR_1, aDefaultLightColor );
    rOutMap.setPropertyValueDefault( PROP_SCENE_LIGHT_COLOR_2, aDefaultLightColor );
    rOutMap.setPropertyValueDefault( PROP_SCENE_LIGHT_COLOR_3, aDefaultLightColor );
    rOutMap.setPropertyValueDefault( PROP_SCENE_LIGHT_COLOR_4, aDefaultLightColor );
    rOutMap.setPropertyValueDefault( PROP_SCENE_LIGHT_COLOR_5, aDefaultLightColor );
    rOutMap.setPropertyValueDefault( PROP_SCENE_LIGHT_COLOR_6, aDefaultLightColor );
    rOutMap.setPropertyValueDefault( PROP_SCENE_LIGHT_COLOR_7, aDefaultLightColor );
    rOutMap.setPropertyValueDefault( PROP_SCENE_LIGHT_COLOR_8, aDefaultLightColor );
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
