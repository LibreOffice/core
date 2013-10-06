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

#include "DummyShapeFactory.hxx"
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

namespace dummy {

uno::Reference< drawing::XShapes > DummyShapeFactory::getOrCreateChartRootShape(
    const uno::Reference< drawing::XDrawPage>& )
{
    return new DummyXShapes();
}

//  methods for 3D shape creation

uno::Reference<drawing::XShape>
        DummyShapeFactory::createCube(
            const uno::Reference<drawing::XShapes>&
            , const drawing::Position3D& , const drawing::Direction3D&
            , sal_Int32
            , const uno::Reference< beans::XPropertySet >&
            , const tPropertyNameMap&
            , bool )
{
    return new DummyXShape();
}

uno::Reference<drawing::XShape>
        DummyShapeFactory::createCylinder(
            const uno::Reference<drawing::XShapes>&
          , const drawing::Position3D& , const drawing::Direction3D&
          , sal_Int32 )
{
    return new DummyXShape();
}

uno::Reference<drawing::XShape>
        DummyShapeFactory::createPyramid(
            const uno::Reference<drawing::XShapes>&
          , const drawing::Position3D& , const drawing::Direction3D&
          , double , bool
          , const uno::Reference< beans::XPropertySet >&
          , const tPropertyNameMap& )
{
    return new DummyXShape();
}

uno::Reference<drawing::XShape>
        DummyShapeFactory::createCone(
            const uno::Reference<drawing::XShapes>&
          , const drawing::Position3D& , const drawing::Direction3D& 
          , double , sal_Int32 )
{
    return new DummyXShape();
}

uno::Reference< drawing::XShape >
        DummyShapeFactory::createPieSegment2D(
                    const uno::Reference< drawing::XShapes >&
                    , double , double
                    , double , double
                    , const drawing::Direction3D&
                    , const drawing::HomogenMatrix& )
{
    return new DummyXShape();
}

uno::Reference< drawing::XShape >
        DummyShapeFactory::createPieSegment(
                    const uno::Reference< drawing::XShapes >&
                    , double , double
                    , double , double
                    , const drawing::Direction3D&
                    , const drawing::HomogenMatrix&
                    , double )
{
    return new DummyXShape();
}

uno::Reference< drawing::XShape >
        DummyShapeFactory::createStripe( const uno::Reference< drawing::XShapes >&
                    , const Stripe&
                    , const uno::Reference< beans::XPropertySet >&
                    , const tPropertyNameMap&
                    , sal_Bool
                    , short
                    , bool )
{
    return new DummyXShape();
}

uno::Reference< drawing::XShape >
        DummyShapeFactory::createArea3D( const uno::Reference< drawing::XShapes >&
                    , const drawing::PolyPolygonShape3D&
                    , double )
{
    return new DummyXShape();
}

uno::Reference< drawing::XShape >
        DummyShapeFactory::createArea2D( const uno::Reference< drawing::XShapes >&
                    , const drawing::PolyPolygonShape3D& )
{
    return new DummyXShape();
}

uno::Reference< drawing::XShape >
        DummyShapeFactory::createSymbol2D(
                      const uno::Reference< drawing::XShapes >&
                    , const drawing::Position3D&
                    , const drawing::Direction3D&
                    , sal_Int32
                    , sal_Int32
                    , sal_Int32 )
{
    return new DummyXShape();
}

uno::Reference< drawing::XShape >
        DummyShapeFactory::createGraphic2D(
                      const uno::Reference< drawing::XShapes >&
                    , const drawing::Position3D&
                    , const drawing::Direction3D&
                    , const uno::Reference< graphic::XGraphic >& )
{
    return new DummyXShape();
}

uno::Reference< drawing::XShapes >
        DummyShapeFactory::createGroup2D( const uno::Reference< drawing::XShapes >&
        , OUString )
{
    return new DummyXShapes();
}

uno::Reference< drawing::XShapes >
        DummyShapeFactory::createGroup3D( const uno::Reference< drawing::XShapes >&
        , OUString )
{
    return new DummyXShapes();
}

uno::Reference< drawing::XShape >
        DummyShapeFactory::createCircle2D( const uno::Reference< drawing::XShapes >&
                    , const drawing::Position3D&
                    , const drawing::Direction3D& )
{
    return new DummyXShape();
}

uno::Reference< drawing::XShape >
    DummyShapeFactory::createCircle( const uno::Reference< drawing::XShapes >&
                    , const awt::Size&
                    , const awt::Point& )
{
    return new DummyXShape();
}

uno::Reference< drawing::XShape >
        DummyShapeFactory::createLine3D( const uno::Reference< drawing::XShapes >&
                    , const drawing::PolyPolygonShape3D&
                    , const VLineProperties& )
{
    return new DummyXShape();
}

uno::Reference< drawing::XShape >
        DummyShapeFactory::createLine2D( const uno::Reference< drawing::XShapes >&
                    , const drawing::PointSequenceSequence&
                    , const VLineProperties* )
{
    return new DummyXShape();
}

uno::Reference< drawing::XShape >
    DummyShapeFactory::createLine ( const uno::Reference< drawing::XShapes >& ,
            const awt::Size& , const awt::Point& )
{
    return new DummyXShape();
}

uno::Reference< drawing::XShape > DummyShapeFactory::createInvisibleRectangle(
            const uno::Reference< drawing::XShapes >&
            , const awt::Size& )
{
    return new DummyXShape();
}

uno::Reference< drawing::XShape > DummyShapeFactory::createRectangle(
        const uno::Reference< drawing::XShapes >&
        , const awt::Size&
        , const awt::Point&
        , const tNameSequence&
        , const tAnySequence& )
{
    return new DummyXShape();
}

uno::Reference< drawing::XShape >
        DummyShapeFactory::createText( const uno::Reference< drawing::XShapes >&
                    , const OUString&
                    , const tNameSequence&
                    , const tAnySequence&
                    , const uno::Any& )
{
    return new DummyXShape();
}

} //namespace dummy

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
