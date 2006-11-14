/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ShapeFactory.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 15:36:22 $
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
#include "ShapeFactory.hxx"
#include "ViewDefines.hxx"
#include "Stripe.hxx"
#include "CommonConverters.hxx"
#include "macros.hxx"
#include "InlineContainer.hxx"
#include "PropertyMapper.hxx"

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
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
#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif

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

#ifndef _SVDOCIRC_HXX
#include <svx/svdocirc.hxx>
#endif
#ifndef _SVDOPATH_HXX
#include <svx/svdopath.hxx>
#endif

#ifndef _BGFX_VECTOR_B2DVECTOR_HXX
#include <basegfx/vector/b2dvector.hxx>
#endif

#ifndef _BGFX_MATRIX_B3DHOMMATRIX_HXX
#include <basegfx/matrix/b3dhommatrix.hxx>
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
    if(!xShape.is())
        return;
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

    //fWidthH stands for Half Width
    const double fWidthH = rSize.DirectionX >=0.0?  rSize.DirectionX/2.0  : -rSize.DirectionX/2.0;
    const double fHeight = rSize.DirectionY;
    const double fDepth  = rSize.DirectionZ >=0.0?  rSize.DirectionZ      : -rSize.DirectionZ ;

    const double fHeightSign = fHeight >= 0.0 ? 1.0 : -1.0;

    const double fOffset = (fDepth * fRoundedEdge) * 1.05;  // increase by 5% for safety
    const bool bRoundEdges = fRoundedEdge && fOffset < fWidthH && 2.0 * fOffset < fHeightSign*fHeight;
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
        *pInnerSequenceY++ = fHeightSign*fOffset;
        *pInnerSequenceY++ = fHeight - fHeightSign*fOffset;
        *pInnerSequenceY++ = fHeight;
        *pInnerSequenceY++ = fHeight; //6.
        *pInnerSequenceY++ = fHeight;
        *pInnerSequenceY++ = fHeight;
        *pInnerSequenceY++ = fHeight - fHeightSign*fOffset;
        *pInnerSequenceY++ = fHeightSign*fOffset; //10.
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

    fRoundedEdge = 0.0;
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
            double fDepth = rGeometry.m_aSize.DirectionZ;
            if(fDepth<0)
                fDepth*=-1.0;
            xProp->setPropertyValue( C2U( UNO_NAME_3D_EXTRUDE_DEPTH )
                , uno::makeAny((sal_Int32)fDepth) );

            //PercentDiagonal
            sal_Int16 nPercentDiagonal = bRounded ? 5 : 0;
            xProp->setPropertyValue( C2U( UNO_NAME_3D_PERCENT_DIAGONAL )
                , uno::makeAny( nPercentDiagonal ) );

            //Polygon
            xProp->setPropertyValue( C2U( UNO_NAME_3D_POLYPOLYGON3D )
                , createPolyPolygon_Cube( rGeometry.m_aSize, double(nPercentDiagonal)/200.0,bRounded) );

            //Matrix for position
            {
                ::basegfx::B3DHomMatrix aM4;
                aM4.translate(rGeometry.m_aPosition.PositionX
                            , rGeometry.m_aPosition.PositionY
                            , rGeometry.m_aPosition.PositionZ - (fDepth/2.0));
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
                ::basegfx::B3DHomMatrix aM4;
                //if(ntest%2) //llllllllllllllllllll
                //aM4.RotateY( fYRotateAnglePi );

                // Note: Uncomment the following to let the objects grow in z
                // direction to fill the diagram
//                 aM4.ScaleZ(rGeometry.m_aSize.DirectionZ/rGeometry.m_aSize.DirectionX);
                aM4.translate(rGeometry.m_aPosition.PositionX, rGeometry.m_aPosition.PositionY, rGeometry.m_aPosition.PositionZ);
                drawing::HomogenMatrix aHM = Matrix4DToHomogenMatrix(aM4);
                xProp->setPropertyValue( C2U( UNO_NAME_3D_TRANSFORM_MATRIX )
                    , uno::makeAny(aHM) );
            }

            //Segments
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

//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------------------------------------

drawing::PolyPolygonBezierCoords getCircularArcBezierCoords(
        double fAngleRadian
        , const ::basegfx::B2DHomMatrix& rTransformationFromUnitCircle )
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

    ::basegfx::B2DVector P0,P1,P2,P3;
    ::basegfx::B2DVector POrigin = rTransformationFromUnitCircle * ::basegfx::B2DVector(0.0, 0.0);

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
        P0.setX(fCos);
        P3.setX(fCos);
        P0.setY(-fSin);
        P3.setY(-P0.getY());

        P1.setX((4.0-fCos)/3.0);
        P2.setX(P1.getX());
        P1.setY((1.0-fCos)*(fCos-3.0)/(3.0*fSin));
        P2.setY(-P1.getY());
        //transform thus startangle equals NULL
        ::basegfx::B2DHomMatrix aStart;
        aStart.rotate(fCurrentSegmentAngle/2.0 + nSegment*fSmallAngleRadian);

        //apply given transformation to get final points
        P0 = rTransformationFromUnitCircle*(aStart*P0);
        P1 = rTransformationFromUnitCircle*(aStart*P1);
        P2 = rTransformationFromUnitCircle*(aStart*P2);
        P3 = rTransformationFromUnitCircle*(aStart*P3);

        aPoints[nPoint].X = static_cast< sal_Int32 >( P0.getX());
        aPoints[nPoint].Y = static_cast< sal_Int32 >( P0.getY());
        aFlags [nPoint++] = drawing::PolygonFlags_NORMAL;

        aPoints[nPoint].X = static_cast< sal_Int32 >( P1.getX());
        aPoints[nPoint].Y = static_cast< sal_Int32 >( P1.getY());
        aFlags[nPoint++] = drawing::PolygonFlags_CONTROL;

        aPoints[nPoint].X = static_cast< sal_Int32 >( P2.getX());
        aPoints[nPoint].Y = static_cast< sal_Int32 >( P2.getY());
        aFlags [nPoint++] = drawing::PolygonFlags_CONTROL;

        if(nSegment==(nSegmentCount-1))
        {
            aPoints[nPoint].X = static_cast< sal_Int32 >( P3.getX());
            aPoints[nPoint].Y = static_cast< sal_Int32 >( P3.getY());
            aFlags [nPoint++] = drawing::PolygonFlags_NORMAL;
        }
    }

    aReturn.Coordinates[0] = aPoints;
    aReturn.Flags[0] = aFlags;

    return aReturn;
}

//------------------------------------------------------------------------------------------------------------

drawing::PolyPolygonBezierCoords getRingBezierCoords(
            double fInnerXRadius
            , double fOuterXRadius, double fOuterYRadius
            , double fWidthAngleRadian, double fStartAngleRadian
            , double fPosX, double fPosY )
{
    drawing::PolyPolygonBezierCoords aReturn = drawing::PolyPolygonBezierCoords();

    aReturn.Coordinates = drawing::PointSequenceSequence(1);
    aReturn.Flags       = drawing::FlagSequenceSequence(1);

    ::basegfx::B2DHomMatrix aTransformationFromUnitCircle_Outer;
    aTransformationFromUnitCircle_Outer.rotate(fStartAngleRadian);
    aTransformationFromUnitCircle_Outer.scale(fOuterXRadius, fOuterYRadius);
    aTransformationFromUnitCircle_Outer.translate(fPosX, fPosY);

    drawing::PolyPolygonBezierCoords aOuterArc = getCircularArcBezierCoords( fWidthAngleRadian, aTransformationFromUnitCircle_Outer );
    aReturn.Coordinates[0] = aOuterArc.Coordinates[0];
    aReturn.Flags[0] = aOuterArc.Flags[0];

    ::basegfx::B2DHomMatrix aTransformationFromUnitCircle_Inner;
    aTransformationFromUnitCircle_Inner.rotate(fStartAngleRadian);
    aTransformationFromUnitCircle_Inner.scale(fInnerXRadius, fInnerXRadius*fOuterYRadius/fOuterXRadius);
    aTransformationFromUnitCircle_Inner.translate(fPosX, fPosY);

    drawing::PolyPolygonBezierCoords aInnerArc = getCircularArcBezierCoords( fWidthAngleRadian, aTransformationFromUnitCircle_Inner );
    appendBezierCoords( aReturn, aInnerArc, sal_True );

    return aReturn;
}

//------------------------------------------------------------------------------------------------------------

uno::Reference< drawing::XShape >
        ShapeFactory::createPieSegment2D(
                    const uno::Reference< drawing::XShapes >& xTarget
                    , double fStartAngleDegree, double fWidthAngleDegree
                    , double fInnerXRadius, double fOuterXRadius, double fOuterYRadius
                    , const drawing::Position3D& rOrigin )
{
    while(fWidthAngleDegree>360)
        fWidthAngleDegree -= 360.0;
    while(fWidthAngleDegree<0)
        fWidthAngleDegree += 360.0;

    //create shape
    uno::Reference< drawing::XShape > xShape(
            m_xShapeFactory->createInstance(
                C2U("com.sun.star.drawing.ClosedBezierShape") ), uno::UNO_QUERY );
    xTarget->add(xShape); //need to add the shape before setting of properties

    //set properties
    uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
    DBG_ASSERT(xProp.is(), "created shape offers no XPropertySet");
    if( xProp.is())
    {
        try
        {
            drawing::PolyPolygonBezierCoords aCoords = getRingBezierCoords(
                fInnerXRadius, fOuterXRadius, fOuterYRadius
                , fWidthAngleDegree*F_PI/180.0, fStartAngleDegree*F_PI/180.0
                , rOrigin.PositionX, rOrigin.PositionY);

            xProp->setPropertyValue( C2U( "PolyPolygonBezier" ), uno::makeAny( aCoords ) );
        }
        catch( uno::Exception& e )
        {
            ASSERT_EXCEPTION( e );
        }
    }
    return xShape;
}

//------------------------------------------------------------------------------------------------------------

uno::Reference< drawing::XShape >
        ShapeFactory::createPieSegment(
                    const uno::Reference< drawing::XShapes >& xTarget
                    , double fStartAngleDegree, double fWidthAngleDegree
                    , double fInnerXRadius, double fOuterXRadius, double fOuterYRadius
                    , const drawing::Position3D& rOrigin
                    , double fDepth )
{
    while(fWidthAngleDegree>360)
        fWidthAngleDegree -= 360.0;
    while(fWidthAngleDegree<0)
        fWidthAngleDegree += 360.0;

    //create shape
    uno::Reference< drawing::XShape > xShape(
            m_xShapeFactory->createInstance(
            C2U("com.sun.star.drawing.Shape3DLatheObject") ), uno::UNO_QUERY );
    xTarget->add(xShape); //need to add the shape before setting of properties

    //set properties
    uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
    DBG_ASSERT(xProp.is(), "created shape offers no XPropertySet");
    if( xProp.is())
    {
        try
        {
            double fXWidthRadius      = fOuterXRadius - fInnerXRadius;

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
                ::basegfx::B3DHomMatrix aM4;
                aM4.scale(1.0, 1.0, fOuterYRadius/fOuterXRadius);
                aM4.rotate( 0.0, ZDIRECTION*fStartAngleDegree*F_PI/180.0, 0.0 );
                aM4.translate(rOrigin.PositionX, rOrigin.PositionZ, rOrigin.PositionY);
                drawing::HomogenMatrix aHM = Matrix4DToHomogenMatrix(aM4);
                xProp->setPropertyValue( C2U( UNO_NAME_3D_TRANSFORM_MATRIX )
                    , uno::makeAny(aHM) );
            }

            //Segments
            xProp->setPropertyValue( C2U( UNO_NAME_3D_HORZ_SEGS )
                , uno::makeAny(CHART_3DOBJECT_SEGMENTCOUNT) );
            xProp->setPropertyValue( C2U( UNO_NAME_3D_VERT_SEGS )
                , uno::makeAny((sal_Int32)20) );//@todo replace this expensive count by 1 if draw bug is fixed
        }
        catch( uno::Exception& e )
        {
            ASSERT_EXCEPTION( e );
        }
    }
    return xShape;
}

//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------

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

            //DoubleSided
            xProp->setPropertyValue( C2U( UNO_NAME_3D_DOUBLE_SIDED )
                , uno::makeAny( (sal_Bool)true) );
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

enum SymbolType { SYMBOL_SQUARE=0
                 , SYMBOL_DIAMOND
                 , SYMBOL_ARROW_DOWN
                 , SYMBOL_ARROW_UP
                 , SYMBOL_ARROW_RIGHT
                 , SYMBOL_ARROW_LEFT
                 , SYMBOL_BOWTIE
                 , SYMBOL_SANDGLASS
                 , SYMBOL_COUNT
                  };

sal_Int32 ShapeFactory::getSymbolCount()
{
    return SYMBOL_COUNT;
}

drawing::PolyPolygonShape3D createPolyPolygon_Symbol( const drawing::Position3D& rPos
                                 , const drawing::Direction3D& rSize
                                 , sal_Int32 nStandardSymbol )
{
    if(nStandardSymbol<0)
        nStandardSymbol*=-1;
    nStandardSymbol = nStandardSymbol%ShapeFactory::getSymbolCount();
    SymbolType eSymbolType=static_cast<SymbolType>(nStandardSymbol);

    const double& fX = rPos.PositionX;
    const double& fY = rPos.PositionY;

    const double fWidthH  = rSize.DirectionX/2.0; //fWidthH stands for Half Width
    const double fHeightH = rSize.DirectionY/2.0; //fHeightH stands for Half Height

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
                    , sal_Int32 nStandardSymbol
                    , sal_Int32 nFillColor )
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
                createPolyPolygon_Symbol( rPosition, rSize, nStandardSymbol ) ));

            //Polygon
            xProp->setPropertyValue( C2U( UNO_NAME_POLYPOLYGON )
                , uno::makeAny( aPoints ) );

            //FillColor
            xProp->setPropertyValue( C2U( UNO_NAME_FILLCOLOR )
                , uno::makeAny( nFillColor ) );
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
                    , sal_Int32 nStandardSymbol
                    , sal_Int32 nFillColor )
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
                , uno::makeAny( createPolyPolygon_Symbol( rPosition, rSize, nStandardSymbol ) ) );

            //FillColor
            xProp->setPropertyValue( C2U( UNO_NAME_FILLCOLOR )
                , uno::makeAny( nFillColor ) );
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
                ::basegfx::B3DHomMatrix aM4;
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

uno::Reference< drawing::XShape > ShapeFactory::createErrorBar2D(
      const uno::Reference< drawing::XShapes >& xTarget
    , const drawing::Position3D& rPos
    , const drawing::Direction3D& rSize
    , tErrorBarDirection eDirection
    , bool bClip
    )
//     , const tNameSequence& rPropNames
//     , const tAnySequence& rPropValues )
{
    const double fX = rPos.PositionX;
    const double fY = rPos.PositionY;
    const double fZ = rPos.PositionZ;

    const double fWidth  = rSize.DirectionX;
    const double fHeight = rSize.DirectionY;
    const double fWidthHalf = fWidth/2.0;
    const double fHeightHalf = fHeight/2.0;

    //create shape and add to page
    uno::Reference< drawing::XShape > xShape(
            m_xShapeFactory->createInstance( C2U(
            "com.sun.star.drawing.PolyLineShape" ) ), uno::UNO_QUERY );
    xTarget->add(xShape);

    drawing::PolyPolygonShape3D aPPShape;

    sal_Int32 nNumOfPolys = bClip ? 1 : 2;
    aPPShape.SequenceX.realloc( nNumOfPolys );
    aPPShape.SequenceY.realloc( nNumOfPolys );
    aPPShape.SequenceZ.realloc( nNumOfPolys );

    aPPShape.SequenceX[0].realloc(2);
    aPPShape.SequenceY[0].realloc(2);
    aPPShape.SequenceZ[0].realloc(2);
    aPPShape.SequenceZ[0][0] = aPPShape.SequenceZ[0][1] = fZ;

    if( !bClip )
    {
        aPPShape.SequenceX[1].realloc(2);
        aPPShape.SequenceY[1].realloc(2);
        aPPShape.SequenceZ[1].realloc(2);
        aPPShape.SequenceZ[1][0] = aPPShape.SequenceZ[1][1] = fZ;
    }


    switch( eDirection )
    {
        case ERROR_BAR_UP:
            // body
            aPPShape.SequenceX[0][0] = aPPShape.SequenceX[0][1] = fX;
            aPPShape.SequenceY[0][0] = fY;
            aPPShape.SequenceY[0][1] = fY - fHeight;
            // head
            if( !bClip )
            {
                aPPShape.SequenceX[1][0] = fX - fWidthHalf;
                aPPShape.SequenceX[1][1] = fX + fWidthHalf;
                aPPShape.SequenceY[1][0] = aPPShape.SequenceY[1][1] = fY - fHeight;
            }
            break;

        case ERROR_BAR_RIGHT:
            // body
            aPPShape.SequenceX[0][0] = fX;
            aPPShape.SequenceX[0][1] = fX + fWidth;
            aPPShape.SequenceY[0][0] = aPPShape.SequenceY[0][1] = fY;
            // head
            if( !bClip )
            {
                aPPShape.SequenceX[1][0] = aPPShape.SequenceX[1][1] = fX + fWidth;
                aPPShape.SequenceY[1][0] = fY - fHeightHalf;
                aPPShape.SequenceY[1][1] = fY + fHeightHalf;
            }
            break;

        case ERROR_BAR_DOWN:
            // body
            aPPShape.SequenceX[0][0] = aPPShape.SequenceX[0][1] = fX;
            aPPShape.SequenceY[0][0] = fY;
            aPPShape.SequenceY[0][1] = fY + fHeight;
            // head
            if( !bClip )
            {
                aPPShape.SequenceX[1][0] = fX - fWidthHalf;
                aPPShape.SequenceX[1][1] = fX + fWidthHalf;
                aPPShape.SequenceY[1][0] = aPPShape.SequenceY[1][1] = fY + fHeight;
            }
            break;

        case ERROR_BAR_LEFT:
            // body
            aPPShape.SequenceX[0][0] = fX;
            aPPShape.SequenceX[0][1] = fX - fWidth;
            aPPShape.SequenceY[0][0] = aPPShape.SequenceY[0][1] = fY;
            // head
            if( !bClip )
            {
                aPPShape.SequenceX[1][0] = aPPShape.SequenceX[1][1] = fX - fWidth;
                aPPShape.SequenceY[1][0] = fY - fHeightHalf;
                aPPShape.SequenceY[1][1] = fY + fHeightHalf;
            }
            break;
    }

    uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
    if( xProp.is() )
    {
        //set properties
//         PropertyMapper::setMultiProperties( rPropNames, rPropValues, xProp );

        //set polygon-shape
        try
        {
            drawing::PointSequenceSequence aPoints( PolyToPointSequence( aPPShape ));

            //Polygon
            xProp->setPropertyValue( C2U( UNO_NAME_POLYPOLYGON ), uno::makeAny( aPoints ) );
//             xProp->setPropertyValue( C2U( "PolyPolygon" ), uno::makeAny( aPtSeq ));
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
    ::basegfx::B2DHomMatrix aM3;
    //As autogrow is active the rectangle is automatically expanded to that side
    //to which the text is not adjusted.
    // aM3.scale( 1, 1 ); Oops? A scale with this parameters is neutral, line commented out
    aM3.rotate( fRotationAnglePi );
    aM3.translate( rScreenPosition2D.X, rScreenPosition2D.Y );
    uno::Any aATransformation = uno::makeAny( B2DHomMatrixToHomogenMatrix3(aM3) );
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
