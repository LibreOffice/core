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

#include <stdio.h>
#include <string.h>
#include <svsys.h>
#include <rtl/strbuf.hxx>
#include <tools/debug.hxx>
#include <tools/poly.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <win/wincomp.hxx>
#include <win/saldata.hxx>
#include <win/salgdi.h>
#include <win/salframe.h>
#include <win/salvd.h>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include "salgdiimpl.hxx"
#include "gdiimpl.hxx"
#include "opengl/win/gdiimpl.hxx"
#include <config_cairo_canvas.h>
#if ENABLE_CAIRO_CANVAS
#include "cairo_win32_cairo.cxx"
#endif

#include <vcl/opengl/OpenGLHelper.hxx>


#define DITHER_PAL_DELTA                51
#define DITHER_PAL_STEPS                6
#define DITHER_PAL_COUNT                (DITHER_PAL_STEPS*DITHER_PAL_STEPS*DITHER_PAL_STEPS)
#define DITHER_MAX_SYSCOLOR             16
#define DITHER_EXTRA_COLORS             1
#define DMAP( _def_nVal, _def_nThres )  ((pDitherDiff[_def_nVal]>(_def_nThres))?pDitherHigh[_def_nVal]:pDitherLow[_def_nVal])

struct SysColorEntry
{
    DWORD           nRGB;
    SysColorEntry*  pNext;
};

static SysColorEntry* pFirstSysColor = NULL;
static SysColorEntry* pActSysColor = NULL;

// Blue7
static PALETTEENTRY aImplExtraColor1 =
{
    0, 184, 255, 0
};

static PALETTEENTRY aImplSalSysPalEntryAry[ DITHER_MAX_SYSCOLOR ] =
{
{    0,    0,    0, 0 },
{    0,    0, 0x80, 0 },
{    0, 0x80,    0, 0 },
{    0, 0x80, 0x80, 0 },
{ 0x80,    0,    0, 0 },
{ 0x80,    0, 0x80, 0 },
{ 0x80, 0x80,    0, 0 },
{ 0x80, 0x80, 0x80, 0 },
{ 0xC0, 0xC0, 0xC0, 0 },
{    0,    0, 0xFF, 0 },
{    0, 0xFF,    0, 0 },
{    0, 0xFF, 0xFF, 0 },
{ 0xFF,    0,    0, 0 },
{ 0xFF,    0, 0xFF, 0 },
{ 0xFF, 0xFF,    0, 0 },
{ 0xFF, 0xFF, 0xFF, 0 }
};

static BYTE aOrdDither8Bit[8][8] =
{
   {  0, 38,  9, 48,  2, 40, 12, 50 },
   { 25, 12, 35, 22, 28, 15, 37, 24 },
   {  6, 44,  3, 41,  8, 47,  5, 44 },
   { 32, 19, 28, 16, 34, 21, 31, 18 },
   {  1, 40, 11, 49,  0, 39, 10, 48 },
   { 27, 14, 36, 24, 26, 13, 36, 23 },
   {  8, 46,  4, 43,  7, 45,  4, 42 },
   { 33, 20, 30, 17, 32, 20, 29, 16 }
};

static BYTE aOrdDither16Bit[8][8] =
{
   { 0, 6, 1, 7, 0, 6, 1, 7 },
   { 4, 2, 5, 3, 4, 2, 5, 3 },
   { 1, 7, 0, 6, 1, 7, 0, 6 },
   { 5, 3, 4, 2, 5, 3, 4, 2 },
   { 0, 6, 1, 7, 0, 6, 1, 7 },
   { 4, 2, 5, 3, 4, 2, 5, 3 },
   { 1, 7, 0, 6, 1, 7, 0, 6 },
   { 5, 3, 4, 2, 5, 3, 4, 2 }
};

// we must create pens with 1-pixel width; otherwise the S3-graphics card
// map has many paint problems when drawing polygons/polyLines and a
// complex is set
#define GSL_PEN_WIDTH                   1

void ImplInitSalGDI()
{
    SalData* pSalData = GetSalData();

    pSalData->mbResourcesAlreadyFreed = false;

    // init stock brushes
    pSalData->maStockPenColorAry[0]     = PALETTERGB( 0, 0, 0 );
    pSalData->maStockPenColorAry[1]     = PALETTERGB( 0xFF, 0xFF, 0xFF );
    pSalData->maStockPenColorAry[2]     = PALETTERGB( 0xC0, 0xC0, 0xC0 );
    pSalData->maStockPenColorAry[3]     = PALETTERGB( 0x80, 0x80, 0x80 );
    pSalData->mhStockPenAry[0]          = CreatePen( PS_SOLID, GSL_PEN_WIDTH, pSalData->maStockPenColorAry[0] );
    pSalData->mhStockPenAry[1]          = CreatePen( PS_SOLID, GSL_PEN_WIDTH, pSalData->maStockPenColorAry[1] );
    pSalData->mhStockPenAry[2]          = CreatePen( PS_SOLID, GSL_PEN_WIDTH, pSalData->maStockPenColorAry[2] );
    pSalData->mhStockPenAry[3]          = CreatePen( PS_SOLID, GSL_PEN_WIDTH, pSalData->maStockPenColorAry[3] );
    pSalData->mnStockPenCount = 4;

    pSalData->maStockBrushColorAry[0]   = PALETTERGB( 0, 0, 0 );
    pSalData->maStockBrushColorAry[1]   = PALETTERGB( 0xFF, 0xFF, 0xFF );
    pSalData->maStockBrushColorAry[2]   = PALETTERGB( 0xC0, 0xC0, 0xC0 );
    pSalData->maStockBrushColorAry[3]   = PALETTERGB( 0x80, 0x80, 0x80 );
    pSalData->mhStockBrushAry[0]        = CreateSolidBrush( pSalData->maStockBrushColorAry[0] );
    pSalData->mhStockBrushAry[1]        = CreateSolidBrush( pSalData->maStockBrushColorAry[1] );
    pSalData->mhStockBrushAry[2]        = CreateSolidBrush( pSalData->maStockBrushColorAry[2] );
    pSalData->mhStockBrushAry[3]        = CreateSolidBrush( pSalData->maStockBrushColorAry[3] );
    pSalData->mnStockBrushCount = 4;

    // initialize cache of device contexts
    pSalData->mpHDCCache = new HDCCache[ CACHESIZE_HDC ];
    memset( pSalData->mpHDCCache, 0, CACHESIZE_HDC * sizeof( HDCCache ) );

    // initialize temporary font list
    pSalData->mpTempFontItem = NULL;

    // support palettes for 256 color displays
    HDC hDC = GetDC( 0 );
    int nBitsPixel = GetDeviceCaps( hDC, BITSPIXEL );
    int nPlanes = GetDeviceCaps( hDC, PLANES );
    int nRasterCaps = GetDeviceCaps( hDC, RASTERCAPS );
    int nBitCount = nBitsPixel * nPlanes;

    if ( (nBitCount > 8) && (nBitCount < 24) )
    {
        // test if we have to dither
        HDC         hMemDC = ::CreateCompatibleDC( hDC );
        HBITMAP     hMemBmp = ::CreateCompatibleBitmap( hDC, 8, 8 );
        HBITMAP     hBmpOld = (HBITMAP) ::SelectObject( hMemDC, hMemBmp );
        HBRUSH      hMemBrush = ::CreateSolidBrush( PALETTERGB( 175, 171, 169 ) );
        HBRUSH      hBrushOld = (HBRUSH) ::SelectObject( hMemDC, hMemBrush );
        bool        bDither16 = TRUE;

        ::PatBlt( hMemDC, 0, 0, 8, 8, PATCOPY );
        const COLORREF aCol( ::GetPixel( hMemDC, 0, 0 ) );

        for( int nY = 0; ( nY < 8 ) && bDither16; nY++ )
            for( int nX = 0; ( nX < 8 ) && bDither16; nX++ )
                if( ::GetPixel( hMemDC, nX, nY ) != aCol )
                    bDither16 = FALSE;

        ::SelectObject( hMemDC, hBrushOld ), ::DeleteObject( hMemBrush );
        ::SelectObject( hMemDC, hBmpOld ), ::DeleteObject( hMemBmp );
        ::DeleteDC( hMemDC );

        if( bDither16 )
        {
            // create DIBPattern for 16Bit dithering
            long n;

            pSalData->mhDitherDIB = GlobalAlloc( GMEM_FIXED, sizeof( BITMAPINFOHEADER ) + 192 );
            pSalData->mpDitherDIB = (BYTE*) GlobalLock( pSalData->mhDitherDIB );
            pSalData->mpDitherDiff = new long[ 256 ];
            pSalData->mpDitherLow = new BYTE[ 256 ];
            pSalData->mpDitherHigh = new BYTE[ 256 ];
            pSalData->mpDitherDIBData = pSalData->mpDitherDIB + sizeof( BITMAPINFOHEADER );
            memset( pSalData->mpDitherDIB, 0, sizeof( BITMAPINFOHEADER ) );

            BITMAPINFOHEADER* pBIH = (BITMAPINFOHEADER*) pSalData->mpDitherDIB;

            pBIH->biSize = sizeof( BITMAPINFOHEADER );
            pBIH->biWidth = 8;
            pBIH->biHeight = 8;
            pBIH->biPlanes = 1;
            pBIH->biBitCount = 24;

            for( n = 0; n < 256L; n++ )
                pSalData->mpDitherDiff[ n ] = n - ( n & 248L );

            for( n = 0; n < 256L; n++ )
                pSalData->mpDitherLow[ n ] = (BYTE) ( n & 248 );

            for( n = 0; n < 256L; n++ )
                pSalData->mpDitherHigh[ n ] = (BYTE) std::min( pSalData->mpDitherLow[ n ] + 8L, 255L );
        }
    }
    else if ( (nRasterCaps & RC_PALETTE) && (nBitCount == 8) )
    {
        BYTE            nRed, nGreen, nBlue;
        BYTE            nR, nG, nB;
        PALETTEENTRY*   pPalEntry;
        LOGPALETTE*     pLogPal;
        const sal_uInt16    nDitherPalCount = DITHER_PAL_COUNT;
        sal_uLong           nTotalCount = DITHER_MAX_SYSCOLOR + nDitherPalCount + DITHER_EXTRA_COLORS;

        // create logical palette
        pLogPal = (LOGPALETTE*) new char[ sizeof( LOGPALETTE ) + ( nTotalCount * sizeof( PALETTEENTRY ) ) ];
        pLogPal->palVersion = 0x0300;
        pLogPal->palNumEntries = (sal_uInt16) nTotalCount;
        pPalEntry = pLogPal->palPalEntry;

        // Standard colors
        memcpy( pPalEntry, aImplSalSysPalEntryAry, DITHER_MAX_SYSCOLOR * sizeof( PALETTEENTRY ) );
        pPalEntry += DITHER_MAX_SYSCOLOR;

        // own palette (6/6/6)
        for( nB=0, nBlue=0; nB < DITHER_PAL_STEPS; nB++, nBlue += DITHER_PAL_DELTA )
        {
            for( nG=0, nGreen=0; nG < DITHER_PAL_STEPS; nG++, nGreen += DITHER_PAL_DELTA )
            {
                for( nR=0, nRed=0; nR < DITHER_PAL_STEPS; nR++, nRed += DITHER_PAL_DELTA )
                {
                    pPalEntry->peRed   = nRed;
                    pPalEntry->peGreen = nGreen;
                    pPalEntry->peBlue  = nBlue;
                    pPalEntry->peFlags = 0;
                    pPalEntry++;
                }
            }
        }

        // insert special 'Blue' as standard drawing color
        *pPalEntry++ = aImplExtraColor1;

        // create palette
        pSalData->mhDitherPal = CreatePalette( pLogPal );
        delete[] (char*) pLogPal;

        if( pSalData->mhDitherPal )
        {
            // create DIBPattern for 8Bit dithering
            long nSize = sizeof( BITMAPINFOHEADER ) + ( 256 * sizeof( short ) ) + 64;
            long n;

            pSalData->mhDitherDIB = GlobalAlloc( GMEM_FIXED, nSize );
            pSalData->mpDitherDIB = (BYTE*) GlobalLock( pSalData->mhDitherDIB );
            pSalData->mpDitherDiff = new long[ 256 ];
            pSalData->mpDitherLow = new BYTE[ 256 ];
            pSalData->mpDitherHigh = new BYTE[ 256 ];
            pSalData->mpDitherDIBData = pSalData->mpDitherDIB + sizeof( BITMAPINFOHEADER ) + ( 256 * sizeof( short ) );
            memset( pSalData->mpDitherDIB, 0, sizeof( BITMAPINFOHEADER ) );

            BITMAPINFOHEADER*   pBIH = (BITMAPINFOHEADER*) pSalData->mpDitherDIB;
            short*              pColors = (short*) ( pSalData->mpDitherDIB + sizeof( BITMAPINFOHEADER ) );

            pBIH->biSize = sizeof( BITMAPINFOHEADER );
            pBIH->biWidth = 8;
            pBIH->biHeight = 8;
            pBIH->biPlanes = 1;
            pBIH->biBitCount = 8;

            for( n = 0; n < nDitherPalCount; n++ )
                pColors[ n ] = (short)( n + DITHER_MAX_SYSCOLOR );

            for( n = 0; n < 256L; n++ )
                pSalData->mpDitherDiff[ n ] = n % 51L;

            for( n = 0; n < 256L; n++ )
                pSalData->mpDitherLow[ n ] = (BYTE) ( n / 51L );

            for( n = 0; n < 256L; n++ )
                pSalData->mpDitherHigh[ n ] = (BYTE)std::min( pSalData->mpDitherLow[ n ] + 1, 5 );
        }

        // get system color entries
        ImplUpdateSysColorEntries();
    }

    ReleaseDC( 0, hDC );
}

void ImplFreeSalGDI()
{
    SalData*    pSalData = GetSalData();

    if (pSalData->mbResourcesAlreadyFreed)
        return;

    // destroy stock objects
    int i;
    for ( i = 0; i < pSalData->mnStockPenCount; i++ )
        DeletePen( pSalData->mhStockPenAry[i] );
    for ( i = 0; i < pSalData->mnStockBrushCount; i++ )
        DeleteBrush( pSalData->mhStockBrushAry[i] );

    // delete 50% Brush
    if ( pSalData->mh50Brush )
    {
        DeleteBrush( pSalData->mh50Brush );
        pSalData->mh50Brush = 0;
    }

    // delete 50% Bitmap
    if ( pSalData->mh50Bmp )
    {
        DeleteBitmap( pSalData->mh50Bmp );
        pSalData->mh50Bmp = 0;
    }

    ImplClearHDCCache( pSalData );
    delete[] pSalData->mpHDCCache;

    // delete Ditherpalette, if existing
    if ( pSalData->mhDitherPal )
    {
        DeleteObject( pSalData->mhDitherPal );
        pSalData->mhDitherPal = 0;
    }

    // delete buffers for dithering DIB patterns, if necessary
    if ( pSalData->mhDitherDIB )
    {
        GlobalUnlock( pSalData->mhDitherDIB );
        GlobalFree( pSalData->mhDitherDIB );
        pSalData->mhDitherDIB = 0;
        delete[] pSalData->mpDitherDiff;
        delete[] pSalData->mpDitherLow;
        delete[] pSalData->mpDitherHigh;
    }

    // delete SysColorList
    SysColorEntry* pEntry = pFirstSysColor;
    while( pEntry )
    {
        SysColorEntry* pTmp = pEntry->pNext;
        delete pEntry;
        pEntry = pTmp;
    }
    pFirstSysColor = NULL;

    // delete icon cache
    SalIcon* pIcon = pSalData->mpFirstIcon;
    pSalData->mpFirstIcon = NULL;
    while( pIcon )
    {
        SalIcon* pTmp = pIcon->pNext;
        DestroyIcon( pIcon->hIcon );
        DestroyIcon( pIcon->hSmallIcon );
        delete pIcon;
        pIcon = pTmp;
    }

    // delete temporary font list
    ImplReleaseTempFonts( *pSalData );

    pSalData->mbResourcesAlreadyFreed = true;
}

int ImplIsSysColorEntry( SalColor nSalColor )
{
    SysColorEntry*  pEntry = pFirstSysColor;
    const DWORD     nTestRGB = (DWORD)RGB( SALCOLOR_RED( nSalColor ),
                                           SALCOLOR_GREEN( nSalColor ),
                                           SALCOLOR_BLUE( nSalColor ) );

    while ( pEntry )
    {
        if ( pEntry->nRGB == nTestRGB )
            return TRUE;
        pEntry = pEntry->pNext;
    }

    return FALSE;
}

static int ImplIsPaletteEntry( BYTE nRed, BYTE nGreen, BYTE nBlue )
{
    // dither color?
    if ( !(nRed % DITHER_PAL_DELTA) && !(nGreen % DITHER_PAL_DELTA) && !(nBlue % DITHER_PAL_DELTA) )
        return TRUE;

    PALETTEENTRY* pPalEntry = aImplSalSysPalEntryAry;

    // standard palette color?
    for ( sal_uInt16 i = 0; i < DITHER_MAX_SYSCOLOR; i++, pPalEntry++ )
    {
        if( pPalEntry->peRed == nRed && pPalEntry->peGreen == nGreen && pPalEntry->peBlue == nBlue )
            return TRUE;
    }

    // extra color?
    if ( aImplExtraColor1.peRed == nRed &&
         aImplExtraColor1.peGreen == nGreen &&
         aImplExtraColor1.peBlue == nBlue )
    {
        return TRUE;
    }

    return FALSE;
}

static void ImplInsertSysColorEntry( int nSysIndex )
{
    const DWORD nRGB = GetSysColor( nSysIndex );

    if ( !ImplIsPaletteEntry( GetRValue( nRGB ), GetGValue( nRGB ), GetBValue( nRGB ) ) )
    {
        if ( !pFirstSysColor )
        {
            pActSysColor = pFirstSysColor = new SysColorEntry;
            pFirstSysColor->nRGB = nRGB;
            pFirstSysColor->pNext = NULL;
        }
        else
        {
            pActSysColor = pActSysColor->pNext = new SysColorEntry;
            pActSysColor->nRGB = nRGB;
            pActSysColor->pNext = NULL;
        }
    }
}

void ImplUpdateSysColorEntries()
{
    // delete old SysColorList
    SysColorEntry* pEntry = pFirstSysColor;
    while( pEntry )
    {
        SysColorEntry* pTmp = pEntry->pNext;
        delete pEntry;
        pEntry = pTmp;
    }
    pActSysColor = pFirstSysColor = NULL;

    // create new sys color list
    ImplInsertSysColorEntry( COLOR_ACTIVEBORDER );
    ImplInsertSysColorEntry( COLOR_INACTIVEBORDER );
    ImplInsertSysColorEntry( COLOR_GRADIENTACTIVECAPTION );
    ImplInsertSysColorEntry( COLOR_GRADIENTINACTIVECAPTION );
    ImplInsertSysColorEntry( COLOR_3DFACE );
    ImplInsertSysColorEntry( COLOR_3DHILIGHT );
    ImplInsertSysColorEntry( COLOR_3DLIGHT );
    ImplInsertSysColorEntry( COLOR_3DSHADOW );
    ImplInsertSysColorEntry( COLOR_3DDKSHADOW );
    ImplInsertSysColorEntry( COLOR_INFOBK );
    ImplInsertSysColorEntry( COLOR_INFOTEXT );
    ImplInsertSysColorEntry( COLOR_BTNTEXT );
    ImplInsertSysColorEntry( COLOR_WINDOW );
    ImplInsertSysColorEntry( COLOR_WINDOWTEXT );
    ImplInsertSysColorEntry( COLOR_HIGHLIGHT );
    ImplInsertSysColorEntry( COLOR_HIGHLIGHTTEXT );
    ImplInsertSysColorEntry( COLOR_MENU );
    ImplInsertSysColorEntry( COLOR_MENUTEXT );
    ImplInsertSysColorEntry( COLOR_ACTIVECAPTION );
    ImplInsertSysColorEntry( COLOR_CAPTIONTEXT );
    ImplInsertSysColorEntry( COLOR_INACTIVECAPTION );
    ImplInsertSysColorEntry( COLOR_INACTIVECAPTIONTEXT );
}

void WinSalGraphics::InitGraphics()
{
    // calculate the minimal line width for the printer
    if ( isPrinter() )
    {
        int nDPIX = GetDeviceCaps( getHDC(), LOGPIXELSX );
        if ( nDPIX <= 300 )
            mnPenWidth = 0;
        else
            mnPenWidth = nDPIX/300;
    }

    ::SetTextAlign( getHDC(), TA_BASELINE | TA_LEFT | TA_NOUPDATECP );
    ::SetBkMode( getHDC(), TRANSPARENT );
    ::SetROP2( getHDC(), R2_COPYPEN );

    mpImpl->Init();
}

void WinSalGraphics::DeInitGraphics()
{
    // clear clip region
    SelectClipRgn( getHDC(), 0 );
    // select default objects
    if ( mhDefPen )
        SelectPen( getHDC(), mhDefPen );
    if ( mhDefBrush )
        SelectBrush( getHDC(), mhDefBrush );
    if ( mhDefFont )
        SelectFont( getHDC(), mhDefFont );

    mpImpl->DeInit();
}

HDC ImplGetCachedDC( sal_uLong nID, HBITMAP hBmp )
{
    SalData*    pSalData = GetSalData();
    HDCCache*   pC = &pSalData->mpHDCCache[ nID ];

    if( !pC->mhDC )
    {
        HDC hDC = GetDC( 0 );

        // create new DC with DefaultBitmap
        pC->mhDC = CreateCompatibleDC( hDC );

        if( pSalData->mhDitherPal )
        {
            pC->mhDefPal = SelectPalette( pC->mhDC, pSalData->mhDitherPal, TRUE );
            RealizePalette( pC->mhDC );
        }

        pC->mhSelBmp = CreateCompatibleBitmap( hDC, CACHED_HDC_DEFEXT, CACHED_HDC_DEFEXT );
        pC->mhDefBmp = (HBITMAP) SelectObject( pC->mhDC, pC->mhSelBmp );

        ReleaseDC( 0, hDC );
    }

    if ( hBmp )
        SelectObject( pC->mhDC, pC->mhActBmp = hBmp );
    else
        pC->mhActBmp = 0;

    return pC->mhDC;
}

void ImplReleaseCachedDC( sal_uLong nID )
{
    SalData*    pSalData = GetSalData();
    HDCCache*   pC = &pSalData->mpHDCCache[ nID ];

    if ( pC->mhActBmp )
        SelectObject( pC->mhDC, pC->mhSelBmp );
}

void ImplClearHDCCache( SalData* pData )
{
    for( sal_uLong i = 0; i < CACHESIZE_HDC; i++ )
    {
        HDCCache* pC = &pData->mpHDCCache[ i ];

        if( pC->mhDC )
        {
            SelectObject( pC->mhDC, pC->mhDefBmp );

            if( pC->mhDefPal )
                SelectPalette( pC->mhDC, pC->mhDefPal, TRUE );

            DeleteDC( pC->mhDC );
            DeleteObject( pC->mhSelBmp );
        }
    }
}

OpenGLCompatibleDC::OpenGLCompatibleDC(SalGraphics &rGraphics, int x, int y, int width, int height)
    : mhBitmap(0)
    , mpData(NULL)
    , maRects(0, 0, width, height, x, y, width, height)
{
    WinSalGraphics& rWinGraphics = static_cast<WinSalGraphics&>(rGraphics);
    mpImpl = dynamic_cast<WinOpenGLSalGraphicsImpl*>(rWinGraphics.mpImpl.get());

    if (!mpImpl)
    {
        // we avoid the OpenGL drawing, instead we draw directly to the DC
        mhCompatibleDC = rWinGraphics.getHDC();
        return;
    }

    mhCompatibleDC = CreateCompatibleDC(rWinGraphics.getHDC());

    // move the origin so that we always paint at 0,0 - to keep the bitmap
    // small
    OffsetViewportOrgEx(mhCompatibleDC, -x, -y, NULL);

    mhBitmap = WinSalVirtualDevice::ImplCreateVirDevBitmap(mhCompatibleDC, width, height, 32, reinterpret_cast<void **>(&mpData));

    mhOrigBitmap = (HBITMAP) SelectObject(mhCompatibleDC, mhBitmap);
}

OpenGLCompatibleDC::~OpenGLCompatibleDC()
{
    if (mpImpl)
    {
        SelectObject(mhCompatibleDC, mhOrigBitmap);
        DeleteObject(mhBitmap);
        DeleteDC(mhCompatibleDC);
    }
}

void OpenGLCompatibleDC::fill(sal_uInt32 color)
{
    if (!mpData)
        return;

    sal_uInt32 *p = mpData;
    for (int i = maRects.mnSrcWidth * maRects.mnSrcHeight; i > 0; --i)
        *p++ = color;
}

OpenGLTexture* OpenGLCompatibleDC::getTexture()
{
    if (!mpImpl)
        return NULL;

    // turn what's in the mpData into a texture
    return new OpenGLTexture(maRects.mnSrcWidth, maRects.mnSrcHeight, GL_BGRA, GL_UNSIGNED_BYTE, reinterpret_cast<sal_uInt8*>(mpData));
}

WinSalGraphics::WinSalGraphics(WinSalGraphics::Type eType, bool bScreen, HWND hWnd, SalGeometryProvider *pProvider):
    mhLocalDC(0),
    mbPrinter(eType == WinSalGraphics::PRINTER),
    mbVirDev(eType == WinSalGraphics::VIRTUAL_DEVICE),
    mbWindow(eType == WinSalGraphics::WINDOW),
    mhWnd(hWnd),
    mbScreen(bScreen),
    mfCurrentFontScale(1.0),
    mhRegion(0),
    mhDefPen(0),
    mhDefBrush(0),
    mhDefFont(0),
    mhDefPal(0),
    mpStdClipRgnData(NULL),
    mpFontAttrCache(NULL),
    mnPenWidth(GSL_PEN_WIDTH)
{
    if (OpenGLHelper::isVCLOpenGLEnabled() && !mbPrinter)
        mpImpl.reset(new WinOpenGLSalGraphicsImpl(*this, pProvider));
    else
        mpImpl.reset(new WinSalGraphicsImpl(*this));

    for( int i = 0; i < MAX_FALLBACK; ++i )
    {
        mhFonts[ i ] = 0;
        mpWinFontData[ i ]  = NULL;
        mpWinFontEntry[ i ] = NULL;
        mfFontScale[ i ] = 1.0;
    }
}

WinSalGraphics::~WinSalGraphics()
{
    // free obsolete GDI objects
    ReleaseFonts();

    if ( mhRegion )
    {
        DeleteRegion( mhRegion );
        mhRegion = 0;
    }

    // delete cache data
    delete [] mpStdClipRgnData;
}

SalGraphicsImpl* WinSalGraphics::GetImpl() const
{
    return mpImpl.get();
}

bool WinSalGraphics::isPrinter() const
{
    return mbPrinter;
}

bool WinSalGraphics::isVirtualDevice() const
{
    return mbVirDev;
}

bool WinSalGraphics::isWindow() const
{
    return mbWindow;
}

bool WinSalGraphics::isScreen() const
{
    return mbScreen;
}

HWND WinSalGraphics::gethWnd()
{
    return mhWnd;
}

void WinSalGraphics::setHWND(HWND hWnd)
{
    mhWnd = hWnd;
}

HPALETTE WinSalGraphics::getDefPal() const
{
    return mhDefPal;
}

void WinSalGraphics::setDefPal(HPALETTE hDefPal)
{
    mhDefPal = hDefPal;
}

HRGN WinSalGraphics::getRegion() const
{
    return mhRegion;
}

void WinSalGraphics::GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY )
{
    rDPIX = GetDeviceCaps( getHDC(), LOGPIXELSX );
    rDPIY = GetDeviceCaps( getHDC(), LOGPIXELSY );

    // #111139# this fixes the symptom of div by zero on startup
    // however, printing will fail most likely as communication with
    // the printer seems not to work in this case
    if( !rDPIX || !rDPIY )
        rDPIX = rDPIY = 600;
}

sal_uInt16 WinSalGraphics::GetBitCount() const
{
    return mpImpl->GetBitCount();
}

long WinSalGraphics::GetGraphicsWidth() const
{
    return mpImpl->GetGraphicsWidth();
}

void WinSalGraphics::ResetClipRegion()
{
    mpImpl->ResetClipRegion();
}

bool WinSalGraphics::setClipRegion( const vcl::Region& i_rClip )
{
    return mpImpl->setClipRegion( i_rClip );
}

void WinSalGraphics::SetLineColor()
{
    mpImpl->SetLineColor();
}

void WinSalGraphics::SetLineColor( SalColor nSalColor )
{
    mpImpl->SetLineColor( nSalColor );
}

void WinSalGraphics::SetFillColor()
{
    mpImpl->SetFillColor();
}

void WinSalGraphics::SetFillColor( SalColor nSalColor )
{
    mpImpl->SetFillColor( nSalColor );
}

void WinSalGraphics::SetXORMode( bool bSet, bool bInvertOnly )
{
    mpImpl->SetXORMode( bSet, bInvertOnly );
}

void WinSalGraphics::SetROPLineColor( SalROPColor nROPColor )
{
    mpImpl->SetROPLineColor( nROPColor );
}

void WinSalGraphics::SetROPFillColor( SalROPColor nROPColor )
{
    mpImpl->SetROPFillColor( nROPColor );
}

void WinSalGraphics::drawPixel( long nX, long nY )
{
    mpImpl->drawPixel( nX, nY );
}

void WinSalGraphics::drawPixel( long nX, long nY, SalColor nSalColor )
{
    mpImpl->drawPixel( nX, nY, nSalColor );
}

void WinSalGraphics::drawLine( long nX1, long nY1, long nX2, long nY2 )
{
    mpImpl->drawLine( nX1, nY1, nX2, nY2 );
}

void WinSalGraphics::drawRect( long nX, long nY, long nWidth, long nHeight )
{
    mpImpl->drawRect( nX, nY, nWidth, nHeight );
}

void WinSalGraphics::drawPolyLine( sal_uInt32 nPoints, const SalPoint* pPtAry )
{
    mpImpl->drawPolyLine( nPoints, pPtAry );
}

void WinSalGraphics::drawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry )
{
    mpImpl->drawPolygon( nPoints, pPtAry );
}

void WinSalGraphics::drawPolyPolygon( sal_uInt32 nPoly, const sal_uInt32* pPoints,
                                   PCONSTSALPOINT* pPtAry )
{
    mpImpl->drawPolyPolygon( nPoly, pPoints, pPtAry );
}

bool WinSalGraphics::drawPolyLineBezier( sal_uInt32 nPoints, const SalPoint* pPtAry, const BYTE* pFlgAry )
{
    return mpImpl->drawPolyLineBezier( nPoints, pPtAry, pFlgAry );
}

bool WinSalGraphics::drawPolygonBezier( sal_uInt32 nPoints, const SalPoint* pPtAry, const BYTE* pFlgAry )
{
    return mpImpl->drawPolygonBezier( nPoints, pPtAry, pFlgAry );
}

bool WinSalGraphics::drawPolyPolygonBezier( sal_uInt32 nPoly, const sal_uInt32* pPoints,
                                             const SalPoint* const* pPtAry, const BYTE* const* pFlgAry )
{
    return mpImpl->drawPolyPolygonBezier( nPoly, pPoints, pPtAry, pFlgAry );
}

static BYTE* ImplSearchEntry( BYTE* pSource, BYTE* pDest, sal_uLong nComp, sal_uLong nSize )
{
    while ( nComp-- >= nSize )
    {
        sal_uLong i;
        for ( i = 0; i < nSize; i++ )
        {
            if ( ( pSource[i]&~0x20 ) != ( pDest[i]&~0x20 ) )
                break;
        }
        if ( i == nSize )
            return pSource;
        pSource++;
    }
    return NULL;
}

static bool ImplGetBoundingBox( double* nNumb, BYTE* pSource, sal_uLong nSize )
{
    bool    bRetValue = FALSE;
    BYTE* pDest = ImplSearchEntry( pSource, (BYTE*)"%%BoundingBox:", nSize, 14 );
    if ( pDest )
    {
        nNumb[0] = nNumb[1] = nNumb[2] = nNumb[3] = 0;
        pDest += 14;

        int nSizeLeft = nSize - ( pDest - pSource );
        if ( nSizeLeft > 100 )
            nSizeLeft = 100;    // only 100 bytes following the bounding box will be checked

        int i;
        for ( i = 0; ( i < 4 ) && nSizeLeft; i++ )
        {
            int     nDivision = 1;
            bool    bDivision = FALSE;
            bool    bNegative = FALSE;
            bool    bValid = TRUE;

            while ( ( --nSizeLeft ) && ( ( *pDest == ' ' ) || ( *pDest == 0x9 ) ) ) pDest++;
            BYTE nByte = *pDest;
            while ( nSizeLeft && ( nByte != ' ' ) && ( nByte != 0x9 ) && ( nByte != 0xd ) && ( nByte != 0xa ) )
            {
                switch ( nByte )
                {
                    case '.' :
                        if ( bDivision )
                            bValid = FALSE;
                        else
                            bDivision = TRUE;
                        break;
                    case '-' :
                        bNegative = TRUE;
                        break;
                    default :
                        if ( ( nByte < '0' ) || ( nByte > '9' ) )
                            nSizeLeft = 1;  // error parsing the bounding box values
                        else if ( bValid )
                        {
                            if ( bDivision )
                                nDivision*=10;
                            nNumb[i] *= 10;
                            nNumb[i] += nByte - '0';
                        }
                        break;
                }
                nSizeLeft--;
                nByte = *(++pDest);
            }
            if ( bNegative )
                nNumb[i] = -nNumb[i];
            if ( bDivision && ( nDivision != 1 ) )
                nNumb[i] /= nDivision;
        }
        if ( i == 4 )
            bRetValue = TRUE;
    }
    return bRetValue;
}

#define POSTSCRIPT_BUFSIZE 0x4000           // MAXIMUM BUFSIZE EQ 0xFFFF

bool WinSalGraphics::drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, sal_uLong nSize )
{
    bool bRetValue = false;

    if ( mbPrinter )
    {
        int nEscape = POSTSCRIPT_PASSTHROUGH;

        if ( Escape( getHDC(), QUERYESCSUPPORT, sizeof( int ), ( LPSTR )&nEscape, 0 ) )
        {
            double  nBoundingBox[4];

            if ( ImplGetBoundingBox( nBoundingBox, (BYTE*)pPtr, nSize ) )
            {
                OStringBuffer aBuf( POSTSCRIPT_BUFSIZE );

                // reserve place for a sal_uInt16
                aBuf.append( "aa" );

                // #107797# Write out EPS encapsulation header

                // directly taken from the PLRM 3.0, p. 726. Note:
                // this will definitely cause problems when
                // recursively creating and embedding PostScript files
                // in OOo, since we use statically-named variables
                // here (namely, b4_Inc_state_salWin, dict_count_salWin and
                // op_count_salWin). Currently, I have no idea on how to
                // work around that, except from scanning and
                // interpreting the EPS for unused identifiers.

                // append the real text
                aBuf.append( "\n\n/b4_Inc_state_salWin save def\n"
                             "/dict_count_salWin countdictstack def\n"
                             "/op_count_salWin count 1 sub def\n"
                             "userdict begin\n"
                             "/showpage {} def\n"
                             "0 setgray 0 setlinecap\n"
                             "1 setlinewidth 0 setlinejoin\n"
                             "10 setmiterlimit [] 0 setdash newpath\n"
                             "/languagelevel where\n"
                             "{\n"
                             "  pop languagelevel\n"
                             "  1 ne\n"
                             "  {\n"
                             "    false setstrokeadjust false setoverprint\n"
                             "  } if\n"
                             "} if\n\n" );

                // #i10737# Apply clipping manually

                // Windows seems to ignore any clipping at the HDC,
                // when followed by a POSTSCRIPT_PASSTHROUGH

                // Check whether we've got a clipping, consisting of
                // exactly one rect (other cases should be, but aren't
                // handled currently)

                // TODO: Handle more than one rectangle here (take
                // care, the buffer can handle only POSTSCRIPT_BUFSIZE
                // characters!)
                if ( mhRegion != 0 &&
                     mpStdClipRgnData != NULL &&
                     mpClipRgnData == mpStdClipRgnData &&
                     mpClipRgnData->rdh.nCount == 1 )
                {
                    RECT* pRect = &(mpClipRgnData->rdh.rcBound);

                    aBuf.append( "\nnewpath\n" );
                    aBuf.append( pRect->left );
                    aBuf.append( " " );
                    aBuf.append( pRect->top );
                    aBuf.append( " moveto\n" );
                    aBuf.append( pRect->right );
                    aBuf.append( " " );
                    aBuf.append( pRect->top );
                    aBuf.append( " lineto\n" );
                    aBuf.append( pRect->right );
                    aBuf.append( " " );
                    aBuf.append( pRect->bottom );
                    aBuf.append( " lineto\n" );
                    aBuf.append( pRect->left );
                    aBuf.append( " " );
                    aBuf.append( pRect->bottom );
                    aBuf.append( " lineto\n"
                                 "closepath\n"
                                 "clip\n"
                                 "newpath\n" );
                }

                // #107797# Write out buffer

                *((sal_uInt16*)aBuf.getStr()) = (sal_uInt16)( aBuf.getLength() - 2 );
                Escape ( getHDC(), nEscape, aBuf.getLength(), (LPTSTR)aBuf.getStr(), 0 );

                // #107797# Write out EPS transformation code

                double  dM11 = nWidth / ( nBoundingBox[2] - nBoundingBox[0] );
                double  dM22 = nHeight / (nBoundingBox[1] - nBoundingBox[3] );
                // reserve a sal_uInt16 again
                aBuf.setLength( 2 );
                aBuf.append( "\n\n[" );
                aBuf.append( dM11 );
                aBuf.append( " 0 0 " );
                aBuf.append( dM22 );
                aBuf.append( ' ' );
                aBuf.append( nX - ( dM11 * nBoundingBox[0] ) );
                aBuf.append( ' ' );
                aBuf.append( nY - ( dM22 * nBoundingBox[3] ) );
                aBuf.append( "] concat\n"
                             "%%BeginDocument:\n" );
                *((sal_uInt16*)aBuf.getStr()) = (sal_uInt16)( aBuf.getLength() - 2 );
                Escape ( getHDC(), nEscape, aBuf.getLength(), (LPTSTR)aBuf.getStr(), 0 );

                // #107797# Write out actual EPS content

                sal_uLong   nToDo = nSize;
                sal_uLong   nDoNow;
                while ( nToDo )
                {
                    nDoNow = nToDo;
                    if ( nToDo > POSTSCRIPT_BUFSIZE - 2 )
                        nDoNow = POSTSCRIPT_BUFSIZE - 2;
                    // the following is based on the string buffer allocation
                    // of size POSTSCRIPT_BUFSIZE at construction time of aBuf
                    *((sal_uInt16*)aBuf.getStr()) = (sal_uInt16)nDoNow;
                    memcpy( (void*)(aBuf.getStr() + 2), (BYTE*)pPtr + nSize - nToDo, nDoNow );
                    sal_uLong nResult = Escape ( getHDC(), nEscape, nDoNow + 2, (LPTSTR)aBuf.getStr(), 0 );
                    if (!nResult )
                        break;
                    nToDo -= nResult;
                }

                // #107797# Write out EPS encapsulation footer

                // reserve a sal_uInt16 again
                aBuf.setLength( 2 );
                aBuf.append( "%%EndDocument\n"
                             "count op_count_salWin sub {pop} repeat\n"
                             "countdictstack dict_count_salWin sub {end} repeat\n"
                             "b4_Inc_state_salWin restore\n\n" );
                *((sal_uInt16*)aBuf.getStr()) = (sal_uInt16)( aBuf.getLength() - 2 );
                Escape ( getHDC(), nEscape, aBuf.getLength(), (LPTSTR)aBuf.getStr(), 0 );
                bRetValue = TRUE;
            }
        }
    }

    return bRetValue;
}

SystemGraphicsData WinSalGraphics::GetGraphicsData() const
{
    SystemGraphicsData aRes;
    aRes.nSize = sizeof(aRes);
    aRes.hDC = const_cast< WinSalGraphics* >(this)->getHDC();
    return aRes;
}

bool WinSalGraphics::SupportsCairo() const
{
#if ENABLE_CAIRO_CANVAS
    return true;
#else
    return false;
#endif
}

/**
 * cairo::createSurface:     Create generic Canvas surface using given Cairo Surface
 *
 * @param rSurface Cairo Surface
 *
 * @return new Surface
 */
cairo::SurfaceSharedPtr WinSalGraphics::CreateSurface(const cairo::CairoSurfaceSharedPtr& rSurface) const
{
#if ENABLE_CAIRO_CANVAS
    return cairo::SurfaceSharedPtr(new cairo::Win32Surface(rSurface));
#else
    (void)rSurface;
    return cairo::SurfaceSharedPtr();
#endif
}

/**
 * cairo::createSurface:     Create Canvas surface using given VCL Window or Virtualdevice
 *
 * @param rSurface Cairo Surface
 *
 *  For VCL Window, use platform native system environment data (struct SystemEnvData in vcl/inc/sysdata.hxx)
 *  For VCL Virtualdevice, use platform native system graphics data (struct SystemGraphicsData in vcl/inc/sysdata.hxx)
 *
 * @return new Surface
 */
cairo::SurfaceSharedPtr WinSalGraphics::CreateSurface( const OutputDevice& rRefDevice,
                                int x, int y, int /* width */, int /* height */) const
{
    cairo::SurfaceSharedPtr surf;

#if ENABLE_CAIRO_CANVAS
    if( rRefDevice.GetOutDevType() == OUTDEV_WINDOW )
    {
        const vcl::Window &rWindow = (const vcl::Window &) rRefDevice;
        const SystemEnvData* pSysData = GetSysData(&rWindow);
        if (pSysData && pSysData->hWnd)
            surf = cairo::SurfaceSharedPtr(new cairo::Win32Surface(GetDC((HWND) pSysData->hWnd), x, y));
    }
    else if( rRefDevice.GetOutDevType() == OUTDEV_VIRDEV )
    {
        SystemGraphicsData aSysData = ((const VirtualDevice&) rRefDevice).GetSystemGfxData();
        if (aSysData.hDC)
            surf = cairo::SurfaceSharedPtr(new cairo::Win32Surface((HDC) aSysData.hDC, x, y));
    }
#else
    (void)rRefDevice;
    (void)x;
    (void)y;
#endif

    return surf;
}

/**
 * cairo::createBitmapSurface:   Create platform native Canvas surface from BitmapSystemData
 * @param OutputDevice (not used)
 * @param rData Platform native image data (struct BitmapSystemData in vcl/inc/bitmap.hxx)
 * @param rSize width and height of the new surface
 *
 * Create a surface based on image data on rData
 *
 * @return new surface or empty surface
 **/
cairo::SurfaceSharedPtr WinSalGraphics::CreateBitmapSurface( const OutputDevice&     /* rRefDevice */,
                                      const BitmapSystemData& rData,
                                      const Size&             rSize ) const
{
    OSL_TRACE( "requested size: %d x %d available size: %d x %d",
               rSize.Width(), rSize.Height(), rData.mnWidth, rData.mnHeight );

#if ENABLE_CAIRO_CANVAS
    if ( rData.mnWidth == rSize.Width() && rData.mnHeight == rSize.Height() )
        return cairo::SurfaceSharedPtr(new cairo::Win32Surface( rData ));
#else
    (void)rData;
    (void)rSize;
#endif
    return cairo::SurfaceSharedPtr();
}

#if ENABLE_CAIRO_CANVAS
namespace
{
    HBITMAP surface2HBitmap( const SurfaceSharedPtr& rSurface, const basegfx::B2ISize& rSize )
    {
        // can't seem to retrieve HBITMAP from cairo. copy content then
        HDC hScreenDC=GetDC(NULL);
        HBITMAP hBmpBitmap = CreateCompatibleBitmap( hScreenDC,
                                                     rSize.getX(),
                                                     rSize.getY() );

        HDC     hBmpDC = CreateCompatibleDC( 0 );
        HBITMAP hBmpOld = (HBITMAP) SelectObject( hBmpDC, hBmpBitmap );

        BitBlt( hBmpDC, 0, 0, rSize.getX(), rSize.getX(),
                cairo_win32_surface_get_dc(rSurface->getCairoSurface().get()),
                0, 0, SRCCOPY );

        SelectObject( hBmpDC, hBmpOld );
        DeleteDC( hBmpDC );

        return hBmpBitmap;
    }
}
#endif

css::uno::Any WinSalGraphics::GetNativeSurfaceHandle(cairo::SurfaceSharedPtr& rSurface, const ::basegfx::B2ISize& rSize) const
{
    // TODO(F2): check whether under all circumstances,
    // the alpha channel is ignored here.
    css::uno::Sequence< css::uno::Any > args( 1 );
    sal_Int64 nHandle;
#if ENABLE_CAIRO_CANVAS
    nHandle = sal_Int64(surface2HBitmap(rSurface, rSize));
#else
    (void)rSurface;
    (void)rSize;
    nHandle = 0;
#endif
    args[1] = css::uno::Any(nHandle);
    // caller frees the bitmap
    return css::uno::Any( args );
}

OpenGLContext *WinSalGraphics::BeginPaint()
{
    return mpImpl->beginPaint();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
