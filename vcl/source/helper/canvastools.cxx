/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: canvastools.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 13:31:25 $
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

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

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

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif
#ifndef _BGFX_VECTOR_B2DSIZE_HXX
#include <basegfx/vector/b2dsize.hxx>
#endif
#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif
#ifndef _BGFX_RANGE_B2DRECTANGLE_HXX
#include <basegfx/range/b2drectangle.hxx>
#endif
#ifndef _BGFX_VECTOR_B2ISIZE_HXX
#include <basegfx/vector/b2isize.hxx>
#endif
#ifndef _BGFX_POINT_B2IPOINT_HXX
#include <basegfx/point/b2ipoint.hxx>
#endif
#ifndef _BGFX_RANGE_B2IRECTANGLE_HXX
#include <basegfx/range/b2irectangle.hxx>
#endif

#include <tools/poly.hxx>
#include <rtl/uuid.h>

#include <salbtype.hxx>
#include <bmpacc.hxx>
#include <bitmapex.hxx>

#include <canvasbitmap.hxx>
#include <canvastools.hxx>


using namespace ::com::sun::star;

namespace vcl
{
    namespace unotools
    {
        namespace
        {
            uno::Sequence< geometry::RealBezierSegment2D > bezierSequenceFromPolygon( const ::Polygon& inputPolygon )
            {
                const USHORT nSize( inputPolygon.GetSize() );

                // adapt polygon size. As we store bezier end
                // and control point in a combined data structure,
                // every control point reduces the number of sequence
                // elements by one.
                USHORT nCurrSize = nSize;
                USHORT i;
                for( i=0; i<nSize; ++i )
                {
                    if( inputPolygon.GetFlags(i) == POLY_CONTROL )
                        --nCurrSize;
                }

                // make room
                uno::Sequence< geometry::RealBezierSegment2D > outputSequence( nCurrSize );
                geometry::RealBezierSegment2D* pOutput = outputSequence.getArray();

                // fill sequence from polygon
                i=0;
                int nOutPos=0;
                while( i<nSize )
                {
                    DBG_ASSERT( nCurrSize > nOutPos,
                                "bezierSequenceFromPolygon(): output size calculated incorrectly");
                    if( nCurrSize <= nOutPos )
                        return uno::Sequence< geometry::RealBezierSegment2D >(); // bail out

                    if( i+2 < nSize )
                    {
                        // we have at least two more point to process
                        // - control points allowed here

                        if( inputPolygon.GetFlags(i) != POLY_CONTROL &&
                            inputPolygon.GetFlags(i+1) == POLY_CONTROL &&
                            inputPolygon.GetFlags(i+2) == POLY_CONTROL )
                        {
                            // start of a bezier segment
                            pOutput[nOutPos++] = geometry::RealBezierSegment2D( inputPolygon[i].X(),
                                                                                inputPolygon[i].Y(),
                                                                                inputPolygon[i+1].X(),
                                                                                inputPolygon[i+1].Y(),
                                                                                inputPolygon[i+2].X(),
                                                                                inputPolygon[i+2].Y() );
                            i += 3;
                        }
                        else
                        {
                            const double nX( inputPolygon[i].X() );
                            const double nY( inputPolygon[i].Y() );

                            // no bezier segment at all - add
                            // degenerated bezier point (i.e. straight line)
                            pOutput[nOutPos++] = geometry::RealBezierSegment2D( nX, nY,
                                                                                nX, nY,
                                                                                nX, nY );
                            i += 1;
                        }
                    }
                    else if( i+1 < nSize )
                    {
#ifdef DBG_UTIL
                        if( inputPolygon.GetFlags(i) == POLY_CONTROL )
                            DBG_WARNING( "bezierSequenceFromPolygon(): Invalid point sequence in Polygon" );
#endif

                        const double nX( inputPolygon[i].X() );
                        const double nY( inputPolygon[i].Y() );

                        // can't have any bezier segments here
                        // (not enough points left) - add
                        // degenerated bezier point (i.e. straight
                        // line)
                        pOutput[nOutPos++] = geometry::RealBezierSegment2D( nX, nY,
                                                                            nX, nY,
                                                                            nX, nY );
                        i += 1;
                    }
                    else
                    {
                        // this is the last point - no control points
                        // allowed any more
#ifdef DBG_UTIL
                        if( inputPolygon.GetFlags(i) == POLY_CONTROL )
                            DBG_WARNING( "bezierSequenceFromPolygon(): Invalid point sequence in Polygon" );
#endif

                        const double nX( inputPolygon[i].X() );
                        const double nY( inputPolygon[i].Y() );

                        pOutput[nOutPos++] = geometry::RealBezierSegment2D( nX, nY,
                                                                            nX, nY,
                                                                            nX, nY );

                        i += 1;
                    }
                }

                return outputSequence;
            }

            uno::Sequence< geometry::RealPoint2D > pointSequenceFromPolygon( const ::Polygon& inputPolygon )
            {
                int i;

                // fetch preliminary polygon size
                const int nSize = inputPolygon.GetSize();

                // make room
                uno::Sequence< geometry::RealPoint2D > outputSequence( nSize );
                geometry::RealPoint2D* pOutput = outputSequence.getArray();

                // fill sequence from polygon
                for( i=0; i<nSize; ++i )
                {
                    pOutput[i] = geometry::RealPoint2D( inputPolygon[i].X(),
                                                        inputPolygon[i].Y() );
                }

                return outputSequence;
            }
        }

        //---------------------------------------------------------------------------------------

        uno::Reference< rendering::XPolyPolygon2D > xPolyPolygonFromPolygon( const uno::Reference< rendering::XGraphicDevice >&     xGraphicDevice,
                                                                             const ::Polygon&                                       inputPolygon )
        {
            RTL_LOGFILE_CONTEXT( aLog, "::vcl::unotools::xPolyPolygonFromPolygon()" );

            uno::Reference< rendering::XPolyPolygon2D > xRes;

            if( !xGraphicDevice.is() )
                return xRes;


            if( inputPolygon.HasFlags() )
            {
                uno::Sequence< uno::Sequence< geometry::RealBezierSegment2D > > outputSequence( 1 );
                outputSequence[0] = bezierSequenceFromPolygon( inputPolygon );

                xRes.set(
                    xGraphicDevice->createCompatibleBezierPolyPolygon( outputSequence ),
                    uno::UNO_QUERY );
            }
            else
            {
                uno::Sequence< uno::Sequence< geometry::RealPoint2D > > outputSequence( 1 );
                outputSequence[0] = pointSequenceFromPolygon( inputPolygon );

                xRes.set(
                    xGraphicDevice->createCompatibleLinePolyPolygon( outputSequence ),
                    uno::UNO_QUERY );
            }

            // determine closed state for the polygon (VCL polygons,
            // by definition, are closed when first and last point are
            // identical)
            if( xRes.is() &&
                inputPolygon.GetSize() > 2 &&
                inputPolygon[0] == inputPolygon[inputPolygon.GetSize()-1] )
            {
                xRes->setClosed( 0, sal_True );
            }

            return xRes;
        }

        //---------------------------------------------------------------------------------------

        uno::Reference< rendering::XPolyPolygon2D > xPolyPolygonFromPolyPolygon( const uno::Reference< rendering::XGraphicDevice >& xGraphicDevice,
                                                                                 const ::PolyPolygon&                               inputPolyPolygon )
        {
            RTL_LOGFILE_CONTEXT( aLog, "::vcl::unotools::xPolyPolygonFromPolyPolygon()" );

            uno::Reference< rendering::XPolyPolygon2D > xRes;

            if( !xGraphicDevice.is() )
                return xRes;

            const int nNumPolys( inputPolyPolygon.Count() );

            int i;
            bool needBeziers( false );

            for( i=0; i<nNumPolys && !needBeziers; ++i )
            {
                // TODO: maybe perform a deep search here, and only revert
                // to beziers when at least one control point is
                // encountered
                if( inputPolyPolygon[i].HasFlags() )
                    needBeziers = true;
            }

            if( needBeziers )
            {
                uno::Sequence< uno::Sequence< geometry::RealBezierSegment2D > > outputSequence( nNumPolys );
                uno::Sequence< geometry::RealBezierSegment2D >* pOutput = outputSequence.getArray();

                for( i=0; i<nNumPolys; ++i )
                {
                    pOutput[i] = bezierSequenceFromPolygon( inputPolyPolygon[i] );
                }

                xRes.set(
                    xGraphicDevice->createCompatibleBezierPolyPolygon( outputSequence ),
                    uno::UNO_QUERY );
            }
            else
            {
                uno::Sequence< uno::Sequence< geometry::RealPoint2D > > outputSequence( nNumPolys );
                uno::Sequence< geometry::RealPoint2D >* pOutput = outputSequence.getArray();

                for( i=0; i<nNumPolys; ++i )
                {
                    pOutput[i] = pointSequenceFromPolygon( inputPolyPolygon[i] );
                }

                xRes.set(
                    xGraphicDevice->createCompatibleLinePolyPolygon( outputSequence ),
                    uno::UNO_QUERY );
            }

            // determine closed state for each contained polygon (VCL
            // polygons, by definition, are closed when first and last
            // point are identical)
            if( xRes.is() )
            {
                for( i=0; i<nNumPolys; ++i )
                {
                    const ::Polygon& rCurrPoly( inputPolyPolygon[i] );

                    if( rCurrPoly.GetSize() > 2 &&
                        rCurrPoly[0] == rCurrPoly[rCurrPoly.GetSize()-1] )
                    {
                        xRes->setClosed( i, sal_True );
                    }
                }
            }

            return xRes;
        }

        //---------------------------------------------------------------------------------------

        ::Polygon polygonFromPoint2DSequence( const uno::Sequence< geometry::RealPoint2D >& points )
        {
            RTL_LOGFILE_CONTEXT( aLog, "::vcl::unotools::polygonFromPoint2DSequence()" );

            const int nCurrSize( points.getLength() );

            ::Polygon aPoly( nCurrSize );

            int nCurrPoint;
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
            const int nSize( curves.getLength() );

            int i, nCurrSize;

            // determine size
            for( i=0, nCurrSize=0; i<nSize; ++i )
            {
                const geometry::RealBezierSegment2D& rCurrSegment( curves[i] );

                if( rCurrSegment.Px == rCurrSegment.C1x &&
                    rCurrSegment.Px == rCurrSegment.C2x &&
                    rCurrSegment.Py == rCurrSegment.C1y &&
                    rCurrSegment.Py == rCurrSegment.C2y     )
                {
                    nCurrSize += 1;
                }
                else
                {
                    nCurrSize += 3;
                }
            }

            ::Polygon aPoly( nCurrSize );

            int nCurrPoint;
            for( nCurrPoint=0, i=0; nCurrPoint<nCurrSize; ++nCurrPoint )
            {
                const geometry::RealBezierSegment2D& rCurrSegment( curves[i] );

                if( rCurrSegment.Px == rCurrSegment.C1x &&
                    rCurrSegment.Px == rCurrSegment.C2x &&
                    rCurrSegment.Py == rCurrSegment.C1y &&
                    rCurrSegment.Py == rCurrSegment.C2y     )
                {
                    aPoly[i++] = Point( FRound( rCurrSegment.Px ),
                                        FRound( rCurrSegment.Py ) );
                }
                else
                {
                    aPoly[i] = Point( FRound( rCurrSegment.Px ),
                                      FRound( rCurrSegment.Py ) );
                    aPoly.SetFlags(i++, POLY_NORMAL);

                    aPoly[i] = Point( FRound( rCurrSegment.C1x ),
                                      FRound( rCurrSegment.C1y ) );
                    aPoly.SetFlags(i++, POLY_CONTROL);

                    aPoly[i] = Point( FRound( rCurrSegment.C2x ),
                                      FRound( rCurrSegment.C2y ) );
                    aPoly.SetFlags(i++, POLY_CONTROL);
                }
            }

            return aPoly;
        }

        //---------------------------------------------------------------------------------------

        ::PolyPolygon polyPolygonFromBezier2DSequenceSequence( const uno::Sequence< uno::Sequence< geometry::RealBezierSegment2D > >& curves )
        {
            ::PolyPolygon aRes;

            int nCurrPoly;
            for( nCurrPoly=0; nCurrPoly<curves.getLength(); ++nCurrPoly )
            {
                aRes.Insert( polygonFromBezier2DSequence( curves[nCurrPoly] ) );
            }

            return aRes;
        }

        //---------------------------------------------------------------------------------------

        uno::Reference< rendering::XBitmap > xBitmapFromBitmap( const uno::Reference< rendering::XGraphicDevice >&  xGraphicDevice,
                                                                const ::Bitmap&                                     inputBitmap )
        {
            RTL_LOGFILE_CONTEXT( aLog, "::vcl::unotools::xBitmapFromBitmap()" );

            return new vcl::unotools::VclCanvasBitmap( BitmapEx( inputBitmap ) );
        }

        //---------------------------------------------------------------------------------------

        uno::Reference< rendering::XBitmap > xBitmapFromBitmapEx( const uno::Reference< rendering::XGraphicDevice >&    xGraphicDevice,
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
                    return BitmapEx( *(BitmapEx*)nPtr );
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

        uno::Sequence< double > colorToDoubleSequence( const uno::Reference< rendering::XGraphicDevice >&   xGraphicDevice,
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

        uno::Sequence< sal_Int8 > colorToIntSequence( const uno::Reference< rendering::XGraphicDevice >&    xGraphicDevice,
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

        Color sequenceToColor( const uno::Reference< rendering::XGraphicDevice >&   xGraphicDevice,
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

        Color sequenceToColor( const uno::Reference< rendering::XGraphicDevice >&   xGraphicDevice,
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
