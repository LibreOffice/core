/*************************************************************************
 *
 *  $RCSfile: ShapeFactory.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: iha $ $Date: 2003-11-13 10:03:52 $
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
#include "ShapeFactory.hxx"
#include "ViewDefines.hxx"
#include "Stripe.hxx"
#include "CommonConverters.hxx"
#include "macros.hxx"
#include "InlineContainer.hxx"
#include "PropertyMapper.hxx"

/*
#ifndef _E3D_EXTRUD3D_HXX
#include <svx/extrud3d.hxx>
#endif
*/

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
/*
#ifndef _COM_SUN_STAR_DRAWING_CAMERAGEOMETRY_HPP_
#include <com/sun/star/drawing/CameraGeometry.hpp>
#endif
*/
#ifndef _COM_SUN_STAR_DRAWING_CIRCLEKIND_HPP_
#include <com/sun/star/drawing/CircleKind.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_DOUBLESEQUENCE_HPP_
#include <com/sun/star/drawing/DoubleSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FLAGSEQUENCE_HPP_
#include <com/sun/star/drawing/FlagSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINESTYLE_HPP_
#include <com/sun/star/drawing/LineStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_NORMALSKIND_HPP_
#include <com/sun/star/drawing/NormalsKind.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POINTSEQUENCE_HPP_
#include <com/sun/star/drawing/PointSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POLYGONKIND_HPP_
#include <com/sun/star/drawing/PolygonKind.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POLYPOLYGONBEZIERCOORDS_HPP_
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_PROJECTIONMODE_HPP_
#include <com/sun/star/drawing/ProjectionMode.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_SHADEMODE_HPP_
#include <com/sun/star/drawing/ShadeMode.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_TEXTFITTOSIZETYPE_HPP_
#include <com/sun/star/drawing/TextFitToSizeType.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_TEXTVERTICALADJUST_HPP_
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_TEXTHORIZONTALADJUST_HPP_
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif
/*
#ifndef _COM_SUN_STAR_DRAWING_TEXTUREMODE_HPP_
#include <com/sun/star/drawing/TextureMode.hpp>
#endif
*/

//only test
/*
#ifndef _COM_SUN_STAR_AWT_FONTSTRIKEOUT_HPP_
#include <com/sun/star/awt/FontStrikeout.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_FONTEMPHASIS_HPP_
#include <com/sun/star/text/FontEmphasis.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/container/XEnumeration.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif
*/

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif


#ifndef _SVX_UNOPRNMS_HXX
#include <svx/unoprnms.hxx>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

#ifndef _XPOLY_HXX
#include <svx/xpoly.hxx>
#endif
#ifndef _SVDOCIRC_HXX
#include <svx/svdocirc.hxx>
#endif
#ifndef _SVDOPATH_HXX
#include <svx/svdopath.hxx>
#endif

#include <algorithm>

using namespace ::com::sun::star;

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// set a name/CID at a shape (is used for selection handling)
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//static
void ShapeFactory::setShapeName( const uno::Reference< drawing::XShape >& xShape
                               , const rtl::OUString& rName )
{
    uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
    DBG_ASSERT(xProp.is(), "shape offers no XPropertySet");
    if( xProp.is())
    {
        try
        {
            xProp->setPropertyValue( C2U( UNO_NAME_MISC_OBJ_NAME )
                , uno::makeAny( rName ) );
        }
        catch( uno::Exception& e )
        {
            ASSERT_EXCEPTION( e );
        }
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  diverse PolyPolygon create methods
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

uno::Any createPolyPolygon_Cube(
            const drawing::Direction3D& rSize, double fRoundedEdge, sal_Bool bRounded = true )
{
    DBG_ASSERT(fRoundedEdge>=0, "fRoundedEdge needs to be >= 0");

    // always use extra points, so set percent diagonal to 0.4 which is 0% in the UI (old Chart comment)
    if( fRoundedEdge == 0.0  && bRounded)
        fRoundedEdge = 0.4 / 200.0;
    else if(!bRounded)
        fRoundedEdge = 0.0;

    const double fWidthH = rSize.DirectionX/2.0; //fWidthH stands for Half Width
    const double fHeight = rSize.DirectionY;
    const double fDepth = rSize.DirectionZ;

    const double fOffset = (fDepth * fRoundedEdge) * 1.05;  // increase by 5% for safety
    const bool bRoundEdges = fRoundedEdge && fOffset < fWidthH && 2.0 * fOffset < fHeight;
    const sal_Int32 nPointCount = bRoundEdges ? 13 : 5;

    //--------------------------------------
    drawing::PolyPolygonShape3D aPP;

    aPP.SequenceX.realloc(1);
    aPP.SequenceY.realloc(1);
    aPP.SequenceZ.realloc(1);

    drawing::DoubleSequence* pOuterSequenceX = aPP.SequenceX.getArray();
    drawing::DoubleSequence* pOuterSequenceY = aPP.SequenceY.getArray();
    drawing::DoubleSequence* pOuterSequenceZ = aPP.SequenceZ.getArray();

    pOuterSequenceX->realloc(nPointCount);
    pOuterSequenceY->realloc(nPointCount);
    pOuterSequenceZ->realloc(nPointCount);

    double* pInnerSequenceX = pOuterSequenceX->getArray();
    double* pInnerSequenceY = pOuterSequenceY->getArray();
    double* pInnerSequenceZ = pOuterSequenceZ->getArray();

    for(sal_Int32 nN = nPointCount; nN--;)
        *pInnerSequenceZ++ = 0.0;
        //*pInnerSequenceZ++ = -fDepth/2.0;

    if(nPointCount == 5)
    {
        *pInnerSequenceY++ = 0.0;
        *pInnerSequenceY++ = fHeight;
        *pInnerSequenceY++ = fHeight;
        *pInnerSequenceY++ = 0.0;
        *pInnerSequenceY++ = 0.0;

        *pInnerSequenceX++ = -fWidthH;
        *pInnerSequenceX++ = -fWidthH;
        *pInnerSequenceX++ = fWidthH;
        *pInnerSequenceX++ = fWidthH;
        *pInnerSequenceX++ = -fWidthH;
    }
    else
    {
        *pInnerSequenceY++ = 0.0; //1.
        *pInnerSequenceY++ = 0.0;
        *pInnerSequenceY++ = fOffset;
        *pInnerSequenceY++ = fHeight - fOffset;
        *pInnerSequenceY++ = fHeight;
        *pInnerSequenceY++ = fHeight; //6.
        *pInnerSequenceY++ = fHeight;
        *pInnerSequenceY++ = fHeight;
        *pInnerSequenceY++ = fHeight - fOffset;
        *pInnerSequenceY++ = fOffset; //10.
        *pInnerSequenceY++ = 0.0;
        *pInnerSequenceY++ = 0.0;
        *pInnerSequenceY++ = 0.0;


        *pInnerSequenceX++ = -fWidthH + fOffset; //1.
        *pInnerSequenceX++ = -fWidthH;
        *pInnerSequenceX++ = -fWidthH;
        *pInnerSequenceX++ = -fWidthH;
        *pInnerSequenceX++ = -fWidthH;
        *pInnerSequenceX++ = -fWidthH + fOffset; //6.
        *pInnerSequenceX++ = fWidthH - fOffset;
        *pInnerSequenceX++ = fWidthH;
        *pInnerSequenceX++ = fWidthH;
        *pInnerSequenceX++ = fWidthH; //10.
        *pInnerSequenceX++ = fWidthH;
        *pInnerSequenceX++ = fWidthH - fOffset;
        *pInnerSequenceX++ = -fWidthH + fOffset;
    }
    return uno::Any( &aPP, ::getCppuType((const drawing::PolyPolygonShape3D*)0) );
}

uno::Any createPolyPolygon_Cylinder(
             double fHeight
           , double fRadius
           , double fRoundedEdge )
{
    //@todo consider offset if Height is negative

//    DBG_ASSERT(fHeight>0, "The height of a cylinder needs to be > 0");
    DBG_ASSERT(fRadius>0, "The radius of a cylinder needs to be > 0");
    DBG_ASSERT(fRoundedEdge>=0, "fRoundedEdge needs to be >= 0");

    // always use extra points, so set percent diagonal to 0.4 which is 0% in the UI (old Chart comment)
    if( fRoundedEdge == 0.0 )
        fRoundedEdge = 0.4 / 200.0;

//     const double fWidth = fRadius;

    const double fOffset = (fRadius * 2.0 * fRoundedEdge) * 1.05;   // increase by 5% for safety
    const bool bRoundEdges = fRoundedEdge && fOffset < fRadius && 2.0 * fOffset < fHeight;
    const sal_Int32 nPointCount = bRoundEdges ? 8 : 4;

    //--------------------------------------
    drawing::PolyPolygonShape3D aPP;

    aPP.SequenceX.realloc(1);
    aPP.SequenceY.realloc(1);
    aPP.SequenceZ.realloc(1);

    drawing::DoubleSequence* pOuterSequenceX = aPP.SequenceX.getArray();
    drawing::DoubleSequence* pOuterSequenceY = aPP.SequenceY.getArray();
    drawing::DoubleSequence* pOuterSequenceZ = aPP.SequenceZ.getArray();

    pOuterSequenceX->realloc(nPointCount);
    pOuterSequenceY->realloc(nPointCount);
    pOuterSequenceZ->realloc(nPointCount);

    double* pInnerSequenceX = pOuterSequenceX->getArray();
    double* pInnerSequenceY = pOuterSequenceY->getArray();
    double* pInnerSequenceZ = pOuterSequenceZ->getArray();

    for(sal_Int32 nN = nPointCount; nN--;)
        *pInnerSequenceZ++ = 0.0;

    if(nPointCount == 4)
    {
        *pInnerSequenceY++ = 0.0;
        *pInnerSequenceY++ = 0.0;
        *pInnerSequenceY++ = fHeight;
        *pInnerSequenceY++ = fHeight;

        *pInnerSequenceX++ = 0.0;
        *pInnerSequenceX++ = fRadius;
        *pInnerSequenceX++ = fRadius;
        *pInnerSequenceX++ = 0.0;
    }
    else
    {
        *pInnerSequenceY++ = 0.0; //1.
        *pInnerSequenceY++ = 0.0;
        *pInnerSequenceY++ = 0.0;
        *pInnerSequenceY++ = fOffset;
        *pInnerSequenceY++ = fHeight - fOffset;
        *pInnerSequenceY++ = fHeight; //6.
        *pInnerSequenceY++ = fHeight;
        *pInnerSequenceY++ = fHeight;

        *pInnerSequenceX++ = 0.0; //1.
        *pInnerSequenceX++ = fRadius - fOffset;
        *pInnerSequenceX++ = fRadius;
        *pInnerSequenceX++ = fRadius;
        *pInnerSequenceX++ = fRadius;
        *pInnerSequenceX++ = fRadius; //6.
        *pInnerSequenceX++ = fRadius - fOffset;
        *pInnerSequenceX++ = 0.0;
    }
    return uno::Any( &aPP, ::getCppuType((const drawing::PolyPolygonShape3D*)0) );
}

uno::Any createPolyPolygon_Cone(
              double fHeight
            , double fRadius
            , double fTopHeight
            , double fRoundedEdge)
{
    //@todo consider offset if Height is negative
/*
    DBG_ASSERT(fHeight>0, "The height of a cone needs to be > 0");
    DBG_ASSERT(fTopHeight>=0, "The height of the cutted top of a cone needs to be >= 0");
*/

    DBG_ASSERT(fRadius>0, "The radius of a cone needs to be > 0");
    DBG_ASSERT(fRoundedEdge>=0, "fRoundedEdge needs to be >= 0");

    //for stacked charts we need cones without top -> fTopHeight != 0 resp. bTopless == true
    //fTopHeight indicates the high of the cutted top only (not the full height)

    // always use extra points, so set percent diagonal to 0.4 which is 0% in the UI (old Chart comment)
    if( fRoundedEdge == 0.0 )
        fRoundedEdge = 0.4 / 200.0;

    fRoundedEdge = 0.0;

    BOOL bTopless = fTopHeight != 0.0;

    double r1= 0.0, r2 = fRadius;
    if(bTopless)
        r1 = fRadius * (fTopHeight)/(fHeight+fTopHeight);

    const double fMinimumDimension = ::std::min(r2*2.0,fHeight);
    const double fOffset = (fMinimumDimension * fRoundedEdge) * 1.05;   // increase by 5% for safety
    const bool   bRoundEdges = fRoundedEdge && fOffset < r2 && 2.0 * fOffset < fHeight
                            && ( bTopless ? fOffset < r1 : true );
    sal_Int32 nPointCount = 8;
    if(bTopless)
    {
        if(!bRoundEdges)
            nPointCount = 4;
    }
    else
    {
        if(bRoundEdges)
            nPointCount = 6;
        else
            nPointCount = 3;
    }

    //--------------------------------------
    drawing::PolyPolygonShape3D aPP;

    aPP.SequenceX.realloc(1);
    aPP.SequenceY.realloc(1);
    aPP.SequenceZ.realloc(1);

    drawing::DoubleSequence* pOuterSequenceX = aPP.SequenceX.getArray();
    drawing::DoubleSequence* pOuterSequenceY = aPP.SequenceY.getArray();
    drawing::DoubleSequence* pOuterSequenceZ = aPP.SequenceZ.getArray();

    pOuterSequenceX->realloc(nPointCount);
    pOuterSequenceY->realloc(nPointCount);
    pOuterSequenceZ->realloc(nPointCount);

    double* pInnerSequenceX = pOuterSequenceX->getArray();
    double* pInnerSequenceY = pOuterSequenceY->getArray();
    double* pInnerSequenceZ = pOuterSequenceZ->getArray();

    for(sal_Int32 nN = nPointCount; nN--;)
        *pInnerSequenceZ++ = 0.0;

    if(bTopless)
    {
        *pInnerSequenceY++ = fHeight; //1.
        *pInnerSequenceX++ = 0.0; //1.

        if(bRoundEdges)
        {
            *pInnerSequenceY++ = fHeight; //2.
            *pInnerSequenceX++ = r1 - fOffset; //2.
        }
    }

    *pInnerSequenceY++ = fHeight; //3.
    *pInnerSequenceX++ = r1; //3.

    if(bRoundEdges)
    {
        *pInnerSequenceY++ = fHeight - fOffset; //4.
        *pInnerSequenceX++ = r1 + fOffset; //4.

        *pInnerSequenceY++ = fOffset; //5.
        *pInnerSequenceX++ = r2 - fOffset; //5.
    }

    *pInnerSequenceY++ = 0.0; //6.
    *pInnerSequenceX++ = r2; //6.

    if(bRoundEdges)
    {
        *pInnerSequenceY++ = 0.0; //7.
        *pInnerSequenceX++ = r2 - fOffset; //7.
    }

    *pInnerSequenceY++ = 0.0; //8.
    *pInnerSequenceX++ = 0.0; //8.

    return uno::Any( &aPP, ::getCppuType((const drawing::PolyPolygonShape3D*)0) );
}

uno::Any createPolyPolygon_PieSegment(
              double fHeight
            , double fXWidthRadius
            , double fInnerXRadius = 0.0)
{
    DBG_ASSERT(fHeight>0, "The height of a pie segment needs to be > 0");
    DBG_ASSERT(fInnerXRadius>=0, "The inner radius of a pie segment needs to be >= 0");
    DBG_ASSERT(fXWidthRadius>0, "The width radius of a pie segment needs to be > 0");

    const BOOL bClosed = true;

    const sal_Int32 nPointCount = bClosed ? 5 : 4;

    //--------------------------------------
    drawing::PolyPolygonShape3D aPP;

    aPP.SequenceX.realloc(1);
    aPP.SequenceY.realloc(1);
    aPP.SequenceZ.realloc(1);

    drawing::DoubleSequence* pOuterSequenceX = aPP.SequenceX.getArray();
    drawing::DoubleSequence* pOuterSequenceY = aPP.SequenceY.getArray();
    drawing::DoubleSequence* pOuterSequenceZ = aPP.SequenceZ.getArray();

    pOuterSequenceX->realloc(nPointCount);
    pOuterSequenceY->realloc(nPointCount);
    pOuterSequenceZ->realloc(nPointCount);

    double* pInnerSequenceX = pOuterSequenceX->getArray();
    double* pInnerSequenceY = pOuterSequenceY->getArray();
    double* pInnerSequenceZ = pOuterSequenceZ->getArray();

    for(sal_Int32 nN = nPointCount; nN--;)
        *pInnerSequenceZ++ = 0.0;

    *pInnerSequenceY++ = 0.0;
    *pInnerSequenceY++ = 0.0;
    *pInnerSequenceY++ = fHeight;
    *pInnerSequenceY++ = fHeight;
    if(bClosed)
        *pInnerSequenceY++ = 0.0;

    *pInnerSequenceX++ = fInnerXRadius;
    *pInnerSequenceX++ = fInnerXRadius+fXWidthRadius;
    *pInnerSequenceX++ = fInnerXRadius+fXWidthRadius;
    *pInnerSequenceX++ = fInnerXRadius;
    if(bClosed)
        *pInnerSequenceX++ = fInnerXRadius;

    return uno::Any( &aPP, ::getCppuType((const drawing::PolyPolygonShape3D*)0) );
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  methods for 3D shape creation
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

uno::Reference<drawing::XShape>
        ShapeFactory::createCube(
            const uno::Reference<drawing::XShapes>& xTarget
            , const DataPointGeometry& rGeometry
            , const uno::Reference< beans::XPropertySet >& xSourceProp
            , const tPropertyNameMap& rPropertyNameMap )
{
    uno::Reference<drawing::XShape> xShape = impl_createCube( xTarget, rGeometry, FALSE );
    uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
    if( xSourceProp.is())
        PropertyMapper::setMappedProperties( xProp, xSourceProp, rPropertyNameMap );
    return xShape;
}

uno::Reference<drawing::XShape>
        ShapeFactory::createRoundedCube(
            const uno::Reference<drawing::XShapes>& xTarget
            , const DataPointGeometry& rGeometry )
{
    return impl_createCube( xTarget, rGeometry, TRUE);
}

uno::Reference<drawing::XShape>
        ShapeFactory::impl_createCube(
              const uno::Reference<drawing::XShapes>& xTarget
            , const DataPointGeometry& rGeometry
            , sal_Bool bRounded )
{
    //create shape
    uno::Reference< drawing::XShape > xShape(
        m_xShapeFactory->createInstance( C2U(
            "com.sun.star.drawing.Shape3DExtrudeObject") ), uno::UNO_QUERY );
    xTarget->add(xShape);

    //set properties
    uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
    DBG_ASSERT(xProp.is(), "created shape offers no XPropertySet");
    if( xProp.is())
    {
        try
        {
            //depth
            xProp->setPropertyValue( C2U( UNO_NAME_3D_EXTRUDE_DEPTH )
                , uno::makeAny((sal_Int32)rGeometry.m_aSize.DirectionZ) );

            //PercentDiagonal
            sal_Int16 nPercentDiagonal = bRounded ? 5 : 0;
            xProp->setPropertyValue( C2U( UNO_NAME_3D_PERCENT_DIAGONAL )
                , uno::makeAny( nPercentDiagonal ) );

            //Polygon
            xProp->setPropertyValue( C2U( UNO_NAME_3D_POLYPOLYGON3D )
                , createPolyPolygon_Cube( rGeometry.m_aSize, double(nPercentDiagonal)/200.0,bRounded) );

            //Matrix for position
            {
                Matrix4D aM4;
                aM4.Translate(rGeometry.m_aPosition.PositionX
                            , rGeometry.m_aPosition.PositionY
                            , rGeometry.m_aPosition.PositionZ - (rGeometry.m_aSize.DirectionZ/2.0));
                drawing::HomogenMatrix aHM = Matrix4DToHomogenMatrix(aM4);
                xProp->setPropertyValue( C2U( UNO_NAME_3D_TRANSFORM_MATRIX )
                    , uno::makeAny(aHM) );
            }
        }
        catch( uno::Exception& e )
        {
            ASSERT_EXCEPTION( e );
        }
    }
    return xShape;
}

uno::Reference<drawing::XShape>
        ShapeFactory::createCylinder(
            const uno::Reference<drawing::XShapes>& xTarget
            , const DataPointGeometry& rGeometry )
{
    return impl_createConeOrCylinder(
              xTarget, rGeometry, CHART_3DOBJECT_SEGMENTCOUNT, true );
}

uno::Reference<drawing::XShape>
        ShapeFactory::createPyramid(
            const uno::Reference<drawing::XShapes>& xTarget
            , const DataPointGeometry& rGeometry )
{
    return impl_createConeOrCylinder( xTarget, rGeometry, 4 );
}

uno::Reference<drawing::XShape>
        ShapeFactory::createCone(
            const uno::Reference<drawing::XShapes>& xTarget
            , const DataPointGeometry& rGeometry )
{
    return impl_createConeOrCylinder( xTarget, rGeometry, CHART_3DOBJECT_SEGMENTCOUNT );
}

uno::Reference<drawing::XShape>
        ShapeFactory::impl_createConeOrCylinder(
              const uno::Reference<drawing::XShapes>& xTarget
            , const DataPointGeometry& rGeometry
            , sal_Int32 nSegments
            , bool bCylinder )
{

    static int ntest = 0;
    ntest++;

    //create shape
    uno::Reference< drawing::XShape > xShape(
            m_xShapeFactory->createInstance( C2U(
            "com.sun.star.drawing.Shape3DLatheObject") ), uno::UNO_QUERY );
    xTarget->add(xShape);


    double fYRotateAnglePi = -ZDIRECTION*(F_PI/2.0 - F_PI/(double)nSegments); // alwayas rotate edge to front (important for pyramids)
                           //or:  ZDIRECTION*(F_PI/2.0 - F_PI/(double)nSegments); // rotate edge to front for even segment count otherwise rotate corner to front
    double fAngle = fYRotateAnglePi;
    {
        while(fAngle<0.0)
            fAngle+=F_PI/2.0;
        while(fAngle>F_PI/2.0)
            fAngle-=F_PI/2.0;
    }
    //double fWidth      = ::std::min( rGeometry.m_aSize.DirectionX, rGeometry.m_aSize.DirectionZ )/2.0;
    double fWidth      = rGeometry.m_aSize.DirectionX/2.0; //The depth will be corrrected within Matrix
    double fRadius     = fWidth; //!!!!!!!! problem in drawing layer: rotation object calculates wrong needed size -> wrong camera (it's a problem with bounding boxes)
//    double fRadius     = fWidth/cos(fAngle); llllllllllllllllllll
    double fHeight     = rGeometry.m_aSize.DirectionY;
    //for Cones:
    double fTopHeight  = rGeometry.m_aSize2.DirectionY;

    //set properties
    uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
    DBG_ASSERT(xProp.is(), "created shape offers no XPropertySet");
    if( xProp.is())
    {
        try
        {
            //UNO_NAME_3D_TRANSFORM_MATRIX drawing::HomogenMatrix
            //UNO_NAME_3D_POLYPOLYGON3D PolyPolygonShape3D
            //UNO_NAME_3D_LATHE_END_ANGLE sal_Int16
            //UNO_NAME_3D_HORZ_SEGS sal_Int32
            //UNO_NAME_3D_PERCENT_DIAGONAL sal_Int16
            //UNO_NAME_3D_VERT_SEGS sal_Int32

            //PercentDiagonal
            sal_Int16 nPercentDiagonal = 5;
            xProp->setPropertyValue( C2U( UNO_NAME_3D_PERCENT_DIAGONAL )
                , uno::makeAny( nPercentDiagonal ) );

            //Polygon
            uno::Any aPPolygon = bCylinder ? createPolyPolygon_Cylinder(
                                                fHeight, fRadius, double(nPercentDiagonal)/200.0)
                                           : createPolyPolygon_Cone(
                                                fHeight, fRadius, fTopHeight, double(nPercentDiagonal)/200.0);
            xProp->setPropertyValue( C2U( UNO_NAME_3D_POLYPOLYGON3D ), aPPolygon );

            //Matrix for position
            {
                Matrix4D aM4;
                //if(ntest%2) //llllllllllllllllllll
                //aM4.RotateY( fYRotateAnglePi );

                // Note: Uncomment the following to let the objects grow in z
                // direction to fill the diagram
//                 aM4.ScaleZ(rGeometry.m_aSize.DirectionZ/rGeometry.m_aSize.DirectionX);
                aM4.Translate(rGeometry.m_aPosition.PositionX, rGeometry.m_aPosition.PositionY, rGeometry.m_aPosition.PositionZ);
                drawing::HomogenMatrix aHM = Matrix4DToHomogenMatrix(aM4);
                xProp->setPropertyValue( C2U( UNO_NAME_3D_TRANSFORM_MATRIX )
                    , uno::makeAny(aHM) );
            }

            //Segments
            xProp->setPropertyValue( C2U( UNO_NAME_3D_VERT_SEGS )
                , uno::makeAny(CHART_3DOBJECT_SEGMENTCOUNT) );
            xProp->setPropertyValue( C2U( UNO_NAME_3D_HORZ_SEGS )
                , uno::makeAny(nSegments) );
        }
        catch( uno::Exception& e )
        {
            ASSERT_EXCEPTION( e );
        }
    }
    return xShape;
}

void appendBezierCoords( drawing::PolyPolygonBezierCoords& rReturn, const drawing::PolyPolygonBezierCoords& rAdd, sal_Bool bAppendInverse )
{
    if(!rAdd.Coordinates.getLength())
        return;
    sal_Int32 nAddCount = rAdd.Coordinates[0].getLength();
    if(!nAddCount)
        return;

    sal_Int32 nOldCount = rReturn.Coordinates[0].getLength();

    rReturn.Coordinates[0].realloc(nOldCount+nAddCount);
    rReturn.Flags[0].realloc(nOldCount+nAddCount);

    for(sal_Int32 nN=0;nN<nAddCount; nN++ )
    {
        sal_Int32 nAdd = bAppendInverse ? (nAddCount-1-nN) : nN;
        rReturn.Coordinates[0][nOldCount+nN] = rAdd.Coordinates[0][nAdd];
        rReturn.Flags[0][nOldCount+nN] = rAdd.Flags[0][nAdd];
    }
}

drawing::PolyPolygonBezierCoords getCircularArcBezierCoords(
        double fAngleRadian
        , const Matrix3D& rTransformationFromUnitCircle )
{
    //at least one polygon is created using two normal and two control points
    //if the angle is larger it is separated into multiple sub angles

    const double fSmallAngleRadian = F_PI/10.0;

    drawing::PolyPolygonBezierCoords aReturn = drawing::PolyPolygonBezierCoords();
    sal_Int32 nSegmentCount = static_cast< sal_Int32 >( fAngleRadian/fSmallAngleRadian );
    if( fAngleRadian > fSmallAngleRadian*nSegmentCount )
        nSegmentCount++;

    sal_Int32 nPointCount     = 1 + 3*nSegmentCount; //first point of next segment equals last point of former segment

    aReturn.Coordinates = drawing::PointSequenceSequence(1);
    aReturn.Flags       = drawing::FlagSequenceSequence(1);

    drawing::PointSequence aPoints(nPointCount);
    drawing::FlagSequence  aFlags(nPointCount);

    //

    Vector2D P0,P1,P2,P3;
    Vector2D POrigin = rTransformationFromUnitCircle*Vector2D(0,0);

    sal_Int32 nPoint=0;
    for(sal_Int32 nSegment=0; nSegment<nSegmentCount; nSegment++)
    {
        double fCurrentSegmentAngle = fSmallAngleRadian;
        //the last segment gets the rest angle that does not fit into equal pieces
        if(nSegment==(nSegmentCount-1))
            fCurrentSegmentAngle = fAngleRadian - fSmallAngleRadian*(nSegmentCount-1);

        //first create untransformed points for a unit circle arc:
        const double fCos = cos(fCurrentSegmentAngle/2.0);
        const double fSin = sin(fCurrentSegmentAngle/2.0);
        P0.X() = P3.X() = fCos;
        P0.Y() = -fSin;
        P3.Y() = -P0.Y();

        P1.X() = P2.X() = (4.0-fCos)/3.0;
        P1.Y() = (1.0-fCos)*(fCos-3.0)/(3.0*fSin);
        P2.Y() = -P1.Y();
        //transform thus startangle equals NULL
        Matrix3D aStart;
        aStart.Rotate(fCurrentSegmentAngle/2.0 + nSegment*fSmallAngleRadian);

        //apply given transformation to get final points
        P0 = rTransformationFromUnitCircle*(aStart*P0);
        P1 = rTransformationFromUnitCircle*(aStart*P1);
        P2 = rTransformationFromUnitCircle*(aStart*P2);
        P3 = rTransformationFromUnitCircle*(aStart*P3);

        aPoints[nPoint].X = static_cast< sal_Int32 >( P0.X());
        aPoints[nPoint].Y = static_cast< sal_Int32 >( P0.Y());
        aFlags [nPoint++] = drawing::PolygonFlags_NORMAL;

        aPoints[nPoint].X = static_cast< sal_Int32 >( P1.X());
        aPoints[nPoint].Y = static_cast< sal_Int32 >( P1.Y());
        aFlags[nPoint++] = drawing::PolygonFlags_CONTROL;

        aPoints[nPoint].X = static_cast< sal_Int32 >( P2.X());
        aPoints[nPoint].Y = static_cast< sal_Int32 >( P2.Y());
        aFlags [nPoint++] = drawing::PolygonFlags_CONTROL;

        if(nSegment==(nSegmentCount-1))
        {
            aPoints[nPoint].X = static_cast< sal_Int32 >( P3.X());
            aPoints[nPoint].Y = static_cast< sal_Int32 >( P3.Y());
            aFlags [nPoint++] = drawing::PolygonFlags_NORMAL;
        }
    }

    aReturn.Coordinates[0] = aPoints;
    aReturn.Flags[0] = aFlags;

    return aReturn;
}

drawing::PolyPolygonBezierCoords getRingBezierCoords(
            double fInnerXRadius
            , double fOuterXRadius, double fOuterYRadius
            , double fWidthAngleRadian, double fStartAngleRadian
            , double fPosX, double fPosY )
{
    drawing::PolyPolygonBezierCoords aReturn = drawing::PolyPolygonBezierCoords();

    aReturn.Coordinates = drawing::PointSequenceSequence(1);
    aReturn.Flags       = drawing::FlagSequenceSequence(1);

    Matrix3D aTransformationFromUnitCircle_Outer;
    aTransformationFromUnitCircle_Outer.Rotate(fStartAngleRadian);
    aTransformationFromUnitCircle_Outer.ScaleX(fOuterXRadius);
    aTransformationFromUnitCircle_Outer.ScaleY(fOuterYRadius);
    aTransformationFromUnitCircle_Outer.TranslateX(fPosX);
    aTransformationFromUnitCircle_Outer.TranslateY(fPosY);

    drawing::PolyPolygonBezierCoords aOuterArc = getCircularArcBezierCoords( fWidthAngleRadian, aTransformationFromUnitCircle_Outer );
    aReturn.Coordinates[0] = aOuterArc.Coordinates[0];
    aReturn.Flags[0] = aOuterArc.Flags[0];

    Matrix3D aTransformationFromUnitCircle_Inner;
    aTransformationFromUnitCircle_Inner.Rotate(fStartAngleRadian);
    aTransformationFromUnitCircle_Inner.ScaleX(fInnerXRadius);
    aTransformationFromUnitCircle_Inner.ScaleY(fInnerXRadius*fOuterYRadius/fOuterXRadius);
    aTransformationFromUnitCircle_Inner.TranslateX(fPosX);
    aTransformationFromUnitCircle_Inner.TranslateY(fPosY);

    drawing::PolyPolygonBezierCoords aInnerArc = getCircularArcBezierCoords( fWidthAngleRadian, aTransformationFromUnitCircle_Inner );
    appendBezierCoords( aReturn, aInnerArc, sal_True );

    return aReturn;
}
/*
XPolygon ImpCalcXPoly(const Rectangle& rRect1, long nStart, long nEnd)
{
    long rx=rRect1.GetWidth()/2;  // Da GetWidth()/GetHeight() jeweils 1
    long ry=rRect1.GetHeight()/2; // draufaddieren wird korrekt gerundet.
    long a=0,e=3600;
    {
        a=nStart/10;
        e=nEnd/10;
        {
            // Drehrichtung umkehren, damit Richtungssinn genauso wie Rechteck
            rx=-rx;
            a=1800-a; if (a<0) a+=3600;
            e=1800-e; if (e<0) e+=3600;
            long nTmp=a;
            a=e;
            e=nTmp;
        }
    }
    FASTBOOL bClose=FALSE;
    XPolygon aXPoly(rRect1.Center(),rx,ry,USHORT(a),USHORT(e),bClose);
    if (nStart==nEnd)
    {
        Point aMerk(aXPoly[0]);
        aXPoly=XPolygon(2);
        aXPoly[0]=rRect1.Center();
        aXPoly[1]=aMerk;
    }
    { // Der Sektor soll Start/Ende im Zentrum haben
        // Polygon um einen Punkt rotieren (Punkte im Array verschieben)
        unsigned nPointAnz=aXPoly.GetPointCount();
        aXPoly.Insert(0,rRect1.Center(),XPOLY_NORMAL);
        aXPoly[aXPoly.GetPointCount()]=rRect1.Center();
    }
     // Die Winkelangaben beziehen sich immer auf die linke obere Ecke von !aRect!
//  if (aGeo.nShearWink!=0) ShearXPoly(aXPoly,aRect.TopLeft(),aGeo.nTan);
//  if (aGeo.nDrehWink!=0) RotateXPoly(aXPoly,aRect.TopLeft(),aGeo.nSin,aGeo.nCos);
     return aXPoly;
}
*/
uno::Reference< drawing::XShape >
        ShapeFactory::createPieSegment2D(
                    const uno::Reference< drawing::XShapes >& xTarget
                    , const DataPointGeometry& rGeometry )
{
    /*
    DataPointGeometry aLogicGeom( drawing::Position3D(0.0,0.0,0.0)
                                , drawing::Direction3D(fOuterXDiameter,fOuterYDiameter,fDepth)
                                , drawing::Direction3D(fInnerXDiameter,fStartAngleDegree,fWidthAngleDegree) );
    */

    double fInnerXRadius      = rGeometry.m_aSize2.DirectionX/2.0;
    //only test
    //fInnerXRadius = 0.0;
    //if( fInnerXRadius > 0.0 ) return NULL;

    double fOuterXRadius      = rGeometry.m_aSize.DirectionX/2.0;
    double fOuterYRadius      = rGeometry.m_aSize.DirectionY/2.0;

    double fStartAngleDegree = rGeometry.m_aSize2.DirectionY;
    double fWidthAngleDegree = rGeometry.m_aSize2.DirectionZ;

    /*
    DBG_ASSERT(fOuterXRadius>0, "The radius of a pie needs to be > 0");
    DBG_ASSERT(fInnerXRadius>=0, "The inner radius of a pie needs to be >= 0");
    DBG_ASSERT(fOuterXRadius>fInnerXRadius, "The outer radius needs to be greater than the inner radius of a pie");
    DBG_ASSERT(fOuterYRadius>0, "The radius of a pie needs to be > 0");

    DBG_ASSERT(fWidthAngleDegree>0, "The angle of a pie needs to be > 0");
    DBG_ASSERT(fWidthAngleDegree<=360, "The angle of a pie needs to be <= 360 degree");
    */

    while(fWidthAngleDegree>360)
        fWidthAngleDegree -= 360.0;
    while(fWidthAngleDegree<0)
        fWidthAngleDegree += 360.0;

    //create shape
    uno::Reference< drawing::XShape > xShape(
            m_xShapeFactory->createInstance(
//            C2U("com.sun.star.drawing.EllipseShape") ), uno::UNO_QUERY );
//            C2U("com.sun.star.drawing.PolyPolygonShape") ), uno::UNO_QUERY );
//            C2U("com.sun.star.drawing.OpenFreeHandShape") ), uno::UNO_QUERY );
                C2U("com.sun.star.drawing.ClosedBezierShape") ), uno::UNO_QUERY );
//                C2U("com.sun.star.drawing.LineShape") ), uno::UNO_QUERY );
//                C2U("com.sun.star.drawing.PolyLineShape") ), uno::UNO_QUERY );

    //need to add the shape before setting of properties
    xTarget->add(xShape);

    //need left upper corner not the middle of the circle (!! x and y are assumed to scale in the same way here)
    double fXPos = rGeometry.m_aPosition.PositionX;
    double fYPos = rGeometry.m_aPosition.PositionY;

    double fXSize = rGeometry.m_aSize.DirectionX;
    double fYSize = rGeometry.m_aSize.DirectionY;

    fXPos -= fXSize/2.0;
    fYPos -= fYSize/2.0;

    //set properties
    uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
    DBG_ASSERT(xProp.is(), "created shape offers no XPropertySet");
    if( xProp.is())
    {
        try
        {
            /*
            Matrix3D aTransformationFromUnitCircle;
            aTransformationFromUnitCircle.Rotate(fStartAngleDegree*F_PI/180.0);
            aTransformationFromUnitCircle.ScaleX(fInnerXRadius+fXWidthRadius);
            aTransformationFromUnitCircle.ScaleY(fInnerXRadius+fXWidthRadius);
            aTransformationFromUnitCircle.TranslateX(rGeometry.m_aPosition.PositionX);
            aTransformationFromUnitCircle.TranslateY(rGeometry.m_aPosition.PositionY);
            drawing::PolyPolygonBezierCoords aCoords = getCircularArcBezierCoords( fWidthAngleDegree * F_PI/180.0
                , aTransformationFromUnitCircle );
            */

            drawing::PolyPolygonBezierCoords aCoords = getRingBezierCoords(
                fInnerXRadius, fOuterXRadius, fOuterYRadius
                , fWidthAngleDegree*F_PI/180.0, fStartAngleDegree*F_PI/180.0
                , rGeometry.m_aPosition.PositionX, rGeometry.m_aPosition.PositionY);
            xProp->setPropertyValue( C2U( "PolyPolygonBezier" )
                , uno::makeAny( aCoords ) );

            /*
            Rectangle aRect(fXPos,fYPos+fYSize,fXPos+fXSize,fYPos);
            XPolygon aXPolygon = ImpCalcXPoly(aRect
                , fStartAngleDegree*100.0
                , (fStartAngleDegree+fWidthAngleDegree)*100.0);

            sal_Int32 nCount = aXPolygon.GetSize();
            if( nCount > 0 )
            {
                */
                /*
                drawing::PointSequence aSeq( nCount );
                awt::Point* pSequence = aSeq.getArray();

                for(sal_Int32 b=0;b<nCount;b++)
                {
                    *pSequence = awt::Point( aXPolygon[b].X(), aXPolygon[b].Y() );
                    pSequence++;
                }
                */
                /*

                drawing::PointSequence aSeq( 4 );
                awt::Point* pSequence = aSeq.getArray();
                pSequence[0] = awt::Point( 0, 0 );
                pSequence[1] = awt::Point( 10000, -10000 );
                pSequence[2] = awt::Point( 10000, 0 );
                pSequence[3] = awt::Point( 0, 0 );


                //UNO_NAME_POLYGON drawing::PointSequence*
                xProp->setPropertyValue( C2U( UNO_NAME_POLYGON )
                , uno::makeAny( aSeq ) );
            }
            */

/*
UNO_NAME_POLYGONKIND "PolygonKind" drawing::PolygonKind* (readonly)

UNO_NAME_POLYPOLYGON "PolyPolygon" drawing::PointSequenceSequence*
UNO_NAME_POLYGON "Polygon" drawing::PointSequence*

UNO_NAME_POLYPOLYGONBEZIER "PolyPolygonBezier" drawing::PolyPolygonBezierCoords*
*/

            /*
            //UNO_NAME_CIRCKIND drawing::CircleKind
            xProp->setPropertyValue( C2U( UNO_NAME_CIRCKIND )
                , uno::makeAny(drawing::CircleKind_SECTION) );

            double factor = 100.0;

            //UNO_NAME_CIRCSTARTANGLE sal_Int32
            xProp->setPropertyValue( C2U( UNO_NAME_CIRCSTARTANGLE )
                , uno::makeAny((sal_Int32)(fStartAngleDegree*factor)) );

            //UNO_NAME_CIRCENDANGLE sal_Int32
            xProp->setPropertyValue( C2U( UNO_NAME_CIRCENDANGLE )
                , uno::makeAny((sal_Int32)((fStartAngleDegree+fWidthAngleDegree)*factor)) );
                */
        }
        catch( uno::Exception& e )
        {
            ASSERT_EXCEPTION( e );
        }
    }

//    xShape->setPosition(awt::Point(rGeometry.m_aPosition.PositionX,rGeometry.m_aPosition.PositionY));

//    xShape->setPosition(awt::Point(fXPos,fYPos));
//    xShape->setSize(awt::Size(fXSize,fYSize));

    /*
    xShape->setPosition(awt::Point(100,100));
    xShape->setSize(awt::Size(10000,10000));
    */

    /*
    //create shape
    uno::Reference< drawing::XShape > xShape(
            m_xShapeFactory->createInstance(
            //C2U("com.sun.star.drawing.EllipseShape") ), uno::UNO_QUERY );
            C2U("com.sun.star.drawing.Shape3DExtrudeObject") ), uno::UNO_QUERY );

    xTarget->add(xShape);

    //set properties
    uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
    DBG_ASSERT(xProp.is(), "created shape offers no XPropertySet");
    if( xProp.is())
    {
        try
        {
            //UNO_NAME_CIRCKIND drawing::CircleKind
            xProp->setPropertyValue( C2U( UNO_NAME_CIRCKIND )
                , uno::makeAny(drawing::CircleKind_SECTION) );

            double factor = 100.0;

            //UNO_NAME_CIRCSTARTANGLE sal_Int32
            xProp->setPropertyValue( C2U( UNO_NAME_CIRCSTARTANGLE )
                , uno::makeAny((sal_Int32)(fStartAngleDegree*factor)) );

            //UNO_NAME_CIRCENDANGLE sal_Int32
            xProp->setPropertyValue( C2U( UNO_NAME_CIRCENDANGLE )
                , uno::makeAny((sal_Int32)((fStartAngleDegree+fWidthAngleDegree)*factor)) );
        }
        catch( uno::Exception& e )
        {
            e;
        }
    }
    //need left upper corner not the middle of the circle (!! x and y are assumed to scale in the same way here)
    double fXPos = rGeometry.m_aPosition.PositionX;
    double fYPos = rGeometry.m_aPosition.PositionY;

    double fXSize = rGeometry.m_aSize.DirectionX;
    double fYSize = rGeometry.m_aSize.DirectionY;

    fXPos -= fXSize/2.0;
    fYPos -= fYSize/2.0;

    xShape->setPosition(awt::Point(fXPos,fYPos));
    xShape->setSize(awt::Size(fXSize,fYSize));
    */
    return xShape;
}

uno::Reference< drawing::XShape >
        ShapeFactory::createPieSegment(
                    const uno::Reference< drawing::XShapes >& xTarget
                    , const DataPointGeometry& rGeometry )
{
    double fInnerXRadius      = rGeometry.m_aSize2.DirectionX/2.0;
    double fXWidthRadius      = rGeometry.m_aSize.DirectionX/2.0 - fInnerXRadius;

    double fDepth           = rGeometry.m_aSize.DirectionZ;

    double fStartAngleDegree = rGeometry.m_aSize2.DirectionY;
    double fWidthAngleDegree = rGeometry.m_aSize2.DirectionZ;

    DBG_ASSERT(fDepth>0, "The height of a pie needs to be > 0");
    DBG_ASSERT(fXWidthRadius>0, "The radius of a pie needs to be > 0");
    DBG_ASSERT(fInnerXRadius>=0, "The inner radius of a pie needs to be > 0");
    DBG_ASSERT(fWidthAngleDegree>0, "The angle of a pie needs to be > 0");
    DBG_ASSERT(fWidthAngleDegree<=360, "The angle of a pie needs to be <= 360 degree");

    while(fWidthAngleDegree>360)
        fWidthAngleDegree -= 360.0;
    while(fWidthAngleDegree<0)
        fWidthAngleDegree += 360.0;

    //create shape
    uno::Reference< drawing::XShape > xShape(
            m_xShapeFactory->createInstance(
            C2U("com.sun.star.drawing.Shape3DLatheObject") ), uno::UNO_QUERY );

    xTarget->add(xShape);

    //set properties
    uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
    DBG_ASSERT(xProp.is(), "created shape offers no XPropertySet");
    if( xProp.is())
    {
        try
        {
            //PercentDiagonal
            xProp->setPropertyValue( C2U( UNO_NAME_3D_PERCENT_DIAGONAL )
                , uno::makeAny((sal_Int16)0) );

            //Polygon
            xProp->setPropertyValue( C2U( UNO_NAME_3D_POLYPOLYGON3D )
                , createPolyPolygon_PieSegment( fDepth, fXWidthRadius, fInnerXRadius ) );

            //UNO_NAME_3D_LATHE_END_ANGLE sal_Int16
            xProp->setPropertyValue( C2U( UNO_NAME_3D_LATHE_END_ANGLE )
                , uno::makeAny((sal_Int16)(fWidthAngleDegree*10.0)) );

            //Matrix for position
            {
                Matrix4D aM4;
                aM4.ScaleZ(rGeometry.m_aSize.DirectionY/rGeometry.m_aSize.DirectionX);
                aM4.RotateY( ZDIRECTION*fStartAngleDegree*F_PI/180.0 );
                aM4.Translate(rGeometry.m_aPosition.PositionX, rGeometry.m_aPosition.PositionZ, rGeometry.m_aPosition.PositionY);
                drawing::HomogenMatrix aHM = Matrix4DToHomogenMatrix(aM4);
                xProp->setPropertyValue( C2U( UNO_NAME_3D_TRANSFORM_MATRIX )
                    , uno::makeAny(aHM) );
            }

            //Segments
            xProp->setPropertyValue( C2U( UNO_NAME_3D_VERT_SEGS )
                , uno::makeAny(CHART_3DOBJECT_SEGMENTCOUNT) );
            xProp->setPropertyValue( C2U( UNO_NAME_3D_HORZ_SEGS )
                , uno::makeAny(CHART_3DOBJECT_SEGMENTCOUNT) );
        }
        catch( uno::Exception& e )
        {
            ASSERT_EXCEPTION( e );
        }
    }
    return xShape;
}

uno::Reference< drawing::XShape >
        ShapeFactory::createStripe( const uno::Reference< drawing::XShapes >& xTarget
                    , const Stripe& rStripe
                    , const uno::Reference< beans::XPropertySet >& xSourceProp
                    , const tPropertyNameMap& rPropertyNameMap
                    , sal_Bool bDoubleSided )
{
    //create shape
    uno::Reference< drawing::XShape > xShape(
            m_xShapeFactory->createInstance( C2U(
            "com.sun.star.drawing.Shape3DPolygonObject" ) ), uno::UNO_QUERY );
    xTarget->add(xShape);

    //set properties
    uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
    DBG_ASSERT(xProp.is(), "created shape offers no XPropertySet");
    if( xProp.is())
    {
        try
        {
            //Polygon
            xProp->setPropertyValue( C2U( UNO_NAME_3D_POLYPOLYGON3D )
                , rStripe.getPolyPolygonShape3D() );

            //Normals Polygon
            xProp->setPropertyValue( C2U( UNO_NAME_3D_NORMALSPOLYGON3D )
                , rStripe.getNormalsPolyPolygonShape3D() );

            //NormalsKind
            xProp->setPropertyValue( C2U( UNO_NAME_3D_NORMALS_KIND )
                , uno::makeAny( drawing::NormalsKind_FLAT ) );

            //LineOnly
            xProp->setPropertyValue( C2U( UNO_NAME_3D_LINEONLY )
                , uno::makeAny( (sal_Bool)false) );

            //DoubleSided
            xProp->setPropertyValue( C2U( UNO_NAME_3D_DOUBLE_SIDED )
                , uno::makeAny(bDoubleSided) );

            if( xSourceProp.is())
                PropertyMapper::setMappedProperties( xProp, xSourceProp, rPropertyNameMap );
        }
        catch( uno::Exception& e )
        {
            ASSERT_EXCEPTION( e );
        }
    }
    return xShape;
}

uno::Reference< drawing::XShape >
        ShapeFactory::createArea3D( const uno::Reference< drawing::XShapes >& xTarget
                    , const drawing::PolyPolygonShape3D& rPolyPolygon
                    , double fDepth )
{
    //create shape
    uno::Reference< drawing::XShape > xShape(
        m_xShapeFactory->createInstance( C2U(
            "com.sun.star.drawing.Shape3DExtrudeObject") ), uno::UNO_QUERY );
    xTarget->add(xShape);

    //set properties
    uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
    DBG_ASSERT(xProp.is(), "created shape offers no XPropertySet");
    if( xProp.is())
    {
        try
        {
            //depth
            xProp->setPropertyValue( C2U( UNO_NAME_3D_EXTRUDE_DEPTH )
                , uno::makeAny((sal_Int32)fDepth) );

            //PercentDiagonal
            sal_Int16 nPercentDiagonal = 0;
            xProp->setPropertyValue( C2U( UNO_NAME_3D_PERCENT_DIAGONAL )
                , uno::makeAny( nPercentDiagonal ) );

            //Polygon
            xProp->setPropertyValue( C2U( UNO_NAME_3D_POLYPOLYGON3D )
                , uno::makeAny( rPolyPolygon ) );
        }
        catch( uno::Exception& e )
        {
            ASSERT_EXCEPTION( e );
        }
    }
    return xShape;
}

uno::Reference< drawing::XShape >
        ShapeFactory::createArea2D( const uno::Reference< drawing::XShapes >& xTarget
                    , const drawing::PolyPolygonShape3D& rPolyPolygon )
{
    //create shape
    uno::Reference< drawing::XShape > xShape(
        m_xShapeFactory->createInstance( C2U(
            "com.sun.star.drawing.PolyPolygonShape") ), uno::UNO_QUERY );
    xTarget->add(xShape);

    //set properties
    uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
    DBG_ASSERT(xProp.is(), "created shape offers no XPropertySet");
    if( xProp.is())
    {
        try
        {
            //UNO_NAME_POLYGON "Polygon" drawing::PointSequence*
            drawing::PointSequenceSequence aPoints( PolyToPointSequence(rPolyPolygon) );

            //Polygon
            xProp->setPropertyValue( C2U( UNO_NAME_POLYPOLYGON )
                , uno::makeAny( aPoints ) );

            //ZOrder
            //an area should always be behind other shapes
            xProp->setPropertyValue( C2U( UNO_NAME_MISC_OBJ_ZORDER )
                , uno::makeAny( sal_Int32(0) ) );
        }
        catch( uno::Exception& e )
        {
            ASSERT_EXCEPTION( e );
        }
    }
    return xShape;
}

drawing::PolyPolygonShape3D createPolyPolygon_Symbol( const drawing::Position3D& rPos
                                 , const drawing::Direction3D& rSize
                                 , SymbolType eSymbolType )
{
    const double& fX = rPos.PositionX;
    const double& fY = rPos.PositionY;

    const double fWidthH  = rSize.DirectionX/2.0; //fWidthH stands for Half Width
    const double fHeightH = rSize.DirectionY/2.0; //fHeightH stands for Half Height

    DBG_ASSERT(eSymbolType!=SYMBOL_NONE, "do not create a symbol with type SYMBOL_NONE");
    if(eSymbolType==SYMBOL_NONE)
        eSymbolType=SYMBOL_SQUARE;

    sal_Int32 nPointCount = 4; //all arrow symbols only need 4 points
    switch( eSymbolType )
    {
        case SYMBOL_SQUARE:
        case SYMBOL_DIAMOND:
        case SYMBOL_BOWTIE:
        case SYMBOL_SANDGLASS:
            nPointCount = 5;
            break;
        default:
            break;
    }

    //--------------------------------------
    drawing::PolyPolygonShape3D aPP;

    aPP.SequenceX.realloc(1);
    aPP.SequenceY.realloc(1);
    aPP.SequenceZ.realloc(1);

    drawing::DoubleSequence* pOuterSequenceX = aPP.SequenceX.getArray();
    drawing::DoubleSequence* pOuterSequenceY = aPP.SequenceY.getArray();
    drawing::DoubleSequence* pOuterSequenceZ = aPP.SequenceZ.getArray();

    pOuterSequenceX->realloc(nPointCount);
    pOuterSequenceY->realloc(nPointCount);
    pOuterSequenceZ->realloc(nPointCount);

    double* pInnerSequenceX = pOuterSequenceX->getArray();
    double* pInnerSequenceY = pOuterSequenceY->getArray();
    double* pInnerSequenceZ = pOuterSequenceZ->getArray();

    for(sal_Int32 nN = nPointCount; nN--;)
        *pInnerSequenceZ++ = 0.0;

    switch(eSymbolType)
    {
        case SYMBOL_ARROW_UP:
        {
            *pInnerSequenceX++ = fX-fWidthH;
            *pInnerSequenceY++ = fY+fHeightH;

            *pInnerSequenceX++ = fX+fWidthH;
            *pInnerSequenceY++ = fY+fHeightH;

            *pInnerSequenceX++ = fX;
            *pInnerSequenceY++ = fY-fHeightH;

            *pInnerSequenceX++ = fX-fWidthH;
            *pInnerSequenceY++ = fY+fHeightH;
            break;
        }
        case SYMBOL_ARROW_DOWN:
        {
            *pInnerSequenceX++ = fX-fWidthH;
            *pInnerSequenceY++ = fY-fHeightH;

            *pInnerSequenceX++ = fX;
            *pInnerSequenceY++ = fY+fHeightH;

            *pInnerSequenceX++ = fX+fWidthH;
            *pInnerSequenceY++ = fY-fHeightH;

            *pInnerSequenceX++ = fX-fWidthH;
            *pInnerSequenceY++ = fY-fHeightH;
            break;
        }
        case SYMBOL_ARROW_RIGHT:
        {
            *pInnerSequenceX++ = fX-fWidthH;
            *pInnerSequenceY++ = fY-fHeightH;

            *pInnerSequenceX++ = fX-fWidthH;
            *pInnerSequenceY++ = fY+fHeightH;

            *pInnerSequenceX++ = fX+fWidthH;
            *pInnerSequenceY++ = fY;

            *pInnerSequenceX++ = fX-fWidthH;
            *pInnerSequenceY++ = fY-fHeightH;
            break;
        }
        case SYMBOL_ARROW_LEFT:
        {
            *pInnerSequenceX++ = fX-fWidthH;
            *pInnerSequenceY++ = fY;

            *pInnerSequenceX++ = fX+fWidthH;
            *pInnerSequenceY++ = fY+fHeightH;

            *pInnerSequenceX++ = fX+fWidthH;
            *pInnerSequenceY++ = fY-fHeightH;

            *pInnerSequenceX++ = fX-fWidthH;
            *pInnerSequenceY++ = fY;
            break;
        }
        case SYMBOL_BOWTIE:
        {
            *pInnerSequenceX++ = fX-fWidthH;
            *pInnerSequenceY++ = fY-fHeightH;

            *pInnerSequenceX++ = fX-fWidthH;
            *pInnerSequenceY++ = fY+fHeightH;

            *pInnerSequenceX++ = fX+fWidthH;
            *pInnerSequenceY++ = fY-fHeightH;

            *pInnerSequenceX++ = fX+fWidthH;
            *pInnerSequenceY++ = fY+fHeightH;

            *pInnerSequenceX++ = fX-fWidthH;
            *pInnerSequenceY++ = fY-fHeightH;
            break;
        }
        case SYMBOL_SANDGLASS:
        {
            *pInnerSequenceX++ = fX-fWidthH;
            *pInnerSequenceY++ = fY+fHeightH;

            *pInnerSequenceX++ = fX+fWidthH;
            *pInnerSequenceY++ = fY+fHeightH;

            *pInnerSequenceX++ = fX-fWidthH;
            *pInnerSequenceY++ = fY-fHeightH;


            *pInnerSequenceX++ = fX+fWidthH;
            *pInnerSequenceY++ = fY-fHeightH;

            *pInnerSequenceX++ = fX-fWidthH;
            *pInnerSequenceY++ = fY+fHeightH;
            break;
        }
        case SYMBOL_DIAMOND:
        {
            *pInnerSequenceX++ = fX-fWidthH;
            *pInnerSequenceY++ = fY;

            *pInnerSequenceX++ = fX;
            *pInnerSequenceY++ = fY+fHeightH;

            *pInnerSequenceX++ = fX+fWidthH;
            *pInnerSequenceY++ = fY;

            *pInnerSequenceX++ = fX;
            *pInnerSequenceY++ = fY-fHeightH;

            *pInnerSequenceX++ = fX-fWidthH;
            *pInnerSequenceY++ = fY;
            break;
        }
        default: //case SYMBOL_SQUARE:
        {
            *pInnerSequenceX++ = fX-fWidthH;
            *pInnerSequenceY++ = fY-fHeightH;

            *pInnerSequenceX++ = fX-fWidthH;
            *pInnerSequenceY++ = fY+fHeightH;

            *pInnerSequenceX++ = fX+fWidthH;
            *pInnerSequenceY++ = fY+fHeightH;

            *pInnerSequenceX++ = fX+fWidthH;
            *pInnerSequenceY++ = fY-fHeightH;

            *pInnerSequenceX++ = fX-fWidthH;
            *pInnerSequenceY++ = fY-fHeightH;
            break;
        }
    }
    //return uno::Any( &aPP, ::getCppuType((const drawing::PolyPolygonShape3D*)0) );
    return aPP;
}

uno::Reference< drawing::XShape >
        ShapeFactory::createSymbol2D(
                      const uno::Reference< drawing::XShapes >& xTarget
                    , const drawing::Position3D& rPosition
                    , const drawing::Direction3D& rSize
                    , const SymbolType& eSymbolType )
{
    //create shape
    uno::Reference< drawing::XShape > xShape(
        m_xShapeFactory->createInstance( C2U(
            "com.sun.star.drawing.PolyPolygonShape") ), uno::UNO_QUERY );
    xTarget->add(xShape);

    //set properties
    uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
    DBG_ASSERT(xProp.is(), "created shape offers no XPropertySet");
    if( xProp.is())
    {
        try
        {
            drawing::PointSequenceSequence aPoints( PolyToPointSequence(
                createPolyPolygon_Symbol( rPosition, rSize, eSymbolType ) ));

            //Polygon
            xProp->setPropertyValue( C2U( UNO_NAME_POLYPOLYGON )
                , uno::makeAny( aPoints ) );
        }
        catch( uno::Exception& e )
        {
            ASSERT_EXCEPTION( e );
        }
    }
    return xShape;
}

uno::Reference< drawing::XShape >
        ShapeFactory::createSymbol3D( const uno::Reference< drawing::XShapes >& xTarget
                    , const drawing::Position3D& rPosition
                    , const drawing::Direction3D& rSize
                    , const SymbolType& eSymbolType )
{
    //create shape
    uno::Reference< drawing::XShape > xShape(
        m_xShapeFactory->createInstance( C2U(
            "com.sun.star.drawing.Shape3DExtrudeObject") ), uno::UNO_QUERY );
    xTarget->add(xShape);

    //set properties
    uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
    DBG_ASSERT(xProp.is(), "created shape offers no XPropertySet");
    if( xProp.is())
    {
        try
        {
            //depth
            xProp->setPropertyValue( C2U( UNO_NAME_3D_EXTRUDE_DEPTH )
                , uno::makeAny((sal_Int32)rSize.DirectionZ) );

            //PercentDiagonal
            sal_Int16 nPercentDiagonal = 0;
            xProp->setPropertyValue( C2U( UNO_NAME_3D_PERCENT_DIAGONAL )
                , uno::makeAny( nPercentDiagonal ) );

            //Polygon
            xProp->setPropertyValue( C2U( UNO_NAME_3D_POLYPOLYGON3D )
                , uno::makeAny( createPolyPolygon_Symbol( rPosition, rSize, eSymbolType ) ) );
        }
        catch( uno::Exception& e )
        {
            ASSERT_EXCEPTION( e );
        }
    }
    return xShape;
}

void setShapeName( uno::Reference< drawing::XShape >& xShape , const ::rtl::OUString& rName )
{
    uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
    DBG_ASSERT(xProp.is(), "created shape offers no XPropertySet");
    if( xProp.is())
    {
        try
        {
            xProp->setPropertyValue( C2U( UNO_NAME_MISC_OBJ_NAME )
                , uno::makeAny( rName ) );
        }
        catch( uno::Exception& e )
        {
            ASSERT_EXCEPTION( e );
        }
    }
}

uno::Reference< drawing::XShapes >
        ShapeFactory::createGroup2D( const uno::Reference< drawing::XShapes >& xTarget
        , ::rtl::OUString aName )
{
    //create and add to target
    uno::Reference< drawing::XShape > xShape(
                m_xShapeFactory->createInstance( C2U(
                "com.sun.star.drawing.GroupShape" ) ), uno::UNO_QUERY );
    xTarget->add(xShape);

    //set name
    if(aName.getLength())
        setShapeName( xShape , aName );

    {//workaround
        //need this null size as otherwise empty group shapes where painted with a gray border
        xShape->setSize(awt::Size(0,0));
    }

    //return
    uno::Reference< drawing::XShapes > xShapes =
        uno::Reference<drawing::XShapes>( xShape, uno::UNO_QUERY );
    return xShapes;
}

uno::Reference< drawing::XShapes >
        ShapeFactory::createGroup3D( const uno::Reference< drawing::XShapes >& xTarget
        , ::rtl::OUString aName )
{
    //create shape
    uno::Reference< drawing::XShape > xShape(
            m_xShapeFactory->createInstance( C2U(
            "com.sun.star.drawing.Shape3DSceneObject" ) ), uno::UNO_QUERY );

    xTarget->add(xShape);

    //it is necessary to set the transform matrix to initialize the scene properly (bug #106316#)
    //otherwise all objects which are placed into this Group will not be visible
    //the following should be unnecessary after a the bug is fixed
    {
        //set properties
        uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
        DBG_ASSERT(xProp.is(), "created shape offers no XPropertySet");
        if( xProp.is())
        {
            try
            {
                Matrix4D aM4;
                xProp->setPropertyValue( C2U( UNO_NAME_3D_TRANSFORM_MATRIX )
                    , uno::makeAny(Matrix4DToHomogenMatrix(aM4)) );
            }
            catch( uno::Exception& e )
            {
                ASSERT_EXCEPTION( e );
            }
        }
    }

    //set name
    if(aName.getLength())
        setShapeName( xShape , aName );

    //return
    uno::Reference< drawing::XShapes > xShapes =
            uno::Reference<drawing::XShapes>( xShape, uno::UNO_QUERY );
    return xShapes;
}

uno::Reference< drawing::XShape >
        ShapeFactory::createLine3D( const uno::Reference< drawing::XShapes >& xTarget
                    , const drawing::PolyPolygonShape3D& rPoints
                    , const VLineProperties& rLineProperties )
{
    if(!rPoints.SequenceX.getLength())
        return NULL;

    //create shape
    uno::Reference< drawing::XShape > xShape(
        m_xShapeFactory->createInstance( C2U(
            "com.sun.star.drawing.Shape3DPolygonObject") ), uno::UNO_QUERY );
    xTarget->add(xShape);

    //set properties
    uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
    DBG_ASSERT(xProp.is(), "created shape offers no XPropertySet");
    if( xProp.is())
    {
        try
        {
            //Polygon
            xProp->setPropertyValue( C2U( UNO_NAME_3D_POLYPOLYGON3D )
                , uno::makeAny( rPoints ) );

            //LineOnly
            xProp->setPropertyValue( C2U( UNO_NAME_3D_LINEONLY )
                , uno::makeAny( (sal_Bool)true ) );

            //Transparency
            if(rLineProperties.Transparence.hasValue())
                xProp->setPropertyValue( C2U( UNO_NAME_LINETRANSPARENCE )
                    , rLineProperties.Transparence );

            //LineStyle
            if(rLineProperties.LineStyle.hasValue())
                xProp->setPropertyValue( C2U( UNO_NAME_LINESTYLE )
                    , rLineProperties.LineStyle );

            //LineWidth
            if(rLineProperties.Width.hasValue())
                xProp->setPropertyValue( C2U( UNO_NAME_LINEWIDTH )
                    , rLineProperties.Width );

            //LineColor
            if(rLineProperties.Color.hasValue())
                xProp->setPropertyValue( C2U( UNO_NAME_LINECOLOR )
                    , rLineProperties.Color );
                    //, uno::makeAny( sal_Int32( Color(COL_RED).GetColor()) ) );
        }
        catch( uno::Exception& e )
        {
            ASSERT_EXCEPTION( e );
        }
    }
    return xShape;
}

uno::Reference< drawing::XShape >
        ShapeFactory::createLine2D( const uno::Reference< drawing::XShapes >& xTarget
                    , const drawing::PointSequenceSequence& rPoints
                    , const VLineProperties& rLineProperties )
{
    if(!rPoints.getLength())
        return NULL;

    //create shape
    uno::Reference< drawing::XShape > xShape(
        m_xShapeFactory->createInstance( C2U(
            //"com.sun.star.drawing.LineShape") ), uno::UNO_QUERY );
            "com.sun.star.drawing.PolyLineShape") ), uno::UNO_QUERY );
            //"com.sun.star.drawing.PolyLinePathShape") ), uno::UNO_QUERY );
            //"com.sun.star.drawing.PolyPolygonPathShape") ), uno::UNO_QUERY );
            //"com.sun.star.drawing.PolyPolygonShape") ), uno::UNO_QUERY );
    xTarget->add(xShape);

    //set properties
    uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
    DBG_ASSERT(xProp.is(), "created shape offers no XPropertySet");
    if( xProp.is())
    {
        try
        {
            //Polygon
            xProp->setPropertyValue( C2U( UNO_NAME_POLYPOLYGON )
                , uno::makeAny( rPoints ) );

            //Transparency
            if(rLineProperties.Transparence.hasValue())
                xProp->setPropertyValue( C2U( UNO_NAME_LINETRANSPARENCE )
                    , rLineProperties.Transparence );

            //LineStyle
            if(rLineProperties.LineStyle.hasValue())
                xProp->setPropertyValue( C2U( UNO_NAME_LINESTYLE )
                    , rLineProperties.LineStyle );

            //LineWidth
            if(rLineProperties.Width.hasValue())
                xProp->setPropertyValue( C2U( UNO_NAME_LINEWIDTH )
                    , rLineProperties.Width );

            //LineColor
            if(rLineProperties.Color.hasValue())
                xProp->setPropertyValue( C2U( UNO_NAME_LINECOLOR )
                    , rLineProperties.Color );

            //LineDash
            if(rLineProperties.Dash.hasValue())
                xProp->setPropertyValue( C2U( UNO_NAME_LINEDASH )
                    , rLineProperties.Dash );
        }
        catch( uno::Exception& e )
        {
            ASSERT_EXCEPTION( e );
        }
    }
    return xShape;
}

uno::Any ShapeFactory::makeTransformation( const awt::Point& rScreenPosition2D, double fRotationAnglePi )
{
    Matrix3D aM3;
    //As autogrow is active the rectangle is automatically expanded to that side
    //to which the text is not adjusted.
    aM3.Scale( 1, 1 );
    aM3.Rotate( fRotationAnglePi );
    aM3.Translate( rScreenPosition2D.X, rScreenPosition2D.Y );
    uno::Any aATransformation = uno::makeAny( Matrix3DToHomogenMatrix3(aM3) );
    return aATransformation;
}

uno::Reference< drawing::XShape >
        ShapeFactory::createText( const uno::Reference< drawing::XShapes >& xTarget
                    , const ::rtl::OUString& rText
                    , const tNameSequence& rPropNames
                    , const tAnySequence& rPropValues
                    , const uno::Any& rATransformation )
{
    //create shape and add to page
    uno::Reference< drawing::XShape > xShape(
            m_xShapeFactory->createInstance( C2U(
            "com.sun.star.drawing.TextShape" ) ), uno::UNO_QUERY );
    xTarget->add(xShape);

    //set text
    uno::Reference< text::XTextRange > xTextRange( xShape, uno::UNO_QUERY );
    if( xTextRange.is() )
        xTextRange->setString( rText );

    uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
    if( xProp.is() )
    {
        //set properties
        PropertyMapper::setMultiProperties( rPropNames, rPropValues, xProp );

        //set position matrix
        //the matrix needs to be set at the end behind autogrow and such position influencing properties
        try
        {
            xProp->setPropertyValue( C2U( "Transformation" ), rATransformation );
        }
        catch( uno::Exception& e )
        {
            ASSERT_EXCEPTION( e );
        }
    }
    return xShape;
        /*
    //create 3D anchor shape
    uno::Reference< drawing::XShape > xShape3DAnchor = createCube( xTarget3D
            , DataPointGeometry( rGeometry.m_aPosition,drawing::Direction3D(1000,1000,1000) )
            , ...);
    //get 2D position from xShape3DAnchor
    //the 2D position of the 3D Shape is not correct initially this is a bug in draw @todo
    //awt::Point aPosition2D( xShape3DAnchor->getPosition() );
    */

    //--
    /*
    uno::Reference< text::XTextRange > xTextRange( xShape2DText, uno::UNO_QUERY );
    uno::Reference< text::XText > xText11( xShape2DText, uno::UNO_QUERY );
    if( xTextRange.is() )
    {
        uno::Reference< text::XText > xText = xTextRange->getText();

        xText->insertString( xTextRange, C2U( "Hello" ), true );
        xText->insertControlCharacter( xTextRange,
            text::ControlCharacter::LINE_BREAK, false );
        xText->insertString( xTextRange, C2U( "World" ), false );

        rtl::OUString aTmp = xText->getString();
        int a=1;
    }
    */

    //--
    /*
    //get the created paragraph and set the character properties there
    uno::Reference< container::XEnumerationAccess > xEnumerationAccess( xShape, uno::UNO_QUERY );
    uno::Reference< container::XEnumeration > xEnumeration = xEnumerationAccess->createEnumeration();
    uno::Any aAParagraph = xEnumeration->nextElement();
    uno::Reference< text::XTextContent > xTextContent = NULL;
    aAParagraph >>= xTextContent;
    uno::Reference< beans::XPropertySet > xParaProp( xTextContent, uno::UNO_QUERY );
    if(xParaProp.is())
    {
        try
        {
            //test:
            //ControlTextEmphasis
            //FontEmphasisMark
            //CharEmphasis
            xParaProp->setPropertyValue( C2U( "CharEmphasize" ),
                uno::makeAny( text::FontEmphasis::DOT_ABOVE ) );

            xParaProp->setPropertyValue( C2U( "FontEmphasisMark" ),
                uno::makeAny( sal_Int16(2) ) );

            xParaProp->setPropertyValue( C2U( "ControlTextEmphasis" ),
                uno::makeAny( sal_Int16(2) ) );
        }
        catch( uno::Exception& e )
        {
            e;
        }
    }
    */
}

//static
rtl::OUString ShapeFactory::getStackedString( const rtl::OUString& rString, bool bStacked )
{
    sal_Int32 nLen = rString.getLength();
    if(!bStacked || !nLen)
        return rString;

    rtl::OUStringBuffer aStackStr;
    rtl::OUStringBuffer aSource(rString);

    //add a newline after each letter
    //as we do not no letters here add a newline after each char
    for( sal_Int32 nPosSrc=0; nPosSrc < nLen; nPosSrc++ )
    {
        aStackStr.append( aSource.charAt( nPosSrc ) );
        aStackStr.append( sal_Unicode('\r') );
    }
    return aStackStr.makeStringAndClear();
}

//.............................................................................
} //namespace chart
//.............................................................................
