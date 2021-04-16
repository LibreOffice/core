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

#include <memory>

#include <cppuhelper/supportsservice.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/bitmapex.hxx>

#include <canvas/canvastools.hxx>

#include "dx_canvasbitmap.hxx"
#include "dx_impltools.hxx"


using namespace ::com::sun::star;

namespace dxcanvas
{
    CanvasBitmap::CanvasBitmap( const IBitmapSharedPtr& rBitmap,
                                const DeviceRef&        rDevice ) :
        mpDevice( rDevice ),
        mpBitmap( rBitmap )
    {
        ENSURE_OR_THROW( mpDevice.is() && mpBitmap,
                         "CanvasBitmap::CanvasBitmap(): Invalid surface or device" );

        maCanvasHelper.setDevice( *mpDevice );
        maCanvasHelper.setTarget( mpBitmap );
    }

    void CanvasBitmap::disposeThis()
    {
        mpBitmap.reset();
        mpDevice.clear();

        // forward to parent
        CanvasBitmap_Base::disposeThis();
    }

    namespace {

    struct AlphaDIB
    {
        BITMAPINFOHEADER bmiHeader;
        RGBQUAD          bmiColors[256];
        AlphaDIB()
            : bmiHeader({0,0,0,1,8,BI_RGB,0,0,0,0,0})
        {
            for (size_t i = 0; i < 256; ++i)
            {
                // this here fills palette with grey level colors, starting
                // from 0,0,0 up to 255,255,255
                BYTE const b(i);
                bmiColors[i] = { b,b,b,b };
            }
        }
    };

    }

    uno::Any SAL_CALL CanvasBitmap::getFastPropertyValue( sal_Int32 nHandle )
    {
        uno::Any aRes;
        // 0 ... get BitmapEx
        // 1 ... get Pixbuf with bitmap RGB content
        // 2 ... get Pixbuf with bitmap alpha mask
        switch( nHandle )
        {
            // sorry, no BitmapEx here...
            case 0:
                aRes <<= reinterpret_cast<sal_Int64>( nullptr );
                break;

            case 1:
            {
                if(!mpBitmap->hasAlpha())
                {
                    HBITMAP aHBmp;
                    mpBitmap->getBitmap()->GetHBITMAP(Gdiplus::Color(), &aHBmp );

                    uno::Sequence< uno::Any > args{ uno::Any(reinterpret_cast<sal_Int64>(aHBmp)) };
                    aRes <<= args;
                }
                else
                {
                    // need to copy&convert the bitmap, since dx
                    // canvas uses inline alpha channel
                    HDC hScreenDC=GetDC(nullptr);
                    const basegfx::B2IVector aSize(mpBitmap->getSize());
                    HBITMAP hBmpBitmap = CreateCompatibleBitmap( hScreenDC,
                                                                 aSize.getX(),
                                                                 aSize.getY() );
                    if( !hBmpBitmap )
                        return aRes;

                    BITMAPINFOHEADER aBIH;

                    aBIH.biSize = sizeof( BITMAPINFOHEADER );
                    aBIH.biWidth = aSize.getX();
                    aBIH.biHeight = -aSize.getY();
                    aBIH.biPlanes = 1;
                    aBIH.biBitCount = 32;
                    aBIH.biCompression = BI_RGB; // expects pixel in
                                                 // bbggrrxx format
                                                 // (little endian)
                    aBIH.biSizeImage = 0;
                    aBIH.biXPelsPerMeter = 0;
                    aBIH.biYPelsPerMeter = 0;
                    aBIH.biClrUsed = 0;
                    aBIH.biClrImportant = 0;

                    Gdiplus::BitmapData aBmpData;
                    aBmpData.Width       = aSize.getX();
                    aBmpData.Height      = aSize.getY();
                    aBmpData.Stride      = 4*aBmpData.Width;
                    aBmpData.PixelFormat = PixelFormat32bppARGB;
                    aBmpData.Scan0       = nullptr;
                    const Gdiplus::Rect aRect( 0,0,aSize.getX(),aSize.getY() );
                    BitmapSharedPtr pGDIPlusBitmap=mpBitmap->getBitmap();
                    if( Gdiplus::Ok != pGDIPlusBitmap->LockBits( &aRect,
                                                                 Gdiplus::ImageLockModeRead,
                                                                 PixelFormat32bppARGB, // outputs ARGB (big endian)
                                                                 &aBmpData ) )
                    {
                        // failed to lock, bail out
                        return aRes;
                    }

                    // now aBmpData.Scan0 contains our bits - push
                    // them into HBITMAP, ignoring alpha
                    SetDIBits( hScreenDC, hBmpBitmap, 0, aSize.getY(), aBmpData.Scan0, reinterpret_cast<PBITMAPINFO>(&aBIH), DIB_RGB_COLORS );

                    pGDIPlusBitmap->UnlockBits( &aBmpData );

                    uno::Sequence< uno::Any > args{ uno::Any(reinterpret_cast<sal_Int64>(hBmpBitmap)) };
                    aRes <<= args;
                }
            }
            break;

            case 2:
            {
                if(!mpBitmap->hasAlpha())
                {
                    return aRes;
                }
                else
                {
                    static AlphaDIB aDIB;

                    // need to copy&convert the bitmap, since dx
                    // canvas uses inline alpha channel
                    HDC hScreenDC=GetDC(nullptr);
                    const basegfx::B2IVector aSize(mpBitmap->getSize());
                    HBITMAP hBmpBitmap = CreateCompatibleBitmap( hScreenDC, aSize.getX(), aSize.getY() );
                    if( !hBmpBitmap )
                        return aRes;

                    aDIB.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
                    aDIB.bmiHeader.biWidth = aSize.getX();
                    aDIB.bmiHeader.biHeight = -aSize.getY();
                    aDIB.bmiHeader.biPlanes = 1;
                    aDIB.bmiHeader.biBitCount = 8;
                    aDIB.bmiHeader.biCompression = BI_RGB;
                    aDIB.bmiHeader.biSizeImage = 0;
                    aDIB.bmiHeader.biXPelsPerMeter = 0;
                    aDIB.bmiHeader.biYPelsPerMeter = 0;
                    aDIB.bmiHeader.biClrUsed = 0;
                    aDIB.bmiHeader.biClrImportant = 0;

                    Gdiplus::BitmapData aBmpData;
                    aBmpData.Width       = aSize.getX();
                    aBmpData.Height      = aSize.getY();
                    aBmpData.Stride      = 4*aBmpData.Width;
                    aBmpData.PixelFormat = PixelFormat32bppARGB;
                    aBmpData.Scan0       = nullptr;
                    const Gdiplus::Rect aRect( 0,0,aSize.getX(),aSize.getY() );
                    BitmapSharedPtr pGDIPlusBitmap=mpBitmap->getBitmap();
                    if( Gdiplus::Ok != pGDIPlusBitmap->LockBits( &aRect,
                                                                 Gdiplus::ImageLockModeRead,
                                                                 PixelFormat32bppARGB, // outputs ARGB (big endian)
                                                                 &aBmpData ) )
                    {
                        // failed to lock, bail out
                        return aRes;
                    }

                    // copy only alpha channel to pAlphaBits
                    const sal_Int32 nScanWidth((aSize.getX() + 3) & ~3);
                    std::unique_ptr<sal_uInt8[]> pAlphaBits( new sal_uInt8[nScanWidth*aSize.getY()] );
                    const sal_uInt8* pInBits=static_cast<sal_uInt8*>(aBmpData.Scan0);
                    pInBits+=3;
                    for( sal_Int32 y=0; y<aSize.getY(); ++y )
                    {
                        sal_uInt8* pOutBits=pAlphaBits.get()+y*nScanWidth;
                        for( sal_Int32 x=0; x<aSize.getX(); ++x )
                        {
                            *pOutBits++ = *pInBits;
                            pInBits += 4;
                        }
                    }

                    pGDIPlusBitmap->UnlockBits( &aBmpData );

                    // set bits to newly create HBITMAP
                    SetDIBits( hScreenDC, hBmpBitmap, 0,
                               aSize.getY(), pAlphaBits.get(),
                               reinterpret_cast<PBITMAPINFO>(&aDIB), DIB_RGB_COLORS );

                    uno::Sequence< uno::Any > args{ uno::Any(reinterpret_cast<sal_Int64>(hBmpBitmap)) };
                    aRes <<= args;
                }
            }
            break;
        }

        return aRes;
    }

    OUString SAL_CALL CanvasBitmap::getImplementationName(  )
    {
        return "DXCanvas.CanvasBitmap";
    }

    sal_Bool SAL_CALL CanvasBitmap::supportsService( const OUString& ServiceName )
    {
        return cppu::supportsService( this, ServiceName );
    }

    uno::Sequence< OUString > SAL_CALL CanvasBitmap::getSupportedServiceNames(  )
    {
        return { "com.sun.star.rendering.CanvasBitmap" };
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
