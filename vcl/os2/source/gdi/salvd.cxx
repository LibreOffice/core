/*************************************************************************
 *
 *  $RCSfile: salvd.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:34 $
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

#include <string.h>

#include <tools/svpm.h>

#define _SV_SALVD_CXX

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

HBITMAP ImplCreateVirDevBitmap( HDC hDC, HPS hPS, long nDX, long nDY,
                                USHORT nBitCount )
{
    if( !nBitCount )
    {
        LONG nDevBitCount;
        DevQueryCaps( hDC, CAPS_COLOR_BITCOUNT, 1, &nDevBitCount );
        nBitCount = nDevBitCount;
    }

    LONG nPlanes;
    DevQueryCaps( hDC, CAPS_COLOR_PLANES, 1, &nPlanes );

    // entsprechende Bitmap zum OutputDevice erzeugen
    HBITMAP hBitmap;
    BITMAPINFOHEADER2 aBitmapInfo;
    memset( &aBitmapInfo, 0, sizeof( BITMAPINFOHEADER2 ) );
    aBitmapInfo.cbFix     = sizeof( BITMAPINFOHEADER2 );
    aBitmapInfo.cx        = nDX;
    aBitmapInfo.cy        = nDY;
    aBitmapInfo.cPlanes   = nPlanes;
    aBitmapInfo.cBitCount = (nBitCount < 4) ? 4 : nBitCount;
    hBitmap  = GpiCreateBitmap( hPS, &aBitmapInfo, 0, NULL, NULL );
    return hBitmap;
}

// -----------------------------------------------------------------------

SalVirtualDevice* SalInstance::CreateVirtualDevice( SalGraphics* pGraphics,
                                                    long nDX, long nDY,
                                                    USHORT nBitCount )
{
    HAB     hAB = GetSalData()->mhAB;
    SIZEL   size;

    // create device context (at this time allways display compatible)
    DEVOPENSTRUC aDevOpenStruc = { NULL, "DISPLAY", NULL, NULL, NULL, NULL, NULL, NULL, NULL };
    HDC hDC = DevOpenDC( hAB, OD_MEMORY, (PSZ)"*", 5, (PDEVOPENDATA)&aDevOpenStruc, 0 );
    if ( !hDC )
        return NULL;

    // create presentation space
    size.cx = nDX;
    size.cy = nDY;
    HPS hPS = GpiCreatePS( hAB, hDC, &size, GPIT_MICRO | GPIA_ASSOC | PU_PELS );
    if ( !hPS )
    {
        DevCloseDC( hDC );
        return NULL;
    }

    // create bitmap for the virtual device
    HBITMAP hBmp = ImplCreateVirDevBitmap( hDC, hPS, nDX, nDY, nBitCount );
    if ( !hBmp )
    {
        GpiDestroyPS( hPS );
        DevCloseDC( hDC );
        return NULL;
    }

    // init data
    SalVirtualDevice*   pVDev               = new SalVirtualDevice;
    SalGraphics*        pVirGraphics        = new SalGraphics;

    pVirGraphics->maGraphicsData.mhDC       = hDC;
    pVirGraphics->maGraphicsData.mhPS       = hPS;
    pVirGraphics->maGraphicsData.mhWnd      = 0;
    pVirGraphics->maGraphicsData.mnHeight   = nDY;
    pVirGraphics->maGraphicsData.mbPrinter  = FALSE;
    pVirGraphics->maGraphicsData.mbVirDev   = TRUE;
    pVirGraphics->maGraphicsData.mbWindow   = FALSE;
    pVirGraphics->maGraphicsData.mbScreen   = pGraphics->maGraphicsData.mbScreen;
    ImplSalInitGraphics( &(pVirGraphics->maGraphicsData) );

    pVDev->maVirDevData.mhDC                = hDC;
    pVDev->maVirDevData.mhPS                = hPS;
    pVDev->maVirDevData.mhBmp               = hBmp;
    pVDev->maVirDevData.mhDefBmp            = GpiSetBitmap( hPS, hBmp );
    pVDev->maVirDevData.mpGraphics          = pVirGraphics;
    pVDev->maVirDevData.mnBitCount          = nBitCount;
    pVDev->maVirDevData.mbGraphics          = FALSE;
    return pVDev;
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
    ImplSalDeInitGraphics( &(maVirDevData.mpGraphics->maGraphicsData) );

    GpiSetBitmap( maVirDevData.mpGraphics->maGraphicsData.mhPS, maVirDevData.mhDefBmp );
    GpiDeleteBitmap( maVirDevData.mhBmp );
    GpiDestroyPS( maVirDevData.mpGraphics->maGraphicsData.mhPS );
    DevCloseDC( maVirDevData.mpGraphics->maGraphicsData.mhDC );
    delete maVirDevData.mpGraphics;
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
    HBITMAP hNewBmp = ImplCreateVirDevBitmap( maVirDevData.mhDC,
                                              maVirDevData.mhPS, nDX, nDY,
                                              maVirDevData.mnBitCount );
    if ( hNewBmp )
    {
        GpiSetBitmap( maVirDevData.mhPS, hNewBmp );
        GpiDeleteBitmap( maVirDevData.mhBmp );
        maVirDevData.mhBmp = hNewBmp;
        maVirDevData.mpGraphics->maGraphicsData.mnHeight  = nDY;
        return TRUE;
    }
    else
        return FALSE;
}
