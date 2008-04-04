/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: canvastools.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 16:17:23 $
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

#ifndef _VCL_CANVASTOOLS_HXX
#define _VCL_CANVASTOOLS_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

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
                                                            ::com::sun::star::rendering::XGraphicDevice >&  xGraphicDevice,
                                                      const ::com::sun::star::uno::Reference<
                                                            ::com::sun::star::rendering::XIntegerBitmap >&  xInputBitmap );

        /** get a unique identifier for identification in XUnoTunnel interface
         */
         enum TunnelIdentifierType { Id_BitmapEx = 0 };
         const com::sun::star::uno::Sequence< sal_Int8 > VCL_DLLPUBLIC getTunnelIdentifier( TunnelIdentifierType eType );

        // Color conversions (vcl/tools Color <-> x)
        // ===================================================================

        /** Create a device-specific color sequence from VCL/Tools color
         */
        ::com::sun::star::uno::Sequence< double >
            VCL_DLLPUBLIC colorToDoubleSequence( const ::com::sun::star::uno::Reference<
                                   ::com::sun::star::rendering::XGraphicDevice >&   xGraphicDevice,
                                   const Color&                                     rColor       );

        /** Create a device-specific color sequence from VCL/Tools color
         */
        ::com::sun::star::uno::Sequence< sal_Int8 >
            VCL_DLLPUBLIC colorToIntSequence( const ::com::sun::star::uno::Reference<
                                 ::com::sun::star::rendering::XGraphicDevice >&     xGraphicDevice,
                                const Color&                                    rColor       );

        /** Convert from XGraphicDevice color space to VCL/Tools Color.

            If the XGraphicDevice interface reference is invalid, no
            color space transformation is performed, but the values
            are taken as BGRA (for four elements) or BGR tuples (for
            three elements)
         */
        Color VCL_DLLPUBLIC sequenceToColor( const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice >&,
                                             const ::com::sun::star::uno::Sequence< double >&                                );

        /** Convert from XGraphicDevice color space to VCL/Tools Color.

            If the XGraphicDevice interface reference is invalid, no
            color space transformation is performed, but the values
            are taken as BGRA (for four elements) or BGR tuples (for
            three elements)
         */
        Color VCL_DLLPUBLIC sequenceToColor( const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice >&,
                                             const ::com::sun::star::uno::Sequence< sal_Int8 >&                                     );

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
