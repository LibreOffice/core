/*************************************************************************
 *
 *  $RCSfile: salvd.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-18 14:52:27 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SVWIN_H
#include <tools/svwin.h>
#endif

#define _SV_SALVD_CXX

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
                                                       USHORT nBitCount )
{
    WinSalGraphics* pGraphics = static_cast<WinSalGraphics*>(pSGraphics);

    HDC     hDC     = CreateCompatibleDC( pGraphics->mhDC );
    HBITMAP hBmp    = ImplCreateVirDevBitmap( pGraphics->mhDC,
                                              nDX, nDY, nBitCount );

    if ( hDC && hBmp )
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

        pVDev->mhDC     = hDC;
        pVDev->mhBmp        = hBmp;
        pVDev->mhDefBmp = SelectBitmap( hDC, hBmp );
        pVDev->mpGraphics   = pVirGraphics;
        pVDev->mnBitCount   = nBitCount;
        pVDev->mbGraphics   = FALSE;

        // insert VirDev in VirDevList
        pVDev->mpNext = pSalData->mpFirstVD;
        pSalData->mpFirstVD = pVDev;

        return pVDev;
    }
    else
    {
        if ( hDC )
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
}

// -----------------------------------------------------------------------

WinSalVirtualDevice::~WinSalVirtualDevice()
{
    SalData* pSalData = GetSalData();

    // destroy saved DC
    if ( mpGraphics->mhDefPal )
        SelectPalette( mpGraphics->mhDC, mpGraphics->mhDefPal, TRUE );
    ImplSalDeInitGraphics( mpGraphics );
    SelectBitmap( mpGraphics->mhDC, mhDefBmp );
    DeleteDC( mpGraphics->mhDC );
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
        return FALSE;
}
