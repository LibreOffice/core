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



#ifndef _VCL_CANVASTOOLS_HXX
#define _VCL_CANVASTOOLS_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/rendering/XColorSpace.hpp>
#include <basegfx/numeric/ftools.hxx>

#include <vcl/dllapi.h>

class Point;
class Size;
class Rectangle;
class Polygon;
class PolyPolygon;
class Bitmap;
class BitmapEx;
class Color;

namespace basegfx
{
    class B2DVector;
    class B2DPoint;
    class B2DRange;
    class B2IVector;
    class B2IPoint;
    class B2IRange;
    class B2DPolygon;
    class B2DPolyPolygon;
}

namespace com { namespace sun { namespace star { namespace geometry
{
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
    class  XBitmap;
    class  XIntegerBitmap;
    class  XIntegerReadOnlyBitmap;
    class  XPolyPolygon2D;
} } } }

namespace vcl
{
    namespace unotools
    {
        // Polygon conversions
        // ===================================================================

        /** Create an XPolyPolygon from VCL/Tools polygon
         */
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D >
            VCL_DLLPUBLIC xPolyPolygonFromPolygon( const ::com::sun::star::uno::Reference<
                                                         ::com::sun::star::rendering::XGraphicDevice >&     xGraphicDevice,
                                                   const ::Polygon&                                         inputPolygon );

        /** Create an XPolyPolygon from VCL/Tools polyPolygon
         */
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D >
            VCL_DLLPUBLIC xPolyPolygonFromPolyPolygon( const ::com::sun::star::uno::Reference<
                                                             ::com::sun::star::rendering::XGraphicDevice >&     xGraphicDevice,
                                                       const ::PolyPolygon&                                     inputPolyPolygon );

        /** Create an VCL/Tools polygon from a point sequence
         */
        ::Polygon VCL_DLLPUBLIC polygonFromPoint2DSequence(
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::geometry::RealPoint2D >& rPoints );

        /** Create an VCL/Tools polyPolygon from a point sequence sequence
         */
        ::PolyPolygon VCL_DLLPUBLIC polyPolygonFromPoint2DSequenceSequence(
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Sequence< ::com::sun::star::geometry::RealPoint2D > >& rPoints );

        /** Create an VCL/Tools polygon from a bezier segment sequence
         */
        ::Polygon VCL_DLLPUBLIC polygonFromBezier2DSequence(
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::geometry::RealBezierSegment2D >& rPoints );

        /** Create an VCL/Tools polyPolygon from a bezier segment sequence sequence
         */
        ::PolyPolygon VCL_DLLPUBLIC polyPolygonFromBezier2DSequenceSequence(
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Sequence< ::com::sun::star::geometry::RealBezierSegment2D > >& rPoints );

        // Bitmap conversions
        // ===================================================================

        /** Create an XBitmap from VCL Bitmap
         */
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmap >
            VCL_DLLPUBLIC xBitmapFromBitmap( const ::com::sun::star::uno::Reference<
                                                   ::com::sun::star::rendering::XGraphicDevice >&   xGraphicDevice,
                                             const ::Bitmap&                                        inputBitmap );

        /** Create an XBitmap from VCL BitmapEx
         */
        ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmap >
            VCL_DLLPUBLIC xBitmapFromBitmapEx( const ::com::sun::star::uno::Reference<
                                                     ::com::sun::star::rendering::XGraphicDevice >&     xGraphicDevice,
                                               const ::BitmapEx&                                        inputBitmap );

        /** Create a BitmapEx from an XBitmap
         */
        ::BitmapEx VCL_DLLPUBLIC bitmapExFromXBitmap( const ::com::sun::star::uno::Reference<
                                                            ::com::sun::star::rendering::XIntegerReadOnlyBitmap >& xInputBitmap );

        /** get a unique identifier for identification in XUnoTunnel interface
         */
         enum TunnelIdentifierType { Id_BitmapEx = 0 };
         const com::sun::star::uno::Sequence< sal_Int8 > VCL_DLLPUBLIC getTunnelIdentifier( TunnelIdentifierType eType );

        // Color conversions (vcl/tools Color <-> canvas standard color space)
        // ===================================================================

        /** Create a device-specific color sequence from VCL/Tools color

            Note that this method assumes a color space equivalent to
            the one returned from createStandardColorSpace()
         */
        ::com::sun::star::uno::Sequence< double >
            VCL_DLLPUBLIC colorToStdColorSpaceSequence( const Color& rColor );

        /** Convert color to device color sequence

            @param rColor
            Color to convert

            @param xColorSpace
            Color space to convert into
         */
        ::com::sun::star::uno::Sequence< double >
        VCL_DLLPUBLIC colorToDoubleSequence( const Color&                                      rColor,
                                             const ::com::sun::star::uno::Reference<
                                                   ::com::sun::star::rendering::XColorSpace >& xColorSpace );

        /** Convert from standard device color space to VCL/Tools color

            Note that this method assumes a color space equivalent to
            the one returned from createStandardColorSpace()
         */
        Color VCL_DLLPUBLIC stdColorSpaceSequenceToColor(
            const ::com::sun::star::uno::Sequence< double >& rColor );

        /** Convert color to device color sequence

            @param rColor
            Color sequence to convert from

            @param xColorSpace
            Color space to convert from
         */
        Color VCL_DLLPUBLIC doubleSequenceToColor( const ::com::sun::star::uno::Sequence< double >   rColor,
                                                   const ::com::sun::star::uno::Reference<
                                                         ::com::sun::star::rendering::XColorSpace >& xColorSpace );

        /// Convert [0,1] double value to [0,255] int
        inline sal_Int8 toByteColor( double val )
        {
            return sal::static_int_cast<sal_Int8>(
                basegfx::fround(val*255.0));
        }

        /// Convert [0,255] int value to [0,1] double value
        inline double toDoubleColor( sal_uInt8 val )
        {
            return val / 255.0;
        }

        /// Create a standard color space suitable for VCL RGB color
        ::com::sun::star::uno::Reference<
            ::com::sun::star::rendering::XColorSpace> VCL_DLLPUBLIC createStandardColorSpace();

        // Geometry conversions (vcl/tools <-> x)
        // ===================================================================

        // geometry::Real
        ::com::sun::star::geometry::RealSize2D          VCL_DLLPUBLIC size2DFromSize( const Size& );
        ::com::sun::star::geometry::RealPoint2D         VCL_DLLPUBLIC point2DFromPoint( const Point& );
        ::com::sun::star::geometry::RealRectangle2D     VCL_DLLPUBLIC rectangle2DFromRectangle( const Rectangle& );

        Size                                            VCL_DLLPUBLIC sizeFromRealSize2D( const ::com::sun::star::geometry::RealSize2D& );
        Point                                           VCL_DLLPUBLIC pointFromRealPoint2D( const ::com::sun::star::geometry::RealPoint2D& );
        Rectangle                                       VCL_DLLPUBLIC rectangleFromRealRectangle2D( const ::com::sun::star::geometry::RealRectangle2D& );

        // geometry::Integer
        ::com::sun::star::geometry::IntegerSize2D       VCL_DLLPUBLIC integerSize2DFromSize( const Size& );
        ::com::sun::star::geometry::IntegerPoint2D      VCL_DLLPUBLIC integerPoint2DFromPoint( const Point& );
        ::com::sun::star::geometry::IntegerRectangle2D  VCL_DLLPUBLIC integerRectangle2DFromRectangle( const Rectangle& );

        Size                                            VCL_DLLPUBLIC sizeFromIntegerSize2D( const ::com::sun::star::geometry::IntegerSize2D& );
        Point                                           VCL_DLLPUBLIC pointFromIntegerPoint2D( const ::com::sun::star::geometry::IntegerPoint2D& );
        Rectangle                                       VCL_DLLPUBLIC rectangleFromIntegerRectangle2D( const ::com::sun::star::geometry::IntegerRectangle2D& );

        // basegfx::B2D
        Size                        VCL_DLLPUBLIC sizeFromB2DSize( const ::basegfx::B2DVector& );
        Point                       VCL_DLLPUBLIC pointFromB2DPoint( const ::basegfx::B2DPoint& );
        Rectangle                   VCL_DLLPUBLIC rectangleFromB2DRectangle( const ::basegfx::B2DRange& );

        basegfx::B2DVector          VCL_DLLPUBLIC b2DSizeFromSize( const Size& );
        basegfx::B2DPoint           VCL_DLLPUBLIC b2DPointFromPoint( const Point& );
        basegfx::B2DRange           VCL_DLLPUBLIC b2DRectangleFromRectangle( const Rectangle& );

        // basegfx::B2I
        Size                        VCL_DLLPUBLIC sizeFromB2ISize( const ::basegfx::B2IVector& );
        Point                       VCL_DLLPUBLIC pointFromB2IPoint( const ::basegfx::B2IPoint& );
        Rectangle                   VCL_DLLPUBLIC rectangleFromB2IRectangle( const ::basegfx::B2IRange& );

        basegfx::B2IVector          VCL_DLLPUBLIC b2ISizeFromSize( const Size& );
        basegfx::B2IPoint           VCL_DLLPUBLIC b2IPointFromPoint( const Point& );
        basegfx::B2IRange           VCL_DLLPUBLIC b2IRectangleFromRectangle( const Rectangle& );
    }
}

#endif /* _VCL_CANVASTOOLS_HXX */
