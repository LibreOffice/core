/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ThreeDHelper.hxx,v $
 * $Revision: 1.3 $
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

#ifndef CHART2_THREEDHELPER_HXX
#define CHART2_THREEDHELPER_HXX

#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/drawing/CameraGeometry.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

enum ThreeDLookScheme
{
    ThreeDLookScheme_Simple,
    ThreeDLookScheme_Realistic,
    ThreeDLookScheme_Unknown
};

class ThreeDHelper
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

    static void switchRightAngledAxes( const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet >& xSceneProperties
            , sal_Bool bRightAngledAxes, bool bRotateLights=true );

    static void adaptRadAnglesForRightAngledAxes( double& rfXAngleRad, double& rfYAngleRad );
    static double getXDegreeAngleLimitForRightAngledAxes();
    static double getYDegreeAngleLimitForRightAngledAxes();
    static double getValueClippedToRange( double fValue, const double& fPositivLimit );

    static double getCameraDistance(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet >& xSceneProperties );
    static void setCameraDistance(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet >& xSceneProperties
            , double fCameraDistance );
    static void ensureCameraDistanceRange( double& rfCameraDistance );
    static void getCameraDistanceRange( double& rfMinimumDistance, double& rfMaximumDistance );

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

//.............................................................................
} //namespace chart
//.............................................................................
#endif
