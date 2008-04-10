/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ThreeDHelper.cxx,v $
 * $Revision: 1.4 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "ThreeDHelper.hxx"
#include "macros.hxx"
#include "DiagramHelper.hxx"
#include "ChartTypeHelper.hxx"
#include "BaseGFXHelper.hxx"
#include "DataSeriesHelper.hxx"
#include <svx/unoprnms.hxx>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

#define FIXED_SIZE_FOR_3D_CHART_VOLUME (10000.0)

namespace
{

void lcl_RotateLightSource( const Reference< beans::XPropertySet >& xSceneProperties
                           , const OUString& rLightSourceDirection
                           , const OUString& rLightSourceOn
                           , const ::basegfx::B3DHomMatrix& rRotationMatrix )
{
    if( xSceneProperties.is() )
    {
        sal_Bool bLightOn = sal_False;
        if( xSceneProperties->getPropertyValue( rLightSourceOn ) >>= bLightOn )
        {
            if( bLightOn )
            {
                drawing::Direction3D aLight;
                if( xSceneProperties->getPropertyValue( rLightSourceDirection ) >>= aLight )
                {
                    ::basegfx::B3DVector aLightVector( BaseGFXHelper::Direction3DToB3DVector( aLight ) );
                    aLightVector = rRotationMatrix*aLightVector;

                    xSceneProperties->setPropertyValue( rLightSourceDirection
                        , uno::makeAny( BaseGFXHelper::B3DVectorToDirection3D( aLightVector ) ) );
                }
            }
        }
    }
}

void lcl_rotateLights( const ::basegfx::B3DHomMatrix& rLightRottion, const Reference< beans::XPropertySet >& xSceneProperties )
{
    if(!xSceneProperties.is())
        return;

    ::basegfx::B3DHomMatrix aLightRottion( rLightRottion );
    BaseGFXHelper::ReduceToRotationMatrix( aLightRottion );

    lcl_RotateLightSource( xSceneProperties, C2U("D3DSceneLightDirection1"), C2U("D3DSceneLightOn1"), aLightRottion );
    lcl_RotateLightSource( xSceneProperties, C2U("D3DSceneLightDirection2"), C2U("D3DSceneLightOn2"), aLightRottion );
    lcl_RotateLightSource( xSceneProperties, C2U("D3DSceneLightDirection3"), C2U("D3DSceneLightOn3"), aLightRottion );
    lcl_RotateLightSource( xSceneProperties, C2U("D3DSceneLightDirection4"), C2U("D3DSceneLightOn4"), aLightRottion );
    lcl_RotateLightSource( xSceneProperties, C2U("D3DSceneLightDirection5"), C2U("D3DSceneLightOn5"), aLightRottion );
    lcl_RotateLightSource( xSceneProperties, C2U("D3DSceneLightDirection6"), C2U("D3DSceneLightOn6"), aLightRottion );
    lcl_RotateLightSource( xSceneProperties, C2U("D3DSceneLightDirection7"), C2U("D3DSceneLightOn7"), aLightRottion );
    lcl_RotateLightSource( xSceneProperties, C2U("D3DSceneLightDirection8"), C2U("D3DSceneLightOn8"), aLightRottion );
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

    sal_Bool bIsOn = sal_False;
    xDiagramProps->getPropertyValue( C2U( UNO_NAME_3D_SCENE_LIGHTON_2 ) ) >>= bIsOn;
    if(!bIsOn)
        return false;

    uno::Reference< chart2::XDiagram > xDiagram( xDiagramProps, uno::UNO_QUERY );
    uno::Reference< chart2::XChartType > xChartType( DiagramHelper::getChartTypeByIndex( xDiagram, 0 ) );

    sal_Int32 nColor = 0;
    xDiagramProps->getPropertyValue( C2U( UNO_NAME_3D_SCENE_LIGHTCOLOR_2 ) ) >>= nColor;
    if( nColor != ::chart::ChartTypeHelper::getDefaultDirectLightColor( !bRealistic, xChartType ) )
        return false;

    sal_Int32 nAmbientColor = 0;
    xDiagramProps->getPropertyValue( C2U( UNO_NAME_3D_SCENE_AMBIENTCOLOR ) ) >>= nAmbientColor;
    if( nAmbientColor != ::chart::ChartTypeHelper::getDefaultAmbientLightColor( !bRealistic, xChartType ) )
        return false;

    drawing::Direction3D aDirection(0,0,0);
    xDiagramProps->getPropertyValue( C2U( UNO_NAME_3D_SCENE_LIGHTDIRECTION_2 ) ) >>= aDirection;

    drawing::Direction3D aDefaultDirection( bRealistic
        ? ChartTypeHelper::getDefaultRealisticLightDirection(xChartType)
        : ChartTypeHelper::getDefaultSimpleLightDirection(xChartType) );

    //rotate default light direction when right angled axes are off but supported
    {
        sal_Bool bRightAngledAxes = sal_False;
        xDiagramProps->getPropertyValue( C2U("RightAngledAxes")) >>= bRightAngledAxes;
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
    if( rScheme == ThreeDLookScheme_Unknown)
        return;

    xDiagramProps->setPropertyValue( C2U( UNO_NAME_3D_SCENE_LIGHTON_2 ), uno::makeAny( sal_True ) );

    uno::Reference< chart2::XDiagram > xDiagram( xDiagramProps, uno::UNO_QUERY );
    uno::Reference< chart2::XChartType > xChartType( DiagramHelper::getChartTypeByIndex( xDiagram, 0 ) );
    uno::Any aADirection( uno::makeAny( rScheme == ThreeDLookScheme_Simple
        ? ChartTypeHelper::getDefaultSimpleLightDirection(xChartType)
        : ChartTypeHelper::getDefaultRealisticLightDirection(xChartType) ) );

    xDiagramProps->setPropertyValue( C2U( UNO_NAME_3D_SCENE_LIGHTDIRECTION_2 ), aADirection );
    //rotate light direction when right angled axes are off but supported
    {
        sal_Bool bRightAngledAxes = sal_False;
        xDiagramProps->getPropertyValue( C2U("RightAngledAxes")) >>= bRightAngledAxes;
        if(!bRightAngledAxes)
        {
            if( ChartTypeHelper::isSupportingRightAngledAxes( xChartType ) )
            {
                ::basegfx::B3DHomMatrix aRotation( lcl_getCompleteRotationMatrix( xDiagramProps ) );
                BaseGFXHelper::ReduceToRotationMatrix( aRotation );
                lcl_RotateLightSource( xDiagramProps, C2U("D3DSceneLightDirection2"), C2U("D3DSceneLightOn2"), aRotation );
            }
        }
    }

    sal_Int32 nColor = ::chart::ChartTypeHelper::getDefaultDirectLightColor( rScheme==ThreeDLookScheme_Simple, xChartType );
    xDiagramProps->setPropertyValue( C2U( UNO_NAME_3D_SCENE_LIGHTCOLOR_2 ), uno::makeAny( nColor ) );

    sal_Int32 nAmbientColor = ::chart::ChartTypeHelper::getDefaultAmbientLightColor( rScheme==ThreeDLookScheme_Simple, xChartType );
    xDiagramProps->setPropertyValue( C2U( UNO_NAME_3D_SCENE_AMBIENTCOLOR ), uno::makeAny( nAmbientColor ) );
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
        vrp = drawing::Position3D( 0.0, 0.0, 87591.2408759124 );//--> 5 percent perspecitve
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
        xSceneProperties->getPropertyValue( C2U( "D3DCameraGeometry" ) ) >>= aCG;

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
    while( fAngleRad<=-F_PI )
        fAngleRad+=(2*F_PI);
    while( fAngleRad>F_PI )
        fAngleRad-=(2*F_PI);
    return fAngleRad;
}

}

double ThreeDHelper::getValueClippedToRange( double fAngle, const double& fPositivLimit )
{
    if( fAngle<-1*fPositivLimit )
        fAngle=-1*fPositivLimit;
    else if( fAngle>fPositivLimit )
        fAngle=fPositivLimit;
    return fAngle;
}

double ThreeDHelper::getXDegreeAngleLimitForRightAngledAxes()
{
    return 90.0;
}

double ThreeDHelper::getYDegreeAngleLimitForRightAngledAxes()
{
    return 45.0;
}

void ThreeDHelper::adaptRadAnglesForRightAngledAxes( double& rfXAngleRad, double& rfYAngleRad )
{
    rfXAngleRad = ThreeDHelper::getValueClippedToRange(rfXAngleRad, BaseGFXHelper::Deg2Rad(ThreeDHelper::getXDegreeAngleLimitForRightAngledAxes()) );
    rfYAngleRad = ThreeDHelper::getValueClippedToRange(rfYAngleRad, BaseGFXHelper::Deg2Rad(ThreeDHelper::getYDegreeAngleLimitForRightAngledAxes()) );
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
        if( xSceneProperties->getPropertyValue( C2U("D3DTransformMatrix")) >>= aHomMatrix )
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

    if(rfZAngleRad<(-F_PI/2) || rfZAngleRad>(F_PI/2))
    {
        rfZAngleRad-=F_PI;
        rfXAngleRad-=F_PI;
        rfYAngleRad=(F_PI-rfYAngleRad);

        rfXAngleRad = lcl_shiftAngleToIntervalMinusPiToPi(rfXAngleRad);
        rfYAngleRad = lcl_shiftAngleToIntervalMinusPiToPi(rfYAngleRad);
        rfZAngleRad = lcl_shiftAngleToIntervalMinusPiToPi(rfZAngleRad);
    }
}

void ThreeDHelper::switchRightAngledAxes( const Reference< beans::XPropertySet >& xSceneProperties, sal_Bool bRightAngledAxes, bool bRotateLights )
{
    try
    {
        if( xSceneProperties.is() )
        {
            sal_Bool bOldRightAngledAxes = sal_False;
            xSceneProperties->getPropertyValue( C2U("RightAngledAxes")) >>= bOldRightAngledAxes;
            if( bOldRightAngledAxes!=bRightAngledAxes)
            {
                xSceneProperties->setPropertyValue( C2U("RightAngledAxes"), uno::makeAny( bRightAngledAxes ));
                if( bRotateLights )
                {
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
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
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
        //remind old rotation for adaption of light directions
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
            C2U("D3DTransformMatrix"), uno::makeAny( BaseGFXHelper::B3DHomMatrixToHomogenMatrix( aSceneRotation )));

        //rotate lights if RightAngledAxes are not set or not supported
        sal_Bool bRightAngledAxes = sal_False;
        xSceneProperties->getPropertyValue( C2U("RightAngledAxes")) >>= bRightAngledAxes;
        uno::Reference< chart2::XDiagram > xDiagram( xSceneProperties, uno::UNO_QUERY );
        if(!bRightAngledAxes || !ChartTypeHelper::isSupportingRightAngledAxes(
                    DiagramHelper::getChartTypeByIndex( xDiagram, 0 ) ) )
        {
            ::basegfx::B3DHomMatrix aNewRotation;
            aNewRotation.rotate( fXAngleRad, fYAngleRad, fZAngleRad );
            lcl_rotateLights( aNewRotation*aInverseOldRotation, xSceneProperties );
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
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
        xSceneProperties->getPropertyValue( C2U( "D3DCameraGeometry" ) ) >>= aCG;
        ::basegfx::B3DVector aVRP( BaseGFXHelper::Position3DToB3DVector( aCG.vrp ) );
        fCameraDistance = aVRP.getLength();

        ensureCameraDistanceRange( fCameraDistance );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
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
        xSceneProperties->getPropertyValue( C2U( "D3DCameraGeometry" ) ) >>= aCG;
        ::basegfx::B3DVector aVRP( BaseGFXHelper::Position3DToB3DVector( aCG.vrp ) );
        if( ::basegfx::fTools::equalZero( aVRP.getLength() ) )
            aVRP = ::basegfx::B3DVector(0,0,1);
        aVRP.setLength(fCameraDistance);
        aCG.vrp = BaseGFXHelper::B3DVectorToPosition3D( aVRP );

        xSceneProperties->setPropertyValue( C2U("D3DCameraGeometry"), uno::makeAny( aCG ));
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

//static
ThreeDLookScheme ThreeDHelper::detectScheme( const uno::Reference< XDiagram >& xDiagram )
{
    ThreeDLookScheme aScheme = ThreeDLookScheme_Unknown;

    sal_Int32 nRoundedEdges;
    sal_Int32 nObjectLines;
    ThreeDHelper::getRoundedEdgesAndObjectLines( xDiagram, nRoundedEdges, nObjectLines );

    //get shade mode and light settings:
    drawing::ShadeMode aShadeMode( drawing::ShadeMode_SMOOTH );
    uno::Reference< beans::XPropertySet > xDiagramProps( xDiagram, uno::UNO_QUERY );
    try
    {
        if( xDiagramProps.is() )
            xDiagramProps->getPropertyValue( C2U( "D3DSceneShadeMode" ) )>>= aShadeMode;
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    if( lcl_isSimpleScheme( aShadeMode, nRoundedEdges, nObjectLines, xDiagram ) )
    {
        if( lcl_isSimpleLightScheme(xDiagramProps) )
            aScheme = ThreeDLookScheme_Simple;
    }
    else if( lcl_isRealisticScheme( aShadeMode, nRoundedEdges, nObjectLines ) )
    {
        if( lcl_isRealisticLightScheme(xDiagramProps) )
            aScheme = ThreeDLookScheme_Realistic;
    }

    return aScheme;
}

void ThreeDHelper::setScheme( const uno::Reference< XDiagram >& xDiagram, ThreeDLookScheme aScheme )
{
    if( aScheme == ThreeDLookScheme_Unknown )
        return;

    drawing::ShadeMode aShadeMode;
    sal_Int32 nRoundedEdges;
    sal_Int32 nObjectLines;

    if( aScheme == ThreeDLookScheme_Simple )
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
            if( ! ( (xProp->getPropertyValue( C2U( "D3DSceneShadeMode" ) )>>=aOldShadeMode) &&
                    aOldShadeMode == aShadeMode ))
            {
                xProp->setPropertyValue( C2U( "D3DSceneShadeMode" ), uno::makeAny( aShadeMode ));
            }
        }

        lcl_setLightsForScheme( xProp, aScheme );
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

}

//static
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

        ::std::vector< uno::Reference< XDataSeries > > aSeriesList(
            DiagramHelper::getDataSeriesFromDiagram( xDiagram ) );
        sal_Int32 nSeriesCount = static_cast<sal_Int32>( aSeriesList.size() );

        rtl::OUString aPercentDiagonalPropertyName( C2U( "PercentDiagonal" ) );
        rtl::OUString aBorderStylePropertyName( C2U( "BorderStyle" ) );

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
                        , aPercentDiagonalPropertyName, uno::makeAny(nPercentDiagonal) ) )
                        bDifferentRoundedEdges = true;
                }
                catch( uno::Exception& e )
                {
                    ASSERT_EXCEPTION( e );
                    bDifferentRoundedEdges = true;
                }
                try
                {
                    xProp->getPropertyValue( aBorderStylePropertyName ) >>= aLineStyle;

                    if( DataSeriesHelper::hasAttributedDataPointDifferentValue( xSeries
                        , aBorderStylePropertyName, uno::makeAny(aLineStyle) ) )
                        bDifferentObjectLines = true;
                }
                catch( uno::Exception& e )
                {
                    ASSERT_EXCEPTION( e );
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
                            , aPercentDiagonalPropertyName, uno::makeAny( static_cast< sal_Int16 >(rnRoundedEdges) ) ) )
                    {
                        bDifferentRoundedEdges = true;
                        nCurrentRoundedEdges = -1;
                    }
                }

                if( !bDifferentObjectLines )
                {
                    drawing::LineStyle aCurrentLineStyle;
                    xProp->getPropertyValue( aBorderStylePropertyName ) >>= aCurrentLineStyle;
                    if(aCurrentLineStyle!=aLineStyle
                        || DataSeriesHelper::hasAttributedDataPointDifferentValue( xSeries
                            , aBorderStylePropertyName, uno::makeAny(aLineStyle) ) )
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
    catch( uno::Exception& e )
    {
        ASSERT_EXCEPTION( e );
    }
}
//static
void ThreeDHelper::setRoundedEdgesAndObjectLines(
            const uno::Reference< XDiagram > & xDiagram
            , sal_Int32 nRoundedEdges, sal_Int32 nObjectLines )
{
    if( (nRoundedEdges<0||nRoundedEdges>100) && nObjectLines!=0 && nObjectLines!=1 )
        return;

    drawing::LineStyle aLineStyle( drawing::LineStyle_NONE );
    if(nObjectLines==1)
        aLineStyle = drawing::LineStyle_SOLID;

    uno::Any aALineStyle( uno::makeAny(aLineStyle));
    uno::Any aARoundedEdges( uno::makeAny( static_cast< sal_Int16 >( nRoundedEdges )));

    ::std::vector< uno::Reference< XDataSeries > > aSeriesList(
        DiagramHelper::getDataSeriesFromDiagram( xDiagram ) );
    sal_Int32 nSeriesCount = static_cast<sal_Int32>( aSeriesList.size() );
    for( sal_Int32 nS = 0; nS < nSeriesCount; ++nS )
    {
        uno::Reference< XDataSeries > xSeries( aSeriesList[nS] );

        if( nRoundedEdges>=0 && nRoundedEdges<=100 )
            DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints( xSeries, C2U( "PercentDiagonal" ), aARoundedEdges );

        if( nObjectLines==0 || nObjectLines==1 )
            DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints( xSeries, C2U( "BorderStyle" ), aALineStyle );
    }
}

//.............................................................................
} //namespace chart
//.............................................................................
