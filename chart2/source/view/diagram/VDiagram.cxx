/*************************************************************************
 *
 *  $RCSfile: VDiagram.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:34 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "VDiagram.hxx"
//#include "DebugHelper.hxx"
#include "PropertyMapper.hxx"
#include "CommonConverters.hxx"
#include "ViewDefines.hxx"
#include "Stripe.hxx"
#include "macros.hxx"

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

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::chart2;

VDiagram::VDiagram(
    const uno::Reference< XDiagram > & xDiagram,
    sal_Int32 nDimension, sal_Bool bPolar )
    : m_xLogicTarget(NULL)
    , m_xFinalTarget(NULL)
    , m_xShapeFactory(NULL)
    , m_pShapeFactory(NULL)
    , m_xOuterGroupShape(NULL)
    , m_xCoordinateRegionShape(NULL)
    , m_oSceneMatrix()
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

void  VDiagram::setSceneMatrix( const drawing::HomogenMatrix& rMatrix )
{
    m_oSceneMatrix = rMatrix;
}

void VDiagram::createShapes( const awt::Point& rPos, const awt::Size& rSize )
{
    if( m_nDimension == 3 )
        createShapes_3d( rPos, rSize );
    else
        createShapes_2d( rPos, rSize );
}

// const sal_Int32 nWallColor = 0xb0d8e8; //Color(COL_LIGHTGRAY).GetColor();
// const sal_Int32 nWallColor = 0xf5fffa;  // mint cream
// const sal_Int32 nWallColor = 0xc6e2ff;  // SlateGray1
const sal_Int32 nWallColor = 0xe0eeee;  // azure2
const sal_Int32 nWallTransparency = 90;

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
        xShape2D->setSize(awt::Size(FIXED_SIZE_FOR_3D_CHART_VOLUME,FIXED_SIZE_FOR_3D_CHART_VOLUME));
        xOuterGroup_Shapes->add(xShape2D);
        uno::Reference< beans::XPropertySet > xProp( xShape2D, uno::UNO_QUERY );
        if( xProp.is())
        {
            try
            {
                if( m_xDiagram.is() )
                {
                    uno::Reference< beans::XPropertySet > xWallProp( m_xDiagram->getWall());
                    if( xWallProp.is())
                        PropertyMapper::setMappedProperties( xProp, xWallProp, lcl_GetWallPropertyMap() );
                }
                else
                {
                    DBG_ERROR( "Invalid Diagram model" );
                    //Transparency
                    xProp->setPropertyValue( C2U( UNO_NAME_FILL_TRANSPARENCE )
                                             , uno::makeAny( (sal_Int16)nWallTransparency ) );
                    //xProp->setPropertyValue( C2U( UNO_NAME_LINETRANSPARENCE )
                    //    , uno::makeAny( (sal_Int16)100 ) );

                    //FillColor
                    xProp->setPropertyValue( C2U( UNO_NAME_FILLCOLOR )
                                             , uno::makeAny(nWallColor) );
                }

                //CID for selection handling
                xProp->setPropertyValue( C2U( UNO_NAME_MISC_OBJ_NAME )
                    , uno::makeAny( C2U("CID/DiagramWall=XXX_CID") ) );
            }
            catch( uno::Exception& e )
            {
                e;
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

void VDiagram::createShapes_3d( const awt::Point& rPos, const awt::Size& rSize )
{
    DBG_ASSERT(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is()&&m_xShapeFactory.is(),"is not proper initialized");
    if(!(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is()&&m_xShapeFactory.is()))
        return;

    //create shape
    m_xOuterGroupShape = uno::Reference< drawing::XShape >(
            m_xShapeFactory->createInstance( C2U(
            "com.sun.star.drawing.Shape3DSceneObject" ) ), uno::UNO_QUERY );
    m_xLogicTarget->add(m_xOuterGroupShape);

    //set properties
    uno::Reference< beans::XPropertySet > xProp( m_xOuterGroupShape, uno::UNO_QUERY );
    DBG_ASSERT(xProp.is(), "created shape offers no XPropertySet");
    if( xProp.is())
    {
        //set TransformationMatrix
        uno::Any aTM = uno::Any( &m_oSceneMatrix, ::getCppuType((const drawing::HomogenMatrix*)0) );

        sal_Int32 nDistance = (sal_Int32)FIXED_SIZE_FOR_3D_CHART_VOLUME*8; //(FIXED_SIZE_FOR_3D_CHART_VOLUME * 4) / 6
        sal_Int32 nFocalLen = nDistance*3;

        try
        {
            xProp->setPropertyValue( C2U( UNO_NAME_3D_TRANSFORM_MATRIX ), aTM );
            //uno::Any aCG = createDefaultCameraGeometry();
            //xProp->setPropertyValue( C2U( UNO_NAME_3D_CAMERA_GEOMETRY ), aCG );
            xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_DISTANCE ), uno::makeAny(nDistance) );
            xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_FOCAL_LENGTH ), uno::makeAny(nFocalLen) );
            xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_PERSPECTIVE ), uno::makeAny(drawing::ProjectionMode_PERSPECTIVE) ); //ProjectionMode_PARALLEL


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

            xProp->setPropertyValue( C2U( UNO_NAME_3D_SCENE_SHADE_MODE ), uno::makeAny(drawing::ShadeMode_PHONG) ); //FLAT, PHONG, SMOOTH, DRAFT

            //UNO_NAME_3D_SCENE_DISTANCE D3DSceneDistance sal_Int32
            //UNO_NAME_3D_SCENE_FOCAL_LENGTH sal_Int32
            //UNO_NAME_3D_SCENE_PERSPECTIVE drawing::ProjectionMode

            //UNO_NAME_3D_SCENE_AMBIENTCOLOR sal_Int32
            //UNO_NAME_3D_SCENE_LIGHTCOLOR_1 sal_Int32
            //UNO_NAME_3D_SCENE_LIGHTDIRECTION_1 drawing::Direction3D
            //UNO_NAME_3D_SCENE_LIGHTON_1 Bool
            //UNO_NAME_3D_SCENE_SHADOW_SLANT sal_Int16
            //UNO_NAME_3D_SCENE_SHADE_MODE drawing::ShadeMode
            //UNO_NAME_3D_SCENE_TWO_SIDED_LIGHTING Bool

            //test only - CID for selection handling
            xProp->setPropertyValue( C2U( UNO_NAME_MISC_OBJ_NAME )
                , uno::makeAny( C2U("testonly;3DScene=XXX_CID") ) );
        }
        catch( uno::Exception& e )
        {
            e;
        }
    }
    m_xOuterGroupShape->setPosition(rPos); //FIXED_SIZE_FOR_3D_CHART_VOLUME
    m_xOuterGroupShape->setSize(rSize); //FIXED_SIZE_FOR_3D_CHART_VOLUME


    uno::Reference< drawing::XShapes > xOuterGroup_Shapes =
            uno::Reference<drawing::XShapes>( m_xOuterGroupShape, uno::UNO_QUERY );


    xOuterGroup_Shapes = m_pShapeFactory->createGroup3D( xOuterGroup_Shapes, C2U("CID/Diagram=XXX_CID") );

    //-------------------------------------------------------------------------
    //create additional group to manipulate the aspect ratio of the whole diagram:
    /*
    {
        uno::Reference< beans::XPropertySet > xProp( xOuterGroup_Shapes, uno::UNO_QUERY );
        DBG_ASSERT(xProp.is(), "created shape offers no XPropertySet");
        if( xProp.is())
        {
            try
            {
                Matrix4D aM4;
                double CatsFactor = 15.0;//@todo needs to be calculated from seriescount and slot distance ...
                aM4.Scale(1.0, 1.0, 1.0/CatsFactor);
                xProp->setPropertyValue( C2U( UNO_NAME_3D_TRANSFORM_MATRIX )
                    , uno::makeAny(Matrix4DToHomogenMatrix(aM4)) );
            }
            catch( uno::Exception& e )
            {
                e;
            }
        }
    }
    */

    //---------------------------

    //add floor plate
    {
        uno::Reference< drawing::XShape > xShape =
            m_pShapeFactory->createCube(xOuterGroup_Shapes,
            DataPointGeometry( drawing::Position3D(FIXED_SIZE_FOR_3D_CHART_VOLUME/2.0,0,FIXED_SIZE_FOR_3D_CHART_VOLUME/2.0)
                              ,drawing::Direction3D(FIXED_SIZE_FOR_3D_CHART_VOLUME,FLOOR_THICKNESS,FIXED_SIZE_FOR_3D_CHART_VOLUME) )
            , ShapeAppearance( Color(COL_LIGHTGRAY).GetColor(), 0 ) );
        ShapeFactory::setShapeName( xShape, C2U("CID/DiagramFloor=XXX_CID") );
    }

    //add left wall
    {
        uno::Reference< drawing::XShape > xShape =
            m_pShapeFactory->createStripe(xOuterGroup_Shapes
            , Stripe(
                  drawing::Position3D(0,0,FIXED_SIZE_FOR_3D_CHART_VOLUME)
                , drawing::Direction3D(0,0,-FIXED_SIZE_FOR_3D_CHART_VOLUME)
                , drawing::Direction3D(0,FIXED_SIZE_FOR_3D_CHART_VOLUME,0) )
            , ShapeAppearance( nWallColor, nWallTransparency )
            , false );
        ShapeFactory::setShapeName( xShape, C2U("CID/DiagramWall=XXX_CID") );
    }
    //add back wall
    {
        uno::Reference< drawing::XShape > xShape =
            m_pShapeFactory->createStripe(xOuterGroup_Shapes
            , Stripe(
                  drawing::Position3D(0,0,0)
                , drawing::Direction3D(FIXED_SIZE_FOR_3D_CHART_VOLUME,0,0)
                , drawing::Direction3D(0,FIXED_SIZE_FOR_3D_CHART_VOLUME,0) )
            , ShapeAppearance( nWallColor, nWallTransparency )
            , false );
        ShapeFactory::setShapeName( xShape, C2U("CID/DiagramWall=XXX_CID") );
    }
    //---------------------------

    //create an additional scene for the smaller inner coordinate region:
    {
        uno::Reference< drawing::XShapes > xShapes = m_pShapeFactory->createGroup3D( xOuterGroup_Shapes,C2U("testonly;CooContainer=XXX_CID") );
        m_xCoordinateRegionShape = uno::Reference< drawing::XShape >( xShapes, uno::UNO_QUERY );

        uno::Reference< beans::XPropertySet > xProp( m_xCoordinateRegionShape, uno::UNO_QUERY );
        DBG_ASSERT(xProp.is(), "created shape offers no XPropertySet");
        if( xProp.is())
        {
            try
            {
                double fXScale = (FIXED_SIZE_FOR_3D_CHART_VOLUME -GRID_TO_WALL_DISTANCE) /FIXED_SIZE_FOR_3D_CHART_VOLUME;
                double fYScale = (FIXED_SIZE_FOR_3D_CHART_VOLUME -FLOOR_THICKNESS      ) /FIXED_SIZE_FOR_3D_CHART_VOLUME;
                double fZScale = (FIXED_SIZE_FOR_3D_CHART_VOLUME -GRID_TO_WALL_DISTANCE) /FIXED_SIZE_FOR_3D_CHART_VOLUME;

                Matrix4D aM4;
                aM4.Translate(GRID_TO_WALL_DISTANCE/fXScale, FLOOR_THICKNESS/fYScale, GRID_TO_WALL_DISTANCE/fZScale);
                aM4.Scale( fXScale, fYScale, fZScale );
                xProp->setPropertyValue( C2U( UNO_NAME_3D_TRANSFORM_MATRIX )
                    , uno::makeAny(Matrix4DToHomogenMatrix(aM4)) );
            }
            catch( uno::Exception& e )
            {
                e;
            }
        }
    }

    //---------------------------
    {//only test

        //only test: create grey borders as replacement for axes
        /*
        uno::Reference< drawing::XShapes > xTarget( m_xCoordinateRegionShape, uno::UNO_QUERY );
        m_pShapeFactory->createCube(xTarget,
            DataPointGeometry( drawing::Position3D(FIXED_SIZE_FOR_3D_CHART_VOLUME/2,0,FIXED_SIZE_FOR_3D_CHART_VOLUME-50)
                              ,drawing::Direction3D(FIXED_SIZE_FOR_3D_CHART_VOLUME,100,100) )
            , ShapeAppearance( Color(COL_LIGHTGRAY).GetColor() ) );
        m_pShapeFactory->createCube(xTarget,
            DataPointGeometry( drawing::Position3D(50,0,FIXED_SIZE_FOR_3D_CHART_VOLUME/2)
                              ,drawing::Direction3D(100,100,FIXED_SIZE_FOR_3D_CHART_VOLUME) )
            , ShapeAppearance( Color(COL_LIGHTGRAY).GetColor() ) );
        m_pShapeFactory->createCube(xTarget,
            DataPointGeometry( drawing::Position3D(50,0,FIXED_SIZE_FOR_3D_CHART_VOLUME-50)
                              ,drawing::Direction3D(100,FIXED_SIZE_FOR_3D_CHART_VOLUME,100) )
            , ShapeAppearance( Color(COL_LIGHTGRAY).GetColor() ) );
        m_pShapeFactory->createCube(xTarget,
            DataPointGeometry( drawing::Position3D(50,0,50)
                              ,drawing::Direction3D(100,FIXED_SIZE_FOR_3D_CHART_VOLUME,100) )
            , ShapeAppearance( Color(COL_LIGHTGRAY).GetColor() ) );

        m_pShapeFactory->createCube(xTarget,
            DataPointGeometry( drawing::Position3D(FIXED_SIZE_FOR_3D_CHART_VOLUME-50,0,FIXED_SIZE_FOR_3D_CHART_VOLUME-50)
                              ,drawing::Direction3D(100,FIXED_SIZE_FOR_3D_CHART_VOLUME,100) )
            , ShapeAppearance( Color(COL_LIGHTGRAY).GetColor() ) );
        m_pShapeFactory->createCube(xTarget,
            DataPointGeometry( drawing::Position3D(FIXED_SIZE_FOR_3D_CHART_VOLUME-50,0,50)
                              ,drawing::Direction3D(100,FIXED_SIZE_FOR_3D_CHART_VOLUME,100) )
            , ShapeAppearance( Color(COL_LIGHTGRAY).GetColor() ) );
        */
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

