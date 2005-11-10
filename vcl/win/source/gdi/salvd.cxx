/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salvd.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-11-10 15:50:03 $
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

#ifndef _SVWIN_H
#include <tools/svwin.h>
#endif

#ifndef _SV_WINCOMP_HXX
#include <wincomp.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALINST_H
#include <salinst.h>
#endif
#ifndef _SV_SALGDI_H
#include <salgdi.h>
#endif
#ifndef _SV_SALVD_H
#include <salvd.h>
#endif
#ifndef _SV_SYSDATA_HXX
#include <sysdata.hxx>
#endif

// =======================================================================

static HBITMAP ImplCreateVirDevBitmap( HDC hDC, long nDX, long nDY,
                                       USHORT nBitCount )
{
    HBITMAP hBitmap;

    if ( nBitCount == 1 )
        hBitmap = CreateBitmap( (int)nDX, (int)nDY, 1, 1, NULL );
    else
        hBitmap = CreateCompatibleBitmap( hDC, (int)nDX, (int)nDY );

    return hBitmap;
}

// =======================================================================

SalVirtualDevice* WinSalInstance::CreateVirtualDevice( SalGraphics* pSGraphics,
                                                       long nDX, long nDY,
                                                       USHORT nBitCount,
                                                       const SystemGraphicsData* pData )
{
    WinSalGraphics* pGraphics = static_cast<WinSalGraphics*>(pSGraphics);

    HDC     hDC = NULL;
    HBITMAP hBmp = NULL;
    BOOL    bOk = FALSE;

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
    SalData* pSalData = GetSalData();

    // destroy saved DC
    if ( mpGraphics->mhDefPal )
        SelectPalette( mpGraphics->mhDC, mpGraphics->mhDefPal, TRUE );
    ImplSalDeInitGraphics( mpGraphics );
    if( mhDefBmp )
        SelectBitmap( mpGraphics->mhDC, mhDefBmp );
    if( !mbForeignDC )
        DeleteDC( mpGraphics->mhDC );
    if( mhBmp )
        DeleteBitmap( mhBmp );
    delete mpGraphics;

    // remove VirDev from VirDevList
    if ( this == pSalData->mpFirstVD )
        pSalData->mpFirstVD = mpNext;
    else
    {
        WinSalVirtualDevice* pTempVD = pSalData->mpFirstVD;
        while ( pTempVD->mpNext != this )
            pTempVD = pTempVD->mpNext;

        pTempVD->mpNext = mpNext;
    }
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

BOOL WinSalVirtualDevice::SetSize( long nDX, long nDY )
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
