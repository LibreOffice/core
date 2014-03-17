/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CHART2_OPENGL_SHAPE_FACTORY
#define CHART2_OPENGL_SHAPE_FACTORY

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
                , const OUString& aName = OUString() );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
        createGroup3D(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShapes >& xTarget
                , const OUString& aName = OUString() );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
            createCube(   const ::com::sun::star::uno::Reference<
                                ::com::sun::star::drawing::XShapes >& xTarget
                        , const ::com::sun::star::drawing::Position3D& rPosition
                        , const ::com::sun::star::drawing::Direction3D& rSize
                        , sal_Int32 nRotateZAngleHundredthDegree
                        , const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xSourceProp
                        , const tPropertyNameMap& rPropertyNameMap
                        , bool bRounded = false);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createCylinder(   const ::com::sun::star::uno::Reference<
                                ::com::sun::star::drawing::XShapes >& xTarget
                        , const ::com::sun::star::drawing::Position3D& rPosition
                        , const ::com::sun::star::drawing::Direction3D& rSize
                        , sal_Int32 nRotateZAngleHundredthDegree );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createPyramid(    const ::com::sun::star::uno::Reference<
                                ::com::sun::star::drawing::XShapes >& xTarget
                        , const ::com::sun::star::drawing::Position3D& rPosition
                        , const ::com::sun::star::drawing::Direction3D& rSize
                        , double fTopHeight
                        , bool bRotateZ
                        , const ::com::sun::star::uno::Reference<
                            ::com::sun::star::beans::XPropertySet >& xSourceProp
                        , const tPropertyNameMap& rPropertyNameMap);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createCone(       const ::com::sun::star::uno::Reference<
                                ::com::sun::star::drawing::XShapes >& xTarget
                        , const ::com::sun::star::drawing::Position3D& rPosition
                        , const ::com::sun::star::drawing::Direction3D& rSize
                        , double fTopHeight, sal_Int32 nRotateZAngleHundredthDegree );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createPieSegment2D( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , double fUnitCircleStartAngleDegree, double fUnitCircleWidthAngleDegree
                    , double fUnitCircleInnerRadius, double fUnitCircleOuterRadius
                    , const ::com::sun::star::drawing::Direction3D& rOffset
                    , const ::com::sun::star::drawing::HomogenMatrix& rUnitCircleToScene );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createPieSegment( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , double fUnitCircleStartAngleDegree, double fUnitCircleWidthAngleDegree
                    , double fUnitCircleInnerRadius, double fUnitCircleOuterRadius
                    , const ::com::sun::star::drawing::Direction3D& rOffset
                    , const ::com::sun::star::drawing::HomogenMatrix& rUnitCircleToScene
                    , double fDepth );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createStripe( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const Stripe& rStripe
                    , const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xSourceProp
                    , const tPropertyNameMap& rPropertyNameMap
                    , sal_Bool bDoubleSided = true
                    , short nRotatedTexture = 0 //0 to 7 are the different possibilities
                    , bool bFlatNormals=true );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createArea3D( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const ::com::sun::star::drawing::PolyPolygonShape3D& rPolyPolygon
                    , double fDepth);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createArea2D( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const ::com::sun::star::drawing::PolyPolygonShape3D& rPolyPolygon);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createSymbol2D( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const ::com::sun::star::drawing::Position3D& rPos
                    , const ::com::sun::star::drawing::Direction3D& rSize
                    , sal_Int32 nStandardSymbol
                    , sal_Int32 nBorderColor=0
                    , sal_Int32 nFillColor=0 );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createGraphic2D( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const ::com::sun::star::drawing::Position3D& rPos
                    , const ::com::sun::star::drawing::Direction3D& rSize
                    , const ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >& xGraphic );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createLine2D( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const ::com::sun::star::drawing::PointSequenceSequence& rPoints
                    , const VLineProperties* pLineProperties = NULL );

    virtual com::sun::star::uno::Reference< com::sun::star::drawing::XShape >
        createLine ( const ::com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& xTarget,
                const com::sun::star::awt::Size& rSize, const com::sun::star::awt::Point& rPosition );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createLine3D( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const ::com::sun::star::drawing::PolyPolygonShape3D& rPoints
                    , const VLineProperties& rLineProperties );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createCircle2D( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const ::com::sun::star::drawing::Position3D& rPos
                    , const ::com::sun::star::drawing::Direction3D& rSize );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createCircle( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const ::com::sun::star::awt::Size& rSize
                    , const ::com::sun::star::awt::Point& rPosition );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createText( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget2D
                    , const OUString& rText
                    , const tNameSequence& rPropNames
                    , const tAnySequence& rPropValues
                    , const ::com::sun::star::uno::Any& rATransformation
                     );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createText( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget2D,
                const com::sun::star::awt::Size& rSize,
                const com::sun::star::awt::Point& rPosition,
                com::sun::star::uno::Sequence< com::sun::star::uno::Reference< com::sun::star::chart2::XFormattedString > >& xFormattedString,
                const com::sun::star::uno::Reference<
                com::sun::star::beans::XPropertySet > & xTextProperties,
                double nRotation, const OUString& aName );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createInvisibleRectangle(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShapes >& xTarget
                , const ::com::sun::star::awt::Size& rSize );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createRectangle(
            const com::sun::star::uno::Reference<
                com::sun::star::drawing::XShapes >& xTarget,
            const com::sun::star::awt::Size& rSize,
            const com::sun::star::awt::Point& rPosition,
            const tNameSequence& rPropNames,
            const tAnySequence& rPropValues,
            StackPosition ePos = Top );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createRectangle(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShapes >& xTarget );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
         getOrCreateChartRootShape( const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XDrawPage>& xPage );

    virtual void setPageSize( com::sun::star::uno::Reference < com::sun::star::drawing::XShapes > xChartShapes, const com::sun::star::awt::Size& rSize );

    virtual void render(com::sun::star::uno::Reference< com::sun::star::drawing::XShapes > xRootShape) SAL_OVERRIDE;

    virtual void clearPage(com::sun::star::uno::Reference< com::sun::star::drawing::XShapes > xRootShape) SAL_OVERRIDE;
};

}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
