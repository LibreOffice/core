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

#ifndef CHART2_THREEDHELPER_HXX
#define CHART2_THREEDHELPER_HXX

#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/drawing/CameraGeometry.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>
#include "charttoolsdllapi.hxx"

namespace chart
{

/**
*/

enum ThreeDLookScheme
{
    ThreeDLookScheme_Simple,
    ThreeDLookScheme_Realistic,
    ThreeDLookScheme_Unknown
};

enum CuboidPlanePosition
{
    CuboidPlanePosition_Left,
    CuboidPlanePosition_Right,
    CuboidPlanePosition_Top,
    CuboidPlanePosition_Bottom,
    CuboidPlanePosition_Front,
    CuboidPlanePosition_Back
};

class OOO_DLLPUBLIC_CHARTTOOLS ThreeDHelper
{
public:

    /** Returns the default camera geometry that is set in the Diagram CTOR.
        This is not the property default!

        @todo deprecate the hard set camera geometry and use the property
              default
     */
    static ::com::sun::star::drawing::CameraGeometry getDefaultCameraGeometry( bool bPie=false );

    static void getRotationAngleFromDiagram(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet >& xSceneProperties
            , double& rfXAngleRad, double& rfYAngleRad, double& rfZAngleRad );
    static void setRotationAngleToDiagram(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet >& xSceneProperties
            , double fXAngleRad, double fYAngleRad, double fZAngleRad );

    static void getRotationFromDiagram(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet >& xSceneProperties
            , sal_Int32& rnHorizontalAngleDegree, sal_Int32& rnVerticalAngleDegree );
    static void setRotationToDiagram(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet >& xSceneProperties
            , sal_Int32 nHorizontalAngleDegree, sal_Int32 nVerticalYAngleDegree );

    static void switchRightAngledAxes( const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet >& xSceneProperties
            , sal_Bool bRightAngledAxes, bool bRotateLights=true );

    static void adaptRadAnglesForRightAngledAxes( double& rfXAngleRad, double& rfYAngleRad );
    static double getXDegreeAngleLimitForRightAngledAxes();
    static double getYDegreeAngleLimitForRightAngledAxes();
    static double getValueClippedToRange( double fValue, const double& fPositivLimit );

    static void convertElevationRotationDegToXYZAngleRad(
        sal_Int32 nElevationDeg, sal_Int32 nRotationDeg
        , double& rfXAngleRad, double& rfYAngleRad, double& rfZAngleRad );

    SAL_DLLPRIVATE static void convertXYZAngleRadToElevationRotationDeg(
        sal_Int32& rnElevationDeg, sal_Int32& rnRotationDeg
        , double fXRad, double fYRad, double fZRad );

    static double getCameraDistance(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet >& xSceneProperties );
    static void setCameraDistance(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet >& xSceneProperties
            , double fCameraDistance );
    SAL_DLLPRIVATE static void ensureCameraDistanceRange( double& rfCameraDistance );
    SAL_DLLPRIVATE static void getCameraDistanceRange( double& rfMinimumDistance, double& rfMaximumDistance );

    static double CameraDistanceToPerspective( double fCameraDistance );
    static double PerspectiveToCameraDistance( double fPerspective );

    static void set3DSettingsToDefault( const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet >& xSceneProperties );
    static void setDefaultRotation( const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet >& xSceneProperties );
    static void setDefaultIllumination( const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet >& xSceneProperties );

    static void setDefaultRotation( const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet >& xSceneProperties, bool bPieOrDonut );

    static CuboidPlanePosition getAutomaticCuboidPlanePositionForStandardLeftWall( const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet >& xSceneProperties );
    static CuboidPlanePosition getAutomaticCuboidPlanePositionForStandardBackWall(const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet >& xSceneProperties );
    static CuboidPlanePosition getAutomaticCuboidPlanePositionForStandardBottom(const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet >& xSceneProperties );

    static ThreeDLookScheme detectScheme( const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDiagram >& xDiagram );
    static void setScheme( const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDiagram >& xDiagram
            , ThreeDLookScheme aScheme );

    //sal_Int32 nRoundedEdges:  <0 or >100 -> mixed state
    //sal_Int32 nObjectLines:  0->no lines; 1->all lines on; other->mixed state

    static void getRoundedEdgesAndObjectLines( const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDiagram >& xDiagram
            , sal_Int32& rnRoundedEdges, sal_Int32& rnObjectLines );
    static void setRoundedEdgesAndObjectLines( const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDiagram >& xDiagram
            , sal_Int32 nRoundedEdges, sal_Int32 nObjectLines );
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
