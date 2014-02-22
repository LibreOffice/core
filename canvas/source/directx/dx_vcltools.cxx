/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <vcl/canvastools.hxx>

#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/bmpacc.hxx>
#include <tools/diagnose_ex.h>

#include "dx_impltools.hxx"
#include <basegfx/numeric/ftools.hxx>

#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>

#include <boost/scoped_array.hpp>

#include "dx_vcltools.hxx"

using namespace ::com::sun::star;

namespace dxcanvas
{
    namespace tools
    {
        namespace
        {
            
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

                return 0; 
            }

            
            bool drawDIBits( const ::boost::shared_ptr< Gdiplus::Graphics >& rGraphics,
                             const void*                                     hDIB )
            {
                bool            bRet( false );
                BitmapSharedPtr pBitmap;

                const BITMAPINFO* pBI = (BITMAPINFO*)GlobalLock( (HGLOBAL)hDIB );

                if( pBI )
                {
                    const BITMAPINFOHEADER* pBIH = (BITMAPINFOHEADER*)pBI;
                    const BYTE*             pBits = (BYTE*) pBI + *(DWORD*)pBI +
                        calcDIBColorCount( *pBIH ) * sizeof( RGBQUAD );

                    
                    
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
            bool drawVCLBitmap( const ::boost::shared_ptr< Gdiplus::Graphics >& rGraphics,
                                ::Bitmap&                                       rBmp )
            {
                BitmapSystemData aBmpSysData;

                if( !rBmp.GetSystemData( aBmpSysData ) ||
                    !aBmpSysData.pDIB )
                {
                    
                    
                    BitmapReadAccess* pReadAcc = rBmp.AcquireReadAccess();

                    
                    
                    
                    
                    if( pReadAcc )
                    {
                        
                        
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

                
                return false;
            }

            /** Create a chunk of raw RGBA data GDI+ Bitmap from VCL BbitmapEX
             */
            RawRGBABitmap bitmapFromVCLBitmapEx( const ::BitmapEx& rBmpEx )
            {
                
                
                
                
                

                ENSURE_OR_THROW( rBmpEx.IsTransparent(),
                                  "::dxcanvas::tools::bitmapFromVCLBitmapEx(): "
                                  "BmpEx not transparent" );

                
                

                const ::Size aBmpSize( rBmpEx.GetSizePixel() );

                RawRGBABitmap aBmpData;
                aBmpData.mnWidth     = aBmpSize.Width();
                aBmpData.mnHeight    = aBmpSize.Height();
                aBmpData.mpBitmapData.reset( new sal_uInt8[ 4*aBmpData.mnWidth*aBmpData.mnHeight ] );

                Bitmap aBitmap( rBmpEx.GetBitmap() );

                Bitmap::ScopedReadAccess pReadAccess( aBitmap );

                const sal_Int32 nWidth( aBmpSize.Width() );
                const sal_Int32 nHeight( aBmpSize.Height() );

                ENSURE_OR_THROW( pReadAccess.get() != NULL,
                                  "::dxcanvas::tools::bitmapFromVCLBitmapEx(): "
                                  "Unable to acquire read acces to bitmap" );

                if( rBmpEx.IsAlpha() )
                {
                    Bitmap aAlpha( rBmpEx.GetAlpha().GetBitmap() );

                    Bitmap::ScopedReadAccess pAlphaReadAccess( aAlpha );

                    
                    
                    //
                    
                    
                    
                    
                    
                    
                    //
                    
                    //
                    
                    
                    

                    ENSURE_OR_THROW( pAlphaReadAccess.get() != NULL,
                                      "::dxcanvas::tools::bitmapFromVCLBitmapEx(): "
                                      "Unable to acquire read acces to alpha" );

                    ENSURE_OR_THROW( pAlphaReadAccess->GetScanlineFormat() == BMP_FORMAT_8BIT_PAL ||
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
                                    
                                    *pCurrOutput++ = *pScan++;
                                    *pCurrOutput++ = *pScan++;
                                    *pCurrOutput++ = *pScan++;

                                    
                                    
                                    
                                    *pCurrOutput++ = 255 - (BYTE)*pAScan++;
                                }
                            }
                            break;

                            
                            
                            
                            case BMP_FORMAT_1BIT_MSB_PAL:
                                
                            case BMP_FORMAT_4BIT_MSN_PAL:
                                
                            case BMP_FORMAT_16BIT_TC_LSB_MASK:
                                
                            case BMP_FORMAT_32BIT_TC_MASK:
                            {
                                Scanline pAScan = pAlphaReadAccess->GetScanline( y );

                                
                                
                                for( x=0; x<nWidth; ++x )
                                {
                                    
                                    aCol = pReadAccess->GetColor(y,x);

                                    *pCurrOutput++ = aCol.GetBlue();
                                    *pCurrOutput++ = aCol.GetGreen();
                                    *pCurrOutput++ = aCol.GetRed();

                                    
                                    
                                    
                                    *pCurrOutput++ = 255 - (BYTE)*pAScan++;
                                }
                            }
                            break;

                            case BMP_FORMAT_1BIT_LSB_PAL:
                                
                            case BMP_FORMAT_4BIT_LSN_PAL:
                                
                            case BMP_FORMAT_8BIT_TC_MASK:
                                
                            case BMP_FORMAT_24BIT_TC_RGB:
                                
                            case BMP_FORMAT_24BIT_TC_MASK:
                                
                            case BMP_FORMAT_16BIT_TC_MSB_MASK:
                                
                            case BMP_FORMAT_32BIT_TC_ABGR:
                                
                            case BMP_FORMAT_32BIT_TC_ARGB:
                                
                            case BMP_FORMAT_32BIT_TC_BGRA:
                                
                            case BMP_FORMAT_32BIT_TC_RGBA:
                                
                            default:
                                ENSURE_OR_THROW( false,
                                                  "::dxcanvas::tools::bitmapFromVCLBitmapEx(): "
                                                  "Unexpected scanline format - has "
                                                  "WinSalBitmap::AcquireBuffer() changed?" );
                        }
                    }
                }
                else
                {
                    Bitmap aMask( rBmpEx.GetMask() );

                    Bitmap::ScopedReadAccess pMaskReadAccess( aMask );

                    
                    
                    //
                    
                    
                    
                    
                    
                    
                    //
                    
                    //
                    
                    
                    

                    ENSURE_OR_THROW( pMaskReadAccess.get() != NULL,
                                      "::dxcanvas::tools::bitmapFromVCLBitmapEx(): "
                                      "Unable to acquire read acces to mask" );

                    ENSURE_OR_THROW( pMaskReadAccess->GetScanlineFormat() == BMP_FORMAT_1BIT_MSB_PAL,
                                      "::dxcanvas::tools::bitmapFromVCLBitmapEx(): "
                                      "Unsupported mask scanline format" );

                    BitmapColor     aCol;
                    int             nCurrBit;
                    const int       nMask( 1L );
                    const int       nInitialBit(7);
                    sal_uInt8*      pCurrOutput( aBmpData.mpBitmapData.get() );
                    int             x, y;

                    
                    
                    sal_uInt8 aColorMap[2];

                    const BitmapColor& rCol0( pMaskReadAccess->GetPaletteColor( 0 ) );
                    const BitmapColor& rCol1( pMaskReadAccess->GetPaletteColor( 1 ) );

                    
                    
                    
                    
                    
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
                                    
                                    *pCurrOutput++ = *pScan++;
                                    *pCurrOutput++ = *pScan++;
                                    *pCurrOutput++ = *pScan++;

                                    *pCurrOutput++ = aColorMap[ (pMScan[ (x & ~7L) >> 3L ] >> nCurrBit ) & nMask ];
                                    nCurrBit = ((nCurrBit - 1) % 8L) & 7L;
                                }
                            }
                            break;

                            
                            
                            
                            case BMP_FORMAT_1BIT_MSB_PAL:
                                
                            case BMP_FORMAT_4BIT_MSN_PAL:
                                
                            case BMP_FORMAT_16BIT_TC_LSB_MASK:
                                
                            case BMP_FORMAT_32BIT_TC_MASK:
                            {
                                Scanline pMScan = pMaskReadAccess->GetScanline( y );

                                
                                
                                for( x=0, nCurrBit=nInitialBit; x<nWidth; ++x )
                                {
                                    
                                    aCol = pReadAccess->GetColor(y,x);

                                    
                                    *pCurrOutput++ = aCol.GetBlue();
                                    *pCurrOutput++ = aCol.GetGreen();
                                    *pCurrOutput++ = aCol.GetRed();

                                    *pCurrOutput++ = aColorMap[ (pMScan[ (x & ~7L) >> 3L ] >> nCurrBit ) & nMask ];
                                    nCurrBit = ((nCurrBit - 1) % 8L) & 7L;
                                }
                            }
                            break;

                            case BMP_FORMAT_1BIT_LSB_PAL:
                                
                            case BMP_FORMAT_4BIT_LSN_PAL:
                                
                            case BMP_FORMAT_8BIT_TC_MASK:
                                
                            case BMP_FORMAT_24BIT_TC_RGB:
                                
                            case BMP_FORMAT_24BIT_TC_MASK:
                                
                            case BMP_FORMAT_16BIT_TC_MSB_MASK:
                                
                            case BMP_FORMAT_32BIT_TC_ABGR:
                                
                            case BMP_FORMAT_32BIT_TC_ARGB:
                                
                            case BMP_FORMAT_32BIT_TC_BGRA:
                                
                            case BMP_FORMAT_32BIT_TC_RGBA:
                                
                            default:
                                ENSURE_OR_THROW( false,
                                                  "::dxcanvas::tools::bitmapFromVCLBitmapEx(): "
                                                  "Unexpected scanline format - has "
                                                  "WinSalBitmap::AcquireBuffer() changed?" );
                        }
                    }
                }

                return aBmpData;
            }

            bool drawVCLBitmapEx( const ::boost::shared_ptr< Gdiplus::Graphics >& rGraphics,
                                  const ::BitmapEx&                               rBmpEx )
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
        }

        bool drawVCLBitmapFromXBitmap( const ::boost::shared_ptr< Gdiplus::Graphics >& rGraphics,
                                       const uno::Reference< rendering::XBitmap >&     xBitmap )
        {
            
            uno::Reference< rendering::XIntegerReadOnlyBitmap > xIntBmp(
                xBitmap, uno::UNO_QUERY );

            if( !xIntBmp.is() )
                return false;

            ::BitmapEx aBmpEx = ::vcl::unotools::bitmapExFromXBitmap( xIntBmp );
            if( !aBmpEx )
                return false;

            return drawVCLBitmapEx( rGraphics, aBmpEx );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
