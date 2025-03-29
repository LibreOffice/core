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
#include <com/sun/star/drawing/CameraGeometry.hpp>
#include <rtl/ref.hxx>

namespace chart
{
class Diagram;

enum class ThreeDLookScheme
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

class ThreeDHelper
{
public:

    /** Returns the default camera geometry that is set in the Diagram CTOR.
        This is not the property default!

        @todo deprecate the hard set camera geometry and use the property
              default
     */
    static css::drawing::CameraGeometry getDefaultCameraGeometry( bool bPie=false );

    static void adaptRadAnglesForRightAngledAxes( double& rfXAngleRad, double& rfYAngleRad );
    static double getXDegreeAngleLimitForRightAngledAxes() { return 90.0; }
    static double getYDegreeAngleLimitForRightAngledAxes() { return 45.0; }

    static double getValueClippedToRange( double fValue, const double& fPositivLimit );

    static void convertElevationRotationDegToXYZAngleRad(
        sal_Int32 nElevationDeg, sal_Int32 nRotationDeg
        , double& rfXAngleRad, double& rfYAngleRad, double& rfZAngleRad );

    static void convertXYZAngleRadToElevationRotationDeg(
        sal_Int32& rnElevationDeg, sal_Int32& rnRotationDeg
        , double fXRad, double fYRad, double fZRad );

    static void ensureCameraDistanceRange( double& rfCameraDistance );
    static void getCameraDistanceRange( double& rfMinimumDistance, double& rfMaximumDistance );

    static double CameraDistanceToPerspective( double fCameraDistance );
    static double PerspectiveToCameraDistance( double fPerspective );

    static CuboidPlanePosition getAutomaticCuboidPlanePositionForStandardLeftWall( const rtl::Reference<
            ::chart::Diagram >& xDiagram );
    static CuboidPlanePosition getAutomaticCuboidPlanePositionForStandardBackWall(const rtl::Reference<
            ::chart::Diagram >& xDiagram );
    static CuboidPlanePosition getAutomaticCuboidPlanePositionForStandardBottom(const rtl::Reference<
            ::chart::Diagram >& xDiagram );

    //sal_Int32 nRoundedEdges:  <0 or >100 -> mixed state
    //sal_Int32 nObjectLines:  0->no lines; 1->all lines on; other->mixed state

    static void getRoundedEdgesAndObjectLines( const rtl::Reference< ::chart::Diagram >& xDiagram
            , sal_Int32& rnRoundedEdges, sal_Int32& rnObjectLines );
    static void setRoundedEdgesAndObjectLines( const rtl::Reference< ::chart::Diagram >& xDiagram
            , sal_Int32 nRoundedEdges, sal_Int32 nObjectLines );
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
