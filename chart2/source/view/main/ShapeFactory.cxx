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
#include <com/sun/star/drawing/XShapes2.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.hxx>

#include <editeng/unoprnms.hxx>
#include <rtl/math.hxx>

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdopath.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <tools/helpers.hxx>
#include <tools/UnitConversion.hxx>
#include <sal/log.hxx>

#include <algorithm>
#include <cmath>
#include <cstddef>

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

css::drawing::PolyPolygonShape3D toPolyPolygonShape3D(const std::vector<std::vector<css::drawing::Position3D>>& rPoints)
{
    css::drawing::PolyPolygonShape3D aUnoPoly;
    aUnoPoly.SequenceX.realloc(rPoints.size());
    aUnoPoly.SequenceY.realloc(rPoints.size());
    aUnoPoly.SequenceZ.realloc(rPoints.size());
    for (std::size_t nPolygonIndex=0; nPolygonIndex<rPoints.size(); ++nPolygonIndex)
    {
        drawing::DoubleSequence* pOuterSequenceX = &aUnoPoly.SequenceX.getArray()[nPolygonIndex];
        drawing::DoubleSequence* pOuterSequenceY = &aUnoPoly.SequenceY.getArray()[nPolygonIndex];
        drawing::DoubleSequence* pOuterSequenceZ = &aUnoPoly.SequenceZ.getArray()[nPolygonIndex];
        pOuterSequenceX->realloc(rPoints[nPolygonIndex].size());
        pOuterSequenceY->realloc(rPoints[nPolygonIndex].size());
        pOuterSequenceZ->realloc(rPoints[nPolygonIndex].size());
        double* pInnerSequenceX = pOuterSequenceX->getArray();
        double* pInnerSequenceY = pOuterSequenceY->getArray();
        double* pInnerSequenceZ = pOuterSequenceZ->getArray();
        for (std::size_t nPointIndex=0; nPointIndex<rPoints[nPolygonIndex].size(); ++nPointIndex)
        {
            auto& rPos = rPoints[nPolygonIndex][nPointIndex];
            pInnerSequenceX[nPointIndex] = rPos.PositionX;
            pInnerSequenceY[nPointIndex] = rPos.PositionY;
            pInnerSequenceZ[nPointIndex] = rPos.PositionZ;
        }
    }
    return aUnoPoly;
}

} // end anonymous namespace

rtl::Reference<SvxShapeGroupAnyD> ShapeFactory::getOrCreateChartRootShape(
    const rtl::Reference<SvxDrawPage>& xDrawPage )
{
    rtl::Reference<SvxShapeGroupAnyD> xRet = ShapeFactory::getChartRootShape(xDrawPage);
    if (xRet.is())
        return xRet;

    // Create a new root shape and set it to the bottom of the page.  The root
    // shape is identified by having the name com.sun.star.chart2.shapes.
    rtl::Reference<SvxShapeGroup> xShapeGroup = new SvxShapeGroup(nullptr, nullptr);
    xShapeGroup->setShapeKind(SdrObjKind::Group);
    // cast to resolve ambiguity in converting to XShape
    xDrawPage->addBottom(static_cast<SvxShape*>(xShapeGroup.get()));

    setShapeName(xShapeGroup, "com.sun.star.chart2.shapes");
    xShapeGroup->setSize(awt::Size(0,0));

    return xShapeGroup;
}

void ShapeFactory::setPageSize(const rtl::Reference<SvxShapeGroupAnyD>&, const awt::Size&) {}

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

rtl::Reference<Svx3DExtrudeObject>
        ShapeFactory::createCube(
            const rtl::Reference<SvxShapeGroupAnyD>& xTarget
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
    rtl::Reference<Svx3DExtrudeObject> xShape = impl_createCube( xTarget, rPosition, rSize, nRotateZAngleHundredthDegree, bRounded );
    if( xSourceProp.is())
        PropertyMapper::setMappedProperties( *xShape, xSourceProp, rPropertyNameMap );
    return xShape;
}

rtl::Reference<Svx3DExtrudeObject>
        ShapeFactory::impl_createCube(
              const rtl::Reference<SvxShapeGroupAnyD>& xTarget
            , const drawing::Position3D& rPosition, const drawing::Direction3D& rSize
            , sal_Int32 nRotateZAngleHundredthDegree
            , bool bRounded )
{
    if( !xTarget.is() )
        return nullptr;

    //create shape
    rtl::Reference<Svx3DExtrudeObject> xShape = new Svx3DExtrudeObject(nullptr);
    xShape->setShapeKind(SdrObjKind::E3D_Extrusion);
    xTarget->addShape(*xShape);

    //set properties
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

        xShape->setPropertyValues(aPropertyNames, aPropertyValues);
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
    return xShape;
}

rtl::Reference<Svx3DLatheObject>
        ShapeFactory::createCylinder(
            const rtl::Reference<SvxShapeGroupAnyD>& xTarget
          , const drawing::Position3D& rPosition, const drawing::Direction3D& rSize
          , sal_Int32 nRotateZAngleHundredthDegree )
{
    return impl_createConeOrCylinder(
              xTarget, rPosition, rSize, 0.0, nRotateZAngleHundredthDegree, true );
}

rtl::Reference<Svx3DSceneObject>
        ShapeFactory::createPyramid(
            const rtl::Reference<SvxShapeGroupAnyD>& xTarget
          , const drawing::Position3D& rPosition, const drawing::Direction3D& rSize
          , double fTopHeight, bool bRotateZ
          , const uno::Reference< beans::XPropertySet >& xSourceProp
          , const tPropertyNameMap& rPropertyNameMap )
{
    if( !xTarget.is() )
        return nullptr;

    rtl::Reference<Svx3DSceneObject> xGroup( ShapeFactory::createGroup3D( xTarget ) );

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

    return xGroup;
}

rtl::Reference<Svx3DLatheObject>
        ShapeFactory::createCone(
            const rtl::Reference<SvxShapeGroupAnyD>& xTarget
          , const drawing::Position3D& rPosition, const drawing::Direction3D& rSize
          , double fTopHeight, sal_Int32 nRotateZAngleHundredthDegree )
{
    return impl_createConeOrCylinder( xTarget, rPosition, rSize, fTopHeight, nRotateZAngleHundredthDegree, false );
}

rtl::Reference<Svx3DLatheObject>
        ShapeFactory::impl_createConeOrCylinder(
              const rtl::Reference<SvxShapeGroupAnyD>& xTarget
            , const drawing::Position3D& rPosition, const drawing::Direction3D& rSize
            , double fTopHeight, sal_Int32 nRotateZAngleHundredthDegree
            , bool bCylinder )
{
    if( !xTarget.is() )
        return nullptr;

    //create shape
    rtl::Reference<Svx3DLatheObject> xShape = new Svx3DLatheObject(nullptr);
    xShape->setShapeKind(SdrObjKind::E3D_Lathe);
    xTarget->addShape(*xShape);

    double fWidth      = rSize.DirectionX/2.0; //The depth will be corrected within Matrix
    double fRadius     = fWidth; //!!!!!!!! problem in drawing layer: rotation object calculates wrong needed size -> wrong camera (it's a problem with bounding boxes)
    double fHeight     = rSize.DirectionY;

    //set properties
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

        xShape->setPropertyValues(aPropertyNames, aPropertyValues);
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
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

rtl::Reference<SvxShapePolyPolygon>
        ShapeFactory::createPieSegment2D(
                    const rtl::Reference<SvxShapeGroupAnyD>& xTarget
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
    rtl::Reference<SvxShapePolyPolygon> xShape = new SvxShapePolyPolygon(nullptr);
    xShape->setShapeKind(SdrObjKind::PathFill); // aka ClosedBezierShape
    xTarget->addShape(*xShape); //need to add the shape before setting of properties

    //set properties
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

        xShape->SvxShape::setPropertyValue( "PolyPolygonBezier", uno::Any( aCoords ) );
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }

    return xShape;
}

rtl::Reference<Svx3DExtrudeObject>
        ShapeFactory::createPieSegment(
                    const rtl::Reference<SvxShapeGroupAnyD>& xTarget
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
    rtl::Reference<Svx3DExtrudeObject> xShape = new Svx3DExtrudeObject(nullptr);
    xShape->setShapeKind(SdrObjKind::E3D_Extrusion);
    xTarget->addShape(*xShape); //need to add the shape before setting of properties

    //set properties
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
        xShape->setPropertyValue( UNO_NAME_3D_EXTRUDE_DEPTH
            , uno::Any(static_cast<sal_Int32>(fDepth)) );

        //PercentDiagonal
        xShape->setPropertyValue( UNO_NAME_3D_PERCENT_DIAGONAL
            , uno::Any( sal_Int16(0) ) );

        //Polygon
        drawing::PolyPolygonShape3D aPoly( BezierToPoly(aCoords) );
        ShapeFactory::closePolygon( aPoly );
        xShape->setPropertyValue( UNO_NAME_3D_POLYPOLYGON3D
            , uno::Any( aPoly ) );

        //DoubleSided
        xShape->setPropertyValue( UNO_NAME_3D_DOUBLE_SIDED
            , uno::Any( true ) );

        //Reduced lines
        xShape->setPropertyValue( UNO_NAME_3D_REDUCED_LINE_GEOMETRY
            , uno::Any( true ) );

        //TextureProjectionMode
        xShape->setPropertyValue( UNO_NAME_3D_TEXTURE_PROJ_Y
            , uno::Any( drawing::TextureProjectionMode_OBJECTSPECIFIC ) );

        //TextureProjectionMode
        xShape->setPropertyValue( UNO_NAME_3D_TEXTURE_PROJ_X
            , uno::Any( drawing::TextureProjectionMode_PARALLEL ) );
        xShape->setPropertyValue( UNO_NAME_3D_TEXTURE_PROJ_Y
            , uno::Any( drawing::TextureProjectionMode_OBJECTSPECIFIC ) );
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
    return xShape;
}

rtl::Reference<Svx3DPolygonObject>
        ShapeFactory::createStripe( const rtl::Reference<SvxShapeGroupAnyD>& xTarget
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
    rtl::Reference<Svx3DPolygonObject> xShape = new Svx3DPolygonObject(nullptr);
    xShape->setShapeKind(SdrObjKind::E3D_Polygon);
    xTarget->addShape(*xShape);

    //set properties
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

        xShape->setPropertyValues(aPropertyNames, aPropertyValues);

        if (xSourceProp)
        {
            PropertyMapper::setMappedProperties(*xShape, xSourceProp, rPropertyNameMap);
        }
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
    return xShape;
}

rtl::Reference<Svx3DExtrudeObject>
        ShapeFactory::createArea3D( const rtl::Reference<SvxShapeGroupAnyD>& xTarget
                    , const std::vector<std::vector<css::drawing::Position3D>>& rPolyPolygon
                    , double fDepth )
{
    if( !xTarget.is() )
        return nullptr;

    if( rPolyPolygon.empty() )
        return nullptr;

    //create shape
    rtl::Reference<Svx3DExtrudeObject> xShape = new Svx3DExtrudeObject(nullptr);
    xShape->setShapeKind(SdrObjKind::E3D_Extrusion);
    xTarget->addShape(*xShape);

    css::drawing::PolyPolygonShape3D aUnoPolyPolygon = toPolyPolygonShape3D(rPolyPolygon);

    //set properties
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
            uno::Any(aUnoPolyPolygon),      // Polygon
            uno::Any(true)               // DoubleSided
        };

        //the z component of the polygon is now ignored by the drawing layer,
        //so we need to translate the object via transformation matrix

        //Matrix for position
        if (!rPolyPolygon.empty() && !rPolyPolygon[0].empty())
        {
            basegfx::B3DHomMatrix aM;
            aM.translate(0, 0, rPolyPolygon[0][0].PositionZ);
            drawing::HomogenMatrix aHM = B3DHomMatrixToHomogenMatrix(aM);
            lcl_addProperty(aPropertyNames, aPropertyValues, UNO_NAME_3D_TRANSFORM_MATRIX, uno::Any(aHM));
        }
        xShape->setPropertyValues(aPropertyNames, aPropertyValues);
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
    return xShape;
}

rtl::Reference<SvxShapePolyPolygon>
        ShapeFactory::createArea2D( const rtl::Reference<SvxShapeGroupAnyD>& xTarget
                    , const std::vector<std::vector<css::drawing::Position3D>>& rPolyPolygon )
{
    if( !xTarget.is() )
        return nullptr;

    //create shape
    rtl::Reference<SdrPathObj> pPath = new SdrPathObj(xTarget->GetSdrObject()->getSdrModelFromSdrObject(), SdrObjKind::Polygon);
    xTarget->GetSdrObject()->GetSubList()->InsertObject(pPath.get());

    //set properties
    try
    {
        // Polygon
        basegfx::B2DPolyPolygon aNewPolyPolygon( PolyToB2DPolyPolygon(rPolyPolygon) );
        // tdf#117145 metric of SdrModel is app-specific, metric of UNO API is 100thmm
        pPath->ForceMetricToItemPoolMetric(aNewPolyPolygon);
        pPath->SetPathPoly(aNewPolyPolygon);
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
    return static_cast<SvxShapePolyPolygon*>(pPath->getUnoShape().get());
}

static drawing::PointSequenceSequence createPolyPolygon_Symbol( const drawing::Position3D& rPos
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

    drawing::PointSequenceSequence aPP;

    aPP.realloc(1);

    uno::Sequence<awt::Point>* pOuterSequence = aPP.getArray();

    pOuterSequence->realloc(nPointCount);

    awt::Point* pInnerSequence = pOuterSequence->getArray();

    auto toPoint = [](double x, double y) -> awt::Point
        {
            return { static_cast<sal_Int32>(x), static_cast<sal_Int32>(y) };
        };
    switch(eSymbolType)
    {
        case Symbol_Square:
        {
            *pInnerSequence++ = toPoint( fX-fWidthH, fY-fHeightH );

            *pInnerSequence++ = toPoint( fX-fWidthH, fY+fHeightH );

            *pInnerSequence++ = toPoint( fX+fWidthH, fY+fHeightH );

            *pInnerSequence++ = toPoint( fX+fWidthH, fY-fHeightH );

            *pInnerSequence++ = toPoint( fX-fWidthH, fY-fHeightH );
            break;
        }
        case Symbol_UpArrow:
        {
            *pInnerSequence++ = toPoint( fX-fWidthH, fY+fHeightH );

            *pInnerSequence++ = toPoint( fX+fWidthH, fY+fHeightH );

            *pInnerSequence++ = toPoint( fX, fY-fHeightH );

            *pInnerSequence++ = toPoint( fX-fWidthH, fY+fHeightH );
            break;
        }
        case Symbol_DownArrow:
        {
            *pInnerSequence++ = toPoint( fX-fWidthH, fY-fHeightH );

            *pInnerSequence++ = toPoint( fX, fY+fHeightH );

            *pInnerSequence++ = toPoint( fX+fWidthH, fY-fHeightH );

            *pInnerSequence++ = toPoint( fX-fWidthH, fY-fHeightH );
            break;
        }
        case Symbol_RightArrow:
        {
            *pInnerSequence++ = toPoint( fX-fWidthH, fY-fHeightH );

            *pInnerSequence++ = toPoint( fX-fWidthH, fY+fHeightH  );

            *pInnerSequence++ = toPoint( fX+fWidthH, fY );

            *pInnerSequence++ = toPoint( fX-fWidthH, fY-fHeightH );
            break;
        }
        case Symbol_LeftArrow:
        {
            *pInnerSequence++ = toPoint( fX-fWidthH, fY );

            *pInnerSequence++ = toPoint( fX+fWidthH, fY+fHeightH );

            *pInnerSequence++ = toPoint( fX+fWidthH, fY-fHeightH );

            *pInnerSequence++ = toPoint( fX-fWidthH, fY );
            break;
        }
        case Symbol_Bowtie:
        {
            *pInnerSequence++ = toPoint( fX-fWidthH, fY-fHeightH );

            *pInnerSequence++ = toPoint( fX-fWidthH, fY+fHeightH );

            *pInnerSequence++ = toPoint( fX+fWidthH, fY-fHeightH );

            *pInnerSequence++ = toPoint( fX+fWidthH, fY+fHeightH );

            *pInnerSequence++ = toPoint( fX-fWidthH, fY-fHeightH );
            break;
        }
        case Symbol_Sandglass:
        {
            *pInnerSequence++ = toPoint( fX-fWidthH, fY+fHeightH );

            *pInnerSequence++ = toPoint( fX+fWidthH, fY+fHeightH );

            *pInnerSequence++ = toPoint( fX-fWidthH, fY-fHeightH );

            *pInnerSequence++ = toPoint( fX+fWidthH, fY-fHeightH );

            *pInnerSequence++ = toPoint( fX-fWidthH, fY+fHeightH );
            break;
        }
        case Symbol_Diamond:
        {
            *pInnerSequence++ = toPoint( fX-fWidthH, fY );

            *pInnerSequence++ = toPoint( fX, fY+fHeightH );

            *pInnerSequence++ = toPoint( fX+fWidthH, fY );

            *pInnerSequence++ = toPoint( fX, fY-fHeightH );

            *pInnerSequence++ = toPoint( fX-fWidthH, fY );
            break;
        }
        case Symbol_HorizontalBar:
        {
            *pInnerSequence++ = toPoint( fX-fWidthH, fY-0.2*fHeightH );

            *pInnerSequence++ = toPoint( fX+fWidthH, fY-0.2*fHeightH );

            *pInnerSequence++ = toPoint( fX+fWidthH, fY+0.2*fHeightH );

            *pInnerSequence++ = toPoint( fX-fWidthH, fY+0.2*fHeightH );

            *pInnerSequence++ = toPoint( fX-fWidthH, fY-0.2*fHeightH );
            break;
        }
        case Symbol_VerticalBar:
        {
            *pInnerSequence++ = toPoint( fX-0.2*fWidthH, fY-fHeightH );

            *pInnerSequence++ = toPoint( fX+0.2*fWidthH, fY-fHeightH );

            *pInnerSequence++ = toPoint( fX+0.2*fWidthH, fY+fHeightH );

            *pInnerSequence++ = toPoint( fX-0.2*fWidthH, fY+fHeightH );

            *pInnerSequence++ = toPoint( fX-0.2*fWidthH, fY-fHeightH );

            break;
        }
        case Symbol_Circle:
        {
            double fOmega = M_PI_2 / (nQuarterCount + 1.0);
            // one point in the middle of each edge to get full size bounding rectangle
            *pInnerSequence++ = toPoint( fX + fWidthH, fY );
            // 0 to PI/2
            for (sal_Int32 i = 1; i <= nQuarterCount; ++i)
            {
                *pInnerSequence++ = toPoint( fX + fWidthH * cos( i * fOmega ), fY - fHeightH * sin( i * fOmega ) );
            }
            // PI/2 to PI
            *pInnerSequence++ = toPoint( fX, fY - fHeightH );
            for (sal_Int32 i = 1; i <= nQuarterCount; ++i)
            {
                *pInnerSequence++ = toPoint( fX - fWidthH * sin( i * fOmega), fY - fHeightH * cos( i * fOmega) );
            }
            // PI to 3/2*PI
            *pInnerSequence++ = toPoint( fX - fWidthH, fY );
            for (sal_Int32 i = 1; i <= nQuarterCount; ++i)
            {
                *pInnerSequence++ = toPoint( fX - fWidthH * cos( i * fOmega), fY + fHeightH * sin( i * fOmega) );
            }
            // 3/2*PI to 2*PI
            *pInnerSequence++ = toPoint( fX, fY + fHeightH );
            for (sal_Int32 i = 1; i <= nQuarterCount; ++i)
            {
                *pInnerSequence++ = toPoint( fX + fWidthH * sin(i * fOmega), fY + fHeightH * cos(i * fOmega) );
            }
            // close polygon
            *pInnerSequence++ = toPoint( fX + fWidthH, fY );
            break;
        }
        case Symbol_Star:
        {
            *pInnerSequence++ = toPoint( fX, fY-fHeightH );

            *pInnerSequence++ = toPoint( fX+0.2*fWidthH, fY-0.2*fHeightH );

            *pInnerSequence++ = toPoint( fX+fWidthH, fY );

            *pInnerSequence++ = toPoint( fX+0.2*fWidthH, fY+0.2*fHeightH );

            *pInnerSequence++ = toPoint( fX, fY+fHeightH );

            *pInnerSequence++ = toPoint( fX-0.2*fWidthH, fY+0.2*fHeightH );

            *pInnerSequence++ = toPoint( fX-fWidthH, fY );

            *pInnerSequence++ = toPoint( fX-0.2*fWidthH, fY-0.2*fHeightH );

            *pInnerSequence++ = toPoint( fX, fY-fHeightH );
            break;
        }
        case Symbol_X:
        {
            const double fScaleX = fWidthH / 128.0;
            const double fScaleY = fHeightH / 128.0;
            const double fSmall = sqrt(200.0);
            const double fLarge = 128.0 - fSmall;

            *pInnerSequence++ = toPoint( fX, fY - fScaleY * fSmall );

            *pInnerSequence++ = toPoint( fX - fScaleX * fLarge, fY - fHeightH );

            *pInnerSequence++ = toPoint( fX - fWidthH, fY - fScaleY * fLarge );

            *pInnerSequence++ = toPoint( fX - fScaleX * fSmall, fY );

            *pInnerSequence++ = toPoint( fX - fWidthH, fY + fScaleY * fLarge );

            *pInnerSequence++ = toPoint( fX - fScaleX * fLarge, fY + fHeightH );

            *pInnerSequence++ = toPoint( fX, fY + fScaleY * fSmall );

            *pInnerSequence++ = toPoint( fX + fScaleX * fLarge, fY + fHeightH );

            *pInnerSequence++ = toPoint( fX + fWidthH, fY + fScaleY * fLarge );

            *pInnerSequence++ = toPoint( fX + fScaleX * fSmall, fY );

            *pInnerSequence++ = toPoint( fX + fWidthH, fY - fScaleY * fLarge );

            *pInnerSequence++ = toPoint( fX + fScaleX * fLarge, fY - fHeightH );

            *pInnerSequence++ = toPoint( fX, fY - fScaleY * fSmall );
            break;

        }
        case Symbol_Plus:
        {
            const double fScaleX = fWidthH / 128.0;
            const double fScaleY = fHeightH / 128.0;
            const double fHalf = 10.0; //half line width on 256 size square
            const double fdX = fScaleX * fHalf;
            const double fdY = fScaleY * fHalf;

            *pInnerSequence++ = toPoint( fX-fdX, fY-fHeightH );

            *pInnerSequence++ = toPoint( fX-fdX, fY-fdY );

            *pInnerSequence++ = toPoint( fX-fWidthH, fY-fdY );

            *pInnerSequence++ = toPoint( fX-fWidthH, fY+fdY );

            *pInnerSequence++ = toPoint( fX-fdX, fY+fdY );

            *pInnerSequence++ = toPoint( fX-fdX, fY+fHeightH );

            *pInnerSequence++ = toPoint( fX+fdX, fY+fHeightH );

            *pInnerSequence++ = toPoint( fX+fdX, fY+fdY );

            *pInnerSequence++ = toPoint( fX+fWidthH, fY+fdY );

            *pInnerSequence++ = toPoint( fX+fWidthH, fY-fdY );

            *pInnerSequence++ = toPoint( fX+fdX, fY-fdY );

            *pInnerSequence++ = toPoint( fX+fdY, fY-fHeightH );

            *pInnerSequence++ = toPoint( fX-fdX, fY-fHeightH );
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
            *pInnerSequence++ = toPoint( fX-fScaleX * fHalf, fY-fHeightH );
            //2
            *pInnerSequence++ = toPoint( fX-fScaleX * fHalf, fY-fScaleY * fTwoY );
            //3
            *pInnerSequence++ = toPoint( fX-fScaleX * fThreeX, fY-fScaleY * fThreeY );
            //4
            *pInnerSequence++ = toPoint( fX-fWidthH, fY-fScaleY * fFourY );
            //5
            *pInnerSequence++ = toPoint( fX-fScaleX * fFiveX, fY );
            //6 as 4
            *pInnerSequence++ = toPoint( fX-fWidthH, fY+fScaleY * fFourY );
            //7 as 3
            *pInnerSequence++ = toPoint( fX-fScaleX * fThreeX, fY+fScaleY * fThreeY );
            //8 as 2
            *pInnerSequence++ = toPoint( fX-fScaleX * fHalf, fY+fScaleY * fTwoY );
            //9 as 1
            *pInnerSequence++ = toPoint( fX-fScaleX * fHalf, fY+fHeightH );
            //10 as 1
            *pInnerSequence++ = toPoint( fX+fScaleX * fHalf, fY+fHeightH );
            //11 as 2
            *pInnerSequence++ = toPoint( fX+fScaleX * fHalf, fY+fScaleY * fTwoY );
            //12 as 3
            *pInnerSequence++ = toPoint( fX+fScaleX * fThreeX, fY+fScaleY * fThreeY );
            //13 as 4
            *pInnerSequence++ = toPoint( fX+fWidthH, fY+fScaleY * fFourY );
            //14 as 5
            *pInnerSequence++ = toPoint( fX+fScaleX * fFiveX, fY );
            //15 as 4
            *pInnerSequence++ = toPoint( fX+fWidthH, fY-fScaleY * fFourY );
            //16 as 3
            *pInnerSequence++ = toPoint( fX+fScaleX * fThreeX, fY-fScaleY * fThreeY );
            //17 as 2
            *pInnerSequence++ = toPoint( fX+fScaleX * fHalf, fY-fScaleY * fTwoY );
            // 18 as 1
            *pInnerSequence++ = toPoint( fX+fScaleX * fHalf, fY-fHeightH );
            // 19 = 1, closing
            *pInnerSequence++ = toPoint( fX-fScaleX * fHalf, fY-fHeightH );
            break;
        }
        default: //case Symbol_Square:
        {
            *pInnerSequence++ = toPoint( fX-fWidthH, fY-fHeightH );

            *pInnerSequence++ = toPoint( fX-fWidthH, fY+fHeightH );

            *pInnerSequence++ = toPoint( fX+fWidthH, fY+fHeightH );

            *pInnerSequence++ = toPoint( fX+fWidthH, fY-fHeightH );

            *pInnerSequence++ = toPoint( fX-fWidthH, fY-fHeightH );
            break;
        }
    }

    return aPP;
}

rtl::Reference<SvxShapePolyPolygon>
        ShapeFactory::createSymbol2D(
                      const rtl::Reference<SvxShapeGroupAnyD>& xTarget
                    , const drawing::Position3D& rPosition
                    , const drawing::Direction3D& rSize
                    , sal_Int32 nStandardSymbol
                    , sal_Int32 nBorderColor
                    , sal_Int32 nFillColor )
{
    if( !xTarget.is() )
        return nullptr;

    //create shape
    rtl::Reference<SvxShapePolyPolygon> xShape = new SvxShapePolyPolygon(nullptr);
    xShape->setShapeKind(SdrObjKind::Polygon);
    xTarget->addShape(*xShape);

    //set properties
    try
    {
        drawing::PointSequenceSequence aPoints =
            createPolyPolygon_Symbol( rPosition, rSize, nStandardSymbol );

        //Polygon
        xShape->SvxShape::setPropertyValue( UNO_NAME_POLYPOLYGON
            , uno::Any( aPoints ) );

        //LineColor
        xShape->SvxShape::setPropertyValue( UNO_NAME_LINECOLOR
            , uno::Any( nBorderColor ) );

        //FillColor
        xShape->SvxShape::setPropertyValue( UNO_NAME_FILLCOLOR
            , uno::Any( nFillColor ) );
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
    return xShape;
}

rtl::Reference<SvxGraphicObject>
        ShapeFactory::createGraphic2D(
                      const rtl::Reference<SvxShapeGroupAnyD>& xTarget
                    , const drawing::Position3D& rPosition
                    , const drawing::Direction3D& rSize
                    , const uno::Reference< graphic::XGraphic >& xGraphic )
{
    if( !xTarget.is() || !xGraphic.is() )
        return nullptr;

    // @todo: change this to a rectangle shape with a fill bitmap for
    // performance reasons (ask AW, said CL)

    //create shape
    rtl::Reference<SvxGraphicObject> xShape = new SvxGraphicObject(nullptr);
    xShape->setShapeKind(SdrObjKind::Graphic);
    xTarget->addShape(*xShape);

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
    try
    {
        xShape->SvxShape::setPropertyValue( "Graphic", uno::Any( xGraphic ));
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
    return xShape;
}

rtl::Reference< SvxShapeGroup >
        ShapeFactory::createGroup2D( const rtl::Reference<SvxShapeGroupAnyD>& xTarget
        , const OUString& aName )
{
    if( !xTarget.is() )
        return nullptr;
    try
    {
        //create and add to target
        rtl::Reference<SvxShapeGroup> xShapeGroup = new SvxShapeGroup(nullptr, nullptr);
        xShapeGroup->setShapeKind(SdrObjKind::Group);
        // cast to resolve ambiguity in converting to XShape
        xTarget->addShape(*xShapeGroup);

        //set name
        if(!aName.isEmpty())
            setShapeName( xShapeGroup, aName );

        {//workaround
            //need this null size as otherwise empty group shapes where painted with a gray border
            xShapeGroup->setSize(awt::Size(0,0));
        }

        return xShapeGroup;
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
    return nullptr;
}

rtl::Reference< SvxShapeGroup >
        ShapeFactory::createGroup2D( const rtl::Reference<SvxDrawPage>& xTarget
        , const OUString& aName )
{
    if( !xTarget.is() )
        return nullptr;
    try
    {
        //create and add to target
        rtl::Reference<SvxShapeGroup> xShapeGroup = new SvxShapeGroup(nullptr, nullptr);
        xShapeGroup->setShapeKind(SdrObjKind::Group);
        // cast to resolve ambiguity in converting to XShape
        xTarget->add(static_cast<SvxShape*>(xShapeGroup.get()));

        //set name
        if(!aName.isEmpty())
            setShapeName( xShapeGroup, aName );

        {//workaround
            //need this null size as otherwise empty group shapes where painted with a gray border
            xShapeGroup->setSize(awt::Size(0,0));
        }

        return xShapeGroup;
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
    return nullptr;
}

rtl::Reference<Svx3DSceneObject>
        ShapeFactory::createGroup3D( const rtl::Reference<SvxShapeGroupAnyD>& xTarget
        , const OUString& aName )
{
    if( !xTarget.is() )
        return nullptr;
    try
    {
        //create shape
        rtl::Reference<Svx3DSceneObject> xShape = new Svx3DSceneObject(nullptr, nullptr);
        xShape->setShapeKind(SdrObjKind::E3D_Scene);
        xTarget->addShape(*xShape);

        //it is necessary to set the transform matrix to initialize the scene properly
        //otherwise all objects which are placed into this Group will not be visible
        //the following should be unnecessary after the bug is fixed
        //set properties
        try
        {
            ::basegfx::B3DHomMatrix aM;
            xShape->SvxShape::setPropertyValue( UNO_NAME_3D_TRANSFORM_MATRIX
                , uno::Any(B3DHomMatrixToHomogenMatrix(aM)) );
        }
        catch( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION("chart2", "" );
        }

        //set name
        if(!aName.isEmpty())
            setShapeName( xShape , aName );

        //return
        return xShape;
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
    return nullptr;
}

rtl::Reference<SvxShapeCircle>
        ShapeFactory::createCircle2D( const rtl::Reference<SvxShapeGroupAnyD>& xTarget
                    , const drawing::Position3D& rPosition
                    , const drawing::Direction3D& rSize )
{
    if( !xTarget.is() )
        return nullptr;

    //create shape
    rtl::Reference<SvxShapeCircle> xShape = new SvxShapeCircle(nullptr);
    xShape->setShapeKind(SdrObjKind::CircleOrEllipse);
    xTarget->addShape(*xShape);

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
    try
    {
        xShape->SvxShape::setPropertyValue( UNO_NAME_CIRCKIND, uno::Any( drawing::CircleKind_FULL ) );
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
    return xShape;
}

rtl::Reference<SvxShapeCircle>
    ShapeFactory::createCircle( const rtl::Reference<SvxShapeGroupAnyD>& xTarget
                    , const awt::Size& rSize
                    , const awt::Point& rPosition )
{
    rtl::Reference<SvxShapeCircle> xShape = new SvxShapeCircle(nullptr);
    xShape->setShapeKind(SdrObjKind::CircleOrEllipse);
    xTarget->addShape(*xShape);
    xShape->setSize( rSize );
    xShape->setPosition( rPosition );

    return xShape;
}

rtl::Reference<Svx3DPolygonObject>
        ShapeFactory::createLine3D( const rtl::Reference<SvxShapeGroupAnyD>& xTarget
                    , const std::vector<std::vector<css::drawing::Position3D>>& rPoints
                    , const VLineProperties& rLineProperties )
{
    if( !xTarget.is() )
        return nullptr;

    if(rPoints.empty())
        return nullptr;

    //create shape
    rtl::Reference<Svx3DPolygonObject> xShape = new Svx3DPolygonObject(nullptr);
    xShape->setShapeKind(SdrObjKind::E3D_Polygon);
    xTarget->addShape(*xShape);

    css::drawing::PolyPolygonShape3D aUnoPoly = toPolyPolygonShape3D(rPoints);

    //set properties
    try
    {
        uno::Sequence<OUString> aPropertyNames {
            UNO_NAME_3D_POLYPOLYGON3D,
            UNO_NAME_3D_LINEONLY
        };

        uno::Sequence<uno::Any> aPropertyValues {
            uno::Any(aUnoPoly),  // Polygon
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
        xShape->setPropertyValues(aPropertyNames, aPropertyValues);
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
    return xShape;
}

rtl::Reference<SvxShapePolyPolygon>
        ShapeFactory::createLine2D( const rtl::Reference<SvxShapeGroupAnyD>& xTarget
                    , const drawing::PointSequenceSequence& rPoints
                    , const VLineProperties* pLineProperties )
{
    if( !xTarget.is() )
        return nullptr;

    if(!rPoints.hasElements())
        return nullptr;

    //create shape
    rtl::Reference<SvxShapePolyPolygon> xShape = new SvxShapePolyPolygon(nullptr);
    xShape->setShapeKind(SdrObjKind::PolyLine);
    xTarget->addShape(*xShape);

    //set properties
    try
    {
        //Polygon
        xShape->SvxShape::setPropertyValue( UNO_NAME_POLYPOLYGON
            , uno::Any( rPoints ) );

        if(pLineProperties)
        {
            //Transparency
            if(pLineProperties->Transparence.hasValue())
                xShape->SvxShape::setPropertyValue( UNO_NAME_LINETRANSPARENCE
                    , pLineProperties->Transparence );

            //LineStyle
            if(pLineProperties->LineStyle.hasValue())
                xShape->SvxShape::setPropertyValue( UNO_NAME_LINESTYLE
                    , pLineProperties->LineStyle );

            //LineWidth
            if(pLineProperties->Width.hasValue())
                xShape->SvxShape::setPropertyValue( UNO_NAME_LINEWIDTH
                    , pLineProperties->Width );

            //LineColor
            if(pLineProperties->Color.hasValue())
                xShape->SvxShape::setPropertyValue( UNO_NAME_LINECOLOR
                    , pLineProperties->Color );

            //LineDashName
            if(pLineProperties->DashName.hasValue())
                xShape->SvxShape::setPropertyValue( "LineDashName"
                    , pLineProperties->DashName );

            //LineCap
            if(pLineProperties->LineCap.hasValue())
                xShape->SvxShape::setPropertyValue( UNO_NAME_LINECAP
                    , pLineProperties->LineCap );
        }
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
    return xShape;
}

rtl::Reference<SvxShapePolyPolygon>
        ShapeFactory::createLine2D( const rtl::Reference<SvxShapeGroupAnyD>& xTarget
                    , const std::vector<std::vector<css::drawing::Position3D>>& rPoints
                    , const VLineProperties* pLineProperties )
{
    if( !xTarget.is() )
        return nullptr;

    if(rPoints.empty())
        return nullptr;

    //create shape
    rtl::Reference<SvxShapePolyPolygon> xShape = new SvxShapePolyPolygon(nullptr);
    xShape->setShapeKind(SdrObjKind::PolyLine);
    xTarget->addShape(*xShape);

    drawing::PointSequenceSequence aAnyPoints = PolyToPointSequence(rPoints);

    //set properties
    try
    {
        //Polygon
        xShape->SvxShape::setPropertyValue( UNO_NAME_POLYPOLYGON
            , uno::Any( aAnyPoints ) );

        if(pLineProperties)
        {
            //Transparency
            if(pLineProperties->Transparence.hasValue())
                xShape->SvxShape::setPropertyValue( UNO_NAME_LINETRANSPARENCE
                    , pLineProperties->Transparence );

            //LineStyle
            if(pLineProperties->LineStyle.hasValue())
                xShape->SvxShape::setPropertyValue( UNO_NAME_LINESTYLE
                    , pLineProperties->LineStyle );

            //LineWidth
            if(pLineProperties->Width.hasValue())
                xShape->SvxShape::setPropertyValue( UNO_NAME_LINEWIDTH
                    , pLineProperties->Width );

            //LineColor
            if(pLineProperties->Color.hasValue())
                xShape->SvxShape::setPropertyValue( UNO_NAME_LINECOLOR
                    , pLineProperties->Color );

            //LineDashName
            if(pLineProperties->DashName.hasValue())
                xShape->SvxShape::setPropertyValue( "LineDashName"
                    , pLineProperties->DashName );

            //LineCap
            if(pLineProperties->LineCap.hasValue())
                xShape->SvxShape::setPropertyValue( UNO_NAME_LINECAP
                    , pLineProperties->LineCap );
        }
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
    return xShape;
}

rtl::Reference<SvxShapePolyPolygon>
    ShapeFactory::createLine ( const rtl::Reference<SvxShapeGroupAnyD>& xTarget,
            const awt::Size& rSize, const awt::Point& rPosition )
{
    //create shape
    rtl::Reference<SvxShapePolyPolygon> xShape = new SvxShapePolyPolygon(nullptr);
    xShape->setShapeKind(SdrObjKind::Line);
    xTarget->addShape(*xShape);
    xShape->setSize( rSize );
    xShape->setPosition( rPosition );

    return xShape;
}

rtl::Reference<SvxShapeRect> ShapeFactory::createInvisibleRectangle(
            const rtl::Reference<SvxShapeGroupAnyD>& xTarget
            , const awt::Size& rSize )
{
    try
    {
        if(!xTarget.is())
            return nullptr;

        rtl::Reference<SvxShapeRect> xShape = new SvxShapeRect(nullptr);
        xShape->setShapeKind(SdrObjKind::Rectangle);
        xTarget->addShape( *xShape );
        ShapeFactory::makeShapeInvisible( xShape );
        xShape->setSize( rSize );
        return xShape;
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
    return nullptr;
}

rtl::Reference<SvxShapeRect> ShapeFactory::createRectangle(
    const rtl::Reference<SvxShapeGroupAnyD>& xTarget,
    const awt::Size& rSize,
    const awt::Point& rPosition,
    const tNameSequence& rPropNames,
    const tAnySequence& rPropValues,
    StackPosition ePos )
{
    rtl::Reference<SvxShapeRect> xShape = new SvxShapeRect(nullptr);
    xShape->setShapeKind(SdrObjKind::Rectangle);
    if (ePos == StackPosition::Bottom)
    {
        uno::Reference<drawing::XShapes2> xTarget2(static_cast<cppu::OWeakObject*>(xTarget.get()), uno::UNO_QUERY);
        assert(xTarget2);
        if (xTarget2.is())
            xTarget2->addBottom(xShape);
    }
    else
        xTarget->addShape(*xShape);

    xShape->setPosition( rPosition );
    xShape->setSize( rSize );
    PropertyMapper::setMultiProperties( rPropNames, rPropValues, *xShape );

    return xShape;
}

rtl::Reference<SvxShapeRect>
    ShapeFactory::createRectangle(
            const rtl::Reference<SvxShapeGroupAnyD>& xTarget )
{
    rtl::Reference<SvxShapeRect> xShape = new SvxShapeRect(nullptr);
    xShape->setShapeKind(SdrObjKind::Rectangle);
    xTarget->addShape( *xShape );

    return xShape;
}

rtl::Reference<SvxShapeText>
        ShapeFactory::createText( const rtl::Reference<SvxShapeGroupAnyD>& xTarget
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
    rtl::Reference<SvxShapeText> xShape = new SvxShapeText(nullptr);
    xShape->setShapeKind(SdrObjKind::Text);
    xTarget->addShape(*xShape);

    //set text
    xShape->setString( rText );

    //set properties
    PropertyMapper::setMultiProperties( rPropNames, rPropValues, *xShape );

    //set position matrix
    //the matrix needs to be set at the end behind autogrow and such position influencing properties
    try
    {
        if (rATransformation.hasValue())
            xShape->SvxShape::setPropertyValue( "Transformation", rATransformation );
        else
            SAL_INFO("chart2", "No rATransformation value is given to ShapeFactory::createText()");

    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
    return xShape;
}

rtl::Reference<SvxShapeText>
    ShapeFactory::createText( const rtl::Reference<SvxShapeGroupAnyD>& xTarget
                , const uno::Sequence< uno::Reference< chart2::XFormattedString > >& xFormattedString
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
    rtl::Reference<SvxShapeText> xShape = new SvxShapeText(nullptr);
    xShape->setShapeKind(SdrObjKind::Text);
    xTarget->addShape(*xShape);

    //set paragraph properties
    bNotEmpty = false;
    // the first cursor is used for appending the next paragraph,
    // after a new string has been inserted the cursor is moved at the end
    // of the inserted string
    // the second cursor is used for selecting the paragraph and apply the
    // passed text properties
    Reference< text::XTextCursor > xInsertCursor = xShape->createTextCursor();
    Reference< text::XTextCursor > xSelectionCursor = xShape->createTextCursor();
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
                    xShape->insertString( xInsertCursor, xFormattedString[nN]->getString(), false );
                    bNotEmpty = true;
                    xSelectionCursor->gotoEnd( true ); // select current paragraph
                    uno::Reference< beans::XPropertySet > xStringProperties( xFormattedString[nN], uno::UNO_QUERY );
                    PropertyMapper::setMappedProperties( xSelectionProp, xStringProperties,
                        PropertyMapper::getPropertyNameMapForTextShapeProperties() );
                }
            }
        }
    }

    if( !bNotEmpty )
        return nullptr;

    //set whole text shape properties
    PropertyMapper::setMultiProperties( rPropNames, rPropValues, *xShape );

    if( rATransformation.hasValue() )
    {
        //set position matrix
        //the matrix needs to be set at the end behind autogrow and such position influencing properties
        try
        {
            xShape->SvxShape::setPropertyValue( "Transformation", rATransformation );
        }
        catch( const uno::Exception& )
        {
            TOOLS_WARN_EXCEPTION("chart2", "" );
        }
    }
    return xShape;
}

rtl::Reference<SvxShapeText>
        ShapeFactory::createText( const rtl::Reference<SvxShapeGroupAnyD>& xTarget,
                const awt::Size& rSize,
                const awt::Point& rPos,
                uno::Sequence< uno::Reference< chart2::XFormattedString > >& xFormattedString,
                const uno::Reference<
                beans::XPropertySet > & xTextProperties,
                double nRotation, const OUString& aName, sal_Int32 nTextMaxWidth )
{
    //create shape and add to page
    rtl::Reference<SvxShapeText> xShape = new SvxShapeText(nullptr);
    xShape->setShapeKind(SdrObjKind::Text);
    try
    {
        xTarget->addShape(*xShape);

        //set text and text properties
        uno::Reference< text::XTextCursor > xTextCursor( xShape->createTextCursor() );
        uno::Reference< text::XTextCursor > xSelectionCursor( xShape->createTextCursor() );
        if( !xTextCursor.is() || !xSelectionCursor.is() )
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
            PropertyMapper::setMultiProperties( aPropNames, aPropValues, *xShape );
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
                size_t nLBreaks = xFormattedString.size() - 1;
                uno::Reference< beans::XPropertySet > xSelectionProp(xSelectionCursor, uno::UNO_QUERY);
                for (const uno::Reference<chart2::XFormattedString>& rxFS : xFormattedString)
                {
                    if (!rxFS->getString().isEmpty())
                    {
                        xTextCursor->gotoEnd(false);
                        xSelectionCursor->gotoEnd(false);
                        OUString aLabel = ShapeFactory::getStackedString(rxFS->getString(), bStackCharacters);
                        if (nLBreaks-- > 0)
                            aLabel += OUStringChar('\r');
                        xShape->insertString(xTextCursor, aLabel, false);
                        xSelectionCursor->gotoEnd(true); // select current paragraph
                        uno::Reference< beans::XPropertySet > xSourceProps(rxFS, uno::UNO_QUERY);
                        if (xFormattedString.size() > 1 && xSelectionProp.is())
                        {
                            PropertyMapper::setMappedProperties(xSelectionProp, xSourceProps,
                                PropertyMapper::getPropertyNameMapForTextShapeProperties());
                        }
                        else
                        {
                            PropertyMapper::setMappedProperties(*xShape, xSourceProps,
                                PropertyMapper::getPropertyNameMapForTextShapeProperties());
                        }
                    }
                }

                // adapt font size according to page size
                awt::Size aOldRefSize;
                if( xTextProperties->getPropertyValue( "ReferencePageSize") >>= aOldRefSize )
                {
                    RelativeSizeHelper::adaptFontSizes( *xShape, aOldRefSize, rSize );
                }
            }
        }
        else
        {
            uno::Reference< beans::XPropertySet > xSelectionProp(xSelectionCursor, uno::UNO_QUERY);
            for (const uno::Reference<chart2::XFormattedString>& rxFS : xFormattedString)
            {
                if (!rxFS->getString().isEmpty())
                {
                    xTextCursor->gotoEnd(false);
                    xSelectionCursor->gotoEnd(false);
                    xShape->insertString(xTextCursor, rxFS->getString(), false);
                    xSelectionCursor->gotoEnd(true); // select current paragraph
                    uno::Reference< beans::XPropertySet > xSourceProps(rxFS, uno::UNO_QUERY);
                    if (xFormattedString.size() > 1 && xSelectionProp.is())
                    {
                        PropertyMapper::setMappedProperties(xSelectionProp, xSourceProps,
                            PropertyMapper::getPropertyNameMapForTextShapeProperties());
                    }
                    else
                    {
                        PropertyMapper::setMappedProperties(*xShape, xSourceProps,
                            PropertyMapper::getPropertyNameMapForTextShapeProperties());
                    }
                }
            }

            if( xFormattedString.hasElements() )
            {
                // adapt font size according to page size
                awt::Size aOldRefSize;
                if( xTextProperties->getPropertyValue("ReferencePageSize") >>= aOldRefSize )
                {
                    RelativeSizeHelper::adaptFontSizes( *xShape, aOldRefSize, rSize );
                }
            }
        }

        // #i109336# Improve auto positioning in chart
        float fFontHeight = 0.0;
        if ( xShape->SvxShape::getPropertyValue( "CharHeight" ) >>= fFontHeight )
        {
            fFontHeight = convertPointToMm100(fFontHeight);
            sal_Int32 nXDistance = static_cast< sal_Int32 >( ::rtl::math::round( fFontHeight * 0.18f ) );
            sal_Int32 nYDistance = static_cast< sal_Int32 >( ::rtl::math::round( fFontHeight * 0.30f ) );
            xShape->SvxShape::setPropertyValue( "TextLeftDistance", uno::Any( nXDistance ) );
            xShape->SvxShape::setPropertyValue( "TextRightDistance", uno::Any( nXDistance ) );
            xShape->SvxShape::setPropertyValue( "TextUpperDistance", uno::Any( nYDistance ) );
            xShape->SvxShape::setPropertyValue( "TextLowerDistance", uno::Any( nYDistance ) );
        }
        sal_Int32 nXPos = rPos.X;
        sal_Int32 nYPos = rPos.Y;

        //set position matrix
        //the matrix needs to be set at the end behind autogrow and such position influencing properties
        ::basegfx::B2DHomMatrix aM;
        aM.rotate( -basegfx::deg2rad(nRotation) );//#i78696#->#i80521#
        aM.translate( nXPos, nYPos );
        xShape->SvxShape::setPropertyValue( "Transformation", uno::Any( B2DHomMatrixToHomogenMatrix3(aM) ) );

        xShape->SvxShape::setPropertyValue( "ParaAdjust", uno::Any( style::ParagraphAdjust_CENTER ) );
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
    return xShape;
}

rtl::Reference<SvxShapeGroupAnyD> ShapeFactory::getChartRootShape(
    const rtl::Reference<SvxDrawPage>& xDrawPage )
{
    rtl::Reference<SvxShapeGroupAnyD> xRet;
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
                    xRet = dynamic_cast<SvxShapeGroupAnyD*>(xShape.get());
                    assert(xRet);
                    break;
                }
            }
        }
    }
    return xRet;
}

void ShapeFactory::makeShapeInvisible( const rtl::Reference< SvxShape >& xShape )
{
    try
    {
        xShape->setPropertyValue( "LineStyle", uno::Any( drawing::LineStyle_NONE ));
        xShape->setPropertyValue( "FillStyle", uno::Any( drawing::FillStyle_NONE ));
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
}

// set a name/CID at a shape (is used for selection handling)

void ShapeFactory::setShapeName( const rtl::Reference< SvxShape >& xShape
                               , const OUString& rName )
{
    if(!xShape.is())
        return;
    try
    {
        xShape->setPropertyValue( UNO_NAME_MISC_OBJ_NAME
            , uno::Any( rName ) );
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
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

bool ShapeFactory::hasPolygonAnyLines( const std::vector<std::vector<css::drawing::Position3D>>& rPoly)
{
    // #i67757# check all contained polygons, if at least one polygon contains 2 or more points, return true
    for( auto const & i : rPoly )
        if( i.size() > 1 )
            return true;
    return false;
}

bool ShapeFactory::isPolygonEmptyOrSinglePoint( const drawing::PolyPolygonShape3D& rPoly)
{
    // true, if empty polypolygon or one polygon with one point
    return !rPoly.SequenceX.hasElements() ||
        ((rPoly.SequenceX.getLength() == 1) && (rPoly.SequenceX[0].getLength() <= 1));
}

bool ShapeFactory::isPolygonEmptyOrSinglePoint( const std::vector<std::vector<css::drawing::Position3D>>& rPoly)
{
    // true, if empty polypolygon or one polygon with one point
    return rPoly.empty() || ((rPoly.size() == 1) && (rPoly[0].size() <= 1));
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

void ShapeFactory::closePolygon( std::vector<std::vector<css::drawing::Position3D>>& rPoly)
{
    OSL_ENSURE( rPoly.size() <= 1, "ShapeFactory::closePolygon - single polygon expected" );
    //add a last point == first point
    if(isPolygonEmptyOrSinglePoint(rPoly))
        return;
    drawing::Position3D aFirst(rPoly[0][0]);
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

::basegfx::B2IRectangle ShapeFactory::getRectangleOfShape( SvxShape& rShape )
{
    ::basegfx::B2IRectangle aRet;

    awt::Point aPos = rShape.getPosition();
    awt::Size aSize = rShape.getSize();
    aRet = BaseGFXHelper::makeRectangle(aPos,aSize);

    return aRet;
}

awt::Size ShapeFactory::getSizeAfterRotation(
         SvxShape& rShape, double fRotationAngleDegree )
{
    awt::Size aRet(0,0);
    const awt::Size aSize( rShape.getSize() );

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
    return aRet;
}

void ShapeFactory::removeSubShapes( const rtl::Reference<SvxShapeGroupAnyD>& xShapes )
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

rtl::Reference<SvxTableShape>
ShapeFactory::createTable(rtl::Reference<SvxShapeGroupAnyD> const& xTarget, OUString const& rName)
{
    if (!xTarget.is())
        return nullptr;

    //create table shape
    rtl::Reference<SvxTableShape> xShape = new SvxTableShape(nullptr);
    xShape->setShapeKind(SdrObjKind::Table);
    xTarget->addShape(*xShape);
    if (!rName.isEmpty())
        setShapeName(xShape, rName);
    return xShape;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
