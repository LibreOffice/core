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

#include <win/wincomp.hxx>
#include <win/saldata.hxx>
#include <win/salgdi.h>
#include <win/salframe.h>

// =======================================================================

// comment out to prevent use of beziers on GDI functions
#define USE_GDI_BEZIERS

// =======================================================================

#define DITHER_PAL_DELTA                51
#define DITHER_PAL_STEPS                6
#define DITHER_PAL_COUNT                (DITHER_PAL_STEPS*DITHER_PAL_STEPS*DITHER_PAL_STEPS)
#define DITHER_MAX_SYSCOLOR             16
#define DITHER_EXTRA_COLORS             1
#define DMAP( _def_nVal, _def_nThres )  ((pDitherDiff[_def_nVal]>(_def_nThres))?pDitherHigh[_def_nVal]:pDitherLow[_def_nVal])

// =======================================================================

struct SysColorEntry
{
    DWORD           nRGB;
    SysColorEntry*  pNext;
};

// =======================================================================

static SysColorEntry* pFirstSysColor = NULL;
static SysColorEntry* pActSysColor = NULL;

// -----------------------------------------------------------------------------

// Blue7
static PALETTEENTRY aImplExtraColor1 =
{
    0, 184, 255, 0
};

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

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

// =======================================================================

// we must create pens with 1-pixel width; otherwise the S3-graphics card
// map has many paint problems when drawing polygons/polyLines and a
// complex is set
#define GSL_PEN_WIDTH                   1

// =======================================================================

#define SAL_POLYPOLYCOUNT_STACKBUF          8
#define SAL_POLYPOLYPOINTS_STACKBUF         64

// =======================================================================

void ImplInitSalGDI()
{
    SalData* pSalData = GetSalData();

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
        sal_Bool        bDither16 = TRUE;

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

// -----------------------------------------------------------------------

void ImplFreeSalGDI()
{
    SalData*    pSalData = GetSalData();

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
}

// -----------------------------------------------------------------------

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

// =======================================================================

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

// =======================================================================

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

// =======================================================================

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

// -----------------------------------------------------------------------

static SalColor ImplGetROPSalColor( SalROPColor nROPColor )
{
    SalColor nSalColor;
    if ( nROPColor == SAL_ROP_0 )
        nSalColor = MAKE_SALCOLOR( 0, 0, 0 );
    else
        nSalColor = MAKE_SALCOLOR( 255, 255, 255 );
    return nSalColor;
}

// =======================================================================

void ImplSalInitGraphics( WinSalGraphics* pData )
{
    // calculate the minimal line width for the printer
    if ( pData->mbPrinter )
    {
        int nDPIX = GetDeviceCaps( pData->getHDC(), LOGPIXELSX );
        if ( nDPIX <= 300 )
            pData->mnPenWidth = 0;
        else
            pData->mnPenWidth = nDPIX/300;
    }

    ::SetTextAlign( pData->getHDC(), TA_BASELINE | TA_LEFT | TA_NOUPDATECP );
    ::SetBkMode( pData->getHDC(), TRANSPARENT );
    ::SetROP2( pData->getHDC(), R2_COPYPEN );
}

// -----------------------------------------------------------------------

void ImplSalDeInitGraphics( WinSalGraphics* pData )
{
    // clear clip region
    SelectClipRgn( pData->getHDC(), 0 );
    // select default objects
    if ( pData->mhDefPen )
        SelectPen( pData->getHDC(), pData->mhDefPen );
    if ( pData->mhDefBrush )
        SelectBrush( pData->getHDC(), pData->mhDefBrush );
    if ( pData->mhDefFont )
        SelectFont( pData->getHDC(), pData->mhDefFont );
}

// =======================================================================

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

// =======================================================================

void ImplReleaseCachedDC( sal_uLong nID )
{
    SalData*    pSalData = GetSalData();
    HDCCache*   pC = &pSalData->mpHDCCache[ nID ];

    if ( pC->mhActBmp )
        SelectObject( pC->mhDC, pC->mhSelBmp );
}

// =======================================================================

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

// =======================================================================

// #100127# Fill point and flag memory from array of points which
// might also contain bezier control points for the PolyDraw() GDI method
// Make sure pWinPointAry and pWinFlagAry are big enough
void ImplPreparePolyDraw( bool                      bCloseFigures,
                          sal_uLong                     nPoly,
                          const sal_uInt32*         pPoints,
                          const SalPoint* const*    pPtAry,
                          const BYTE* const*        pFlgAry,
                          POINT*                    pWinPointAry,
                          BYTE*                     pWinFlagAry     )
{
    sal_uLong nCurrPoly;
    for( nCurrPoly=0; nCurrPoly<nPoly; ++nCurrPoly )
    {
        const POINT* pCurrPoint = reinterpret_cast<const POINT*>( *pPtAry++ );
        const BYTE* pCurrFlag = *pFlgAry++;
        const sal_uInt32 nCurrPoints = *pPoints++;
        const bool bHaveFlagArray( pCurrFlag );
        sal_uLong nCurrPoint;

        if( nCurrPoints )
        {
            // start figure
            *pWinPointAry++ = *pCurrPoint++;
            *pWinFlagAry++  = PT_MOVETO;
            ++pCurrFlag;

            for( nCurrPoint=1; nCurrPoint<nCurrPoints; )
            {
                // #102067# Check existence of flag array
                if( bHaveFlagArray &&
                    ( nCurrPoint + 2 ) < nCurrPoints )
                {
                    BYTE P4( pCurrFlag[ 2 ] );

                    if( ( POLY_CONTROL == pCurrFlag[ 0 ] ) &&
                        ( POLY_CONTROL == pCurrFlag[ 1 ] ) &&
                        ( POLY_NORMAL == P4 || POLY_SMOOTH == P4 || POLY_SYMMTR == P4 ) )
                    {
                        // control point one
                        *pWinPointAry++ = *pCurrPoint++;
                        *pWinFlagAry++  = PT_BEZIERTO;

                        // control point two
                        *pWinPointAry++ = *pCurrPoint++;
                        *pWinFlagAry++  = PT_BEZIERTO;

                        // end point
                        *pWinPointAry++ = *pCurrPoint++;
                        *pWinFlagAry++  = PT_BEZIERTO;

                        nCurrPoint += 3;
                        pCurrFlag += 3;
                        continue;
                    }
                }

                // regular line point
                *pWinPointAry++ = *pCurrPoint++;
                *pWinFlagAry++  = PT_LINETO;
                ++pCurrFlag;
                ++nCurrPoint;
            }

            // end figure?
            if( bCloseFigures )
                pWinFlagAry[-1] |= PT_CLOSEFIGURE;
        }
    }
}

// =======================================================================

// #100127# draw an array of points which might also contain bezier control points
void ImplRenderPath( HDC hdc, sal_uLong nPoints, const SalPoint* pPtAry, const BYTE* pFlgAry )
{
    if( nPoints )
    {
        sal_uInt16 i;
        // TODO: profile whether the following options are faster:
        // a) look ahead and draw consecutive bezier or line segments by PolyBezierTo/PolyLineTo resp.
        // b) convert our flag array to window's and use PolyDraw

        MoveToEx( hdc, pPtAry->mnX, pPtAry->mnY, NULL );
        ++pPtAry; ++pFlgAry;

        for( i=1; i<nPoints; ++i, ++pPtAry, ++pFlgAry )
        {
            if( *pFlgAry != POLY_CONTROL )
            {
                LineTo( hdc, pPtAry->mnX, pPtAry->mnY );
            }
            else if( nPoints - i > 2 )
            {
                PolyBezierTo( hdc, reinterpret_cast<const POINT*>(pPtAry), 3 );
                i += 2; pPtAry += 2; pFlgAry += 2;
            }
        }
    }
}

// =======================================================================

WinSalGraphics::WinSalGraphics()
{
    for( int i = 0; i < MAX_FALLBACK; ++i )
    {
        mhFonts[ i ] = 0;
        mpWinFontData[ i ]  = NULL;
        mpWinFontEntry[ i ] = NULL;
        mfFontScale[ i ] = 1.0;
    }

    mfCurrentFontScale = 1.0;

    mhLocalDC           = 0;
    mhPen               = 0;
    mhBrush             = 0;
    mhRegion            = 0;
    mhDefPen            = 0;
    mhDefBrush          = 0;
    mhDefFont           = 0;
    mhDefPal            = 0;
    mpStdClipRgnData    = NULL;
    mpLogFont           = NULL;
    mpFontCharSets      = NULL;
    mpFontAttrCache     = NULL;
    mnFontCharSetCount  = 0;
    mpFontKernPairs     = NULL;
    mnFontKernPairCount = 0;
    mbFontKernInit      = FALSE;
    mbXORMode           = FALSE;
    mnPenWidth          = GSL_PEN_WIDTH;
}

// -----------------------------------------------------------------------

WinSalGraphics::~WinSalGraphics()
{
    // free obsolete GDI objects
        ReleaseFonts();

    if ( mhPen )
    {
        if ( !mbStockPen )
            DeletePen( mhPen );
    }
    if ( mhBrush )
    {
        if ( !mbStockBrush )
            DeleteBrush( mhBrush );
    }

    if ( mhRegion )
    {
        DeleteRegion( mhRegion );
        mhRegion = 0;
    }

    // delete cache data
    if ( mpStdClipRgnData )
        delete [] mpStdClipRgnData;

    delete mpLogFont;

    delete mpFontCharSets;

    delete mpFontKernPairs;
}

// -----------------------------------------------------------------------

void WinSalGraphics::GetResolution( long& rDPIX, long& rDPIY )
{
    rDPIX = GetDeviceCaps( getHDC(), LOGPIXELSX );
    rDPIY = GetDeviceCaps( getHDC(), LOGPIXELSY );

    // #111139# this fixes the symptom of div by zero on startup
    // however, printing will fail most likely as communication with
    // the printer seems not to work in this case
    if( !rDPIX || !rDPIY )
        rDPIX = rDPIY = 600;
}

// -----------------------------------------------------------------------

sal_uInt16 WinSalGraphics::GetBitCount() const
{
    return (sal_uInt16)GetDeviceCaps( getHDC(), BITSPIXEL );
}

// -----------------------------------------------------------------------

long WinSalGraphics::GetGraphicsWidth() const
{
    if( mhWnd && IsWindow( mhWnd ) )
    {
        WinSalFrame* pFrame = GetWindowPtr( mhWnd );
        if( pFrame )
        {
            if( pFrame->maGeometry.nWidth )
                return pFrame->maGeometry.nWidth;
            else
            {
                // TODO: perhaps not needed, maGeometry should always be up-to-date
                RECT aRect;
                GetClientRect( mhWnd, &aRect );
                return aRect.right;
            }
        }
    }

    return 0;
}

// -----------------------------------------------------------------------

void WinSalGraphics::ResetClipRegion()
{
    if ( mhRegion )
    {
        DeleteRegion( mhRegion );
        mhRegion = 0;
    }

    SelectClipRgn( getHDC(), 0 );
}

// -----------------------------------------------------------------------

bool WinSalGraphics::setClipRegion( const Region& i_rClip )
{
    if ( mhRegion )
    {
        DeleteRegion( mhRegion );
        mhRegion = 0;
    }

    if( i_rClip.HasPolyPolygonOrB2DPolyPolygon() )
    {
        const basegfx::B2DPolyPolygon aPolyPolygon( i_rClip.GetAsB2DPolyPolygon() );
        const sal_uInt32 nCount(aPolyPolygon.count());

        if( nCount )
        {
            std::vector< POINT > aPolyPoints;
            aPolyPoints.reserve( 1024 );
            std::vector< INT > aPolyCounts( nCount, 0 );

            for(sal_uInt32 a(0); a < nCount; a++)
            {
                basegfx::B2DPolygon aPoly(aPolyPolygon.getB2DPolygon(a));

                aPoly = basegfx::tools::adaptiveSubdivideByDistance( aPoly, 1 );
                const sal_uInt32 nPoints = aPoly.count();
                aPolyCounts[a] = nPoints;

                for( sal_uInt32 b = 0; b < nPoints; b++ )
                {
                    basegfx::B2DPoint aPt( aPoly.getB2DPoint( b ) );
                    POINT aPOINT;
                    aPOINT.x = (LONG)aPt.getX();
                    aPOINT.y = (LONG)aPt.getY();
                    aPolyPoints.push_back( aPOINT );
                }
            }

            mhRegion = CreatePolyPolygonRgn( &aPolyPoints[0], &aPolyCounts[0], nCount, ALTERNATE );
        }
    }
    else
    {
        RectangleVector aRectangles;
        i_rClip.GetRegionRectangles(aRectangles);

        sal_uLong nRectBufSize = sizeof(RECT)*aRectangles.size();
        if ( aRectangles.size() < SAL_CLIPRECT_COUNT )
        {
            if ( !mpStdClipRgnData )
                mpStdClipRgnData = (RGNDATA*)new BYTE[sizeof(RGNDATA)-1+(SAL_CLIPRECT_COUNT*sizeof(RECT))];
            mpClipRgnData = mpStdClipRgnData;
        }
        else
            mpClipRgnData = (RGNDATA*)new BYTE[sizeof(RGNDATA)-1+nRectBufSize];
        mpClipRgnData->rdh.dwSize   = sizeof( RGNDATAHEADER );
        mpClipRgnData->rdh.iType    = RDH_RECTANGLES;
        mpClipRgnData->rdh.nCount   = aRectangles.size();
        mpClipRgnData->rdh.nRgnSize = nRectBufSize;
        RECT*       pBoundRect = &(mpClipRgnData->rdh.rcBound);
        SetRectEmpty( pBoundRect );
        RECT* pNextClipRect         = (RECT*)(&(mpClipRgnData->Buffer));
        bool bFirstClipRect         = true;

        for(RectangleVector::const_iterator aRectIter(aRectangles.begin()); aRectIter != aRectangles.end(); ++aRectIter)
        {
            const long nW(aRectIter->GetWidth());
            const long nH(aRectIter->GetHeight());

            if(nW && nH)
            {
                const long nRight(aRectIter->Left() + nW);
                const long nBottom(aRectIter->Top() + nH);

                if(bFirstClipRect)
                {
                    pBoundRect->left = aRectIter->Left();
                    pBoundRect->top = aRectIter->Top();
                    pBoundRect->right = nRight;
                    pBoundRect->bottom = nBottom;
                    bFirstClipRect = false;
                }
                else
                {
                    if(aRectIter->Left() < pBoundRect->left)
                    {
                        pBoundRect->left = (int)aRectIter->Left();
                    }

                    if(aRectIter->Top() < pBoundRect->top)
                    {
                        pBoundRect->top = (int)aRectIter->Top();
                    }

                    if(nRight > pBoundRect->right)
                    {
                        pBoundRect->right = (int)nRight;
                    }

                    if(nBottom > pBoundRect->bottom)
                    {
                        pBoundRect->bottom = (int)nBottom;
                    }
                }

                pNextClipRect->left = (int)aRectIter->Left();
                pNextClipRect->top = (int)aRectIter->Top();
                pNextClipRect->right = (int)nRight;
                pNextClipRect->bottom = (int)nBottom;
                pNextClipRect++;
            }
            else
            {
                mpClipRgnData->rdh.nCount--;
                mpClipRgnData->rdh.nRgnSize -= sizeof( RECT );
            }
        }

        // create clip region from ClipRgnData
        if ( mpClipRgnData->rdh.nCount == 1 )
        {
            RECT* pRect = &(mpClipRgnData->rdh.rcBound);
            mhRegion = CreateRectRgn( pRect->left, pRect->top,
                                                     pRect->right, pRect->bottom );
        }
        else if( mpClipRgnData->rdh.nCount > 1 )
        {
            sal_uLong nSize = mpClipRgnData->rdh.nRgnSize+sizeof(RGNDATAHEADER);
            mhRegion = ExtCreateRegion( NULL, nSize, mpClipRgnData );

            // if ExtCreateRegion(...) is not supported
            if( !mhRegion )
            {
                RGNDATAHEADER* pHeader = (RGNDATAHEADER*) mpClipRgnData;

                if( pHeader->nCount )
                {
                    RECT* pRect = (RECT*) mpClipRgnData->Buffer;
                    mhRegion = CreateRectRgn( pRect->left, pRect->top, pRect->right, pRect->bottom );
                    pRect++;

                    for( sal_uLong n = 1; n < pHeader->nCount; n++, pRect++ )
                    {
                        HRGN hRgn = CreateRectRgn( pRect->left, pRect->top, pRect->right, pRect->bottom );
                        CombineRgn( mhRegion, mhRegion, hRgn, RGN_OR );
                        DeleteRegion( hRgn );
                    }
                }
            }

            if ( mpClipRgnData != mpStdClipRgnData )
                delete [] mpClipRgnData;
        }
    }

    if( mhRegion )
        SelectClipRgn( getHDC(), mhRegion );
    return mhRegion != 0;
}

// -----------------------------------------------------------------------

void WinSalGraphics::SetLineColor()
{
    // create and select new pen
    HPEN hNewPen = GetStockPen( NULL_PEN );
    HPEN hOldPen = SelectPen( getHDC(), hNewPen );

    // destroy or save old pen
    if ( mhPen )
    {
        if ( !mbStockPen )
            DeletePen( mhPen );
    }
    else
        mhDefPen = hOldPen;

    // set new data
    mhPen       = hNewPen;
    mbPen       = FALSE;
    mbStockPen  = TRUE;
}

// -----------------------------------------------------------------------

void WinSalGraphics::SetLineColor( SalColor nSalColor )
{
    maLineColor = nSalColor;
    COLORREF    nPenColor = PALETTERGB( SALCOLOR_RED( nSalColor ),
                                        SALCOLOR_GREEN( nSalColor ),
                                        SALCOLOR_BLUE( nSalColor ) );
    HPEN        hNewPen = 0;
    sal_Bool        bStockPen = FALSE;

    // search for stock pen (only screen, because printer have problems,
    // when we use stock objects)
    if ( !mbPrinter )
    {
        SalData* pSalData = GetSalData();
        for ( sal_uInt16 i = 0; i < pSalData->mnStockPenCount; i++ )
        {
            if ( nPenColor == pSalData->maStockPenColorAry[i] )
            {
                hNewPen = pSalData->mhStockPenAry[i];
                bStockPen = TRUE;
                break;
            }
        }
    }

    // create new pen
    if ( !hNewPen )
    {
        if ( !mbPrinter )
        {
            if ( GetSalData()->mhDitherPal && ImplIsSysColorEntry( nSalColor ) )
                nPenColor = PALRGB_TO_RGB( nPenColor );
        }

        hNewPen = CreatePen( PS_SOLID, mnPenWidth, nPenColor );
        bStockPen = FALSE;
    }

    // select new pen
    HPEN hOldPen = SelectPen( getHDC(), hNewPen );

    // destroy or save old pen
    if ( mhPen )
    {
        if ( !mbStockPen )
            DeletePen( mhPen );
    }
    else
        mhDefPen = hOldPen;

    // set new data
    mnPenColor  = nPenColor;
    mhPen       = hNewPen;
    mbPen       = TRUE;
    mbStockPen  = bStockPen;
}

// -----------------------------------------------------------------------

void WinSalGraphics::SetFillColor()
{
    // create and select new brush
    HBRUSH hNewBrush = GetStockBrush( NULL_BRUSH );
    HBRUSH hOldBrush = SelectBrush( getHDC(), hNewBrush );

    // destroy or save old brush
    if ( mhBrush )
    {
        if ( !mbStockBrush )
            DeleteBrush( mhBrush );
    }
    else
        mhDefBrush = hOldBrush;

    // set new data
    mhBrush     = hNewBrush;
    mbBrush     = FALSE;
    mbStockBrush = TRUE;
}

// -----------------------------------------------------------------------

void WinSalGraphics::SetFillColor( SalColor nSalColor )
{
    maFillColor = nSalColor;
    SalData*    pSalData    = GetSalData();
    BYTE        nRed        = SALCOLOR_RED( nSalColor );
    BYTE        nGreen      = SALCOLOR_GREEN( nSalColor );
    BYTE        nBlue       = SALCOLOR_BLUE( nSalColor );
    COLORREF    nBrushColor = PALETTERGB( nRed, nGreen, nBlue );
    HBRUSH      hNewBrush   = 0;
    sal_Bool        bStockBrush = FALSE;

    // search for stock brush (only screen, because printer have problems,
    // when we use stock objects)
    if ( !mbPrinter )
    {
        for ( sal_uInt16 i = 0; i < pSalData->mnStockBrushCount; i++ )
        {
            if ( nBrushColor == pSalData->maStockBrushColorAry[ i ] )
            {
                hNewBrush = pSalData->mhStockBrushAry[i];
                bStockBrush = TRUE;
                break;
            }
        }
    }

    // create new brush
    if ( !hNewBrush )
    {
        if ( mbPrinter || !pSalData->mhDitherDIB )
            hNewBrush = CreateSolidBrush( nBrushColor );
        else
        {
            if ( 24 == ((BITMAPINFOHEADER*)pSalData->mpDitherDIB)->biBitCount )
            {
                BYTE* pTmp = pSalData->mpDitherDIBData;
                long* pDitherDiff = pSalData->mpDitherDiff;
                BYTE* pDitherLow = pSalData->mpDitherLow;
                BYTE* pDitherHigh = pSalData->mpDitherHigh;

                for( long nY = 0L; nY < 8L; nY++ )
                {
                    for( long nX = 0L; nX < 8L; nX++ )
                    {
                        const long nThres = aOrdDither16Bit[ nY ][ nX ];
                        *pTmp++ = DMAP( nBlue, nThres );
                        *pTmp++ = DMAP( nGreen, nThres );
                        *pTmp++ = DMAP( nRed, nThres );
                    }
                }

                hNewBrush = CreateDIBPatternBrush( pSalData->mhDitherDIB, DIB_RGB_COLORS );
            }
            else if ( ImplIsSysColorEntry( nSalColor ) )
            {
                nBrushColor = PALRGB_TO_RGB( nBrushColor );
                hNewBrush = CreateSolidBrush( nBrushColor );
            }
            else if ( ImplIsPaletteEntry( nRed, nGreen, nBlue ) )
                hNewBrush = CreateSolidBrush( nBrushColor );
            else
            {
                BYTE* pTmp = pSalData->mpDitherDIBData;
                long* pDitherDiff = pSalData->mpDitherDiff;
                BYTE* pDitherLow = pSalData->mpDitherLow;
                BYTE* pDitherHigh = pSalData->mpDitherHigh;

                for ( long nY = 0L; nY < 8L; nY++ )
                {
                    for ( long nX = 0L; nX < 8L; nX++ )
                    {
                        const long nThres = aOrdDither8Bit[ nY ][ nX ];
                        *pTmp = DMAP( nRed, nThres ) + DMAP( nGreen, nThres ) * 6 + DMAP( nBlue, nThres ) * 36;
                        pTmp++;
                    }
                }

                hNewBrush = CreateDIBPatternBrush( pSalData->mhDitherDIB, DIB_PAL_COLORS );
            }
        }

        bStockBrush = FALSE;
    }

    // select new brush
    HBRUSH hOldBrush = SelectBrush( getHDC(), hNewBrush );

    // destroy or save old brush
    if ( mhBrush )
    {
        if ( !mbStockBrush )
            DeleteBrush( mhBrush );
    }
    else
        mhDefBrush = hOldBrush;

    // set new data
    mnBrushColor = nBrushColor;
    mhBrush     = hNewBrush;
    mbBrush     = TRUE;
    mbStockBrush = bStockBrush;
}

// -----------------------------------------------------------------------

void WinSalGraphics::SetXORMode( bool bSet, bool )
{
    mbXORMode = bSet;
    ::SetROP2( getHDC(), bSet ? R2_XORPEN : R2_COPYPEN );
}

// -----------------------------------------------------------------------

void WinSalGraphics::SetROPLineColor( SalROPColor nROPColor )
{
    SetLineColor( ImplGetROPSalColor( nROPColor ) );
}

// -----------------------------------------------------------------------

void WinSalGraphics::SetROPFillColor( SalROPColor nROPColor )
{
    SetFillColor( ImplGetROPSalColor( nROPColor ) );
}

// -----------------------------------------------------------------------

void WinSalGraphics::drawPixel( long nX, long nY )
{
    if ( mbXORMode )
    {
        HBRUSH  hBrush = CreateSolidBrush( mnPenColor );
        HBRUSH  hOldBrush = SelectBrush( getHDC(), hBrush );
        PatBlt( getHDC(), (int)nX, (int)nY, (int)1, (int)1, PATINVERT );
        SelectBrush( getHDC(), hOldBrush );
        DeleteBrush( hBrush );
    }
    else
        SetPixel( getHDC(), (int)nX, (int)nY, mnPenColor );
}

// -----------------------------------------------------------------------

void WinSalGraphics::drawPixel( long nX, long nY, SalColor nSalColor )
{
    COLORREF nCol = PALETTERGB( SALCOLOR_RED( nSalColor ),
                                SALCOLOR_GREEN( nSalColor ),
                                SALCOLOR_BLUE( nSalColor ) );

    if ( !mbPrinter &&
         GetSalData()->mhDitherPal &&
         ImplIsSysColorEntry( nSalColor ) )
        nCol = PALRGB_TO_RGB( nCol );

    if ( mbXORMode )
    {
        HBRUSH  hBrush = CreateSolidBrush( nCol );
        HBRUSH  hOldBrush = SelectBrush( getHDC(), hBrush );
        PatBlt( getHDC(), (int)nX, (int)nY, (int)1, (int)1, PATINVERT );
        SelectBrush( getHDC(), hOldBrush );
        DeleteBrush( hBrush );
    }
    else
        ::SetPixel( getHDC(), (int)nX, (int)nY, nCol );
}

// -----------------------------------------------------------------------

void WinSalGraphics::drawLine( long nX1, long nY1, long nX2, long nY2 )
{
    MoveToEx( getHDC(), (int)nX1, (int)nY1, NULL );

    // we must paint the endpoint
    int bPaintEnd = TRUE;
    if ( nX1 == nX2 )
    {
        bPaintEnd = FALSE;
        if ( nY1 <= nY2 )
            nY2++;
        else
            nY2--;
    }
    if ( nY1 == nY2 )
    {
        bPaintEnd = FALSE;
        if ( nX1 <= nX2 )
            nX2++;
        else
            nX2--;
    }

    LineTo( getHDC(), (int)nX2, (int)nY2 );

    if ( bPaintEnd && !mbPrinter )
    {
        if ( mbXORMode )
        {
            HBRUSH  hBrush = CreateSolidBrush( mnPenColor );
            HBRUSH  hOldBrush = SelectBrush( getHDC(), hBrush );
            PatBlt( getHDC(), (int)nX2, (int)nY2, (int)1, (int)1, PATINVERT );
            SelectBrush( getHDC(), hOldBrush );
            DeleteBrush( hBrush );
        }
        else
            SetPixel( getHDC(), (int)nX2, (int)nY2, mnPenColor );
    }
}

// -----------------------------------------------------------------------

void WinSalGraphics::drawRect( long nX, long nY, long nWidth, long nHeight )
{
    if ( !mbPen )
    {
        if ( !mbPrinter )
        {
            PatBlt( getHDC(), (int)nX, (int)nY, (int)nWidth, (int)nHeight,
                    mbXORMode ? PATINVERT : PATCOPY );
        }
        else
        {
            RECT aWinRect;
            aWinRect.left   = nX;
            aWinRect.top    = nY;
            aWinRect.right  = nX+nWidth;
            aWinRect.bottom = nY+nHeight;
            ::FillRect( getHDC(), &aWinRect, mhBrush );
        }
    }
    else
        WIN_Rectangle( getHDC(), (int)nX, (int)nY, (int)(nX+nWidth), (int)(nY+nHeight) );
}

// -----------------------------------------------------------------------

void WinSalGraphics::drawPolyLine( sal_uLong nPoints, const SalPoint* pPtAry )
{
    // for NT, we can handover the array directly
    DBG_ASSERT( sizeof( POINT ) == sizeof( SalPoint ),
                "WinSalGraphics::DrawPolyLine(): POINT != SalPoint" );

    POINT* pWinPtAry = (POINT*)pPtAry;

    // we assume there are at least 2 points (Polyline requres at least 2 point, see MSDN)
    // we must paint the endpoint for last line
    BOOL bPaintEnd = TRUE;
    if ( pWinPtAry[nPoints-2].x == pWinPtAry[nPoints-1].x )
    {
        bPaintEnd = FALSE;
        if ( pWinPtAry[nPoints-2].y <=  pWinPtAry[nPoints-1].y )
            pWinPtAry[nPoints-1].y++;
        else
            pWinPtAry[nPoints-1].y--;
    }
    if ( pWinPtAry[nPoints-2].y == pWinPtAry[nPoints-1].y )
    {
        bPaintEnd = FALSE;
        if ( pWinPtAry[nPoints-2].x <= pWinPtAry[nPoints-1].x )
            pWinPtAry[nPoints-1].x++;
        else
            pWinPtAry[nPoints-1].x--;
    }

    // for Windows 95 and its maximum number of points
    if ( !Polyline( getHDC(), pWinPtAry, (int)nPoints ) && (nPoints > MAX_64KSALPOINTS) )
        Polyline( getHDC(), pWinPtAry, MAX_64KSALPOINTS );

    if ( bPaintEnd && !mbPrinter )
    {
        if ( mbXORMode )
        {
            HBRUSH     hBrush = CreateSolidBrush( mnPenColor );
            HBRUSH     hOldBrush = SelectBrush( getHDC(), hBrush );
            PatBlt( getHDC(), (int)(pWinPtAry[nPoints-1].x), (int)(pWinPtAry[nPoints-1].y), (int)1, (int)1, PATINVERT );
            SelectBrush( getHDC(), hOldBrush );
            DeleteBrush( hBrush );
        }
        else
            SetPixel( getHDC(), (int)(pWinPtAry[nPoints-1].x), (int)(pWinPtAry[nPoints-1].y), mnPenColor );
    }
}

// -----------------------------------------------------------------------

void WinSalGraphics::drawPolygon( sal_uLong nPoints, const SalPoint* pPtAry )
{
    // for NT, we can handover the array directly
    DBG_ASSERT( sizeof( POINT ) == sizeof( SalPoint ),
                "WinSalGraphics::DrawPolygon(): POINT != SalPoint" );

    POINT* pWinPtAry = (POINT*)pPtAry;
    // for Windows 95 and its maximum number of points
    if ( !WIN_Polygon( getHDC(), pWinPtAry, (int)nPoints ) && (nPoints > MAX_64KSALPOINTS) )
        WIN_Polygon( getHDC(), pWinPtAry, MAX_64KSALPOINTS );
}

// -----------------------------------------------------------------------

void WinSalGraphics::drawPolyPolygon( sal_uInt32 nPoly, const sal_uInt32* pPoints,
                                   PCONSTSALPOINT* pPtAry )
{
    UINT    aWinPointAry[SAL_POLYPOLYCOUNT_STACKBUF];
    UINT*   pWinPointAry;
    UINT    nPolyPolyPoints = 0;
    UINT    nPoints;
    UINT    i;

    if ( nPoly <= SAL_POLYPOLYCOUNT_STACKBUF )
        pWinPointAry = aWinPointAry;
    else
        pWinPointAry = new UINT[nPoly];

    for ( i = 0; i < (UINT)nPoly; i++ )
    {
        nPoints = (UINT)pPoints[i]+1;
        pWinPointAry[i] = nPoints;
        nPolyPolyPoints += nPoints;
    }

    POINT  aWinPointAryAry[SAL_POLYPOLYPOINTS_STACKBUF];
    POINT* pWinPointAryAry;
    if ( nPolyPolyPoints <= SAL_POLYPOLYPOINTS_STACKBUF )
        pWinPointAryAry = aWinPointAryAry;
    else
        pWinPointAryAry = new POINT[nPolyPolyPoints];
    // for NT, we can handover the array directly
   DBG_ASSERT( sizeof( POINT ) == sizeof( SalPoint ),
                "WinSalGraphics::DrawPolyPolygon(): POINT != SalPoint" );
    const SalPoint* pPolyAry;
    UINT            n = 0;
    for ( i = 0; i < (UINT)nPoly; i++ )
    {
        nPoints = pWinPointAry[i];
        pPolyAry = pPtAry[i];
        memcpy( pWinPointAryAry+n, pPolyAry, (nPoints-1)*sizeof(POINT) );
        pWinPointAryAry[n+nPoints-1] = pWinPointAryAry[n];
        n += nPoints;
    }

    if ( !WIN_PolyPolygon( getHDC(), pWinPointAryAry, (int*)pWinPointAry, (UINT)nPoly ) &&
         (nPolyPolyPoints > MAX_64KSALPOINTS) )
    {
        nPolyPolyPoints  = 0;
        nPoly = 0;
        do
        {
            nPolyPolyPoints += pWinPointAry[(UINT)nPoly];
            nPoly++;
        }
        while ( nPolyPolyPoints < MAX_64KSALPOINTS );
        nPoly--;
        if ( pWinPointAry[(UINT)nPoly] > MAX_64KSALPOINTS )
            pWinPointAry[(UINT)nPoly] = MAX_64KSALPOINTS;
        if ( nPoly == 1 )
            WIN_Polygon( getHDC(), pWinPointAryAry, *pWinPointAry );
        else
            WIN_PolyPolygon( getHDC(), pWinPointAryAry, (int*)pWinPointAry, nPoly );
    }

    if ( pWinPointAry != aWinPointAry )
        delete [] pWinPointAry;
    if ( pWinPointAryAry != aWinPointAryAry )
        delete [] pWinPointAryAry;
}

// -----------------------------------------------------------------------

#define SAL_POLY_STACKBUF       32

// -----------------------------------------------------------------------

sal_Bool WinSalGraphics::drawPolyLineBezier( sal_uLong nPoints, const SalPoint* pPtAry, const BYTE* pFlgAry )
{
#ifdef USE_GDI_BEZIERS
    // for NT, we can handover the array directly
    DBG_ASSERT( sizeof( POINT ) == sizeof( SalPoint ),
                "WinSalGraphics::DrawPolyLineBezier(): POINT != SalPoint" );

    ImplRenderPath( getHDC(), nPoints, pPtAry, pFlgAry );

    return sal_True;
#else
    return sal_False;
#endif
}

// -----------------------------------------------------------------------

sal_Bool WinSalGraphics::drawPolygonBezier( sal_uLong nPoints, const SalPoint* pPtAry, const BYTE* pFlgAry )
{
#ifdef USE_GDI_BEZIERS
    // for NT, we can handover the array directly
    DBG_ASSERT( sizeof( POINT ) == sizeof( SalPoint ),
                "WinSalGraphics::DrawPolygonBezier(): POINT != SalPoint" );

    POINT   aStackAry1[SAL_POLY_STACKBUF];
    BYTE    aStackAry2[SAL_POLY_STACKBUF];
    POINT*  pWinPointAry;
    BYTE*   pWinFlagAry;
    if( nPoints > SAL_POLY_STACKBUF )
    {
        pWinPointAry = new POINT[ nPoints ];
        pWinFlagAry = new BYTE[ nPoints ];
    }
    else
    {
        pWinPointAry = aStackAry1;
        pWinFlagAry = aStackAry2;
    }

    sal_uInt32 nPoints_i32(nPoints);
    ImplPreparePolyDraw(true, 1, &nPoints_i32, &pPtAry, &pFlgAry, pWinPointAry, pWinFlagAry);

    sal_Bool bRet( sal_False );

    if( BeginPath( getHDC() ) )
    {
        PolyDraw(getHDC(), pWinPointAry, pWinFlagAry, nPoints);

        if( EndPath( getHDC() ) )
        {
            if( StrokeAndFillPath( getHDC() ) )
                bRet = sal_True;
        }
    }

    if( pWinPointAry != aStackAry1 )
    {
        delete [] pWinPointAry;
        delete [] pWinFlagAry;
    }

    return bRet;
#else
    return sal_False;
#endif
}

// -----------------------------------------------------------------------

sal_Bool WinSalGraphics::drawPolyPolygonBezier( sal_uInt32 nPoly, const sal_uInt32* pPoints,
                                             const SalPoint* const* pPtAry, const BYTE* const* pFlgAry )
{
#ifdef USE_GDI_BEZIERS
    // for NT, we can handover the array directly
    DBG_ASSERT( sizeof( POINT ) == sizeof( SalPoint ),
                "WinSalGraphics::DrawPolyPolygonBezier(): POINT != SalPoint" );

    sal_uLong nCurrPoly, nTotalPoints;
    const sal_uInt32* pCurrPoints = pPoints;
    for( nCurrPoly=0, nTotalPoints=0; nCurrPoly<nPoly; ++nCurrPoly )
        nTotalPoints += *pCurrPoints++;

    POINT   aStackAry1[SAL_POLY_STACKBUF];
    BYTE    aStackAry2[SAL_POLY_STACKBUF];
    POINT*  pWinPointAry;
    BYTE*   pWinFlagAry;
    if( nTotalPoints > SAL_POLY_STACKBUF )
    {
        pWinPointAry = new POINT[ nTotalPoints ];
        pWinFlagAry = new BYTE[ nTotalPoints ];
    }
    else
    {
        pWinPointAry = aStackAry1;
        pWinFlagAry = aStackAry2;
    }

    ImplPreparePolyDraw(true, nPoly, pPoints, pPtAry, pFlgAry, pWinPointAry, pWinFlagAry);

    sal_Bool bRet( sal_False );

    if( BeginPath( getHDC() ) )
    {
        PolyDraw(getHDC(), pWinPointAry, pWinFlagAry, nTotalPoints);

        if( EndPath( getHDC() ) )
        {
            if( StrokeAndFillPath( getHDC() ) )
                bRet = sal_True;
        }
    }

    if( pWinPointAry != aStackAry1 )
    {
        delete [] pWinPointAry;
        delete [] pWinFlagAry;
    }

    return bRet;
#else
    return sal_False;
#endif
}

// -----------------------------------------------------------------------

#define POSTSCRIPT_BUFSIZE 0x4000           // MAXIMUM BUFSIZE EQ 0xFFFF

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

static sal_Bool ImplGetBoundingBox( double* nNumb, BYTE* pSource, sal_uLong nSize )
{
    sal_Bool    bRetValue = FALSE;
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
            sal_Bool    bDivision = FALSE;
            sal_Bool    bNegative = FALSE;
            sal_Bool    bValid = TRUE;

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

sal_Bool WinSalGraphics::drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, sal_uLong nSize )
{
    sal_Bool bRetValue = FALSE;

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
                // ----------------------------------------------------------------------------------

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
                // ----------------------------------------------------------------------------------

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
                // ----------------------------------------------------------------------------------
                *((sal_uInt16*)aBuf.getStr()) = (sal_uInt16)( aBuf.getLength() - 2 );
                Escape ( getHDC(), nEscape, aBuf.getLength(), (LPTSTR)aBuf.getStr(), 0 );


                // #107797# Write out EPS transformation code
                // ----------------------------------------------------------------------------------
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
                // ----------------------------------------------------------------------------------
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
                // ----------------------------------------------------------------------------------
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

// -----------------------------------------------------------------------

SystemGraphicsData WinSalGraphics::GetGraphicsData() const
{
    SystemGraphicsData aRes;
    aRes.nSize = sizeof(aRes);
    aRes.hDC = const_cast< WinSalGraphics* >(this)->getHDC();
    return aRes;
}

// -----------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
