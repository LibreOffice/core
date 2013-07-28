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

#include "VDiagram.hxx"
#include "PropertyMapper.hxx"
#include "ViewDefines.hxx"
#include "Stripe.hxx"
#include "macros.hxx"
#include "ObjectIdentifier.hxx"
#include "DiagramHelper.hxx"
#include "BaseGFXHelper.hxx"
#include "CommonConverters.hxx"
#include "ChartTypeHelper.hxx"
#include "ThreeDHelper.hxx"
#include "defines.hxx"
#include <editeng/unoprnms.hxx>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
// header for class SvxShape
#include <svx/unoshape.hxx>
// header for class E3dScene
#include <svx/scene3d.hxx>
#include <svx/e3dsceneupdater.hxx>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

VDiagram::VDiagram(
    const uno::Reference< XDiagram > & xDiagram
    , const drawing::Direction3D& rPreferredAspectRatio
    , sal_Int32 nDimension, sal_Bool bPolar )
    : m_xLogicTarget(NULL)
    , m_xFinalTarget(NULL)
    , m_xShapeFactory(NULL)
    , m_pShapeFactory(NULL)
    , m_xOuterGroupShape(NULL)
    , m_xCoordinateRegionShape(NULL)
    , m_xWall2D(NULL)
    , m_nDimensionCount(nDimension)
    , m_bPolar(bPolar)
    , m_xDiagram(xDiagram)
    , m_aPreferredAspectRatio(rPreferredAspectRatio)
    , m_xAspectRatio3D()
    , m_fXAnglePi(0)
    , m_fYAnglePi(0)
    , m_fZAnglePi(0)
    , m_bRightAngledAxes(sal_False)
{
    if( m_nDimensionCount == 3)
    {
        uno::Reference< beans::XPropertySet > xSourceProp( m_xDiagram, uno::UNO_QUERY );
        ThreeDHelper::getRotationAngleFromDiagram( xSourceProp, m_fXAnglePi, m_fYAnglePi, m_fZAnglePi );
        if( ChartTypeHelper::isSupportingRightAngledAxes(
                DiagramHelper::getChartTypeByIndex( m_xDiagram, 0 ) ) )
        {
            if(xSourceProp.is())
                xSourceProp->getPropertyValue("RightAngledAxes") >>= m_bRightAngledAxes;
            if( m_bRightAngledAxes )
            {
                ThreeDHelper::adaptRadAnglesForRightAngledAxes( m_fXAnglePi, m_fYAnglePi );
                m_fZAnglePi=0.0;
            }
        }
    }
}

VDiagram::~VDiagram()
{
    delete m_pShapeFactory;
}

void VDiagram::init(
                const uno::Reference< drawing::XShapes >& xLogicTarget
              , const uno::Reference< drawing::XShapes >& xFinalTarget
              , const uno::Reference< lang::XMultiServiceFactory >& xFactory )
{
    OSL_PRECOND(xLogicTarget.is()&&xFinalTarget.is()&&xFactory.is(),"no proper initialization parameters");

    m_xLogicTarget  = xLogicTarget;
    m_xFinalTarget  = xFinalTarget;
    m_xShapeFactory = xFactory;
    m_pShapeFactory = new ShapeFactory(xFactory);
}

void VDiagram::createShapes( const awt::Point& rPos, const awt::Size& rSize )
{
    m_aAvailablePosIncludingAxes = rPos;
    m_aAvailableSizeIncludingAxes = rSize;

    if( m_nDimensionCount == 3 )
        createShapes_3d();
    else
        createShapes_2d();
}

::basegfx::B2IRectangle VDiagram::adjustPosAndSize( const awt::Point& rPos, const awt::Size& rSize )
{
    ::basegfx::B2IRectangle aAllowedRect( BaseGFXHelper::makeRectangle(m_aAvailablePosIncludingAxes,m_aAvailableSizeIncludingAxes) );
    ::basegfx::B2IRectangle aNewInnerRect( BaseGFXHelper::makeRectangle(rPos,rSize) );
    aNewInnerRect.intersect( aAllowedRect );

    if( m_nDimensionCount == 3 )
        aNewInnerRect = adjustPosAndSize_3d( BaseGFXHelper::B2IRectangleToAWTPoint(aNewInnerRect), BaseGFXHelper::B2IRectangleToAWTSize(aNewInnerRect) );
    else
        aNewInnerRect = adjustPosAndSize_2d( BaseGFXHelper::B2IRectangleToAWTPoint(aNewInnerRect), BaseGFXHelper::B2IRectangleToAWTSize(aNewInnerRect) );

    return aNewInnerRect;
}

::basegfx::B2IRectangle VDiagram::adjustPosAndSize_2d( const awt::Point& rPos, const awt::Size& rAvailableSize )
{
    m_aCurrentPosWithoutAxes = rPos;
    m_aCurrentSizeWithoutAxes = rAvailableSize;
    if( m_aPreferredAspectRatio.DirectionX > 0 && m_aPreferredAspectRatio.DirectionY > 0)
    {
        //do not change aspect ratio
        awt::Size  aAspectRatio( static_cast<sal_Int32>(m_aPreferredAspectRatio.DirectionX*FIXED_SIZE_FOR_3D_CHART_VOLUME),
                                 static_cast<sal_Int32>(m_aPreferredAspectRatio.DirectionY*FIXED_SIZE_FOR_3D_CHART_VOLUME ));
        m_aCurrentSizeWithoutAxes = awt::Size( ShapeFactory::calculateNewSizeRespectingAspectRatio(
                        rAvailableSize, aAspectRatio ) );
        //center diagram position
        m_aCurrentPosWithoutAxes = awt::Point( ShapeFactory::calculateTopLeftPositionToCenterObject(
            rPos, rAvailableSize, m_aCurrentSizeWithoutAxes ) );

    }

    if( m_xWall2D.is() )
    {
        m_xWall2D->setSize( m_aCurrentSizeWithoutAxes);
        m_xWall2D->setPosition(m_aCurrentPosWithoutAxes);
    }

    return ::basegfx::B2IRectangle( BaseGFXHelper::makeRectangle(m_aCurrentPosWithoutAxes,m_aCurrentSizeWithoutAxes) );
}

void VDiagram::createShapes_2d()
{
    OSL_PRECOND(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is()&&m_xShapeFactory.is(),"is not proper initialized");
    if(!(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is()&&m_xShapeFactory.is()))
        return;

    //create group shape
    uno::Reference< drawing::XShapes > xOuterGroup_Shapes = m_pShapeFactory->createGroup2D(m_xLogicTarget);
    m_xOuterGroupShape = uno::Reference<drawing::XShape>( xOuterGroup_Shapes, uno::UNO_QUERY );

    uno::Reference< drawing::XShapes > xGroupForWall( m_pShapeFactory->createGroup2D(xOuterGroup_Shapes,"PlotAreaExcludingAxes") );

    //create independent group shape as container for datapoints and such things
    {
        uno::Reference< drawing::XShapes > xShapes = m_pShapeFactory->createGroup2D(xOuterGroup_Shapes,"testonly;CooContainer=XXX_CID");
        m_xCoordinateRegionShape = uno::Reference<drawing::XShape>( xShapes, uno::UNO_QUERY );
    }

    bool bAddFloorAndWall = DiagramHelper::isSupportingFloorAndWall( m_xDiagram );

    //add back wall
    {
        m_xWall2D = uno::Reference< drawing::XShape >(
            m_xShapeFactory->createInstance(
            "com.sun.star.drawing.RectangleShape" ), uno::UNO_QUERY );

        xGroupForWall->add(m_xWall2D);
        uno::Reference< beans::XPropertySet > xProp( m_xWall2D, uno::UNO_QUERY );
        if( xProp.is())
        {
            try
            {
                OSL_ENSURE( m_xDiagram.is(), "Invalid Diagram model" );
                if( m_xDiagram.is() )
                {
                    uno::Reference< beans::XPropertySet > xWallProp( m_xDiagram->getWall());
                    if( xWallProp.is())
                        PropertyMapper::setMappedProperties( xProp, xWallProp, PropertyMapper::getPropertyNameMapForFillAndLineProperties() );
                }
                if( !bAddFloorAndWall )
                {
                    //we always need this object as dummy object for correct scene dimensions
                    //but it should not be visible in this case:
                    ShapeFactory::makeShapeInvisible( m_xWall2D );
                }
                else
                {
                    //CID for selection handling
                    OUString aWallCID( ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_DIAGRAM_WALL, OUString() ) );//@todo read CID from model
                    xProp->setPropertyValue( UNO_NAME_MISC_OBJ_NAME, uno::makeAny( aWallCID ) );
                }
            }
            catch( const uno::Exception& e )
            {
                ASSERT_EXCEPTION( e );
            }
        }

    }

    //position and size for diagram
    adjustPosAndSize_2d( m_aAvailablePosIncludingAxes, m_aAvailableSizeIncludingAxes );
}

E3dScene* lcl_getE3dScene( const uno::Reference< drawing::XShape >& xShape )
{
    E3dScene* pRet=NULL;
    uno::Reference< lang::XUnoTunnel > xUnoTunnel( xShape, uno::UNO_QUERY );
    uno::Reference< lang::XTypeProvider > xTypeProvider( xShape, uno::UNO_QUERY );
    if(xUnoTunnel.is()&&xTypeProvider.is())
    {
        SvxShape* pSvxShape = reinterpret_cast<SvxShape*>(xUnoTunnel->getSomething( SvxShape::getUnoTunnelId() ));
        if(pSvxShape)
        {
            SdrObject* pObj = pSvxShape->GetSdrObject();
            if( pObj && pObj->ISA(E3dScene) )
                pRet = (E3dScene*)pObj;
        }
    }
    return pRet;
}

void lcl_setLightSources(
    const uno::Reference< beans::XPropertySet > & xSource,
    const uno::Reference< beans::XPropertySet > & xDest )
{
    xDest->setPropertyValue( UNO_NAME_3D_SCENE_LIGHTON_1,
                             xSource->getPropertyValue( UNO_NAME_3D_SCENE_LIGHTON_1));
    xDest->setPropertyValue( UNO_NAME_3D_SCENE_LIGHTON_2,
                             xSource->getPropertyValue( UNO_NAME_3D_SCENE_LIGHTON_2));
    xDest->setPropertyValue( UNO_NAME_3D_SCENE_LIGHTON_3,
                             xSource->getPropertyValue( UNO_NAME_3D_SCENE_LIGHTON_3));
    xDest->setPropertyValue( UNO_NAME_3D_SCENE_LIGHTON_4,
                             xSource->getPropertyValue( UNO_NAME_3D_SCENE_LIGHTON_4));
    xDest->setPropertyValue( UNO_NAME_3D_SCENE_LIGHTON_5,
                             xSource->getPropertyValue( UNO_NAME_3D_SCENE_LIGHTON_5));
    xDest->setPropertyValue( UNO_NAME_3D_SCENE_LIGHTON_6,
                             xSource->getPropertyValue( UNO_NAME_3D_SCENE_LIGHTON_6));
    xDest->setPropertyValue( UNO_NAME_3D_SCENE_LIGHTON_7,
                             xSource->getPropertyValue( UNO_NAME_3D_SCENE_LIGHTON_7));
    xDest->setPropertyValue( UNO_NAME_3D_SCENE_LIGHTON_8,
                             xSource->getPropertyValue( UNO_NAME_3D_SCENE_LIGHTON_8));

    xDest->setPropertyValue( UNO_NAME_3D_SCENE_LIGHTDIRECTION_1,
                             xSource->getPropertyValue( UNO_NAME_3D_SCENE_LIGHTDIRECTION_1));
    xDest->setPropertyValue( UNO_NAME_3D_SCENE_LIGHTDIRECTION_2,
                             xSource->getPropertyValue( UNO_NAME_3D_SCENE_LIGHTDIRECTION_2));
    xDest->setPropertyValue( UNO_NAME_3D_SCENE_LIGHTDIRECTION_3,
                             xSource->getPropertyValue( UNO_NAME_3D_SCENE_LIGHTDIRECTION_3));
    xDest->setPropertyValue( UNO_NAME_3D_SCENE_LIGHTDIRECTION_4,
                             xSource->getPropertyValue( UNO_NAME_3D_SCENE_LIGHTDIRECTION_4));
    xDest->setPropertyValue( UNO_NAME_3D_SCENE_LIGHTDIRECTION_5,
                             xSource->getPropertyValue( UNO_NAME_3D_SCENE_LIGHTDIRECTION_5));
    xDest->setPropertyValue( UNO_NAME_3D_SCENE_LIGHTDIRECTION_6,
                             xSource->getPropertyValue( UNO_NAME_3D_SCENE_LIGHTDIRECTION_6));
    xDest->setPropertyValue( UNO_NAME_3D_SCENE_LIGHTDIRECTION_7,
                             xSource->getPropertyValue( UNO_NAME_3D_SCENE_LIGHTDIRECTION_7));
    xDest->setPropertyValue( UNO_NAME_3D_SCENE_LIGHTDIRECTION_8,
                             xSource->getPropertyValue( UNO_NAME_3D_SCENE_LIGHTDIRECTION_8));

    xDest->setPropertyValue( UNO_NAME_3D_SCENE_LIGHTCOLOR_1,
                             xSource->getPropertyValue( UNO_NAME_3D_SCENE_LIGHTCOLOR_1));
    xDest->setPropertyValue( UNO_NAME_3D_SCENE_LIGHTCOLOR_2,
                             xSource->getPropertyValue( UNO_NAME_3D_SCENE_LIGHTCOLOR_2));
    xDest->setPropertyValue( UNO_NAME_3D_SCENE_LIGHTCOLOR_3,
                             xSource->getPropertyValue( UNO_NAME_3D_SCENE_LIGHTCOLOR_3));
    xDest->setPropertyValue( UNO_NAME_3D_SCENE_LIGHTCOLOR_4,
                             xSource->getPropertyValue( UNO_NAME_3D_SCENE_LIGHTCOLOR_4));
    xDest->setPropertyValue( UNO_NAME_3D_SCENE_LIGHTCOLOR_5,
                             xSource->getPropertyValue( UNO_NAME_3D_SCENE_LIGHTCOLOR_5));
    xDest->setPropertyValue( UNO_NAME_3D_SCENE_LIGHTCOLOR_6,
                             xSource->getPropertyValue( UNO_NAME_3D_SCENE_LIGHTCOLOR_6));
    xDest->setPropertyValue( UNO_NAME_3D_SCENE_LIGHTCOLOR_7,
                             xSource->getPropertyValue( UNO_NAME_3D_SCENE_LIGHTCOLOR_7));
    xDest->setPropertyValue( UNO_NAME_3D_SCENE_LIGHTCOLOR_8,
                             xSource->getPropertyValue( UNO_NAME_3D_SCENE_LIGHTCOLOR_8));
}

namespace
{

void lcl_ensureScaleValue( double& rfScale )
{
    OSL_ENSURE(rfScale>0, "calculation error for automatic 3D height in chart");
    if( rfScale<0 )
        rfScale = 1.0;
    else if( rfScale<0.2 )
        rfScale = 0.2;
    else if( rfScale>5.0 )
        rfScale = 5.0;
}

}

void VDiagram::adjustAspectRatio3d( const awt::Size& rAvailableSize )
{
    OSL_PRECOND(m_xAspectRatio3D.is(), "created shape offers no XPropertySet");
    if( m_xAspectRatio3D.is())
    {
        try
        {
            double fScaleX = m_aPreferredAspectRatio.DirectionX;
            double fScaleY = m_aPreferredAspectRatio.DirectionY;
            double fScaleZ = m_aPreferredAspectRatio.DirectionZ;

            //normalize scale factors
            {
                double fMax = std::max( std::max( fScaleX, fScaleY) , fScaleZ );
                fScaleX/=fMax;
                fScaleY/=fMax;
                fScaleZ/=fMax;
            }

            if( fScaleX<0 || fScaleY<0 || fScaleZ<0 )
            {
                //calculate automatic 3D aspect ratio that fits good into the given 2D area
                double fW = rAvailableSize.Width;
                double fH = rAvailableSize.Height;

                double sx = fabs(sin(m_fXAnglePi));
                double sy = fabs(sin(m_fYAnglePi));
                double cz = fabs(cos(m_fZAnglePi));
                double sz = fabs(sin(m_fZAnglePi));

                if(m_bRightAngledAxes)
                {
                    //base equations:
                    //fH*zoomfactor == sx*fScaleZ + fScaleY;
                    //fW*zoomfactor == sy*fScaleZ + fScaleX;

                    if( fScaleX>0 && fScaleZ>0 )
                    {
                        //calculate fScaleY:
                        if( !::basegfx::fTools::equalZero(fW) )
                        {
                            fScaleY = (fH/fW)*(sy*fScaleZ+fScaleX)-(sx*fScaleZ);
                            lcl_ensureScaleValue( fScaleY );
                        }
                        else
                            fScaleY = 1.0;//looking from top or bottom the height is irrelevant
                    }
                    else if( fScaleY>0 && fScaleZ>0 )
                    {
                        //calculate fScaleX:
                        if( !::basegfx::fTools::equalZero(fH) )
                        {
                            fScaleX = (fW/fH)*(sx*fScaleZ+fScaleY)-(sy*fScaleZ);
                            lcl_ensureScaleValue(fScaleX);
                        }
                        else
                            fScaleX = 1.0;//looking from top or bottom hieght is irrelevant
                    }
                    else
                    {
                        //todo
                        OSL_FAIL("not implemented yet");

                        if( fScaleX<0 )
                            fScaleX = 1.0;
                        if( fScaleY<0 )
                            fScaleY = 1.0;
                        if( fScaleZ<0 )
                            fScaleZ = 1.0;
                    }
                }
                else
                {
                    //base equations:
                    //fH*zoomfactor == cz*fScaleY + sz*fScaleX;
                    //fW*zoomfactor == cz*fScaleX + sz*fScaleY;
                    //==>  fScaleY*(fH*sz-fW*cz) == fScaleX*(fW*sz-fH*cz);
                    if( fScaleX>0 && fScaleZ>0 )
                    {
                        //calculate fScaleY:
                        double fDivide = fH*sz-fW*cz;
                        if( !::basegfx::fTools::equalZero(fDivide) )
                        {
                            fScaleY = fScaleX*(fW*sz-fH*cz) / fDivide;
                            lcl_ensureScaleValue(fScaleY);
                        }
                        else
                            fScaleY = 1.0;//looking from top or bottom the height is irrelevant

                    }
                    else if( fScaleY>0 && fScaleZ>0 )
                    {
                        //calculate fScaleX:
                        double fDivide = fW*sz-fH*cz;
                        if( !::basegfx::fTools::equalZero(fDivide) )
                        {
                            fScaleX = fScaleY*(fH*sz-fW*cz) / fDivide;
                            lcl_ensureScaleValue(fScaleX);
                        }
                        else
                            fScaleX = 1.0;//looking from top or bottom hieght is irrelevant
                    }
                    else
                    {
                        //todo
                        OSL_FAIL("not implemented yet");

                        if( fScaleX<0 )
                            fScaleX = 1.0;
                        if( fScaleY<0 )
                            fScaleY = 1.0;
                        if( fScaleZ<0 )
                            fScaleZ = 1.0;
                    }
                }
            }

            //normalize scale factors
            {
                double fMax = std::max( std::max( fScaleX, fScaleY) , fScaleZ );
                fScaleX/=fMax;
                fScaleY/=fMax;
                fScaleZ/=fMax;
            }

            // identity matrix
            ::basegfx::B3DHomMatrix aResult;
            aResult.translate( -FIXED_SIZE_FOR_3D_CHART_VOLUME/2.0,
                            -FIXED_SIZE_FOR_3D_CHART_VOLUME/2.0,
                            -FIXED_SIZE_FOR_3D_CHART_VOLUME/2.0 );
            aResult.scale( fScaleX, fScaleY, fScaleZ );
            aResult.translate( FIXED_SIZE_FOR_3D_CHART_VOLUME/2.0,
                            FIXED_SIZE_FOR_3D_CHART_VOLUME/2.0,
                            FIXED_SIZE_FOR_3D_CHART_VOLUME/2.0 );

            // To get the 3D aspect ratio's effect on the 2D scene size, the scene's 2D size needs to be adapted to
            // 3D content changes here. The tooling class remembers the current 3D transformation stack
            // and in it's destructor, calculates a new 2D SnapRect for the scene and it's modified 3D geometry.
            E3DModifySceneSnapRectUpdater aUpdater(lcl_getE3dScene( m_xOuterGroupShape ));

            m_xAspectRatio3D->setPropertyValue( UNO_NAME_3D_TRANSFORM_MATRIX
                , uno::makeAny(BaseGFXHelper::B3DHomMatrixToHomogenMatrix( aResult )) );
        }
        catch( const uno::Exception& e )
        {
            ASSERT_EXCEPTION( e );
        }
    }
}

::basegfx::B2IRectangle VDiagram::adjustPosAndSize_3d( const awt::Point& rPos, const awt::Size& rAvailableSize )
{
    adjustAspectRatio3d( rAvailableSize );

    //do not change aspect ratio of 3D scene with 2D bound rect
    m_aCurrentSizeWithoutAxes = ShapeFactory::calculateNewSizeRespectingAspectRatio(
                    rAvailableSize, m_xOuterGroupShape->getSize() );
    m_xOuterGroupShape->setSize( m_aCurrentSizeWithoutAxes );

    //center diagram position
    m_aCurrentPosWithoutAxes= ShapeFactory::calculateTopLeftPositionToCenterObject(
         rPos, rAvailableSize, m_aCurrentSizeWithoutAxes );
    m_xOuterGroupShape->setPosition(m_aCurrentPosWithoutAxes);

    return ::basegfx::B2IRectangle( BaseGFXHelper::makeRectangle(m_aCurrentPosWithoutAxes,m_aCurrentSizeWithoutAxes) );
}

void VDiagram::createShapes_3d()
{
    OSL_PRECOND(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is()&&m_xShapeFactory.is(),"is not proper initialized");
    if(!(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is()&&m_xShapeFactory.is()))
        return;

    //create shape
    m_xOuterGroupShape = uno::Reference< drawing::XShape >(
            m_xShapeFactory->createInstance(
            "com.sun.star.drawing.Shape3DSceneObject" ), uno::UNO_QUERY );
    ShapeFactory::setShapeName( m_xOuterGroupShape, "PlotAreaExcludingAxes" );
    m_xLogicTarget->add(m_xOuterGroupShape);

    uno::Reference< drawing::XShapes > xOuterGroup_Shapes =
            uno::Reference<drawing::XShapes>( m_xOuterGroupShape, uno::UNO_QUERY );

    //create additional group to manipulate the aspect ratio of the whole diagram:
    xOuterGroup_Shapes = m_pShapeFactory->createGroup3D( xOuterGroup_Shapes, OUString() );

    m_xAspectRatio3D = uno::Reference< beans::XPropertySet >( xOuterGroup_Shapes, uno::UNO_QUERY );

    bool bAddFloorAndWall = DiagramHelper::isSupportingFloorAndWall( m_xDiagram );

    const bool bDoubleSided = false;
    const bool bFlatNormals = true;

    //add walls
    {
        uno::Reference< beans::XPropertySet > xWallProp( NULL );
        if( m_xDiagram.is() )
            xWallProp=uno::Reference< beans::XPropertySet >( m_xDiagram->getWall());

        OUString aWallCID( ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_DIAGRAM_WALL, OUString() ) );//@todo read CID from model
        if( !bAddFloorAndWall )
            aWallCID = OUString();
        uno::Reference< drawing::XShapes > xWallGroup_Shapes( m_pShapeFactory->createGroup3D( xOuterGroup_Shapes, aWallCID ) );

        CuboidPlanePosition eLeftWallPos( ThreeDHelper::getAutomaticCuboidPlanePositionForStandardLeftWall( uno::Reference< beans::XPropertySet >( m_xDiagram, uno::UNO_QUERY ) ) );
        CuboidPlanePosition eBackWallPos( ThreeDHelper::getAutomaticCuboidPlanePositionForStandardBackWall( uno::Reference< beans::XPropertySet >( m_xDiagram, uno::UNO_QUERY ) ) );

        //add left wall
        {
            short nRotatedTexture = ( CuboidPlanePosition_Front==eBackWallPos ) ? 3 : 1;
            double xPos = 0.0;
            if( CuboidPlanePosition_Right==eLeftWallPos )
                xPos = FIXED_SIZE_FOR_3D_CHART_VOLUME;
            Stripe aStripe( drawing::Position3D(xPos,FIXED_SIZE_FOR_3D_CHART_VOLUME,0)
                , drawing::Direction3D(0,0,FIXED_SIZE_FOR_3D_CHART_VOLUME)
                , drawing::Direction3D(0,-FIXED_SIZE_FOR_3D_CHART_VOLUME,0) );
            if( CuboidPlanePosition_Right==eLeftWallPos )
            {
                nRotatedTexture = ( CuboidPlanePosition_Front==eBackWallPos ) ? 2 : 0;
                aStripe = Stripe( drawing::Position3D(xPos,FIXED_SIZE_FOR_3D_CHART_VOLUME,0)
                    , drawing::Direction3D(0,-FIXED_SIZE_FOR_3D_CHART_VOLUME,0)
                    , drawing::Direction3D(0,0,FIXED_SIZE_FOR_3D_CHART_VOLUME) );
            }
            aStripe.InvertNormal(true);

            uno::Reference< drawing::XShape > xShape =
                m_pShapeFactory->createStripe( xWallGroup_Shapes, aStripe
                    , xWallProp, PropertyMapper::getPropertyNameMapForFillAndLineProperties(), bDoubleSided, nRotatedTexture, bFlatNormals );
            if( !bAddFloorAndWall )
            {
                //we always need this object as dummy object for correct scene dimensions
                //but it should not be visible in this case:
                ShapeFactory::makeShapeInvisible( xShape );
            }
        }
        //add back wall
        {
            short nRotatedTexture = 0;
            double zPos = 0.0;
            if( CuboidPlanePosition_Front==eBackWallPos )
                    zPos = FIXED_SIZE_FOR_3D_CHART_VOLUME;
            Stripe aStripe( drawing::Position3D(0,FIXED_SIZE_FOR_3D_CHART_VOLUME,zPos)
                , drawing::Direction3D(0,-FIXED_SIZE_FOR_3D_CHART_VOLUME,0)
                , drawing::Direction3D(FIXED_SIZE_FOR_3D_CHART_VOLUME,0,0) );
            if( CuboidPlanePosition_Front==eBackWallPos )
            {
                aStripe = Stripe( drawing::Position3D(0,FIXED_SIZE_FOR_3D_CHART_VOLUME,zPos)
                , drawing::Direction3D(FIXED_SIZE_FOR_3D_CHART_VOLUME,0,0)
                , drawing::Direction3D(0,-FIXED_SIZE_FOR_3D_CHART_VOLUME,0) );
                nRotatedTexture = 3;
            }
            aStripe.InvertNormal(true);

            uno::Reference< drawing::XShape > xShape =
                m_pShapeFactory->createStripe(xWallGroup_Shapes, aStripe
                    , xWallProp, PropertyMapper::getPropertyNameMapForFillAndLineProperties(), bDoubleSided, nRotatedTexture, bFlatNormals );
            if( !bAddFloorAndWall )
            {
                //we always need this object as dummy object for correct scene dimensions
                //but it should not be visible in this case:
                ShapeFactory::makeShapeInvisible( xShape );
            }
        }
    }

    try
    {
        uno::Reference< beans::XPropertySet > xSourceProp( m_xDiagram, uno::UNO_QUERY_THROW );
        uno::Reference< beans::XPropertySet > xDestProp( m_xOuterGroupShape, uno::UNO_QUERY_THROW );

        //perspective
        {
            //ignore distance and focal length from file format and model comcpletely
            //use vrp only to indicate the distance of the camera and thus influence the perspecitve
            xDestProp->setPropertyValue( UNO_NAME_3D_SCENE_DISTANCE, uno::makeAny(
                                        static_cast<sal_Int32>(ThreeDHelper::getCameraDistance( xSourceProp ))));
            xDestProp->setPropertyValue( UNO_NAME_3D_SCENE_PERSPECTIVE,
                                        xSourceProp->getPropertyValue( UNO_NAME_3D_SCENE_PERSPECTIVE));
        }

        //light
        {
            xDestProp->setPropertyValue( UNO_NAME_3D_SCENE_SHADE_MODE,
                                        xSourceProp->getPropertyValue( UNO_NAME_3D_SCENE_SHADE_MODE));
            xDestProp->setPropertyValue( UNO_NAME_3D_SCENE_AMBIENTCOLOR,
                                        xSourceProp->getPropertyValue( UNO_NAME_3D_SCENE_AMBIENTCOLOR));
            xDestProp->setPropertyValue( UNO_NAME_3D_SCENE_TWO_SIDED_LIGHTING,
                                        xSourceProp->getPropertyValue( UNO_NAME_3D_SCENE_TWO_SIDED_LIGHTING));
            lcl_setLightSources( xSourceProp, xDestProp );
        }

        //rotation
        {
            //set diagrams rotation is set exclusively vie the transformation matrix
            //don't set a camera at all!
            //the cameras rotation is incorporated into this matrix

            ::basegfx::B3DHomMatrix aEffectiveTranformation;
            aEffectiveTranformation.translate(-FIXED_SIZE_FOR_3D_CHART_VOLUME/2.0, -FIXED_SIZE_FOR_3D_CHART_VOLUME/2.0, -FIXED_SIZE_FOR_3D_CHART_VOLUME/2.0);

            if(!m_bRightAngledAxes)
                aEffectiveTranformation.rotate(m_fXAnglePi,m_fYAnglePi,m_fZAnglePi);
            else
                aEffectiveTranformation.shearXY(m_fYAnglePi,-m_fXAnglePi);

            //#i98497# 3D charts are rendered with wrong size
            E3DModifySceneSnapRectUpdater aUpdater(lcl_getE3dScene( m_xOuterGroupShape ));
            xDestProp->setPropertyValue( UNO_NAME_3D_TRANSFORM_MATRIX,
                    uno::makeAny( BaseGFXHelper::B3DHomMatrixToHomogenMatrix( aEffectiveTranformation ) ) );
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    //add floor plate
    {
        uno::Reference< beans::XPropertySet > xFloorProp( NULL );
        if( m_xDiagram.is() )
            xFloorProp=uno::Reference< beans::XPropertySet >( m_xDiagram->getFloor());

        Stripe aStripe( drawing::Position3D(0,0,0)
            , drawing::Direction3D(0,0,FIXED_SIZE_FOR_3D_CHART_VOLUME)
            , drawing::Direction3D(FIXED_SIZE_FOR_3D_CHART_VOLUME,0,0) );
        aStripe.InvertNormal(true);

        uno::Reference< drawing::XShape > xShape =
            m_pShapeFactory->createStripe(xOuterGroup_Shapes, aStripe
                , xFloorProp, PropertyMapper::getPropertyNameMapForFillAndLineProperties(), bDoubleSided, 0, bFlatNormals );

        CuboidPlanePosition eBottomPos( ThreeDHelper::getAutomaticCuboidPlanePositionForStandardBottom( uno::Reference< beans::XPropertySet >( m_xDiagram, uno::UNO_QUERY ) ) );
        if( !bAddFloorAndWall || (CuboidPlanePosition_Bottom!=eBottomPos) )
        {
            //we always need this object as dummy object for correct scene dimensions
            //but it should not be visible in this case:
            ShapeFactory::makeShapeInvisible( xShape );
        }
        else
        {
            OUString aFloorCID( ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_DIAGRAM_FLOOR, OUString() ) );//@todo read CID from model
            ShapeFactory::setShapeName( xShape, aFloorCID );
        }
    }

    //create an additional scene for the smaller inner coordinate region:
    {
        uno::Reference< drawing::XShapes > xShapes = m_pShapeFactory->createGroup3D( xOuterGroup_Shapes,"testonly;CooContainer=XXX_CID" );
        m_xCoordinateRegionShape = uno::Reference< drawing::XShape >( xShapes, uno::UNO_QUERY );

        uno::Reference< beans::XPropertySet > xShapeProp( m_xCoordinateRegionShape, uno::UNO_QUERY );
        OSL_ENSURE(xShapeProp.is(), "created shape offers no XPropertySet");
        if( xShapeProp.is())
        {
            try
            {
                double fXScale = (FIXED_SIZE_FOR_3D_CHART_VOLUME -GRID_TO_WALL_DISTANCE) /FIXED_SIZE_FOR_3D_CHART_VOLUME;
                double fYScale = (FIXED_SIZE_FOR_3D_CHART_VOLUME -GRID_TO_WALL_DISTANCE) /FIXED_SIZE_FOR_3D_CHART_VOLUME;
                double fZScale = (FIXED_SIZE_FOR_3D_CHART_VOLUME -GRID_TO_WALL_DISTANCE) /FIXED_SIZE_FOR_3D_CHART_VOLUME;

                ::basegfx::B3DHomMatrix aM;
                aM.translate(GRID_TO_WALL_DISTANCE/fXScale, GRID_TO_WALL_DISTANCE/fYScale, GRID_TO_WALL_DISTANCE/fZScale);
                aM.scale( fXScale, fYScale, fZScale );
                E3DModifySceneSnapRectUpdater aUpdater(lcl_getE3dScene( m_xOuterGroupShape ));
                xShapeProp->setPropertyValue( UNO_NAME_3D_TRANSFORM_MATRIX
                    , uno::makeAny(BaseGFXHelper::B3DHomMatrixToHomogenMatrix(aM)) );
            }
            catch( const uno::Exception& e )
            {
                ASSERT_EXCEPTION( e );
            }
        }
    }

    m_aCurrentPosWithoutAxes = m_aAvailablePosIncludingAxes;
    m_aCurrentSizeWithoutAxes = m_aAvailableSizeIncludingAxes;
    adjustPosAndSize_3d( m_aAvailablePosIncludingAxes, m_aAvailableSizeIncludingAxes );
}

uno::Reference< drawing::XShapes > VDiagram::getCoordinateRegion()
{
    return uno::Reference<drawing::XShapes>( m_xCoordinateRegionShape, uno::UNO_QUERY );
}

::basegfx::B2IRectangle VDiagram::getCurrentRectangle()
{
    return BaseGFXHelper::makeRectangle(m_aCurrentPosWithoutAxes,m_aCurrentSizeWithoutAxes);
}

void VDiagram::reduceToMimimumSize()
{
    if( m_xOuterGroupShape.is() )
    {
        awt::Size aMaxSize( m_aAvailableSizeIncludingAxes );
        awt::Point aMaxPos( m_aAvailablePosIncludingAxes );

        sal_Int32 nNewWidth = aMaxSize.Width/3;
        sal_Int32 nNewHeight = aMaxSize.Height/3;
        awt::Size aNewSize( nNewWidth, nNewHeight );
        awt::Point aNewPos( aMaxPos );
        aNewPos.X += nNewWidth;
        aNewPos.Y += nNewHeight;

        adjustPosAndSize( aNewPos, aNewSize );
    }
}

::basegfx::B2IRectangle VDiagram::adjustInnerSize( const ::basegfx::B2IRectangle& rConsumedOuterRect )
{
    awt::Point aNewPos( m_aCurrentPosWithoutAxes );
    awt::Size aNewSize( m_aCurrentSizeWithoutAxes );

    ::basegfx::B2IRectangle rAvailableOuterRect(
        BaseGFXHelper::makeRectangle(m_aAvailablePosIncludingAxes,m_aAvailableSizeIncludingAxes) );

    sal_Int32 nDeltaWidth = static_cast<sal_Int32>(rAvailableOuterRect.getWidth() - rConsumedOuterRect.getWidth());
    sal_Int32 nDeltaHeight = static_cast<sal_Int32>(rAvailableOuterRect.getHeight() - rConsumedOuterRect.getHeight());
    if( (aNewSize.Width + nDeltaWidth) < rAvailableOuterRect.getWidth()/3 )
        nDeltaWidth = static_cast<sal_Int32>(rAvailableOuterRect.getWidth()/3 - aNewSize.Width);
    aNewSize.Width += nDeltaWidth;

    if( (aNewSize.Height + nDeltaHeight) < rAvailableOuterRect.getHeight()/3 )
        nDeltaHeight = static_cast<sal_Int32>(rAvailableOuterRect.getHeight()/3 - aNewSize.Height);
    aNewSize.Height += nDeltaHeight;

    sal_Int32 nDiffLeft = rConsumedOuterRect.getMinX() - rAvailableOuterRect.getMinX();
    sal_Int32 nDiffRight = rAvailableOuterRect.getMaxX() - rConsumedOuterRect.getMaxX();
    if( nDiffLeft >= 0 )
        aNewPos.X -= nDiffLeft;
    else if( nDiffRight >= 0 )
    {
        if( nDiffRight > -nDiffLeft )
            aNewPos.X += abs(nDiffLeft);
        else if( nDiffRight > abs(nDeltaWidth) )
            aNewPos.X += nDiffRight;
        else
            aNewPos.X += abs(nDeltaWidth);
    }

    sal_Int32 nDiffUp = rConsumedOuterRect.getMinY() - rAvailableOuterRect.getMinY();
    sal_Int32 nDiffDown = rAvailableOuterRect.getMaxY() - rConsumedOuterRect.getMaxY();
    if( nDiffUp >= 0 )
        aNewPos.Y -= nDiffUp;
    else if( nDiffDown >= 0 )
    {
        if( nDiffDown > -nDiffUp )
            aNewPos.Y += abs(nDiffUp);
        else if( nDiffDown > abs(nDeltaHeight) )
            aNewPos.Y += nDiffDown;
        else
            aNewPos.Y += abs(nDeltaHeight);
    }

    return adjustPosAndSize( aNewPos, aNewSize );
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
