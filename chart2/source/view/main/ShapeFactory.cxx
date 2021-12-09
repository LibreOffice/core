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

#include <ShapeFactory.hxx>
#include <BaseGFXHelper.hxx>
#include <ViewDefines.hxx>
#include <Stripe.hxx>
#include <CommonConverters.hxx>
#include <RelativeSizeHelper.hxx>
#include <PropertyMapper.hxx>
#include <VLineProperties.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/chart2/XFormattedString.hpp>
#include <com/sun/star/drawing/CircleKind.hpp>
#include <com/sun/star/drawing/DoubleSequence.hpp>
#include <com/sun/star/drawing/FlagSequence.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/NormalsKind.hpp>
#include <com/sun/star/drawing/PointSequence.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/drawing/TextureProjectionMode.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XShapes2.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.hxx>

#include <editeng/unoprnms.hxx>
#include <rtl/math.hxx>

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <tools/diagnose_ex.h>
#include <tools/helpers.hxx>
#include <tools/UnitConversion.hxx>
#include <sal/log.hxx>

#include <algorithm>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;

namespace chart
{

namespace
{

void lcl_addProperty(uno::Sequence<OUString> & rPropertyNames, uno::Sequence<uno::Any> & rPropertyValues,
                 OUString const & rName, uno::Any const & rAny)
{
    rPropertyNames.realloc(rPropertyNames.getLength() + 1);
    rPropertyNames.getArray()[rPropertyNames.getLength() - 1] = rName;

    rPropertyValues.realloc(rPropertyValues.getLength() + 1);
    rPropertyValues.getArray()[rPropertyValues.getLength() - 1] = rAny;
}

} // end anonymous namespace

uno::Reference< drawing::XShapes > ShapeFactory::getOrCreateChartRootShape(
    const uno::Reference< drawing::XDrawPage>& xDrawPage )
{
    uno::Reference<drawing::XShapes> xRet = ShapeFactory::getChartRootShape(xDrawPage);
    if (xRet.is())
        return xRet;

    // Create a new root shape and set it to the bottom of the page.  The root
    // shape is identified by having the name com.sun.star.chart2.shapes.
    uno::Reference<drawing::XShape> xShape(
        m_xShapeFactory->createInstance("com.sun.star.drawing.GroupShape"), uno::UNO_QUERY);
    uno::Reference<drawing::XShapes2> xShapes2(xDrawPage, uno::UNO_QUERY_THROW);
    xShapes2->addBottom(xShape);

    setShapeName(xShape, "com.sun.star.chart2.shapes");
    xShape->setSize(awt::Size(0,0));

    xRet.set(xShape, uno::UNO_QUERY);
    return xRet;
}

void ShapeFactory::setPageSize(const uno::Reference<drawing::XShapes>&, const awt::Size&) {}

//  diverse tools::PolyPolygon create methods

static uno::Any createPolyPolygon_Cube(
            const drawing::Direction3D& rSize, double fRoundedEdge, bool bRounded )
{
    OSL_PRECOND(fRoundedEdge>=0, "fRoundedEdge needs to be >= 0");

    // always use extra points, so set percent diagonal to 0.4 which is 0% in the UI (old Chart comment)
    if( fRoundedEdge == 0.0  && bRounded)
        fRoundedEdge = 0.4 / 200.0;
    else if(!bRounded)
        fRoundedEdge = 0.0;

    //fWidthH stands for Half Width
    const double fWidthH = rSize.DirectionX >=0.0?  rSize.DirectionX/2.0  : -rSize.DirectionX/2.0;
    const double fHeight = rSize.DirectionY;

    const double fHeightSign = fHeight >= 0.0 ? 1.0 : -1.0;

    const double fOffset = (fWidthH * fRoundedEdge) * 1.05; // increase by 5% for safety
    const bool bRoundEdges = fRoundedEdge && fOffset < fWidthH && 2.0 * fOffset < fHeightSign*fHeight;
    const sal_Int32 nPointCount = bRoundEdges ? 13 : 5;

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

    if(nPointCount == 5)
    {
        *pInnerSequenceY++ = 0.0;
        *pInnerSequenceY++ = 0.0;
        *pInnerSequenceY++ = fHeight;
        *pInnerSequenceY++ = fHeight;
        *pInnerSequenceY++ = 0.0;

        *pInnerSequenceX++ = -fWidthH;
        *pInnerSequenceX++ = fWidthH;
        *pInnerSequenceX++ = fWidthH;
        *pInnerSequenceX++ = -fWidthH;
        *pInnerSequenceX++ = -fWidthH;
    }
    else
    {
        *pInnerSequenceY++ = 0.0;
        *pInnerSequenceY++ = 0.0;
        *pInnerSequenceY++ = 0.0;
        *pInnerSequenceY++ = fHeightSign*fOffset;
        *pInnerSequenceY++ = fHeight - fHeightSign*fOffset;
        *pInnerSequenceY++ = fHeight;
        *pInnerSequenceY++ = fHeight;
        *pInnerSequenceY++ = fHeight;
        *pInnerSequenceY++ = fHeight;
        *pInnerSequenceY++ = fHeight - fHeightSign*fOffset;
        *pInnerSequenceY++ = fHeightSign*fOffset;
        *pInnerSequenceY++ = 0.0;
        *pInnerSequenceY++ = 0.0;

        *pInnerSequenceX++ = -fWidthH + fOffset;
        *pInnerSequenceX++ = fWidthH - fOffset;
        *pInnerSequenceX++ = fWidthH;
        *pInnerSequenceX++ = fWidthH;
        *pInnerSequenceX++ = fWidthH;
        *pInnerSequenceX++ = fWidthH;
        *pInnerSequenceX++ = fWidthH - fOffset;
        *pInnerSequenceX++ = -fWidthH + fOffset;
        *pInnerSequenceX++ = -fWidthH;
        *pInnerSequenceX++ = -fWidthH;
        *pInnerSequenceX++ = -fWidthH;
        *pInnerSequenceX++ = -fWidthH;
        *pInnerSequenceX++ = -fWidthH + fOffset;
    }
    return uno::Any( &aPP, cppu::UnoType<drawing::PolyPolygonShape3D>::get());
}

static uno::Any createPolyPolygon_Cylinder(
             double fHeight
           , double fRadius
           , sal_Int32& nVerticalSegmentCount )
{
    //fHeight may be negative
    OSL_PRECOND(fRadius>0, "The radius of a cylinder needs to be > 0");

    drawing::PolyPolygonShape3D aPP;

    nVerticalSegmentCount=1;

    aPP.SequenceX.realloc(3);
    aPP.SequenceY.realloc(3);
    aPP.SequenceZ.realloc(3);

    drawing::DoubleSequence* pOuterSequenceX = aPP.SequenceX.getArray();
    drawing::DoubleSequence* pOuterSequenceY = aPP.SequenceY.getArray();
    drawing::DoubleSequence* pOuterSequenceZ = aPP.SequenceZ.getArray();

    pOuterSequenceX->realloc(2);
    pOuterSequenceY->realloc(2);
    pOuterSequenceZ->realloc(2);

    double* pInnerSequenceX = pOuterSequenceX->getArray();
    double* pInnerSequenceY = pOuterSequenceY->getArray();
    double* pInnerSequenceZ = pOuterSequenceZ->getArray();

    double fY1 = 0.0;
    double fY2 = fHeight;

    if( fHeight<0.0 )
        std::swap(fY1,fY2);

    for(sal_Int32 nN = 2; nN--;)
        *pInnerSequenceZ++ = 0.0;

    *pInnerSequenceX++ = 0.0;
    *pInnerSequenceY++ = fY1;

    *pInnerSequenceX++ = fRadius;
    *pInnerSequenceY++ = fY1;

    pOuterSequenceX++;pOuterSequenceY++;pOuterSequenceZ++;
    pOuterSequenceX->realloc(2);
    pOuterSequenceY->realloc(2);
    pOuterSequenceZ->realloc(2);

    pInnerSequenceX = pOuterSequenceX->getArray();
    pInnerSequenceY = pOuterSequenceY->getArray();
    pInnerSequenceZ = pOuterSequenceZ->getArray();

    for(sal_Int32 nN = 2; nN--;)
        *pInnerSequenceZ++ = 0.0;

    *pInnerSequenceX++ = fRadius;
    *pInnerSequenceY++ = fY1;

    *pInnerSequenceX++ = fRadius;
    *pInnerSequenceY++ = fY2;

    pOuterSequenceX++;pOuterSequenceY++;pOuterSequenceZ++;
    pOuterSequenceX->realloc(2);
    pOuterSequenceY->realloc(2);
    pOuterSequenceZ->realloc(2);

    pInnerSequenceX = pOuterSequenceX->getArray();
    pInnerSequenceY = pOuterSequenceY->getArray();
    pInnerSequenceZ = pOuterSequenceZ->getArray();

    for(sal_Int32 nN = 2; nN--;)
        *pInnerSequenceZ++ = 0.0;

    *pInnerSequenceX++ = fRadius;
    *pInnerSequenceY++ = fY2;

    *pInnerSequenceX++ = 0.0;
    *pInnerSequenceY++ = fY2;

    return uno::Any( &aPP, cppu::UnoType<drawing::PolyPolygonShape3D>::get());
}

static uno::Any createPolyPolygon_Cone( double fHeight, double fRadius, double fTopHeight
            , sal_Int32& nVerticalSegmentCount )
{
    OSL_PRECOND(fRadius>0, "The radius of a cone needs to be > 0");

    //for stacked charts we need cones without top -> fTopHeight != 0 resp. bTopless == true
    //fTopHeight indicates the high of the cutted top only (not the full height)
    bool bTopless = !::rtl::math::approxEqual( fHeight, fHeight + fTopHeight );

    double r1= 0.0, r2 = fRadius;
    if(bTopless)
        // #i63212# fHeight may be negative, fTopHeight is always positive -> use fabs(fHeight)
        r1 = fRadius * fTopHeight/(fabs(fHeight)+fTopHeight);

    nVerticalSegmentCount=1;
    drawing::PolyPolygonShape3D aPP;

    aPP.SequenceX.realloc(2);
    aPP.SequenceY.realloc(2);
    aPP.SequenceZ.realloc(2);

    drawing::DoubleSequence* pOuterSequenceX = aPP.SequenceX.getArray();
    drawing::DoubleSequence* pOuterSequenceY = aPP.SequenceY.getArray();
    drawing::DoubleSequence* pOuterSequenceZ = aPP.SequenceZ.getArray();

    pOuterSequenceX->realloc(2);
    pOuterSequenceY->realloc(2);
    pOuterSequenceZ->realloc(2);

    double* pInnerSequenceX = pOuterSequenceX->getArray();
    double* pInnerSequenceY = pOuterSequenceY->getArray();
    double* pInnerSequenceZ = pOuterSequenceZ->getArray();

    double fX1 = 0.0;
    double fX2 = r2;
    double fX3 = r1;

    double fY1 = 0.0;
    double fY2 = 0.0;
    double fY3 = fHeight;

    if( fHeight<0.0 )
    {
        std::swap(fX1,fX3);
        std::swap(fY1,fY3);
    }

    for(sal_Int32 nN = 2; nN--;)
        *pInnerSequenceZ++ = 0.0;

    *pInnerSequenceY++ = fY1;
    *pInnerSequenceX++ = fX1;

    *pInnerSequenceY++ = fY2;
    *pInnerSequenceX++ = fX2;

    pOuterSequenceX++;pOuterSequenceY++;pOuterSequenceZ++;
    pOuterSequenceX->realloc(2);
    pOuterSequenceY->realloc(2);
    pOuterSequenceZ->realloc(2);

    pInnerSequenceX = pOuterSequenceX->getArray();
    pInnerSequenceY = pOuterSequenceY->getArray();
    pInnerSequenceZ = pOuterSequenceZ->getArray();

    for(sal_Int32 nN = 2; nN--;)
        *pInnerSequenceZ++ = 0.0;

    *pInnerSequenceY++ = fY2;
    *pInnerSequenceX++ = fX2;

    *pInnerSequenceY++ = fY3;
    *pInnerSequenceX++ = fX3;

    return uno::Any( &aPP, cppu::UnoType<drawing::PolyPolygonShape3D>::get());
}

//  methods for 3D shape creation

uno::Reference<drawing::XShape>
        ShapeFactory::createCube(
            const uno::Reference<drawing::XShapes>& xTarget
            , const drawing::Position3D& rPosition, const drawing::Direction3D& rSize
            , sal_Int32 nRotateZAngleHundredthDegree
            , const uno::Reference< beans::XPropertySet >& xSourceProp
            , const tPropertyNameMap& rPropertyNameMap
            , bool bRounded )
{
    if( !xTarget.is() )
        return nullptr;
    if( bRounded )
    {
        try
        {
            if( xSourceProp.is() )
            {
                drawing::LineStyle aLineStyle;
                xSourceProp->getPropertyValue( "BorderStyle" ) >>= aLineStyle;
                if( aLineStyle == drawing::LineStyle_SOLID )
                    bRounded = false;
            }
        }
        catch( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION("chart2", "" );
        }
    }
    uno::Reference<drawing::XShape> xShape = impl_createCube( xTarget, rPosition, rSize, nRotateZAngleHundredthDegree, bRounded );
    uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
    if( xSourceProp.is())
        PropertyMapper::setMappedProperties( xProp, xSourceProp, rPropertyNameMap );
    return xShape;
}

uno::Reference<drawing::XShape>
        ShapeFactory::impl_createCube(
              const uno::Reference<drawing::XShapes>& xTarget
            , const drawing::Position3D& rPosition, const drawing::Direction3D& rSize
            , sal_Int32 nRotateZAngleHundredthDegree
            , bool bRounded )
{
    if( !xTarget.is() )
        return nullptr;

    //create shape
    uno::Reference< drawing::XShape > xShape(
        m_xShapeFactory->createInstance(
            "com.sun.star.drawing.Shape3DExtrudeObject" ), uno::UNO_QUERY );
    xTarget->add(xShape);

    //set properties
    uno::Reference<beans::XMultiPropertySet> xMultiPropertySet(xShape, uno::UNO_QUERY);
    OSL_ENSURE(xMultiPropertySet.is(), "created shape offers no XMultiPropertySet");
    if (xMultiPropertySet.is())
    {
        try
        {
            //depth
            double fDepth = rSize.DirectionZ;
            if (fDepth<0)
                fDepth*=-1.0;

            //PercentDiagonal
            sal_Int16 nPercentDiagonal = bRounded ? 3 : 0;

            //Matrix for position
            basegfx::B3DHomMatrix aHomMatrix;
            if (nRotateZAngleHundredthDegree != 0)
                aHomMatrix.rotate(0.0, 0.0, -basegfx::deg2rad<100>(nRotateZAngleHundredthDegree));
            aHomMatrix.translate(rPosition.PositionX, rPosition.PositionY,
                                 rPosition.PositionZ - (fDepth / 2.0));

            uno::Sequence<OUString> aPropertyNames {
                UNO_NAME_3D_EXTRUDE_DEPTH,
                UNO_NAME_3D_PERCENT_DIAGONAL,
                UNO_NAME_3D_POLYPOLYGON3D,
                UNO_NAME_3D_TRANSFORM_MATRIX,
            };

            uno::Sequence<uno::Any> aPropertyValues {
                uno::Any(sal_Int32(fDepth)), // Depth
                uno::Any(nPercentDiagonal),  // PercentDiagonal
                createPolyPolygon_Cube(rSize, double(nPercentDiagonal) / 200.0, bRounded),
                uno::Any(B3DHomMatrixToHomogenMatrix(aHomMatrix))
            };

            xMultiPropertySet->setPropertyValues(aPropertyNames, aPropertyValues);
        }
        catch( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION("chart2", "" );
        }
    }
    return xShape;
}

uno::Reference<drawing::XShape>
        ShapeFactory::createCylinder(
            const uno::Reference<drawing::XShapes>& xTarget
          , const drawing::Position3D& rPosition, const drawing::Direction3D& rSize
          , sal_Int32 nRotateZAngleHundredthDegree )
{
    return impl_createConeOrCylinder(
              xTarget, rPosition, rSize, 0.0, nRotateZAngleHundredthDegree, true );
}

uno::Reference<drawing::XShape>
        ShapeFactory::createPyramid(
            const uno::Reference<drawing::XShapes>& xTarget
          , const drawing::Position3D& rPosition, const drawing::Direction3D& rSize
          , double fTopHeight, bool bRotateZ
          , const uno::Reference< beans::XPropertySet >& xSourceProp
          , const tPropertyNameMap& rPropertyNameMap )
{
    if( !xTarget.is() )
        return nullptr;

    Reference< drawing::XShapes > xGroup( ShapeFactory::createGroup3D( xTarget ) );

    bool bDoubleSided = false;
    short nRotatedTexture = 0;

    const double fWidth = rSize.DirectionX;
    const double fDepth = rSize.DirectionZ;
    const double fHeight = rSize.DirectionY;

    drawing::Position3D aBottomP1( rPosition.PositionX, rPosition.PositionY, rPosition.PositionZ - fDepth/2.0  );
    if(bRotateZ)
        aBottomP1.PositionY -= fWidth/2.0;
    else
        aBottomP1.PositionX -= fWidth/2.0;
    drawing::Position3D aBottomP2( aBottomP1 );
    if(bRotateZ)
        aBottomP2.PositionY += fWidth;
    else
        aBottomP2.PositionX += fWidth;
    drawing::Position3D aBottomP3( aBottomP2 );
    drawing::Position3D aBottomP4( aBottomP1 );
    aBottomP3.PositionZ += fDepth;
    aBottomP4.PositionZ += fDepth;

    const double fTopFactor = fTopHeight/(fabs(fHeight)+fTopHeight);
    drawing::Position3D aTopP1( rPosition.PositionX, rPosition.PositionY, rPosition.PositionZ - fDepth*fTopFactor/2.0  );
    if(bRotateZ)
    {
        aTopP1.PositionY -= fWidth*fTopFactor/2.0;
        aTopP1.PositionX += fHeight;
    }
    else
    {
        aTopP1.PositionX -= fWidth*fTopFactor/2.0;
        aTopP1.PositionY += fHeight;
    }
    drawing::Position3D aTopP2( aTopP1 );
    if(bRotateZ)
        aTopP2.PositionY += fWidth*fTopFactor;
    else
        aTopP2.PositionX += fWidth*fTopFactor;
    drawing::Position3D aTopP3( aTopP2 );
    drawing::Position3D aTopP4( aTopP1 );
    aTopP3.PositionZ += fDepth*fTopFactor;
    aTopP4.PositionZ += fDepth*fTopFactor;

    Stripe aStripeBottom( aBottomP1, aBottomP4, aBottomP3, aBottomP2 );

    drawing::Position3D aNormalsBottomP1( aBottomP1 );
    drawing::Position3D aNormalsBottomP2( aBottomP2 );
    drawing::Position3D aNormalsBottomP3( aBottomP3 );
    drawing::Position3D aNormalsBottomP4( aBottomP4 );
    drawing::Position3D aNormalsTopP1( aBottomP1 );
    drawing::Position3D aNormalsTopP2( aBottomP2 );
    drawing::Position3D aNormalsTopP3( aBottomP3 );
    drawing::Position3D aNormalsTopP4( aBottomP4 );
    if( bRotateZ )
    {
        aNormalsTopP1.PositionX += fHeight;
        aNormalsTopP2.PositionX += fHeight;
        aNormalsTopP3.PositionX += fHeight;
        aNormalsTopP4.PositionX += fHeight;
    }
    else
    {
        aNormalsTopP1.PositionY += fHeight;
        aNormalsTopP2.PositionY += fHeight;
        aNormalsTopP3.PositionY += fHeight;
        aNormalsTopP4.PositionY += fHeight;
    }

    bool bInvertPolygon = false;
    bool bInvertNormals = false;

    if(bRotateZ)
    {
        //bars
        if(fHeight>=0.0)
        {
            nRotatedTexture = 2;
            bInvertNormals = true;
            aStripeBottom = Stripe( aBottomP1, aBottomP4, aBottomP3, aBottomP2 );
        }
        else
        {
            bInvertPolygon = true;
            nRotatedTexture = 1;
            aStripeBottom = Stripe( aBottomP2, aBottomP3, aBottomP4, aBottomP1 );
        }
    }
    else
    {
        //columns
        if(fHeight>=0.0)
        {
            bInvertPolygon = true;
            nRotatedTexture = 2;
            aStripeBottom = Stripe( aBottomP2, aBottomP3, aBottomP4, aBottomP1 );
        }
        else
        {
            nRotatedTexture = 3;
            bInvertNormals = true;
            aStripeBottom = Stripe( aBottomP4, aBottomP3, aBottomP2, aBottomP1 );
        }
    }
    aStripeBottom.InvertNormal(true);

    Stripe aStripe1( aTopP2, aTopP1, aBottomP1, aBottomP2 );
    Stripe aStripe2( aTopP3, aTopP2, aBottomP2, aBottomP3 );
    Stripe aStripe3( aTopP4, aTopP3, aBottomP3, aBottomP4 );
    Stripe aStripe4( aTopP1, aTopP4, aBottomP4, aBottomP1 );

    if( bInvertPolygon )
    {
        aStripe1 = Stripe( aBottomP1, aTopP1, aTopP2, aBottomP2 );
        aStripe2 = Stripe( aBottomP2, aTopP2, aTopP3, aBottomP3 );
        aStripe3 = Stripe( aBottomP3, aTopP3, aTopP4, aBottomP4 );
        aStripe4 = Stripe( aBottomP4, aTopP4, aTopP1, aBottomP1 );
    }

    Stripe aNormalsStripe1( aNormalsTopP1, aNormalsBottomP1, aNormalsBottomP2, aNormalsTopP2 );
    Stripe aNormalsStripe2( aNormalsTopP2, aNormalsBottomP2, aNormalsBottomP3, aNormalsTopP3 );
    Stripe aNormalsStripe3( aNormalsTopP3, aNormalsBottomP3, aNormalsBottomP4, aNormalsTopP4 );
    Stripe aNormalsStripe4( aNormalsTopP4, aNormalsBottomP4, aNormalsBottomP1, aNormalsTopP1 );

    if( bInvertNormals )
    {
        aNormalsStripe1 = Stripe( aNormalsTopP2, aNormalsBottomP2, aNormalsBottomP1, aNormalsTopP1 );
        aNormalsStripe2 = Stripe( aNormalsTopP3, aNormalsBottomP3, aNormalsBottomP2, aNormalsTopP2 );
        aNormalsStripe3 = Stripe( aNormalsTopP4, aNormalsBottomP4, aNormalsBottomP3, aNormalsTopP3 );
        aNormalsStripe4 = Stripe( aNormalsTopP1, aNormalsBottomP1, aNormalsBottomP4, aNormalsTopP4 );
    }

    aStripe1.SetManualNormal( aNormalsStripe1.getNormal() );
    aStripe2.SetManualNormal( aNormalsStripe2.getNormal() );
    aStripe3.SetManualNormal( aNormalsStripe3.getNormal() );
    aStripe4.SetManualNormal( aNormalsStripe4.getNormal() );

    const bool bFlatNormals = false;
    ShapeFactory::createStripe( xGroup, aStripe1, xSourceProp, rPropertyNameMap, bDoubleSided, nRotatedTexture, bFlatNormals );
    ShapeFactory::createStripe( xGroup, aStripe2, xSourceProp, rPropertyNameMap, bDoubleSided, nRotatedTexture, bFlatNormals );
    ShapeFactory::createStripe( xGroup, aStripe3, xSourceProp, rPropertyNameMap, bDoubleSided, nRotatedTexture, bFlatNormals );
    ShapeFactory::createStripe( xGroup, aStripe4, xSourceProp, rPropertyNameMap, bDoubleSided, nRotatedTexture, bFlatNormals );
    ShapeFactory::createStripe( xGroup, aStripeBottom, xSourceProp, rPropertyNameMap, bDoubleSided, nRotatedTexture, bFlatNormals );

    return Reference< drawing::XShape >( xGroup, uno::UNO_QUERY );
}

uno::Reference<drawing::XShape>
        ShapeFactory::createCone(
            const uno::Reference<drawing::XShapes>& xTarget
          , const drawing::Position3D& rPosition, const drawing::Direction3D& rSize
          , double fTopHeight, sal_Int32 nRotateZAngleHundredthDegree )
{
    return impl_createConeOrCylinder( xTarget, rPosition, rSize, fTopHeight, nRotateZAngleHundredthDegree, false );
}

uno::Reference<drawing::XShape>
        ShapeFactory::impl_createConeOrCylinder(
              const uno::Reference<drawing::XShapes>& xTarget
            , const drawing::Position3D& rPosition, const drawing::Direction3D& rSize
            , double fTopHeight, sal_Int32 nRotateZAngleHundredthDegree
            , bool bCylinder )
{
    if( !xTarget.is() )
        return nullptr;

    //create shape
    uno::Reference< drawing::XShape > xShape(
            m_xShapeFactory->createInstance(
            "com.sun.star.drawing.Shape3DLatheObject" ), uno::UNO_QUERY );
    xTarget->add(xShape);

    double fWidth      = rSize.DirectionX/2.0; //The depth will be corrected within Matrix
    double fRadius     = fWidth; //!!!!!!!! problem in drawing layer: rotation object calculates wrong needed size -> wrong camera (it's a problem with bounding boxes)
    double fHeight     = rSize.DirectionY;

    //set properties
    uno::Reference<beans::XMultiPropertySet> xMultiPropertySet(xShape, uno::UNO_QUERY);
    OSL_ENSURE(xMultiPropertySet.is(), "created shape offers no XMultiPropertySet");
    if (xMultiPropertySet.is())
    {
        try
        {
            //Polygon
            sal_Int32 nVerticalSegmentCount = 0;
            uno::Any aPPolygon = bCylinder
                ? createPolyPolygon_Cylinder(fHeight, fRadius, nVerticalSegmentCount)
                : createPolyPolygon_Cone(fHeight, fRadius, fTopHeight, nVerticalSegmentCount);

            //Matrix for position
            basegfx::B3DHomMatrix aHomMatrix;
            if (nRotateZAngleHundredthDegree != 0)
                aHomMatrix.rotate(0.0,0.0,-basegfx::deg2rad<100>(nRotateZAngleHundredthDegree));
            //stretch the symmetric objects to given depth
            aHomMatrix.scale(1.0,1.0,rSize.DirectionZ/rSize.DirectionX);
            aHomMatrix.translate(rPosition.PositionX, rPosition.PositionY, rPosition.PositionZ);

            uno::Sequence<OUString> aPropertyNames{
                UNO_NAME_3D_PERCENT_DIAGONAL,
                UNO_NAME_3D_POLYPOLYGON3D,
                UNO_NAME_3D_TRANSFORM_MATRIX,
                UNO_NAME_3D_HORZ_SEGS,
                UNO_NAME_3D_VERT_SEGS,
                UNO_NAME_3D_REDUCED_LINE_GEOMETRY
            };

            uno::Sequence<uno::Any> aPropertyValues {
                uno::Any(sal_Int16(5)),  // PercentDiagonal
                aPPolygon,               // Polygon
                uno::Any(B3DHomMatrixToHomogenMatrix(aHomMatrix)), // Matrix
                uno::Any(CHART_3DOBJECT_SEGMENTCOUNT), // Horizontal Segments
                uno::Any(nVerticalSegmentCount),       // Vertical Segments
                uno::Any(true)                         // Reduced lines
            };

            xMultiPropertySet->setPropertyValues(aPropertyNames, aPropertyValues);
        }
        catch( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION("chart2", "" );
        }
    }
    return xShape;
}

static void appendAndCloseBezierCoords( drawing::PolyPolygonBezierCoords& rReturn, const drawing::PolyPolygonBezierCoords& rAdd, bool bAppendInverse )
{
    if(!rAdd.Coordinates.hasElements())
        return;
    sal_Int32 nAddCount = rAdd.Coordinates[0].getLength();
    if(!nAddCount)
        return;

    sal_Int32 nOldCount = rReturn.Coordinates[0].getLength();

    auto pCoordinates = rReturn.Coordinates.getArray();
    pCoordinates[0].realloc(nOldCount + nAddCount + 1);
    auto pCoordinates0 = pCoordinates[0].getArray();
    auto pFlags = rReturn.Flags.getArray();
    pFlags[0].realloc(nOldCount+nAddCount+1);
    auto pFlags0 = pFlags[0].getArray();

    for(sal_Int32 nN=0;nN<nAddCount; nN++ )
    {
        sal_Int32 nAdd = bAppendInverse ? (nAddCount-1-nN) : nN;
        pCoordinates0[nOldCount+nN] = rAdd.Coordinates[0][nAdd];
        pFlags0[nOldCount+nN] = rAdd.Flags[0][nAdd];
    }

    //close
    pCoordinates0[nOldCount+nAddCount] = rReturn.Coordinates[0][0];
    pFlags0[nOldCount+nAddCount] = rReturn.Flags[0][0];
}

static drawing::PolyPolygonBezierCoords getCircularArcBezierCoords(
        double fStartAngleRadian, double fWidthAngleRadian, double fUnitRadius
        , const ::basegfx::B2DHomMatrix& rTransformationFromUnitCircle
        , const double fAngleSubdivisionRadian )
{
    //at least one polygon is created using two normal and two control points
    //if the angle is larger it is separated into multiple sub angles

    drawing::PolyPolygonBezierCoords aReturn;
    sal_Int32 nSegmentCount = static_cast< sal_Int32 >( fWidthAngleRadian/fAngleSubdivisionRadian );
    if( fWidthAngleRadian > fAngleSubdivisionRadian*nSegmentCount )
        nSegmentCount++;

    double fFirstSegmentAngle = fAngleSubdivisionRadian;
    double fLastSegmentAngle = fAngleSubdivisionRadian;
    if(nSegmentCount==1)
    {
        fFirstSegmentAngle = fWidthAngleRadian;
        fLastSegmentAngle = 0.0;
    }
    else
    {
        double fFirstAngleOnSubDivision = (static_cast<sal_Int32>(fStartAngleRadian/fAngleSubdivisionRadian)+1)*fAngleSubdivisionRadian;
        if( !::rtl::math::approxEqual( fStartAngleRadian, fFirstAngleOnSubDivision ) )
            fFirstSegmentAngle = fFirstAngleOnSubDivision-fStartAngleRadian;

        if(nSegmentCount>1)
        {
            fLastSegmentAngle = fWidthAngleRadian-fFirstSegmentAngle-fAngleSubdivisionRadian*(nSegmentCount-2);
            if( fLastSegmentAngle<0 )
                nSegmentCount--;
            if( fLastSegmentAngle>fAngleSubdivisionRadian )
            {
                fLastSegmentAngle-=fAngleSubdivisionRadian;
                nSegmentCount++;
            }
        }
    }

    sal_Int32 nPointCount     = 1 + 3*nSegmentCount; //first point of next segment equals last point of former segment

    drawing::PointSequence aPoints(nPointCount);
    auto pPoints = aPoints.getArray();
    drawing::FlagSequence  aFlags(nPointCount);
    auto pFlags = aFlags.getArray();

    //!! applying matrix to vector does ignore translation, so it is important to use a B2DPoint here instead of B2DVector
    ::basegfx::B2DPoint P0,P1,P2,P3;

    sal_Int32 nPoint=0;
    double fCurrentRotateAngle = fStartAngleRadian;
    for(sal_Int32 nSegment=0; nSegment<nSegmentCount; nSegment++)
    {
        double fCurrentSegmentAngle = fAngleSubdivisionRadian;
        if(nSegment==0)//first segment gets only a smaller peace until the next subdivision
            fCurrentSegmentAngle = fFirstSegmentAngle;
        else if(nSegment==(nSegmentCount-1)) //the last segment gets the rest angle that does not fit into equal pieces
            fCurrentSegmentAngle = fLastSegmentAngle;

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
        aStart.rotate(fCurrentSegmentAngle/2.0 + fCurrentRotateAngle );
        fCurrentRotateAngle+=fCurrentSegmentAngle;

        aStart.scale( fUnitRadius, fUnitRadius );

        //apply given transformation to get final points
        P0 = rTransformationFromUnitCircle*(aStart*P0);
        P1 = rTransformationFromUnitCircle*(aStart*P1);
        P2 = rTransformationFromUnitCircle*(aStart*P2);
        P3 = rTransformationFromUnitCircle*(aStart*P3);

        pPoints[nPoint].X = static_cast< sal_Int32 >( P0.getX());
        pPoints[nPoint].Y = static_cast< sal_Int32 >( P0.getY());
        pFlags [nPoint++] = drawing::PolygonFlags_NORMAL;

        pPoints[nPoint].X = static_cast< sal_Int32 >( P1.getX());
        pPoints[nPoint].Y = static_cast< sal_Int32 >( P1.getY());
        pFlags[nPoint++] = drawing::PolygonFlags_CONTROL;

        pPoints[nPoint].X = static_cast< sal_Int32 >( P2.getX());
        pPoints[nPoint].Y = static_cast< sal_Int32 >( P2.getY());
        pFlags [nPoint++] = drawing::PolygonFlags_CONTROL;

        if(nSegment==(nSegmentCount-1))
        {
            pPoints[nPoint].X = static_cast< sal_Int32 >( P3.getX());
            pPoints[nPoint].Y = static_cast< sal_Int32 >( P3.getY());
            pFlags [nPoint++] = drawing::PolygonFlags_NORMAL;
        }
    }

    aReturn.Coordinates = { aPoints };
    aReturn.Flags = { aFlags };

    return aReturn;
}

static drawing::PolyPolygonBezierCoords getRingBezierCoords(
            double fUnitCircleInnerRadius
            , double fUnitCircleOuterRadius
            , double fStartAngleRadian, double fWidthAngleRadian
            , const ::basegfx::B2DHomMatrix& aTransformationFromUnitCircle
            , const double fAngleSubdivisionRadian )
{
    drawing::PolyPolygonBezierCoords aReturn;

    drawing::PolyPolygonBezierCoords aOuterArc = getCircularArcBezierCoords(
        fStartAngleRadian, fWidthAngleRadian, fUnitCircleOuterRadius, aTransformationFromUnitCircle, fAngleSubdivisionRadian );
    aReturn.Coordinates = { aOuterArc.Coordinates[0] };
    aReturn.Flags = { aOuterArc.Flags[0] };

    drawing::PolyPolygonBezierCoords aInnerArc = getCircularArcBezierCoords(
        fStartAngleRadian, fWidthAngleRadian, fUnitCircleInnerRadius, aTransformationFromUnitCircle, fAngleSubdivisionRadian );
    appendAndCloseBezierCoords( aReturn, aInnerArc, true );

    return aReturn;
}

uno::Reference< drawing::XShape >
        ShapeFactory::createPieSegment2D(
                    const uno::Reference< drawing::XShapes >& xTarget
                    , double fUnitCircleStartAngleDegree, double fUnitCircleWidthAngleDegree
                    , double fUnitCircleInnerRadius, double fUnitCircleOuterRadius
                    , const drawing::Direction3D& rOffset
                    , const drawing::HomogenMatrix& rUnitCircleToScene )
{
    if( !xTarget.is() )
        return nullptr;

    // tdf#123504: both 0 and 360 are valid and different values here!
    while (fUnitCircleWidthAngleDegree > 360)
        fUnitCircleWidthAngleDegree -= 360.0;
    while (fUnitCircleWidthAngleDegree < 0)
        fUnitCircleWidthAngleDegree += 360.0;

    //create shape
    uno::Reference< drawing::XShape > xShape(
            m_xShapeFactory->createInstance(
                "com.sun.star.drawing.ClosedBezierShape" ), uno::UNO_QUERY );
    xTarget->add(xShape); //need to add the shape before setting of properties

    //set properties
    uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
    OSL_ENSURE(xProp.is(), "created shape offers no XPropertySet");
    if( xProp.is())
    {
        try
        {
            ::basegfx::B2DHomMatrix aTransformationFromUnitCircle( IgnoreZ( HomogenMatrixToB3DHomMatrix(rUnitCircleToScene) ) );
            aTransformationFromUnitCircle.translate(rOffset.DirectionX,rOffset.DirectionY);

            const double fAngleSubdivisionRadian = M_PI/10.0;

            drawing::PolyPolygonBezierCoords aCoords
                = getRingBezierCoords(fUnitCircleInnerRadius, fUnitCircleOuterRadius,
                                      basegfx::deg2rad(fUnitCircleStartAngleDegree),
                                      basegfx::deg2rad(fUnitCircleWidthAngleDegree),
                                      aTransformationFromUnitCircle, fAngleSubdivisionRadian);

            xProp->setPropertyValue( "PolyPolygonBezier", uno::Any( aCoords ) );
        }
        catch( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION("chart2", "" );
        }
    }

    return xShape;
}

uno::Reference< drawing::XShape >
        ShapeFactory::createPieSegment(
                    const uno::Reference< drawing::XShapes >& xTarget
                    , double fUnitCircleStartAngleDegree, double fUnitCircleWidthAngleDegree
                    , double fUnitCircleInnerRadius, double fUnitCircleOuterRadius
                    , const drawing::Direction3D& rOffset
                    , const drawing::HomogenMatrix& rUnitCircleToScene
                    , double fDepth )
{
    if( !xTarget.is() )
        return nullptr;

    // tdf#123504: both 0 and 360 are valid and different values here!
    while (fUnitCircleWidthAngleDegree > 360)
        fUnitCircleWidthAngleDegree -= 360.0;
    while (fUnitCircleWidthAngleDegree < 0)
        fUnitCircleWidthAngleDegree += 360.0;

    //create shape
    uno::Reference< drawing::XShape > xShape(
        m_xShapeFactory->createInstance(
            "com.sun.star.drawing.Shape3DExtrudeObject" ), uno::UNO_QUERY );
    xTarget->add(xShape); //need to add the shape before setting of properties

    //set properties
    uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
    OSL_ENSURE(xProp.is(), "created shape offers no XPropertySet");
    if( xProp.is())
    {
        try
        {
            ::basegfx::B2DHomMatrix aTransformationFromUnitCircle( IgnoreZ( HomogenMatrixToB3DHomMatrix(rUnitCircleToScene) ) );
            aTransformationFromUnitCircle.translate(rOffset.DirectionX,rOffset.DirectionY);

            const double fAngleSubdivisionRadian = M_PI/32.0;

            drawing::PolyPolygonBezierCoords aCoords
                = getRingBezierCoords(fUnitCircleInnerRadius, fUnitCircleOuterRadius,
                                      basegfx::deg2rad(fUnitCircleStartAngleDegree),
                                      basegfx::deg2rad(fUnitCircleWidthAngleDegree),
                                      aTransformationFromUnitCircle, fAngleSubdivisionRadian);

            //depth
            xProp->setPropertyValue( UNO_NAME_3D_EXTRUDE_DEPTH
                , uno::Any(static_cast<sal_Int32>(fDepth)) );

            //PercentDiagonal
            xProp->setPropertyValue( UNO_NAME_3D_PERCENT_DIAGONAL
                , uno::Any( sal_Int16(0) ) );

            //Polygon
            drawing::PolyPolygonShape3D aPoly( BezierToPoly(aCoords) );
            ShapeFactory::closePolygon( aPoly );
            xProp->setPropertyValue( UNO_NAME_3D_POLYPOLYGON3D
                , uno::Any( aPoly ) );

            //DoubleSided
            xProp->setPropertyValue( UNO_NAME_3D_DOUBLE_SIDED
                , uno::Any( true ) );

            //Reduced lines
            xProp->setPropertyValue( UNO_NAME_3D_REDUCED_LINE_GEOMETRY
                , uno::Any( true ) );

            //TextureProjectionMode
            xProp->setPropertyValue( UNO_NAME_3D_TEXTURE_PROJ_Y
                , uno::Any( drawing::TextureProjectionMode_OBJECTSPECIFIC ) );

            //TextureProjectionMode
            xProp->setPropertyValue( UNO_NAME_3D_TEXTURE_PROJ_X
                , uno::Any( drawing::TextureProjectionMode_PARALLEL ) );
            xProp->setPropertyValue( UNO_NAME_3D_TEXTURE_PROJ_Y
                , uno::Any( drawing::TextureProjectionMode_OBJECTSPECIFIC ) );
        }
        catch( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION("chart2", "" );
        }
    }
    return xShape;
}

uno::Reference< drawing::XShape >
        ShapeFactory::createStripe( const uno::Reference< drawing::XShapes >& xTarget
                    , const Stripe& rStripe
                    , const uno::Reference< beans::XPropertySet >& xSourceProp
                    , const tPropertyNameMap& rPropertyNameMap
                    , bool bDoubleSided
                    , short nRotatedTexture
                    , bool bFlatNormals )
{
    if( !xTarget.is() )
        return nullptr;

    //create shape
    uno::Reference< drawing::XShape > xShape(
            m_xShapeFactory->createInstance(
            "com.sun.star.drawing.Shape3DPolygonObject" ), uno::UNO_QUERY );
    xTarget->add(xShape);

    //set properties
    uno::Reference<beans::XMultiPropertySet> xMultiPropertySet(xShape, uno::UNO_QUERY);
    OSL_ENSURE(xMultiPropertySet.is(), "created shape offers no XMultiPropertySet");
    if (xMultiPropertySet.is())
    {
        try
        {
            uno::Sequence<OUString> aPropertyNames{
                UNO_NAME_3D_POLYPOLYGON3D,
                UNO_NAME_3D_TEXTUREPOLYGON3D,
                UNO_NAME_3D_NORMALSPOLYGON3D,
                UNO_NAME_3D_LINEONLY,
                UNO_NAME_3D_DOUBLE_SIDED
            };

            uno::Sequence<uno::Any> aPropertyValues {
                rStripe.getPolyPolygonShape3D(),            // Polygon
                Stripe::getTexturePolygon(nRotatedTexture), // TexturePolygon
                rStripe.getNormalsPolygon(),                // Normals Polygon
                uno::Any(false),        // LineOnly
                uno::Any(bDoubleSided)  // DoubleSided
            };

            //NormalsKind
            if (bFlatNormals)
                lcl_addProperty(aPropertyNames, aPropertyValues,
                                UNO_NAME_3D_NORMALS_KIND, uno::Any(drawing::NormalsKind_FLAT));

            xMultiPropertySet->setPropertyValues(aPropertyNames, aPropertyValues);

            uno::Reference<beans::XPropertySet> xPropertySet(xShape, uno::UNO_QUERY);
            if (xSourceProp.is() && xPropertySet.is())
            {
                PropertyMapper::setMappedProperties(xPropertySet, xSourceProp, rPropertyNameMap);
            }
        }
        catch( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION("chart2", "" );
        }
    }
    return xShape;
}

uno::Reference< drawing::XShape >
        ShapeFactory::createArea3D( const uno::Reference< drawing::XShapes >& xTarget
                    , const drawing::PolyPolygonShape3D& rPolyPolygon
                    , double fDepth )
{
    if( !xTarget.is() )
        return nullptr;

    if( !rPolyPolygon.SequenceX.hasElements())
        return nullptr;

    //create shape
    uno::Reference< drawing::XShape > xShape(
        m_xShapeFactory->createInstance(
            "com.sun.star.drawing.Shape3DExtrudeObject" ), uno::UNO_QUERY );
    xTarget->add(xShape);

    //set properties
    uno::Reference<beans::XMultiPropertySet> xMultiPropertySet(xShape, uno::UNO_QUERY);
    OSL_ENSURE(xMultiPropertySet.is(), "created shape offers no XMultiPropertySet");
    if (xMultiPropertySet.is())
    {
        try
        {
            uno::Sequence<OUString> aPropertyNames{
                UNO_NAME_3D_EXTRUDE_DEPTH,
                UNO_NAME_3D_PERCENT_DIAGONAL,
                UNO_NAME_3D_POLYPOLYGON3D,
                UNO_NAME_3D_DOUBLE_SIDED,
            };

            uno::Sequence<uno::Any> aPropertyValues {
                uno::Any(sal_Int32(fDepth)), // depth
                uno::Any(sal_Int16(0)),      // PercentDiagonal
                uno::Any(rPolyPolygon),      // Polygon
                uno::Any(true)               // DoubleSided
            };

            //the z component of the polygon is now ignored by the drawing layer,
            //so we need to translate the object via transformation matrix

            //Matrix for position
            if (rPolyPolygon.SequenceZ.hasElements()&& rPolyPolygon.SequenceZ[0].hasElements())
            {
                basegfx::B3DHomMatrix aM;
                aM.translate(0, 0, rPolyPolygon.SequenceZ[0][0]);
                drawing::HomogenMatrix aHM = B3DHomMatrixToHomogenMatrix(aM);
                lcl_addProperty(aPropertyNames, aPropertyValues, UNO_NAME_3D_TRANSFORM_MATRIX, uno::Any(aHM));
            }
            xMultiPropertySet->setPropertyValues(aPropertyNames, aPropertyValues);
        }
        catch( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION("chart2", "" );
        }
    }
    return xShape;
}

uno::Reference< drawing::XShape >
        ShapeFactory::createArea2D( const uno::Reference< drawing::XShapes >& xTarget
                    , const drawing::PolyPolygonShape3D& rPolyPolygon
                    , bool bSetZOrderToZero )
{
    if( !xTarget.is() )
        return nullptr;

    //create shape
    uno::Reference< drawing::XShape > xShape(
        m_xShapeFactory->createInstance(
            "com.sun.star.drawing.PolyPolygonShape" ), uno::UNO_QUERY );
    xTarget->add(xShape);

    //set properties
    uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
    OSL_ENSURE(xProp.is(), "created shape offers no XPropertySet");
    if( xProp.is())
    {
        try
        {
            //UNO_NAME_POLYGON "Polygon" drawing::PointSequence*
            drawing::PointSequenceSequence aPoints( PolyToPointSequence(rPolyPolygon) );

            //Polygon
            xProp->setPropertyValue( UNO_NAME_POLYPOLYGON
                , uno::Any( aPoints ) );

            //ZOrder
            //an area should always be behind other shapes
            if (bSetZOrderToZero)
                xProp->setPropertyValue( UNO_NAME_MISC_OBJ_ZORDER
                    , uno::Any( sal_Int32(0) ) );
        }
        catch( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION("chart2", "" );
        }
    }
    return xShape;
}

static drawing::PolyPolygonShape3D createPolyPolygon_Symbol( const drawing::Position3D& rPos
                                 , const drawing::Direction3D& rSize
                                 , sal_Int32 nStandardSymbol )
{
    if(nStandardSymbol<0)
        nStandardSymbol*=-1;
    nStandardSymbol = nStandardSymbol%ShapeFactory::getSymbolCount();
    SymbolEnum eSymbolType=static_cast<SymbolEnum>(nStandardSymbol);

    const double& fX = rPos.PositionX;
    const double& fY = rPos.PositionY;

    const double fWidthH  = rSize.DirectionX/2.0; //fWidthH stands for Half Width
    const double fHeightH = rSize.DirectionY/2.0; //fHeightH stands for Half Height

    const sal_Int32 nQuarterCount = 35; // points inside a quadrant, used in case circle

    sal_Int32 nPointCount = 4; //all arrow symbols only need 4 points
    switch( eSymbolType )
    {
        case Symbol_Square:
        case Symbol_Diamond:
        case Symbol_Bowtie:
        case Symbol_Sandglass:
        case Symbol_HorizontalBar:
        case Symbol_VerticalBar:
            nPointCount = 5;
            break;
        case Symbol_X:
            nPointCount = 13;
            break;
        case Symbol_Plus:
            nPointCount = 13;
            break;
        case Symbol_Star:
            nPointCount = 9;
            break;
        case Symbol_Asterisk:
            nPointCount = 19;
            break;
        case Symbol_Circle:
            nPointCount = 5 + 4 * nQuarterCount;
            break;
        default:
            break;
    }

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
        case Symbol_Square:
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
        case Symbol_UpArrow:
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
        case Symbol_DownArrow:
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
        case Symbol_RightArrow:
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
        case Symbol_LeftArrow:
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
        case Symbol_Bowtie:
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
        case Symbol_Sandglass:
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
        case Symbol_Diamond:
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
        case Symbol_HorizontalBar:
        {
            *pInnerSequenceX++ = fX-fWidthH;
            *pInnerSequenceY++ = fY-0.2*fHeightH;

            *pInnerSequenceX++ = fX+fWidthH;
            *pInnerSequenceY++ = fY-0.2*fHeightH;

            *pInnerSequenceX++ = fX+fWidthH;
            *pInnerSequenceY++ = fY+0.2*fHeightH;

            *pInnerSequenceX++ = fX-fWidthH;
            *pInnerSequenceY++ = fY+0.2*fHeightH;

            *pInnerSequenceX++ = fX-fWidthH;
            *pInnerSequenceY++ = fY-0.2*fHeightH;
            break;
        }
        case Symbol_VerticalBar:
        {
            *pInnerSequenceX++ = fX-0.2*fWidthH;
            *pInnerSequenceY++ = fY-fHeightH;

            *pInnerSequenceX++ = fX+0.2*fWidthH;
            *pInnerSequenceY++ = fY-fHeightH;

            *pInnerSequenceX++ = fX+0.2*fWidthH;
            *pInnerSequenceY++ = fY+fHeightH;

            *pInnerSequenceX++ = fX-0.2*fWidthH;
            *pInnerSequenceY++ = fY+fHeightH;

            *pInnerSequenceX++ = fX-0.2*fWidthH;
            *pInnerSequenceY++ = fY-fHeightH;

            break;
        }
        case Symbol_Circle:
        {
            double fOmega = 1.5707963267948966192 / (nQuarterCount + 1.0);
            // one point in the middle of each edge to get full size bounding rectangle
            *pInnerSequenceX++ = fX + fWidthH;
            *pInnerSequenceY++ = fY;
            // 0 to PI/2
            for (sal_Int32 i = 1; i <= nQuarterCount; ++i)
            {
                *pInnerSequenceX++ = fX + fWidthH * cos( i * fOmega );
                *pInnerSequenceY++ = fY - fHeightH * sin( i * fOmega );
            }
            // PI/2 to PI
            *pInnerSequenceX++ = fX;
            *pInnerSequenceY++ = fY - fHeightH;
            for (sal_Int32 i = 1; i <= nQuarterCount; ++i)
            {
                *pInnerSequenceX++ = fX - fWidthH * sin( i * fOmega);
                *pInnerSequenceY++ = fY - fHeightH * cos( i * fOmega);
            }
            // PI to 3/2*PI
            *pInnerSequenceX++ = fX - fWidthH;
            *pInnerSequenceY++ = fY;
            for (sal_Int32 i = 1; i <= nQuarterCount; ++i)
            {
                *pInnerSequenceX++ = fX - fWidthH * cos( i * fOmega);
                *pInnerSequenceY++ = fY + fHeightH * sin( i * fOmega);
            }
            // 3/2*PI to 2*PI
            *pInnerSequenceX++ = fX;
            *pInnerSequenceY++ = fY + fHeightH;
            for (sal_Int32 i = 1; i <= nQuarterCount; ++i)
            {
                *pInnerSequenceX++ = fX + fWidthH * sin(i * fOmega);
                *pInnerSequenceY++ = fY + fHeightH * cos(i * fOmega);
            }
            // close polygon
            *pInnerSequenceX++ = fX + fWidthH;
            *pInnerSequenceY++ = fY;
            break;
        }
        case Symbol_Star:
        {
            *pInnerSequenceX++ = fX;
            *pInnerSequenceY++ = fY-fHeightH;

            *pInnerSequenceX++ = fX+0.2*fWidthH;
            *pInnerSequenceY++ = fY-0.2*fHeightH;

            *pInnerSequenceX++ = fX+fWidthH;
            *pInnerSequenceY++ = fY;

            *pInnerSequenceX++ = fX+0.2*fWidthH;
            *pInnerSequenceY++ = fY+0.2*fHeightH;

            *pInnerSequenceX++ = fX;
            *pInnerSequenceY++ = fY+fHeightH;

            *pInnerSequenceX++ = fX-0.2*fWidthH;
            *pInnerSequenceY++ = fY+0.2*fHeightH;

            *pInnerSequenceX++ = fX-fWidthH;
            *pInnerSequenceY++ = fY;

            *pInnerSequenceX++ = fX-0.2*fWidthH;
            *pInnerSequenceY++ = fY-0.2*fHeightH;

            *pInnerSequenceX++ = fX;
            *pInnerSequenceY++ = fY-fHeightH;
            break;
        }
        case Symbol_X:
        {
            const double fScaleX = fWidthH / 128.0;
            const double fScaleY = fHeightH / 128.0;
            const double fSmall = sqrt(200.0);
            const double fLarge = 128.0 - fSmall;

            *pInnerSequenceX++ = fX;
            *pInnerSequenceY++ = fY - fScaleY * fSmall;

            *pInnerSequenceX++ = fX - fScaleX * fLarge;
            *pInnerSequenceY++ = fY - fHeightH;

            *pInnerSequenceX++ = fX - fWidthH;
            *pInnerSequenceY++ = fY - fScaleY * fLarge;

            *pInnerSequenceX++ = fX - fScaleX * fSmall;
            *pInnerSequenceY++ = fY;

            *pInnerSequenceX++ = fX - fWidthH;
            *pInnerSequenceY++ = fY + fScaleY * fLarge;

            *pInnerSequenceX++ = fX - fScaleX * fLarge;
            *pInnerSequenceY++ = fY + fHeightH;

            *pInnerSequenceX++ = fX;
            *pInnerSequenceY++ = fY + fScaleY * fSmall;

            *pInnerSequenceX++ = fX + fScaleX * fLarge;
            *pInnerSequenceY++ = fY + fHeightH;

            *pInnerSequenceX++ = fX + fWidthH;
            *pInnerSequenceY++ = fY + fScaleY * fLarge;

            *pInnerSequenceX++ = fX + fScaleX * fSmall;
            *pInnerSequenceY++ = fY;

            *pInnerSequenceX++ = fX + fWidthH;
            *pInnerSequenceY++ = fY - fScaleY * fLarge;

            *pInnerSequenceX++ = fX + fScaleX * fLarge;
            *pInnerSequenceY++ = fY - fHeightH;

            *pInnerSequenceX++ = fX;
            *pInnerSequenceY++ = fY - fScaleY * fSmall;
            break;

        }
        case Symbol_Plus:
        {
            const double fScaleX = fWidthH / 128.0;
            const double fScaleY = fHeightH / 128.0;
            const double fHalf = 10.0; //half line width on 256 size square
            const double fdX = fScaleX * fHalf;
            const double fdY = fScaleY * fHalf;

            *pInnerSequenceX++ = fX-fdX;
            *pInnerSequenceY++ = fY-fHeightH;

            *pInnerSequenceX++ = fX-fdX;
            *pInnerSequenceY++ = fY-fdY;

            *pInnerSequenceX++ = fX-fWidthH;
            *pInnerSequenceY++ = fY-fdY;

            *pInnerSequenceX++ = fX-fWidthH;
            *pInnerSequenceY++ = fY+fdY;

            *pInnerSequenceX++ = fX-fdX;
            *pInnerSequenceY++ = fY+fdY;

            *pInnerSequenceX++ = fX-fdX;
            *pInnerSequenceY++ = fY+fHeightH;

            *pInnerSequenceX++ = fX+fdX;
            *pInnerSequenceY++ = fY+fHeightH;

            *pInnerSequenceX++ = fX+fdX;
            *pInnerSequenceY++ = fY+fdY;

            *pInnerSequenceX++ = fX+fWidthH;
            *pInnerSequenceY++ = fY+fdY;

            *pInnerSequenceX++ = fX+fWidthH;
            *pInnerSequenceY++ = fY-fdY;

            *pInnerSequenceX++ = fX+fdX;
            *pInnerSequenceY++ = fY-fdY;

            *pInnerSequenceX++ = fX+fdY;
            *pInnerSequenceY++ = fY-fHeightH;

            *pInnerSequenceX++ = fX-fdX;
            *pInnerSequenceY++ = fY-fHeightH;
            break;

        }
        case Symbol_Asterisk:
        {
            const double fHalf = 10.0; // half line width on 256 size square
            const double fTwoY = fHalf * sqrt(3.0);
            const double fFourY = (128.0 - 2.0 * fHalf ) / sqrt(3.0);
            const double fThreeX = 128.0 - fHalf;
            const double fThreeY = fHalf * sqrt(3.0) + fFourY;
            const double fFiveX = 2.0 * fHalf;

            const double fScaleX = fWidthH / 128.0;
            const double fScaleY = fHeightH / 128.0;

            //1
            *pInnerSequenceX++ = fX-fScaleX * fHalf;
            *pInnerSequenceY++ = fY-fHeightH;
            //2
            *pInnerSequenceX++ = fX-fScaleX * fHalf;
            *pInnerSequenceY++ = fY-fScaleY * fTwoY;
            //3
            *pInnerSequenceX++ = fX-fScaleX * fThreeX;
            *pInnerSequenceY++ = fY-fScaleY * fThreeY;
            //4
            *pInnerSequenceX++ = fX-fWidthH;
            *pInnerSequenceY++ = fY-fScaleY * fFourY;
            //5
            *pInnerSequenceX++ = fX-fScaleX * fFiveX;
            *pInnerSequenceY++ = fY;
            //6 as 4
            *pInnerSequenceX++ = fX-fWidthH;
            *pInnerSequenceY++ = fY+fScaleY * fFourY;
            //7 as 3
            *pInnerSequenceX++ = fX-fScaleX * fThreeX;
            *pInnerSequenceY++ = fY+fScaleY * fThreeY;
            //8 as 2
            *pInnerSequenceX++ = fX-fScaleX * fHalf;
            *pInnerSequenceY++ = fY+fScaleY * fTwoY;
            //9 as 1
            *pInnerSequenceX++ = fX-fScaleX * fHalf;
            *pInnerSequenceY++ = fY+fHeightH;
            //10 as 1
            *pInnerSequenceX++ = fX+fScaleX * fHalf;
            *pInnerSequenceY++ = fY+fHeightH;
            //11 as 2
            *pInnerSequenceX++ = fX+fScaleX * fHalf;
            *pInnerSequenceY++ = fY+fScaleY * fTwoY;
            //12 as 3
            *pInnerSequenceX++ = fX+fScaleX * fThreeX;
            *pInnerSequenceY++ = fY+fScaleY * fThreeY;
            //13 as 4
            *pInnerSequenceX++ = fX+fWidthH;
            *pInnerSequenceY++ = fY+fScaleY * fFourY;
            //14 as 5
            *pInnerSequenceX++ = fX+fScaleX * fFiveX;
            *pInnerSequenceY++ = fY;
            //15 as 4
            *pInnerSequenceX++ = fX+fWidthH;
            *pInnerSequenceY++ = fY-fScaleY * fFourY;
            //16 as 3
            *pInnerSequenceX++ = fX+fScaleX * fThreeX;
            *pInnerSequenceY++ = fY-fScaleY * fThreeY;
            //17 as 2
            *pInnerSequenceX++ = fX+fScaleX * fHalf;
            *pInnerSequenceY++ = fY-fScaleY * fTwoY;
            // 18 as 1
            *pInnerSequenceX++ = fX+fScaleX * fHalf;
            *pInnerSequenceY++ = fY-fHeightH;
            // 19 = 1, closing
            *pInnerSequenceX++ = fX-fScaleX * fHalf;
            *pInnerSequenceY++ = fY-fHeightH;
            break;
        }
        default: //case Symbol_Square:
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

    return aPP;
}

uno::Reference< drawing::XShape >
        ShapeFactory::createSymbol2D(
                      const uno::Reference< drawing::XShapes >& xTarget
                    , const drawing::Position3D& rPosition
                    , const drawing::Direction3D& rSize
                    , sal_Int32 nStandardSymbol
                    , sal_Int32 nBorderColor
                    , sal_Int32 nFillColor )
{
    if( !xTarget.is() )
        return nullptr;

    //create shape
    uno::Reference< drawing::XShape > xShape(
        m_xShapeFactory->createInstance(
            "com.sun.star.drawing.PolyPolygonShape" ), uno::UNO_QUERY );
    xTarget->add(xShape);

    //set properties
    uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
    OSL_ENSURE(xProp.is(), "created shape offers no XPropertySet");
    if( xProp.is())
    {
        try
        {
            drawing::PointSequenceSequence aPoints( PolyToPointSequence(
                createPolyPolygon_Symbol( rPosition, rSize, nStandardSymbol ) ));

            //Polygon
            xProp->setPropertyValue( UNO_NAME_POLYPOLYGON
                , uno::Any( aPoints ) );

            //LineColor
            xProp->setPropertyValue( UNO_NAME_LINECOLOR
                , uno::Any( nBorderColor ) );

            //FillColor
            xProp->setPropertyValue( UNO_NAME_FILLCOLOR
                , uno::Any( nFillColor ) );
        }
        catch( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION("chart2", "" );
        }
    }
    return xShape;
}

uno::Reference< drawing::XShape >
        ShapeFactory::createGraphic2D(
                      const uno::Reference< drawing::XShapes >& xTarget
                    , const drawing::Position3D& rPosition
                    , const drawing::Direction3D& rSize
                    , const uno::Reference< graphic::XGraphic >& xGraphic )
{
    if( !xTarget.is() || !xGraphic.is() )
        return nullptr;

    // @todo: change this to a rectangle shape with a fill bitmap for
    // performance reasons (ask AW, said CL)

    //create shape
    uno::Reference< drawing::XShape > xShape(
        m_xShapeFactory->createInstance(
            "com.sun.star.drawing.GraphicObjectShape" ), uno::UNO_QUERY );
    xTarget->add(xShape);

    try
    {
        // assume position is upper left corner. Transform to center.
        drawing::Position3D aCenterPosition(
            rPosition.PositionX - (rSize.DirectionX / 2.0),
            rPosition.PositionY - (rSize.DirectionY / 2.0),
            rPosition.PositionZ );
        xShape->setPosition( Position3DToAWTPoint( aCenterPosition ));
        xShape->setSize( Direction3DToAWTSize( rSize ));
    }
    catch( const uno::Exception & )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
    uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
    OSL_ENSURE(xProp.is(), "created shape offers no XPropertySet");
    if( xProp.is())
    {
        try
        {
            xProp->setPropertyValue( "Graphic", uno::Any( xGraphic ));
        }
        catch( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION("chart2", "" );
        }
    }
    return xShape;
}

uno::Reference< drawing::XShapes >
        ShapeFactory::createGroup2D( const uno::Reference< drawing::XShapes >& xTarget
        , const OUString& aName )
{
    if( !xTarget.is() )
        return nullptr;
    try
    {
        //create and add to target
        uno::Reference< drawing::XShape > xShape(
                    m_xShapeFactory->createInstance(
                    "com.sun.star.drawing.GroupShape" ), uno::UNO_QUERY );
        xTarget->add(xShape);

        //set name
        if(!aName.isEmpty())
            setShapeName( xShape , aName );

        {//workaround
            //need this null size as otherwise empty group shapes where painted with a gray border
            xShape->setSize(awt::Size(0,0));
        }

        //return
        uno::Reference< drawing::XShapes > xShapes( xShape, uno::UNO_QUERY );
        return xShapes;
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
    return nullptr;
}

uno::Reference< drawing::XShapes >
        ShapeFactory::createGroup3D( const uno::Reference< drawing::XShapes >& xTarget
        , const OUString& aName )
{
    if( !xTarget.is() )
        return nullptr;
    try
    {
        //create shape
        uno::Reference< drawing::XShape > xShape(
                m_xShapeFactory->createInstance(
                "com.sun.star.drawing.Shape3DSceneObject" ), uno::UNO_QUERY );

        xTarget->add(xShape);

        //it is necessary to set the transform matrix to initialize the scene properly
        //otherwise all objects which are placed into this Group will not be visible
        //the following should be unnecessary after the bug is fixed
        {
            //set properties
            uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
            OSL_ENSURE(xProp.is(), "created shape offers no XPropertySet");
            if( xProp.is())
            {
                try
                {
                    ::basegfx::B3DHomMatrix aM;
                    xProp->setPropertyValue( UNO_NAME_3D_TRANSFORM_MATRIX
                        , uno::Any(B3DHomMatrixToHomogenMatrix(aM)) );
                }
                catch( const uno::Exception& )
                {
                    TOOLS_WARN_EXCEPTION("chart2", "" );
                }
            }
        }

        //set name
        if(!aName.isEmpty())
            setShapeName( xShape , aName );

        //return
        uno::Reference< drawing::XShapes > xShapes( xShape, uno::UNO_QUERY );
        return xShapes;
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
    return nullptr;
}

uno::Reference< drawing::XShape >
        ShapeFactory::createCircle2D( const uno::Reference< drawing::XShapes >& xTarget
                    , const drawing::Position3D& rPosition
                    , const drawing::Direction3D& rSize )
{
    if( !xTarget.is() )
        return nullptr;

    //create shape
    uno::Reference< drawing::XShape > xShape(
        m_xShapeFactory->createInstance(
            "com.sun.star.drawing.EllipseShape" ), uno::UNO_QUERY );
    xTarget->add(xShape);

    try
    {
        drawing::Position3D aCenterPosition(
            rPosition.PositionX - (rSize.DirectionX / 2.0),
            rPosition.PositionY - (rSize.DirectionY / 2.0),
            rPosition.PositionZ );
        xShape->setPosition( Position3DToAWTPoint( aCenterPosition ));
        xShape->setSize( Direction3DToAWTSize( rSize ));
    }
    catch( const uno::Exception & )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }

    //set properties
    uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
    OSL_ENSURE(xProp.is(), "created shape offers no XPropertySet");
    if( xProp.is())
    {
        try
        {
            xProp->setPropertyValue( UNO_NAME_CIRCKIND, uno::Any( drawing::CircleKind_FULL ) );
        }
        catch( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION("chart2", "" );
        }
    }
    return xShape;
}

uno::Reference< drawing::XShape >
    ShapeFactory::createCircle( const uno::Reference< drawing::XShapes >& xTarget
                    , const awt::Size& rSize
                    , const awt::Point& rPosition )
{
    uno::Reference< drawing::XShape > xShape(
        m_xShapeFactory->createInstance(
            "com.sun.star.drawing.EllipseShape" ), uno::UNO_QUERY );
    xTarget->add(xShape);
    xShape->setSize( rSize );
    xShape->setPosition( rPosition );

    return xShape;
}

uno::Reference< drawing::XShape >
        ShapeFactory::createLine3D( const uno::Reference< drawing::XShapes >& xTarget
                    , const drawing::PolyPolygonShape3D& rPoints
                    , const VLineProperties& rLineProperties )
{
    if( !xTarget.is() )
        return nullptr;

    if(!rPoints.SequenceX.hasElements())
        return nullptr;

    //create shape
    uno::Reference< drawing::XShape > xShape(
        m_xShapeFactory->createInstance(
            "com.sun.star.drawing.Shape3DPolygonObject" ), uno::UNO_QUERY );
    xTarget->add(xShape);

    //set properties
    uno::Reference<beans::XMultiPropertySet> xMultiPropertySet(xShape, uno::UNO_QUERY);
    OSL_ENSURE(xMultiPropertySet.is(), "created shape offers no XMultiPropertySet");
    if (xMultiPropertySet.is())
    {
        try
        {
            uno::Sequence<OUString> aPropertyNames {
                UNO_NAME_3D_POLYPOLYGON3D,
                UNO_NAME_3D_LINEONLY
            };

            uno::Sequence<uno::Any> aPropertyValues {
                uno::Any(rPoints),  // Polygon
                uno::Any(true)      // LineOnly
            };

            //Transparency
            if(rLineProperties.Transparence.hasValue())
            {
                lcl_addProperty(aPropertyNames, aPropertyValues,
                                UNO_NAME_LINETRANSPARENCE,
                                rLineProperties.Transparence);
            }

            //LineStyle
            if(rLineProperties.LineStyle.hasValue())
            {
                lcl_addProperty(aPropertyNames, aPropertyValues,
                                UNO_NAME_LINESTYLE,
                                rLineProperties.LineStyle);
            }

            //LineWidth
            if(rLineProperties.Width.hasValue())
            {
                lcl_addProperty(aPropertyNames, aPropertyValues,
                                UNO_NAME_LINEWIDTH,
                                rLineProperties.Width);
            }

            //LineColor
            if(rLineProperties.Color.hasValue())
            {
                lcl_addProperty(aPropertyNames, aPropertyValues,
                                UNO_NAME_LINECOLOR,
                                rLineProperties.Color);
            }
            xMultiPropertySet->setPropertyValues(aPropertyNames, aPropertyValues);
        }
        catch( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION("chart2", "" );
        }
    }
    return xShape;
}

uno::Reference< drawing::XShape >
        ShapeFactory::createLine2D( const uno::Reference< drawing::XShapes >& xTarget
                    , const drawing::PointSequenceSequence& rPoints
                    , const VLineProperties* pLineProperties )
{
    if( !xTarget.is() )
        return nullptr;

    if(!rPoints.hasElements())
        return nullptr;

    //create shape
    uno::Reference< drawing::XShape > xShape(
        m_xShapeFactory->createInstance(
            "com.sun.star.drawing.PolyLineShape" ), uno::UNO_QUERY );
    xTarget->add(xShape);

    //set properties
    uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
    OSL_ENSURE(xProp.is(), "created shape offers no XPropertySet");
    if( xProp.is())
    {
        try
        {
            //Polygon
            xProp->setPropertyValue( UNO_NAME_POLYPOLYGON
                , uno::Any( rPoints ) );

            if(pLineProperties)
            {
                //Transparency
                if(pLineProperties->Transparence.hasValue())
                    xProp->setPropertyValue( UNO_NAME_LINETRANSPARENCE
                        , pLineProperties->Transparence );

                //LineStyle
                if(pLineProperties->LineStyle.hasValue())
                    xProp->setPropertyValue( UNO_NAME_LINESTYLE
                        , pLineProperties->LineStyle );

                //LineWidth
                if(pLineProperties->Width.hasValue())
                    xProp->setPropertyValue( UNO_NAME_LINEWIDTH
                        , pLineProperties->Width );

                //LineColor
                if(pLineProperties->Color.hasValue())
                    xProp->setPropertyValue( UNO_NAME_LINECOLOR
                        , pLineProperties->Color );

                //LineDashName
                if(pLineProperties->DashName.hasValue())
                    xProp->setPropertyValue( "LineDashName"
                        , pLineProperties->DashName );

                //LineCap
                if(pLineProperties->LineCap.hasValue())
                    xProp->setPropertyValue( UNO_NAME_LINECAP
                        , pLineProperties->LineCap );
            }
        }
        catch( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION("chart2", "" );
        }
    }
    return xShape;
}

uno::Reference< drawing::XShape >
    ShapeFactory::createLine ( const uno::Reference< drawing::XShapes >& xTarget,
            const awt::Size& rSize, const awt::Point& rPosition )
{
    //create shape
    uno::Reference< drawing::XShape > xShape(
        m_xShapeFactory->createInstance(
            "com.sun.star.drawing.LineShape" ), uno::UNO_QUERY );
    xTarget->add(xShape);
    xShape->setSize( rSize );
    xShape->setPosition( rPosition );

    return xShape;
}

uno::Reference< drawing::XShape > ShapeFactory::createInvisibleRectangle(
            const uno::Reference< drawing::XShapes >& xTarget
            , const awt::Size& rSize )
{
    try
    {
        if(!xTarget.is())
            return nullptr;

        uno::Reference< drawing::XShape > xShape( m_xShapeFactory->createInstance(
                "com.sun.star.drawing.RectangleShape"), uno::UNO_QUERY );
        if( xShape.is())
        {
            xTarget->add( xShape );
            ShapeFactory::makeShapeInvisible( xShape );
            xShape->setSize( rSize );
        }
        return xShape;
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
    return nullptr;
}

uno::Reference< drawing::XShape > ShapeFactory::createRectangle(
    const uno::Reference< drawing::XShapes >& xTarget,
    const awt::Size& rSize,
    const awt::Point& rPosition,
    const tNameSequence& rPropNames,
    const tAnySequence& rPropValues,
    StackPosition ePos )
{
    uno::Reference< drawing::XShape > xShape( m_xShapeFactory->createInstance(
                "com.sun.star.drawing.RectangleShape"), uno::UNO_QUERY );
    if( xShape.is())
    {
        if (ePos == StackPosition::Bottom)
        {
            uno::Reference<drawing::XShapes2> xTarget2(xTarget, uno::UNO_QUERY);
            if (xTarget2.is())
                xTarget2->addBottom(xShape);
        }
        else
            xTarget->add(xShape);

        xShape->setPosition( rPosition );
        xShape->setSize( rSize );
        uno::Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_QUERY_THROW );
        PropertyMapper::setMultiProperties( rPropNames, rPropValues, xPropSet );
    }

    return xShape;
}

uno::Reference< drawing::XShape >
    ShapeFactory::createRectangle(
            const uno::Reference<
            drawing::XShapes >& xTarget )
{
    uno::Reference< drawing::XShape > xShape( m_xShapeFactory->createInstance(
                "com.sun.star.drawing.RectangleShape"), uno::UNO_QUERY );
    xTarget->add( xShape );

    return xShape;
}

uno::Reference< drawing::XShape >
        ShapeFactory::createText( const uno::Reference< drawing::XShapes >& xTarget
                    , const OUString& rText
                    , const tNameSequence& rPropNames
                    , const tAnySequence& rPropValues
                    , const uno::Any& rATransformation )
{
    if( !xTarget.is() )
        return nullptr;

    if(rText.isEmpty())
        return nullptr;

    //create shape and add to page
    uno::Reference< drawing::XShape > xShape(
            m_xShapeFactory->createInstance(
            "com.sun.star.drawing.TextShape" ), uno::UNO_QUERY );
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
            if (rATransformation.hasValue())
                xProp->setPropertyValue( "Transformation", rATransformation );
            else
                SAL_INFO("chart2", "No rATransformation value is given to ShapeFactory::createText()");

        }
        catch( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION("chart2", "" );
        }
    }
    return xShape;
}

uno::Reference< drawing::XShape >
    ShapeFactory::createText( const uno::Reference< drawing::XShapes >& xTarget
                , uno::Sequence< uno::Reference< chart2::XFormattedString > >& xFormattedString
                , const tNameSequence& rPropNames
                , const tAnySequence& rPropValues
                , const uno::Any& rATransformation )
{
    if( !xTarget.is() )
        return nullptr;

    if( !xFormattedString.hasElements() )
        return nullptr;

    sal_Int32 nNumberOfParagraphs = xFormattedString.getLength();

    bool bNotEmpty = false;
    for( sal_Int32 nN = 0; nN < nNumberOfParagraphs; ++nN )
    {
        if( !xFormattedString[nN]->getString().isEmpty() )
        {
            bNotEmpty = true;
            break;
        }
    }
    if( !bNotEmpty )
        return nullptr;

    //create shape and add to page
    uno::Reference< drawing::XShape > xShape(
            m_xShapeFactory->createInstance(
            "com.sun.star.drawing.TextShape" ), uno::UNO_QUERY );
    xTarget->add(xShape);

    //set paragraph properties
    bNotEmpty = false;
    Reference< text::XText > xText( xShape, uno::UNO_QUERY );
    if( xText.is() )
    {
        // the first cursor is used for appending the next paragraph,
        // after a new string has been inserted the cursor is moved at the end
        // of the inserted string
        // the second cursor is used for selecting the paragraph and apply the
        // passed text properties
        Reference< text::XTextCursor > xInsertCursor = xText->createTextCursor();
        Reference< text::XTextCursor > xSelectionCursor = xText->createTextCursor();
        if( xInsertCursor.is() && xSelectionCursor.is() )
        {
            uno::Reference< beans::XPropertySet > xSelectionProp( xSelectionCursor, uno::UNO_QUERY );
            if( xSelectionProp.is() )
            {
                for( sal_Int32 nN = 0; nN < nNumberOfParagraphs; ++nN )
                {
                    if( !xFormattedString[nN]->getString().isEmpty() )
                    {
                        xInsertCursor->gotoEnd( false );
                        xSelectionCursor->gotoEnd( false );
                        xText->insertString( xInsertCursor, xFormattedString[nN]->getString(), false );
                        bNotEmpty = true;
                        xSelectionCursor->gotoEnd( true ); // select current paragraph
                        uno::Reference< beans::XPropertySet > xStringProperties( xFormattedString[nN], uno::UNO_QUERY );
                        PropertyMapper::setMappedProperties( xSelectionProp, xStringProperties,
                            PropertyMapper::getPropertyNameMapForTextShapeProperties() );
                    }
                }
            }
        }
    }

    if( !bNotEmpty )
        return nullptr;

    uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
    if( xProp.is() )
    {
        //set whole text shape properties
        PropertyMapper::setMultiProperties( rPropNames, rPropValues, xProp );

        if( rATransformation.hasValue() )
        {
            //set position matrix
            //the matrix needs to be set at the end behind autogrow and such position influencing properties
            try
            {
                xProp->setPropertyValue( "Transformation", rATransformation );
            }
            catch( const uno::Exception& )
            {
                TOOLS_WARN_EXCEPTION("chart2", "" );
            }
        }
    }
    return xShape;
}

uno::Reference< drawing::XShape >
        ShapeFactory::createText( const uno::Reference< drawing::XShapes >& xTarget,
                const awt::Size& rSize,
                const awt::Point& rPos,
                uno::Sequence< uno::Reference< chart2::XFormattedString > >& xFormattedString,
                const uno::Reference<
                beans::XPropertySet > & xTextProperties,
                double nRotation, const OUString& aName, sal_Int32 nTextMaxWidth )
{
    //create shape and add to page
    uno::Reference< drawing::XShape > xShape(
            m_xShapeFactory->createInstance(
                "com.sun.star.drawing.TextShape" ), uno::UNO_QUERY );
    try
    {
        xTarget->add(xShape);

        //set text and text properties
        uno::Reference< text::XText > xText( xShape, uno::UNO_QUERY );
        uno::Reference< text::XTextCursor > xTextCursor( xText->createTextCursor() );
        uno::Reference< beans::XPropertySet > xShapeProp( xShape, uno::UNO_QUERY );
        if( !xText.is() || !xTextCursor.is() || !xShapeProp.is() || !xTextProperties.is() )
            return xShape;

        tPropertyNameValueMap aValueMap;
        //fill line-, fill- and paragraph-properties into the ValueMap
        {
            tPropertyNameMap aNameMap = PropertyMapper::getPropertyNameMapForParagraphProperties();
            auto const & add = PropertyMapper::getPropertyNameMapForFillAndLineProperties();
            aNameMap.insert(add.begin(), add.end());

            PropertyMapper::getValueMap( aValueMap, aNameMap, xTextProperties );
        }

        //fill some more shape properties into the ValueMap
        {
            aValueMap.insert( { "TextHorizontalAdjust", uno::Any(drawing::TextHorizontalAdjust_CENTER) } ); // drawing::TextHorizontalAdjust
            aValueMap.insert( { "TextVerticalAdjust", uno::Any(drawing::TextVerticalAdjust_CENTER) } ); //drawing::TextVerticalAdjust
            aValueMap.insert( { "TextAutoGrowHeight", uno::Any(true) } ); // sal_Bool
            aValueMap.insert( { "TextAutoGrowWidth", uno::Any(true) } ); // sal_Bool
            aValueMap.insert( { "TextMaximumFrameWidth", uno::Any(nTextMaxWidth) } ); // sal_Int32

            //set name/classified ObjectID (CID)
            if( !aName.isEmpty() )
                aValueMap.emplace( "Name", uno::Any( aName ) ); //CID OUString
        }

        //set global title properties
        {
            tNameSequence aPropNames;
            tAnySequence aPropValues;
            PropertyMapper::getMultiPropertyListsFromValueMap( aPropNames, aPropValues, aValueMap );
            PropertyMapper::setMultiProperties( aPropNames, aPropValues, xShapeProp );
        }

        bool bStackCharacters(false);
        try
        {
            xTextProperties->getPropertyValue( "StackCharacters" ) >>= bStackCharacters;
        }
        catch( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION("chart2", "" );
        }

        if(bStackCharacters)
        {
            //if the characters should be stacked we use only the first character properties for code simplicity
            if( xFormattedString.hasElements() )
            {
                OUString aLabel;
                for( const auto & i : std::as_const(xFormattedString) )
                    aLabel += i->getString();
                aLabel = ShapeFactory::getStackedString( aLabel, bStackCharacters );

                xTextCursor->gotoEnd(false);
                xText->insertString( xTextCursor, aLabel, false );
                xTextCursor->gotoEnd(true);
                uno::Reference< beans::XPropertySet > xTargetProps( xShape, uno::UNO_QUERY );
                uno::Reference< beans::XPropertySet > xSourceProps( xFormattedString[0], uno::UNO_QUERY );

                PropertyMapper::setMappedProperties( xTargetProps, xSourceProps
                        , PropertyMapper::getPropertyNameMapForCharacterProperties() );

                // adapt font size according to page size
                awt::Size aOldRefSize;
                if( xTextProperties->getPropertyValue( "ReferencePageSize") >>= aOldRefSize )
                {
                    RelativeSizeHelper::adaptFontSizes( xTargetProps, aOldRefSize, rSize );
                }
            }
        }
        else
        {
            for( const uno::Reference< chart2::XFormattedString >& rxFS : std::as_const(xFormattedString) )
            {
                xTextCursor->gotoEnd(false);
                xText->insertString( xTextCursor, rxFS->getString(), false );
                xTextCursor->gotoEnd(true);
            }
            awt::Size aOldRefSize;
            bool bHasRefPageSize =
                ( xTextProperties->getPropertyValue( "ReferencePageSize") >>= aOldRefSize );

            if( xFormattedString.hasElements() )
            {
                uno::Reference< beans::XPropertySet > xTargetProps( xShape, uno::UNO_QUERY );
                uno::Reference< beans::XPropertySet > xSourceProps( xFormattedString[0], uno::UNO_QUERY );
                PropertyMapper::setMappedProperties( xTargetProps, xSourceProps, PropertyMapper::getPropertyNameMapForCharacterProperties() );

                // adapt font size according to page size
                if( bHasRefPageSize )
                {
                    RelativeSizeHelper::adaptFontSizes( xTargetProps, aOldRefSize, rSize );
                }
            }
        }

        // #i109336# Improve auto positioning in chart
        float fFontHeight = 0.0;
        if ( xShapeProp.is() && ( xShapeProp->getPropertyValue( "CharHeight" ) >>= fFontHeight ) )
        {
            fFontHeight = convertPointToMm100(fFontHeight);
            sal_Int32 nXDistance = static_cast< sal_Int32 >( ::rtl::math::round( fFontHeight * 0.18f ) );
            sal_Int32 nYDistance = static_cast< sal_Int32 >( ::rtl::math::round( fFontHeight * 0.30f ) );
            xShapeProp->setPropertyValue( "TextLeftDistance", uno::Any( nXDistance ) );
            xShapeProp->setPropertyValue( "TextRightDistance", uno::Any( nXDistance ) );
            xShapeProp->setPropertyValue( "TextUpperDistance", uno::Any( nYDistance ) );
            xShapeProp->setPropertyValue( "TextLowerDistance", uno::Any( nYDistance ) );
        }
        sal_Int32 nXPos = rPos.X;
        sal_Int32 nYPos = rPos.Y;

        //set position matrix
        //the matrix needs to be set at the end behind autogrow and such position influencing properties
        ::basegfx::B2DHomMatrix aM;
        aM.rotate( -basegfx::deg2rad(nRotation) );//#i78696#->#i80521#
        aM.translate( nXPos, nYPos );
        xShapeProp->setPropertyValue( "Transformation", uno::Any( B2DHomMatrixToHomogenMatrix3(aM) ) );

        xShapeProp->setPropertyValue( "ParaAdjust", uno::Any( style::ParagraphAdjust_CENTER ) );
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
    return xShape;
}

ShapeFactory* ShapeFactory::getOrCreateShapeFactory(const uno::Reference< lang::XMultiServiceFactory>& xFactory)
{
    static ShapeFactory* pShapeFactory = new ShapeFactory(xFactory);
    return pShapeFactory;
}

uno::Reference< drawing::XShapes > ShapeFactory::getChartRootShape(
    const uno::Reference< drawing::XDrawPage>& xDrawPage )
{
    uno::Reference< drawing::XShapes > xRet;
    const uno::Reference< drawing::XShapes > xShapes = xDrawPage;
    if( xShapes.is() )
    {
        sal_Int32 nCount = xShapes->getCount();
        uno::Reference< drawing::XShape > xShape;
        for( sal_Int32 nN = nCount; nN--; )
        {
            if( xShapes->getByIndex( nN ) >>= xShape )
            {
                if( ShapeFactory::getShapeName( xShape ) == "com.sun.star.chart2.shapes" )
                {
                    xRet.set( xShape, uno::UNO_QUERY );
                    break;
                }
            }
        }
    }
    return xRet;
}

void ShapeFactory::makeShapeInvisible( const uno::Reference< drawing::XShape >& xShape )
{
    uno::Reference< beans::XPropertySet > xShapeProp( xShape, uno::UNO_QUERY );
    OSL_ENSURE(xShapeProp.is(), "created shape offers no XPropertySet");
    if( xShapeProp.is())
    {
        try
        {
            xShapeProp->setPropertyValue( "LineStyle", uno::Any( drawing::LineStyle_NONE ));
            xShapeProp->setPropertyValue( "FillStyle", uno::Any( drawing::FillStyle_NONE ));
        }
        catch( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION("chart2", "" );
        }
    }
}

// set a name/CID at a shape (is used for selection handling)

void ShapeFactory::setShapeName( const uno::Reference< drawing::XShape >& xShape
                               , const OUString& rName )
{
    if(!xShape.is())
        return;
    uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
    OSL_ENSURE(xProp.is(), "shape offers no XPropertySet");
    if( xProp.is())
    {
        try
        {
            xProp->setPropertyValue( UNO_NAME_MISC_OBJ_NAME
                , uno::Any( rName ) );
        }
        catch( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION("chart2", "" );
        }
    }
}

OUString ShapeFactory::getShapeName( const uno::Reference< drawing::XShape >& xShape )
{
    OUString aRet;

    uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
    OSL_ENSURE(xProp.is(), "shape offers no XPropertySet");
    if( xProp.is())
    {
        try
        {
            xProp->getPropertyValue( UNO_NAME_MISC_OBJ_NAME ) >>= aRet;
        }
        catch( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION("chart2", "" );
        }
    }

    return aRet;
}

uno::Any ShapeFactory::makeTransformation( const awt::Point& rScreenPosition2D, double fRotationAnglePi )
{
    ::basegfx::B2DHomMatrix aM;
    //As autogrow is active the rectangle is automatically expanded to that side
    //to which the text is not adjusted.
    // aM.scale( 1, 1 ); Oops? A scale with this parameters is neutral, line commented out
    aM.rotate( fRotationAnglePi );
    aM.translate( rScreenPosition2D.X, rScreenPosition2D.Y );
    uno::Any aATransformation( B2DHomMatrixToHomogenMatrix3(aM) );
    return aATransformation;
}

OUString ShapeFactory::getStackedString( const OUString& rString, bool bStacked )
{
    sal_Int32 nLen = rString.getLength();
    if(!bStacked || !nLen)
        return rString;

    OUStringBuffer aStackStr;

    //add a newline after each letter
    //as we do not no letters here add a newline after each char
    for( sal_Int32 nPosSrc=0; nPosSrc < nLen; nPosSrc++ )
    {
        if( nPosSrc )
            aStackStr.append( '\r' );
        aStackStr.append(rString[nPosSrc]);
    }
    return aStackStr.makeStringAndClear();
}

bool ShapeFactory::hasPolygonAnyLines( drawing::PolyPolygonShape3D& rPoly)
{
    // #i67757# check all contained polygons, if at least one polygon contains 2 or more points, return true
    for( auto const & i : std::as_const(rPoly.SequenceX) )
        if( i.getLength() > 1 )
            return true;
    return false;
}

bool ShapeFactory::isPolygonEmptyOrSinglePoint( drawing::PolyPolygonShape3D& rPoly)
{
    // true, if empty polypolygon or one polygon with one point
    return !rPoly.SequenceX.hasElements() ||
        ((rPoly.SequenceX.getLength() == 1) && (rPoly.SequenceX[0].getLength() <= 1));
}

void ShapeFactory::closePolygon( drawing::PolyPolygonShape3D& rPoly)
{
    OSL_ENSURE( rPoly.SequenceX.getLength() <= 1, "ShapeFactory::closePolygon - single polygon expected" );
    //add a last point == first point
    if(isPolygonEmptyOrSinglePoint(rPoly))
        return;
    drawing::Position3D aFirst(rPoly.SequenceX[0][0],rPoly.SequenceY[0][0],rPoly.SequenceZ[0][0]);
    AddPointToPoly( rPoly, aFirst );
}

awt::Size ShapeFactory::calculateNewSizeRespectingAspectRatio(
         const awt::Size& rTargetSize
         , const awt::Size& rSourceSizeWithCorrectAspectRatio )
{
    awt::Size aNewSize;

    double fFactorWidth = double(rTargetSize.Width)/double(rSourceSizeWithCorrectAspectRatio.Width);
    double fFactorHeight = double(rTargetSize.Height)/double(rSourceSizeWithCorrectAspectRatio.Height);
    double fFactor = std::min(fFactorWidth,fFactorHeight);
    aNewSize.Width=static_cast<sal_Int32>(fFactor*rSourceSizeWithCorrectAspectRatio.Width);
    aNewSize.Height=static_cast<sal_Int32>(fFactor*rSourceSizeWithCorrectAspectRatio.Height);

    return aNewSize;
}

awt::Point ShapeFactory::calculateTopLeftPositionToCenterObject(
           const awt::Point& rTargetAreaPosition
         , const awt::Size& rTargetAreaSize
         , const awt::Size& rObjectSize )
{
    awt::Point aNewPosition(rTargetAreaPosition);
    aNewPosition.X += static_cast<sal_Int32>(double(rTargetAreaSize.Width-rObjectSize.Width)/2.0);
    aNewPosition.Y += static_cast<sal_Int32>(double(rTargetAreaSize.Height-rObjectSize.Height)/2.0);
    return aNewPosition;
}

::basegfx::B2IRectangle ShapeFactory::getRectangleOfShape(
        const uno::Reference< drawing::XShape >& xShape )
{
    ::basegfx::B2IRectangle aRet;

    if( xShape.is() )
    {
        awt::Point aPos = xShape->getPosition();
        awt::Size aSize = xShape->getSize();
        aRet = BaseGFXHelper::makeRectangle(aPos,aSize);
    }
    return aRet;

}

awt::Size ShapeFactory::getSizeAfterRotation(
         const uno::Reference< drawing::XShape >& xShape, double fRotationAngleDegree )
{
    awt::Size aRet(0,0);
    if(xShape.is())
    {
        const awt::Size aSize( xShape->getSize() );

        if( fRotationAngleDegree == 0.0 )
            aRet = aSize;
        else
        {
            fRotationAngleDegree = NormAngle360(fRotationAngleDegree);
            if(fRotationAngleDegree>270.0)
                fRotationAngleDegree=360.0-fRotationAngleDegree;
            else if(fRotationAngleDegree>180.0)
                fRotationAngleDegree=fRotationAngleDegree-180.0;
            else if(fRotationAngleDegree>90.0)
                fRotationAngleDegree=180.0-fRotationAngleDegree;

            const double fAnglePi = basegfx::deg2rad(fRotationAngleDegree);

            aRet.Height = static_cast<sal_Int32>(
                aSize.Width*std::sin( fAnglePi )
                + aSize.Height*std::cos( fAnglePi ));
            aRet.Width = static_cast<sal_Int32>(
                aSize.Width*std::cos( fAnglePi )
                + aSize.Height*std::sin( fAnglePi ));
        }
    }
    return aRet;
}

void ShapeFactory::removeSubShapes( const uno::Reference< drawing::XShapes >& xShapes )
{
    if( xShapes.is() )
    {
        sal_Int32 nSubCount = xShapes->getCount();
        uno::Reference< drawing::XShape > xShape;
        for( sal_Int32 nS = nSubCount; nS--; )
        {
            if( xShapes->getByIndex( nS ) >>= xShape )
                xShapes->remove( xShape );
        }
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
