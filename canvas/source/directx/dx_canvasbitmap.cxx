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


#include <ctype.h>
#include <canvas/debug.hxx>
#include <canvas/canvastools.hxx>
#include <cppuhelper/supportsservice.hxx>
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
        
        
        
        switch( nHandle )
        {
            
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
                    aBIH.biCompression = BI_RGB; 
                                                 
                                                 
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
                                                                 PixelFormat32bppARGB, 
                                                                 &aBmpData ) )
                    {
                        
                        return aRes;
                    }

                    
                    
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
                                
                                
                                
#define BOOST_PP_LOCAL_MACRO(n_) \
                    BOOST_PP_COMMA_IF(n_) \
                    {n_,n_,n_,n_}
#define BOOST_PP_LOCAL_LIMITS     (0, 255)
#include BOOST_PP_LOCAL_ITERATE()
                            }
                        };

                    
                    
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
                                                                 PixelFormat32bppARGB, 
                                                                 &aBmpData ) )
                    {
                        
                        return aRes;
                    }

                    
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

    OUString SAL_CALL CanvasBitmap::getImplementationName(  ) throw (uno::RuntimeException)
    {
        return OUString( "DXCanvas.CanvasBitmap" );
    }

    sal_Bool SAL_CALL CanvasBitmap::supportsService( const OUString& ServiceName ) throw (uno::RuntimeException)
    {
        return cppu::supportsService( this, ServiceName );
    }

    uno::Sequence< OUString > SAL_CALL CanvasBitmap::getSupportedServiceNames(  ) throw (uno::RuntimeException)
    {
        uno::Sequence< OUString > aRet(1);
        aRet[0] = "com.sun.star.rendering.CanvasBitmap";

        return aRet;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
