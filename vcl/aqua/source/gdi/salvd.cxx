/*************************************************************************
 *
 *  $RCSfile: salvd.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pluby $ $Date: 2000-11-01 03:12:45 $
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

#ifndef _SV_SALAQUA_HXX
#include <salaqua.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_SALVD_HXX
#include <salvd.hxx>
#endif

// =======================================================================

static HBITMAP ImplCreateVirDevBitmap( VCLVIEW hDC, long nDX, long nDY,
                                       USHORT nBitCount )
{
    HBITMAP hBitmap;

#ifdef WIN
    if ( nBitCount == 1 )
        hBitmap = CreateBitmap( (int)nDX, (int)nDY, 1, 1, NULL );
    else
        hBitmap = CreateCompatibleBitmap( hDC, (int)nDX, (int)nDY );
#endif

    return hBitmap;
}

// =======================================================================

SalVirtualDevice* SalInstance::CreateVirtualDevice( SalGraphics* pGraphics,
                                                    long nDX, long nDY,
                                                    USHORT nBitCount )
{
#ifdef WIN
    VCLVIEW     hDC     = CreateCompatibleDC( pGraphics->maGraphicsData.mhDC );
    HBITMAP hBmp    = ImplCreateVirDevBitmap( pGraphics->maGraphicsData.mhDC,
                                              nDX, nDY, nBitCount );

    if ( hDC && hBmp )
    {
        SalVirtualDevice*   pVDev = new SalVirtualDevice;
        SalData*            pSalData = GetSalData();
        SalGraphics*        pVirGraphics = new SalGraphics;
        pVirGraphics->maGraphicsData.mhDC      = hDC;
        pVirGraphics->maGraphicsData.mhWnd     = 0;
        pVirGraphics->maGraphicsData.mbPrinter = FALSE;
        pVirGraphics->maGraphicsData.mbVirDev  = TRUE;
        pVirGraphics->maGraphicsData.mbWindow  = FALSE;
        pVirGraphics->maGraphicsData.mbScreen  = pGraphics->maGraphicsData.mbScreen;
        if ( pSalData->mhDitherPal && pVirGraphics->maGraphicsData.mbScreen )
        {
            pVirGraphics->maGraphicsData.mhDefPal = SelectPalette( hDC, pSalData->mhDitherPal, TRUE );
            RealizePalette( hDC );
        }
        ImplSalInitGraphics( &(pVirGraphics->maGraphicsData) );

        pVDev->maVirDevData.mhDC        = hDC;
        pVDev->maVirDevData.mhBmp       = hBmp;
        pVDev->maVirDevData.mhDefBmp    = SelectBitmap( hDC, hBmp );
        pVDev->maVirDevData.mpGraphics  = pVirGraphics;
        pVDev->maVirDevData.mnBitCount  = nBitCount;
        pVDev->maVirDevData.mbGraphics  = FALSE;

        // insert VirDev in VirDevList
        pVDev->maVirDevData.mpNext = pSalData->mpFirstVD;
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
#endif
}

// -----------------------------------------------------------------------

void SalInstance::DestroyVirtualDevice( SalVirtualDevice* pDevice )
{
    delete pDevice;
}

// =======================================================================

SalVirtualDevice::SalVirtualDevice()
{
}

// -----------------------------------------------------------------------

SalVirtualDevice::~SalVirtualDevice()
{
    SalData* pSalData = GetSalData();

#ifdef WIN
    // destroy saved DC
    if ( maVirDevData.mpGraphics->maGraphicsData.mhDefPal )
        SelectPalette( maVirDevData.mpGraphics->maGraphicsData.mhDC, maVirDevData.mpGraphics->maGraphicsData.mhDefPal, TRUE );
#endif
    ImplSalDeInitGraphics( &(maVirDevData.mpGraphics->maGraphicsData) );
    SelectBitmap( maVirDevData.mpGraphics->maGraphicsData.mhDC, maVirDevData.mhDefBmp );
#ifdef WIN
    DeleteDC( maVirDevData.mpGraphics->maGraphicsData.mhDC );
#endif
    DeleteBitmap( maVirDevData.mhBmp );
    delete maVirDevData.mpGraphics;

    // remove VirDev from VirDevList
    if ( this == pSalData->mpFirstVD )
        pSalData->mpFirstVD = maVirDevData.mpNext;
    else
    {
        SalVirtualDevice* pTempVD = pSalData->mpFirstVD;
        while ( pTempVD->maVirDevData.mpNext != this )
            pTempVD = pTempVD->maVirDevData.mpNext;

        pTempVD->maVirDevData.mpNext = maVirDevData.mpNext;
    }
}

// -----------------------------------------------------------------------

SalGraphics* SalVirtualDevice::GetGraphics()
{
    if ( maVirDevData.mbGraphics )
        return NULL;

    if ( maVirDevData.mpGraphics )
        maVirDevData.mbGraphics = TRUE;

    return maVirDevData.mpGraphics;
}

// -----------------------------------------------------------------------

void SalVirtualDevice::ReleaseGraphics( SalGraphics* )
{
    maVirDevData.mbGraphics = FALSE;
}

// -----------------------------------------------------------------------

BOOL SalVirtualDevice::SetSize( long nDX, long nDY )
{
    HBITMAP hNewBmp = ImplCreateVirDevBitmap( maVirDevData.mhDC, nDX, nDY,
                                              maVirDevData.mnBitCount );
    if ( hNewBmp )
    {
        SelectBitmap( maVirDevData.mhDC, hNewBmp );
        DeleteBitmap( maVirDevData.mhBmp );
        maVirDevData.mhBmp = hNewBmp;
        return TRUE;
    }
    else
        return FALSE;
}
