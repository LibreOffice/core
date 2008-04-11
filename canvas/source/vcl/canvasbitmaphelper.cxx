/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: canvasbitmaphelper.cxx,v $
 * $Revision: 1.12 $
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
#include "precompiled_canvas.hxx"

#include <canvas/debug.hxx>

#include <com/sun/star/rendering/IntegerBitmapFormat.hpp>
#include <com/sun/star/rendering/Endianness.hpp>

#include <rtl/logfile.hxx>
#include <rtl/math.hxx>

#include <tools/poly.hxx>
#include <vcl/window.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/canvastools.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/numeric/ftools.hxx>

#include <canvas/canvastools.hxx>

#include "canvasbitmap.hxx"
#include "canvasbitmaphelper.hxx"


using namespace ::com::sun::star;

namespace vclcanvas
{
    CanvasBitmapHelper::CanvasBitmapHelper() :
        mpBackBuffer()
    {
    }

    void CanvasBitmapHelper::setBitmap( const BitmapEx& rBitmap )
    {
        ENSURE_AND_THROW( mpDevice && mpDevice->getOutDev(),
                          "CanvasBitmapHelper::setBitmap(): Invalid reference device" );

        mpBackBuffer.reset( new BitmapBackBuffer( rBitmap,
                                                  *mpDevice->getOutDev() ) );

        // tell canvas helper about the new target OutDev (don't
        // protect state, it's our own VirDev, anyways)
        setOutDev( mpBackBuffer, false );
    }

    void CanvasBitmapHelper::init( const BitmapEx&  rBitmap,
                                   SpriteCanvas&    rSpriteCanvas )
    {
        ENSURE_AND_THROW( rSpriteCanvas.getOutDev(),
                          "CanvasBitmapHelper::setBitmap(): Invalid reference device" );

        mpBackBuffer.reset( new BitmapBackBuffer( rBitmap,
                                                  *rSpriteCanvas.getOutDev() ) );

        // forward new settings to base class (ref device, output
        // surface, no protection (own backbuffer), alpha depends on
        // whether BmpEx is transparent or not)
        CanvasHelper::init( rSpriteCanvas,
                            mpBackBuffer,
                            false,
                            rBitmap.IsTransparent() );
    }

    void CanvasBitmapHelper::disposing()
    {
        mpBackBuffer.reset();

        // forward to base class
        CanvasHelper::disposing();
    }

    geometry::IntegerSize2D CanvasBitmapHelper::getSize()
    {
        if( !mpBackBuffer )
            return geometry::IntegerSize2D();

        return ::vcl::unotools::integerSize2DFromSize( mpBackBuffer->getBitmapReference().GetSizePixel() );
    }

    uno::Reference< rendering::XBitmap > CanvasBitmapHelper::getScaledBitmap( const geometry::RealSize2D&   newSize,
                                                                              sal_Bool                      beFast )
    {
        ENSURE_AND_THROW( mpDevice,
                          "CanvasBitmapHelper::getScaledBitmap(): disposed CanvasHelper" );

        RTL_LOGFILE_CONTEXT( aLog, "::vclcanvas::CanvasBitmapHelper::getScaledBitmap()" );

        if( !mpBackBuffer )
            return uno::Reference< rendering::XBitmap >(); // we're disposed

        BitmapEx aRes( mpBackBuffer->getBitmapReference() );

        aRes.Scale( ::vcl::unotools::sizeFromRealSize2D(newSize),
                     beFast ? BMP_SCALE_FAST : BMP_SCALE_INTERPOLATE );

        return uno::Reference< rendering::XBitmap >(
            new CanvasBitmap( aRes, mpDevice ) );
    }

    uno::Sequence< sal_Int8 > CanvasBitmapHelper::getData( rendering::IntegerBitmapLayout&      aLayout,
                                                           const geometry::IntegerRectangle2D&  rect )
    {
        RTL_LOGFILE_CONTEXT( aLog, "::vclcanvas::CanvasBitmapHelper::getData()" );

        if( !mpBackBuffer )
            return uno::Sequence< sal_Int8 >(); // we're disposed

        const sal_Int32 nWidth( rect.X2 - rect.X1 );
        const sal_Int32 nHeight( rect.Y2 - rect.Y1 );
        aLayout.ScanLines = nHeight;
        aLayout.ScanLineBytes = nWidth*4;
        aLayout.ScanLineStride = aLayout.ScanLineBytes;
        aLayout.PlaneStride = 0;
        aLayout.ColorSpace.set( mpDevice );
        aLayout.NumComponents = 4;
        aLayout.ComponentMasks.realloc(4);
        aLayout.ComponentMasks[0] = 0x00FF0000;
        aLayout.ComponentMasks[1] = 0x0000FF00;
        aLayout.ComponentMasks[2] = 0x000000FF;
        aLayout.ComponentMasks[3] = 0xFF000000;
        aLayout.Palette.clear();
        aLayout.Endianness = rendering::Endianness::LITTLE;
        aLayout.Format = rendering::IntegerBitmapFormat::CHUNKY_32BIT;
        aLayout.IsMsbFirst = sal_False;

        Bitmap aBitmap( mpBackBuffer->getBitmapReference().GetBitmap() );
        Bitmap aAlpha( mpBackBuffer->getBitmapReference().GetAlpha().GetBitmap() );

        ScopedBitmapReadAccess pReadAccess( aBitmap.AcquireReadAccess(),
                                            aBitmap );
        ScopedBitmapReadAccess pAlphaReadAccess( aAlpha.IsEmpty() ?
                                                 (BitmapReadAccess*)NULL : aAlpha.AcquireReadAccess(),
                                                 aAlpha );

        if( pReadAccess.get() != NULL )
        {
            // TODO(F1): Support more formats.
            const Size aBmpSize( aBitmap.GetSizePixel() );

            // for the time being, always return as BGRA
            uno::Sequence< sal_Int8 > aRes( 4*aBmpSize.Width()*aBmpSize.Height() );
            sal_Int8* pRes = aRes.getArray();

            int nCurrPos(0);
            for( int y=rect.Y1;
                 y<aBmpSize.Height() && y<rect.Y2;
                 ++y )
            {
                if( pAlphaReadAccess.get() != NULL )
                {
                    for( int x=rect.X1;
                         x<aBmpSize.Width() && x<rect.X2;
                         ++x )
                    {
                        pRes[ nCurrPos++ ] = pReadAccess->GetColor( y, x ).GetBlue();
                        pRes[ nCurrPos++ ] = pReadAccess->GetColor( y, x ).GetGreen();
                        pRes[ nCurrPos++ ] = pReadAccess->GetColor( y, x ).GetRed();
                        pRes[ nCurrPos++ ] = pAlphaReadAccess->GetPixel( y, x ).GetIndex();
                    }
                }
                else
                {
                    for( int x=rect.X1;
                         x<aBmpSize.Width() && x<rect.X2;
                         ++x )
                    {
                        pRes[ nCurrPos++ ] = pReadAccess->GetColor( y, x ).GetBlue();
                        pRes[ nCurrPos++ ] = pReadAccess->GetColor( y, x ).GetGreen();
                        pRes[ nCurrPos++ ] = pReadAccess->GetColor( y, x ).GetRed();
                        pRes[ nCurrPos++ ] = sal_uInt8(255);
                    }
                }
            }

            return aRes;
        }

        return uno::Sequence< sal_Int8 >();
    }

    void CanvasBitmapHelper::setData( const uno::Sequence< sal_Int8 >&      data,
                                      const rendering::IntegerBitmapLayout& ,
                                      const geometry::IntegerRectangle2D&   rect )
    {
        RTL_LOGFILE_CONTEXT( aLog, "::vclcanvas::CanvasBitmapHelper::setData()" );

        if( !mpBackBuffer )
            return; // we're disposed

        // retrieve local copies from the BitmapEx, which are later
        // stored back. Unfortunately, the BitmapEx does not permit
        // in-place modifications, as they are necessary here.
        Bitmap aBitmap( mpBackBuffer->getBitmapReference().GetBitmap() );
        Bitmap aAlpha( mpBackBuffer->getBitmapReference().GetAlpha().GetBitmap() );

        bool bCopyBack( false ); // only copy something back, if we
                                 // actually changed a pixel

        {
            ScopedBitmapWriteAccess pWriteAccess( aBitmap.AcquireWriteAccess(),
                                                  aBitmap );
            ScopedBitmapWriteAccess pAlphaWriteAccess( aAlpha.IsEmpty() ?
                                                       (BitmapWriteAccess*)NULL : aAlpha.AcquireWriteAccess(),
                                                       aAlpha );

            if( pAlphaWriteAccess.get() )
            {
                DBG_ASSERT( pAlphaWriteAccess->GetScanlineFormat() == BMP_FORMAT_8BIT_PAL ||
                            pAlphaWriteAccess->GetScanlineFormat() == BMP_FORMAT_8BIT_TC_MASK,
                            "CanvasBitmapHelper::setData(): non-8bit alpha not supported!" );
            }

            if( pWriteAccess.get() != NULL )
            {
                // TODO(F1): Support more formats.
                const Size aBmpSize( aBitmap.GetSizePixel() );

                // for the time being, always read as BGRA
                int x, y, nCurrPos(0);
                for( y=rect.Y1;
                     y<aBmpSize.Height() && y<rect.Y2;
                     ++y )
                {
                    if( pAlphaWriteAccess.get() != NULL )
                    {
                        switch( pWriteAccess->GetScanlineFormat() )
                        {
                            case BMP_FORMAT_8BIT_PAL:
                            {
                                Scanline pScan  = pWriteAccess->GetScanline( y );
                                Scanline pAScan = pAlphaWriteAccess->GetScanline( y );

                                for( x=rect.X1;
                                     x<aBmpSize.Width() && x<rect.X2;
                                     ++x )
                                {
                                    *pScan++ = (BYTE)pWriteAccess->GetBestPaletteIndex(
                                        BitmapColor( data[ nCurrPos+2 ],
                                                     data[ nCurrPos+1 ],
                                                     data[ nCurrPos ] ) );

                                    nCurrPos += 3;

                                    // cast to unsigned byte, for correct subtraction result
                                    *pAScan++ = static_cast<BYTE>(255 -
                                                                  static_cast<sal_uInt8>(data[ nCurrPos++ ]));
                                }
                            }
                            break;

                            case BMP_FORMAT_24BIT_TC_BGR:
                            {
                                Scanline pScan  = pWriteAccess->GetScanline( y );
                                Scanline pAScan = pAlphaWriteAccess->GetScanline( y );

                                for( x=rect.X1;
                                     x<aBmpSize.Width() && x<rect.X2;
                                     ++x )
                                {
                                    *pScan++ = data[ nCurrPos   ];
                                    *pScan++ = data[ nCurrPos+1 ];
                                    *pScan++ = data[ nCurrPos+2 ];

                                    nCurrPos += 3;

                                    // cast to unsigned byte, for correct subtraction result
                                    *pAScan++ = static_cast<BYTE>(255 -
                                                                  static_cast<sal_uInt8>(data[ nCurrPos++ ]));
                                }
                            }
                            break;

                            case BMP_FORMAT_24BIT_TC_RGB:
                            {
                                Scanline pScan  = pWriteAccess->GetScanline( y );
                                Scanline pAScan = pAlphaWriteAccess->GetScanline( y );

                                for( x=rect.X1;
                                     x<aBmpSize.Width() && x<rect.X2;
                                     ++x )
                                {
                                    *pScan++ = data[ nCurrPos+2 ];
                                    *pScan++ = data[ nCurrPos+1 ];
                                    *pScan++ = data[ nCurrPos   ];

                                    nCurrPos += 3;

                                    // cast to unsigned byte, for correct subtraction result
                                    *pAScan++ = static_cast<BYTE>(255 -
                                                                  static_cast<sal_uInt8>(data[ nCurrPos++ ]));
                                }
                            }
                            break;

                            default:
                            {
                                for( x=rect.X1;
                                     x<aBmpSize.Width() && x<rect.X2;
                                     ++x )
                                {
                                    pWriteAccess->SetPixel( y, x, BitmapColor( data[ nCurrPos+2 ],
                                                                               data[ nCurrPos+1 ],
                                                                               data[ nCurrPos ] ) );
                                    nCurrPos += 3;

                                    // cast to unsigned byte, for correct subtraction result
                                    pAlphaWriteAccess->SetPixel( y, x,
                                                                 BitmapColor(
                                                                     static_cast<BYTE>(255 -
                                                                                       static_cast<sal_uInt8>(data[ nCurrPos++ ])) ) );
                                }
                            }
                            break;
                        }
                    }
                    else
                    {
                        // TODO(Q3): This is copy'n'pasted from
                        // canvashelper.cxx, unify!
                        switch( pWriteAccess->GetScanlineFormat() )
                        {
                            case BMP_FORMAT_8BIT_PAL:
                            {
                                Scanline pScan = pWriteAccess->GetScanline( y );

                                for( x=rect.X1;
                                     x<aBmpSize.Width() && x<rect.X2;
                                     ++x )
                                {
                                    *pScan++ = (BYTE)pWriteAccess->GetBestPaletteIndex(
                                        BitmapColor( data[ nCurrPos+2 ],
                                                     data[ nCurrPos+1 ],
                                                     data[ nCurrPos ] ) );

                                    nCurrPos += 4; // skip three colors, _plus_ alpha
                                }
                            }
                            break;

                            case BMP_FORMAT_24BIT_TC_BGR:
                            {
                                Scanline pScan = pWriteAccess->GetScanline( y );

                                for( x=rect.X1;
                                     x<aBmpSize.Width() && x<rect.X2;
                                     ++x )
                                {
                                    *pScan++ = data[ nCurrPos   ];
                                    *pScan++ = data[ nCurrPos+1 ];
                                    *pScan++ = data[ nCurrPos+2 ];

                                    nCurrPos += 4; // skip three colors, _plus_ alpha
                                }
                            }
                            break;

                            case BMP_FORMAT_24BIT_TC_RGB:
                            {
                                Scanline pScan = pWriteAccess->GetScanline( y );

                                for( x=rect.X1;
                                     x<aBmpSize.Width() && x<rect.X2;
                                     ++x )
                                {
                                    *pScan++ = data[ nCurrPos+2 ];
                                    *pScan++ = data[ nCurrPos+1 ];
                                    *pScan++ = data[ nCurrPos   ];

                                    nCurrPos += 4; // skip three colors, _plus_ alpha
                                }
                            }
                            break;

                            default:
                            {
                                for( x=rect.X1;
                                     x<aBmpSize.Width() && x<rect.X2;
                                     ++x )
                                {
                                    pWriteAccess->SetPixel( y, x, BitmapColor( data[ nCurrPos+2 ],
                                                                               data[ nCurrPos+1 ],
                                                                               data[ nCurrPos ] ) );
                                    nCurrPos += 4; // skip three colors, _plus_ alpha
                                }
                            }
                            break;
                        }
                    }

                    bCopyBack = true;
                }
            }
        }

        // copy back only here, since the BitmapAccessors must be
        // destroyed beforehand
        if( bCopyBack )
        {
            if( aAlpha.IsEmpty() )
                setBitmap( BitmapEx( aBitmap ) );
            else
                setBitmap( BitmapEx( aBitmap,
                                     AlphaMask( aAlpha ) ) );
        }
    }

    void CanvasBitmapHelper::setPixel( const uno::Sequence< sal_Int8 >&         color,
                                       const rendering::IntegerBitmapLayout&    ,
                                       const geometry::IntegerPoint2D&          pos )
    {
        RTL_LOGFILE_CONTEXT( aLog, "::vclcanvas::CanvasBitmapHelper::setPixel()" );

        if( !mpBackBuffer )
            return; // we're disposed

        const Size aBmpSize( mpBackBuffer->getBitmapReference().GetSizePixel() );

        CHECK_AND_THROW( pos.X >= 0 && pos.X < aBmpSize.Width(),
                         "CanvasBitmapHelper::setPixel: X coordinate out of bounds" );
        CHECK_AND_THROW( pos.Y >= 0 && pos.Y < aBmpSize.Height(),
                         "CanvasBitmapHelper::setPixel: Y coordinate out of bounds" );
        CHECK_AND_THROW( color.getLength() > 3,
                         "CanvasBitmapHelper::setPixel: not enough color components" );

        // retrieve local copies from the BitmapEx, which are later
        // stored back. Unfortunately, the BitmapEx does not permit
        // in-place modifications, as they are necessary here.
        Bitmap aBitmap( mpBackBuffer->getBitmapReference().GetBitmap() );
        Bitmap aAlpha( mpBackBuffer->getBitmapReference().GetAlpha().GetBitmap() );

        bool bCopyBack( false ); // only copy something back, if we
                                 // actually changed a pixel

        {
            ScopedBitmapWriteAccess pWriteAccess( aBitmap.AcquireWriteAccess(),
                                                  aBitmap );
            ScopedBitmapWriteAccess pAlphaWriteAccess( aAlpha.IsEmpty() ?
                                                       (BitmapWriteAccess*)NULL : aAlpha.AcquireWriteAccess(),
                                                       aAlpha );

            if( pWriteAccess.get() != NULL )
            {
                pWriteAccess->SetPixel( pos.Y, pos.X, BitmapColor( color[ 2 ],
                                                                   color[ 1 ],
                                                                   color[ 0 ] ) );

                if( pAlphaWriteAccess.get() != NULL )
                    pAlphaWriteAccess->SetPixel( pos.Y, pos.X, BitmapColor( 255 - color[ 3 ] ) );

                bCopyBack = true;
            }
        }

        // copy back only here, since the BitmapAccessors must be
        // destroyed beforehand
        if( bCopyBack )
        {
            if( aAlpha.IsEmpty() )
                setBitmap( BitmapEx( aBitmap ) );
            else
                setBitmap( BitmapEx( aBitmap,
                                     AlphaMask( aAlpha ) ) );
        }
    }

    uno::Sequence< sal_Int8 > CanvasBitmapHelper::getPixel( rendering::IntegerBitmapLayout& ,
                                                            const geometry::IntegerPoint2D& pos )
    {
        RTL_LOGFILE_CONTEXT( aLog, "::vclcanvas::CanvasBitmapHelper::getPixel()" );

        if( !mpBackBuffer )
            return uno::Sequence< sal_Int8 >(); // we're disposed

        const Size aBmpSize( mpBackBuffer->getBitmapReference().GetSizePixel() );

        CHECK_AND_THROW( pos.X >= 0 && pos.X < aBmpSize.Width(),
                         "CanvasBitmapHelper::getPixel: X coordinate out of bounds" );
        CHECK_AND_THROW( pos.Y >= 0 && pos.Y < aBmpSize.Height(),
                         "CanvasBitmapHelper::getPixel: Y coordinate out of bounds" );

        Bitmap aBitmap( mpBackBuffer->getBitmapReference().GetBitmap() );
        Bitmap aAlpha( mpBackBuffer->getBitmapReference().GetAlpha().GetBitmap() );

        ScopedBitmapReadAccess pReadAccess( aBitmap.AcquireReadAccess(),
                                            aBitmap );
        ScopedBitmapReadAccess pAlphaReadAccess( aAlpha.IsEmpty() ?
                                                 (BitmapReadAccess*)NULL : aAlpha.AcquireReadAccess(),
                                                 aAlpha );

        if( pReadAccess.get() != NULL )
        {
            // for the time being, always return as BGRA
            uno::Sequence< sal_Int8 > aRes( 4 );
            sal_Int8* pRes = aRes.getArray();

            const BitmapColor aColor( pReadAccess->GetColor( pos.Y, pos.X ) );
            pRes[ 3 ] = aColor.GetRed();
            pRes[ 2 ] = aColor.GetGreen();
            pRes[ 1 ] = aColor.GetBlue();

            if( pAlphaReadAccess.get() != NULL )
                pRes[ 3 ] = pAlphaReadAccess->GetPixel( pos.Y, pos.X ).GetIndex();
            else
                pRes[ 3 ] = sal_uInt8(255);

            return aRes;
        }

        return uno::Sequence< sal_Int8 >();
    }

    uno::Reference< rendering::XBitmapPalette > CanvasBitmapHelper::getPalette()
    {
        // TODO(F1): Provide palette support
        return uno::Reference< rendering::XBitmapPalette >();
    }

    rendering::IntegerBitmapLayout CanvasBitmapHelper::getMemoryLayout()
    {
        // TODO(F1): finish that one
        rendering::IntegerBitmapLayout aLayout;

        if( !mpBackBuffer )
            return aLayout; // we're disposed

        const BitmapEx& rBmpEx( mpBackBuffer->getBitmapReference() );
        const Size aBmpSize( rBmpEx.GetSizePixel() );

        aLayout.ScanLines = aBmpSize.Height();
        aLayout.ScanLineBytes = aBmpSize.Width()*4;
        aLayout.ScanLineStride = aLayout.ScanLineBytes;
        aLayout.PlaneStride = 0;
        // cast away const, need to change refcount (as this is
        // ~invisible to client code, still logically const)
        aLayout.ColorSpace.set( mpDevice );
        aLayout.NumComponents = 4;
        aLayout.ComponentMasks.realloc(4);
        aLayout.ComponentMasks[0] = 0x00FF0000;
        aLayout.ComponentMasks[1] = 0x0000FF00;
        aLayout.ComponentMasks[2] = 0x000000FF;
        aLayout.ComponentMasks[3] = 0xFF000000;
        aLayout.Palette.clear();
        aLayout.Endianness = rendering::Endianness::LITTLE;
        aLayout.Format = rendering::IntegerBitmapFormat::CHUNKY_32BIT;
        aLayout.IsMsbFirst = sal_False;

        return aLayout;
    }

    BitmapEx CanvasBitmapHelper::getBitmap() const
    {
        if( !mpBackBuffer )
            return BitmapEx(); // we're disposed
        else
            return mpBackBuffer->getBitmapReference();
    }

}
