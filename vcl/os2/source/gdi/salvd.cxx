/*************************************************************************
 *
 *  $RCSfile: salvd.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-20 16:08:11 $
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

#include <svpm.h>

#define _SV_SALVD_CXX

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

#ifndef __H_FT2LIB
#include <wingdi.h>
#include <ft2lib.h>
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

SalVirtualDevice* Os2SalInstance::CreateVirtualDevice( SalGraphics* pSGraphics,
                                                    long nDX, long nDY,
                                                    USHORT nBitCount,
                                                       const SystemGraphicsData* pData )
{
    Os2SalGraphics* pGraphics = static_cast<Os2SalGraphics*>(pSGraphics);
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
    HPS hPS = Ft2CreatePS( hAB, hDC, &size, GPIT_MICRO | GPIA_ASSOC | PU_PELS );
    if ( !hPS )
    {
        DevCloseDC( hDC );
        return NULL;
    }

    // create bitmap for the virtual device
    HBITMAP hBmp = ImplCreateVirDevBitmap( hDC, hPS, nDX, nDY, nBitCount );
    if ( !hBmp )
    {
        Ft2DestroyPS( hPS );
        DevCloseDC( hDC );
        return NULL;
    }

    // init data
    Os2SalVirtualDevice*    pVDev               = new Os2SalVirtualDevice;
    Os2SalGraphics*     pVirGraphics        = new Os2SalGraphics;

    pVirGraphics->mhDC      = hDC;
    pVirGraphics->mhPS      = hPS;
    pVirGraphics->mhWnd     = 0;
    pVirGraphics->mnHeight  = nDY;
    pVirGraphics->mbPrinter = FALSE;
    pVirGraphics->mbVirDev  = TRUE;
    pVirGraphics->mbWindow  = FALSE;
    pVirGraphics->mbScreen  = pGraphics->mbScreen;
    ImplSalInitGraphics( pVirGraphics );

    pVDev->mhDC             = hDC;
    pVDev->mhPS             = hPS;
    pVDev->mhBmp            = hBmp;
    pVDev->mhDefBmp         = Ft2SetBitmap( hPS, hBmp );
    pVDev->mpGraphics       = pVirGraphics;
    pVDev->mnBitCount       = nBitCount;
    pVDev->mbGraphics       = FALSE;
    return pVDev;
}

// -----------------------------------------------------------------------

void Os2SalInstance::DestroyVirtualDevice( SalVirtualDevice* pDevice )
{
    delete pDevice;
}

// =======================================================================

Os2SalVirtualDevice::Os2SalVirtualDevice()
{
}

// -----------------------------------------------------------------------

Os2SalVirtualDevice::~Os2SalVirtualDevice()
{
    ImplSalDeInitGraphics( mpGraphics );

    Ft2SetBitmap( mpGraphics->mhPS, mhDefBmp );
    GpiDeleteBitmap( mhBmp );
    Ft2DestroyPS( mpGraphics->mhPS );
    DevCloseDC( mpGraphics->mhDC );
    delete mpGraphics;
}

// -----------------------------------------------------------------------

SalGraphics* Os2SalVirtualDevice::GetGraphics()
{
    if ( mbGraphics )
        return NULL;

    if ( mpGraphics )
        mbGraphics = TRUE;

    return mpGraphics;
}

// -----------------------------------------------------------------------

void Os2SalVirtualDevice::ReleaseGraphics( SalGraphics* )
{
    mbGraphics = FALSE;
}

// -----------------------------------------------------------------------

BOOL Os2SalVirtualDevice::SetSize( long nDX, long nDY )
{
    HBITMAP hNewBmp = ImplCreateVirDevBitmap( mhDC,
                                              mhPS, nDX, nDY,
                                              mnBitCount );
    if ( hNewBmp )
    {
        Ft2SetBitmap( mhPS, hNewBmp );
        GpiDeleteBitmap( mhBmp );
        mhBmp = hNewBmp;
        mpGraphics->mnHeight  = nDY;
        return TRUE;
    }
    else
        return FALSE;
}

void Os2SalVirtualDevice::GetSize( long& rWidth, long& rHeight )
{
    DevQueryCaps( mpGraphics->mhDC, CAPS_WIDTH, CAPS_WIDTH, (LONG*)rWidth );
    DevQueryCaps( mpGraphics->mhDC, CAPS_HEIGHT, CAPS_HEIGHT, (LONG*)rHeight );
}
