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
#ifndef _CHART2_VIEW_SHAPEFACTORY_HXX
#define _CHART2_VIEW_SHAPEFACTORY_HXX

#include "PropertyMapper.hxx"
#include "VLineProperties.hxx"
#include "BaseGFXHelper.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/drawing/PolyPolygonShape3D.hpp>
#include <com/sun/star/drawing/Position3D.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>

namespace chart
{

class Stripe;
class ShapeFactory
{
public:
    ShapeFactory(::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory> xFactory)
        {m_xShapeFactory = xFactory;}

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
        createGroup2D(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShapes >& xTarget
                , OUString aName = OUString() );

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
        createGroup3D(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShapes >& xTarget
                , OUString aName = OUString() );

    //------

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
            createCube(   const ::com::sun::star::uno::Reference<
                                ::com::sun::star::drawing::XShapes >& xTarget
                        , const ::com::sun::star::drawing::Position3D& rPosition
                        , const ::com::sun::star::drawing::Direction3D& rSize
                        , sal_Int32 nRotateZAngleHundredthDegree
                        , const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xSourceProp
                        , const tPropertyNameMap& rPropertyNameMap
                        , bool bRounded = false);

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createCylinder(   const ::com::sun::star::uno::Reference<
                                ::com::sun::star::drawing::XShapes >& xTarget
                        , const ::com::sun::star::drawing::Position3D& rPosition
                        , const ::com::sun::star::drawing::Direction3D& rSize
                        , sal_Int32 nRotateZAngleHundredthDegree );

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createPyramid(    const ::com::sun::star::uno::Reference<
                                ::com::sun::star::drawing::XShapes >& xTarget
                        , const ::com::sun::star::drawing::Position3D& rPosition
                        , const ::com::sun::star::drawing::Direction3D& rSize
                        , double fTopHeight
                        , bool bRotateZ
                        , const ::com::sun::star::uno::Reference<
                            ::com::sun::star::beans::XPropertySet >& xSourceProp
                        , const tPropertyNameMap& rPropertyNameMap);

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createCone(       const ::com::sun::star::uno::Reference<
                                ::com::sun::star::drawing::XShapes >& xTarget
                        , const ::com::sun::star::drawing::Position3D& rPosition
                        , const ::com::sun::star::drawing::Direction3D& rSize
                        , double fTopHeight, sal_Int32 nRotateZAngleHundredthDegree );

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createPieSegment2D( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , double fUnitCircleStartAngleDegree, double fUnitCircleWidthAngleDegree
                    , double fUnitCircleInnerRadius, double fUnitCircleOuterRadius
                    , const ::com::sun::star::drawing::Direction3D& rOffset
                    , const ::com::sun::star::drawing::HomogenMatrix& rUnitCircleToScene );

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createPieSegment( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , double fUnitCircleStartAngleDegree, double fUnitCircleWidthAngleDegree
                    , double fUnitCircleInnerRadius, double fUnitCircleOuterRadius
                    , const ::com::sun::star::drawing::Direction3D& rOffset
                    , const ::com::sun::star::drawing::HomogenMatrix& rUnitCircleToScene
                    , double fDepth );

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createStripe( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const Stripe& rStripe
                    , const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xSourceProp
                    , const tPropertyNameMap& rPropertyNameMap
                    , sal_Bool bDoubleSided = true
                    , short nRotatedTexture = 0 //0 to 7 are the different possibilities
                    , bool bFlatNormals=true );

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createArea3D( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const ::com::sun::star::drawing::PolyPolygonShape3D& rPolyPolygon
                    , double fDepth);

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createArea2D( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const ::com::sun::star::drawing::PolyPolygonShape3D& rPolyPolygon);

    static sal_Int32 getSymbolCount();

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createSymbol2D( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const ::com::sun::star::drawing::Position3D& rPos
                    , const ::com::sun::star::drawing::Direction3D& rSize
                    , sal_Int32 nStandardSymbol
                    , sal_Int32 nBorderColor=0
                    , sal_Int32 nFillColor=0 );

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createGraphic2D( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const ::com::sun::star::drawing::Position3D& rPos
                    , const ::com::sun::star::drawing::Direction3D& rSize
                    , const ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >& xGraphic );

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createLine2D( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const ::com::sun::star::drawing::PointSequenceSequence& rPoints
                    , const VLineProperties* pLineProperties = NULL );

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createLine3D( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const ::com::sun::star::drawing::PolyPolygonShape3D& rPoints
                    , const VLineProperties& rLineProperties );

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createCircle2D( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const ::com::sun::star::drawing::Position3D& rPos
                    , const ::com::sun::star::drawing::Direction3D& rSize );

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createText( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget2D
                    , const OUString& rText
                    , const tNameSequence& rPropNames
                    , const tAnySequence& rPropValues
                    , const ::com::sun::star::uno::Any& rATransformation
                     );

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        createInvisibleRectangle(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::XShapes >& xTarget
                , const ::com::sun::star::awt::Size& rSize );

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
         getOrCreateChartRootShape( const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XDrawPage>& xPage );

    static ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >
         getChartRootShape( const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XDrawPage>& xPage );

    //------
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

private:
    ShapeFactory();

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        impl_createCube( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const ::com::sun::star::drawing::Position3D& rPosition
                    , const ::com::sun::star::drawing::Direction3D& rSize, sal_Int32 nRotateZAngleHundredthDegree
                    , bool bRounded );

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
        impl_createConeOrCylinder( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& xTarget
                    , const ::com::sun::star::drawing::Position3D& rPosition
                    , const ::com::sun::star::drawing::Direction3D& rSize
                    , double fTopHeight, sal_Int32 nRotateZAngleHundredthDegree
                    , bool bCylinder = false);

    //member:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>
        m_xShapeFactory;
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
