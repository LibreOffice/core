/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CHART2_SOURCE_VIEW_INC_OPENGLSHAPEFACTORY_HXX
#define INCLUDED_CHART2_SOURCE_VIEW_INC_OPENGLSHAPEFACTORY_HXX

#include "AbstractShapeFactory.hxx"

namespace chart {

namespace opengl {

// This class is stateless!!!!!
// Don't add member variables
class OpenglShapeFactory : public chart::AbstractShapeFactory
{
public:

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
        createGroup2D(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShapes >& xTarget
                , const OUString& aName = OUString() ) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
        createGroup3D(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShapes >& xTarget
                , const OUString& aName = OUString() ) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
            createCube(   const ::com::sun::star::uno::Reference<
                                ::com::sun::star::drawing::XShapes >& xTarget
                        , const ::com::sun::star::drawing::Position3D& rPosition
                        , const ::com::sun::star::drawing::Direction3D& rSize
                        , sal_Int32 nRotateZAngleHundredthDegree
                        , const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xSourceProp
                        , const tPropertyNameMap& rPropertyNameMap
                        , bool bRounded = false) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createCylinder(   const ::com::sun::star::uno::Reference<
                                ::com::sun::star::drawing::XShapes >& xTarget
                        , const ::com::sun::star::drawing::Position3D& rPosition
                        , const ::com::sun::star::drawing::Direction3D& rSize
                        , sal_Int32 nRotateZAngleHundredthDegree ) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createPyramid(    const ::com::sun::star::uno::Reference<
                                ::com::sun::star::drawing::XShapes >& xTarget
                        , const ::com::sun::star::drawing::Position3D& rPosition
                        , const ::com::sun::star::drawing::Direction3D& rSize
                        , double fTopHeight
                        , bool bRotateZ
                        , const ::com::sun::star::uno::Reference<
                            ::com::sun::star::beans::XPropertySet >& xSourceProp
                        , const tPropertyNameMap& rPropertyNameMap) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createCone(       const ::com::sun::star::uno::Reference<
                                ::com::sun::star::drawing::XShapes >& xTarget
                        , const ::com::sun::star::drawing::Position3D& rPosition
                        , const ::com::sun::star::drawing::Direction3D& rSize
                        , double fTopHeight, sal_Int32 nRotateZAngleHundredthDegree ) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createPieSegment2D( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , double fUnitCircleStartAngleDegree, double fUnitCircleWidthAngleDegree
                    , double fUnitCircleInnerRadius, double fUnitCircleOuterRadius
                    , const ::com::sun::star::drawing::Direction3D& rOffset
                    , const ::com::sun::star::drawing::HomogenMatrix& rUnitCircleToScene ) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createPieSegment( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , double fUnitCircleStartAngleDegree, double fUnitCircleWidthAngleDegree
                    , double fUnitCircleInnerRadius, double fUnitCircleOuterRadius
                    , const ::com::sun::star::drawing::Direction3D& rOffset
                    , const ::com::sun::star::drawing::HomogenMatrix& rUnitCircleToScene
                    , double fDepth ) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createStripe( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const Stripe& rStripe
                    , const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xSourceProp
                    , const tPropertyNameMap& rPropertyNameMap
                    , bool bDoubleSided = true
                    , short nRotatedTexture = 0 //0 to 7 are the different possibilities
                    , bool bFlatNormals=true ) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createArea3D( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const ::com::sun::star::drawing::PolyPolygonShape3D& rPolyPolygon
                    , double fDepth) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createArea2D( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const ::com::sun::star::drawing::PolyPolygonShape3D& rPolyPolygon) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createSymbol2D( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const ::com::sun::star::drawing::Position3D& rPos
                    , const ::com::sun::star::drawing::Direction3D& rSize
                    , sal_Int32 nStandardSymbol
                    , sal_Int32 nBorderColor=0
                    , sal_Int32 nFillColor=0 ) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createGraphic2D( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const ::com::sun::star::drawing::Position3D& rPos
                    , const ::com::sun::star::drawing::Direction3D& rSize
                    , const ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >& xGraphic ) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createLine2D( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const ::com::sun::star::drawing::PointSequenceSequence& rPoints
                    , const VLineProperties* pLineProperties = NULL ) override;

    virtual com::sun::star::uno::Reference< com::sun::star::drawing::XShape >
        createLine ( const ::com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& xTarget,
                const com::sun::star::awt::Size& rSize, const com::sun::star::awt::Point& rPosition ) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createLine3D( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const ::com::sun::star::drawing::PolyPolygonShape3D& rPoints
                    , const VLineProperties& rLineProperties ) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createCircle2D( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const ::com::sun::star::drawing::Position3D& rPos
                    , const ::com::sun::star::drawing::Direction3D& rSize ) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createCircle( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const ::com::sun::star::awt::Size& rSize
                    , const ::com::sun::star::awt::Point& rPosition ) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createText( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget2D
                    , const OUString& rText
                    , const tNameSequence& rPropNames
                    , const tAnySequence& rPropValues
                    , const ::com::sun::star::uno::Any& rATransformation
                     ) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createText( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const ::com::sun::star::uno::Sequence< OUString >& rTextParagraphs
                    , const ::com::sun::star::uno::Sequence< tNameSequence >& rParaPropNames
                    , const ::com::sun::star::uno::Sequence< tAnySequence >& rParaPropValues
                    , const tNameSequence& rPropNames
                    , const tAnySequence& rPropValues
                    , const ::com::sun::star::uno::Any& rATransformation ) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createText( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget2D,
                const com::sun::star::awt::Size& rSize,
                const com::sun::star::awt::Point& rPosition,
                com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::chart2::XFormattedString > >& xFormattedString,
                const com::sun::star::uno::Reference<
                com::sun::star::beans::XPropertySet > & xTextProperties,
                double nRotation, const OUString& aName ) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createInvisibleRectangle(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShapes >& xTarget
                , const ::com::sun::star::awt::Size& rSize ) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createRectangle(
            const com::sun::star::uno::Reference<
                com::sun::star::drawing::XShapes >& xTarget,
            const com::sun::star::awt::Size& rSize,
            const com::sun::star::awt::Point& rPosition,
            const tNameSequence& rPropNames,
            const tAnySequence& rPropValues,
            StackPosition ePos = Top ) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createRectangle(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShapes >& xTarget ) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
         getOrCreateChartRootShape( const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XDrawPage>& xPage ) override;

    virtual void setPageSize( com::sun::star::uno::Reference < com::sun::star::drawing::XShapes > xChartShapes, const com::sun::star::awt::Size& rSize ) override;

    virtual void render(com::sun::star::uno::Reference< com::sun::star::drawing::XShapes > xDrawPage, bool bInitOpenGL = true) override;

    virtual bool preRender(com::sun::star::uno::Reference< com::sun::star::drawing::XShapes > xRootShape, OpenGLWindow* pWindow) override;
    virtual void postRender(OpenGLWindow* pWindow) override;

    virtual void clearPage(com::sun::star::uno::Reference< com::sun::star::drawing::XShapes > xDrawPage) override;
};

}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
