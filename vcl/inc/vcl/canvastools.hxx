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

        // Color conversions (vcl/tools Color <-> canvas standard color space)
        // ===================================================================

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

        Size                                            VCL_DLLPUBLIC sizeFromRealSize2D( const ::com::sun::star::geometry::RealSize2D& );
        Point                                           VCL_DLLPUBLIC pointFromRealPoint2D( const ::com::sun::star::geometry::RealPoint2D& );

        // geometry::Integer
        ::com::sun::star::geometry::IntegerSize2D       VCL_DLLPUBLIC integerSize2DFromSize( const Size& );

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
        Point                       VCL_DLLPUBLIC pointFromB2IPoint( const ::basegfx::B2IPoint& );
        Rectangle                   VCL_DLLPUBLIC rectangleFromB2IRectangle( const ::basegfx::B2IRange& );
    }
}

#endif /* _VCL_CANVASTOOLS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
