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

#include <ThreeDHelper.hxx>
#include <DiagramHelper.hxx>
#include <ChartTypeHelper.hxx>
#include <BaseGFXHelper.hxx>
#include <DataSeriesHelper.hxx>
#include <defines.hxx>

#include <editeng/unoprnms.hxx>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>
#include <tools/diagnose_ex.h>
#include <tools/helpers.hxx>
#include <rtl/math.hxx>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::uno::Reference;
using ::rtl::math::cos;
using ::rtl::math::sin;
using ::rtl::math::tan;

namespace
{

bool lcl_isRightAngledAxesSetAndSupported( const Reference< beans::XPropertySet >& xSceneProperties )
{
    if( xSceneProperties.is() )
    {
        bool bRightAngledAxes = false;
        xSceneProperties->getPropertyValue( "RightAngledAxes") >>= bRightAngledAxes;
        if(bRightAngledAxes)
        {
            uno::Reference< chart2::XDiagram > xDiagram( xSceneProperties, uno::UNO_QUERY );
            if( ChartTypeHelper::isSupportingRightAngledAxes(
                    DiagramHelper::getChartTypeByIndex( xDiagram, 0 ) ) )
            {
                return true;
            }
        }
    }
    return false;
}

void lcl_RotateLightSource( const Reference< beans::XPropertySet >& xSceneProperties
                           , const OUString& rLightSourceDirection
                           , const OUString& rLightSourceOn
                           , const ::basegfx::B3DHomMatrix& rRotationMatrix )
{
    if( !xSceneProperties.is() )
        return;

    bool bLightOn = false;
    if( !(xSceneProperties->getPropertyValue( rLightSourceOn ) >>= bLightOn) )
        return;

    if( bLightOn )
    {
        drawing::Direction3D aLight;
        if( xSceneProperties->getPropertyValue( rLightSourceDirection ) >>= aLight )
        {
            ::basegfx::B3DVector aLightVector( BaseGFXHelper::Direction3DToB3DVector( aLight ) );
            aLightVector = rRotationMatrix*aLightVector;

            xSceneProperties->setPropertyValue( rLightSourceDirection
                , uno::Any( BaseGFXHelper::B3DVectorToDirection3D( aLightVector ) ) );
        }
    }
}

void lcl_rotateLights( const ::basegfx::B3DHomMatrix& rLightRottion, const Reference< beans::XPropertySet >& xSceneProperties )
{
    if(!xSceneProperties.is())
        return;

    ::basegfx::B3DHomMatrix aLightRottion( rLightRottion );
    BaseGFXHelper::ReduceToRotationMatrix( aLightRottion );

    lcl_RotateLightSource( xSceneProperties, "D3DSceneLightDirection1", "D3DSceneLightOn1", aLightRottion );
    lcl_RotateLightSource( xSceneProperties, "D3DSceneLightDirection2", "D3DSceneLightOn2", aLightRottion );
    lcl_RotateLightSource( xSceneProperties, "D3DSceneLightDirection3", "D3DSceneLightOn3", aLightRottion );
    lcl_RotateLightSource( xSceneProperties, "D3DSceneLightDirection4", "D3DSceneLightOn4", aLightRottion );
    lcl_RotateLightSource( xSceneProperties, "D3DSceneLightDirection5", "D3DSceneLightOn5", aLightRottion );
    lcl_RotateLightSource( xSceneProperties, "D3DSceneLightDirection6", "D3DSceneLightOn6", aLightRottion );
    lcl_RotateLightSource( xSceneProperties, "D3DSceneLightDirection7", "D3DSceneLightOn7", aLightRottion );
    lcl_RotateLightSource( xSceneProperties, "D3DSceneLightDirection8", "D3DSceneLightOn8", aLightRottion );
}

::basegfx::B3DHomMatrix lcl_getInverseRotationMatrix( const Reference< beans::XPropertySet >& xSceneProperties )
{
    ::basegfx::B3DHomMatrix aInverseRotation;
    double fXAngleRad=0.0;
    double fYAngleRad=0.0;
    double fZAngleRad=0.0;
    ThreeDHelper::getRotationAngleFromDiagram(
        xSceneProperties, fXAngleRad, fYAngleRad, fZAngleRad );
    aInverseRotation.rotate( 0.0, 0.0, -fZAngleRad );
    aInverseRotation.rotate( 0.0, -fYAngleRad, 0.0 );
    aInverseRotation.rotate( -fXAngleRad, 0.0, 0.0 );
    return aInverseRotation;
}

::basegfx::B3DHomMatrix lcl_getCompleteRotationMatrix( const Reference< beans::XPropertySet >& xSceneProperties )
{
    ::basegfx::B3DHomMatrix aCompleteRotation;
    double fXAngleRad=0.0;
    double fYAngleRad=0.0;
    double fZAngleRad=0.0;
    ThreeDHelper::getRotationAngleFromDiagram(
        xSceneProperties, fXAngleRad, fYAngleRad, fZAngleRad );
    aCompleteRotation.rotate( fXAngleRad, fYAngleRad, fZAngleRad );
    return aCompleteRotation;
}

bool lcl_isEqual( const drawing::Direction3D& rA, const drawing::Direction3D& rB )
{
    return ::rtl::math::approxEqual(rA.DirectionX, rB.DirectionX)
        && ::rtl::math::approxEqual(rA.DirectionY, rB.DirectionY)
        && ::rtl::math::approxEqual(rA.DirectionZ, rB.DirectionZ);
}

bool lcl_isLightScheme( const uno::Reference< beans::XPropertySet >& xDiagramProps, bool bRealistic )
{
    if(!xDiagramProps.is())
        return false;

    bool bIsOn = false;
    xDiagramProps->getPropertyValue( UNO_NAME_3D_SCENE_LIGHTON_2 ) >>= bIsOn;
    if(!bIsOn)
        return false;

    uno::Reference< chart2::XDiagram > xDiagram( xDiagramProps, uno::UNO_QUERY );
    uno::Reference< chart2::XChartType > xChartType( DiagramHelper::getChartTypeByIndex( xDiagram, 0 ) );

    sal_Int32 nColor = 0;
    xDiagramProps->getPropertyValue( UNO_NAME_3D_SCENE_LIGHTCOLOR_2 ) >>= nColor;
    if( nColor != ::chart::ChartTypeHelper::getDefaultDirectLightColor( !bRealistic, xChartType ) )
        return false;

    sal_Int32 nAmbientColor = 0;
    xDiagramProps->getPropertyValue( UNO_NAME_3D_SCENE_AMBIENTCOLOR ) >>= nAmbientColor;
    if( nAmbientColor != ::chart::ChartTypeHelper::getDefaultAmbientLightColor( !bRealistic, xChartType ) )
        return false;

    drawing::Direction3D aDirection(0,0,0);
    xDiagramProps->getPropertyValue( UNO_NAME_3D_SCENE_LIGHTDIRECTION_2 ) >>= aDirection;

    drawing::Direction3D aDefaultDirection( bRealistic
        ? ChartTypeHelper::getDefaultRealisticLightDirection(xChartType)
        : ChartTypeHelper::getDefaultSimpleLightDirection(xChartType) );

    //rotate default light direction when right angled axes are off but supported
    {
        bool bRightAngledAxes = false;
        xDiagramProps->getPropertyValue( "RightAngledAxes") >>= bRightAngledAxes;
        if(!bRightAngledAxes)
        {
            if( ChartTypeHelper::isSupportingRightAngledAxes(
                    DiagramHelper::getChartTypeByIndex( xDiagram, 0 ) ) )
            {
                ::basegfx::B3DHomMatrix aRotation( lcl_getCompleteRotationMatrix( xDiagramProps ) );
                BaseGFXHelper::ReduceToRotationMatrix( aRotation );
                ::basegfx::B3DVector aLightVector( BaseGFXHelper::Direction3DToB3DVector( aDefaultDirection ) );
                aLightVector = aRotation*aLightVector;
                aDefaultDirection = BaseGFXHelper::B3DVectorToDirection3D( aLightVector );
            }
        }
    }

    return lcl_isEqual( aDirection, aDefaultDirection );
}

bool lcl_isRealisticLightScheme( const uno::Reference< beans::XPropertySet >& xDiagramProps )
{
    return lcl_isLightScheme( xDiagramProps, true /*bRealistic*/ );
}
bool lcl_isSimpleLightScheme( const uno::Reference< beans::XPropertySet >& xDiagramProps )
{
    return lcl_isLightScheme( xDiagramProps, false /*bRealistic*/ );
}
void lcl_setLightsForScheme( const uno::Reference< beans::XPropertySet >& xDiagramProps, const ThreeDLookScheme& rScheme )
{
    if(!xDiagramProps.is())
        return;
    if( rScheme == ThreeDLookScheme::ThreeDLookScheme_Unknown)
        return;

    xDiagramProps->setPropertyValue( UNO_NAME_3D_SCENE_LIGHTON_2, uno::Any( true ) );

    uno::Reference< chart2::XDiagram > xDiagram( xDiagramProps, uno::UNO_QUERY );
    uno::Reference< chart2::XChartType > xChartType( DiagramHelper::getChartTypeByIndex( xDiagram, 0 ) );
    uno::Any aADirection( rScheme == ThreeDLookScheme::ThreeDLookScheme_Simple
        ? ChartTypeHelper::getDefaultSimpleLightDirection(xChartType)
        : ChartTypeHelper::getDefaultRealisticLightDirection(xChartType) );

    xDiagramProps->setPropertyValue( UNO_NAME_3D_SCENE_LIGHTDIRECTION_2, aADirection );
    //rotate light direction when right angled axes are off but supported
    {
        bool bRightAngledAxes = false;
        xDiagramProps->getPropertyValue( "RightAngledAxes") >>= bRightAngledAxes;
        if(!bRightAngledAxes)
        {
            if( ChartTypeHelper::isSupportingRightAngledAxes( xChartType ) )
            {
                ::basegfx::B3DHomMatrix aRotation( lcl_getCompleteRotationMatrix( xDiagramProps ) );
                BaseGFXHelper::ReduceToRotationMatrix( aRotation );
                lcl_RotateLightSource( xDiagramProps, "D3DSceneLightDirection2", "D3DSceneLightOn2", aRotation );
            }
        }
    }

    sal_Int32 nColor = ::chart::ChartTypeHelper::getDefaultDirectLightColor(
        rScheme == ThreeDLookScheme::ThreeDLookScheme_Simple, xChartType);
    xDiagramProps->setPropertyValue( UNO_NAME_3D_SCENE_LIGHTCOLOR_2, uno::Any( nColor ) );

    sal_Int32 nAmbientColor = ::chart::ChartTypeHelper::getDefaultAmbientLightColor(
        rScheme == ThreeDLookScheme::ThreeDLookScheme_Simple, xChartType);
    xDiagramProps->setPropertyValue( UNO_NAME_3D_SCENE_AMBIENTCOLOR, uno::Any( nAmbientColor ) );
}

bool lcl_isRealisticScheme( drawing::ShadeMode aShadeMode
                    , sal_Int32 nRoundedEdges
                    , sal_Int32 nObjectLines )
{
    if(aShadeMode!=drawing::ShadeMode_SMOOTH)
        return false;
    if(nRoundedEdges!=5)
        return false;
    if(nObjectLines!=0)
        return false;
    return true;
}

bool lcl_isSimpleScheme( drawing::ShadeMode aShadeMode
                    , sal_Int32 nRoundedEdges
                    , sal_Int32 nObjectLines
                    , const uno::Reference< XDiagram >& xDiagram )
{
    if(aShadeMode!=drawing::ShadeMode_FLAT)
        return false;
    if(nRoundedEdges!=0)
        return false;
    if(nObjectLines==0)
    {
        uno::Reference< chart2::XChartType > xChartType( DiagramHelper::getChartTypeByIndex( xDiagram, 0 ) );
        return ChartTypeHelper::noBordersForSimpleScheme( xChartType );
    }
    if(nObjectLines!=1)
        return false;
    return true;
}

void lcl_setRealisticScheme( drawing::ShadeMode& rShadeMode
                    , sal_Int32& rnRoundedEdges
                    , sal_Int32& rnObjectLines )
{
    rShadeMode = drawing::ShadeMode_SMOOTH;
    rnRoundedEdges = 5;
    rnObjectLines = 0;
}

void lcl_setSimpleScheme( drawing::ShadeMode& rShadeMode
                    , sal_Int32& rnRoundedEdges
                    , sal_Int32& rnObjectLines
                    , const uno::Reference< XDiagram >& xDiagram )
{
    rShadeMode = drawing::ShadeMode_FLAT;
    rnRoundedEdges = 0;

    uno::Reference< chart2::XChartType > xChartType( DiagramHelper::getChartTypeByIndex( xDiagram, 0 ) );
    rnObjectLines = ChartTypeHelper::noBordersForSimpleScheme( xChartType ) ? 0 : 1;
}

} //end anonymous namespace

drawing::CameraGeometry ThreeDHelper::getDefaultCameraGeometry( bool bPie )
{
    // ViewReferencePoint (Point on the View plane)
    drawing::Position3D vrp(17634.6218373783, 10271.4823817647, 24594.8639082739);
    // ViewPlaneNormal (Normal to the View Plane)
    drawing::Direction3D vpn(0.416199821709347, 0.173649045905254, 0.892537795986984);
    // ViewUpVector (determines the v-axis direction on the view plane as
    // projection of VUP parallel to VPN onto th view pane)
    drawing::Direction3D vup(-0.0733876362771618, 0.984807599917971, -0.157379306090273);

    if( bPie )
    {
        vrp = drawing::Position3D( 0.0, 0.0, 87591.2408759124 );//--> 5 percent perspective
        vpn = drawing::Direction3D( 0.0, 0.0, 1.0 );
        vup = drawing::Direction3D( 0.0, 1.0, 0.0 );
    }

    return drawing::CameraGeometry( vrp, vpn, vup );
}

namespace
{
::basegfx::B3DHomMatrix lcl_getCameraMatrix( const uno::Reference< beans::XPropertySet >& xSceneProperties )
{
    drawing::HomogenMatrix aCameraMatrix;

    drawing::CameraGeometry aCG( ThreeDHelper::getDefaultCameraGeometry() );
    if( xSceneProperties.is() )
        xSceneProperties->getPropertyValue( "D3DCameraGeometry" ) >>= aCG;

    ::basegfx::B3DVector aVPN( BaseGFXHelper::Direction3DToB3DVector( aCG.vpn ) );
    ::basegfx::B3DVector aVUP( BaseGFXHelper::Direction3DToB3DVector( aCG.vup ) );

    //normalize vectors:
    aVPN.normalize();
    aVUP.normalize();

    ::basegfx::B3DVector aCross = ::basegfx::cross( aVUP, aVPN );

    //first line is VUP x VPN
    aCameraMatrix.Line1.Column1 = aCross[0];
    aCameraMatrix.Line1.Column2 = aCross[1];
    aCameraMatrix.Line1.Column3 = aCross[2];
    aCameraMatrix.Line1.Column4 = 0.0;

    //second line is VUP
    aCameraMatrix.Line2.Column1 = aVUP[0];
    aCameraMatrix.Line2.Column2 = aVUP[1];
    aCameraMatrix.Line2.Column3 = aVUP[2];
    aCameraMatrix.Line2.Column4 = 0.0;

    //third line is VPN
    aCameraMatrix.Line3.Column1 = aVPN[0];
    aCameraMatrix.Line3.Column2 = aVPN[1];
    aCameraMatrix.Line3.Column3 = aVPN[2];
    aCameraMatrix.Line3.Column4 = 0.0;

    //fourth line is 0 0 0 1
    aCameraMatrix.Line4.Column1 = 0.0;
    aCameraMatrix.Line4.Column2 = 0.0;
    aCameraMatrix.Line4.Column3 = 0.0;
    aCameraMatrix.Line4.Column4 = 1.0;

    return BaseGFXHelper::HomogenMatrixToB3DHomMatrix( aCameraMatrix );
}

double lcl_shiftAngleToIntervalMinusPiToPi( double fAngleRad )
{
    //valid range:  ]-Pi,Pi]
    while( fAngleRad<=-M_PI )
        fAngleRad+=(2*M_PI);
    while( fAngleRad>M_PI )
        fAngleRad-=(2*M_PI);
    return fAngleRad;
}

void lcl_ensureIntervalMinus1To1( double& rSinOrCos )
{
    if (rSinOrCos < -1.0)
       rSinOrCos = -1.0;
    else if (rSinOrCos > 1.0)
        rSinOrCos = 1.0;
}

bool lcl_isSinZero( double fAngleRad )
{
    return ::basegfx::fTools::equalZero( sin(fAngleRad), 0.0000001 );
}
bool lcl_isCosZero( double fAngleRad )
{
    return ::basegfx::fTools::equalZero( cos(fAngleRad), 0.0000001 );
}

}

void ThreeDHelper::convertElevationRotationDegToXYZAngleRad(
    sal_Int32 nElevationDeg, sal_Int32 nRotationDeg,
    double& rfXAngleRad, double& rfYAngleRad, double& rfZAngleRad)
{
    // for a description of the algorithm see issue 72994
    //https://bz.apache.org/ooo/show_bug.cgi?id=72994
    //https://bz.apache.org/ooo/attachment.cgi?id=50608

    nElevationDeg = NormAngle360(nElevationDeg);
    nRotationDeg = NormAngle360(nRotationDeg);

    double& x = rfXAngleRad;
    double& y = rfYAngleRad;
    double& z = rfZAngleRad;

    double E = basegfx::deg2rad(nElevationDeg); //elevation in Rad
    double R = basegfx::deg2rad(nRotationDeg); //rotation in Rad

    if( (nRotationDeg == 0 || nRotationDeg == 180 )
        && ( nElevationDeg == 90 || nElevationDeg == 270 ) )
    {
        //sR==0 && cE==0
        z = 0.0;
        //element 23
        double f23 = cos(R)*sin(E);
        if(f23>0)
            x = M_PI_2;
        else
            x = -M_PI_2;
        y = R;
    }
    else if( ( nRotationDeg == 90 || nRotationDeg == 270 )
        && ( nElevationDeg == 90 || nElevationDeg == 270 ) )
    {
        //cR==0 && cE==0
        z = M_PI_2;
        if( sin(R)>0 )
            x = M_PI_2;
        else
            x = -M_PI_2;

        if( (sin(R)*sin(E))>0 )
            y = 0.0;
        else
            y = M_PI;
    }
    else if( (nRotationDeg == 0 || nRotationDeg == 180 )
        && ( nElevationDeg == 0 || nElevationDeg == 180 ) )
    {
        //sR==0 && sE==0
        z = 0.0;
        y = R;
        x = E;
    }
    else if( ( nRotationDeg == 90 || nRotationDeg == 270 )
        && ( nElevationDeg == 0 || nElevationDeg == 180 ) )
    {
        //cR==0 && sE==0
        z = 0.0;

        if( (sin(R)/cos(E))>0 )
            y = M_PI_2;
        else
            y = -M_PI_2;

        if( (cos(E))>0 )
            x = 0;
        else
            x = M_PI;
    }
    else if ( nElevationDeg == 0 || nElevationDeg == 180 )
    {
        //sR!=0 cR!=0 sE==0
        z = 0.0;
        x = E;
        y = R;
        //use element 13 for sign
        if((cos(x)*sin(y)*sin(R))<0.0)
            y *= -1.0;
    }
    else if ( nElevationDeg == 90 || nElevationDeg == 270 )
    {
        //sR!=0 cR!=0 cE==0
        //element 12 + 22 --> y=0 or M_PI and x=+-M_PI/2
        //-->element 13/23:
        z = atan(sin(R)/(cos(R)*sin(E)));
        //use element 13 for sign for x
        if( (sin(R)*sin(z))>0.0 )
            x = M_PI_2;
        else
            x = -M_PI_2;
        //use element 21 for y
        if( (sin(R)*sin(E)*sin(z))>0.0)
            y = 0.0;
        else
            y = M_PI;
    }
    else if ( nRotationDeg == 0 || nRotationDeg == 180 )
    {
        //sE!=0 cE!=0 sR==0
        z = 0.0;
        x = E;
        y = R;
        double f23 = cos(R)*sin(E);
        if( (f23 * sin(x)) < 0.0 )
            x *= -1.0; //todo ??
    }
    else if (nRotationDeg == 90 || nRotationDeg == 270)
    {
        //sE!=0 cE!=0 cR==0
        //z = +- M_PI/2;
        //x = +- M_PI/2;
        z = M_PI_2;
        x = M_PI_2;
        double sR = sin(R);
        if( sR<0.0 )
            x *= -1.0; //different signs for x and z

        //use element 21:
        double cy = sR*sin(E)/sin(z);
        lcl_ensureIntervalMinus1To1(cy);
        y = acos(cy);

        //use element 22 for sign:
        if( (sin(x)*sin(y)*sin(z)*cos(E))<0.0)
            y *= -1.0;
    }
    else
    {
        z = atan(tan(R) * sin(E));
        if(cos(z)==0.0)
        {
            OSL_FAIL("calculation error in ThreeDHelper::convertElevationRotationDegToXYZAngleRad");
            return;
        }
        double cy = cos(R)/cos(z);
        lcl_ensureIntervalMinus1To1(cy);
        y = acos(cy);

        //element 12 in 23
        double fDenominator = cos(z)*(1.0-pow(sin(y),2));
        if(fDenominator==0.0)
        {
            OSL_FAIL("calculation error in ThreeDHelper::convertElevationRotationDegToXYZAngleRad");
            return;
        }
        double sx = cos(R)*sin(E)/fDenominator;
        lcl_ensureIntervalMinus1To1(sx);
        x = asin( sx );

        //use element 13 for sign:
        double f13a = cos(x)*cos(z)*sin(y);
        double f13b = sin(R)-sx*sin(z);
        if( (f13b*f13a)<0.0 )
        {
            //change x or y
            //use element 22 for further investigations:
            //try
            y *= -1;
            double f22a = cos(x)*cos(z);
            double f22b = cos(E)-(sx*sin(y)*sin(z));
            if( (f22a*f22b)<0.0 )
            {
                y *= -1;
                x=(M_PI-x);
            }
        }
        else
        {
            //change nothing or both
            //use element 22 for further investigations:
            double f22a = cos(x)*cos(z);
            double f22b = cos(E)-(sx*sin(y)*sin(z));
            if( (f22a*f22b)<0.0 )
            {
                y *= -1;
                x=(M_PI-x);
            }
        }
    }
}

void ThreeDHelper::convertXYZAngleRadToElevationRotationDeg(
    sal_Int32& rnElevationDeg, sal_Int32& rnRotationDeg,
    double fXRad, double fYRad, double fZRad)
{
    // for a description of the algorithm see issue 72994
    //https://bz.apache.org/ooo/show_bug.cgi?id=72994
    //https://bz.apache.org/ooo/attachment.cgi?id=50608

    double R = 0.0; //Rotation in Rad
    double E = 0.0; //Elevation in Rad

    double& x = fXRad;
    double& y = fYRad;
    double& z = fZRad;

    double f11 = cos(y)*cos(z);

    if( lcl_isSinZero(y) )
    {
        //siny == 0

        if( lcl_isCosZero(x) )
        {
            //siny == 0 && cosx == 0

            if( lcl_isSinZero(z) )
            {
                //siny == 0 && cosx == 0 && sinz == 0
                //example: x=+-90 y=0oder180 z=0(oder180)

                //element 13+11
                if( f11 > 0 )
                    R = 0.0;
                else
                    R = M_PI;

                //element 23
                double f23 = cos(z)*sin(x) / cos(R);
                if( f23 > 0 )
                    E = M_PI_2;
                else
                    E = -M_PI_2;
            }
            else if( lcl_isCosZero(z) )
            {
                //siny == 0 && cosx == 0 && cosz == 0
                //example: x=+-90 y=0oder180 z=+-90

                double f13 = sin(x)*sin(z);
                //element 13+11
                if( f13 > 0 )
                    R = M_PI_2;
                else
                    R = -M_PI_2;

                //element 21
                double f21 = cos(y)*sin(z) / sin(R);
                if( f21 > 0 )
                    E = M_PI_2;
                else
                    E = -M_PI_2;
            }
            else
            {
                //siny == 0 && cosx == 0 && cosz != 0 && sinz != 0
                //element 11 && 13
                double f13 = sin(x)*sin(z);
                R = atan2( f13, f11 );
                //element 23
                double f23 = cos(z)*sin(x);
                if( f23/cos(R) > 0 )
                    E = M_PI_2;
                else
                    E = -M_PI_2;
            }
        }
        else if( lcl_isSinZero(x) )
        {
            //sinY==0 sinX==0
            //element 13+11
            if( f11 > 0 )
                R = 0.0;
            else
                R = M_PI;

            double f22 = cos(x)*cos(z);
            if( f22 > 0 )
                E = 0.0;
            else
                E = M_PI;
        }
        else if( lcl_isSinZero(z) )
        {
            //sinY==0 sinZ==0 sinx!=0 cosx!=0
            //element 13+11
            if( f11 > 0 )
                R = 0.0;
            else
                R = M_PI;

            //element 22 && 23
            double f22 = cos(x)*cos(z);
            double f23 = cos(z)*sin(x);
            E = atan( f23/(f22*cos(R)) );
            if( (f22*cos(E))<0 )
                E+=M_PI;
        }
        else if( lcl_isCosZero(z) )
        {
            //sinY == 0 && cosZ == 0 && cosx != 0 && sinx != 0
            double f13 = sin(x)*sin(z);
            //element 13+11
            if( f13 > 0 )
                R = M_PI_2;
            else
                R = -M_PI_2;

            //element 21+22
            double f21 = cos(y)*sin(z);
            if( f21/sin(R) > 0 )
                E = M_PI_2;
            else
                E = -M_PI_2;
        }
        else
        {
            //sinY == 0 && all other !=0
            double f13 = sin(x)*sin(z);
            R = atan( f13/f11 );
            if( (f11*cos(R))<0.0 )
                R+=M_PI;

            double f22 = cos(x)*cos(z);
            if( !lcl_isCosZero(R) )
                E = atan( cos(z)*sin(x) /( f22*cos(R) ) );
            else
                E = atan( cos(y)*sin(z) /( f22*sin(R) ) );
            if( (f22*cos(E))<0 )
                E+=M_PI;
        }
    }
    else if( lcl_isCosZero(y) )
    {
        //cosY==0

        double f13 = sin(x)*sin(z)+cos(x)*cos(z)*sin(y);
        if( f13 >= 0 )
            R = M_PI_2;
        else
            R = -M_PI_2;

        double f22 = cos(x)*cos(z)+sin(x)*sin(y)*sin(z);
        if( f22 >= 0 )
            E = 0.0;
        else
            E = M_PI;
    }
    else if( lcl_isSinZero(x) )
    {
        //cosY!=0 sinY!=0 sinX=0
        if( lcl_isSinZero(z) )
        {
            //cosY!=0 sinY!=0 sinX=0 sinZ=0
            double f13 = cos(x)*cos(z)*sin(y);
            R = atan2( f13, f11 );
            double f22 = cos(x)*cos(z);
            if( f22>0 )
                E = 0.0;
            else
                E = M_PI;
        }
        else if( lcl_isCosZero(z) )
        {
            //cosY!=0 sinY!=0 sinX=0 cosZ=0
            R = x;
            E = y;//or -y
            //use 23 for 'signs'
            double f23 =  -1.0*cos(x)*sin(y)*sin(z);
            if( (f23*cos(R)*sin(E))<0.0 )
            {
                //change R or E
                E = -y;
            }
        }
        else
        {
            //cosY!=0 sinY!=0 sinX=0 sinZ!=0 cosZ!=0
            double f13 = cos(x)*cos(z)*sin(y);
            R = atan2( f13, f11 );
            double f21 = cos(y)*sin(z);
            double f22 = cos(x)*cos(z);
            E = atan(f21/(f22*sin(R)) );

            if( (f22*cos(E))<0.0 )
                E+=M_PI;
        }
    }
    else if( lcl_isCosZero(x) )
    {
        //cosY!=0 sinY!=0 cosX=0

        if( lcl_isSinZero(z) )
        {
            //cosY!=0 sinY!=0 cosX=0 sinZ=0
            R=0;//13 -> R=0 or M_PI
            if( f11<0.0 )
                R=M_PI;
            E=M_PI_2;//22 -> E=+-M_PI/2
            //use element 11 and 23 for sign
            double f23 = cos(z)*sin(x);
            if( (f11*f23*sin(E))<0.0 )
                E=-M_PI_2;
        }
        else if( lcl_isCosZero(z) )
        {
            //cosY!=0 sinY!=0 cosX=0 cosZ=0
            //element 11 & 13:
            if( (sin(x)*sin(z))>0.0 )
                R=M_PI_2;
            else
                R=-M_PI_2;
            //element 22:
            E=acos( sin(x)*sin(y)*sin(z));
            //use element 21 for sign:
            if( (cos(y)*sin(z)*sin(R)*sin(E))<0.0 )
                E*=-1.0;
        }
        else
        {
            //cosY!=0 sinY!=0 cosX=0 sinZ!=0 cosZ!=0
            //element 13/11
            R = atan2( sin(x)*sin(z), (cos(y)*cos(z)) );
            //element 22
            E = acos(sin(x)*sin(y)*sin(z) );
            //use 21 for sign
            if( (cos(y)*sin(z)*sin(R)*sin(E))<0.0 )
                E*=-1.0;
        }
    }
    else if( lcl_isSinZero(z) )
    {
        //cosY!=0 sinY!=0 sinX!=0 cosX!=0 sinZ=0
        //element 11
        R=y;
        //use element 13 for sign
        if( (cos(x)*cos(z)*sin(y)*sin(R))<0.0 )
            R*=-1.0;
        //element 22
        E = acos( cos(x)*cos(z) );
        //use element 23 for sign
        if( (cos(z)*sin(x)*cos(R)*sin(E))<0.0 )
            E*=-1.0;
    }
    else if( lcl_isCosZero(z) )
    {
        //cosY!=0 sinY!=0 sinX!=0 cosX!=0 cosZ=0
        //element 21/23
        R=atan(-cos(y)/(cos(x)*sin(y)));
        //use element 13 for 'sign'
        if( (sin(x)*sin(z)*sin(R))<0.0 )
            R+=M_PI;
        //element 21/22
        E=atan( cos(y)*sin(z)/(sin(R)*sin(x)*sin(y)*sin(z)) );
        //use element 23 for 'sign'
        if( (-cos(x)*sin(y)*sin(z)*cos(R)*sin(E))<0.0 )
            E+=M_PI;
    }
    else
    {
        //cosY!=0 sinY!=0 sinX!=0 cosX!=0 sinZ!=0 cosZ!=0
        //13/11:
        double f13 = sin(x)*sin(z)+cos(x)*cos(z)*sin(y);
        R = atan2( f13, f11 );
        double f22 = cos(x)*cos(z)+sin(x)*sin(y)*sin(z);
        double f23 = cos(x)*sin(y)*sin(z)-cos(z)*sin(x);
        //23/22:
        E = atan( -1.0*f23/(f22*cos(R)) );
        if(f22<0.0)
            E+=M_PI;
    }

    rnElevationDeg = basegfx::fround(basegfx::rad2deg(E));
    rnRotationDeg = basegfx::fround(basegfx::rad2deg(R));
}

double ThreeDHelper::getValueClippedToRange( double fAngle, const double& fPositivLimit )
{
    if( fAngle<-1*fPositivLimit )
        fAngle=-1*fPositivLimit;
    else if( fAngle>fPositivLimit )
        fAngle=fPositivLimit;
    return fAngle;
}

void ThreeDHelper::adaptRadAnglesForRightAngledAxes( double& rfXAngleRad, double& rfYAngleRad )
{
    rfXAngleRad = ThreeDHelper::getValueClippedToRange(rfXAngleRad, basegfx::deg2rad(ThreeDHelper::getXDegreeAngleLimitForRightAngledAxes()) );
    rfYAngleRad = ThreeDHelper::getValueClippedToRange(rfYAngleRad, basegfx::deg2rad(ThreeDHelper::getYDegreeAngleLimitForRightAngledAxes()) );
}

void ThreeDHelper::getRotationAngleFromDiagram(
        const Reference< beans::XPropertySet >& xSceneProperties, double& rfXAngleRad, double& rfYAngleRad, double& rfZAngleRad )
{
    //takes the camera and the transformation matrix into account

    rfXAngleRad = rfYAngleRad = rfZAngleRad = 0.0;

    if( !xSceneProperties.is() )
        return;

    //get camera rotation
    ::basegfx::B3DHomMatrix aFixCameraRotationMatrix( lcl_getCameraMatrix( xSceneProperties ) );
    BaseGFXHelper::ReduceToRotationMatrix( aFixCameraRotationMatrix );

    //get scene rotation
    ::basegfx::B3DHomMatrix aSceneRotation;
    {
        drawing::HomogenMatrix aHomMatrix;
        if( xSceneProperties->getPropertyValue( "D3DTransformMatrix") >>= aHomMatrix )
        {
            aSceneRotation = BaseGFXHelper::HomogenMatrixToB3DHomMatrix( aHomMatrix );
            BaseGFXHelper::ReduceToRotationMatrix( aSceneRotation );
        }
    }

    ::basegfx::B3DHomMatrix aResultRotation = aFixCameraRotationMatrix * aSceneRotation;
    ::basegfx::B3DTuple aRotation( BaseGFXHelper::GetRotationFromMatrix( aResultRotation ) );

    rfXAngleRad = lcl_shiftAngleToIntervalMinusPiToPi(aRotation.getX());
    rfYAngleRad = lcl_shiftAngleToIntervalMinusPiToPi(aRotation.getY());
    rfZAngleRad = lcl_shiftAngleToIntervalMinusPiToPi(aRotation.getZ());

    if(rfZAngleRad<-M_PI_2 || rfZAngleRad>M_PI_2)
    {
        rfZAngleRad-=M_PI;
        rfXAngleRad-=M_PI;
        rfYAngleRad=(M_PI-rfYAngleRad);

        rfXAngleRad = lcl_shiftAngleToIntervalMinusPiToPi(rfXAngleRad);
        rfYAngleRad = lcl_shiftAngleToIntervalMinusPiToPi(rfYAngleRad);
        rfZAngleRad = lcl_shiftAngleToIntervalMinusPiToPi(rfZAngleRad);
    }
}

void ThreeDHelper::switchRightAngledAxes( const Reference< beans::XPropertySet >& xSceneProperties, bool bRightAngledAxes )
{
    try
    {
        if( xSceneProperties.is() )
        {
            bool bOldRightAngledAxes = false;
            xSceneProperties->getPropertyValue( "RightAngledAxes") >>= bOldRightAngledAxes;
            if( bOldRightAngledAxes!=bRightAngledAxes)
            {
                xSceneProperties->setPropertyValue( "RightAngledAxes", uno::Any( bRightAngledAxes ));
                if(bRightAngledAxes)
                {
                    ::basegfx::B3DHomMatrix aInverseRotation( lcl_getInverseRotationMatrix( xSceneProperties ) );
                    lcl_rotateLights( aInverseRotation, xSceneProperties );
                }
                else
                {
                    ::basegfx::B3DHomMatrix aCompleteRotation( lcl_getCompleteRotationMatrix( xSceneProperties ) );
                    lcl_rotateLights( aCompleteRotation, xSceneProperties );
                }
            }
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

void ThreeDHelper::setRotationAngleToDiagram(
    const Reference< beans::XPropertySet >& xSceneProperties
        , double fXAngleRad, double fYAngleRad, double fZAngleRad )
{
    //the rotation of the camera is not touched but taken into account
    //the rotation difference is applied to the transformation matrix

    //the light sources will be adapted also

    if( !xSceneProperties.is() )
        return;

    try
    {
        //remind old rotation for adaptation of light directions
        ::basegfx::B3DHomMatrix aInverseOldRotation( lcl_getInverseRotationMatrix( xSceneProperties ) );

        ::basegfx::B3DHomMatrix aInverseCameraRotation;
        {
            ::basegfx::B3DTuple aR( BaseGFXHelper::GetRotationFromMatrix(
                    lcl_getCameraMatrix( xSceneProperties ) ) );
            aInverseCameraRotation.rotate( 0.0, 0.0, -aR.getZ() );
            aInverseCameraRotation.rotate( 0.0, -aR.getY(), 0.0 );
            aInverseCameraRotation.rotate( -aR.getX(), 0.0, 0.0 );
        }

        ::basegfx::B3DHomMatrix aCumulatedRotation;
        aCumulatedRotation.rotate( fXAngleRad, fYAngleRad, fZAngleRad );

        //calculate new scene matrix
        ::basegfx::B3DHomMatrix aSceneRotation = aInverseCameraRotation*aCumulatedRotation;
        BaseGFXHelper::ReduceToRotationMatrix( aSceneRotation );

        //set new rotation to transformation matrix
        xSceneProperties->setPropertyValue(
            "D3DTransformMatrix", uno::Any( BaseGFXHelper::B3DHomMatrixToHomogenMatrix( aSceneRotation )));

        //rotate lights if RightAngledAxes are not set or not supported
        bool bRightAngledAxes = false;
        xSceneProperties->getPropertyValue( "RightAngledAxes") >>= bRightAngledAxes;
        uno::Reference< chart2::XDiagram > xDiagram( xSceneProperties, uno::UNO_QUERY );
        if(!bRightAngledAxes || !ChartTypeHelper::isSupportingRightAngledAxes(
                    DiagramHelper::getChartTypeByIndex( xDiagram, 0 ) ) )
        {
            ::basegfx::B3DHomMatrix aNewRotation;
            aNewRotation.rotate( fXAngleRad, fYAngleRad, fZAngleRad );
            lcl_rotateLights( aNewRotation*aInverseOldRotation, xSceneProperties );
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

void ThreeDHelper::getRotationFromDiagram( const uno::Reference< beans::XPropertySet >& xSceneProperties
            , sal_Int32& rnHorizontalAngleDegree, sal_Int32& rnVerticalAngleDegree )
{
    double fXAngle, fYAngle, fZAngle;
    ThreeDHelper::getRotationAngleFromDiagram( xSceneProperties, fXAngle, fYAngle, fZAngle );

    if( !lcl_isRightAngledAxesSetAndSupported( xSceneProperties ) )
    {
        ThreeDHelper::convertXYZAngleRadToElevationRotationDeg(
            rnHorizontalAngleDegree, rnVerticalAngleDegree, fXAngle, fYAngle, fZAngle);
        rnVerticalAngleDegree*=-1;
    }
    else
    {
        rnHorizontalAngleDegree = basegfx::fround(basegfx::rad2deg(fXAngle));
        rnVerticalAngleDegree = basegfx::fround(-1.0 * basegfx::rad2deg(fYAngle));
        // nZRotation = basegfx::fround(-1.0 * basegfx::rad2deg(fZAngle));
    }

    rnHorizontalAngleDegree = NormAngle180(rnHorizontalAngleDegree);
    rnVerticalAngleDegree = NormAngle180(rnVerticalAngleDegree);
}

void ThreeDHelper::setRotationToDiagram( const uno::Reference< beans::XPropertySet >& xSceneProperties
            , sal_Int32 nHorizontalAngleDegree, sal_Int32 nVerticalYAngleDegree )
{
    //todo: x and y is not equal to horz and vert in case of RightAngledAxes==false
    double fXAngle = basegfx::deg2rad(nHorizontalAngleDegree);
    double fYAngle = basegfx::deg2rad(-1 * nVerticalYAngleDegree);
    double fZAngle = 0.0;

    if( !lcl_isRightAngledAxesSetAndSupported( xSceneProperties ) )
        ThreeDHelper::convertElevationRotationDegToXYZAngleRad(
            nHorizontalAngleDegree, -1*nVerticalYAngleDegree, fXAngle, fYAngle, fZAngle );

    ThreeDHelper::setRotationAngleToDiagram( xSceneProperties, fXAngle, fYAngle, fZAngle );
}

void ThreeDHelper::getCameraDistanceRange( double& rfMinimumDistance, double& rfMaximumDistance )
{
    rfMinimumDistance = 3.0/4.0*FIXED_SIZE_FOR_3D_CHART_VOLUME;//empiric value
    rfMaximumDistance = 20.0*FIXED_SIZE_FOR_3D_CHART_VOLUME;//empiric value
}

void ThreeDHelper::ensureCameraDistanceRange( double& rfCameraDistance )
{
    double fMin, fMax;
    getCameraDistanceRange( fMin, fMax );
    if( rfCameraDistance < fMin )
        rfCameraDistance = fMin;
    if( rfCameraDistance > fMax )
        rfCameraDistance = fMax;
}

double ThreeDHelper::getCameraDistance(
        const Reference< beans::XPropertySet >& xSceneProperties )
{
    double fCameraDistance = FIXED_SIZE_FOR_3D_CHART_VOLUME;

    if( !xSceneProperties.is() )
        return fCameraDistance;

    try
    {
        drawing::CameraGeometry aCG( ThreeDHelper::getDefaultCameraGeometry() );
        xSceneProperties->getPropertyValue( "D3DCameraGeometry" ) >>= aCG;
        ::basegfx::B3DVector aVRP( BaseGFXHelper::Position3DToB3DVector( aCG.vrp ) );
        fCameraDistance = aVRP.getLength();

        ensureCameraDistanceRange( fCameraDistance );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
    return fCameraDistance;
}

void ThreeDHelper::setCameraDistance(
        const Reference< beans::XPropertySet >& xSceneProperties, double fCameraDistance )
{
    if( !xSceneProperties.is() )
        return;

    try
    {
        if( fCameraDistance <= 0 )
            fCameraDistance = FIXED_SIZE_FOR_3D_CHART_VOLUME;

        drawing::CameraGeometry aCG( ThreeDHelper::getDefaultCameraGeometry() );
        xSceneProperties->getPropertyValue( "D3DCameraGeometry" ) >>= aCG;
        ::basegfx::B3DVector aVRP( BaseGFXHelper::Position3DToB3DVector( aCG.vrp ) );
        if( ::basegfx::fTools::equalZero( aVRP.getLength() ) )
            aVRP = ::basegfx::B3DVector(0,0,1);
        aVRP.setLength(fCameraDistance);
        aCG.vrp = BaseGFXHelper::B3DVectorToPosition3D( aVRP );

        xSceneProperties->setPropertyValue( "D3DCameraGeometry", uno::Any( aCG ));
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

double ThreeDHelper::CameraDistanceToPerspective( double fCameraDistance )
{
    double fMin, fMax;
    ThreeDHelper::getCameraDistanceRange( fMin, fMax );
    //fMax <-> 0; fMin <->100
    //a/x + b = y
    double a = 100.0*fMax*fMin/(fMax-fMin);
    double b = -a/fMax;

    double fRet = a/fCameraDistance + b;

    return fRet;
}

double ThreeDHelper::PerspectiveToCameraDistance( double fPerspective )
{
    double fMin, fMax;
    ThreeDHelper::getCameraDistanceRange( fMin, fMax );
    //fMax <-> 0; fMin <->100
    //a/x + b = y
    double a = 100.0*fMax*fMin/(fMax-fMin);
    double b = -a/fMax;

    double fRet = a/(fPerspective - b);

    return fRet;
}

ThreeDLookScheme ThreeDHelper::detectScheme( const uno::Reference< XDiagram >& xDiagram )
{
    ThreeDLookScheme aScheme = ThreeDLookScheme::ThreeDLookScheme_Unknown;

    sal_Int32 nRoundedEdges;
    sal_Int32 nObjectLines;
    ThreeDHelper::getRoundedEdgesAndObjectLines( xDiagram, nRoundedEdges, nObjectLines );

    //get shade mode and light settings:
    drawing::ShadeMode aShadeMode( drawing::ShadeMode_SMOOTH );
    uno::Reference< beans::XPropertySet > xDiagramProps( xDiagram, uno::UNO_QUERY );
    try
    {
        if( xDiagramProps.is() )
            xDiagramProps->getPropertyValue( "D3DSceneShadeMode" )>>= aShadeMode;
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    if( lcl_isSimpleScheme( aShadeMode, nRoundedEdges, nObjectLines, xDiagram ) )
    {
        if( lcl_isSimpleLightScheme(xDiagramProps) )
            aScheme = ThreeDLookScheme::ThreeDLookScheme_Simple;
    }
    else if( lcl_isRealisticScheme( aShadeMode, nRoundedEdges, nObjectLines ) )
    {
        if( lcl_isRealisticLightScheme(xDiagramProps) )
            aScheme = ThreeDLookScheme::ThreeDLookScheme_Realistic;
    }

    return aScheme;
}

void ThreeDHelper::setScheme( const uno::Reference< XDiagram >& xDiagram, ThreeDLookScheme aScheme )
{
    if( aScheme == ThreeDLookScheme::ThreeDLookScheme_Unknown )
        return;

    drawing::ShadeMode aShadeMode;
    sal_Int32 nRoundedEdges;
    sal_Int32 nObjectLines;

    if( aScheme == ThreeDLookScheme::ThreeDLookScheme_Simple )
        lcl_setSimpleScheme(aShadeMode,nRoundedEdges,nObjectLines,xDiagram);
    else
        lcl_setRealisticScheme(aShadeMode,nRoundedEdges,nObjectLines);

    try
    {
        ThreeDHelper::setRoundedEdgesAndObjectLines( xDiagram, nRoundedEdges, nObjectLines );

        uno::Reference< beans::XPropertySet > xProp( xDiagram, uno::UNO_QUERY );
        if( xProp.is() )
        {
            drawing::ShadeMode aOldShadeMode;
            if( ! ( (xProp->getPropertyValue( "D3DSceneShadeMode" )>>=aOldShadeMode) &&
                    aOldShadeMode == aShadeMode ))
            {
                xProp->setPropertyValue( "D3DSceneShadeMode", uno::Any( aShadeMode ));
            }
        }

        lcl_setLightsForScheme( xProp, aScheme );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

}

void ThreeDHelper::set3DSettingsToDefault( const uno::Reference< beans::XPropertySet >& xSceneProperties )
{
    Reference< beans::XPropertyState > xState( xSceneProperties, uno::UNO_QUERY );
    if(xState.is())
    {
        xState->setPropertyToDefault( "D3DSceneDistance");
        xState->setPropertyToDefault( "D3DSceneFocalLength");
    }
    ThreeDHelper::setDefaultRotation( xSceneProperties );
    ThreeDHelper::setDefaultIllumination( xSceneProperties );
}

void ThreeDHelper::setDefaultRotation( const uno::Reference< beans::XPropertySet >& xSceneProperties, bool bPieOrDonut )
{
    if( !xSceneProperties.is() )
        return;

    drawing::CameraGeometry aCameraGeo( ThreeDHelper::getDefaultCameraGeometry( bPieOrDonut ) );
    xSceneProperties->setPropertyValue( "D3DCameraGeometry", uno::Any( aCameraGeo ));

    ::basegfx::B3DHomMatrix aSceneRotation;
    if( bPieOrDonut )
        aSceneRotation.rotate( -M_PI/3.0, 0, 0 );
    xSceneProperties->setPropertyValue( "D3DTransformMatrix",
        uno::Any( BaseGFXHelper::B3DHomMatrixToHomogenMatrix( aSceneRotation )));
}

void ThreeDHelper::setDefaultRotation( const uno::Reference< beans::XPropertySet >& xSceneProperties )
{
    bool bPieOrDonut( DiagramHelper::isPieOrDonutChart( uno::Reference< XDiagram >(xSceneProperties, uno::UNO_QUERY) ) );
    ThreeDHelper::setDefaultRotation( xSceneProperties, bPieOrDonut );
}

void ThreeDHelper::setDefaultIllumination( const uno::Reference< beans::XPropertySet >& xSceneProperties )
{
    if( !xSceneProperties.is() )
        return;

    drawing::ShadeMode aShadeMode( drawing::ShadeMode_SMOOTH );
    try
    {
        xSceneProperties->getPropertyValue( "D3DSceneShadeMode" )>>= aShadeMode;
        xSceneProperties->setPropertyValue( UNO_NAME_3D_SCENE_LIGHTON_1, uno::Any( false ) );
        xSceneProperties->setPropertyValue( UNO_NAME_3D_SCENE_LIGHTON_3, uno::Any( false ) );
        xSceneProperties->setPropertyValue( UNO_NAME_3D_SCENE_LIGHTON_4, uno::Any( false ) );
        xSceneProperties->setPropertyValue( UNO_NAME_3D_SCENE_LIGHTON_5, uno::Any( false ) );
        xSceneProperties->setPropertyValue( UNO_NAME_3D_SCENE_LIGHTON_6, uno::Any( false ) );
        xSceneProperties->setPropertyValue( UNO_NAME_3D_SCENE_LIGHTON_7, uno::Any( false ) );
        xSceneProperties->setPropertyValue( UNO_NAME_3D_SCENE_LIGHTON_8, uno::Any( false ) );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    ThreeDLookScheme aScheme = (aShadeMode == drawing::ShadeMode_FLAT)
                                   ? ThreeDLookScheme::ThreeDLookScheme_Simple
                                   : ThreeDLookScheme::ThreeDLookScheme_Realistic;
    lcl_setLightsForScheme( xSceneProperties, aScheme );
}

void ThreeDHelper::getRoundedEdgesAndObjectLines(
            const uno::Reference< XDiagram > & xDiagram
            , sal_Int32& rnRoundedEdges, sal_Int32& rnObjectLines )
{
    rnRoundedEdges = -1;
    rnObjectLines = -1;
    try
    {
        bool bDifferentRoundedEdges = false;
        bool bDifferentObjectLines = false;

        drawing::LineStyle aLineStyle( drawing::LineStyle_SOLID );

        std::vector< uno::Reference< XDataSeries > > aSeriesList(
            DiagramHelper::getDataSeriesFromDiagram( xDiagram ) );
        sal_Int32 nSeriesCount = static_cast<sal_Int32>( aSeriesList.size() );

        OUString aPercentDiagonalPropertyName( "PercentDiagonal" );
        OUString aBorderStylePropertyName( "BorderStyle" );

        for( sal_Int32 nS = 0; nS < nSeriesCount; ++nS )
        {
            uno::Reference< XDataSeries > xSeries( aSeriesList[nS] );
            uno::Reference< beans::XPropertySet > xProp( xSeries, uno::UNO_QUERY );
            if(!nS)
            {
                rnRoundedEdges = 0;
                try
                {
                    sal_Int16 nPercentDiagonal = 0;

                    xProp->getPropertyValue( aPercentDiagonalPropertyName ) >>= nPercentDiagonal;
                    rnRoundedEdges = static_cast< sal_Int32 >( nPercentDiagonal );

                    if( DataSeriesHelper::hasAttributedDataPointDifferentValue( xSeries
                        , aPercentDiagonalPropertyName, uno::Any(nPercentDiagonal) ) )
                        bDifferentRoundedEdges = true;
                }
                catch( const uno::Exception& )
                {
                    TOOLS_WARN_EXCEPTION("chart2", "" );
                    bDifferentRoundedEdges = true;
                }
                try
                {
                    xProp->getPropertyValue( aBorderStylePropertyName ) >>= aLineStyle;

                    if( DataSeriesHelper::hasAttributedDataPointDifferentValue( xSeries
                        , aBorderStylePropertyName, uno::Any(aLineStyle) ) )
                        bDifferentObjectLines = true;
                }
                catch( const uno::Exception& )
                {
                    TOOLS_WARN_EXCEPTION("chart2", "" );
                    bDifferentObjectLines = true;
                }
            }
            else
            {
                if( !bDifferentRoundedEdges )
                {
                    sal_Int16 nPercentDiagonal = 0;
                    xProp->getPropertyValue( aPercentDiagonalPropertyName ) >>= nPercentDiagonal;
                    sal_Int32 nCurrentRoundedEdges = static_cast< sal_Int32 >( nPercentDiagonal );
                    if(nCurrentRoundedEdges!=rnRoundedEdges
                        || DataSeriesHelper::hasAttributedDataPointDifferentValue( xSeries
                            , aPercentDiagonalPropertyName, uno::Any( static_cast< sal_Int16 >(rnRoundedEdges) ) ) )
                    {
                        bDifferentRoundedEdges = true;
                    }
                }

                if( !bDifferentObjectLines )
                {
                    drawing::LineStyle aCurrentLineStyle;
                    xProp->getPropertyValue( aBorderStylePropertyName ) >>= aCurrentLineStyle;
                    if(aCurrentLineStyle!=aLineStyle
                        || DataSeriesHelper::hasAttributedDataPointDifferentValue( xSeries
                            , aBorderStylePropertyName, uno::Any(aLineStyle) ) )
                        bDifferentObjectLines = true;
                }
            }
            if( bDifferentRoundedEdges && bDifferentObjectLines )
                break;
        }

        //set rnObjectLines
        rnObjectLines = 0;
        if( bDifferentObjectLines )
            rnObjectLines = -1;
        else if( aLineStyle == drawing::LineStyle_SOLID )
            rnObjectLines = 1;
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
}

void ThreeDHelper::setRoundedEdgesAndObjectLines(
            const uno::Reference< XDiagram > & xDiagram
            , sal_Int32 nRoundedEdges, sal_Int32 nObjectLines )
{
    if( (nRoundedEdges<0||nRoundedEdges>100) && nObjectLines!=0 && nObjectLines!=1 )
        return;

    drawing::LineStyle aLineStyle( drawing::LineStyle_NONE );
    if(nObjectLines==1)
        aLineStyle = drawing::LineStyle_SOLID;

    uno::Any aALineStyle( aLineStyle);
    uno::Any aARoundedEdges( static_cast< sal_Int16 >( nRoundedEdges ));

    std::vector< uno::Reference< XDataSeries > > aSeriesList(
        DiagramHelper::getDataSeriesFromDiagram( xDiagram ) );
    sal_Int32 nSeriesCount = static_cast<sal_Int32>( aSeriesList.size() );
    for( sal_Int32 nS = 0; nS < nSeriesCount; ++nS )
    {
        uno::Reference< XDataSeries > xSeries( aSeriesList[nS] );

        if( nRoundedEdges>=0 && nRoundedEdges<=100 )
            DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints( xSeries, "PercentDiagonal", aARoundedEdges );

        if( nObjectLines==0 || nObjectLines==1 )
            DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints( xSeries, "BorderStyle", aALineStyle );
    }
}

CuboidPlanePosition ThreeDHelper::getAutomaticCuboidPlanePositionForStandardLeftWall( const Reference< beans::XPropertySet >& xSceneProperties )
{
    CuboidPlanePosition eRet(CuboidPlanePosition_Left);

    double fXAngleRad=0.0; double fYAngleRad=0.0; double fZAngleRad=0.0;
    ThreeDHelper::getRotationAngleFromDiagram( xSceneProperties, fXAngleRad, fYAngleRad, fZAngleRad );
    if( lcl_isRightAngledAxesSetAndSupported( xSceneProperties ) )
    {
        ThreeDHelper::adaptRadAnglesForRightAngledAxes( fXAngleRad, fYAngleRad );
    }
    if( sin(fYAngleRad)>0.0 )
        eRet = CuboidPlanePosition_Right;
    return eRet;
}

CuboidPlanePosition ThreeDHelper::getAutomaticCuboidPlanePositionForStandardBackWall( const Reference< beans::XPropertySet >& xSceneProperties )
{
    CuboidPlanePosition eRet(CuboidPlanePosition_Back);

    double fXAngleRad=0.0; double fYAngleRad=0.0; double fZAngleRad=0.0;
    ThreeDHelper::getRotationAngleFromDiagram( xSceneProperties, fXAngleRad, fYAngleRad, fZAngleRad );
    if( lcl_isRightAngledAxesSetAndSupported( xSceneProperties ) )
    {
        ThreeDHelper::adaptRadAnglesForRightAngledAxes( fXAngleRad, fYAngleRad );
    }
    if( cos(fXAngleRad)*cos(fYAngleRad)<0.0 )
        eRet = CuboidPlanePosition_Front;
    return eRet;
}

CuboidPlanePosition ThreeDHelper::getAutomaticCuboidPlanePositionForStandardBottom( const Reference< beans::XPropertySet >& xSceneProperties )
{
    CuboidPlanePosition eRet(CuboidPlanePosition_Bottom);

    double fXAngleRad=0.0; double fYAngleRad=0.0; double fZAngleRad=0.0;
    ThreeDHelper::getRotationAngleFromDiagram( xSceneProperties, fXAngleRad, fYAngleRad, fZAngleRad );
    if( lcl_isRightAngledAxesSetAndSupported( xSceneProperties ) )
    {
        ThreeDHelper::adaptRadAnglesForRightAngledAxes( fXAngleRad, fYAngleRad );
    }
    if( sin(fXAngleRad)*cos(fYAngleRad)<0.0 )
        eRet = CuboidPlanePosition_Top;
    return eRet;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
