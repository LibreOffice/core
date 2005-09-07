/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: canvasbitmaphelper.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 23:17:47 $
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

#include <canvas/debug.hxx>

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif
#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

#ifndef _TL_POLY_HXX
#include <tools/poly.hxx>
#endif
#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _SV_BITMAPEX_HXX
#include <vcl/bitmapex.hxx>
#endif
#ifndef _SV_BMPACC_HXX
#include <vcl/bmpacc.hxx>
#endif
#ifndef _VCL_CANVASTOOLS_HXX
#include <vcl/canvastools.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif
#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif
#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#include <basegfx/tools/canvastools.hxx>
#endif
#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

#include <canvas/canvastools.hxx>

#include "canvasbitmap.hxx"


using namespace ::com::sun::star;

namespace vclcanvas
{
    CanvasBitmapHelper::CanvasBitmapHelper() :
        mpBackBuffer()
    {
    }

    void CanvasBitmapHelper::setBitmap( const BitmapEx&                     rBitmap,
                                        const WindowGraphicDevice::ImplRef& rDevice )
    {
        ENSURE_AND_THROW( rDevice.is() && rDevice->getOutDev(),
                          "CanvasBitmapHelper::setBitmap(): Invalid reference device" );

        mpBackBuffer.reset( new BitmapBackBuffer( rBitmap,
                                                  *rDevice->getOutDev() ) );

        // forward new settings to base class
        setGraphicDevice( rDevice );
        setOutDev( mpBackBuffer, false );
    }

    void CanvasBitmapHelper::disposing()
    {
        mpBackBuffer.reset();

        // forward to base class
        CanvasHelper::disposing();
    }

    geometry::IntegerSize2D SAL_CALL CanvasBitmapHelper::getSize()
    {
        if( !mpBackBuffer.get() )
            return geometry::IntegerSize2D();

        return ::vcl::unotools::integerSize2DFromSize( mpBackBuffer->getBitmapReference().GetSizePixel() );
    }

    uno::Reference< rendering::XBitmap > SAL_CALL CanvasBitmapHelper::getScaledBitmap( const geometry::RealSize2D&  newSize,
                                                                                       sal_Bool                     beFast )
    {
        RTL_LOGFILE_CONTEXT( aLog, "::vclcanvas::CanvasBitmapHelper::getScaledBitmap()" );

        if( !mpBackBuffer.get() )
            return uno::Reference< rendering::XBitmap >(); // we're disposed

        BitmapEx aRes( mpBackBuffer->getBitmapReference() );

        aRes.Scale( ::vcl::unotools::sizeFromRealSize2D(newSize),
                     beFast ? BMP_SCALE_FAST : BMP_SCALE_INTERPOLATE );

        return uno::Reference< rendering::XBitmap >( new CanvasBitmap( aRes,
                                                                       mxDevice ) );
    }

    uno::Sequence< sal_Int8 > SAL_CALL CanvasBitmapHelper::getData( const geometry::IntegerRectangle2D& rect )
    {
        RTL_LOGFILE_CONTEXT( aLog, "::vclcanvas::CanvasBitmapHelper::getData()" );

        if( !mpBackBuffer.get() )
            return uno::Sequence< sal_Int8 >(); // we're disposed

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
                        pRes[ nCurrPos++ ] = (sal_Int8)255;
                    }
                }
            }

            return aRes;
        }

        return uno::Sequence< sal_Int8 >();
    }

    void SAL_CALL CanvasBitmapHelper::setData( const uno::Sequence< sal_Int8 >&     data,
                                               const geometry::IntegerRectangle2D&  rect )
    {
        RTL_LOGFILE_CONTEXT( aLog, "::vclcanvas::CanvasBitmapHelper::setData()" );

        if( !mpBackBuffer.get() )
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
                                    Scanline pTmp = pScan + x * 3;

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
                                    Scanline pTmp = pScan + x * 3;

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
                                    Scanline pTmp = pScan + x * 3;

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
                                    Scanline pTmp = pScan + x * 3;

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
                setBitmap( BitmapEx( aBitmap ),
                           mxDevice );
            else
                setBitmap( BitmapEx( aBitmap,
                                     AlphaMask( aAlpha ) ),
                           mxDevice );
        }
    }

    void SAL_CALL CanvasBitmapHelper::setPixel( const uno::Sequence< sal_Int8 >&    color,
                                                const geometry::IntegerPoint2D&     pos )
    {
        RTL_LOGFILE_CONTEXT( aLog, "::vclcanvas::CanvasBitmapHelper::setPixel()" );

        if( !mpBackBuffer.get() )
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
                setBitmap( BitmapEx( aBitmap ),
                           mxDevice );
            else
                setBitmap( BitmapEx( aBitmap,
                                     AlphaMask( aAlpha ) ),
                           mxDevice );
        }
    }

    uno::Sequence< sal_Int8 > SAL_CALL CanvasBitmapHelper::getPixel( const geometry::IntegerPoint2D& pos )
    {
        RTL_LOGFILE_CONTEXT( aLog, "::vclcanvas::CanvasBitmapHelper::getPixel()" );

        if( !mpBackBuffer.get() )
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
                pRes[ 3 ] = (sal_Int8)255;

            return aRes;
        }

        return uno::Sequence< sal_Int8 >();
    }

    uno::Reference< rendering::XBitmapPalette > SAL_CALL CanvasBitmapHelper::getPalette()
    {
        // TODO(F1): Provide palette support
        return uno::Reference< rendering::XBitmapPalette >();
    }

    rendering::IntegerBitmapLayout SAL_CALL CanvasBitmapHelper::getMemoryLayout()
    {
        // TODO(F1): finish that one
        rendering::IntegerBitmapLayout aLayout;

        if( !mpBackBuffer.get() )
            return aLayout; // we're disposed

        const Size aBmpSize( mpBackBuffer->getBitmapReference().GetSizePixel() );

        aLayout.ScanLines = aBmpSize.Height();
        aLayout.ScanLineBytes = aBmpSize.Width()*4;
        aLayout.ScanLineStride = aLayout.ScanLineBytes;
        aLayout.Format = 0;
        aLayout.NumComponents = 4;
        aLayout.ComponentMasks = uno::Sequence<sal_Int64>();
        aLayout.Endianness = 0;
        aLayout.IsPseudoColor = false;

        return aLayout;
    }

    BitmapEx CanvasBitmapHelper::getBitmap() const
    {
        if( !mpBackBuffer.get() )
            return BitmapEx(); // we're disposed
        else
            return mpBackBuffer->getBitmapReference();
    }

}
