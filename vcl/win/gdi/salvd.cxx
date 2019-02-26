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

#include <comphelper/windowserrorstring.hxx>

#include <vcl/sysdata.hxx>

#include <win/wincomp.hxx>
#include <win/saldata.hxx>
#include <win/salinst.h>
#include <win/salgdi.h>
#include <win/salvd.h>
#include <opengl/win/gdiimpl.hxx>
#include <sal/log.hxx>
#include <o3tl/temporary.hxx>

HBITMAP WinSalVirtualDevice::ImplCreateVirDevBitmap(HDC hDC, long nDX, long nDY, sal_uInt16 nBitCount, void **ppData)
{
    HBITMAP hBitmap;

    if ( nBitCount == 1 )
    {
        hBitmap = CreateBitmap( static_cast<int>(nDX), static_cast<int>(nDY), 1, 1, nullptr );
        SAL_WARN_IF( !hBitmap, "vcl", "CreateBitmap failed: " << WindowsErrorString( GetLastError() ) );
        ppData = nullptr;
    }
    else
    {
        if (nBitCount == 0)
            nBitCount = static_cast<WORD>(GetDeviceCaps(hDC, BITSPIXEL));

        // #146839# Don't use CreateCompatibleBitmap() - there seem to
        // be built-in limits for those HBITMAPs, at least this fails
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
                                    DIB_RGB_COLORS, ppData, nullptr,
                                    0 );
        SAL_WARN_IF( !hBitmap, "vcl", "CreateDIBSection failed: " << WindowsErrorString( GetLastError() ) );
    }

    return hBitmap;
}

std::unique_ptr<SalVirtualDevice> WinSalInstance::CreateVirtualDevice( SalGraphics* pSGraphics,
                                                       long &nDX, long &nDY,
                                                       DeviceFormat eFormat,
                                                       const SystemGraphicsData* pData )
{
    WinSalGraphics* pGraphics = static_cast<WinSalGraphics*>(pSGraphics);
    HDC hDC = nullptr;

    if( pData )
    {
        hDC = (pData->hDC) ? pData->hDC : GetDC(pData->hWnd);
        if (hDC)
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
        SAL_WARN_IF( !hDC, "vcl", "CreateCompatibleDC failed: " << WindowsErrorString( GetLastError() ) );
    }

    if (!hDC)
        return nullptr;

    sal_uInt16 nBitCount = (eFormat == DeviceFormat::BITMASK) ? 1 : 0;

    HBITMAP hBmp = nullptr;
    if (!pData)
    {
        // #124826# continue even if hBmp could not be created
        // if we would return a failure in this case, the process
        // would terminate which is not required
        hBmp = WinSalVirtualDevice::ImplCreateVirDevBitmap(pGraphics->getHDC(),
                                                           nDX, nDY, nBitCount,
                                                           &o3tl::temporary<void*>(nullptr));
    }

    const bool bForeignDC = pData != nullptr && pData->hDC != nullptr;
    const SalData* pSalData = GetSalData();

    WinSalVirtualDevice* pVDev = new WinSalVirtualDevice(hDC, hBmp, nBitCount,
                                                         bForeignDC, nDX, nDY);

    WinSalGraphics* pVirGraphics = new WinSalGraphics(WinSalGraphics::VIRTUAL_DEVICE,
                                                      pGraphics->isScreen(), nullptr, pVDev);

    // by default no! mirroring for VirtualDevices, can be enabled with EnableRTL()
    pVirGraphics->SetLayout( SalLayoutFlags::NONE );
    pVirGraphics->setHDC(hDC);

    if ( pSalData->mhDitherPal && pVirGraphics->isScreen() )
    {
        pVirGraphics->setDefPal(SelectPalette( hDC, pSalData->mhDitherPal, TRUE ));
        RealizePalette( hDC );
    }

    pVirGraphics->InitGraphics();
    pVDev->setGraphics(pVirGraphics);

    return std::unique_ptr<SalVirtualDevice>(pVDev);
}

WinSalVirtualDevice::WinSalVirtualDevice(HDC hDC, HBITMAP hBMP, sal_uInt16 nBitCount, bool bForeignDC, long nWidth, long nHeight)
    : mhLocalDC(hDC),          // HDC or 0 for Cache Device
      mhBmp(hBMP),             // Memory Bitmap
      mnBitCount(nBitCount),   // BitCount (0 or 1)
      mbGraphics(false),       // is Graphics used
      mbForeignDC(bForeignDC), // uses a foreign DC instead of a bitmap
      mnWidth(nWidth),
      mnHeight(nHeight)
{
    // Default Bitmap
    if (hBMP)
        mhDefBmp = SelectBitmap(hDC, hBMP);
    else
        mhDefBmp = nullptr;

    // insert VirDev into list of virtual devices
    SalData* pSalData = GetSalData();
    mpNext = pSalData->mpFirstVD;
    pSalData->mpFirstVD = this;
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
}

SalGraphics* WinSalVirtualDevice::AcquireGraphics()
{
    if ( mbGraphics )
        return nullptr;

    if ( mpGraphics )
        mbGraphics = true;

    return mpGraphics.get();
}

void WinSalVirtualDevice::ReleaseGraphics( SalGraphics* )
{
    mbGraphics = false;
}

bool WinSalVirtualDevice::SetSize( long nDX, long nDY )
{
    if( mbForeignDC || !mhBmp )
        return true;    // ???

    HBITMAP hNewBmp = ImplCreateVirDevBitmap(getHDC(), nDX, nDY, mnBitCount,
                                             &o3tl::temporary<void*>(nullptr));
    if (!hNewBmp)
    {
        mnWidth = 0;
        mnHeight = 0;
        return false;
    }

    mnWidth = nDX;
    mnHeight = nDY;

    SelectBitmap(getHDC(), hNewBmp);
    mhBmp.reset(hNewBmp);

    if (mpGraphics)
    {
        WinOpenGLSalGraphicsImpl *pImpl;
        pImpl = dynamic_cast< WinOpenGLSalGraphicsImpl * >(mpGraphics->GetImpl());
        if (pImpl)
            pImpl->Init();
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
