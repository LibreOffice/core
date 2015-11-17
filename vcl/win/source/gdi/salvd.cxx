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

#include <svsys.h>

#include <vcl/sysdata.hxx>

#include <win/wincomp.hxx>
#include <win/saldata.hxx>
#include <win/salinst.h>
#include <win/salgdi.h>
#include <win/salvd.h>
#include "opengl/win/gdiimpl.hxx"

HBITMAP WinSalVirtualDevice::ImplCreateVirDevBitmap(HDC hDC, long nDX, long nDY, sal_uInt16 nBitCount, void **ppData)
{
    HBITMAP hBitmap;

     if ( nBitCount == 1 )
     {
         hBitmap = CreateBitmap( (int)nDX, (int)nDY, 1, 1, NULL );
         ppData = NULL;
     }
     else
     {
         if (nBitCount == 0)
             nBitCount = (WORD)GetDeviceCaps(hDC, BITSPIXEL);

        // #146839# Don't use CreateCompatibleBitmap() - there seem to
        // be build-in limits for those HBITMAPs, at least this fails
        // rather often on large displays/multi-monitor setups.
         BITMAPINFO aBitmapInfo;
         aBitmapInfo.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
         aBitmapInfo.bmiHeader.biWidth = nDX;
         aBitmapInfo.bmiHeader.biHeight = nDY;
         aBitmapInfo.bmiHeader.biPlanes = 1;
         aBitmapInfo.bmiHeader.biBitCount = nBitCount;
         aBitmapInfo.bmiHeader.biCompression = BI_RGB;
         aBitmapInfo.bmiHeader.biSizeImage = 0;
         aBitmapInfo.bmiHeader.biXPelsPerMeter = 0;
         aBitmapInfo.bmiHeader.biYPelsPerMeter = 0;
         aBitmapInfo.bmiHeader.biClrUsed = 0;
         aBitmapInfo.bmiHeader.biClrImportant = 0;

         hBitmap = CreateDIBSection( hDC, &aBitmapInfo,
                                     DIB_RGB_COLORS, ppData, NULL,
                                     0 );
     }

    return hBitmap;
}

SalVirtualDevice* WinSalInstance::CreateVirtualDevice( SalGraphics* pSGraphics,
                                                       long &nDX, long &nDY,
                                                       DeviceFormat eFormat,
                                                       const SystemGraphicsData* pData )
{
    WinSalGraphics* pGraphics = static_cast<WinSalGraphics*>(pSGraphics);

    sal_uInt16 nBitCount;
    switch (eFormat)
    {
        case DeviceFormat::BITMASK:
            nBitCount = 1;
            break;
        case DeviceFormat::GRAYSCALE:
            nBitCount = 8;
            break;
        default:
            nBitCount = 0;
            break;
    }

    HDC     hDC = NULL;
    HBITMAP hBmp = NULL;
    bool    bOk = FALSE;

    if( pData )
    {
        hDC = (pData->hDC) ? pData->hDC : GetDC(pData->hWnd);
        hBmp = NULL;
        bOk = (hDC != NULL);
        if (bOk)
        {
            nDX = GetDeviceCaps( hDC, HORZRES );
            nDY = GetDeviceCaps( hDC, VERTRES );
        }
        else
        {
            nDX = 0;
            nDY = 0;
        }
    }
    else
    {
        hDC = CreateCompatibleDC( pGraphics->getHDC() );
        if( !hDC )
            ImplWriteLastError( GetLastError(), "CreateCompatibleDC in CreateVirtualDevice" );

        void *pDummy;
        hBmp = WinSalVirtualDevice::ImplCreateVirDevBitmap(pGraphics->getHDC(), nDX, nDY, nBitCount, &pDummy);
        if( !hBmp )
            ImplWriteLastError( GetLastError(), "ImplCreateVirDevBitmap in CreateVirtualDevice" );
        // #124826# continue even if hBmp could not be created
        // if we would return a failure in this case, the process
        // would terminate which is not required

        DBG_ASSERT( hBmp, "WinSalInstance::CreateVirtualDevice(), could not create Bitmap!" );

        bOk = (hDC != NULL);
    }

    if ( bOk )
    {
        WinSalVirtualDevice*    pVDev = new WinSalVirtualDevice;
        SalData*                pSalData = GetSalData();
        WinSalGraphics*         pVirGraphics = new WinSalGraphics(WinSalGraphics::VIRTUAL_DEVICE, pGraphics->isScreen(), 0, pVDev);
        pVirGraphics->SetLayout( SalLayoutFlags::NONE );   // by default no! mirroring for VirtualDevices, can be enabled with EnableRTL()
        pVirGraphics->setHDC(hDC);
        if ( pSalData->mhDitherPal && pVirGraphics->isScreen() )
        {
            pVirGraphics->setDefPal(SelectPalette( hDC, pSalData->mhDitherPal, TRUE ));
            RealizePalette( hDC );
        }
        pVirGraphics->InitGraphics();

        pVDev->mnWidth = nDX;
        pVDev->mnHeight = nDY;
        pVDev->setHDC(hDC);
        pVDev->mhBmp        = hBmp;
        if( hBmp )
            pVDev->mhDefBmp = SelectBitmap( hDC, hBmp );
        else
            pVDev->mhDefBmp = NULL;
        pVDev->mpGraphics   = pVirGraphics;
        pVDev->mnBitCount   = nBitCount;
        pVDev->mbGraphics   = FALSE;
        pVDev->mbForeignDC  = (pData != NULL && pData->hDC != NULL );

        // insert VirDev in VirDevList
        pVDev->mpNext = pSalData->mpFirstVD;
        pSalData->mpFirstVD = pVDev;

        return pVDev;
    }
    else
    {
        if ( hDC && !pData )
            DeleteDC( hDC );
        if ( hBmp )
            DeleteBitmap( hBmp );
        return NULL;
    }
}

WinSalVirtualDevice::WinSalVirtualDevice()
{
    setHDC((HDC)NULL);          // HDC or 0 for Cache Device
    mhBmp = (HBITMAP) NULL;     // Memory Bitmap
    mhDefBmp = (HBITMAP) NULL;  // Default Bitmap
    mpGraphics = NULL;          // current VirDev graphics
    mpNext = NULL;              // next VirDev
    mnBitCount = 0;             // BitCount (0 or 1)
    mbGraphics = FALSE;         // is Graphics used
    mbForeignDC = FALSE;        // uses a foreign DC instead of a bitmap
    mnWidth = 0;
    mnHeight = 0;
}

WinSalVirtualDevice::~WinSalVirtualDevice()
{
    // remove VirDev from list of virtual devices
    SalData* pSalData = GetSalData();
    WinSalVirtualDevice** ppVirDev = &pSalData->mpFirstVD;
    for(; (*ppVirDev != this) && *ppVirDev; ppVirDev = &(*ppVirDev)->mpNext );
    if( *ppVirDev )
        *ppVirDev = mpNext;

    // destroy saved DC
    if( mpGraphics->getDefPal() )
        SelectPalette( mpGraphics->getHDC(), mpGraphics->getDefPal(), TRUE );
    mpGraphics->DeInitGraphics();
    if( mhDefBmp )
        SelectBitmap( mpGraphics->getHDC(), mhDefBmp );
    if( !mbForeignDC )
        DeleteDC( mpGraphics->getHDC() );
    if( mhBmp )
        DeleteBitmap( mhBmp );
    delete mpGraphics;
    mpGraphics = NULL;
}

SalGraphics* WinSalVirtualDevice::AcquireGraphics()
{
    if ( mbGraphics )
        return NULL;

    if ( mpGraphics )
        mbGraphics = TRUE;

    return mpGraphics;
}

void WinSalVirtualDevice::ReleaseGraphics( SalGraphics* )
{
    mbGraphics = FALSE;
}

bool WinSalVirtualDevice::SetSize( long nDX, long nDY )
{
    if( mbForeignDC || !mhBmp )
        return TRUE;    // ???
    else
    {
        void *pDummy;
        HBITMAP hNewBmp = ImplCreateVirDevBitmap(getHDC(), nDX, nDY, mnBitCount, &pDummy);
        if ( hNewBmp )
        {
            mnWidth = nDX;
            mnHeight = nDY;

            SelectBitmap( getHDC(), hNewBmp );
            DeleteBitmap( mhBmp );
            mhBmp = hNewBmp;

            if (mpGraphics)
            {
                WinOpenGLSalGraphicsImpl *pImpl;
                pImpl = dynamic_cast< WinOpenGLSalGraphicsImpl * >(mpGraphics->GetImpl());
                if (pImpl)
                    pImpl->Init();
            }
            return TRUE;
        }
        else
        {
            ImplWriteLastError( GetLastError(), "ImplCreateVirDevBitmap in SetSize" );
            mnWidth = 0;
            mnHeight = 0;
            return FALSE;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
