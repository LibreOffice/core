/*************************************************************************
 *
 *  $RCSfile: canvastools.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 20:43:34 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_GEOMETRY_REALSIZE2D_HPP__
#include <drafts/com/sun/star/geometry/RealSize2D.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_GEOMETRY_REALPOINT2D_HPP__
#include <drafts/com/sun/star/geometry/RealPoint2D.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_GEOMETRY_REALRECTANGLE2D_HPP__
#include <drafts/com/sun/star/geometry/RealRectangle2D.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_GEOMETRY_INTEGERSIZE2D_HPP__
#include <drafts/com/sun/star/geometry/IntegerSize2D.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_GEOMETRY_INTEGERPOINT2D_HPP__
#include <drafts/com/sun/star/geometry/IntegerPoint2D.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_GEOMETRY_INTEGERRECTANGLE2D_HPP__
#include <drafts/com/sun/star/geometry/IntegerRectangle2D.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_XGRAPHICDEVICE_HPP__
#include <drafts/com/sun/star/rendering/XGraphicDevice.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_XBITMAP_HPP__
#include <drafts/com/sun/star/rendering/XBitmap.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_XPOLYPOLYGON2D_HPP__
#include <drafts/com/sun/star/rendering/XPolyPolygon2D.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_GEOMETRY_REALBEZIERSEGMENT2D_HPP__
#include <drafts/com/sun/star/geometry/RealBezierSegment2D.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_INTEGERBITMAPFORMAT_HPP__
#include <drafts/com/sun/star/rendering/IntegerBitmapFormat.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_ENDIANNESS_HPP__
#include <drafts/com/sun/star/rendering/Endianness.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_XINTEGERBITMAP_HPP__
#include <drafts/com/sun/star/rendering/XIntegerBitmap.hpp>
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

#include <salbtype.hxx>
#include <bmpacc.hxx>
#include <bitmapex.hxx>

#include <canvastools.hxx>


using namespace ::drafts::com::sun::star;
using namespace ::com::sun::star;

namespace vcl
{
    namespace unotools
    {
        namespace
        {
            uno::Sequence< geometry::RealBezierSegment2D > bezierSequenceFromPolygon( const ::Polygon& inputPolygon )
            {
                int i;

                // fetch preliminary polygon size
                int nCurrSize = inputPolygon.GetSize();

                // adapt polygon size. As we store bezier end
                // and control point in a combined data structure,
                // every control point reduces the number of sequence
                // elements by one.
                for( i=0; i<nCurrSize; ++i )
                {
                    if( inputPolygon.GetFlags(i) == POLY_CONTROL )
                        --nCurrSize;
                }

                // make room
                uno::Sequence< geometry::RealBezierSegment2D > outputSequence( nCurrSize );

                // fill sequence from polygon
                i=0;
                int nOutPos=0;
                while( i<nCurrSize )
                {
                    DBG_ASSERT( nCurrSize > nOutPos,
                                "bezierSequenceFromPolygon(): output size calculated incorrectly");

                    if( i+2 < nCurrSize )
                    {
                        // we have at least two more point to process
                        // - control points allowed here

                        if( inputPolygon.GetFlags(i) != POLY_CONTROL &&
                            inputPolygon.GetFlags(i+1) == POLY_CONTROL &&
                            inputPolygon.GetFlags(i+2) == POLY_CONTROL )
                        {
                            // start of a bezier segment
                            outputSequence[nOutPos++] = geometry::RealBezierSegment2D( inputPolygon[i].X(),
                                                                                       inputPolygon[i].Y(),
                                                                                       inputPolygon[i+1].X(),
                                                                                       inputPolygon[i+1].Y(),
                                                                                       inputPolygon[i+2].X(),
                                                                                       inputPolygon[i+2].Y() );
                            i += 3;
                        }
                        else
                        {
                            // no bezier segment at all - add
                            // degenerated bezier point (i.e. straight line)
                            outputSequence[nOutPos++] = geometry::RealBezierSegment2D( inputPolygon[i].X(),
                                                                                       inputPolygon[i].Y(),
                                                                                       inputPolygon[i].X(),
                                                                                       inputPolygon[i].Y(),
                                                                                       inputPolygon[i].X(),
                                                                                       inputPolygon[i].Y() );
                            i += 1;
                        }
                    }
                    else if( i+1 < nCurrSize )
                    {
                        DBG_ASSERT( inputPolygon.GetFlags(i) != POLY_CONTROL,
                                    "bezierSequenceFromPolygon(): Invalid point sequence in Polygon");

                        // can't have any bezier segments here
                        // (not enough points left) - add
                        // degenerated bezier point (i.e. straight
                        // line)
                        outputSequence[nOutPos++] = geometry::RealBezierSegment2D( inputPolygon[i].X(),
                                                                                   inputPolygon[i].Y(),
                                                                                   inputPolygon[i].X(),
                                                                                   inputPolygon[i].Y(),
                                                                                   inputPolygon[i].X(),
                                                                                   inputPolygon[i].Y() );
                        i += 1;
                    }
                    else
                    {
                        // this is the last point - no control points
                        // allowed any more
                        DBG_ASSERT( inputPolygon.GetFlags(i) != POLY_CONTROL,
                                    "bezierSequenceFromPolygon(): Invalid point sequence in Polygon");

                        outputSequence[nOutPos++] = geometry::RealBezierSegment2D( inputPolygon[i].X(),
                                                                                   inputPolygon[i].Y(),
                                                                                   0.0, 0.0,
                                                                                   0.0, 0.0 );
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

                // fill sequence from polygon
                for( i=0; i<nSize; ++i )
                {
                    outputSequence[i] = geometry::RealPoint2D( inputPolygon[i].X(),
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

            if( !xGraphicDevice.is() )
                return uno::Reference< rendering::XPolyPolygon2D >();

            if( inputPolygon.HasFlags() )
            {
                uno::Sequence< uno::Sequence< geometry::RealBezierSegment2D > > outputSequence( 1 );
                outputSequence[0] = bezierSequenceFromPolygon( inputPolygon );

                return uno::Reference< rendering::XPolyPolygon2D >(
                    xGraphicDevice->createCompatibleBezierPolyPolygon( outputSequence ),
                    uno::UNO_QUERY );
            }
            else
            {
                uno::Sequence< uno::Sequence< geometry::RealPoint2D > > outputSequence( 1 );
                outputSequence[0] = pointSequenceFromPolygon( inputPolygon );

                return uno::Reference< rendering::XPolyPolygon2D >(
                    xGraphicDevice->createCompatibleLinePolyPolygon( outputSequence ),
                    uno::UNO_QUERY );
            }
        }

        //---------------------------------------------------------------------------------------

        uno::Reference< rendering::XPolyPolygon2D > xPolyPolygonFromPolyPolygon( const uno::Reference< rendering::XGraphicDevice >& xGraphicDevice,
                                                                                 const ::PolyPolygon&                               inputPolyPolygon )
        {
            RTL_LOGFILE_CONTEXT( aLog, "::vcl::unotools::xPolyPolygonFromPolyPolygon()" );

            if( !xGraphicDevice.is() )
                return uno::Reference< rendering::XPolyPolygon2D >();

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

                for( i=0; i<nNumPolys; ++i )
                {
                    outputSequence[i] = bezierSequenceFromPolygon( inputPolyPolygon[i] );
                }

                return uno::Reference< rendering::XPolyPolygon2D >(
                    xGraphicDevice->createCompatibleBezierPolyPolygon( outputSequence ),
                    uno::UNO_QUERY );
            }
            else
            {
                uno::Sequence< uno::Sequence< geometry::RealPoint2D > > outputSequence( nNumPolys );

                for( i=0; i<nNumPolys; ++i )
                {
                    outputSequence[i] = pointSequenceFromPolygon( inputPolyPolygon[i] );
                }

                return uno::Reference< rendering::XPolyPolygon2D >(
                    xGraphicDevice->createCompatibleLinePolyPolygon( outputSequence ),
                    uno::UNO_QUERY );
            }
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
                const geometry::RealBezierSegment2D aCurrSegment( curves[i] );

                if( aCurrSegment.Px == aCurrSegment.C1x &&
                    aCurrSegment.Px == aCurrSegment.C2x &&
                    aCurrSegment.Py == aCurrSegment.C1y &&
                    aCurrSegment.Py == aCurrSegment.C2y     )
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
                const geometry::RealBezierSegment2D aCurrSegment( curves[i] );

                if( aCurrSegment.Px == aCurrSegment.C1x &&
                    aCurrSegment.Px == aCurrSegment.C2x &&
                    aCurrSegment.Py == aCurrSegment.C1y &&
                    aCurrSegment.Py == aCurrSegment.C2y     )
                {
                    aPoly[i++] = Point( FRound( aCurrSegment.Px ),
                                        FRound( aCurrSegment.Py ) );
                }
                else
                {
                    aPoly[i] = Point( FRound( aCurrSegment.Px ),
                                      FRound( aCurrSegment.Py ) );
                    aPoly.SetFlags(i++, POLY_NORMAL);

                    aPoly[i] = Point( FRound( aCurrSegment.C1x ),
                                      FRound( aCurrSegment.C1y ) );
                    aPoly.SetFlags(i++, POLY_CONTROL);

                    aPoly[i] = Point( FRound( aCurrSegment.C2x ),
                                      FRound( aCurrSegment.C2y ) );
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

            if( !xGraphicDevice.is() )
                return uno::Reference< rendering::XBitmap >();

            const Size aPixelSize( inputBitmap.GetSizePixel() );

            uno::Reference< rendering::XBitmap >
                xBitmap( xGraphicDevice->createCompatibleBitmap(
                             integerSize2DFromSize( aPixelSize ) ) );

            uno::Reference< rendering::XIntegerBitmap >
                xIntegerBitmap( xBitmap,
                                uno::UNO_QUERY );

            ::Bitmap copyOfInputBitmap( inputBitmap );
            ScopedBitmapReadAccess pAcc( copyOfInputBitmap.AcquireReadAccess(),
                                         copyOfInputBitmap );

            const int nPixel( aPixelSize.Width() * aPixelSize.Height() );
            uno::Sequence< sal_Int8 > bmpData( nPixel*4 );

            // TODO: can we handle this more generically?
            // arrayToSequence comes to mind, or other templatized
            // algos.
            // TODO: handle the other types here!
            // TODO: handle bitmap layout, too!
            if( pAcc.get() &&
                xIntegerBitmap.is() )
            {
                BitmapColor aCol;

                // read from bitmap
                // =======================

                int x, y, i;
                for( y=0, i=0; y<aPixelSize.Height(); ++y )
                {
                    switch( pAcc->GetScanlineFormat() )
                    {
                        case BMP_FORMAT_8BIT_PAL:
                        {
                            Scanline pScan = pAcc->GetScanline( y );

                            for( x=0; x<aPixelSize.Width(); ++x )
                            {
                                aCol = pAcc->GetPaletteColor( pScan[x] );

                                // store as RGBA
                                // TODO: differentiate between alpha and non-alpha
                                // bitmaps. Maybe we even need a
                                // createCompatibleAlphaBitmap on the XGraphicDevice
                                bmpData[i++] = aCol.GetBlue();
                                bmpData[i++] = aCol.GetGreen();
                                bmpData[i++] = aCol.GetRed();
                                bmpData[i++] = static_cast<sal_uInt8>(255);
                            }
                        }
                        break;

                        case BMP_FORMAT_24BIT_TC_BGR:
                        {
                            Scanline pScan = pAcc->GetScanline( y );

                            for( x=0; x<aPixelSize.Width(); ++x )
                            {
                                Scanline pTmp = pScan + x * 3;

                                // store as RGBA
                                // TODO: differentiate between alpha and non-alpha
                                // bitmaps. Maybe we even need a
                                // createCompatibleAlphaBitmap on the XGraphicDevice
                                bmpData[i++] = pTmp[ 0 ];
                                bmpData[i++] = pTmp[ 1 ];
                                bmpData[i++] = pTmp[ 2 ];
                                bmpData[i++] = static_cast<sal_uInt8>(255);
                            }
                        }
                        break;

                        case BMP_FORMAT_24BIT_TC_RGB:
                        {
                            Scanline pScan = pAcc->GetScanline( y );

                            for( x=0; x<aPixelSize.Width(); ++x )
                            {
                                Scanline pTmp = pScan + x * 3;

                                // store as RGBA
                                // TODO: differentiate between alpha and non-alpha
                                // bitmaps. Maybe we even need a
                                // createCompatibleAlphaBitmap on the XGraphicDevice
                                bmpData[i++] = pTmp[ 2 ];
                                bmpData[i++] = pTmp[ 1 ];
                                bmpData[i++] = pTmp[ 0 ];
                                bmpData[i++] = static_cast<sal_uInt8>(255);
                            }
                        }
                        break;

                        default:
                        {
                            for( x=0; x<aPixelSize.Width(); ++x )
                            {
                                // yes. x and y are swapped on Get/SetPixel
                                aCol = pAcc->GetColor(y,x);

                                // store as RGBA
                                // TODO: differentiate between alpha and non-alpha
                                // bitmaps. Maybe we even need a
                                // createCompatibleAlphaBitmap on the XGraphicDevice
                                bmpData[i++] = aCol.GetBlue();
                                bmpData[i++] = aCol.GetGreen();
                                bmpData[i++] = aCol.GetRed();
                                bmpData[i++] = static_cast<sal_uInt8>(255);
                            }
                        }
                        break;
                    }
                }
            }

            // TODO(P2): Completely avoid alpha channel for plain
            // bitmaps here!
            uno::Sequence< sal_Int64 > aComponentMask(4);
            aComponentMask[0] = 0x0000FF00;
            aComponentMask[1] = 0x00FF0000;
            aComponentMask[2] = 0xFF000000;
            aComponentMask[3] = 0x000000FF;

            rendering::IntegerBitmapLayout aMemLayout(
                aPixelSize.Height(),
                4*aPixelSize.Width(),
                4*aPixelSize.Width(),
                0,
                4,
                aComponentMask,
                rendering::Endianness::LITTLE,
                rendering::IntegerBitmapFormat::CHUNKY_32BIT,
                sal_False );

            xIntegerBitmap->setData( bmpData,
                                     aMemLayout,
                                     geometry::IntegerRectangle2D(0,0,
                                                                  aPixelSize.Width(),
                                                                  aPixelSize.Height()) );

            return uno::Reference< rendering::XBitmap >( xIntegerBitmap,
                                                         uno::UNO_QUERY );
        }

        //---------------------------------------------------------------------------------------

        uno::Reference< rendering::XBitmap > xBitmapFromBitmapEx( const uno::Reference< rendering::XGraphicDevice >&    xGraphicDevice,
                                                                  const ::BitmapEx&                                     inputBitmap )
        {
            RTL_LOGFILE_CONTEXT( aLog, "::vcl::unotools::xBitmapFromBitmapEx()" );

            // if bitmapex is not transparent: revert to plain bitmap
            // method
            if( !inputBitmap.IsTransparent() )
                return xBitmapFromBitmap( xGraphicDevice,
                                          inputBitmap.GetBitmap() );

            if( !xGraphicDevice.is() )
                return uno::Reference< rendering::XBitmap >();

            const Size aPixelSize( inputBitmap.GetSizePixel() );

            uno::Reference< rendering::XBitmap >
                xBitmap( xGraphicDevice->createCompatibleAlphaBitmap(
                             integerSize2DFromSize( aPixelSize ) ) );

            uno::Reference< rendering::XIntegerBitmap >
                xIntegerBitmap( xBitmap,
                                uno::UNO_QUERY );

            Bitmap aBitmap( inputBitmap.GetBitmap() );
            ScopedBitmapReadAccess pAcc( aBitmap.AcquireReadAccess(),
                                         aBitmap );

            const int nPixel( aPixelSize.Width() * aPixelSize.Height() );
            uno::Sequence< sal_Int8 > bmpData( nPixel*4 );

            // TODO: can we handle this more generically?
            // arrayToSequence comes to mind, or other templatized
            // algos.
            // TODO: handle the other types here!
            // TODO: handle bitmap layout, too!
            if( pAcc.get() &&
                xIntegerBitmap.is() )
            {
                BitmapColor aCol;

                if( inputBitmap.IsTransparent() )
                {
                    if( inputBitmap.IsAlpha() )
                    {
                        // read from alpha bitmap
                        // ======================

                        Bitmap aAlpha( inputBitmap.GetAlpha().GetBitmap() );
                        ScopedBitmapReadAccess pAlphaAcc( aAlpha.AcquireReadAccess(),
                                                          aAlpha );

                        DBG_ASSERT( pAlphaAcc->GetScanlineFormat() == BMP_FORMAT_8BIT_PAL ||
                                    pAlphaAcc->GetScanlineFormat() == BMP_FORMAT_8BIT_TC_MASK,
                                    "::vcl::unotools::xBitmapFromBitmapEx(): non-8bit alpha not supported!" );

                        if( pAlphaAcc.get() )
                        {
                            int x, y, i;
                            for( y=0, i=0; y<aPixelSize.Height(); ++y )
                            {
                                switch( pAcc->GetScanlineFormat() )
                                {
                                    case BMP_FORMAT_8BIT_PAL:
                                    {
                                        Scanline pScan  = pAcc->GetScanline( y );
                                        Scanline pAScan = pAlphaAcc->GetScanline( y );

                                        for( x=0; x<aPixelSize.Width(); ++x )
                                        {
                                            aCol = pAcc->GetPaletteColor( pScan[x] );

                                            bmpData[i++] = aCol.GetBlue();
                                            bmpData[i++] = aCol.GetGreen();
                                            bmpData[i++] = aCol.GetRed();

                                            // out notion of alpha is different from the rest of the world's
                                            bmpData[i++] = 255 - (BYTE)pAScan[x];
                                        }
                                    }
                                    break;

                                    case BMP_FORMAT_24BIT_TC_BGR:
                                    {
                                        Scanline pScan  = pAcc->GetScanline( y );
                                        Scanline pAScan = pAlphaAcc->GetScanline( y );

                                        for( x=0; x<aPixelSize.Width(); ++x )
                                        {
                                            Scanline pTmp = pScan + x * 3;

                                            // store as RGBA
                                            bmpData[i++] = pTmp[ 0 ];
                                            bmpData[i++] = pTmp[ 1 ];
                                            bmpData[i++] = pTmp[ 2 ];

                                            // out notion of alpha is different from the rest of the world's
                                            bmpData[i++] = 255 - (BYTE)pAScan[x];
                                        }
                                    }
                                    break;

                                    case BMP_FORMAT_24BIT_TC_RGB:
                                    {
                                        Scanline pScan  = pAcc->GetScanline( y );
                                        Scanline pAScan = pAlphaAcc->GetScanline( y );

                                        for( x=0; x<aPixelSize.Width(); ++x )
                                        {
                                            Scanline pTmp = pScan + x * 3;

                                            // store as RGBA
                                            bmpData[i++] = pTmp[ 2 ];
                                            bmpData[i++] = pTmp[ 1 ];
                                            bmpData[i++] = pTmp[ 0 ];

                                            // out notion of alpha is different from the rest of the world's
                                            bmpData[i++] = 255 - (BYTE)pAScan[x];
                                        }
                                    }
                                    break;

                                    default:
                                    {
                                        for( x=0; x<aPixelSize.Width(); ++x )
                                        {
                                            // yes. x and y are swapped on Get/SetPixel
                                            aCol = pAcc->GetColor(y,x);

                                            bmpData[i++] = aCol.GetBlue();
                                            bmpData[i++] = aCol.GetGreen();
                                            bmpData[i++] = aCol.GetRed();

                                            // out notion of alpha is different from the rest of the world's
                                            bmpData[i++] = 255 - (BYTE)pAlphaAcc->GetPixel(y,x);
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                    }
                    else
                    {
                        // read from masked bitmap
                        // =======================

                        Bitmap aMask( inputBitmap.GetMask() );
                        ScopedBitmapReadAccess pMaskAcc( aMask.AcquireReadAccess(),
                                                         aMask );

                        if( pMaskAcc.get() )
                        {
                            DBG_ASSERT( pMaskAcc->GetScanlineFormat() == BMP_FORMAT_1BIT_MSB_PAL ||
                                        pMaskAcc->GetScanlineFormat() == BMP_FORMAT_1BIT_LSB_PAL,
                                        "::vcl::unotools::xBitmapFromBitmapEx(): non-1bit masks not supported!" );

                            const int nBitIncrement(
                                pMaskAcc->GetScanlineFormat() == BMP_FORMAT_1BIT_MSB_PAL ? -1 : 1 );
                            const int nInitialBit(
                                pMaskAcc->GetScanlineFormat() == BMP_FORMAT_1BIT_MSB_PAL ? 7 : 0 );

                            int x, y, i, nCurrBit, nMask;
                            for( y=0, i=0; y<aPixelSize.Height(); ++y )
                            {
                                switch( pAcc->GetScanlineFormat() )
                                {
                                    case BMP_FORMAT_8BIT_PAL:
                                    {
                                        Scanline pScan  = pAcc->GetScanline( y );
                                        Scanline pMScan = pMaskAcc->GetScanline( y );

                                        for( x=0, nCurrBit=nInitialBit; x<aPixelSize.Width(); ++x )
                                        {
                                            aCol = pAcc->GetPaletteColor( pScan[x] );

                                            bmpData[i++] = aCol.GetBlue();
                                            bmpData[i++] = aCol.GetGreen();
                                            bmpData[i++] = aCol.GetRed();

                                            nMask = 1L << nCurrBit;
                                            bmpData[i++] = (1 - (pMScan[ (x & ~7L) >> 3L ] & nMask) / nMask) * 255;
                                            nCurrBit = ((nCurrBit + nBitIncrement) % 8L) & 7L;
                                        }
                                    }
                                    break;

                                    case BMP_FORMAT_24BIT_TC_BGR:
                                    {
                                        Scanline pScan  = pAcc->GetScanline( y );
                                        Scanline pMScan = pMaskAcc->GetScanline( y );

                                        for( x=0, nCurrBit=nInitialBit; x<aPixelSize.Width(); ++x )
                                        {
                                            Scanline pTmp = pScan + x * 3;

                                            // store as RGBA
                                            bmpData[i++] = pTmp[ 0 ];
                                            bmpData[i++] = pTmp[ 1 ];
                                            bmpData[i++] = pTmp[ 2 ];

                                            nMask = 1L << nCurrBit;
                                            bmpData[i++] = (1 - (pMScan[ (x & ~7L) >> 3L ] & nMask) / nMask) * 255;
                                            nCurrBit = ((nCurrBit + nBitIncrement) % 8L) & 7L;
                                        }
                                    }
                                    break;

                                    case BMP_FORMAT_24BIT_TC_RGB:
                                    {
                                        Scanline pScan  = pAcc->GetScanline( y );
                                        Scanline pMScan = pMaskAcc->GetScanline( y );

                                        for( x=0, nCurrBit=nInitialBit; x<aPixelSize.Width(); ++x )
                                        {
                                            Scanline pTmp = pScan + x * 3;

                                            // store as RGBA
                                            bmpData[i++] = pTmp[ 2 ];
                                            bmpData[i++] = pTmp[ 1 ];
                                            bmpData[i++] = pTmp[ 0 ];

                                            nMask = 1L << nCurrBit;
                                            bmpData[i++] = (1 - (pMScan[ (x & ~7L) >> 3L ] & nMask) / nMask) * 255;
                                            nCurrBit = ((nCurrBit + nBitIncrement) % 8L) & 7L;
                                        }
                                    }
                                    break;

                                    default:
                                    {
                                        for( x=0; x<aPixelSize.Width(); ++x )
                                        {
                                            // yes. x and y are swapped on Get/SetPixel
                                            aCol = pAcc->GetColor(y,x);

                                            // store as RGBA
                                            bmpData[i++] = aCol.GetBlue();
                                            bmpData[i++] = aCol.GetGreen();
                                            bmpData[i++] = aCol.GetRed();
                                            bmpData[i++] = ((BYTE)pMaskAcc->GetPixel(y,x)) ? 0 : 255;
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            uno::Sequence< sal_Int64 > aComponentMask(4);
            aComponentMask[0] = 0x0000FF00;
            aComponentMask[1] = 0x00FF0000;
            aComponentMask[2] = 0xFF000000;
            aComponentMask[3] = 0x000000FF;

            rendering::IntegerBitmapLayout aMemLayout(
                aPixelSize.Height(),
                4*aPixelSize.Width(),
                4*aPixelSize.Width(),
                0,
                4,
                aComponentMask,
                rendering::Endianness::LITTLE,
                rendering::IntegerBitmapFormat::CHUNKY_32BIT,
                sal_False );

            xIntegerBitmap->setData( bmpData,
                                     aMemLayout,
                                     geometry::IntegerRectangle2D(0,0,
                                                                  aPixelSize.Width(),
                                                                  aPixelSize.Height()) );

            return uno::Reference< rendering::XBitmap >( xIntegerBitmap,
                                                         uno::UNO_QUERY );
        }

        //---------------------------------------------------------------------------------------

        ::BitmapEx bitmapExFromXBitmap( const uno::Reference< rendering::XGraphicDevice >&  xGraphicDevice,
                                        const uno::Reference< rendering::XIntegerBitmap >&  xInputBitmap )
        {
            RTL_LOGFILE_CONTEXT( aLog, "::vcl::unotools::bitmapExFromXBitmap()" );

            if( !xGraphicDevice.is() || !xInputBitmap.is() )
                return ::BitmapEx();

            const Size aPixelSize( sizeFromIntegerSize2D( xInputBitmap->getSize() ) );
            const uno::Sequence< sal_Int8 > data( xInputBitmap->getData(
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

            aRet[0] = rColor.GetRed() / 255.0;
            aRet[1] = rColor.GetGreen() / 255.0;
            aRet[2] = rColor.GetBlue() / 255.0;

            // out notion of alpha is different from the rest of the world's
            aRet[3] = 1.0 - rColor.GetTransparency() / 255.0;

            return aRet;
        }

        uno::Sequence< sal_Int8 > colorToIntSequence( const uno::Reference< rendering::XGraphicDevice >&    xGraphicDevice,
                                                      const Color&                                          rColor       )
        {
            // TODO: handle color space conversions, when defined on canvas/graphicDevice
            uno::Sequence< sal_Int8 > aRet(4);

            aRet[0] = rColor.GetRed();
            aRet[1] = rColor.GetGreen();
            aRet[2] = rColor.GetBlue();

            // out notion of alpha is different from the rest of the world's
            aRet[3] = 255 - rColor.GetTransparency();

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
