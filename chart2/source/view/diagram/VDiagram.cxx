/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VDiagram.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 15:34:49 $
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
#include "VDiagram.hxx"
//#include "DebugHelper.hxx"
#include "PropertyMapper.hxx"
#include "CommonConverters.hxx"
#include "ViewDefines.hxx"
#include "Stripe.hxx"
#include "macros.hxx"
#include "Rotation.hxx"

#ifndef _SVX_UNOPRNMS_HXX
#include <svx/unoprnms.hxx>
#endif

#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_DRAWING_PROJECTIONMODE_HPP_
#include <com/sun/star/drawing/ProjectionMode.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_SHADEMODE_HPP_
#include <com/sun/star/drawing/ShadeMode.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_SCENEDESCRIPTOR_HPP_
#include <com/sun/star/chart2/SceneDescriptor.hpp>
#endif


#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
// header for class SvxShape
#ifndef _SVX_UNOSHAPE_HXX
#include <svx/unoshape.hxx>
#endif
// header for class E3dScene
#ifndef _E3D_SCENE3D_HXX
#include <svx/scene3d.hxx>
#endif
#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

VDiagram::VDiagram(
    const uno::Reference< XDiagram > & xDiagram,
    sal_Int32 nDimension, sal_Bool bPolar )
    : m_xLogicTarget(NULL)
    , m_xFinalTarget(NULL)
    , m_xShapeFactory(NULL)
    , m_pShapeFactory(NULL)
    , m_xOuterGroupShape(NULL)
    , m_xCoordinateRegionShape(NULL)
    , m_nDimension(nDimension)
    , m_bPolar(bPolar)
    , m_xDiagram(xDiagram)
{
}

VDiagram::~VDiagram()
{
    delete m_pShapeFactory;
}

void SAL_CALL VDiagram::init(
                const uno::Reference< drawing::XShapes >& xLogicTarget
              , const uno::Reference< drawing::XShapes >& xFinalTarget
              , const uno::Reference< lang::XMultiServiceFactory >& xFactory )
{
    DBG_ASSERT(xLogicTarget.is()&&xFinalTarget.is()&&xFactory.is(),"no proper initialization parameters");

    m_xLogicTarget  = xLogicTarget;
    m_xFinalTarget  = xFinalTarget;
    m_xShapeFactory = xFactory;
    m_pShapeFactory = new ShapeFactory(xFactory);
}

void VDiagram::createShapes( const awt::Point& rPos, const awt::Size& rSize )
{
    if( m_nDimension == 3 )
        createShapes_3d( rPos, rSize );
    else
        createShapes_2d( rPos, rSize );
}

namespace
{
tPropertyNameMap & lcl_GetWallPropertyMap()
{
    static tPropertyNameMap aWallPropertyMap(
        tMakePropertyNameMap
        ( C2U( "FillStyle" ),         C2U( "FillStyle" ) )
        ( C2U( "FillColor" ),         C2U( "FillColor" ) )
        ( C2U( "FillTransparence" ),  C2U( "FillTransparence" ) )
        ( C2U( "FillGradient" ),      C2U( "FillGradient" ) )
        ( C2U( "FillHatch" ),         C2U( "FillHatch" ) )
        ( C2U( "LineStyle" ),         C2U( "LineStyle" ) )
        ( C2U( "LineWidth" ),         C2U( "LineWidth" ) )
        ( C2U( "LineDash" ),          C2U( "LineDash" ) )
        ( C2U( "LineColor" ),         C2U( "LineColor" ) )
        ( C2U( "LineTransparence" ),  C2U( "LineTransparence" ) )
        ( C2U( "LineJoint" ),         C2U( "LineJoint" ) )
        );

    return aWallPropertyMap;
};
} // anonymous namespace

void VDiagram::createShapes_2d( const awt::Point& rPos, const awt::Size& rSize )
{
    DBG_ASSERT(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is()&&m_xShapeFactory.is(),"is not proper initialized");
    if(!(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is()&&m_xShapeFactory.is()))
        return;

    //create group shape
    uno::Reference< drawing::XShapes > xOuterGroup_Shapes = m_pShapeFactory->createGroup2D(m_xLogicTarget,C2U("CID/Diagram=XXX_CID"));//@todo read CID from model
    m_xOuterGroupShape = uno::Reference<drawing::XShape>( xOuterGroup_Shapes, uno::UNO_QUERY );

    //---------------------------
    //add back wall
    {
        uno::Reference< drawing::XShape > xShape2D(
            m_xShapeFactory->createInstance( C2U(
            "com.sun.star.drawing.RectangleShape" ) ), uno::UNO_QUERY );
        xShape2D->setPosition(awt::Point(0,0));
        xShape2D->setSize(awt::Size(
                              static_cast< sal_Int32 >( FIXED_SIZE_FOR_3D_CHART_VOLUME ),
                              static_cast< sal_Int32 >( FIXED_SIZE_FOR_3D_CHART_VOLUME )));
        xOuterGroup_Shapes->add(xShape2D);
        uno::Reference< beans::XPropertySet > xProp( xShape2D, uno::UNO_QUERY );
        if( xProp.is())
        {
            try
            {
                DBG_ASSERT( m_xDiagram.is(), "Invalid Diagram model" );
                if( m_xDiagram.is() )
                {
                    uno::Reference< beans::XPropertySet > xWallProp( m_xDiagram->getWall());
                    if( xWallProp.is())
                        PropertyMapper::setMappedProperties( xProp, xWallProp, lcl_GetWallPropertyMap() );
                }
                //CID for selection handling
                xProp->setPropertyValue( C2U( UNO_NAME_MISC_OBJ_NAME )
                    , uno::makeAny( C2U("CID/DiagramWall=XXX_CID") ) );
            }
            catch( uno::Exception& e )
            {
                ASSERT_EXCEPTION( e );
            }
        }

    }
    //---------------------------

    m_xOuterGroupShape->setPosition(rPos);
    m_xOuterGroupShape->setSize(rSize);

    //create independent group shape as container for datapoints and such things
    {
        uno::Reference< drawing::XShapes > xShapes = m_pShapeFactory->createGroup2D(m_xLogicTarget,C2U("testonly;CooContainer=XXX_CID"));
        m_xCoordinateRegionShape = uno::Reference<drawing::XShape>( xShapes, uno::UNO_QUERY );
    }
}

SceneDescriptor lcl_getDefaultSceneDescriptor()
{
    DBG_ERROR("not implemented yet");
    SceneDescriptor aSceneDescriptor;
    return aSceneDescriptor;
}

drawing::HomogenMatrix lcl_getTransformationMatrixForOuterScene( const uno::Reference< XDiagram >& xDiagram )
{
    //because of a bug in the drawing layer the matrix at the outer scene does only affect rotation

    SceneDescriptor aSceneDescriptor;
    uno::Reference< beans::XPropertySet > xProp( xDiagram, uno::UNO_QUERY );
    if( !xProp.is() || !(xProp->getPropertyValue( C2U( "SceneProperties" ) ) >>= aSceneDescriptor) )
        aSceneDescriptor = lcl_getDefaultSceneDescriptor();

    ::basegfx::B3DHomMatrix aTranslateM4;
    aTranslateM4.translate(-FIXED_SIZE_FOR_3D_CHART_VOLUME/2.0, -FIXED_SIZE_FOR_3D_CHART_VOLUME/2.0, 0);

    ::basegfx::B3DHomMatrix aRotateM4 = Rotation::getRotationMatrixFromAxisAngleRepresentation( aSceneDescriptor.aDirection, aSceneDescriptor.fRotationAngle );

    ::basegfx::B3DHomMatrix aM4 = aRotateM4*aTranslateM4;

    //@todo remove this test:
    double fTestAngle=1.0;
    drawing::Direction3D aTestDirection(1,1,1);
    Rotation::getRotationAxisAngleFromMatrixRepresentation( aTestDirection, fTestAngle, aRotateM4 );
    /*
    Matrix4D aM4;
    aM4.RotateY( -ZDIRECTION*F_PI/9.0 );
    aM4.RotateX( ZDIRECTION*F_PI/10.0 );
    */

    //aM4.RotateY( ZDIRECTION*F_PI/2.0 );
    //aM4.RotateX( -ZDIRECTION*F_PI/2.0 );
//    aM4.RotateY( -ZDIRECTION*F_PI/2.0 );

    //Matrix4D aTest;
    //aTest.RotateY( aSceneDescriptor.fRotationAngle*F_PI/180.0 );
    //aM4.RotateX( -ZDIRECTION*F_PI/2.0 );

//  aM4.Translate(0, -FIXED_SIZE_FOR_3D_CHART_VOLUME/2.0, 0);
//  aM4.Scale(1, 1, ZDIRECTION); //invert direction of z coordinate to get a left handed system
//        aM4.Scale(1.0, 1.0, 1.0/50.0);
    return Matrix4DToHomogenMatrix(aM4);
}

drawing::HomogenMatrix lcl_getTransformationMatrixForInnerScene( const uno::Reference< XDiagram >& xDiagram )
{
    SceneDescriptor aSceneDescriptor;
    uno::Reference< beans::XPropertySet > xProp( xDiagram, uno::UNO_QUERY );
    if( !xProp.is() || !(xProp->getPropertyValue( C2U( "SceneProperties" ) ) >>= aSceneDescriptor) )
        aSceneDescriptor = lcl_getDefaultSceneDescriptor();

    ::basegfx::B3DHomMatrix aM4;
    double CatsFactor = 3.0;//@todo needs to be calculated from seriescount and slot distance ...
    aM4.scale(1.0, aSceneDescriptor.fRelativeHeight, aSceneDescriptor.fRelativeDepth/CatsFactor);
    return Matrix4DToHomogenMatrix(aM4);
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

/*
//default is Light 1 on, but we want to use the Light 2 as in the old chart (Light 2 is not specular by default)
xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_LIGHTON_1 ), uno::makeAny((sal_Bool)false) );
xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_LIGHTON_2 ), uno::makeAny((sal_Bool)true) );

//(204,204,204);
uno::Any aAnyLightColor_1     = xProp->getPropertyValue( C2U( UNO_NAME_3D_SCENE_LIGHTCOLOR_1 ) );
uno::Any aAnyLightDirection_1 = xProp->getPropertyValue( C2U( UNO_NAME_3D_SCENE_LIGHTDIRECTION_1 ) );

sal_Int32 nLightColor_1; aAnyLightColor_1 >>= nLightColor_1;
drawing::Direction3D aLightDirection_1; aAnyLightDirection_1 >>= aLightDirection_1;

xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_LIGHTCOLOR_2 ), aAnyLightColor_1 );
xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_LIGHTDIRECTION_2 ), uno::makeAny(drawing::Direction3D(1,1,1)) );
*/

void lcl_setLightSource( const LightSource& rLightSource, sal_Int32 nNumber, const uno::Reference< beans::XPropertySet >& xProp )
{
    if( nNumber >= 8 )
        return;
    if(0==nNumber)
    {
        xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_LIGHTON_1 ), uno::makeAny((sal_Bool)true) );
        xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_LIGHTCOLOR_1 ), uno::makeAny(rLightSource.nDiffuseColor) );
        xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_LIGHTDIRECTION_1 ), uno::makeAny(rLightSource.aDirection) );
    }
    else if(1==nNumber)
    {
        xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_LIGHTON_2 ), uno::makeAny((sal_Bool)true) );
        xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_LIGHTCOLOR_2 ), uno::makeAny(rLightSource.nDiffuseColor) );
        xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_LIGHTDIRECTION_2 ), uno::makeAny(rLightSource.aDirection) );
    }
    else if(2==nNumber)
    {
        xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_LIGHTON_3 ), uno::makeAny((sal_Bool)true) );
        xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_LIGHTCOLOR_3 ), uno::makeAny(rLightSource.nDiffuseColor) );
        xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_LIGHTDIRECTION_3 ), uno::makeAny(rLightSource.aDirection) );
    }
    else if(3==nNumber)
    {
        xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_LIGHTON_4 ), uno::makeAny((sal_Bool)true) );
        xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_LIGHTCOLOR_4 ), uno::makeAny(rLightSource.nDiffuseColor) );
        xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_LIGHTDIRECTION_4 ), uno::makeAny(rLightSource.aDirection) );
    }
    else if(4==nNumber)
    {
        xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_LIGHTON_5 ), uno::makeAny((sal_Bool)true) );
        xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_LIGHTCOLOR_5 ), uno::makeAny(rLightSource.nDiffuseColor) );
        xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_LIGHTDIRECTION_5 ), uno::makeAny(rLightSource.aDirection) );
    }
    else if(5==nNumber)
    {
        xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_LIGHTON_6 ), uno::makeAny((sal_Bool)true) );
        xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_LIGHTCOLOR_6 ), uno::makeAny(rLightSource.nDiffuseColor) );
        xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_LIGHTDIRECTION_6 ), uno::makeAny(rLightSource.aDirection) );
    }
    else if(6==nNumber)
    {
        xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_LIGHTON_7 ), uno::makeAny((sal_Bool)true) );
        xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_LIGHTCOLOR_7 ), uno::makeAny(rLightSource.nDiffuseColor) );
        xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_LIGHTDIRECTION_7 ), uno::makeAny(rLightSource.aDirection) );
    }
    else if(7==nNumber)
    {
        xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_LIGHTON_8 ), uno::makeAny((sal_Bool)true) );
        xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_LIGHTCOLOR_8 ), uno::makeAny(rLightSource.nDiffuseColor) );
        xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_LIGHTDIRECTION_8 ), uno::makeAny(rLightSource.aDirection) );
    }
}

void lcl_setScenePropertiesExceptTransformation( const SceneDescriptor& rSceneDescriptor, const uno::Reference< beans::XPropertySet >& xProp )
{
    xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_AMBIENTCOLOR ), uno::makeAny( rSceneDescriptor.nAmbientLightColor )); //UNO_NAME_3D_SCENE_AMBIENTCOLOR sal_Int32
    xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_SHADE_MODE ), uno::makeAny( rSceneDescriptor.aShadeMode )); // drawing::ShadeMode_FLAT FLAT, PHONG, SMOOTH, DRAFT

    for( sal_Int32 nL=0; nL<rSceneDescriptor.aLightSources.getLength();nL++)
    {
        const LightSource& rLightSource = rSceneDescriptor.aLightSources[nL];
        lcl_setLightSource( rLightSource, nL, xProp );
    }
}

void VDiagram::createShapes_3d( const awt::Point& rPos, const awt::Size& rReservedSize )
{
    DBG_ASSERT(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is()&&m_xShapeFactory.is(),"is not proper initialized");
    if(!(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is()&&m_xShapeFactory.is()))
        return;

    //create shape
    m_xOuterGroupShape = uno::Reference< drawing::XShape >(
            m_xShapeFactory->createInstance( C2U(
            "com.sun.star.drawing.Shape3DSceneObject" ) ), uno::UNO_QUERY );
    m_xLogicTarget->add(m_xOuterGroupShape);
    ShapeFactory::setShapeName( m_xOuterGroupShape, C2U("MarkHandles") );

    //set properties
    uno::Reference< beans::XPropertySet > xProp( m_xOuterGroupShape, uno::UNO_QUERY );
    DBG_ASSERT(xProp.is(), "created shape offers no XPropertySet");
    if( xProp.is())
    {
        sal_Int32 nDistance = (sal_Int32)FIXED_SIZE_FOR_3D_CHART_VOLUME*8; //(FIXED_SIZE_FOR_3D_CHART_VOLUME * 4) / 6
        sal_Int32 nFocalLen = nDistance*3;

        try
        {
            //set TransformationMatrix
            xProp->setPropertyValue( C2U( UNO_NAME_3D_TRANSFORM_MATRIX ), uno::makeAny(lcl_getTransformationMatrixForOuterScene(m_xDiagram)) );
            //uno::Any aCG = createDefaultCameraGeometry();
            //xProp->setPropertyValue( C2U( UNO_NAME_3D_CAMERA_GEOMETRY ), aCG );
            xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_DISTANCE ), uno::makeAny(nDistance) );
            xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_FOCAL_LENGTH ), uno::makeAny(nFocalLen) );
            xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_PERSPECTIVE ), uno::makeAny(drawing::ProjectionMode_PERSPECTIVE) ); //ProjectionMode_PARALLEL

            SceneDescriptor aSceneDescriptor;
            uno::Reference< beans::XPropertySet > xModelDiaProp( m_xDiagram, uno::UNO_QUERY );
            if( !xModelDiaProp.is() || !( xModelDiaProp->getPropertyValue( C2U( "SceneProperties" )) >>= aSceneDescriptor ))
                aSceneDescriptor = lcl_getDefaultSceneDescriptor();
            lcl_setScenePropertiesExceptTransformation( aSceneDescriptor, xProp );

            //D3DSceneTwoSidedLighting
            xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_TWO_SIDED_LIGHTING )
                , uno::makeAny( (sal_Bool)true) );

            //UNO_NAME_3D_SCENE_DISTANCE D3DSceneDistance sal_Int32
            //UNO_NAME_3D_SCENE_FOCAL_LENGTH sal_Int32
            //UNO_NAME_3D_SCENE_PERSPECTIVE drawing::ProjectionMode

            //UNO_NAME_3D_SCENE_LIGHTCOLOR_1 sal_Int32
            //UNO_NAME_3D_SCENE_LIGHTDIRECTION_1 drawing::Direction3D
            //UNO_NAME_3D_SCENE_LIGHTON_1 Bool
            //UNO_NAME_3D_SCENE_SHADOW_SLANT sal_Int16
            //UNO_NAME_3D_SCENE_SHADE_MODE drawing::ShadeMode
            //UNO_NAME_3D_SCENE_TWO_SIDED_LIGHTING Bool

            //CID for selection handling
            xProp->setPropertyValue( C2U( UNO_NAME_MISC_OBJ_NAME )
                , uno::makeAny( C2U("CID/Diagram=XXX_CID") ) );
        }
        catch( uno::Exception& e )
        {
            ASSERT_EXCEPTION( e );
        }
    }

    uno::Reference< drawing::XShapes > xOuterGroup_Shapes =
            uno::Reference<drawing::XShapes>( m_xOuterGroupShape, uno::UNO_QUERY );


    xOuterGroup_Shapes = m_pShapeFactory->createGroup3D( xOuterGroup_Shapes, rtl::OUString() );

    //-------------------------------------------------------------------------
    //create additional group to manipulate the aspect ratio of the whole diagram:
    {
        xProp.set( xOuterGroup_Shapes, uno::UNO_QUERY );
        DBG_ASSERT(xProp.is(), "created shape offers no XPropertySet");
        if( xProp.is())
        {
            try
            {
                xProp->setPropertyValue( C2U( UNO_NAME_3D_TRANSFORM_MATRIX ), uno::makeAny(lcl_getTransformationMatrixForInnerScene(m_xDiagram)) );
            }
            catch( uno::Exception& e )
            {
                e;
            }
        }
    }

    //---------------------------

    //add floor plate
    {
        uno::Reference< beans::XPropertySet > xFloorProp( NULL );
        if( m_xDiagram.is() )
            xFloorProp=uno::Reference< beans::XPropertySet >( m_xDiagram->getFloor());

        uno::Reference< drawing::XShape > xShape =
            m_pShapeFactory->createCube(xOuterGroup_Shapes,
            DataPointGeometry( drawing::Position3D(FIXED_SIZE_FOR_3D_CHART_VOLUME/2.0,0,FIXED_SIZE_FOR_3D_CHART_VOLUME/2.0)
                              ,drawing::Direction3D(FIXED_SIZE_FOR_3D_CHART_VOLUME,FLOOR_THICKNESS,FIXED_SIZE_FOR_3D_CHART_VOLUME) )
            , xFloorProp, lcl_GetWallPropertyMap() );
        ShapeFactory::setShapeName( xShape, C2U("CID/DiagramFloor=XXX_CID") );
    }

    uno::Reference< beans::XPropertySet > xWallProp( NULL );
    if( m_xDiagram.is() )
        xWallProp=uno::Reference< beans::XPropertySet >( m_xDiagram->getWall());

    uno::Reference< drawing::XShapes > xWallGroup_Shapes( m_pShapeFactory->createGroup3D( xOuterGroup_Shapes, C2U("CID/DiagramWall=XXX_CID") ) );
    //add left wall
    {
        uno::Reference< drawing::XShape > xShape =
            m_pShapeFactory->createStripe(xWallGroup_Shapes
            , Stripe(
                  drawing::Position3D(0,0,FIXED_SIZE_FOR_3D_CHART_VOLUME)
                , drawing::Direction3D(0,0,-FIXED_SIZE_FOR_3D_CHART_VOLUME)
                , drawing::Direction3D(0,FIXED_SIZE_FOR_3D_CHART_VOLUME,0) )
            , xWallProp, lcl_GetWallPropertyMap(), false );
    }
    //add back wall
    {
        uno::Reference< drawing::XShape > xShape =
            m_pShapeFactory->createStripe(xWallGroup_Shapes
            , Stripe(
                  drawing::Position3D(0,0,0)
                , drawing::Direction3D(FIXED_SIZE_FOR_3D_CHART_VOLUME,0,0)
                , drawing::Direction3D(0,FIXED_SIZE_FOR_3D_CHART_VOLUME,0) )
            , xWallProp, lcl_GetWallPropertyMap(), false );
    }
    //---------------------------

    //create an additional scene for the smaller inner coordinate region:
    {
        uno::Reference< drawing::XShapes > xShapes = m_pShapeFactory->createGroup3D( xOuterGroup_Shapes,C2U("testonly;CooContainer=XXX_CID") );
        m_xCoordinateRegionShape = uno::Reference< drawing::XShape >( xShapes, uno::UNO_QUERY );

        uno::Reference< beans::XPropertySet > xShapeProp( m_xCoordinateRegionShape, uno::UNO_QUERY );
        DBG_ASSERT(xShapeProp.is(), "created shape offers no XPropertySet");
        if( xShapeProp.is())
        {
            try
            {
                double fXScale = (FIXED_SIZE_FOR_3D_CHART_VOLUME -GRID_TO_WALL_DISTANCE) /FIXED_SIZE_FOR_3D_CHART_VOLUME;
                double fYScale = (FIXED_SIZE_FOR_3D_CHART_VOLUME -FLOOR_THICKNESS      ) /FIXED_SIZE_FOR_3D_CHART_VOLUME;
                double fZScale = (FIXED_SIZE_FOR_3D_CHART_VOLUME -GRID_TO_WALL_DISTANCE) /FIXED_SIZE_FOR_3D_CHART_VOLUME;

                ::basegfx::B3DHomMatrix aM4;
                aM4.translate(GRID_TO_WALL_DISTANCE/fXScale, FLOOR_THICKNESS/fYScale, GRID_TO_WALL_DISTANCE/fZScale);
                aM4.scale( fXScale, fYScale, fZScale );
                xShapeProp->setPropertyValue( C2U( UNO_NAME_3D_TRANSFORM_MATRIX )
                    , uno::makeAny(Matrix4DToHomogenMatrix(aM4)) );
            }
            catch( uno::Exception& e )
            {
                ASSERT_EXCEPTION( e );
            }
        }
    }

    //calculate correct 2d dimensions for getting a correct initial 2D aspect ratio
    E3dScene* pScene = lcl_getE3dScene( m_xOuterGroupShape );
    pScene->CorrectSceneDimensions();

    //do not change aspect ratio of 3D scene with 2D bound rect
    awt::Size aDiagramSize;
    {
        awt::Size aAspectRatioSize = m_xOuterGroupShape->getSize();
        double fMax = std::max(rReservedSize.Width,rReservedSize.Height);
        double fFactorWidth = double(rReservedSize.Width)/double(aAspectRatioSize.Width);
        double fFactorHeight = double(rReservedSize.Height)/double(aAspectRatioSize.Height);
        double fFactor = std::min(fFactorWidth,fFactorHeight);
        aDiagramSize.Width=static_cast<sal_Int32>(fFactor*aAspectRatioSize.Width);
        aDiagramSize.Height=static_cast<sal_Int32>(fFactor*aAspectRatioSize.Height);
        m_xOuterGroupShape->setSize(aDiagramSize);
    }
    //center diagram position
    {
        awt::Point aNewPosition(rPos);
        aNewPosition.X += static_cast<sal_Int32>(double(rReservedSize.Width-aDiagramSize.Width)/2.0);
        aNewPosition.Y += static_cast<sal_Int32>(double(rReservedSize.Height-aDiagramSize.Height)/2.0);
        m_xOuterGroupShape->setPosition(aNewPosition);
    }
}

uno::Reference< drawing::XShapes > VDiagram::getCoordinateRegion()
{
    return uno::Reference<drawing::XShapes>( m_xCoordinateRegionShape, uno::UNO_QUERY );
}




/*
uno::Any createDefaultCameraGeometry( )
{
    long    nX      = 0;
    long    nY      = 0;
    long    nW      = (long)FIXED_SIZE_FOR_3D_CHART_VOLUME;
    long    nH      = (long)FIXED_SIZE_FOR_3D_CHART_VOLUME;
    long    nZ      = (long)((FIXED_SIZE_FOR_3D_CHART_VOLUME * 4.0) / 6.0);
    long nDepth = -nZ;
*/
    /*
    //defaults:
    double fFocalLen = 80.0; //UNO_NAME_3D_SCENE_FOCAL_LENGTH sal_Int32
    //UNO_NAME_3D_SCENE_AMBIENTCOLOR sal_Int32
    //UNO_NAME_3D_SCENE_PERSPECTIVE drawing::ProjectionMode
    double fTiltAng = DEG2RAD(-(double)nZAngle / 10.0);
    */

//    drawing::CameraGeometry aRet;
    //  Vector3D aCamPosition(nX + nW/2, nH/2, nW/2);
    //  Vector3D aLookDirection(nX + nW/2, nH/2, nDepth/2) ;
    //Vector3D aVUP;


    /*
    Vector3D aCamPosition(0, 0, 400);
    Vector3D aLookDirection(0,0,1) ;
    Vector3D aVUP(1,1,0);
    */
/*
    Vector3D aCamPosition(0, 0, 0);
    Vector3D aLookDirection(0,0,0) ;
    Vector3D aVUP(0,0,0);

    aRet.vrp.PositionX = aCamPosition.X(); //ViewReferencePoint (Point on the View plane)
    aRet.vrp.PositionY = aCamPosition.Y();
    aRet.vrp.PositionZ = aCamPosition.Z();
    aRet.vpn.DirectionX = aLookDirection.X(); //ViewPlaneNormal (Normal to the View Plane)
    aRet.vpn.DirectionY = aLookDirection.Y();
    aRet.vpn.DirectionZ = aLookDirection.Z();
*/
    //on the view plane there is a '3D viewing-reference coordinate (VCR) system' with VRP as origin
    //and orthogonal axes n,v,u (v and u in the view plane)
/*  aRet.vup.DirectionX = aVUP.X(); //view up vector; determines the v-axis direction on the view plane as projection of VUP parallel to VPN onto th view pane
    aRet.vup.DirectionY = aVUP.Y();
    aRet.vup.DirectionZ = aVUP.Z();
*/
    //a window is defined on the view plane with minimum and maximum v and u values
    //...where to set via uno??

//  return uno::Any(&aRet, ::getCppuType((const drawing::CameraGeometry*)0) );
    /*
    --------------
    Camera3D aCam(pScene->GetCamera());
    Vector3D aCamPos(nX + nW/2, nH/2, nW/2);
    Vector3D aLookAt(nX + nW/2, nH/2, nDepth/2);
    aCam.SetViewWindow(-nW/2, -nH/2, nW, nH);
    aCam.SetDefaults(aCamPos, aLookAt, 80, DEG2RAD(-(double)nZAngle / 10.0));
    //double fFocalLen = 35.0, double fBankAng = 0);
    aCam.Reset();
    aCam.SetProjection(eProjection);
    aCam.RotateAroundLookAt(DEG2RAD((double)nYAngle / 10.0), DEG2RAD((double)nXAngle / 10.0));
    aCam.SetAspectMapping(AS_HOLD_SIZE);
    pScene->SetCamera(aCam);
    --------------
    // copy lightsource 1 (Base3DLight0) to lightsource 2
    // color
    SetItem( Svx3DLightcolor2Item( GetLightGroup().GetIntensity( Base3DMaterialDiffuse, Base3DLight0 )));
    // direction
    SetItem( Svx3DLightDirection2Item( GetLightGroup().GetDirection( Base3DLight0 )));

    // enable light source 2
    SetItem( Svx3DLightOnOff2Item( TRUE ));
    // disable light source 1
    SetItem( Svx3DLightOnOff1Item( FALSE ));
    */
//}


/*
void DebugHelper_ShowSceneProperties(uno::Reference< drawing::XShape > xShape)
{
    drawing::HomogenMatrix aMatrix;
    drawing::CameraGeometry aCamera;
    sal_Int32 nDistance = 0;
    sal_Int32 nFocalLen = 0;
    drawing::ProjectionMode ePerspective;

    uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
    DBG_ASSERT(xProp.is(), "created shape offers no XPropertySet");
    if( xProp.is())
    {
        xProp->getPropertyValue( rtl::OUString::createFromAscii( UNO_NAME_3D_TRANSFORM_MATRIX ) )
         >>= aMatrix;
        xProp->getPropertyValue( rtl::OUString::createFromAscii( UNO_NAME_3D_CAMERA_GEOMETRY ) )
         >>= aCamera;
        xProp->getPropertyValue( rtl::OUString::createFromAscii( UNO_NAME_3D_SCENE_DISTANCE ) )
         >>= nDistance;
        xProp->getPropertyValue( rtl::OUString::createFromAscii( UNO_NAME_3D_SCENE_FOCAL_LENGTH ) )
         >>= nFocalLen;
        xProp->getPropertyValue( rtl::OUString::createFromAscii( UNO_NAME_3D_SCENE_PERSPECTIVE ) )
         >>= ePerspective;
    }
}
*/

//.............................................................................
} //namespace chart
//.............................................................................

