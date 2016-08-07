/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#define _BGFX_TOOLS_CANVASTOOLS_HXX

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
    class B2IVector;
    class B2IPoint;
    class B2IRange;
    class B2DPolygon;
    class B2DPolyPolygon;

    namespace unotools
    {
        // Polygon conversions
        // ===================================================================

        BASEGFX_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D >
            xPolyPolygonFromB2DPolygon( const ::com::sun::star::uno::Reference<
                                            ::com::sun::star::rendering::XGraphicDevice >&  xGraphicDevice,
                                        const ::basegfx::B2DPolygon&                        rPoly    );

        BASEGFX_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D >
            xPolyPolygonFromB2DPolyPolygon( const ::com::sun::star::uno::Reference<
                                                 ::com::sun::star::rendering::XGraphicDevice >&     xGraphicDevice,
                                            const ::basegfx::B2DPolyPolygon&                    rPolyPoly    );


        BASEGFX_DLLPUBLIC ::com::sun::star::uno::Sequence<
              ::com::sun::star::uno::Sequence<
                  ::com::sun::star::geometry::RealBezierSegment2D > >
                    bezierSequenceSequenceFromB2DPolyPolygon( const ::basegfx::B2DPolyPolygon& rPolyPoly );

        BASEGFX_DLLPUBLIC ::com::sun::star::uno::Sequence<
              ::com::sun::star::uno::Sequence<
                  ::com::sun::star::geometry::RealPoint2D > >
                    pointSequenceSequenceFromB2DPolyPolygon( const ::basegfx::B2DPolyPolygon& rPolyPoly );

        BASEGFX_DLLPUBLIC ::basegfx::B2DPolygon polygonFromPoint2DSequence(
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::geometry::RealPoint2D >& rPoints );

        BASEGFX_DLLPUBLIC ::basegfx::B2DPolyPolygon polyPolygonFromPoint2DSequenceSequence(
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Sequence< ::com::sun::star::geometry::RealPoint2D > >& rPoints );

        BASEGFX_DLLPUBLIC ::basegfx::B2DPolygon polygonFromBezier2DSequence(
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::geometry::RealBezierSegment2D >& rPoints );

        BASEGFX_DLLPUBLIC ::basegfx::B2DPolyPolygon polyPolygonFromBezier2DSequenceSequence(
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Sequence< ::com::sun::star::geometry::RealBezierSegment2D > >& rPoints );

        BASEGFX_DLLPUBLIC ::basegfx::B2DPolyPolygon b2DPolyPolygonFromXPolyPolygon2D(
            const ::com::sun::star::uno::Reference<
                     ::com::sun::star::rendering::XPolyPolygon2D >& rPoly );

        // Matrix conversions
        // ===================================================================

        BASEGFX_DLLPUBLIC ::com::sun::star::geometry::AffineMatrix2D&
            affineMatrixFromHomMatrix( ::com::sun::star::geometry::AffineMatrix2D&  matrix,
                                       const ::basegfx::B2DHomMatrix&               transform);

        BASEGFX_DLLPUBLIC ::com::sun::star::geometry::AffineMatrix3D& affineMatrixFromHomMatrix3D(
            ::com::sun::star::geometry::AffineMatrix3D& matrix,
            const ::basegfx::B3DHomMatrix& transform);

        BASEGFX_DLLPUBLIC ::basegfx::B2DHomMatrix&
            homMatrixFromAffineMatrix( ::basegfx::B2DHomMatrix&                             transform,
                                       const ::com::sun::star::geometry::AffineMatrix2D&    matrix );

        BASEGFX_DLLPUBLIC ::basegfx::B2DHomMatrix homMatrixFromAffineMatrix( const ::com::sun::star::geometry::AffineMatrix2D& matrix );
        BASEGFX_DLLPUBLIC ::basegfx::B3DHomMatrix homMatrixFromAffineMatrix3D( const ::com::sun::star::geometry::AffineMatrix3D& matrix );

        BASEGFX_DLLPUBLIC ::com::sun::star::geometry::Matrix2D&
            matrixFromHomMatrix( ::com::sun::star::geometry::Matrix2D& matrix,
                                 const ::basegfx::B2DHomMatrix&        transform);

        BASEGFX_DLLPUBLIC ::basegfx::B2DHomMatrix&
            homMatrixFromMatrix( ::basegfx::B2DHomMatrix&                    transform,
                                 const ::com::sun::star::geometry::Matrix2D& matrix );

        // Geometry conversions
        // ===================================================================

        BASEGFX_DLLPUBLIC ::com::sun::star::geometry::RealSize2D        size2DFromB2DSize( const ::basegfx::B2DVector& );
        BASEGFX_DLLPUBLIC ::com::sun::star::geometry::RealPoint2D       point2DFromB2DPoint( const ::basegfx::B2DPoint& );
        BASEGFX_DLLPUBLIC ::com::sun::star::geometry::RealRectangle2D   rectangle2DFromB2DRectangle( const ::basegfx::B2DRange& );
        BASEGFX_DLLPUBLIC ::com::sun::star::geometry::RealRectangle3D   rectangle3DFromB3DRectangle( const ::basegfx::B3DRange& );

        BASEGFX_DLLPUBLIC ::basegfx::B2DVector      b2DSizeFromRealSize2D( const ::com::sun::star::geometry::RealSize2D& );
        BASEGFX_DLLPUBLIC ::basegfx::B2DPoint       b2DPointFromRealPoint2D( const ::com::sun::star::geometry::RealPoint2D& );
        BASEGFX_DLLPUBLIC ::basegfx::B2DRange       b2DRectangleFromRealRectangle2D( const ::com::sun::star::geometry::RealRectangle2D& );
        BASEGFX_DLLPUBLIC ::basegfx::B3DRange       b3DRectangleFromRealRectangle3D( const ::com::sun::star::geometry::RealRectangle3D& );

        BASEGFX_DLLPUBLIC ::com::sun::star::geometry::IntegerSize2D         integerSize2DFromB2ISize( const ::basegfx::B2IVector& );
        BASEGFX_DLLPUBLIC ::com::sun::star::geometry::IntegerPoint2D    integerPoint2DFromB2IPoint( const ::basegfx::B2IPoint& );
        BASEGFX_DLLPUBLIC ::com::sun::star::geometry::IntegerRectangle2D    integerRectangle2DFromB2IRectangle( const ::basegfx::B2IRange& );

        BASEGFX_DLLPUBLIC ::basegfx::B2IVector      b2ISizeFromIntegerSize2D( const ::com::sun::star::geometry::IntegerSize2D& );
        BASEGFX_DLLPUBLIC ::basegfx::B2IPoint       b2IPointFromIntegerPoint2D( const ::com::sun::star::geometry::IntegerPoint2D& );
        BASEGFX_DLLPUBLIC ::basegfx::B2IRange       b2IRectangleFromIntegerRectangle2D( const ::com::sun::star::geometry::IntegerRectangle2D& );

        BASEGFX_DLLPUBLIC ::com::sun::star::awt::Size       awtSizeFromB2ISize( const ::basegfx::B2IVector& );
        BASEGFX_DLLPUBLIC ::com::sun::star::awt::Point          awtPointFromB2IPoint( const ::basegfx::B2IPoint& );
        BASEGFX_DLLPUBLIC ::com::sun::star::awt::Rectangle  awtRectangleFromB2IRectangle( const ::basegfx::B2IRange& );

        BASEGFX_DLLPUBLIC ::basegfx::B2IVector      b2ISizeFromAwtSize( const ::com::sun::star::awt::Size& );
        BASEGFX_DLLPUBLIC ::basegfx::B2IPoint       b2IPointFromAwtPoint( const ::com::sun::star::awt::Point& );
        BASEGFX_DLLPUBLIC ::basegfx::B2IRange       b2IRectangleFromAwtRectangle( const ::com::sun::star::awt::Rectangle& );

        // Geometry comparisons
        // ===================================================================

        BASEGFX_DLLPUBLIC bool RealSize2DAreEqual( const ::com::sun::star::geometry::RealSize2D& rA, const ::com::sun::star::geometry::RealSize2D& rB );
        BASEGFX_DLLPUBLIC bool RealPoint2DAreEqual( const ::com::sun::star::geometry::RealPoint2D& rA, const ::com::sun::star::geometry::RealPoint2D& rB );
        BASEGFX_DLLPUBLIC bool RealRectangle2DAreEqual( const ::com::sun::star::geometry::RealRectangle2D& rA, const ::com::sun::star::geometry::RealRectangle2D& rB );
        BASEGFX_DLLPUBLIC bool RealRectangle3DAreEqual( const ::com::sun::star::geometry::RealRectangle3D& rA, const ::com::sun::star::geometry::RealRectangle3D& rB );
        BASEGFX_DLLPUBLIC bool AffineMatrix2DAreEqual( const ::com::sun::star::geometry::AffineMatrix2D& rA, const ::com::sun::star::geometry::AffineMatrix2D& rB );

        BASEGFX_DLLPUBLIC bool IntegerSize2DAreEqual( const ::com::sun::star::geometry::IntegerSize2D& rA, const ::com::sun::star::geometry::IntegerSize2D& rB );
        BASEGFX_DLLPUBLIC bool IntegerPoint2DAreEqual( const ::com::sun::star::geometry::IntegerPoint2D& rA, const ::com::sun::star::geometry::IntegerPoint2D& rB );
        BASEGFX_DLLPUBLIC bool IntegerRectangle2DAreEqual( const ::com::sun::star::geometry::IntegerRectangle2D& rA, const ::com::sun::star::geometry::IntegerRectangle2D& rB );

        BASEGFX_DLLPUBLIC bool awtSizeAreEqual( const ::com::sun::star::awt::Size& rA, const ::com::sun::star::awt::Size& rB );
        BASEGFX_DLLPUBLIC bool awtPointAreEqual( const ::com::sun::star::awt::Point& rA, const ::com::sun::star::awt::Point& rB );
        BASEGFX_DLLPUBLIC bool awtRectangleAreEqual( const ::com::sun::star::awt::Rectangle& rA, const ::com::sun::star::awt::Rectangle& rB );

        /** Return smalltest integer range, which completely contains
            given floating point range.

            @param rRange
            Input range. Values must be within the representable
            bounds of sal_Int32

            @return the closest integer range, which completely
            contains rRange.
         */
        BASEGFX_DLLPUBLIC ::basegfx::B2IRange   b2ISurroundingRangeFromB2DRange( const ::basegfx::B2DRange& rRange );

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

#endif /* _BGFX_TOOLS_CANVASTOOLS_HXX */
