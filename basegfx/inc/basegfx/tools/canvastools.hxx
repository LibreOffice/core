/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#define _BGFX_TOOLS_CANVASTOOLS_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>


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
    class B2IVector;
    class B2IPoint;
    class B2IRange;
    class B2DPolygon;
    class B2DPolyPolygon;

    namespace unotools
    {
        // Polygon conversions
        // ===================================================================

        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D >
            xPolyPolygonFromB2DPolygon( const ::com::sun::star::uno::Reference<
                                            ::com::sun::star::rendering::XGraphicDevice >&  xGraphicDevice,
                                        const ::basegfx::B2DPolygon&                        rPoly    );

        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D >
            xPolyPolygonFromB2DPolyPolygon( const ::com::sun::star::uno::Reference<
                                                 ::com::sun::star::rendering::XGraphicDevice >&     xGraphicDevice,
                                            const ::basegfx::B2DPolyPolygon&                    rPolyPoly    );


        ::com::sun::star::uno::Sequence<
              ::com::sun::star::uno::Sequence<
                  ::com::sun::star::geometry::RealBezierSegment2D > >
                    bezierSequenceSequenceFromB2DPolyPolygon( const ::basegfx::B2DPolyPolygon& rPolyPoly );

        ::com::sun::star::uno::Sequence<
              ::com::sun::star::uno::Sequence<
                  ::com::sun::star::geometry::RealPoint2D > >
                    pointSequenceSequenceFromB2DPolyPolygon( const ::basegfx::B2DPolyPolygon& rPolyPoly );

        ::basegfx::B2DPolygon polygonFromPoint2DSequence(
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::geometry::RealPoint2D >& rPoints );

        ::basegfx::B2DPolyPolygon polyPolygonFromPoint2DSequenceSequence(
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Sequence< ::com::sun::star::geometry::RealPoint2D > >& rPoints );

        ::basegfx::B2DPolygon polygonFromBezier2DSequence(
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::geometry::RealBezierSegment2D >& rPoints );

        ::basegfx::B2DPolyPolygon polyPolygonFromBezier2DSequenceSequence(
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Sequence< ::com::sun::star::geometry::RealBezierSegment2D > >& rPoints );

        ::basegfx::B2DPolyPolygon b2DPolyPolygonFromXPolyPolygon2D(
            const ::com::sun::star::uno::Reference<
                     ::com::sun::star::rendering::XPolyPolygon2D >& rPoly );

        // Matrix conversions
        // ===================================================================

        ::com::sun::star::geometry::AffineMatrix2D&
            affineMatrixFromHomMatrix( ::com::sun::star::geometry::AffineMatrix2D&  matrix,
                                       const ::basegfx::B2DHomMatrix&               transform);

        ::com::sun::star::geometry::AffineMatrix3D& affineMatrixFromHomMatrix3D(
            ::com::sun::star::geometry::AffineMatrix3D& matrix,
            const ::basegfx::B3DHomMatrix& transform);

        ::basegfx::B2DHomMatrix&
            homMatrixFromAffineMatrix( ::basegfx::B2DHomMatrix&                             transform,
                                       const ::com::sun::star::geometry::AffineMatrix2D&    matrix );

        ::basegfx::B2DHomMatrix homMatrixFromAffineMatrix( const ::com::sun::star::geometry::AffineMatrix2D& matrix );
        ::basegfx::B3DHomMatrix homMatrixFromAffineMatrix3D( const ::com::sun::star::geometry::AffineMatrix3D& matrix );

        ::com::sun::star::geometry::Matrix2D&
            matrixFromHomMatrix( ::com::sun::star::geometry::Matrix2D& matrix,
                                 const ::basegfx::B2DHomMatrix&        transform);

        ::basegfx::B2DHomMatrix&
            homMatrixFromMatrix( ::basegfx::B2DHomMatrix&                    transform,
                                 const ::com::sun::star::geometry::Matrix2D& matrix );

        // Geometry conversions
        // ===================================================================

        ::com::sun::star::geometry::RealSize2D          size2DFromB2DSize( const ::basegfx::B2DVector& );
        ::com::sun::star::geometry::RealPoint2D         point2DFromB2DPoint( const ::basegfx::B2DPoint& );
        ::com::sun::star::geometry::RealRectangle2D     rectangle2DFromB2DRectangle( const ::basegfx::B2DRange& );
        ::com::sun::star::geometry::RealRectangle3D     rectangle3DFromB3DRectangle( const ::basegfx::B3DRange& );

        ::basegfx::B2DVector    b2DSizeFromRealSize2D( const ::com::sun::star::geometry::RealSize2D& );
        ::basegfx::B2DPoint     b2DPointFromRealPoint2D( const ::com::sun::star::geometry::RealPoint2D& );
        ::basegfx::B2DRange     b2DRectangleFromRealRectangle2D( const ::com::sun::star::geometry::RealRectangle2D& );
        ::basegfx::B3DRange     b3DRectangleFromRealRectangle3D( const ::com::sun::star::geometry::RealRectangle3D& );

        ::com::sun::star::geometry::IntegerSize2D       integerSize2DFromB2ISize( const ::basegfx::B2IVector& );
        ::com::sun::star::geometry::IntegerPoint2D      integerPoint2DFromB2IPoint( const ::basegfx::B2IPoint& );
        ::com::sun::star::geometry::IntegerRectangle2D  integerRectangle2DFromB2IRectangle( const ::basegfx::B2IRange& );

        ::basegfx::B2IVector    b2ISizeFromIntegerSize2D( const ::com::sun::star::geometry::IntegerSize2D& );
        ::basegfx::B2IPoint     b2IPointFromIntegerPoint2D( const ::com::sun::star::geometry::IntegerPoint2D& );
        ::basegfx::B2IRange     b2IRectangleFromIntegerRectangle2D( const ::com::sun::star::geometry::IntegerRectangle2D& );

        ::com::sun::star::awt::Size         awtSizeFromB2ISize( const ::basegfx::B2IVector& );
        ::com::sun::star::awt::Point        awtPointFromB2IPoint( const ::basegfx::B2IPoint& );
        ::com::sun::star::awt::Rectangle    awtRectangleFromB2IRectangle( const ::basegfx::B2IRange& );

        ::basegfx::B2IVector    b2ISizeFromAwtSize( const ::com::sun::star::awt::Size& );
        ::basegfx::B2IPoint     b2IPointFromAwtPoint( const ::com::sun::star::awt::Point& );
        ::basegfx::B2IRange     b2IRectangleFromAwtRectangle( const ::com::sun::star::awt::Rectangle& );

        // Geometry comparisons
        // ===================================================================

        bool RealSize2DAreEqual( const ::com::sun::star::geometry::RealSize2D& rA, const ::com::sun::star::geometry::RealSize2D& rB );
        bool RealPoint2DAreEqual( const ::com::sun::star::geometry::RealPoint2D& rA, const ::com::sun::star::geometry::RealPoint2D& rB );
        bool RealRectangle2DAreEqual( const ::com::sun::star::geometry::RealRectangle2D& rA, const ::com::sun::star::geometry::RealRectangle2D& rB );
        bool RealRectangle3DAreEqual( const ::com::sun::star::geometry::RealRectangle3D& rA, const ::com::sun::star::geometry::RealRectangle3D& rB );
        bool AffineMatrix2DAreEqual( const ::com::sun::star::geometry::AffineMatrix2D& rA, const ::com::sun::star::geometry::AffineMatrix2D& rB );

        bool IntegerSize2DAreEqual( const ::com::sun::star::geometry::IntegerSize2D& rA, const ::com::sun::star::geometry::IntegerSize2D& rB );
        bool IntegerPoint2DAreEqual( const ::com::sun::star::geometry::IntegerPoint2D& rA, const ::com::sun::star::geometry::IntegerPoint2D& rB );
        bool IntegerRectangle2DAreEqual( const ::com::sun::star::geometry::IntegerRectangle2D& rA, const ::com::sun::star::geometry::IntegerRectangle2D& rB );

        bool awtSizeAreEqual( const ::com::sun::star::awt::Size& rA, const ::com::sun::star::awt::Size& rB );
        bool awtPointAreEqual( const ::com::sun::star::awt::Point& rA, const ::com::sun::star::awt::Point& rB );
        bool awtRectangleAreEqual( const ::com::sun::star::awt::Rectangle& rA, const ::com::sun::star::awt::Rectangle& rB );

        /** Return smalltest integer range, which completely contains
            given floating point range.

            @param rRange
            Input range. Values must be within the representable
            bounds of sal_Int32

            @return the closest integer range, which completely
            contains rRange.
         */
        ::basegfx::B2IRange     b2ISurroundingRangeFromB2DRange( const ::basegfx::B2DRange& rRange );

        /** Return smalltest B2DRange with integer values, which
            completely contains given floating point range.

            @param rRange
            Input range.

            @return the closest B2DRange with integer coordinates,
            which completely contains rRange.
         */
        ::basegfx::B2DRange     b2DSurroundingIntegerRangeFromB2DRange( const ::basegfx::B2DRange& rRange );

    }
}

#endif /* _BGFX_TOOLS_CANVASTOOLS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
