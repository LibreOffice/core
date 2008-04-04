/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: canvastools.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 16:02:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#define _BGFX_TOOLS_CANVASTOOLS_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif


namespace com { namespace sun { namespace star { namespace geometry
{
    struct AffineMatrix2D;
    struct Matrix2D;
    struct RealPoint2D;
    struct RealSize2D;
    struct RealRectangle2D;
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
    class B2DVector;
    class B2DPoint;
    class B2DRange;
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

        ::basegfx::B2DHomMatrix&
            homMatrixFromAffineMatrix( ::basegfx::B2DHomMatrix&                             transform,
                                       const ::com::sun::star::geometry::AffineMatrix2D&    matrix );

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

        ::basegfx::B2DVector    b2DSizeFromRealSize2D( const ::com::sun::star::geometry::RealSize2D& );
        ::basegfx::B2DPoint     b2DPointFromRealPoint2D( const ::com::sun::star::geometry::RealPoint2D& );
        ::basegfx::B2DRange     b2DRectangleFromRealRectangle2D( const ::com::sun::star::geometry::RealRectangle2D& );

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
