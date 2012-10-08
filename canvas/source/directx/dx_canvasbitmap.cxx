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


#include <ctype.h> // don't ask. msdev breaks otherwise...
#include <canvas/debug.hxx>
#include <canvas/canvastools.hxx>
#include <tools/diagnose_ex.h>

#include <vcl/bitmapex.hxx>

#include <boost/preprocessor/repetition.hpp>
#include <boost/preprocessor/iteration/local.hpp>
#include <boost/scoped_array.hpp>

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

        maCanvasHelper.setDevice( *mpDevice.get() );
        maCanvasHelper.setTarget( mpBitmap );
    }

    void CanvasBitmap::disposeThis()
    {
        mpBitmap.reset();
        mpDevice.clear();

        // forward to parent
        CanvasBitmap_Base::disposeThis();
    }

    struct AlphaDIB
    {
        BITMAPINFOHEADER bmiHeader;
        RGBQUAD          bmiColors[256];
    };

    uno::Any SAL_CALL CanvasBitmap::getFastPropertyValue( sal_Int32 nHandle )  throw (uno::RuntimeException)
    {
        uno::Any aRes;
        // 0 ... get BitmapEx
        // 1 ... get Pixbuf with bitmap RGB content
        // 2 ... get Pixbuf with bitmap alpha mask
        switch( nHandle )
        {
            // sorry, no BitmapEx here...
            case 0:
                aRes = ::com::sun::star::uno::Any( reinterpret_cast<sal_Int64>( (BitmapEx*) NULL ) );
                break;

            case 1:
            {
                if(!mpBitmap->hasAlpha())
                {
                    HBITMAP aHBmp;
                    mpBitmap->getBitmap()->GetHBITMAP(Gdiplus::Color(), &aHBmp );

                    uno::Sequence< uno::Any > args(1);
                    args[0] = uno::Any( sal_Int64(aHBmp) );

                    aRes <<= args;
                }
                else
                {
                    // need to copy&convert the bitmap, since dx
                    // canvas uses inline alpha channel
                    HDC hScreenDC=GetDC(NULL);
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
                    aBmpData.Scan0       = NULL;
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
                    SetDIBits( hScreenDC, hBmpBitmap, 0, aSize.getY(), aBmpData.Scan0, (PBITMAPINFO)&aBIH, DIB_RGB_COLORS );

                    pGDIPlusBitmap->UnlockBits( &aBmpData );

                    uno::Sequence< uno::Any > args(1);
                    args[0] = uno::Any( sal_Int64(hBmpBitmap) );

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
                    static AlphaDIB aDIB=
                        {
                            {0,0,0,1,8,BI_RGB,0,0,0,0,0},
                            {
                                // this here fills palette with grey
                                // level colors, starting from 0,0,0
                                // up to 255,255,255
#define BOOST_PP_LOCAL_MACRO(n_) \
                    BOOST_PP_COMMA_IF(n_) \
                    {n_,n_,n_,n_}
#define BOOST_PP_LOCAL_LIMITS     (0, 255)
#include BOOST_PP_LOCAL_ITERATE()
                            }
                        };

                    // need to copy&convert the bitmap, since dx
                    // canvas uses inline alpha channel
                    HDC hScreenDC=GetDC(NULL);
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
                    aBmpData.Scan0       = NULL;
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
                    boost::scoped_array<sal_uInt8> pAlphaBits( new sal_uInt8[nScanWidth*aSize.getY()] );
                    const sal_uInt8* pInBits=(sal_uInt8*)aBmpData.Scan0;
                    pInBits+=3;
                    sal_uInt8* pOutBits;
                    for( sal_Int32 y=0; y<aSize.getY(); ++y )
                    {
                        pOutBits=pAlphaBits.get()+y*nScanWidth;
                        for( sal_Int32 x=0; x<aSize.getX(); ++x )
                        {
                            *pOutBits++ = 255-*pInBits;
                            pInBits += 4;
                        }
                    }

                    pGDIPlusBitmap->UnlockBits( &aBmpData );

                    // set bits to newly create HBITMAP
                    SetDIBits( hScreenDC, hBmpBitmap, 0,
                               aSize.getY(), pAlphaBits.get(),
                               (PBITMAPINFO)&aDIB, DIB_RGB_COLORS );

                    uno::Sequence< uno::Any > args(1);
                    args[0] = uno::Any( sal_Int64(hBmpBitmap) );

                    aRes <<= args;
                }
            }
            break;
        }

        return aRes;
    }

#define IMPLEMENTATION_NAME "DXCanvas.CanvasBitmap"
#define SERVICE_NAME "com.sun.star.rendering.CanvasBitmap"

    ::rtl::OUString SAL_CALL CanvasBitmap::getImplementationName(  ) throw (uno::RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLEMENTATION_NAME ) );
    }

    sal_Bool SAL_CALL CanvasBitmap::supportsService( const ::rtl::OUString& ServiceName ) throw (uno::RuntimeException)
    {
        return ServiceName == SERVICE_NAME;
    }

    uno::Sequence< ::rtl::OUString > SAL_CALL CanvasBitmap::getSupportedServiceNames(  ) throw (uno::RuntimeException)
    {
        uno::Sequence< ::rtl::OUString > aRet(1);
        aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );

        return aRet;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
