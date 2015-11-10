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
#include "BaseGFXHelper.hxx"
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
// <rsc/rsc-vcl-shared-types.hxx>
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

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>
        m_xShapeFactory;
public:

    enum StackPosition { Top, Bottom };
    void setShapeFactory(com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory> xFactory)
        { m_xShapeFactory = xFactory; }

    static AbstractShapeFactory* getOrCreateShapeFactory(::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory> xFactory);

    virtual ~AbstractShapeFactory() {};

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
        createGroup2D(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShapes >& xTarget
                , const OUString& aName = OUString() ) = 0;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
        createGroup3D(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShapes >& xTarget
                , const OUString& aName = OUString() ) = 0;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
            createCube(   const ::com::sun::star::uno::Reference<
                                ::com::sun::star::drawing::XShapes >& xTarget
                        , const ::com::sun::star::drawing::Position3D& rPosition
                        , const ::com::sun::star::drawing::Direction3D& rSize
                        , sal_Int32 nRotateZAngleHundredthDegree
                        , const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xSourceProp
                        , const tPropertyNameMap& rPropertyNameMap
                        , bool bRounded = false) = 0;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createCylinder(   const ::com::sun::star::uno::Reference<
                                ::com::sun::star::drawing::XShapes >& xTarget
                        , const ::com::sun::star::drawing::Position3D& rPosition
                        , const ::com::sun::star::drawing::Direction3D& rSize
                        , sal_Int32 nRotateZAngleHundredthDegree ) = 0;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createPyramid(    const ::com::sun::star::uno::Reference<
                                ::com::sun::star::drawing::XShapes >& xTarget
                        , const ::com::sun::star::drawing::Position3D& rPosition
                        , const ::com::sun::star::drawing::Direction3D& rSize
                        , double fTopHeight
                        , bool bRotateZ
                        , const ::com::sun::star::uno::Reference<
                            ::com::sun::star::beans::XPropertySet >& xSourceProp
                        , const tPropertyNameMap& rPropertyNameMap) = 0;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createCone(       const ::com::sun::star::uno::Reference<
                                ::com::sun::star::drawing::XShapes >& xTarget
                        , const ::com::sun::star::drawing::Position3D& rPosition
                        , const ::com::sun::star::drawing::Direction3D& rSize
                        , double fTopHeight, sal_Int32 nRotateZAngleHundredthDegree ) = 0;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createPieSegment2D( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , double fUnitCircleStartAngleDegree, double fUnitCircleWidthAngleDegree
                    , double fUnitCircleInnerRadius, double fUnitCircleOuterRadius
                    , const ::com::sun::star::drawing::Direction3D& rOffset
                    , const ::com::sun::star::drawing::HomogenMatrix& rUnitCircleToScene ) = 0;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createPieSegment( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , double fUnitCircleStartAngleDegree, double fUnitCircleWidthAngleDegree
                    , double fUnitCircleInnerRadius, double fUnitCircleOuterRadius
                    , const ::com::sun::star::drawing::Direction3D& rOffset
                    , const ::com::sun::star::drawing::HomogenMatrix& rUnitCircleToScene
                    , double fDepth ) = 0;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createStripe( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const Stripe& rStripe
                    , const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xSourceProp
                    , const tPropertyNameMap& rPropertyNameMap
                    , bool bDoubleSided = true
                    , short nRotatedTexture = 0 //0 to 7 are the different possibilities
                    , bool bFlatNormals=true ) = 0;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createArea3D( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const ::com::sun::star::drawing::PolyPolygonShape3D& rPolyPolygon
                    , double fDepth) = 0;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createArea2D( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const ::com::sun::star::drawing::PolyPolygonShape3D& rPolyPolygon) = 0;

    static sal_Int32 getSymbolCount() { return Symbol_COUNT; }

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createSymbol2D( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const ::com::sun::star::drawing::Position3D& rPos
                    , const ::com::sun::star::drawing::Direction3D& rSize
                    , sal_Int32 nStandardSymbol
                    , sal_Int32 nBorderColor=0
                    , sal_Int32 nFillColor=0 ) = 0;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createGraphic2D( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const ::com::sun::star::drawing::Position3D& rPos
                    , const ::com::sun::star::drawing::Direction3D& rSize
                    , const ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >& xGraphic ) = 0;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createLine2D( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const ::com::sun::star::drawing::PointSequenceSequence& rPoints
                    , const VLineProperties* pLineProperties = nullptr ) = 0;

    virtual com::sun::star::uno::Reference< com::sun::star::drawing::XShape >
        createLine ( const ::com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& xTarget,
                const com::sun::star::awt::Size& rSize, const com::sun::star::awt::Point& rPosition ) = 0;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createLine3D( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const ::com::sun::star::drawing::PolyPolygonShape3D& rPoints
                    , const VLineProperties& rLineProperties ) = 0;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createCircle2D( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const ::com::sun::star::drawing::Position3D& rPos
                    , const ::com::sun::star::drawing::Direction3D& rSize ) = 0;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createCircle( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const ::com::sun::star::awt::Size& rSize
                    , const ::com::sun::star::awt::Point& rPosition ) = 0;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createText( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget2D
                    , const OUString& rText
                    , const tNameSequence& rPropNames
                    , const tAnySequence& rPropValues
                    , const ::com::sun::star::uno::Any& rATransformation
                     ) = 0;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createText( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const ::com::sun::star::uno::Sequence< OUString >& rTextParagraphs
                    , const ::com::sun::star::uno::Sequence< tNameSequence >& rParaPropNames
                    , const ::com::sun::star::uno::Sequence< tAnySequence >& rParaPropValues
                    , const tNameSequence& rPropNames
                    , const tAnySequence& rPropValues
                    , const ::com::sun::star::uno::Any& rATransformation ) = 0;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createText( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget2D,
                const com::sun::star::awt::Size& rSize,
                const com::sun::star::awt::Point& rPosition,
                com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::chart2::XFormattedString > >& xFormattedString,
                const com::sun::star::uno::Reference<
                com::sun::star::beans::XPropertySet > & xTextProperties,
                double nRotation, const OUString& aName ) = 0;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createInvisibleRectangle(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShapes >& xTarget
                , const ::com::sun::star::awt::Size& rSize ) = 0;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createRectangle(
            const com::sun::star::uno::Reference<
                com::sun::star::drawing::XShapes >& xTarget,
            const com::sun::star::awt::Size& rSize,
            const com::sun::star::awt::Point& rPosition,
            const tNameSequence& rPropNames,
            const tAnySequence& rPropValues,
            StackPosition ePos = Top ) = 0;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createRectangle(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShapes >& xTarget ) = 0;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
         getOrCreateChartRootShape( const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XDrawPage>& xPage ) = 0;

    virtual void setPageSize( com::sun::star::uno::Reference < com::sun::star::drawing::XShapes > xChartShapes, const com::sun::star::awt::Size& rSize ) = 0;

    /**
     * Only necessary for stateless implementations
     */
    virtual void render(com::sun::star::uno::Reference< com::sun::star::drawing::XShapes > xRootShape, bool bInitOpenGL = true) = 0;

    virtual bool preRender(com::sun::star::uno::Reference< com::sun::star::drawing::XShapes > xRootShape, OpenGLWindow* pWindow) = 0;
    virtual void postRender(OpenGLWindow* pWindow) = 0;

    virtual void clearPage(com::sun::star::uno::Reference< com::sun::star::drawing::XShapes > xRootShape) = 0;

    static ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
         getChartRootShape( const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XDrawPage>& xPage );

    static void makeShapeInvisible( const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XShape >& xShape );

    static void setShapeName( const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XShape >& xShape
            , const OUString& rName );

    static OUString getShapeName( const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XShape >& xShape );

    static ::com::sun::star::uno::Any makeTransformation( const ::com::sun::star::awt::Point& rScreenPosition2D, double fRotationAnglePi=0.0 );

    static OUString getStackedString( const OUString& rString, bool bStacked=true );

    static bool hasPolygonAnyLines( ::com::sun::star::drawing::PolyPolygonShape3D& rPoly );
    static bool isPolygonEmptyOrSinglePoint( ::com::sun::star::drawing::PolyPolygonShape3D& rPoly );
    static void closePolygon( ::com::sun::star::drawing::PolyPolygonShape3D& rPoly );

    static ::com::sun::star::awt::Size calculateNewSizeRespectingAspectRatio(
            const ::com::sun::star::awt::Size& rTargetSize
            , const ::com::sun::star::awt::Size& rSourceSizeWithCorrectAspectRatio );

    static ::com::sun::star::awt::Point calculateTopLeftPositionToCenterObject(
            const ::com::sun::star::awt::Point& rTargetAreaPosition
            , const ::com::sun::star::awt::Size& rTargetAreaSize
            , const ::com::sun::star::awt::Size& rObjectSize );

    static ::basegfx::B2IRectangle getRectangleOfShape(
            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape );

    static ::com::sun::star::awt::Size getSizeAfterRotation(
            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape, double fRotationAngleDegree );

    static void removeSubShapes( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xShapes );

};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
