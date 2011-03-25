/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"
#include <windows.h>
#include <wincomp.hxx>
#include <saldata.hxx>
#include <salinst.h>
#include <salgdi.h>
#include <salvd.h>
#include <vcl/sysdata.hxx>

// =======================================================================

static HBITMAP ImplCreateVirDevBitmap( HDC hDC, long nDX, long nDY,
                                       sal_uInt16 nBitCount )
{
    HBITMAP hBitmap;

     if ( nBitCount == 1 )
     {
         hBitmap = CreateBitmap( (int)nDX, (int)nDY, 1, 1, NULL );
     }
     else
     {
        // #146839# Don't use CreateCompatibleBitmap() - there seem to
        // be build-in limits for those HBITMAPs, at least this fails
        // rather often on large displays/multi-monitor setups.
         BITMAPINFO aBitmapInfo;
         aBitmapInfo.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
         aBitmapInfo.bmiHeader.biWidth = nDX;
         aBitmapInfo.bmiHeader.biHeight = nDY;
         aBitmapInfo.bmiHeader.biPlanes = 1;
         aBitmapInfo.bmiHeader.biBitCount = (WORD)GetDeviceCaps( hDC,
                                                                 BITSPIXEL );
         aBitmapInfo.bmiHeader.biCompression = BI_RGB;
         aBitmapInfo.bmiHeader.biSizeImage = 0;
         aBitmapInfo.bmiHeader.biXPelsPerMeter = 0;
         aBitmapInfo.bmiHeader.biYPelsPerMeter = 0;
         aBitmapInfo.bmiHeader.biClrUsed = 0;
         aBitmapInfo.bmiHeader.biClrImportant = 0;

         void* pDummy;
         hBitmap = CreateDIBSection( hDC, &aBitmapInfo,
                                     DIB_RGB_COLORS, &pDummy, NULL,
                                     0 );
     }

    return hBitmap;
}

// =======================================================================

SalVirtualDevice* WinSalInstance::CreateVirtualDevice( SalGraphics* pSGraphics,
                                                       long nDX, long nDY,
                                                       sal_uInt16 nBitCount,
                                                       const SystemGraphicsData* pData )
{
    WinSalGraphics* pGraphics = static_cast<WinSalGraphics*>(pSGraphics);

    HDC     hDC = NULL;
    HBITMAP hBmp = NULL;
    sal_Bool    bOk = FALSE;

    if( pData )
    {
        hDC = pData->hDC;
        hBmp = NULL;
        bOk = (hDC != NULL);
    }
    else
    {
        hDC     = CreateCompatibleDC( pGraphics->mhDC );
        if( !hDC )
            ImplWriteLastError( GetLastError(), "CreateCompatibleDC in CreateVirtualDevice" );

        hBmp    = ImplCreateVirDevBitmap( pGraphics->mhDC,
                                        nDX, nDY, nBitCount );
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
        WinSalGraphics*         pVirGraphics = new WinSalGraphics;
        pVirGraphics->SetLayout( 0 );   // by default no! mirroring for VirtualDevices, can be enabled with EnableRTL()
        pVirGraphics->mhDC     = hDC;
        pVirGraphics->mhWnd    = 0;
        pVirGraphics->mbPrinter = FALSE;
        pVirGraphics->mbVirDev  = TRUE;
        pVirGraphics->mbWindow  = FALSE;
        pVirGraphics->mbScreen  = pGraphics->mbScreen;
        if ( pSalData->mhDitherPal && pVirGraphics->mbScreen )
        {
            pVirGraphics->mhDefPal = SelectPalette( hDC, pSalData->mhDitherPal, TRUE );
            RealizePalette( hDC );
        }
        ImplSalInitGraphics( pVirGraphics );

        pVDev->mhDC         = hDC;
        pVDev->mhBmp        = hBmp;
        if( hBmp )
            pVDev->mhDefBmp = SelectBitmap( hDC, hBmp );
        else
            pVDev->mhDefBmp = NULL;
        pVDev->mpGraphics   = pVirGraphics;
        pVDev->mnBitCount   = nBitCount;
        pVDev->mbGraphics   = FALSE;
        pVDev->mbForeignDC  = (pData != NULL);

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

// -----------------------------------------------------------------------

void WinSalInstance::DestroyVirtualDevice( SalVirtualDevice* pDevice )
{
    delete pDevice;
}

// =======================================================================

WinSalVirtualDevice::WinSalVirtualDevice()
{
    mhDC = (HDC) NULL;          // HDC or 0 for Cache Device
    mhBmp = (HBITMAP) NULL;     // Memory Bitmap
    mhDefBmp = (HBITMAP) NULL;  // Default Bitmap
    mpGraphics = NULL;          // current VirDev graphics
    mpNext = NULL;              // next VirDev
    mnBitCount = 0;             // BitCount (0 or 1)
    mbGraphics = FALSE;         // is Graphics used
    mbForeignDC = FALSE;        // uses a foreign DC instead of a bitmap
}

// -----------------------------------------------------------------------

WinSalVirtualDevice::~WinSalVirtualDevice()
{
    // remove VirDev from list of virtual devices
    SalData* pSalData = GetSalData();
    WinSalVirtualDevice** ppVirDev = &pSalData->mpFirstVD;
    for(; (*ppVirDev != this) && *ppVirDev; ppVirDev = &(*ppVirDev)->mpNext );
    if( *ppVirDev )
        *ppVirDev = mpNext;

    // destroy saved DC
    if( mpGraphics->mhDefPal )
        SelectPalette( mpGraphics->mhDC, mpGraphics->mhDefPal, TRUE );
    ImplSalDeInitGraphics( mpGraphics );
    if( mhDefBmp )
        SelectBitmap( mpGraphics->mhDC, mhDefBmp );
    if( !mbForeignDC )
        DeleteDC( mpGraphics->mhDC );
    if( mhBmp )
        DeleteBitmap( mhBmp );
    delete mpGraphics;
    mpGraphics = NULL;
}

// -----------------------------------------------------------------------

SalGraphics* WinSalVirtualDevice::GetGraphics()
{
    if ( mbGraphics )
        return NULL;

    if ( mpGraphics )
        mbGraphics = TRUE;

    return mpGraphics;
}

// -----------------------------------------------------------------------

void WinSalVirtualDevice::ReleaseGraphics( SalGraphics* )
{
    mbGraphics = FALSE;
}

// -----------------------------------------------------------------------

sal_Bool WinSalVirtualDevice::SetSize( long nDX, long nDY )
{
    if( mbForeignDC || !mhBmp )
        return TRUE;    // ???
    else
    {
        HBITMAP hNewBmp = ImplCreateVirDevBitmap( mhDC, nDX, nDY,
                                              mnBitCount );
        if ( hNewBmp )
        {
            SelectBitmap( mhDC, hNewBmp );
            DeleteBitmap( mhBmp );
            mhBmp = hNewBmp;
            return TRUE;
        }
        else
        {
            ImplWriteLastError( GetLastError(), "ImplCreateVirDevBitmap in SetSize" );
            return FALSE;
        }
    }
}

void WinSalVirtualDevice::GetSize( long& rWidth, long& rHeight )
{
    rWidth = GetDeviceCaps( mhDC, HORZRES );
    rHeight= GetDeviceCaps( mhDC, VERTRES );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
