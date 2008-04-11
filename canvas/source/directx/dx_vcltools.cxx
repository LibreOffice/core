/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dx_vcltools.cxx,v $
 * $Revision: 1.3 $
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

#include <vcl/canvastools.hxx>

#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/bmpacc.hxx>

#include "dx_impltools.hxx"
#include <basegfx/numeric/ftools.hxx>

#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>


#include <boost/scoped_array.hpp>

#include "dx_vcltools.hxx"

using namespace ::com::sun::star;

namespace dxcanvas
{
    namespace tools
    {
        namespace
        {
            /// Calc number of colors in given BitmapInfoHeader
            sal_Int32 calcDIBColorCount( const BITMAPINFOHEADER& rBIH )
            {
                if( rBIH.biSize != sizeof( BITMAPCOREHEADER ) )
                {
                    if( rBIH.biBitCount <= 8 )
                    {
                        if( rBIH.biClrUsed )
                            return rBIH.biClrUsed;
                        else
                            return 1L << rBIH.biBitCount;
                    }
                }
                else
                {
                    BITMAPCOREHEADER* pCoreHeader = (BITMAPCOREHEADER*)&rBIH;

                    if( pCoreHeader->bcBitCount <= 8 )
                        return 1L << pCoreHeader->bcBitCount;
                }

                return 0; // nothing known
            }

            /// Draw DI bits to given Graphics
            bool drawDIBits( const ::boost::shared_ptr< SurfaceGraphics >&  rGraphics,
                             const void*                                    hDIB )
            {
                bool            bRet( false );
                BitmapSharedPtr pBitmap;

                const BITMAPINFO* pBI = (BITMAPINFO*)GlobalLock( (HGLOBAL)hDIB );

                if( pBI )
                {
                    const BITMAPINFOHEADER* pBIH = (BITMAPINFOHEADER*)pBI;
                    const BYTE*             pBits = (BYTE*) pBI + *(DWORD*)pBI +
                        calcDIBColorCount( *pBIH ) * sizeof( RGBQUAD );

                    // forward to outsourced GDI+ rendering method
                    // (header clashes)
                    bRet = tools::drawDIBits( rGraphics, *pBI, (void*)pBits );

                    GlobalUnlock( (HGLOBAL)hDIB );
                }

                return bRet;
            }

            /** Draw VCL bitmap to given Graphics

                @param rBmp
                Reference to bitmap. Might get modified, in such a way
                that it will hold a DIB after a successful function call.
             */
            bool drawVCLBitmap( const ::boost::shared_ptr< SurfaceGraphics >&   rGraphics,
                                ::Bitmap&                                       rBmp )
            {
                BitmapSystemData aBmpSysData;

                if( !rBmp.GetSystemData( aBmpSysData ) ||
                    !aBmpSysData.pDIB )
                {
                    // first of all, ensure that Bitmap contains a DIB, by
                    // aquiring a read access
                    BitmapReadAccess* pReadAcc = rBmp.AcquireReadAccess();

                    // TODO(P2): Acquiring a read access can actually
                    // force a read from VRAM, thus, avoiding this
                    // step somehow will increase performance
                    // here.
                    if( pReadAcc )
                    {
                        // try again: now, WinSalBitmap must have
                        // generated a DIB
                        if( rBmp.GetSystemData( aBmpSysData ) &&
                            aBmpSysData.pDIB )
                        {
                            return drawDIBits( rGraphics,
                                               aBmpSysData.pDIB );
                        }

                        rBmp.ReleaseAccess( pReadAcc );
                    }
                }
                else
                {
                    return drawDIBits( rGraphics,
                                       aBmpSysData.pDIB );
                }

                // failed to generate DIBits from vcl bitmap
                return false;
            }

            /** Create a chunk of raw RGBA data GDI+ Bitmap from VCL BbitmapEX
             */
            RawRGBABitmap bitmapFromVCLBitmapEx( const ::BitmapEx& rBmpEx )
            {
                // TODO(P2): Avoid temporary bitmap generation, maybe
                // even ensure that created DIBs are copied back to
                // BmpEx (currently, every AcquireReadAccess() will
                // make the local bitmap copy unique, effectively
                // duplicating the memory used)

                ENSURE_AND_THROW( rBmpEx.IsTransparent(),
                                  "::dxcanvas::tools::bitmapFromVCLBitmapEx(): "
                                  "BmpEx not transparent" );

                // convert transparent bitmap to 32bit RGBA
                // ========================================

                const ::Size aBmpSize( rBmpEx.GetSizePixel() );

                RawRGBABitmap aBmpData;
                aBmpData.mnWidth     = aBmpSize.Width();
                aBmpData.mnHeight    = aBmpSize.Height();
                aBmpData.mpBitmapData.reset( new sal_uInt8[ 4*aBmpData.mnWidth*aBmpData.mnHeight ] );

                Bitmap aBitmap( rBmpEx.GetBitmap() );

                ScopedBitmapReadAccess pReadAccess( aBitmap.AcquireReadAccess(),
                                                    aBitmap );

                const sal_Int32 nWidth( aBmpSize.Width() );
                const sal_Int32 nHeight( aBmpSize.Height() );

                ENSURE_AND_THROW( pReadAccess.get() != NULL,
                                  "::dxcanvas::tools::bitmapFromVCLBitmapEx(): "
                                  "Unable to acquire read acces to bitmap" );

                if( rBmpEx.IsAlpha() )
                {
                    Bitmap aAlpha( rBmpEx.GetAlpha().GetBitmap() );

                    ScopedBitmapReadAccess pAlphaReadAccess( aAlpha.AcquireReadAccess(),
                                                             aAlpha );

                    // By convention, the access buffer always has
                    // one of the following formats:
                    //
                    //    BMP_FORMAT_1BIT_MSB_PAL
                    //    BMP_FORMAT_4BIT_MSN_PAL
                    //    BMP_FORMAT_8BIT_PAL
                    //    BMP_FORMAT_16BIT_TC_LSB_MASK
                    //    BMP_FORMAT_24BIT_TC_BGR
                    //    BMP_FORMAT_32BIT_TC_MASK
                    //
                    // and is always BMP_FORMAT_BOTTOM_UP
                    //
                    // This is the way
                    // WinSalBitmap::AcquireBuffer() sets up the
                    // buffer

                    ENSURE_AND_THROW( pAlphaReadAccess.get() != NULL,
                                      "::dxcanvas::tools::bitmapFromVCLBitmapEx(): "
                                      "Unable to acquire read acces to alpha" );

                    ENSURE_AND_THROW( pAlphaReadAccess->GetScanlineFormat() == BMP_FORMAT_8BIT_PAL ||
                                      pAlphaReadAccess->GetScanlineFormat() == BMP_FORMAT_8BIT_TC_MASK,
                                      "::dxcanvas::tools::bitmapFromVCLBitmapEx(): "
                                      "Unsupported alpha scanline format" );

                    BitmapColor     aCol;
                    const sal_Int32 nWidth( aBmpSize.Width() );
                    const sal_Int32 nHeight( aBmpSize.Height() );
                    sal_uInt8*      pCurrOutput( aBmpData.mpBitmapData.get() );
                    int             x, y;

                    for( y=0; y<nHeight; ++y )
                    {
                        switch( pReadAccess->GetScanlineFormat() )
                        {
                            case BMP_FORMAT_8BIT_PAL:
                            {
                                Scanline pScan  = pReadAccess->GetScanline( y );
                                Scanline pAScan = pAlphaReadAccess->GetScanline( y );

                                for( x=0; x<nWidth; ++x )
                                {
                                    aCol = pReadAccess->GetPaletteColor( *pScan++ );

                                    *pCurrOutput++ = aCol.GetBlue();
                                    *pCurrOutput++ = aCol.GetGreen();
                                    *pCurrOutput++ = aCol.GetRed();

                                    // out notion of alpha is
                                    // different from the rest
                                    // of the world's
                                    *pCurrOutput++ = 255 - (BYTE)*pAScan++;
                                }
                            }
                            break;

                            case BMP_FORMAT_24BIT_TC_BGR:
                            {
                                Scanline pScan  = pReadAccess->GetScanline( y );
                                Scanline pAScan = pAlphaReadAccess->GetScanline( y );

                                for( x=0; x<nWidth; ++x )
                                {
                                    // store as RGBA
                                    *pCurrOutput++ = *pScan++;
                                    *pCurrOutput++ = *pScan++;
                                    *pCurrOutput++ = *pScan++;

                                    // out notion of alpha is
                                    // different from the rest
                                    // of the world's
                                    *pCurrOutput++ = 255 - (BYTE)*pAScan++;
                                }
                            }
                            break;

                            // TODO(P2): Might be advantageous
                            // to hand-formulate the following
                            // formats, too.
                            case BMP_FORMAT_1BIT_MSB_PAL:
                                // FALLTHROUGH intended
                            case BMP_FORMAT_4BIT_MSN_PAL:
                                // FALLTHROUGH intended
                            case BMP_FORMAT_16BIT_TC_LSB_MASK:
                                // FALLTHROUGH intended
                            case BMP_FORMAT_32BIT_TC_MASK:
                            {
                                Scanline pAScan = pAlphaReadAccess->GetScanline( y );

                                // using fallback for those
                                // seldom formats
                                for( x=0; x<nWidth; ++x )
                                {
                                    // yes. x and y are swapped on Get/SetPixel
                                    aCol = pReadAccess->GetColor(y,x);

                                    *pCurrOutput++ = aCol.GetBlue();
                                    *pCurrOutput++ = aCol.GetGreen();
                                    *pCurrOutput++ = aCol.GetRed();

                                    // out notion of alpha is
                                    // different from the rest
                                    // of the world's
                                    *pCurrOutput++ = 255 - (BYTE)*pAScan++;
                                }
                            }
                            break;

                            case BMP_FORMAT_1BIT_LSB_PAL:
                                // FALLTHROUGH intended
                            case BMP_FORMAT_4BIT_LSN_PAL:
                                // FALLTHROUGH intended
                            case BMP_FORMAT_8BIT_TC_MASK:
                                // FALLTHROUGH intended
                            case BMP_FORMAT_24BIT_TC_RGB:
                                // FALLTHROUGH intended
                            case BMP_FORMAT_24BIT_TC_MASK:
                                // FALLTHROUGH intended
                            case BMP_FORMAT_16BIT_TC_MSB_MASK:
                                // FALLTHROUGH intended
                            case BMP_FORMAT_32BIT_TC_ABGR:
                                // FALLTHROUGH intended
                            case BMP_FORMAT_32BIT_TC_ARGB:
                                // FALLTHROUGH intended
                            case BMP_FORMAT_32BIT_TC_BGRA:
                                // FALLTHROUGH intended
                            case BMP_FORMAT_32BIT_TC_RGBA:
                                // FALLTHROUGH intended
                            default:
                                ENSURE_AND_THROW( false,
                                                  "::dxcanvas::tools::bitmapFromVCLBitmapEx(): "
                                                  "Unexpected scanline format - has "
                                                  "WinSalBitmap::AcquireBuffer() changed?" );
                        }
                    }
                }
                else
                {
                    Bitmap aMask( rBmpEx.GetMask() );

                    ScopedBitmapReadAccess pMaskReadAccess( aMask.AcquireReadAccess(),
                                                            aMask );

                    // By convention, the access buffer always has
                    // one of the following formats:
                    //
                    //    BMP_FORMAT_1BIT_MSB_PAL
                    //    BMP_FORMAT_4BIT_MSN_PAL
                    //    BMP_FORMAT_8BIT_PAL
                    //    BMP_FORMAT_16BIT_TC_LSB_MASK
                    //    BMP_FORMAT_24BIT_TC_BGR
                    //    BMP_FORMAT_32BIT_TC_MASK
                    //
                    // and is always BMP_FORMAT_BOTTOM_UP
                    //
                    // This is the way
                    // WinSalBitmap::AcquireBuffer() sets up the
                    // buffer

                    ENSURE_AND_THROW( pMaskReadAccess.get() != NULL,
                                      "::dxcanvas::tools::bitmapFromVCLBitmapEx(): "
                                      "Unable to acquire read acces to mask" );

                    ENSURE_AND_THROW( pMaskReadAccess->GetScanlineFormat() == BMP_FORMAT_1BIT_MSB_PAL,
                                      "::dxcanvas::tools::bitmapFromVCLBitmapEx(): "
                                      "Unsupported mask scanline format" );

                    BitmapColor     aCol;
                    int             nCurrBit;
                    const int       nMask( 1L );
                    const int       nInitialBit(7);
                    sal_uInt8*      pCurrOutput( aBmpData.mpBitmapData.get() );
                    int             x, y;

                    // mapping table, to get from mask index color to
                    // alpha value (which depends on the mask's palette)
                    sal_uInt8 aColorMap[2];

                    const BitmapColor& rCol0( pMaskReadAccess->GetPaletteColor( 0 ) );
                    const BitmapColor& rCol1( pMaskReadAccess->GetPaletteColor( 1 ) );

                    // shortcut for true luminance calculation
                    // (assumes that palette is grey-level). Note the
                    // swapped the indices here, to account for the
                    // fact that VCL's notion of alpha is inverted to
                    // the rest of the world's.
                    aColorMap[0] = rCol1.GetRed();
                    aColorMap[1] = rCol0.GetRed();

                    for( y=0; y<nHeight; ++y )
                    {
                        switch( pReadAccess->GetScanlineFormat() )
                        {
                            case BMP_FORMAT_8BIT_PAL:
                            {
                                Scanline pScan  = pReadAccess->GetScanline( y );
                                Scanline pMScan = pMaskReadAccess->GetScanline( y );

                                for( x=0, nCurrBit=nInitialBit; x<nWidth; ++x )
                                {
                                    aCol = pReadAccess->GetPaletteColor( *pScan++ );

                                    *pCurrOutput++ = aCol.GetBlue();
                                    *pCurrOutput++ = aCol.GetGreen();
                                    *pCurrOutput++ = aCol.GetRed();

                                    *pCurrOutput++ = aColorMap[ (pMScan[ (x & ~7L) >> 3L ] >> nCurrBit ) & nMask ];
                                    nCurrBit = ((nCurrBit - 1) % 8L) & 7L;
                                }
                            }
                            break;

                            case BMP_FORMAT_24BIT_TC_BGR:
                            {
                                Scanline pScan  = pReadAccess->GetScanline( y );
                                Scanline pMScan = pMaskReadAccess->GetScanline( y );

                                for( x=0, nCurrBit=nInitialBit; x<nWidth; ++x )
                                {
                                    // store as RGBA
                                    *pCurrOutput++ = *pScan++;
                                    *pCurrOutput++ = *pScan++;
                                    *pCurrOutput++ = *pScan++;

                                    *pCurrOutput++ = aColorMap[ (pMScan[ (x & ~7L) >> 3L ] >> nCurrBit ) & nMask ];
                                    nCurrBit = ((nCurrBit - 1) % 8L) & 7L;
                                }
                            }
                            break;

                            // TODO(P2): Might be advantageous
                            // to hand-formulate the following
                            // formats, too.
                            case BMP_FORMAT_1BIT_MSB_PAL:
                                // FALLTHROUGH intended
                            case BMP_FORMAT_4BIT_MSN_PAL:
                                // FALLTHROUGH intended
                            case BMP_FORMAT_16BIT_TC_LSB_MASK:
                                // FALLTHROUGH intended
                            case BMP_FORMAT_32BIT_TC_MASK:
                            {
                                Scanline pMScan = pMaskReadAccess->GetScanline( y );

                                // using fallback for those
                                // seldom formats
                                for( x=0, nCurrBit=nInitialBit; x<nWidth; ++x )
                                {
                                    // yes. x and y are swapped on Get/SetPixel
                                    aCol = pReadAccess->GetColor(y,x);

                                    // store as RGBA
                                    *pCurrOutput++ = aCol.GetBlue();
                                    *pCurrOutput++ = aCol.GetGreen();
                                    *pCurrOutput++ = aCol.GetRed();

                                    *pCurrOutput++ = aColorMap[ (pMScan[ (x & ~7L) >> 3L ] >> nCurrBit ) & nMask ];
                                    nCurrBit = ((nCurrBit - 1) % 8L) & 7L;
                                }
                            }
                            break;

                            case BMP_FORMAT_1BIT_LSB_PAL:
                                // FALLTHROUGH intended
                            case BMP_FORMAT_4BIT_LSN_PAL:
                                // FALLTHROUGH intended
                            case BMP_FORMAT_8BIT_TC_MASK:
                                // FALLTHROUGH intended
                            case BMP_FORMAT_24BIT_TC_RGB:
                                // FALLTHROUGH intended
                            case BMP_FORMAT_24BIT_TC_MASK:
                                // FALLTHROUGH intended
                            case BMP_FORMAT_16BIT_TC_MSB_MASK:
                                // FALLTHROUGH intended
                            case BMP_FORMAT_32BIT_TC_ABGR:
                                // FALLTHROUGH intended
                            case BMP_FORMAT_32BIT_TC_ARGB:
                                // FALLTHROUGH intended
                            case BMP_FORMAT_32BIT_TC_BGRA:
                                // FALLTHROUGH intended
                            case BMP_FORMAT_32BIT_TC_RGBA:
                                // FALLTHROUGH intended
                            default:
                                ENSURE_AND_THROW( false,
                                                  "::dxcanvas::tools::bitmapFromVCLBitmapEx(): "
                                                  "Unexpected scanline format - has "
                                                  "WinSalBitmap::AcquireBuffer() changed?" );
                        }
                    }
                }

                return aBmpData;
            }

            bool drawVCLBitmapEx( const ::boost::shared_ptr< SurfaceGraphics >& rGraphics,
                                  const ::BitmapEx&                             rBmpEx )
            {
                if( !rBmpEx.IsTransparent() )
                {
                    Bitmap aBmp( rBmpEx.GetBitmap() );
                    return drawVCLBitmap( rGraphics, aBmp );
                }
                else
                {
                    return drawRGBABits( rGraphics,
                                         bitmapFromVCLBitmapEx( rBmpEx ) );
                }
            }

            bool bitmapExFromUnoTunnel( BitmapEx&                                   o_rBmpEx,
                                        const uno::Reference< lang::XUnoTunnel >&   xTunnel )
            {
                if( !xTunnel.is() )
                    return false;

                sal_Int64 nPtr = xTunnel->getSomething( vcl::unotools::getTunnelIdentifier( vcl::unotools::Id_BitmapEx ) );
                if( !nPtr )
                    return false;

                o_rBmpEx = *(BitmapEx*)nPtr;

                return true;
            }
        }

        bool drawVCLBitmapFromUnoTunnel( const ::boost::shared_ptr< SurfaceGraphics >&  rGraphics,
                                         const uno::Reference< lang::XUnoTunnel >&      xTunnel )
        {
            BitmapEx aBmpEx;

            if( !bitmapExFromUnoTunnel( aBmpEx,
                                        xTunnel ) )
                return false;

            return drawVCLBitmapEx( rGraphics, aBmpEx );
        }

        util::TriState isAlphaVCLBitmapFromUnoTunnel( const uno::Reference< lang::XUnoTunnel >& xTunnel )
        {
            if( !xTunnel.is() )
                return util::TriState_INDETERMINATE;

            sal_Int64 nPtr = xTunnel->getSomething( vcl::unotools::getTunnelIdentifier( vcl::unotools::Id_BitmapEx ) );
            if( !nPtr )
                return util::TriState_INDETERMINATE;

            BitmapEx& rBmpEx( *(BitmapEx*)nPtr );

            return rBmpEx.IsTransparent() ?
                util::TriState_YES : util::TriState_NO;
        }
    }
}
