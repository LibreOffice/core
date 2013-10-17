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

#include "OpenglShapeFactory.hxx"
#include "DummyXShape.hxx"
#include "ViewDefines.hxx"
#include "Stripe.hxx"
#include "CommonConverters.hxx"
#include "macros.hxx"
#include "PropertyMapper.hxx"
#include <comphelper/InlineContainer.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/CircleKind.hpp>
#include <com/sun/star/drawing/DoubleSequence.hpp>
#include <com/sun/star/drawing/FlagSequence.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/NormalsKind.hpp>
#include <com/sun/star/drawing/PointSequence.hpp>
#include <com/sun/star/drawing/PolygonKind.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>
#include <com/sun/star/drawing/TextFitToSizeType.hpp>
#include <com/sun/star/drawing/TextureProjectionMode.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/uno/Any.hxx>

#include <editeng/unoprnms.hxx>
#include <rtl/math.hxx>
#include <svx/svdocirc.hxx>
#include <svx/svdopath.hxx>

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>

#include <algorithm>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;


namespace chart
{

SAL_CALL extern "C" {

SAL_DLLPUBLIC_EXPORT opengl::OpenglShapeFactory* getOpenglShapeFactory()
{
    return new opengl::OpenglShapeFactory();
}

}

using dummy::DummyXShape;
using dummy::DummyXShapes;
using dummy::DummyCylinder;
using dummy::DummyRectangle;
using dummy::DummyPyramid;
using dummy::DummyCone;

namespace opengl {

uno::Reference< drawing::XShapes > OpenglShapeFactory::getOrCreateChartRootShape(
    const uno::Reference< drawing::XDrawPage>& )
{
    return new dummy::DummyChart();
}

//  methods for 3D shape creation

uno::Reference<drawing::XShape>
        OpenglShapeFactory::createCube(
            const uno::Reference<drawing::XShapes>& xTarget
            , const drawing::Position3D& rPosition, const drawing::Direction3D& rSize
            , sal_Int32 nRotateZAngleHundredthDegree
            , const uno::Reference< beans::XPropertySet >& xSourceProp
            , const tPropertyNameMap& rPropertyNameMap
            , bool bRounded )
{
    dummy::DummyCube* pCube = new dummy::DummyCube(rPosition, rSize,
            nRotateZAngleHundredthDegree, xSourceProp,
            rPropertyNameMap, bRounded);
    xTarget->add(pCube);
    return pCube;
}

uno::Reference<drawing::XShape>
        OpenglShapeFactory::createCylinder(
            const uno::Reference<drawing::XShapes>& xTarget
          , const drawing::Position3D& rPosition, const drawing::Direction3D& rSize
          , sal_Int32 nRotateZAngleHundredthDegree )
{
    dummy::DummyCylinder* pCylinder = new dummy::DummyCylinder( rPosition, rSize,
            nRotateZAngleHundredthDegree );
    xTarget->add(pCylinder);
    return pCylinder;
}

uno::Reference<drawing::XShape>
        OpenglShapeFactory::createPyramid(
            const uno::Reference<drawing::XShapes>& xTarget
          , const drawing::Position3D& rPosition, const drawing::Direction3D& rSize
          , double fTopHeight, bool bRotateZ
          , const uno::Reference< beans::XPropertySet >& xSourceProp
          , const tPropertyNameMap& rPropertyNameMap )
{
    dummy::DummyPyramid* pPyramid = new dummy::DummyPyramid(rPosition, rSize,
            fTopHeight, bRotateZ,
            xSourceProp, rPropertyNameMap );
    xTarget->add(pPyramid);
    return pPyramid;
}

uno::Reference<drawing::XShape>
        OpenglShapeFactory::createCone(
            const uno::Reference<drawing::XShapes>& xTarget
          , const drawing::Position3D& rPosition, const drawing::Direction3D& rSize
          , double fTopHeight, sal_Int32 nRotateZAngleHundredthDegree )
{
    dummy::DummyCone* pCone = new dummy::DummyCone(rPosition, rSize, fTopHeight,
            nRotateZAngleHundredthDegree);
    xTarget->add(pCone);
    return pCone;
}

uno::Reference< drawing::XShape >
        OpenglShapeFactory::createPieSegment2D(
                    const uno::Reference< drawing::XShapes >& xTarget
                    , double fUnitCircleStartAngleDegree, double fUnitCircleWidthAngleDegree
                    , double fUnitCircleInnerRadius, double fUnitCircleOuterRadius
                    , const drawing::Direction3D& rOffset
                    , const drawing::HomogenMatrix& rUnitCircleToScene )
{
    dummy::DummyPieSegment2D* pSegment = new dummy::DummyPieSegment2D(fUnitCircleStartAngleDegree,
            fUnitCircleWidthAngleDegree, fUnitCircleInnerRadius, fUnitCircleOuterRadius,
            rOffset, rUnitCircleToScene);
    xTarget->add(pSegment);
    return pSegment;
}

uno::Reference< drawing::XShape >
        OpenglShapeFactory::createPieSegment(
                    const uno::Reference< drawing::XShapes >& xTarget
                    , double fUnitCircleStartAngleDegree, double fUnitCircleWidthAngleDegree
                    , double fUnitCircleInnerRadius, double fUnitCircleOuterRadius
                    , const drawing::Direction3D& rOffset
                    , const drawing::HomogenMatrix& rUnitCircleToScene
                    , double fDepth )
{
    dummy::DummyPieSegment* pSegment = new dummy::DummyPieSegment(fUnitCircleStartAngleDegree,
            fUnitCircleWidthAngleDegree, fUnitCircleInnerRadius, fUnitCircleOuterRadius,
            rOffset, rUnitCircleToScene, fDepth);

    xTarget->add(pSegment);
    return pSegment;
}

uno::Reference< drawing::XShape >
        OpenglShapeFactory::createStripe( const uno::Reference< drawing::XShapes >& xTarget
                    , const Stripe& rStripe
                    , const uno::Reference< beans::XPropertySet >& xSourceProp
                    , const tPropertyNameMap& rPropertyNameMap
                    , sal_Bool bDoubleSided
                    , short nRotatedTexture
                    , bool bFlatNormals )
{
    dummy::DummyStripe* pStripe = new dummy::DummyStripe(rStripe,
            xSourceProp, rPropertyNameMap,
            bDoubleSided, nRotatedTexture,
            bFlatNormals);
    xTarget->add(pStripe);
    return pStripe;
}

uno::Reference< drawing::XShape >
        OpenglShapeFactory::createArea3D( const uno::Reference< drawing::XShapes >& xTarget
                    , const drawing::PolyPolygonShape3D& rPolyPolygon
                    , double fDepth )
{
    dummy::DummyArea3D* pArea = new dummy::DummyArea3D(rPolyPolygon, fDepth);
    xTarget->add(pArea);
    return pArea;
}

uno::Reference< drawing::XShape >
        OpenglShapeFactory::createArea2D( const uno::Reference< drawing::XShapes >& xTarget
                    , const drawing::PolyPolygonShape3D& rPolyPolygon )
{
    dummy::DummyArea2D* pArea = new dummy::DummyArea2D(rPolyPolygon);
    xTarget->add(pArea);
    return pArea;
}

uno::Reference< drawing::XShape >
        OpenglShapeFactory::createSymbol2D(
                      const uno::Reference< drawing::XShapes >& xTarget
                    , const drawing::Position3D& rPosition
                    , const drawing::Direction3D& rSize
                    , sal_Int32 nStandardSymbol
                    , sal_Int32 nBorderColor
                    , sal_Int32 nFillColor )
{
    dummy::DummySymbol2D* pSymbol = new dummy::DummySymbol2D(rPosition, rSize,
            nStandardSymbol, nBorderColor, nFillColor);
    xTarget->add(pSymbol);
    return pSymbol;
}

uno::Reference< drawing::XShape >
        OpenglShapeFactory::createGraphic2D(
                      const uno::Reference< drawing::XShapes >& xTarget
                    , const drawing::Position3D& rPosition
                    , const drawing::Direction3D& rSize
                    , const uno::Reference< graphic::XGraphic >& xGraphic )
{
    dummy::DummyGraphic2D* pGraphic = new dummy::DummyGraphic2D(rPosition, rSize,
            xGraphic);
    xTarget->add(pGraphic);
    return pGraphic;
}

uno::Reference< drawing::XShapes >
        OpenglShapeFactory::createGroup2D( const uno::Reference< drawing::XShapes >& xTarget
        , OUString aName)
{
    dummy::DummyGroup2D* pNewShape = new dummy::DummyGroup2D(aName);
    xTarget->add(pNewShape);
    return pNewShape;
}

uno::Reference< drawing::XShapes >
        OpenglShapeFactory::createGroup3D( const uno::Reference< drawing::XShapes >& xTarget
        , OUString aName )
{
    dummy::DummyGroup3D* pNewShape = new dummy::DummyGroup3D(aName);
    xTarget->add(pNewShape);
    return pNewShape;
}

uno::Reference< drawing::XShape >
        OpenglShapeFactory::createCircle2D( const uno::Reference< drawing::XShapes >& xTarget
                    , const drawing::Position3D& rPosition
                    , const drawing::Direction3D& rSize )
{
    drawing::Position3D aCenterPosition(
            rPosition.PositionX - (rSize.DirectionX / 2.0),
            rPosition.PositionY - (rSize.DirectionY / 2.0),
            rPosition.PositionZ );
    dummy::DummyCircle* pCircle = new dummy::DummyCircle(Position3DToAWTPoint( aCenterPosition ),
            Direction3DToAWTSize( rSize ));
    xTarget->add(pCircle);
    return pCircle;
}

uno::Reference< drawing::XShape >
    OpenglShapeFactory::createCircle( const uno::Reference< drawing::XShapes >& xTarget
                    , const awt::Size& rSize
                    , const awt::Point& rPosition )
{
    dummy::DummyCircle* pCircle = new dummy::DummyCircle(rPosition, rSize);
    xTarget->add(pCircle);
    return pCircle;
}

uno::Reference< drawing::XShape >
        OpenglShapeFactory::createLine3D( const uno::Reference< drawing::XShapes >& xTarget
                    , const drawing::PolyPolygonShape3D& rPoints
                    , const VLineProperties& rLineProperties )
{
    dummy::DummyLine3D* pLine = new dummy::DummyLine3D(rPoints, rLineProperties);
    xTarget->add(pLine);
    return pLine;
}

uno::Reference< drawing::XShape >
        OpenglShapeFactory::createLine2D( const uno::Reference< drawing::XShapes >& xTarget
                    , const drawing::PointSequenceSequence& rPoints
                    , const VLineProperties* pLineProperties )
{
    dummy::DummyLine2D* pLine = new dummy::DummyLine2D(rPoints, pLineProperties);
    xTarget->add(pLine);
    return pLine;
}

uno::Reference< drawing::XShape >
    OpenglShapeFactory::createLine ( const uno::Reference< drawing::XShapes >& xTarget,
            const awt::Size& rSize, const awt::Point& rPosition )
{
    dummy::DummyLine2D* pLine = new dummy::DummyLine2D(rSize, rPosition);
    xTarget->add(pLine);
    return pLine;
}

uno::Reference< drawing::XShape > OpenglShapeFactory::createInvisibleRectangle(
            const uno::Reference< drawing::XShapes >& xTarget
            , const awt::Size& rSize )
{
    dummy::DummyRectangle* pRectangle = new dummy::DummyRectangle(rSize);
    xTarget->add(pRectangle);
    return pRectangle;
}

uno::Reference< drawing::XShape > OpenglShapeFactory::createRectangle(
        const uno::Reference< drawing::XShapes >& xTarget
        , const awt::Size& rSize
        , const awt::Point& rPosition
        , const tNameSequence& rPropNames
        , const tAnySequence& rPropValues )
{
    dummy::DummyRectangle* pRectangle = new dummy::DummyRectangle(rSize, rPosition,
            rPropNames, rPropValues);

    xTarget->add(pRectangle);
    return pRectangle;
}

uno::Reference< drawing::XShape >
    OpenglShapeFactory::createRectangle(
            const uno::Reference<
            drawing::XShapes >& xTarget)
{
    dummy::DummyRectangle* pRectangle = new dummy::DummyRectangle();
    xTarget->add(pRectangle);
    return pRectangle;
}

uno::Reference< drawing::XShape >
        OpenglShapeFactory::createText( const uno::Reference< drawing::XShapes >& xTarget
                    , const OUString& rText
                    , const tNameSequence& rPropNames
                    , const tAnySequence& rPropValues
                    , const uno::Any& rATransformation )
{
    dummy::DummyText* pText = new dummy::DummyText( rText, rPropNames, rPropValues,
            rATransformation );
    xTarget->add(pText);
    return pText;
}


uno::Reference< drawing::XShape >
        OpenglShapeFactory::createText( const uno::Reference< drawing::XShapes >& ,
                const awt::Size& , const awt::Point& ,
                uno::Sequence< uno::Reference< chart2::XFormattedString > > ,
                const uno::Reference< beans::XPropertySet > ,
                double , const OUString& )
{
    // how the hell should we support that?
    return uno::Reference< drawing::XShape >();
}

void OpenglShapeFactory::createSeries( const uno::Reference<
        drawing::XShapes> & ,
        const DataSeriesState& )
{
}

void OpenglShapeFactory::renderSeries( const uno::Reference<
        drawing::XShapes> & ,
        const DataSeriesState& ,
        const DataSeriesState&,
        double )
{
}

} //namespace dummy

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
