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
#pragma once

#include "PropertyMapper.hxx"
#include <basegfx/range/b2irectangle.hxx>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>

#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <svx/unoshape.hxx>
#include <svx/unodraw/SvxTableShape.hxx>
#include <svx/unopage.hxx>

namespace chart { struct VLineProperties; }
namespace com::sun::star::beans { class XPropertySet; }
namespace com::sun::star::chart2 { class XFormattedString; }
namespace com::sun::star::drawing { class XShape; }
namespace com::sun::star::drawing { struct HomogenMatrix; }
namespace com::sun::star::drawing { struct PolyPolygonShape3D; }
namespace com::sun::star::drawing { struct Position3D; }
namespace com::sun::star::graphic { class XGraphic; }
namespace com::sun::star::drawing { struct Direction3D; }

namespace chart
{
class Stripe;

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


class ShapeFactory
{
public:
    enum class StackPosition { Top, Bottom };

    ShapeFactory() = delete;

    static rtl::Reference< SvxShapeGroup >
        createGroup2D(
            const rtl::Reference<SvxShapeGroupAnyD>& xTarget
          , const OUString& aName = OUString() );

    static rtl::Reference< SvxShapeGroup >
        createGroup2D(
            const rtl::Reference<SvxDrawPage>& xTarget
          , const OUString& aName = OUString() );

    static rtl::Reference<Svx3DSceneObject>
        createGroup3D(
            const rtl::Reference<SvxShapeGroupAnyD>& xTarget
          , const OUString& aName = OUString() );

    static rtl::Reference<Svx3DExtrudeObject>
            createCube(   const rtl::Reference<SvxShapeGroupAnyD>& xTarget
                        , const css::drawing::Position3D& rPosition
                        , const css::drawing::Direction3D& rSize
                        , sal_Int32 nRotateZAngleHundredthDegree
                        , const css::uno::Reference< css::beans::XPropertySet >& xSourceProp
                        , const tPropertyNameMap& rPropertyNameMap
                        , bool bRounded = false);

    static rtl::Reference<Svx3DLatheObject>
        createCylinder(   const rtl::Reference<SvxShapeGroupAnyD>& xTarget
                        , const css::drawing::Position3D& rPosition
                        , const css::drawing::Direction3D& rSize
                        , sal_Int32 nRotateZAngleHundredthDegree );

    static rtl::Reference<Svx3DSceneObject>
        createPyramid(    const rtl::Reference<SvxShapeGroupAnyD>& xTarget
                        , const css::drawing::Position3D& rPosition
                        , const css::drawing::Direction3D& rSize
                        , double fTopHeight
                        , bool bRotateZ
                        , const css::uno::Reference< css::beans::XPropertySet >& xSourceProp
                        , const tPropertyNameMap& rPropertyNameMap);

    static rtl::Reference<Svx3DLatheObject>
        createCone(       const rtl::Reference<SvxShapeGroupAnyD>& xTarget
                        , const css::drawing::Position3D& rPosition
                        , const css::drawing::Direction3D& rSize
                        , double fTopHeight, sal_Int32 nRotateZAngleHundredthDegree );

    static rtl::Reference<SvxShapePolyPolygon>
        createPieSegment2D( const rtl::Reference<SvxShapeGroupAnyD>& xTarget
                    , double fUnitCircleStartAngleDegree, double fUnitCircleWidthAngleDegree
                    , double fUnitCircleInnerRadius, double fUnitCircleOuterRadius
                    , const css::drawing::Direction3D& rOffset
                    , const css::drawing::HomogenMatrix& rUnitCircleToScene );

    static rtl::Reference<Svx3DExtrudeObject>
        createPieSegment( const rtl::Reference<SvxShapeGroupAnyD>& xTarget
                    , double fUnitCircleStartAngleDegree, double fUnitCircleWidthAngleDegree
                    , double fUnitCircleInnerRadius, double fUnitCircleOuterRadius
                    , const css::drawing::Direction3D& rOffset
                    , const css::drawing::HomogenMatrix& rUnitCircleToScene
                    , double fDepth );

    static rtl::Reference<Svx3DPolygonObject>
        createStripe( const rtl::Reference<SvxShapeGroupAnyD>& xTarget
                    , const Stripe& rStripe
                    , const css::uno::Reference< css::beans::XPropertySet >& xSourceProp
                    , const tPropertyNameMap& rPropertyNameMap
                    , bool bDoubleSided
                    , short nRotatedTexture = 0 //0 to 7 are the different possibilities
                    , bool bFlatNormals=true );

    static rtl::Reference<Svx3DExtrudeObject>
        createArea3D( const rtl::Reference<SvxShapeGroupAnyD>& xTarget
                    , const std::vector<std::vector<css::drawing::Position3D>>& rPolyPolygon
                    , double fDepth);

    static rtl::Reference<SvxShapePolyPolygon>
        createArea2D( const rtl::Reference<SvxShapeGroupAnyD>& xTarget
                    , const std::vector<std::vector<css::drawing::Position3D>>& rPolyPolygon);

    static rtl::Reference<SvxShapePolyPolygon>
        createSymbol2D( const rtl::Reference<SvxShapeGroupAnyD>& xTarget
                    , const css::drawing::Position3D& rPos
                    , const css::drawing::Direction3D& rSize
                    , sal_Int32 nStandardSymbol
                    , sal_Int32 nBorderColor
                    , sal_Int32 nFillColor );

    static rtl::Reference<SvxGraphicObject>
        createGraphic2D( const rtl::Reference<SvxShapeGroupAnyD>& xTarget
                    , const css::drawing::Position3D& rPos
                    , const css::drawing::Direction3D& rSize
                    , const css::uno::Reference< css::graphic::XGraphic >& xGraphic );

    static rtl::Reference<SvxShapePolyPolygon>
        createLine2D( const rtl::Reference<SvxShapeGroupAnyD>& xTarget
                    , const css::drawing::PointSequenceSequence& rPoints
                    , const VLineProperties* pLineProperties = nullptr );
    static rtl::Reference<SvxShapePolyPolygon>
        createLine2D( const rtl::Reference<SvxShapeGroupAnyD>& xTarget
                    , const std::vector<std::vector<css::drawing::Position3D>>& rPoints
                    , const VLineProperties* pLineProperties = nullptr );

    static rtl::Reference<SvxShapePolyPolygon>
        createLine ( const rtl::Reference<SvxShapeGroupAnyD>& xTarget,
                const css::awt::Size& rSize, const css::awt::Point& rPosition );

    static rtl::Reference<Svx3DPolygonObject>
        createLine3D( const rtl::Reference<SvxShapeGroupAnyD>& xTarget
                    , const std::vector<std::vector<css::drawing::Position3D>>& rPoints
                    , const VLineProperties& rLineProperties );

    static rtl::Reference<SvxShapeCircle>
        createCircle2D( const rtl::Reference<SvxShapeGroupAnyD>& xTarget
                    , const css::drawing::Position3D& rPos
                    , const css::drawing::Direction3D& rSize );

    static rtl::Reference<SvxShapeCircle>
        createCircle( const rtl::Reference<SvxShapeGroupAnyD>& xTarget
                    , const css::awt::Size& rSize
                    , const css::awt::Point& rPosition );

    static rtl::Reference<SvxShapeText>
        createText( const rtl::Reference<SvxShapeGroupAnyD>& xTarget2D
                    , const OUString& rText
                    , const tNameSequence& rPropNames
                    , const tAnySequence& rPropValues
                    , const css::uno::Any& rATransformation
                     );

    static rtl::Reference<SvxShapeText>
        createText(const rtl::Reference<SvxShapeGroupAnyD>& xTarget
            , const css::uno::Sequence< css::uno::Reference< css::chart2::XFormattedString > >& xFormattedString
            , const tNameSequence& rPropNames
            , const tAnySequence& rPropValues
            , const css::uno::Any& rATransformation);

    static rtl::Reference<SvxShapeText>
        createText( const rtl::Reference<SvxShapeGroupAnyD>& xTarget2D,
                const css::awt::Size& rSize,
                const css::awt::Point& rPosition,
                css::uno::Sequence< css::uno::Reference< css::chart2::XFormattedString > >& xFormattedString,
                const css::uno::Reference< css::beans::XPropertySet > & xTextProperties,
                double nRotation, const OUString& aName, sal_Int32 nTextMaxWidth );

    static rtl::Reference<SvxTableShape> createTable(rtl::Reference<SvxShapeGroupAnyD> const& xTarget, OUString const& rName = OUString());

    static rtl::Reference<SvxShapeRect>
        createInvisibleRectangle(
            const rtl::Reference<SvxShapeGroupAnyD>& xTarget
          , const css::awt::Size& rSize );

    static rtl::Reference<SvxShapeRect>
        createRectangle(
            const rtl::Reference<SvxShapeGroupAnyD>& xTarget,
            const css::awt::Size& rSize,
            const css::awt::Point& rPosition,
            const tNameSequence& rPropNames,
            const tAnySequence& rPropValues,
            StackPosition ePos = StackPosition::Top );

    static rtl::Reference<SvxShapeRect>
        createRectangle(
            const rtl::Reference<SvxShapeGroupAnyD>& xTarget );

    static rtl::Reference<SvxShapeGroupAnyD>
         getOrCreateChartRootShape( const rtl::Reference<SvxDrawPage>& xPage );

    static void setPageSize(const rtl::Reference<SvxShapeGroupAnyD>& xChartShapes,
                     const css::awt::Size& rSize);

    static rtl::Reference<SvxShapeGroupAnyD>
         getChartRootShape( const rtl::Reference<SvxDrawPage>& xPage );

    static void makeShapeInvisible( const rtl::Reference< SvxShape >& rShape );

    static void setShapeName( const rtl::Reference< SvxShape >& xShape
            , const OUString& rName );

    static OUString getShapeName( const css::uno::Reference< css::drawing::XShape >& xShape );

    static css::uno::Any makeTransformation( const css::awt::Point& rScreenPosition2D, double fRotationAnglePi=0.0 );

    static OUString getStackedString( const OUString& rString, bool bStacked );

    static bool hasPolygonAnyLines( const std::vector<std::vector<css::drawing::Position3D>>& rPoly );
    static bool isPolygonEmptyOrSinglePoint( const css::drawing::PolyPolygonShape3D& rPoly );
    static bool isPolygonEmptyOrSinglePoint( const std::vector<std::vector<css::drawing::Position3D>>& rPoly );
    static void closePolygon( css::drawing::PolyPolygonShape3D& rPoly );
    static void closePolygon( std::vector<std::vector<css::drawing::Position3D>>& rPoly );

    static css::awt::Size calculateNewSizeRespectingAspectRatio(
            const css::awt::Size& rTargetSize
            , const css::awt::Size& rSourceSizeWithCorrectAspectRatio );

    static css::awt::Point calculateTopLeftPositionToCenterObject(
            const css::awt::Point& rTargetAreaPosition
            , const css::awt::Size& rTargetAreaSize
            , const css::awt::Size& rObjectSize );

    static ::basegfx::B2IRectangle getRectangleOfShape( SvxShape& rShape );

    static css::awt::Size getSizeAfterRotation(
            SvxShape& rShape, double fRotationAngleDegree );

    static void removeSubShapes( const rtl::Reference<SvxShapeGroupAnyD>& xShapes );

    static sal_Int32 getSymbolCount() { return Symbol_COUNT; }

private:
    static rtl::Reference<Svx3DExtrudeObject>
        impl_createCube( const rtl::Reference<SvxShapeGroupAnyD>& xTarget
                    , const css::drawing::Position3D& rPosition
                    , const css::drawing::Direction3D& rSize, sal_Int32 nRotateZAngleHundredthDegree
                    , bool bRounded );

    static rtl::Reference<Svx3DLatheObject>
        impl_createConeOrCylinder( const rtl::Reference<SvxShapeGroupAnyD>& xTarget
                    , const css::drawing::Position3D& rPosition
                    , const css::drawing::Direction3D& rSize
                    , double fTopHeight, sal_Int32 nRotateZAngleHundredthDegree
                    , bool bCylinder);
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
