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

#ifndef INCLUDED_BASEGFX_TOOLS_CANVASTOOLS_HXX
#define INCLUDED_BASEGFX_TOOLS_CANVASTOOLS_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <basegfx/basegfxdllapi.h>


namespace com { namespace sun { namespace star { namespace geometry
{
    struct AffineMatrix2D;
    struct AffineMatrix3D;
    struct Matrix2D;
    struct RealPoint2D;
    struct RealSize2D;
    struct RealRectangle2D;
    struct RealRectangle3D;
    struct IntegerPoint2D;
    struct IntegerSize2D;
    struct IntegerRectangle2D;
    struct RealBezierSegment2D;
} } } }

namespace com { namespace sun { namespace star { namespace rendering
{
    class  XGraphicDevice;
    class  XPolyPolygon2D;
} } } }

namespace com { namespace sun { namespace star { namespace awt
{
    struct Point;
    struct Size;
    struct Rectangle;
} } } }

namespace basegfx
{
    class B2DHomMatrix;
    class B3DHomMatrix;
    class B2DVector;
    class B2DPoint;
    class B2DRange;
    class B3DRange;
    class B2IBox;
    class B2IVector;
    class B2IPoint;
    class B2IRange;
    class B2DPolygon;
    class B2DPolyPolygon;

    namespace unotools
    {
        // Polygon conversions


        BASEGFX_DLLPUBLIC css::uno::Reference< css::rendering::XPolyPolygon2D >
            xPolyPolygonFromB2DPolygon( const css::uno::Reference< css::rendering::XGraphicDevice >&  xGraphicDevice,
                                        const ::basegfx::B2DPolygon&                        rPoly    );

        BASEGFX_DLLPUBLIC css::uno::Reference< css::rendering::XPolyPolygon2D >
            xPolyPolygonFromB2DPolyPolygon( const css::uno::Reference< css::rendering::XGraphicDevice >& xGraphicDevice,
                                            const ::basegfx::B2DPolyPolygon&                    rPolyPoly    );


        BASEGFX_DLLPUBLIC css::uno::Sequence<
              css::uno::Sequence< css::geometry::RealBezierSegment2D > >
                    bezierSequenceSequenceFromB2DPolyPolygon( const ::basegfx::B2DPolyPolygon& rPolyPoly );

        BASEGFX_DLLPUBLIC css::uno::Sequence<
              css::uno::Sequence< css::geometry::RealPoint2D > >
                    pointSequenceSequenceFromB2DPolyPolygon( const ::basegfx::B2DPolyPolygon& rPolyPoly );

        BASEGFX_DLLPUBLIC ::basegfx::B2DPolygon polygonFromPoint2DSequence(
            const css::uno::Sequence< css::geometry::RealPoint2D >& rPoints );

        BASEGFX_DLLPUBLIC ::basegfx::B2DPolyPolygon polyPolygonFromPoint2DSequenceSequence(
            const css::uno::Sequence< css::uno::Sequence< css::geometry::RealPoint2D > >& rPoints );

        BASEGFX_DLLPUBLIC ::basegfx::B2DPolygon polygonFromBezier2DSequence(
            const css::uno::Sequence< css::geometry::RealBezierSegment2D >& rPoints );

        BASEGFX_DLLPUBLIC ::basegfx::B2DPolyPolygon polyPolygonFromBezier2DSequenceSequence(
            const css::uno::Sequence< css::uno::Sequence< css::geometry::RealBezierSegment2D > >& rPoints );

        BASEGFX_DLLPUBLIC ::basegfx::B2DPolyPolygon b2DPolyPolygonFromXPolyPolygon2D(
            const css::uno::Reference< css::rendering::XPolyPolygon2D >& rPoly );

        // Matrix conversions


        BASEGFX_DLLPUBLIC css::geometry::AffineMatrix2D&
            affineMatrixFromHomMatrix( css::geometry::AffineMatrix2D&  matrix,
                                       const ::basegfx::B2DHomMatrix&               transform);

        BASEGFX_DLLPUBLIC css::geometry::AffineMatrix3D& affineMatrixFromHomMatrix3D(
            css::geometry::AffineMatrix3D& matrix,
            const ::basegfx::B3DHomMatrix& transform);

        BASEGFX_DLLPUBLIC ::basegfx::B2DHomMatrix&
            homMatrixFromAffineMatrix( ::basegfx::B2DHomMatrix&                transform,
                                       const css::geometry::AffineMatrix2D&    matrix );

        BASEGFX_DLLPUBLIC ::basegfx::B3DHomMatrix homMatrixFromAffineMatrix3D( const css::geometry::AffineMatrix3D& matrix );

        // Geometry conversions


        BASEGFX_DLLPUBLIC css::geometry::RealSize2D        size2DFromB2DSize( const ::basegfx::B2DVector& );
        BASEGFX_DLLPUBLIC css::geometry::RealPoint2D       point2DFromB2DPoint( const ::basegfx::B2DPoint& );
        BASEGFX_DLLPUBLIC css::geometry::RealRectangle2D   rectangle2DFromB2DRectangle( const ::basegfx::B2DRange& );
        BASEGFX_DLLPUBLIC css::geometry::RealRectangle3D   rectangle3DFromB3DRectangle( const ::basegfx::B3DRange& );

        BASEGFX_DLLPUBLIC ::basegfx::B2DPoint       b2DPointFromRealPoint2D( const css::geometry::RealPoint2D& );
        BASEGFX_DLLPUBLIC ::basegfx::B2DRange       b2DRectangleFromRealRectangle2D( const css::geometry::RealRectangle2D& );
        BASEGFX_DLLPUBLIC ::basegfx::B3DRange       b3DRectangleFromRealRectangle3D( const css::geometry::RealRectangle3D& );

        BASEGFX_DLLPUBLIC css::geometry::IntegerSize2D         integerSize2DFromB2ISize( const ::basegfx::B2IVector& );

        BASEGFX_DLLPUBLIC ::basegfx::B2IVector      b2ISizeFromIntegerSize2D( const css::geometry::IntegerSize2D& );
        BASEGFX_DLLPUBLIC ::basegfx::B2IRange       b2IRectangleFromIntegerRectangle2D( const css::geometry::IntegerRectangle2D& );

        BASEGFX_DLLPUBLIC ::basegfx::B2IRange       b2IRectangleFromAwtRectangle( const css::awt::Rectangle& );

        // Geometry comparisons


        /** Return smalltest integer range, which completely contains
            given floating point range.

            @param rRange
            Input range. Values must be within the representable
            bounds of sal_Int32

            @return the closest integer range, which completely
            contains rRange.
         */
        BASEGFX_DLLPUBLIC ::basegfx::B2IRange   b2ISurroundingRangeFromB2DRange( const ::basegfx::B2DRange& rRange );

        /** Return smalltest integer box, which completely contains
            given floating point range.

            @param rRange
            Input range. Values must be within the representable
            bounds of sal_Int32

            @return the closest integer box, which completely contains
            rRange. Note that this box will contain all pixel affected
            by a polygon fill operation over the input range.
         */
        BASEGFX_DLLPUBLIC ::basegfx::B2IBox     b2ISurroundingBoxFromB2DRange( const ::basegfx::B2DRange& rRange );

        /** Return smalltest B2DRange with integer values, which
            completely contains given floating point range.

            @param rRange
            Input range.

            @return the closest B2DRange with integer coordinates,
            which completely contains rRange.
         */
        BASEGFX_DLLPUBLIC ::basegfx::B2DRange   b2DSurroundingIntegerRangeFromB2DRange( const ::basegfx::B2DRange& rRange );

    }
}

#endif // INCLUDED_BASEGFX_TOOLS_CANVASTOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
