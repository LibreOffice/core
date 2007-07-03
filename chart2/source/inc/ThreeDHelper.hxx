/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ThreeDHelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-03 13:42:01 $
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

#ifndef CHART2_THREEDHELPER_HXX
#define CHART2_THREEDHELPER_HXX

#ifndef _COM_SUN_STAR_CHART2_XDIAGRAM_HPP_
#include <com/sun/star/chart2/XDiagram.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_CAMERAGEOMETRY_HPP_
#include <com/sun/star/drawing/CameraGeometry.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_SHADEMODE_HPP_
#include <com/sun/star/drawing/ShadeMode.hpp>
#endif

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
