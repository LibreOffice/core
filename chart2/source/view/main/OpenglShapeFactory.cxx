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

#include <svx/unoshape.hxx>
#include "OpenglShapeFactory.hxx"
#include "DummyXShape.hxx"
#include "ViewDefines.hxx"
#include "Stripe.hxx"
#include "CommonConverters.hxx"
#include "macros.hxx"
#include "PropertyMapper.hxx"
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
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.hxx>

#include <editeng/unoprnms.hxx>
#include <rtl/math.hxx>
#include <svx/svdocirc.hxx>
#include <svx/svdopath.hxx>
#include <vcl/openglwin.hxx>

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>

#include "RelativeSizeHelper.hxx"

using namespace ::com::sun::star;

namespace chart
{

extern "C" {
    SAL_DLLPUBLIC_EXPORT opengl::OpenglShapeFactory* getOpenglShapeFactory()
                              {    return new opengl::OpenglShapeFactory();}
    }

using dummy::DummyCylinder;
using dummy::DummyRectangle;
using dummy::DummyPyramid;
using dummy::DummyCone;

namespace opengl {

namespace {

uno::Reference< drawing::XShapes > getChartShape(
    const uno::Reference< drawing::XDrawPage>& xDrawPage )
{
    uno::Reference< drawing::XShapes > xRet;
    uno::Reference< drawing::XShapes > xShapes( xDrawPage, uno::UNO_QUERY );
    if( xShapes.is() )
    {
        sal_Int32 nCount = xShapes->getCount();
        uno::Reference< drawing::XShape > xShape;
        for( sal_Int32 nN = nCount; nN--; )
        {
            if( xShapes->getByIndex( nN ) >>= xShape )
            {

                OUString aRet;

                uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
                xProp->getPropertyValue( UNO_NAME_MISC_OBJ_NAME ) >>= aRet;
                if( aRet == "com.sun.star.chart2.shapes" )
                {
                    xRet = dynamic_cast<SvxDummyShapeContainer&>(*xShape.get()).getWrappedShape();
                    break;
                }
            }
        }
    }
    return xRet;
}

}

uno::Reference< drawing::XShapes > OpenglShapeFactory::getOrCreateChartRootShape(
    const uno::Reference< drawing::XDrawPage>& xDrawPage )
{
    uno::Reference< drawing::XShapes > xRet( getChartShape( xDrawPage ) );
    if( !xRet.is()  )
    {
        //create the root shape
        SAL_WARN("chart2.opengl", "getOrCreateChartRootShape");

        dummy::DummyChart *pChart = new dummy::DummyChart();
        SvxDummyShapeContainer* pContainer = new SvxDummyShapeContainer(pChart);
        pContainer->setSize(awt::Size(0,0));
        xRet = pChart;
        xDrawPage->add(pContainer);
    }
    return xRet;
}

void OpenglShapeFactory::setPageSize( uno::Reference < drawing::XShapes > xChartShapes, const awt::Size& rSize )
{
    uno::Reference< drawing::XShape > xShape(xChartShapes, uno::UNO_QUERY_THROW);
    xShape->setSize(rSize);
}

//  methods for 3D shape creation

uno::Reference<drawing::XShape>
        OpenglShapeFactory::createCube(
            const uno::Reference<drawing::XShapes>& xTarget
            , const drawing::Position3D& rPosition, const drawing::Direction3D& rSize
            , sal_Int32
            , const uno::Reference< beans::XPropertySet >& xSourceProp
            , const tPropertyNameMap& rPropertyNameMap
            , bool )
{
    dummy::DummyCube* pCube = new dummy::DummyCube(rPosition, rSize,
            xSourceProp,
            rPropertyNameMap);
    xTarget->add(pCube);
    return pCube;
}

uno::Reference<drawing::XShape>
        OpenglShapeFactory::createCylinder(
            const uno::Reference<drawing::XShapes>& xTarget
          , const drawing::Position3D& rPosition, const drawing::Direction3D& rSize
          , sal_Int32 )
{
    dummy::DummyCylinder* pCylinder = new dummy::DummyCylinder( rPosition, rSize );
    xTarget->add(pCylinder);
    return pCylinder;
}

uno::Reference<drawing::XShape>
        OpenglShapeFactory::createPyramid(
            const uno::Reference<drawing::XShapes>& xTarget
          , const drawing::Position3D& rPosition, const drawing::Direction3D& rSize
          , double, bool
          , const uno::Reference< beans::XPropertySet >& xSourceProp
          , const tPropertyNameMap& rPropertyNameMap )
{
    dummy::DummyPyramid* pPyramid = new dummy::DummyPyramid(rPosition, rSize,
            xSourceProp, rPropertyNameMap );
    xTarget->add(pPyramid);
    return pPyramid;
}

uno::Reference<drawing::XShape>
        OpenglShapeFactory::createCone(
            const uno::Reference<drawing::XShapes>& xTarget
          , const drawing::Position3D& rPosition, const drawing::Direction3D& rSize
          , double, sal_Int32 )
{
    dummy::DummyCone* pCone = new dummy::DummyCone(rPosition, rSize);
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
                    , double, double
                    , double, double
                    , const drawing::Direction3D&
                    , const drawing::HomogenMatrix&
                    , double )
{
    dummy::DummyPieSegment* pSegment = new dummy::DummyPieSegment;

    xTarget->add(pSegment);
    return pSegment;
}

uno::Reference< drawing::XShape >
        OpenglShapeFactory::createStripe( const uno::Reference< drawing::XShapes >& xTarget
                    , const Stripe&
                    , const uno::Reference< beans::XPropertySet >& xSourceProp
                    , const tPropertyNameMap& rPropertyNameMap
                    , bool
                    , short
                    , bool )
{
    dummy::DummyStripe* pStripe = new dummy::DummyStripe(
            xSourceProp, rPropertyNameMap);
    xTarget->add(pStripe);
    return pStripe;
}

uno::Reference< drawing::XShape >
        OpenglShapeFactory::createArea3D( const uno::Reference< drawing::XShapes >& xTarget
                    , const drawing::PolyPolygonShape3D&
                    , double )
{
    dummy::DummyArea3D* pArea = new dummy::DummyArea3D;
    xTarget->add(pArea);
    return pArea;
}

uno::Reference< drawing::XShape >
        OpenglShapeFactory::createArea2D( const uno::Reference< drawing::XShapes >& xTarget
                    , const drawing::PolyPolygonShape3D& rPolyPolygon )
{
    dummy::DummyArea2D* pArea = new dummy::DummyArea2D(PolyToPointSequence(rPolyPolygon));
    xTarget->add(pArea);
    return pArea;
}

uno::Reference< drawing::XShape >
        OpenglShapeFactory::createSymbol2D(
                      const uno::Reference< drawing::XShapes >& xTarget
                    , const drawing::Position3D& rPosition
                    , const drawing::Direction3D& rSize
                    , sal_Int32 nStandardSymbol
                    , sal_Int32
                    , sal_Int32 nFillColor )
{
    dummy::DummySymbol2D* pSymbol = new dummy::DummySymbol2D(rPosition, rSize,
            nStandardSymbol, nFillColor);
    xTarget->add(pSymbol);
    return pSymbol;
}

uno::Reference< drawing::XShape >
        OpenglShapeFactory::createGraphic2D(
                      const uno::Reference< drawing::XShapes >& xTarget
                    , const drawing::Position3D& rPosition
                    , const drawing::Direction3D& rSize
                    , const uno::Reference< graphic::XGraphic >&  )
{
    dummy::DummyGraphic2D* pGraphic = new dummy::DummyGraphic2D(rPosition, rSize);
    xTarget->add(pGraphic);
    return pGraphic;
}

uno::Reference< drawing::XShapes >
        OpenglShapeFactory::createGroup2D( const uno::Reference< drawing::XShapes >& xTarget
        , const OUString& aName)
{
    dummy::DummyGroup2D* pNewShape = new dummy::DummyGroup2D(aName);
    xTarget->add(pNewShape);
    return pNewShape;
}

uno::Reference< drawing::XShapes >
        OpenglShapeFactory::createGroup3D( const uno::Reference< drawing::XShapes >& xTarget
        , const OUString& aName )
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
                    , const drawing::PolyPolygonShape3D&
                    , const VLineProperties& rLineProperties )
{
    dummy::DummyLine3D* pLine = new dummy::DummyLine3D(rLineProperties);
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
    pRectangle->setPropertyValue("Invisible", uno::Any(true));
    xTarget->add(pRectangle);
    return pRectangle;
}

uno::Reference< drawing::XShape > OpenglShapeFactory::createRectangle(
    const uno::Reference< drawing::XShapes >& xTarget,
    const awt::Size& rSize,
    const awt::Point& rPosition,
    const tNameSequence& rPropNames,
    const tAnySequence& rPropValues,
    StackPosition /*ePos*/ )
{
    dummy::DummyRectangle* pRectangle = new dummy::DummyRectangle(rSize, rPosition,
            rPropNames, rPropValues);

    // TODO : Honor stack position.
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
            rATransformation, xTarget, 0 );
    return pText;
}

uno::Reference< drawing::XShape >
        OpenglShapeFactory::createText( const uno::Reference< drawing::XShapes >& xTarget
                    , const uno::Sequence< OUString >& rTextParagraphs
                    , const uno::Sequence< tNameSequence >& /*rParaPropNames*/
                    , const uno::Sequence< tAnySequence >& /*rParaPropValues*/
                    , const tNameSequence& rPropNames
                    , const tAnySequence& rPropValues
                    , const uno::Any& rATransformation )
{
    dummy::DummyText* pText = new dummy::DummyText( rTextParagraphs[0], rPropNames, rPropValues,
            rATransformation, xTarget, 0 );
    return pText;
}

uno::Reference< drawing::XShape >
        OpenglShapeFactory::createText( const uno::Reference< drawing::XShapes >& xTarget,
                const awt::Size& , const awt::Point& rPos,
                uno::Sequence< uno::Reference< chart2::XFormattedString > >& rFormattedString,
                const uno::Reference< beans::XPropertySet > & xTextProperties,
                double nRotation, const OUString& rName)
{
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
        drawing::TextHorizontalAdjust eHorizontalAdjust = drawing::TextHorizontalAdjust_CENTER;
        drawing::TextVerticalAdjust eVerticalAdjust = drawing::TextVerticalAdjust_CENTER;

        aValueMap.insert( tPropertyNameValueMap::value_type( "TextHorizontalAdjust", uno::Any(eHorizontalAdjust) ) ); // drawing::TextHorizontalAdjust
        aValueMap.insert( tPropertyNameValueMap::value_type( "TextVerticalAdjust", uno::Any(eVerticalAdjust) ) ); //drawing::TextVerticalAdjust
        aValueMap.insert( tPropertyNameValueMap::value_type( "TextAutoGrowHeight", uno::Any(true) ) ); // sal_Bool
        aValueMap.insert( tPropertyNameValueMap::value_type( "TextAutoGrowWidth", uno::Any(true) ) ); // sal_Bool

    }

    //set global title properties
    tNameSequence aPropNames;
    tAnySequence aPropValues;
    PropertyMapper::getMultiPropertyListsFromValueMap( aPropNames, aPropValues, aValueMap );

    OUString aString = rFormattedString[0]->getString();

    sal_Int32 nXPos = rPos.X;
    sal_Int32 nYPos = rPos.Y;
    ::basegfx::B2DHomMatrix aM;
    aM.rotate( -nRotation*F_PI/180.0 );//#i78696#->#i80521#
    aM.translate( nXPos, nYPos );

    dummy::DummyText* pText = new dummy::DummyText(aString, aPropNames, aPropValues,
            uno::Any(B2DHomMatrixToHomogenMatrix3(aM)), xTarget, nRotation);
    pText->setName(rName);
    return pText;
}

void OpenglShapeFactory::render(uno::Reference< drawing::XShapes > xRootShape, bool bInitOpenGL)
{
    dummy::DummyChart& rChart = dynamic_cast<dummy::DummyChart&>(*xRootShape.get());
    if(bInitOpenGL)
    {
        rChart.invalidateInit();
    }
    rChart.render();
}

bool OpenglShapeFactory::preRender(uno::Reference< drawing::XShapes > xRootShape, OpenGLWindow* pWindow)
{
    if(!pWindow)
        return false;

    pWindow->Show();
    pWindow->getContext().makeCurrent();
    Size aSize = pWindow->GetSizePixel();
    pWindow->getContext().setWinSize(aSize);
    dummy::DummyChart& rChart = dynamic_cast<dummy::DummyChart&>(*xRootShape.get());
    rChart.getRenderer().SetSizePixel(aSize.Width(), aSize.Height());
    return true;
}

void OpenglShapeFactory::postRender(OpenGLWindow* pWindow)
{
    pWindow->getContext().swapBuffers();
}

void OpenglShapeFactory::clearPage(uno::Reference< drawing::XShapes > xRootShape)
{
    dummy::DummyChart& rChart = dynamic_cast<dummy::DummyChart&>(*xRootShape.get());
    rChart.clear();
}

} //namespace dummy

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
