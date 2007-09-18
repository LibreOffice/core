/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SceneProperties.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-18 15:10:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "SceneProperties.hxx"
#include "macros.hxx"
#include "ChartTypeHelper.hxx"
#include "ThreeDHelper.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_HOMOGENMATRIX_HPP_
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_SHADEMODE_HPP_
#include <com/sun/star/drawing/ShadeMode.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_DIRECTION3D_HPP_
#include <com/sun/star/drawing/Direction3D.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_PROJECTIONMODE_HPP_
#include <com/sun/star/drawing/ProjectionMode.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_CAMERAGEOMETRY_HPP_
#include <com/sun/star/drawing/CameraGeometry.hpp>
#endif

// for F_PI
#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;

namespace chart
{

void SceneProperties::AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    // transformation matrix
    rOutProperties.push_back(
        Property( C2U( "D3DTransformMatrix" ),
                  PROP_SCENE_TRANSF_MATRIX,
                  ::getCppuType( reinterpret_cast< const drawing::HomogenMatrix * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    // distance: deprecated ( this is not used by the chart view; it's only here for compatibility with old chart  )
    rOutProperties.push_back(
        Property( C2U( "D3DSceneDistance" ),
                  PROP_SCENE_DISTANCE,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    // focalLength: deprecated ( this is not used by the chart view; it's only here for compatibility with old chart  )
    rOutProperties.push_back(
        Property( C2U( "D3DSceneFocalLength" ),
                  PROP_SCENE_FOCAL_LENGTH,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    // shadowSlant
    rOutProperties.push_back(
        Property( C2U( "D3DSceneShadowSlant" ),
                  PROP_SCENE_SHADOW_SLANT,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    // shadeMode
    rOutProperties.push_back(
        Property( C2U( "D3DSceneShadeMode" ),
                  PROP_SCENE_SHADE_MODE,
                  ::getCppuType( reinterpret_cast< const drawing::ShadeMode * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    // ambientColor
    rOutProperties.push_back(
        Property( C2U( "D3DSceneAmbientColor" ),
                  PROP_SCENE_AMBIENT_COLOR,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    // lightingMode
    rOutProperties.push_back(
        Property( C2U( "D3DSceneTwoSidedLighting" ),
                  PROP_SCENE_TWO_SIDED_LIGHTING,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    // camera geometry
    rOutProperties.push_back(
        Property( C2U( "D3DCameraGeometry" ),
                  PROP_SCENE_CAMERA_GEOMETRY,
                  ::getCppuType( reinterpret_cast< const drawing::CameraGeometry * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    // perspective
    rOutProperties.push_back(
        Property( C2U( "D3DScenePerspective" ),
                  PROP_SCENE_PERSPECTIVE,
                  ::getCppuType( reinterpret_cast< const drawing::ProjectionMode * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));


    // Light Sources
    // -------------
    // light source 1
    rOutProperties.push_back(
        Property( C2U( "D3DSceneLightColor1" ),
                  PROP_SCENE_LIGHT_COLOR_1,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "D3DSceneLightDirection1" ),
                  PROP_SCENE_LIGHT_DIRECTION_1,
                  ::getCppuType( reinterpret_cast< const drawing::Direction3D * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "D3DSceneLightOn1" ),
                  PROP_SCENE_LIGHT_ON_1,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    // light source 2
    rOutProperties.push_back(
        Property( C2U( "D3DSceneLightColor2" ),
                  PROP_SCENE_LIGHT_COLOR_2,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "D3DSceneLightDirection2" ),
                  PROP_SCENE_LIGHT_DIRECTION_2,
                  ::getCppuType( reinterpret_cast< const drawing::Direction3D * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "D3DSceneLightOn2" ),
                  PROP_SCENE_LIGHT_ON_2,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    // light source 3
    rOutProperties.push_back(
        Property( C2U( "D3DSceneLightColor3" ),
                  PROP_SCENE_LIGHT_COLOR_3,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "D3DSceneLightDirection3" ),
                  PROP_SCENE_LIGHT_DIRECTION_3,
                  ::getCppuType( reinterpret_cast< const drawing::Direction3D * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "D3DSceneLightOn3" ),
                  PROP_SCENE_LIGHT_ON_3,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    // light source 4
    rOutProperties.push_back(
        Property( C2U( "D3DSceneLightColor4" ),
                  PROP_SCENE_LIGHT_COLOR_4,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "D3DSceneLightDirection4" ),
                  PROP_SCENE_LIGHT_DIRECTION_4,
                  ::getCppuType( reinterpret_cast< const drawing::Direction3D * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "D3DSceneLightOn4" ),
                  PROP_SCENE_LIGHT_ON_4,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    // light source 5
    rOutProperties.push_back(
        Property( C2U( "D3DSceneLightColor5" ),
                  PROP_SCENE_LIGHT_COLOR_5,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "D3DSceneLightDirection5" ),
                  PROP_SCENE_LIGHT_DIRECTION_5,
                  ::getCppuType( reinterpret_cast< const drawing::Direction3D * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "D3DSceneLightOn5" ),
                  PROP_SCENE_LIGHT_ON_5,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    // light source 6
    rOutProperties.push_back(
        Property( C2U( "D3DSceneLightColor6" ),
                  PROP_SCENE_LIGHT_COLOR_6,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "D3DSceneLightDirection6" ),
                  PROP_SCENE_LIGHT_DIRECTION_6,
                  ::getCppuType( reinterpret_cast< const drawing::Direction3D * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "D3DSceneLightOn6" ),
                  PROP_SCENE_LIGHT_ON_6,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    // light source 7
    rOutProperties.push_back(
        Property( C2U( "D3DSceneLightColor7" ),
                  PROP_SCENE_LIGHT_COLOR_7,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "D3DSceneLightDirection7" ),
                  PROP_SCENE_LIGHT_DIRECTION_7,
                  ::getCppuType( reinterpret_cast< const drawing::Direction3D * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "D3DSceneLightOn7" ),
                  PROP_SCENE_LIGHT_ON_7,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    // light source 8
    rOutProperties.push_back(
        Property( C2U( "D3DSceneLightColor8" ),
                  PROP_SCENE_LIGHT_COLOR_8,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "D3DSceneLightDirection8" ),
                  PROP_SCENE_LIGHT_DIRECTION_8,
                  ::getCppuType( reinterpret_cast< const drawing::Direction3D * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "D3DSceneLightOn8" ),
                  PROP_SCENE_LIGHT_ON_8,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
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

    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_SCENE_TRANSF_MATRIX, aMtx );
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, PROP_SCENE_DISTANCE, 4200 );
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, PROP_SCENE_FOCAL_LENGTH, 8000 );

//     PROP_SCENE_SHADOW_SLANT;
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_SCENE_SHADE_MODE, drawing::ShadeMode_SMOOTH );

    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >(
        rOutMap, PROP_SCENE_AMBIENT_COLOR, ChartTypeHelper::getDefaultAmbientLightColor(ThreeDLookScheme_Simple,0));

    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_SCENE_TWO_SIDED_LIGHTING, true );

    drawing::Position3D vrp( 0.0, 0.0, 1.0 );
    drawing::Direction3D vpn( 0.0, 0.0, 1.0 );
    drawing::Direction3D vup( 0.0, 1.0, 0.0 );
    drawing::CameraGeometry aDefaultCameraGeometry( vrp, vpn, vup );

    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_SCENE_CAMERA_GEOMETRY, aDefaultCameraGeometry );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_SCENE_PERSPECTIVE, drawing::ProjectionMode_PERSPECTIVE );

    // Light Sources
    // -------------
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_SCENE_LIGHT_ON_1, false );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_SCENE_LIGHT_ON_2, true );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_SCENE_LIGHT_ON_3, false );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_SCENE_LIGHT_ON_4, false );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_SCENE_LIGHT_ON_5, false );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_SCENE_LIGHT_ON_6, false );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_SCENE_LIGHT_ON_7, false );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_SCENE_LIGHT_ON_8, false );

    uno::Any aDefaultLightDirection( uno::makeAny( drawing::Direction3D( 0.0, 0.0, 1.0 ) ) );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_SCENE_LIGHT_DIRECTION_1, aDefaultLightDirection );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_SCENE_LIGHT_DIRECTION_2, ChartTypeHelper::getDefaultSimpleLightDirection(0));
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_SCENE_LIGHT_DIRECTION_3, aDefaultLightDirection );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_SCENE_LIGHT_DIRECTION_4, aDefaultLightDirection );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_SCENE_LIGHT_DIRECTION_5, aDefaultLightDirection );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_SCENE_LIGHT_DIRECTION_6, aDefaultLightDirection );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_SCENE_LIGHT_DIRECTION_7, aDefaultLightDirection );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_SCENE_LIGHT_DIRECTION_8, aDefaultLightDirection );

    uno::Any aDefaultLightColor;
    aDefaultLightColor <<= ChartTypeHelper::getDefaultDirectLightColor(ThreeDLookScheme_Simple,0);
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_SCENE_LIGHT_COLOR_1, aDefaultLightColor );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_SCENE_LIGHT_COLOR_2, aDefaultLightColor );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_SCENE_LIGHT_COLOR_3, aDefaultLightColor );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_SCENE_LIGHT_COLOR_4, aDefaultLightColor );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_SCENE_LIGHT_COLOR_5, aDefaultLightColor );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_SCENE_LIGHT_COLOR_6, aDefaultLightColor );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_SCENE_LIGHT_COLOR_7, aDefaultLightColor );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_SCENE_LIGHT_COLOR_8, aDefaultLightColor );
}

} //  namespace chart
