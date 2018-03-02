/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include <string.h>

#include <svpm.h>

#include <tools/svwin.h>

#include <vcl/sysdata.hxx>

#include <os2/saldata.hxx>
#include <os2/salinst.h>
#include <os2/salgdi.h>
#include <os2/salvd.h>

/*
#define _SV_SALVD_CXX
#include <saldata.hxx>
#include <salinst.h>
#include <salgdi.h>
#include <salvd.h>
*/

#ifndef __H_FT2LIB
#include <os2/wingdi.h>
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

sal_Bool Os2SalVirtualDevice::SetSize( long nDX, long nDY )
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
    LONG alData;
    DevQueryCaps( mpGraphics->mhDC, CAPS_WIDTH, 1L, &alData);
    rWidth = alData;
    DevQueryCaps( mpGraphics->mhDC, CAPS_HEIGHT, 1L, &alData);
    rHeight = alData;
}
