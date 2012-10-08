/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>

#include <com/sun/star/util/Endianness.hpp>

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
        mpBackBuffer(),
        mpOutDevReference()
    {
    }

    void CanvasBitmapHelper::setBitmap( const BitmapEx& rBitmap )
    {
        ENSURE_OR_THROW( mpOutDev,
                         "Invalid reference device" );

        mpBackBuffer.reset( new BitmapBackBuffer( rBitmap,
                                                  mpOutDev->getOutDev() ) );

        // tell canvas helper about the new target OutDev (don't
        // protect state, it's our own VirDev, anyways)
        setOutDev( mpBackBuffer, false );
    }

    void CanvasBitmapHelper::init( const BitmapEx&                rBitmap,
                                   rendering::XGraphicDevice&     rDevice,
                                   const OutDevProviderSharedPtr& rOutDevReference )
    {
        mpOutDevReference = rOutDevReference;
        mpBackBuffer.reset( new BitmapBackBuffer( rBitmap, rOutDevReference->getOutDev() ));

        // forward new settings to base class (ref device, output
        // surface, no protection (own backbuffer), alpha depends on
        // whether BmpEx is transparent or not)
        CanvasHelper::init( rDevice,
                            mpBackBuffer,
                            false,
                            rBitmap.IsTransparent() );
    }

    void CanvasBitmapHelper::disposing()
    {
        mpBackBuffer.reset();
        mpOutDevReference.reset();

        // forward to base class
        CanvasHelper::disposing();
    }

    geometry::IntegerSize2D CanvasBitmapHelper::getSize()
    {
        if( !mpBackBuffer )
            return geometry::IntegerSize2D();

        return ::vcl::unotools::integerSize2DFromSize( mpBackBuffer->getBitmapSizePixel() );
    }

    void CanvasBitmapHelper::clear()
    {
        // are we disposed?
        if( mpBackBuffer )
            mpBackBuffer->clear(); // alpha vdev needs special treatment
    }

    uno::Reference< rendering::XBitmap > CanvasBitmapHelper::getScaledBitmap( const geometry::RealSize2D&   newSize,
                                                                              sal_Bool                      beFast )
    {
        ENSURE_OR_THROW( mpDevice,
                          "disposed CanvasHelper" );

        RTL_LOGFILE_CONTEXT( aLog, "::vclcanvas::CanvasBitmapHelper::getScaledBitmap()" );

        if( !mpBackBuffer || mpDevice )
            return uno::Reference< rendering::XBitmap >(); // we're disposed

        BitmapEx aRes( mpBackBuffer->getBitmapReference() );

        aRes.Scale( ::vcl::unotools::sizeFromRealSize2D(newSize),
                     beFast ? BMP_SCALE_FAST : BMP_SCALE_DEFAULT );

        return uno::Reference< rendering::XBitmap >(
            new CanvasBitmap( aRes, *mpDevice, mpOutDevReference ) );
    }

    uno::Sequence< sal_Int8 > CanvasBitmapHelper::getData( rendering::IntegerBitmapLayout&      rLayout,
                                                           const geometry::IntegerRectangle2D&  rect )
    {
        RTL_LOGFILE_CONTEXT( aLog, "::vclcanvas::CanvasBitmapHelper::getData()" );

        if( !mpBackBuffer )
            return uno::Sequence< sal_Int8 >(); // we're disposed

        rLayout = getMemoryLayout();
        Bitmap aBitmap( mpBackBuffer->getBitmapReference().GetBitmap() );
        Bitmap aAlpha( mpBackBuffer->getBitmapReference().GetAlpha().GetBitmap() );

        Bitmap::ScopedReadAccess pReadAccess( aBitmap );
        Bitmap::ScopedReadAccess pAlphaReadAccess( aAlpha.IsEmpty() ?
                                                 (BitmapReadAccess*)NULL : aAlpha.AcquireReadAccess(),
                                                 aAlpha );

        ENSURE_OR_THROW( pReadAccess.get() != NULL,
                         "Could not acquire read access to bitmap" );

        // TODO(F1): Support more formats.
        const Size aBmpSize( aBitmap.GetSizePixel() );

        rLayout.ScanLines = aBmpSize.Height();
        rLayout.ScanLineBytes = aBmpSize.Width()*4;
        rLayout.ScanLineStride = rLayout.ScanLineBytes;

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
                    pRes[ nCurrPos++ ] = pReadAccess->GetColor( y, x ).GetRed();
                    pRes[ nCurrPos++ ] = pReadAccess->GetColor( y, x ).GetGreen();
                    pRes[ nCurrPos++ ] = pReadAccess->GetColor( y, x ).GetBlue();
                    pRes[ nCurrPos++ ] = pAlphaReadAccess->GetPixel( y, x ).GetIndex();
                }
            }
            else
            {
                for( int x=rect.X1;
                     x<aBmpSize.Width() && x<rect.X2;
                     ++x )
                {
                    pRes[ nCurrPos++ ] = pReadAccess->GetColor( y, x ).GetRed();
                    pRes[ nCurrPos++ ] = pReadAccess->GetColor( y, x ).GetGreen();
                    pRes[ nCurrPos++ ] = pReadAccess->GetColor( y, x ).GetBlue();
                    pRes[ nCurrPos++ ] = sal_uInt8(255);
                }
            }
        }

        return aRes;
    }

    void CanvasBitmapHelper::setData( const uno::Sequence< sal_Int8 >&      data,
                                      const rendering::IntegerBitmapLayout& rLayout,
                                      const geometry::IntegerRectangle2D&   rect )
    {
        RTL_LOGFILE_CONTEXT( aLog, "::vclcanvas::CanvasBitmapHelper::setData()" );

        if( !mpBackBuffer )
            return; // we're disposed

        const rendering::IntegerBitmapLayout aRefLayout( getMemoryLayout() );
        ENSURE_ARG_OR_THROW( aRefLayout.PlaneStride != rLayout.PlaneStride ||
                             aRefLayout.ColorSpace  != rLayout.ColorSpace ||
                             aRefLayout.Palette     != rLayout.Palette ||
                             aRefLayout.IsMsbFirst  != rLayout.IsMsbFirst,
                             "Mismatching memory layout" );

        // retrieve local copies from the BitmapEx, which are later
        // stored back. Unfortunately, the BitmapEx does not permit
        // in-place modifications, as they are necessary here.
        Bitmap aBitmap( mpBackBuffer->getBitmapReference().GetBitmap() );
        Bitmap aAlpha( mpBackBuffer->getBitmapReference().GetAlpha().GetBitmap() );

        bool bCopyBack( false ); // only copy something back, if we
                                 // actually changed a pixel

        {
            Bitmap::ScopedWriteAccess pWriteAccess( aBitmap );
            Bitmap::ScopedWriteAccess pAlphaWriteAccess( aAlpha.IsEmpty() ?
                                                       (BitmapWriteAccess*)NULL : aAlpha.AcquireWriteAccess(),
                                                       aAlpha );

            if( pAlphaWriteAccess.get() )
            {
                DBG_ASSERT( pAlphaWriteAccess->GetScanlineFormat() == BMP_FORMAT_8BIT_PAL ||
                            pAlphaWriteAccess->GetScanlineFormat() == BMP_FORMAT_8BIT_TC_MASK,
                            "non-8bit alpha not supported!" );
            }

            ENSURE_OR_THROW( pWriteAccess.get() != NULL,
                             "Could not acquire write access to bitmap" );

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
                                *pScan++ = (sal_uInt8)pWriteAccess->GetBestPaletteIndex(
                                    BitmapColor( data[ nCurrPos   ],
                                                 data[ nCurrPos+1 ],
                                                 data[ nCurrPos+2 ] ) );

                                nCurrPos += 3;

                                // cast to unsigned byte, for correct subtraction result
                                *pAScan++ = static_cast<sal_uInt8>(255 -
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
                                *pScan++ = data[ nCurrPos+2 ];
                                *pScan++ = data[ nCurrPos+1 ];
                                *pScan++ = data[ nCurrPos   ];

                                nCurrPos += 3;

                                // cast to unsigned byte, for correct subtraction result
                                *pAScan++ = static_cast<sal_uInt8>(255 -
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
                                *pScan++ = data[ nCurrPos   ];
                                *pScan++ = data[ nCurrPos+1 ];
                                *pScan++ = data[ nCurrPos+2 ];

                                nCurrPos += 3;

                                // cast to unsigned byte, for correct subtraction result
                                *pAScan++ = static_cast<sal_uInt8>(255 -
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
                                pWriteAccess->SetPixel( y, x, BitmapColor( data[ nCurrPos   ],
                                                                           data[ nCurrPos+1 ],
                                                                           data[ nCurrPos+2 ] ) );
                                nCurrPos += 3;

                                // cast to unsigned byte, for correct subtraction result
                                pAlphaWriteAccess->SetPixel( y, x,
                                                             BitmapColor(
                                                                 static_cast<sal_uInt8>(255 -
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
                                *pScan++ = (sal_uInt8)pWriteAccess->GetBestPaletteIndex(
                                    BitmapColor( data[ nCurrPos   ],
                                                 data[ nCurrPos+1 ],
                                                 data[ nCurrPos+2 ] ) );

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
                                *pScan++ = data[ nCurrPos+2 ];
                                *pScan++ = data[ nCurrPos+1 ];
                                *pScan++ = data[ nCurrPos   ];

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
                                *pScan++ = data[ nCurrPos   ];
                                *pScan++ = data[ nCurrPos+1 ];
                                *pScan++ = data[ nCurrPos+2 ];

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
                                pWriteAccess->SetPixel( y, x, BitmapColor( data[ nCurrPos   ],
                                                                           data[ nCurrPos+1 ],
                                                                           data[ nCurrPos+2 ] ) );
                                nCurrPos += 4; // skip three colors, _plus_ alpha
                            }
                        }
                        break;
                    }
                }

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

    void CanvasBitmapHelper::setPixel( const uno::Sequence< sal_Int8 >&      color,
                                       const rendering::IntegerBitmapLayout& rLayout,
                                       const geometry::IntegerPoint2D&       pos )
    {
        RTL_LOGFILE_CONTEXT( aLog, "::vclcanvas::CanvasBitmapHelper::setPixel()" );

        if( !mpBackBuffer )
            return; // we're disposed

        const Size aBmpSize( mpBackBuffer->getBitmapReference().GetSizePixel() );

        ENSURE_ARG_OR_THROW( pos.X >= 0 && pos.X < aBmpSize.Width(),
                         "X coordinate out of bounds" );
        ENSURE_ARG_OR_THROW( pos.Y >= 0 && pos.Y < aBmpSize.Height(),
                         "Y coordinate out of bounds" );
        ENSURE_ARG_OR_THROW( color.getLength() > 3,
                         "not enough color components" );

        const rendering::IntegerBitmapLayout aRefLayout( getMemoryLayout() );
        ENSURE_ARG_OR_THROW( aRefLayout.PlaneStride != rLayout.PlaneStride ||
                             aRefLayout.ColorSpace  != rLayout.ColorSpace ||
                             aRefLayout.Palette     != rLayout.Palette ||
                             aRefLayout.IsMsbFirst  != rLayout.IsMsbFirst,
                             "Mismatching memory layout" );

        // retrieve local copies from the BitmapEx, which are later
        // stored back. Unfortunately, the BitmapEx does not permit
        // in-place modifications, as they are necessary here.
        Bitmap aBitmap( mpBackBuffer->getBitmapReference().GetBitmap() );
        Bitmap aAlpha( mpBackBuffer->getBitmapReference().GetAlpha().GetBitmap() );

        bool bCopyBack( false ); // only copy something back, if we
                                 // actually changed a pixel

        {
            Bitmap::ScopedWriteAccess pWriteAccess( aBitmap );
            Bitmap::ScopedWriteAccess pAlphaWriteAccess( aAlpha.IsEmpty() ?
                                                       (BitmapWriteAccess*)NULL : aAlpha.AcquireWriteAccess(),
                                                       aAlpha );

            ENSURE_OR_THROW( pWriteAccess.get() != NULL,
                             "Could not acquire write access to bitmap" );

            pWriteAccess->SetPixel( pos.Y, pos.X, BitmapColor( color[ 0 ],
                                                               color[ 1 ],
                                                               color[ 2 ] ) );

            if( pAlphaWriteAccess.get() != NULL )
                pAlphaWriteAccess->SetPixel( pos.Y, pos.X, BitmapColor( 255 - color[ 3 ] ) );

            bCopyBack = true;
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

    uno::Sequence< sal_Int8 > CanvasBitmapHelper::getPixel( rendering::IntegerBitmapLayout& rLayout,
                                                            const geometry::IntegerPoint2D& pos )
    {
        RTL_LOGFILE_CONTEXT( aLog, "::vclcanvas::CanvasBitmapHelper::getPixel()" );

        if( !mpBackBuffer )
            return uno::Sequence< sal_Int8 >(); // we're disposed

        rLayout = getMemoryLayout();
        rLayout.ScanLines = 1;
        rLayout.ScanLineBytes = 4;
        rLayout.ScanLineStride = rLayout.ScanLineBytes;

        const Size aBmpSize( mpBackBuffer->getBitmapReference().GetSizePixel() );

        ENSURE_ARG_OR_THROW( pos.X >= 0 && pos.X < aBmpSize.Width(),
                         "X coordinate out of bounds" );
        ENSURE_ARG_OR_THROW( pos.Y >= 0 && pos.Y < aBmpSize.Height(),
                         "Y coordinate out of bounds" );

        Bitmap aBitmap( mpBackBuffer->getBitmapReference().GetBitmap() );
        Bitmap aAlpha( mpBackBuffer->getBitmapReference().GetAlpha().GetBitmap() );

        Bitmap::ScopedReadAccess pReadAccess( aBitmap );
        Bitmap::ScopedReadAccess pAlphaReadAccess( aAlpha.IsEmpty() ?
                                                 (BitmapReadAccess*)NULL : aAlpha.AcquireReadAccess(),
                                                 aAlpha );
        ENSURE_OR_THROW( pReadAccess.get() != NULL,
                         "Could not acquire read access to bitmap" );

        uno::Sequence< sal_Int8 > aRes( 4 );
        sal_Int8* pRes = aRes.getArray();

        const BitmapColor aColor( pReadAccess->GetColor( pos.Y, pos.X ) );
        pRes[ 0 ] = aColor.GetRed();
        pRes[ 1 ] = aColor.GetGreen();
        pRes[ 2 ] = aColor.GetBlue();

        if( pAlphaReadAccess.get() != NULL )
            pRes[ 3 ] = pAlphaReadAccess->GetPixel( pos.Y, pos.X ).GetIndex();
        else
            pRes[ 3 ] = sal_uInt8(255);

        return aRes;
    }

    rendering::IntegerBitmapLayout CanvasBitmapHelper::getMemoryLayout()
    {
        if( !mpOutDev.get() )
            return rendering::IntegerBitmapLayout(); // we're disposed

        return ::canvas::tools::getStdMemoryLayout(getSize());
    }

    BitmapEx CanvasBitmapHelper::getBitmap() const
    {
        if( !mpBackBuffer )
            return BitmapEx(); // we're disposed
        else
            return mpBackBuffer->getBitmapReference();
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
