/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: canvastools.cxx,v $
 * $Revision: 1.14 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"
#include <rtl/logfile.hxx>

#ifndef _COM_SUN_STAR_GEOMETRY_REALSIZE2D_HPP__
#include <com/sun/star/geometry/RealSize2D.hpp>
#endif
#ifndef _COM_SUN_STAR_GEOMETRY_REALPOINT2D_HPP__
#include <com/sun/star/geometry/RealPoint2D.hpp>
#endif
#ifndef _COM_SUN_STAR_GEOMETRY_REALRECTANGLE2D_HPP__
#include <com/sun/star/geometry/RealRectangle2D.hpp>
#endif
#ifndef _COM_SUN_STAR_GEOMETRY_INTEGERSIZE2D_HPP__
#include <com/sun/star/geometry/IntegerSize2D.hpp>
#endif
#ifndef _COM_SUN_STAR_GEOMETRY_INTEGERPOINT2D_HPP__
#include <com/sun/star/geometry/IntegerPoint2D.hpp>
#endif
#ifndef _COM_SUN_STAR_GEOMETRY_INTEGERRECTANGLE2D_HPP__
#include <com/sun/star/geometry/IntegerRectangle2D.hpp>
#endif

#ifndef _COM_SUN_STAR_RENDERING_XGRAPHICDEVICE_HPP__
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XBITMAP_HPP__
#include <com/sun/star/rendering/XBitmap.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XPOLYPOLYGON2D_HPP__
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>
#endif
#ifndef _COM_SUN_STAR_GEOMETRY_REALBEZIERSEGMENT2D_HPP__
#include <com/sun/star/geometry/RealBezierSegment2D.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_INTEGERBITMAPFORMAT_HPP__
#include <com/sun/star/rendering/IntegerBitmapFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_INTEGERBITMAPLAYOUT_HPP__
#include <com/sun/star/rendering/IntegerBitmapLayout.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_ENDIANNESS_HPP__
#include <com/sun/star/rendering/Endianness.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XINTEGERBITMAP_HPP__
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#endif
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/vector/b2isize.hxx>
#include <basegfx/point/b2ipoint.hxx>
#include <basegfx/range/b2irectangle.hxx>

// #i79917#
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

#include <tools/poly.hxx>
#include <rtl/uuid.h>

#include <vcl/salbtype.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/bitmapex.hxx>

#include <vcl/canvasbitmap.hxx>
#include <vcl/canvastools.hxx>
#include <hash_map>


using namespace ::com::sun::star;

namespace vcl
{
    namespace unotools
    {
        // #i79917# removed helpers bezierSequenceFromPolygon and
        // pointSequenceFromPolygon here
        // Also all helpers using tools Polygon and PolyPolygon will get mapped to the
        // B2DPolygon helpers for these cases, see comments with the same TaskID below.
        // TODO: Remove those wrapped methods

        //---------------------------------------------------------------------------------------

        uno::Reference< rendering::XPolyPolygon2D > xPolyPolygonFromPolygon( const uno::Reference< rendering::XGraphicDevice >&     xGraphicDevice,
                                                                             const ::Polygon&                                       inputPolygon )
        {
            RTL_LOGFILE_CONTEXT( aLog, "::vcl::unotools::xPolyPolygonFromPolygon()" );

            // #i79917# map to basegfx
            const basegfx::B2DPolygon aB2DPolygon(inputPolygon.getB2DPolygon());
            return basegfx::unotools::xPolyPolygonFromB2DPolygon(xGraphicDevice, aB2DPolygon);
        }

        //---------------------------------------------------------------------------------------

        uno::Reference< rendering::XPolyPolygon2D > xPolyPolygonFromPolyPolygon( const uno::Reference< rendering::XGraphicDevice >& xGraphicDevice,
                                                                                 const ::PolyPolygon&                               inputPolyPolygon )
        {
            RTL_LOGFILE_CONTEXT( aLog, "::vcl::unotools::xPolyPolygonFromPolyPolygon()" );

            // #i79917# map to basegfx
            const basegfx::B2DPolyPolygon aB2DPolyPolygon(inputPolyPolygon.getB2DPolyPolygon());
            return basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(xGraphicDevice, aB2DPolyPolygon);
        }

        //---------------------------------------------------------------------------------------

        ::Polygon polygonFromPoint2DSequence( const uno::Sequence< geometry::RealPoint2D >& points )
        {
            RTL_LOGFILE_CONTEXT( aLog, "::vcl::unotools::polygonFromPoint2DSequence()" );

            const USHORT nCurrSize( sal::static_int_cast<USHORT>(points.getLength()) );

            ::Polygon aPoly( nCurrSize );

            USHORT nCurrPoint;
            for( nCurrPoint=0; nCurrPoint<nCurrSize; ++nCurrPoint )
                aPoly[nCurrPoint] = pointFromRealPoint2D( points[nCurrPoint] );

            return aPoly;
        }

        //---------------------------------------------------------------------------------------

        ::PolyPolygon polyPolygonFromPoint2DSequenceSequence( const uno::Sequence< uno::Sequence< geometry::RealPoint2D > >& points )
        {
            RTL_LOGFILE_CONTEXT( aLog, "::vcl::unotools::polyPolygonFromPoint2DSequenceSequence()" );

            ::PolyPolygon aRes;

            int nCurrPoly;
            for( nCurrPoly=0; nCurrPoly<points.getLength(); ++nCurrPoly )
            {
                aRes.Insert( polygonFromPoint2DSequence( points[nCurrPoly] ) );
            }

            return aRes;
        }

        //---------------------------------------------------------------------------------------

        ::Polygon polygonFromBezier2DSequence( const uno::Sequence< geometry::RealBezierSegment2D >& curves )
        {
            // #i79917# map to basegfx
            const basegfx::B2DPolygon aB2DPolygon(basegfx::unotools::polygonFromBezier2DSequence(curves));
            return ::Polygon(aB2DPolygon);
        }

        //---------------------------------------------------------------------------------------

        ::PolyPolygon polyPolygonFromBezier2DSequenceSequence( const uno::Sequence< uno::Sequence< geometry::RealBezierSegment2D > >& curves )
        {
            // #i79917# map to basegfx
            const basegfx::B2DPolyPolygon aB2DPolyPolygon(basegfx::unotools::polyPolygonFromBezier2DSequenceSequence(curves));
            return ::PolyPolygon(aB2DPolyPolygon);
        }

        //---------------------------------------------------------------------------------------

        uno::Reference< rendering::XBitmap > xBitmapFromBitmap( const uno::Reference< rendering::XGraphicDevice >&  /*xGraphicDevice*/,
                                                                const ::Bitmap&                                     inputBitmap )
        {
            RTL_LOGFILE_CONTEXT( aLog, "::vcl::unotools::xBitmapFromBitmap()" );

            return new vcl::unotools::VclCanvasBitmap( BitmapEx( inputBitmap ) );
        }

        //---------------------------------------------------------------------------------------

        uno::Reference< rendering::XBitmap > xBitmapFromBitmapEx( const uno::Reference< rendering::XGraphicDevice >&    /*xGraphicDevice*/,
                                                                  const ::BitmapEx&                                     inputBitmap )
        {
            RTL_LOGFILE_CONTEXT( aLog, "::vcl::unotools::xBitmapFromBitmapEx()" );

            return new vcl::unotools::VclCanvasBitmap( inputBitmap );
        }

        //---------------------------------------------------------------------------------------

        const uno::Sequence< sal_Int8 > getTunnelIdentifier( TunnelIdentifierType eType )
        {
            static std::hash_map< int, uno::Sequence< sal_Int8 > > aIds;
            std::hash_map< int, uno::Sequence< sal_Int8 > >::iterator it =
                aIds.find( eType );
            if( it == aIds.end() )
            {
                uno::Sequence< sal_Int8 > aNewId( 16 );
                rtl_createUuid( (sal_uInt8*)aNewId.getArray(), NULL, sal_True );
                aIds[ eType ] = aNewId;
                it = aIds.find( eType );
            }
            return it->second;
        }

        //---------------------------------------------------------------------------------------

        ::BitmapEx bitmapExFromXBitmap( const uno::Reference< rendering::XGraphicDevice >&  xGraphicDevice,
                                        const uno::Reference< rendering::XIntegerBitmap >&  xInputBitmap )
        {
            RTL_LOGFILE_CONTEXT( aLog, "::vcl::unotools::bitmapExFromXBitmap()" );

            if( !xGraphicDevice.is() || !xInputBitmap.is() )
                return ::BitmapEx();

            uno::Reference< lang::XUnoTunnel > xTunnel( xInputBitmap, uno::UNO_QUERY );
            if( xTunnel.is() )
            {
                const uno::Sequence< sal_Int8 >& rTest = getTunnelIdentifier( Id_BitmapEx );
                sal_Int64 nPtr = xTunnel->getSomething( rTest );
                if( nPtr != 0 )
                {
                    return BitmapEx( *(BitmapEx*)sal_IntPtr(nPtr) );
                }
            }

            rendering::IntegerBitmapLayout aFormat;
            const Size aPixelSize( sizeFromIntegerSize2D( xInputBitmap->getSize() ) );
            const uno::Sequence< sal_Int8 > data( xInputBitmap->getData(
                                                      aFormat,
                                                      geometry::IntegerRectangle2D(0,0,aPixelSize.Width(),aPixelSize.Height()) ) );

            ::Bitmap aBitmap( aPixelSize, 24 ); // create 24bpp Bitmap
            ::Bitmap aAlpha( aPixelSize, 8 ); // create 8bpp alpha Bitmap

            {
                ScopedBitmapWriteAccess pWriteAccess( aBitmap.AcquireWriteAccess(),
                                                      aBitmap );
                ScopedBitmapWriteAccess pAlphaWriteAccess( aAlpha.AcquireWriteAccess(),
                                                           aAlpha );

                if( pWriteAccess.get() != NULL &&
                    pAlphaWriteAccess.get() != NULL )
                {
                    // for the time being, always read as BGRA
                    int nCurrPos(0);
                    for( int y=0; y<aPixelSize.Height(); ++y )
                    {
                        for( int x=0; x<aPixelSize.Width(); ++x )
                        {
                            pWriteAccess->SetPixel( y, x, BitmapColor( data[ nCurrPos+2 ],
                                                                       data[ nCurrPos+1 ],
                                                                       data[ nCurrPos ] ) );
                            nCurrPos += 3;

                            pAlphaWriteAccess->SetPixel( y, x, BitmapColor( 255 - data[ nCurrPos++ ] ) );
                        }
                    }
                }
            }

            return ::BitmapEx( aBitmap,
                               AlphaMask( aAlpha ) );
        }

        //---------------------------------------------------------------------------------------

        uno::Sequence< double > colorToDoubleSequence( const uno::Reference< rendering::XGraphicDevice >&   /*xGraphicDevice*/,
                                                       const Color&                                         rColor       )
        {
            // TODO: handle color space conversions, when defined on canvas/graphicDevice
            uno::Sequence< double > aRet(4);
            double* pRet = aRet.getArray();

            pRet[0] = rColor.GetRed() / 255.0;
            pRet[1] = rColor.GetGreen() / 255.0;
            pRet[2] = rColor.GetBlue() / 255.0;

            // out notion of alpha is different from the rest of the world's
            pRet[3] = 1.0 - rColor.GetTransparency() / 255.0;

            return aRet;
        }

        uno::Sequence< sal_Int8 > colorToIntSequence( const uno::Reference< rendering::XGraphicDevice >&    /*xGraphicDevice*/,
                                                      const Color&                                          rColor       )
        {
            // TODO: handle color space conversions, when defined on canvas/graphicDevice
            uno::Sequence< sal_Int8 > aRet(4);
            sal_Int8* pRet = aRet.getArray();

            pRet[0] = rColor.GetRed();
            pRet[1] = rColor.GetGreen();
            pRet[2] = rColor.GetBlue();

            // out notion of alpha is different from the rest of the world's
            pRet[3] = 255 - rColor.GetTransparency();

            return aRet;
        }

        Color sequenceToColor( const uno::Reference< rendering::XGraphicDevice >&   /*xGraphicDevice*/,
                               const uno::Sequence< sal_Int8 >&                     rColor       )
        {
            OSL_ENSURE( rColor.getLength() > 2, "sequenceToColor: need at least three channels" );

            // TODO: handle color space conversions, when defined on canvas/graphicDevice
            Color aColor;

            aColor.SetRed  ( rColor[0] );
            aColor.SetGreen( rColor[1] );
            aColor.SetBlue ( rColor[2] );

            if( rColor.getLength() > 3 )
            {
                // out notion of alpha is different from the rest of the world's
                aColor.SetTransparency( 255 - rColor[3] );
            }

            return aColor;
        }

        Color sequenceToColor( const uno::Reference< rendering::XGraphicDevice >&   /*xGraphicDevice*/,
                               const uno::Sequence< double >&                       rColor       )
        {
            OSL_ENSURE( rColor.getLength() > 2, "sequenceToColor: need at least three channels" );

            // TODO: handle color space conversions, when defined on canvas/graphicDevice
            Color aColor;

            aColor.SetRed  ( static_cast<UINT8>( 255*rColor[0] + .5 ) );
            aColor.SetGreen( static_cast<UINT8>( 255*rColor[1] + .5 ) );
            aColor.SetBlue ( static_cast<UINT8>( 255*rColor[2] + .5 ) );

            if( rColor.getLength() > 3 )
            {
                // out notion of alpha is different from the rest of the world's
                aColor.SetTransparency( static_cast<UINT8>( 255 - 255*rColor[3] + .5 ) );
            }

            return aColor;
        }

        //---------------------------------------------------------------------------------------

        geometry::RealSize2D size2DFromSize( const Size& rSize )
        {
            return geometry::RealSize2D( rSize.Width(),
                                         rSize.Height() );
        }

        geometry::RealPoint2D point2DFromPoint( const Point& rPoint )
        {
            return geometry::RealPoint2D( rPoint.X(),
                                          rPoint.Y() );
        }

        geometry::RealRectangle2D rectangle2DFromRectangle( const Rectangle& rRect )
        {
            return geometry::RealRectangle2D( rRect.Left(), rRect.Top(),
                                              rRect.Right(), rRect.Bottom() );
        }

        Size sizeFromRealSize2D( const geometry::RealSize2D& rSize )
        {
            return Size( static_cast<long>(rSize.Width + .5),
                         static_cast<long>(rSize.Height + .5) );
        }

        Point pointFromRealPoint2D( const geometry::RealPoint2D& rPoint )
        {
            return Point( static_cast<long>(rPoint.X + .5),
                          static_cast<long>(rPoint.Y + .5) );
        }

        Rectangle rectangleFromRealRectangle2D( const geometry::RealRectangle2D& rRect )
        {
            return Rectangle( static_cast<long>(rRect.X1 + .5),
                              static_cast<long>(rRect.Y1 + .5),
                              static_cast<long>(rRect.X2 + .5),
                              static_cast<long>(rRect.Y2 + .5) );
        }

        ::Size sizeFromB2DSize( const ::basegfx::B2DVector& rVec )
        {
            return ::Size( FRound( rVec.getX() ),
                           FRound( rVec.getY() ) );
        }

        ::Point pointFromB2DPoint( const ::basegfx::B2DPoint& rPoint )
        {
            return ::Point( FRound( rPoint.getX() ),
                            FRound( rPoint.getY() ) );
        }

        ::Rectangle rectangleFromB2DRectangle( const ::basegfx::B2DRange& rRect )
        {
            return ::Rectangle( FRound( rRect.getMinX() ),
                                FRound( rRect.getMinY() ),
                                FRound( rRect.getMaxX() ),
                                FRound( rRect.getMaxY() ) );
        }

        Size sizeFromB2ISize( const ::basegfx::B2IVector& rVec )
        {
            return ::Size( rVec.getX(),
                           rVec.getY() );
        }

        Point pointFromB2IPoint( const ::basegfx::B2IPoint& rPoint )
        {
            return ::Point( rPoint.getX(),
                            rPoint.getY() );
        }

        Rectangle rectangleFromB2IRectangle( const ::basegfx::B2IRange& rRect )
        {
            return ::Rectangle( rRect.getMinX(),
                                rRect.getMinY(),
                                rRect.getMaxX(),
                                rRect.getMaxY() );
        }

        ::basegfx::B2DVector b2DSizeFromSize( const ::Size& rSize )
        {
            return ::basegfx::B2DVector( rSize.Width(),
                                         rSize.Height() );
        }

        ::basegfx::B2DPoint b2DPointFromPoint( const ::Point& rPoint )
        {
            return ::basegfx::B2DPoint( rPoint.X(),
                                        rPoint.Y() );
        }

        ::basegfx::B2DRange b2DRectangleFromRectangle( const ::Rectangle& rRect )
        {
            return ::basegfx::B2DRange( rRect.Left(),
                                        rRect.Top(),
                                        rRect.Right(),
                                        rRect.Bottom() );
        }

        basegfx::B2IVector b2ISizeFromSize( const Size& rSize )
        {
            return ::basegfx::B2IVector( rSize.Width(),
                                         rSize.Height() );
        }

        basegfx::B2IPoint b2IPointFromPoint( const Point& rPoint )
        {
            return ::basegfx::B2IPoint( rPoint.X(),
                                        rPoint.Y() );
        }

        basegfx::B2IRange b2IRectangleFromRectangle( const Rectangle& rRect )
        {
            return ::basegfx::B2IRange( rRect.Left(),
                                        rRect.Top(),
                                        rRect.Right(),
                                        rRect.Bottom() );
        }

        geometry::IntegerSize2D integerSize2DFromSize( const Size& rSize )
        {
            return geometry::IntegerSize2D( rSize.Width(),
                                            rSize.Height() );
        }

        geometry::IntegerPoint2D integerPoint2DFromPoint( const Point& rPoint )
        {
            return geometry::IntegerPoint2D( rPoint.X(),
                                             rPoint.Y() );
        }

        geometry::IntegerRectangle2D integerRectangle2DFromRectangle( const Rectangle& rRectangle )
        {
            return geometry::IntegerRectangle2D( rRectangle.Left(), rRectangle.Top(),
                                                 rRectangle.Right(), rRectangle.Bottom() );
        }

        Size sizeFromIntegerSize2D( const geometry::IntegerSize2D& rSize )
        {
            return Size( rSize.Width,
                         rSize.Height );
        }

        Point pointFromIntegerPoint2D( const geometry::IntegerPoint2D& rPoint )
        {
            return Point( rPoint.X,
                          rPoint.Y );
        }

        Rectangle rectangleFromIntegerRectangle2D( const geometry::IntegerRectangle2D& rRectangle )
        {
            return Rectangle( rRectangle.X1, rRectangle.Y1,
                              rRectangle.X2, rRectangle.Y2 );
        }

    } // namespace vcltools

} // namespace canvas

// eof
