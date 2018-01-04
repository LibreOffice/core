/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CHART2_SOURCE_VIEW_INC_ABSTRACTSHAPEFACTORY_HXX
#define INCLUDED_CHART2_SOURCE_VIEW_INC_ABSTRACTSHAPEFACTORY_HXX

#include "PropertyMapper.hxx"
#include "VLineProperties.hxx"
#include <BaseGFXHelper.hxx>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/chart2/XFormattedString.hpp>
#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/drawing/PolyPolygonShape3D.hpp>
#include <com/sun/star/drawing/Position3D.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XShapes2.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <rtl/ustring.hxx>

class OpenGLWindow;

namespace chart {

// Be careful here not to clash with the SYMBOL_FOO #defines in
// <vcl/vclenum.hxx>
enum SymbolEnum { Symbol_Square=0
                 , Symbol_Diamond
                 , Symbol_DownArrow
                 , Symbol_UpArrow
                 , Symbol_RightArrow
                 , Symbol_LeftArrow
                 , Symbol_Bowtie
                 , Symbol_Sandglass
                 , Symbol_Circle
                 , Symbol_Star
                 , Symbol_X
                 , Symbol_Plus
                 , Symbol_Asterisk
                 , Symbol_HorizontalBar
                 , Symbol_VerticalBar
                 , Symbol_COUNT
};

class Stripe;

class AbstractShapeFactory
{
protected:
    css::uno::Reference< css::lang::XMultiServiceFactory>   m_xShapeFactory;

public:

    enum StackPosition { Top, Bottom };

    static AbstractShapeFactory* getOrCreateShapeFactory(const css::uno::Reference< css::lang::XMultiServiceFactory>& xFactory);

    virtual ~AbstractShapeFactory() {};

    virtual css::uno::Reference< css::drawing::XShapes >
        createGroup2D(
            const css::uno::Reference< css::drawing::XShapes >& xTarget
          , const OUString& aName = OUString() ) = 0;

    virtual css::uno::Reference< css::drawing::XShapes >
        createGroup3D(
            const css::uno::Reference< css::drawing::XShapes >& xTarget
          , const OUString& aName = OUString() ) = 0;

    virtual css::uno::Reference< css::drawing::XShape >
            createCube(   const css::uno::Reference< css::drawing::XShapes >& xTarget
                        , const css::drawing::Position3D& rPosition
                        , const css::drawing::Direction3D& rSize
                        , sal_Int32 nRotateZAngleHundredthDegree
                        , const css::uno::Reference< css::beans::XPropertySet >& xSourceProp
                        , const tPropertyNameMap& rPropertyNameMap
                        , bool bRounded = false) = 0;

    virtual css::uno::Reference< css::drawing::XShape >
        createCylinder(   const css::uno::Reference< css::drawing::XShapes >& xTarget
                        , const css::drawing::Position3D& rPosition
                        , const css::drawing::Direction3D& rSize
                        , sal_Int32 nRotateZAngleHundredthDegree ) = 0;

    virtual css::uno::Reference< css::drawing::XShape >
        createPyramid(    const css::uno::Reference< css::drawing::XShapes >& xTarget
                        , const css::drawing::Position3D& rPosition
                        , const css::drawing::Direction3D& rSize
                        , double fTopHeight
                        , bool bRotateZ
                        , const css::uno::Reference<
                            css::beans::XPropertySet >& xSourceProp
                        , const tPropertyNameMap& rPropertyNameMap) = 0;

    virtual css::uno::Reference< css::drawing::XShape >
        createCone(       const css::uno::Reference< css::drawing::XShapes >& xTarget
                        , const css::drawing::Position3D& rPosition
                        , const css::drawing::Direction3D& rSize
                        , double fTopHeight, sal_Int32 nRotateZAngleHundredthDegree ) = 0;

    virtual css::uno::Reference< css::drawing::XShape >
        createPieSegment2D( const css::uno::Reference< css::drawing::XShapes >& xTarget
                    , double fUnitCircleStartAngleDegree, double fUnitCircleWidthAngleDegree
                    , double fUnitCircleInnerRadius, double fUnitCircleOuterRadius
                    , const css::drawing::Direction3D& rOffset
                    , const css::drawing::HomogenMatrix& rUnitCircleToScene ) = 0;

    virtual css::uno::Reference< css::drawing::XShape >
        createPieSegment( const css::uno::Reference< css::drawing::XShapes >& xTarget
                    , double fUnitCircleStartAngleDegree, double fUnitCircleWidthAngleDegree
                    , double fUnitCircleInnerRadius, double fUnitCircleOuterRadius
                    , const css::drawing::Direction3D& rOffset
                    , const css::drawing::HomogenMatrix& rUnitCircleToScene
                    , double fDepth ) = 0;

    virtual css::uno::Reference< css::drawing::XShape >
        createStripe( const css::uno::Reference< css::drawing::XShapes >& xTarget
                    , const Stripe& rStripe
                    , const css::uno::Reference< css::beans::XPropertySet >& xSourceProp
                    , const tPropertyNameMap& rPropertyNameMap
                    , bool bDoubleSided
                    , short nRotatedTexture = 0 //0 to 7 are the different possibilities
                    , bool bFlatNormals=true ) = 0;

    virtual css::uno::Reference< css::drawing::XShape >
        createArea3D( const css::uno::Reference< css::drawing::XShapes >& xTarget
                    , const css::drawing::PolyPolygonShape3D& rPolyPolygon
                    , double fDepth) = 0;

    virtual css::uno::Reference< css::drawing::XShape >
        createArea2D( const css::uno::Reference< css::drawing::XShapes >& xTarget
                    , const css::drawing::PolyPolygonShape3D& rPolyPolygon) = 0;

    static sal_Int32 getSymbolCount() { return Symbol_COUNT; }

    virtual css::uno::Reference< css::drawing::XShape >
        createSymbol2D( const css::uno::Reference< css::drawing::XShapes >& xTarget
                    , const css::drawing::Position3D& rPos
                    , const css::drawing::Direction3D& rSize
                    , sal_Int32 nStandardSymbol
                    , sal_Int32 nBorderColor
                    , sal_Int32 nFillColor ) = 0;

    virtual css::uno::Reference< css::drawing::XShape >
        createGraphic2D( const css::uno::Reference< css::drawing::XShapes >& xTarget
                    , const css::drawing::Position3D& rPos
                    , const css::drawing::Direction3D& rSize
                    , const css::uno::Reference< css::graphic::XGraphic >& xGraphic ) = 0;

    virtual css::uno::Reference< css::drawing::XShape >
        createLine2D( const css::uno::Reference< css::drawing::XShapes >& xTarget
                    , const css::drawing::PointSequenceSequence& rPoints
                    , const VLineProperties* pLineProperties = nullptr ) = 0;

    virtual css::uno::Reference< css::drawing::XShape >
        createLine ( const css::uno::Reference< css::drawing::XShapes >& xTarget,
                const css::awt::Size& rSize, const css::awt::Point& rPosition ) = 0;

    virtual css::uno::Reference< css::drawing::XShape >
        createLine3D( const css::uno::Reference< css::drawing::XShapes >& xTarget
                    , const css::drawing::PolyPolygonShape3D& rPoints
                    , const VLineProperties& rLineProperties ) = 0;

    virtual css::uno::Reference< css::drawing::XShape >
        createCircle2D( const css::uno::Reference< css::drawing::XShapes >& xTarget
                    , const css::drawing::Position3D& rPos
                    , const css::drawing::Direction3D& rSize ) = 0;

    virtual css::uno::Reference< css::drawing::XShape >
        createCircle( const css::uno::Reference< css::drawing::XShapes >& xTarget
                    , const css::awt::Size& rSize
                    , const css::awt::Point& rPosition ) = 0;

    virtual css::uno::Reference< css::drawing::XShape >
        createText( const css::uno::Reference< css::drawing::XShapes >& xTarget2D
                    , const OUString& rText
                    , const tNameSequence& rPropNames
                    , const tAnySequence& rPropValues
                    , const css::uno::Any& rATransformation
                     ) = 0;

    virtual css::uno::Reference< css::drawing::XShape >
        createText( const css::uno::Reference< css::drawing::XShapes >& xTarget
                    , const css::uno::Sequence< OUString >& rTextParagraphs
                    , const css::uno::Sequence< tNameSequence >& rParaPropNames
                    , const css::uno::Sequence< tAnySequence >& rParaPropValues
                    , const tNameSequence& rPropNames
                    , const tAnySequence& rPropValues
                    , const css::uno::Any& rATransformation ) = 0;

    virtual css::uno::Reference< css::drawing::XShape >
        createText(const css::uno::Reference< css::drawing::XShapes >& xTarget
            , css::uno::Sequence< css::uno::Reference< css::chart2::XFormattedString > >& xFormattedString
            , const tNameSequence& rPropNames
            , const tAnySequence& rPropValues
            , const css::uno::Any& rATransformation) = 0;

    virtual css::uno::Reference< css::drawing::XShape >
        createText( const css::uno::Reference< css::drawing::XShapes >& xTarget2D,
                const css::awt::Size& rSize,
                const css::awt::Point& rPosition,
                css::uno::Sequence< css::uno::Reference< css::chart2::XFormattedString > >& xFormattedString,
                const css::uno::Reference< css::beans::XPropertySet > & xTextProperties,
                double nRotation, const OUString& aName ) = 0;

    virtual css::uno::Reference< css::drawing::XShape >
        createInvisibleRectangle(
            const css::uno::Reference< css::drawing::XShapes >& xTarget
                , const css::awt::Size& rSize ) = 0;

    virtual css::uno::Reference< css::drawing::XShape >
        createRectangle(
            const css::uno::Reference< css::drawing::XShapes >& xTarget,
            const css::awt::Size& rSize,
            const css::awt::Point& rPosition,
            const tNameSequence& rPropNames,
            const tAnySequence& rPropValues,
            StackPosition ePos = Top ) = 0;

    virtual css::uno::Reference< css::drawing::XShape >
        createRectangle(
            const css::uno::Reference< css::drawing::XShapes >& xTarget ) = 0;

    virtual css::uno::Reference< css::drawing::XShapes >
         getOrCreateChartRootShape( const css::uno::Reference< css::drawing::XDrawPage>& xPage ) = 0;

    virtual void setPageSize( css::uno::Reference < css::drawing::XShapes > xChartShapes, const css::awt::Size& rSize ) = 0;

    /**
     * Only necessary for stateless implementations
     */
    virtual void render(css::uno::Reference< css::drawing::XShapes > xRootShape, bool bInitOpenGL) = 0;

    virtual bool preRender(css::uno::Reference< css::drawing::XShapes > xRootShape, OpenGLWindow* pWindow) = 0;
    virtual void postRender(OpenGLWindow* pWindow) = 0;

    virtual void clearPage(css::uno::Reference< css::drawing::XShapes > xRootShape) = 0;

    static css::uno::Reference< css::drawing::XShapes >
         getChartRootShape( const css::uno::Reference< css::drawing::XDrawPage>& xPage );

    static void makeShapeInvisible( const css::uno::Reference< css::drawing::XShape >& xShape );

    static void setShapeName( const css::uno::Reference< css::drawing::XShape >& xShape
            , const OUString& rName );

    static OUString getShapeName( const css::uno::Reference< css::drawing::XShape >& xShape );

    static css::uno::Any makeTransformation( const css::awt::Point& rScreenPosition2D, double fRotationAnglePi=0.0 );

    static OUString getStackedString( const OUString& rString, bool bStacked );

    static bool hasPolygonAnyLines( css::drawing::PolyPolygonShape3D& rPoly );
    static bool isPolygonEmptyOrSinglePoint( css::drawing::PolyPolygonShape3D& rPoly );
    static void closePolygon( css::drawing::PolyPolygonShape3D& rPoly );

    static css::awt::Size calculateNewSizeRespectingAspectRatio(
            const css::awt::Size& rTargetSize
            , const css::awt::Size& rSourceSizeWithCorrectAspectRatio );

    static css::awt::Point calculateTopLeftPositionToCenterObject(
            const css::awt::Point& rTargetAreaPosition
            , const css::awt::Size& rTargetAreaSize
            , const css::awt::Size& rObjectSize );

    static ::basegfx::B2IRectangle getRectangleOfShape(
            const css::uno::Reference< css::drawing::XShape >& xShape );

    static css::awt::Size getSizeAfterRotation(
            const css::uno::Reference< css::drawing::XShape >& xShape, double fRotationAngleDegree );

    static void removeSubShapes( const css::uno::Reference< css::drawing::XShapes >& xShapes );

};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
