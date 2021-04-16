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

#include <sal/config.h>

#include <basegfx/numeric/ftools.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <tools/diagnose_ex.h>
#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/BitmapReadAccess.hxx>
#include <vcl/canvastools.hxx>

#include "dx_impltools.hxx"
#include "dx_vcltools.hxx"

using namespace ::com::sun::star;

namespace dxcanvas::tools
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
                            return 1 << rBIH.biBitCount;
                    }
                }
                else
                {
                    BITMAPCOREHEADER const * pCoreHeader = reinterpret_cast<BITMAPCOREHEADER const *>(&rBIH);

                    if( pCoreHeader->bcBitCount <= 8 )
                        return 1 << pCoreHeader->bcBitCount;
                }

                return 0; // nothing known
            }

            /// Draw DI bits to given Graphics
            bool drawDIBits( const std::shared_ptr< Gdiplus::Graphics >& rGraphics,
                             const void*                                     hDIB )
            {
                bool            bRet( false );
                BitmapSharedPtr pBitmap;

                const BITMAPINFO* pBI = static_cast<BITMAPINFO*>(GlobalLock( const_cast<void *>(hDIB) ));

                if( pBI )
                {
                    const BYTE*             pBits = reinterpret_cast<BYTE const *>(pBI) + pBI->bmiHeader.biSize +
                        calcDIBColorCount( pBI->bmiHeader ) * sizeof( RGBQUAD );

                    // forward to outsourced GDI+ rendering method
                    // (header clashes)
                    bRet = tools::drawDIBits( rGraphics, *pBI, pBits );

                    GlobalUnlock( const_cast<void *>(hDIB) );
                }

                return bRet;
            }

            /** Draw VCL bitmap to given Graphics

                @param rBmp
                Reference to bitmap. Might get modified, in such a way
                that it will hold a DIB after a successful function call.
             */
            bool drawVCLBitmap( const std::shared_ptr< Gdiplus::Graphics >& rGraphics,
                                ::Bitmap&                                       rBmp )
            {
                BitmapSystemData aBmpSysData;

                if( !rBmp.GetSystemData( aBmpSysData ) ||
                    !aBmpSysData.pDIB )
                {
                    // first of all, ensure that Bitmap contains a DIB, by
                    // acquiring a read access
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

                        Bitmap::ReleaseAccess( pReadAcc );
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

            /** Create a chunk of raw RGBA data GDI+ Bitmap from VCL BitmapEx
             */
            RawRGBABitmap bitmapFromVCLBitmapEx( const ::BitmapEx& rBmpEx )
            {
                // TODO(P2): Avoid temporary bitmap generation, maybe
                // even ensure that created DIBs are copied back to
                // BmpEx (currently, every AcquireReadAccess() will
                // make the local bitmap copy unique, effectively
                // duplicating the memory used)

                ENSURE_OR_THROW( rBmpEx.IsAlpha(),
                                  "::dxcanvas::tools::bitmapFromVCLBitmapEx(): "
                                  "BmpEx has no alpha" );

                // convert transparent bitmap to 32bit RGBA
                // ========================================

                const ::Size aBmpSize( rBmpEx.GetSizePixel() );

                RawRGBABitmap aBmpData;
                aBmpData.mnWidth     = aBmpSize.Width();
                aBmpData.mnHeight    = aBmpSize.Height();
                aBmpData.maBitmapData.resize(4*aBmpData.mnWidth*aBmpData.mnHeight);

                Bitmap aBitmap( rBmpEx.GetBitmap() );

                Bitmap::ScopedReadAccess pReadAccess( aBitmap );

                const sal_Int32 nWidth( aBmpSize.Width() );
                const sal_Int32 nHeight( aBmpSize.Height() );

                ENSURE_OR_THROW( pReadAccess.get() != nullptr,
                                  "::dxcanvas::tools::bitmapFromVCLBitmapEx(): "
                                  "Unable to acquire read access to bitmap" );

                Bitmap aAlpha( rBmpEx.GetAlpha().GetBitmap() );

                Bitmap::ScopedReadAccess pAlphaReadAccess( aAlpha );

                // By convention, the access buffer always has
                // one of the following formats:

                //    ScanlineFormat::N1BitMsbPal
                //    ScanlineFormat::N8BitPal
                //    ScanlineFormat::N24BitTcBgr
                //    ScanlineFormat::N32BitTcMask

                // and is always ScanlineFormat::BottomUp

                // This is the way
                // WinSalBitmap::AcquireBuffer() sets up the
                // buffer

                ENSURE_OR_THROW( pAlphaReadAccess.get() != nullptr,
                                "::dxcanvas::tools::bitmapFromVCLBitmapEx(): "
                                "Unable to acquire read access to alpha" );

                ENSURE_OR_THROW( pAlphaReadAccess->GetScanlineFormat() == ScanlineFormat::N8BitPal,
                                "::dxcanvas::tools::bitmapFromVCLBitmapEx(): "
                                "Unsupported alpha scanline format" );

                BitmapColor     aCol;
                sal_uInt8*      pCurrOutput(aBmpData.maBitmapData.data());
                int             x, y;

                for( y=0; y<nHeight; ++y )
                {
                    switch( pReadAccess->GetScanlineFormat() )
                    {
                        case ScanlineFormat::N8BitPal:
                            {
                                Scanline pScan  = pReadAccess->GetScanline( y );
                                Scanline pAScan = pAlphaReadAccess->GetScanline( y );

                                for( x=0; x<nWidth; ++x )
                                {
                                    aCol = pReadAccess->GetPaletteColor( *pScan++ );

                                    *pCurrOutput++ = aCol.GetBlue();
                                    *pCurrOutput++ = aCol.GetGreen();
                                    *pCurrOutput++ = aCol.GetRed();
                                    *pCurrOutput++ = static_cast<BYTE>(*pAScan++);
                                }
                            }
                            break;

                        case ScanlineFormat::N24BitTcBgr:
                            {
                                Scanline pScan  = pReadAccess->GetScanline( y );
                                Scanline pAScan = pAlphaReadAccess->GetScanline( y );

                                for( x=0; x<nWidth; ++x )
                                {
                                    // store as RGBA
                                    *pCurrOutput++ = *pScan++;
                                    *pCurrOutput++ = *pScan++;
                                    *pCurrOutput++ = *pScan++;
                                    *pCurrOutput++ = static_cast<BYTE>(*pAScan++);
                                }
                            }
                            break;

                            // TODO(P2): Might be advantageous
                            // to hand-formulate the following
                            // formats, too.
                        case ScanlineFormat::N1BitMsbPal:
                        case ScanlineFormat::N32BitTcMask:
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
                                    *pCurrOutput++ = static_cast<BYTE>(*pAScan++);
                                }
                            }
                            break;

                        case ScanlineFormat::N1BitLsbPal:
                        case ScanlineFormat::N24BitTcRgb:
                        case ScanlineFormat::N32BitTcAbgr:
                        case ScanlineFormat::N32BitTcArgb:
                        case ScanlineFormat::N32BitTcBgra:
                        case ScanlineFormat::N32BitTcRgba:
                        default:
                            ENSURE_OR_THROW( false,
                                            "::dxcanvas::tools::bitmapFromVCLBitmapEx(): "
                                            "Unexpected scanline format - has "
                                            "WinSalBitmap::AcquireBuffer() changed?" );
                    }
                }

                return aBmpData;
            }

            bool drawVCLBitmapEx( const std::shared_ptr< Gdiplus::Graphics >& rGraphics,
                                  const ::BitmapEx&                               rBmpEx )
            {
                if( !rBmpEx.IsAlpha() )
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
        }

        bool drawVCLBitmapFromXBitmap( const std::shared_ptr< Gdiplus::Graphics >& rGraphics,
                                       const uno::Reference< rendering::XBitmap >&     xBitmap )
        {
            // TODO(F2): add support for floating point bitmap formats
            uno::Reference< rendering::XIntegerReadOnlyBitmap > xIntBmp(
                xBitmap, uno::UNO_QUERY );

            if( !xIntBmp.is() )
                return false;

            ::BitmapEx aBmpEx = vcl::unotools::bitmapExFromXBitmap( xIntBmp );
            if( aBmpEx.IsEmpty() )
                return false;

            return drawVCLBitmapEx( rGraphics, aBmpEx );
        }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
