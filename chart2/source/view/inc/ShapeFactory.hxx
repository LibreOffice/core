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
#ifndef INCLUDED_CHART2_SOURCE_VIEW_INC_SHAPEFACTORY_HXX
#define INCLUDED_CHART2_SOURCE_VIEW_INC_SHAPEFACTORY_HXX

#include "PropertyMapper.hxx"
#include <basegfx/range/b2irectangle.hxx>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>

#include <rtl/ustring.hxx>

namespace chart { struct VLineProperties; }
namespace com { namespace sun { namespace star { namespace beans { class XPropertySet; } } } }
namespace com { namespace sun { namespace star { namespace chart2 { class XFormattedString; } } } }
namespace com { namespace sun { namespace star { namespace drawing { class XDrawPage; } } } }
namespace com { namespace sun { namespace star { namespace drawing { class XShape; } } } }
namespace com { namespace sun { namespace star { namespace drawing { class XShapes; } } } }
namespace com { namespace sun { namespace star { namespace drawing { struct HomogenMatrix; } } } }
namespace com { namespace sun { namespace star { namespace drawing { struct PolyPolygonShape3D; } } } }
namespace com { namespace sun { namespace star { namespace drawing { struct Position3D; } } } }
namespace com { namespace sun { namespace star { namespace graphic { class XGraphic; } } } }
namespace com { namespace sun { namespace star { namespace lang { class XMultiServiceFactory; } } } }
namespace com { namespace sun { namespace star { namespace drawing { struct Direction3D; } } } }


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
    css::uno::Reference< css::lang::XMultiServiceFactory>   m_xShapeFactory;

    ShapeFactory(css::uno::Reference< css::lang::XMultiServiceFactory> const & xFactory)
        {m_xShapeFactory = xFactory;}

public:
    enum class StackPosition { Top, Bottom };

    static ShapeFactory* getOrCreateShapeFactory(const css::uno::Reference< css::lang::XMultiServiceFactory>& xFactory);

    ShapeFactory() = delete;
    css::uno::Reference< css::drawing::XShapes >
        createGroup2D(
            const css::uno::Reference< css::drawing::XShapes >& xTarget
          , const OUString& aName = OUString() );

    css::uno::Reference< css::drawing::XShapes >
        createGroup3D(
            const css::uno::Reference< css::drawing::XShapes >& xTarget
          , const OUString& aName = OUString() );

    css::uno::Reference< css::drawing::XShape >
            createCube(   const css::uno::Reference< css::drawing::XShapes >& xTarget
                        , const css::drawing::Position3D& rPosition
                        , const css::drawing::Direction3D& rSize
                        , sal_Int32 nRotateZAngleHundredthDegree
                        , const css::uno::Reference< css::beans::XPropertySet >& xSourceProp
                        , const tPropertyNameMap& rPropertyNameMap
                        , bool bRounded = false);

    css::uno::Reference< css::drawing::XShape >
        createCylinder(   const css::uno::Reference< css::drawing::XShapes >& xTarget
                        , const css::drawing::Position3D& rPosition
                        , const css::drawing::Direction3D& rSize
                        , sal_Int32 nRotateZAngleHundredthDegree );

    css::uno::Reference< css::drawing::XShape >
        createPyramid(    const css::uno::Reference< css::drawing::XShapes >& xTarget
                        , const css::drawing::Position3D& rPosition
                        , const css::drawing::Direction3D& rSize
                        , double fTopHeight
                        , bool bRotateZ
                        , const css::uno::Reference< css::beans::XPropertySet >& xSourceProp
                        , const tPropertyNameMap& rPropertyNameMap);

    css::uno::Reference< css::drawing::XShape >
        createCone(       const css::uno::Reference< css::drawing::XShapes >& xTarget
                        , const css::drawing::Position3D& rPosition
                        , const css::drawing::Direction3D& rSize
                        , double fTopHeight, sal_Int32 nRotateZAngleHundredthDegree );

    css::uno::Reference< css::drawing::XShape >
        createPieSegment2D( const css::uno::Reference< css::drawing::XShapes >& xTarget
                    , double fUnitCircleStartAngleDegree, double fUnitCircleWidthAngleDegree
                    , double fUnitCircleInnerRadius, double fUnitCircleOuterRadius
                    , const css::drawing::Direction3D& rOffset
                    , const css::drawing::HomogenMatrix& rUnitCircleToScene );

    css::uno::Reference< css::drawing::XShape >
        createPieSegment( const css::uno::Reference< css::drawing::XShapes >& xTarget
                    , double fUnitCircleStartAngleDegree, double fUnitCircleWidthAngleDegree
                    , double fUnitCircleInnerRadius, double fUnitCircleOuterRadius
                    , const css::drawing::Direction3D& rOffset
                    , const css::drawing::HomogenMatrix& rUnitCircleToScene
                    , double fDepth );

    css::uno::Reference< css::drawing::XShape >
        createStripe( const css::uno::Reference< css::drawing::XShapes >& xTarget
                    , const Stripe& rStripe
                    , const css::uno::Reference< css::beans::XPropertySet >& xSourceProp
                    , const tPropertyNameMap& rPropertyNameMap
                    , bool bDoubleSided
                    , short nRotatedTexture = 0 //0 to 7 are the different possibilities
                    , bool bFlatNormals=true );

    css::uno::Reference< css::drawing::XShape >
        createArea3D( const css::uno::Reference< css::drawing::XShapes >& xTarget
                    , const css::drawing::PolyPolygonShape3D& rPolyPolygon
                    , double fDepth);

    css::uno::Reference< css::drawing::XShape >
        createArea2D( const css::uno::Reference< css::drawing::XShapes >& xTarget
                    , const css::drawing::PolyPolygonShape3D& rPolyPolygon);

    css::uno::Reference< css::drawing::XShape >
        createSymbol2D( const css::uno::Reference< css::drawing::XShapes >& xTarget
                    , const css::drawing::Position3D& rPos
                    , const css::drawing::Direction3D& rSize
                    , sal_Int32 nStandardSymbol
                    , sal_Int32 nBorderColor
                    , sal_Int32 nFillColor );

    css::uno::Reference< css::drawing::XShape >
        createGraphic2D( const css::uno::Reference< css::drawing::XShapes >& xTarget
                    , const css::drawing::Position3D& rPos
                    , const css::drawing::Direction3D& rSize
                    , const css::uno::Reference< css::graphic::XGraphic >& xGraphic );

    css::uno::Reference< css::drawing::XShape >
        createLine2D( const css::uno::Reference< css::drawing::XShapes >& xTarget
                    , const css::drawing::PointSequenceSequence& rPoints
                    , const VLineProperties* pLineProperties = nullptr );

    css::uno::Reference< css::drawing::XShape >
        createLine ( const css::uno::Reference< css::drawing::XShapes >& xTarget,
                const css::awt::Size& rSize, const css::awt::Point& rPosition );

    css::uno::Reference< css::drawing::XShape >
        createLine3D( const css::uno::Reference< css::drawing::XShapes >& xTarget
                    , const css::drawing::PolyPolygonShape3D& rPoints
                    , const VLineProperties& rLineProperties );

    css::uno::Reference< css::drawing::XShape >
        createCircle2D( const css::uno::Reference< css::drawing::XShapes >& xTarget
                    , const css::drawing::Position3D& rPos
                    , const css::drawing::Direction3D& rSize );

    css::uno::Reference< css::drawing::XShape >
        createCircle( const css::uno::Reference< css::drawing::XShapes >& xTarget
                    , const css::awt::Size& rSize
                    , const css::awt::Point& rPosition );

    css::uno::Reference< css::drawing::XShape >
        createText( const css::uno::Reference< css::drawing::XShapes >& xTarget2D
                    , const OUString& rText
                    , const tNameSequence& rPropNames
                    , const tAnySequence& rPropValues
                    , const css::uno::Any& rATransformation
                     );

    /** This method creates a text shape made up by a set of paragraphs.
     *  For each paragraph the related text style is passed to the method.
     *
     *  @param xTarget
     *      where to append the new created text shape.
     *
     *  @param rTextParagraphs
     *      the set of paragraphs which made up the text shape.
     *
     *  @param rParaPropNames
     *      a collection of lists of text property names:
     *      there must be a list of text property names for each paragraph.
     *
     *  @param rParaPropValues
     *      a collection of lists of text property values:
     *      there must be a list of text property values for each paragraph.
     *
     *  @param rPropNames
     *      a list of text property names to be applied to the whole text shape.
     *
     *  @param rPropValues
     *      a list of text property values to be applied to the whole text shape.
     *
     *  @param rATransformation
     *      a transformation to be applied to the text shape as final step.
     *
     */
    css::uno::Reference< css::drawing::XShape >
        createText( const css::uno::Reference< css::drawing::XShapes >& xTarget
                    , const css::uno::Sequence< OUString >& rTextParagraphs
                    , const css::uno::Sequence< tNameSequence >& rParaPropNames
                    , const css::uno::Sequence< tAnySequence >& rParaPropValues
                    , const tNameSequence& rPropNames
                    , const tAnySequence& rPropValues
                    , const css::uno::Any& rATransformation );

    css::uno::Reference< css::drawing::XShape >
        createText(const css::uno::Reference< css::drawing::XShapes >& xTarget
            , css::uno::Sequence< css::uno::Reference< css::chart2::XFormattedString > >& xFormattedString
            , const tNameSequence& rPropNames
            , const tAnySequence& rPropValues
            , const css::uno::Any& rATransformation);

    css::uno::Reference< css::drawing::XShape >
        createText( const css::uno::Reference< css::drawing::XShapes >& xTarget2D,
                const css::awt::Size& rSize,
                const css::awt::Point& rPosition,
                css::uno::Sequence< css::uno::Reference< css::chart2::XFormattedString > >& xFormattedString,
                const css::uno::Reference< css::beans::XPropertySet > & xTextProperties,
                double nRotation, const OUString& aName );

    css::uno::Reference< css::drawing::XShape >
        createInvisibleRectangle(
            const css::uno::Reference< css::drawing::XShapes >& xTarget
          , const css::awt::Size& rSize );

    css::uno::Reference< css::drawing::XShape >
        createRectangle(
            const css::uno::Reference< css::drawing::XShapes >& xTarget,
            const css::awt::Size& rSize,
            const css::awt::Point& rPosition,
            const tNameSequence& rPropNames,
            const tAnySequence& rPropValues,
            StackPosition ePos = StackPosition::Top );

    css::uno::Reference< css::drawing::XShape >
        createRectangle(
            const css::uno::Reference< css::drawing::XShapes >& xTarget );

    css::uno::Reference< css::drawing::XShapes >
         getOrCreateChartRootShape( const css::uno::Reference<
            css::drawing::XDrawPage>& xPage );

    static void setPageSize(const css::uno::Reference<css::drawing::XShapes>& xChartShapes,
                     const css::awt::Size& rSize);

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

    static sal_Int32 getSymbolCount() { return Symbol_COUNT; }

private:
    css::uno::Reference< css::drawing::XShape >
        impl_createCube( const css::uno::Reference< css::drawing::XShapes >& xTarget
                    , const css::drawing::Position3D& rPosition
                    , const css::drawing::Direction3D& rSize, sal_Int32 nRotateZAngleHundredthDegree
                    , bool bRounded );

    css::uno::Reference< css::drawing::XShape >
        impl_createConeOrCylinder( const css::uno::Reference< css::drawing::XShapes >& xTarget
                    , const css::drawing::Position3D& rPosition
                    , const css::drawing::Direction3D& rSize
                    , double fTopHeight, sal_Int32 nRotateZAngleHundredthDegree
                    , bool bCylinder);
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
