/*************************************************************************
 *
 *  $RCSfile: salgdi.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-18 14:51:27 $
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

#include <stdio.h>
#include <string.h>

#ifndef _SVWIN_H
#include <tools/svwin.h>
#endif
#define _SV_SALGDI_CXX

#ifndef _SV_WINCOMP_HXX
#include <wincomp.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALGDI_H
#include <salgdi.h>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_SALFRAME_H
#include <salframe.h>
#endif
#ifndef _SV_POLY_HXX
#include "poly.hxx"
#endif

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
     0, 38,  9, 48,  2, 40, 12, 50,
    25, 12, 35, 22, 28, 15, 37, 24,
     6, 44,  3, 41,  8, 47,  5, 44,
    32, 19, 28, 16, 34, 21, 31, 18,
     1, 40, 11, 49,  0, 39, 10, 48,
    27, 14, 36, 24, 26, 13, 36, 23,
     8, 46,  4, 43,  7, 45,  4, 42,
    33, 20, 30, 17, 32, 20, 29, 16
};

// -----------------------------------------------------------------------------

static BYTE aOrdDither16Bit[8][8] =
{
    0, 6, 1, 7, 0, 6, 1, 7,
    4, 2, 5, 3, 4, 2, 5, 3,
    1, 7, 0, 6, 1, 7, 0, 6,
    5, 3, 4, 2, 5, 3, 4, 2,
    0, 6, 1, 7, 0, 6, 1, 7,
    4, 2, 5, 3, 4, 2, 5, 3,
    1, 7, 0, 6, 1, 7, 0, 6,
    5, 3, 4, 2, 5, 3, 4, 2
};

// =======================================================================

// Pens muessen wir mit 1 Pixel-Breite erzeugen, da ansonsten die S3-Karte
// viele Paintprobleme hat, wenn Polygone/PolyLines gezeichnet werden und
// eine komplexe ClipRegion gesetzt ist
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
        // test, if we have to dither
        HDC         hMemDC = ::CreateCompatibleDC( hDC );
        HBITMAP     hMemBmp = ::CreateCompatibleBitmap( hDC, 8, 8 );
        HBITMAP     hBmpOld = (HBITMAP) ::SelectObject( hMemDC, hMemBmp );
        HBRUSH      hMemBrush = ::CreateSolidBrush( PALETTERGB( 175, 171, 169 ) );
        HBRUSH      hBrushOld = (HBRUSH) ::SelectObject( hMemDC, hMemBrush );
        BOOL        bDither16 = TRUE;

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
                pSalData->mpDitherHigh[ n ] = (BYTE) Min( pSalData->mpDitherLow[ n ] + 8L, 255L );
        }
    }
    else if ( (nRasterCaps & RC_PALETTE) && (nBitCount == 8) )
    {
        BYTE            nRed, nGreen, nBlue;
        BYTE            nR, nG, nB;
        PALETTEENTRY*   pPalEntry;
        LOGPALETTE*     pLogPal;
        const USHORT    nDitherPalCount = DITHER_PAL_COUNT;
        ULONG           nTotalCount = DITHER_MAX_SYSCOLOR + nDitherPalCount + DITHER_EXTRA_COLORS;

        // create logical palette
        pLogPal = (LOGPALETTE*) new char[ sizeof( LOGPALETTE ) + ( nTotalCount * sizeof( PALETTEENTRY ) ) ];
        pLogPal->palVersion = 0x0300;
        pLogPal->palNumEntries = (USHORT) nTotalCount;
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
                pSalData->mpDitherHigh[ n ] = Min( pSalData->mpDitherLow[ n ] + 1, 5 );
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

    // 50% Brush loeschen
    if ( pSalData->mh50Brush )
    {
        DeleteBrush( pSalData->mh50Brush );
        pSalData->mh50Brush = 0;
    }

    // 50% Bitmap loeschen
    if ( pSalData->mh50Bmp )
    {
        DeleteBitmap( pSalData->mh50Bmp );
        pSalData->mh50Bmp = 0;
    }

    ImplClearHDCCache( pSalData );
    delete[] pSalData->mpHDCCache;

    // Ditherpalette loeschen, wenn vorhanden
    if ( pSalData->mhDitherPal )
    {
        DeleteObject( pSalData->mhDitherPal );
        pSalData->mhDitherPal = 0;
    }

    // delete buffers for dithering DIB patterns, if neccessary
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
    for ( USHORT i = 0; i < DITHER_MAX_SYSCOLOR; i++, pPalEntry++ )
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
    if( aSalShlData.mnVersion >= 410 )
    {
        ImplInsertSysColorEntry( COLOR_GRADIENTACTIVECAPTION );
        ImplInsertSysColorEntry( COLOR_GRADIENTINACTIVECAPTION );
    }
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
    // Beim Printer berechnen wir die minimale Linienstaerke
    if ( pData->mbPrinter )
    {
        int nDPIX = GetDeviceCaps( pData->mhDC, LOGPIXELSX );
        if ( nDPIX <= 300 )
            pData->mnPenWidth = 0;
        else
            pData->mnPenWidth = nDPIX/300;
    }

    ::SetTextAlign( pData->mhDC, TA_BASELINE | TA_LEFT | TA_NOUPDATECP );
    ::SetBkMode( pData->mhDC, TRANSPARENT );
    ::SetROP2( pData->mhDC, R2_COPYPEN );
}

// -----------------------------------------------------------------------

void ImplSalDeInitGraphics( WinSalGraphics* pData )
{
    // Default Objekte selektieren
    if ( pData->mhDefPen )
        SelectPen( pData->mhDC, pData->mhDefPen );
    if ( pData->mhDefBrush )
        SelectBrush( pData->mhDC, pData->mhDefBrush );
    if ( pData->mhDefFont )
        SelectFont( pData->mhDC, pData->mhDefFont );
}

// =======================================================================

HDC ImplGetCachedDC( ULONG nID, HBITMAP hBmp )
{
    SalData*    pSalData = GetSalData();
    HDCCache*   pC = &pSalData->mpHDCCache[ nID ];

    if( !pC->mhDC )
    {
        HDC hDC = GetDC( 0 );

        // neuen DC mit DefaultBitmap anlegen
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

void ImplReleaseCachedDC( ULONG nID )
{
    SalData*    pSalData = GetSalData();
    HDCCache*   pC = &pSalData->mpHDCCache[ nID ];

    if ( pC->mhActBmp )
        SelectObject( pC->mhDC, pC->mhSelBmp );
}

// =======================================================================

void ImplClearHDCCache( SalData* pData )
{
    for( ULONG i = 0; i < CACHESIZE_HDC; i++ )
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
                          ULONG                     nPoly,
                          const ULONG*              pPoints,
                          const SalPoint* const*    pPtAry,
                          const BYTE* const*        pFlgAry,
                          POINT*                    pWinPointAry,
                          BYTE*                     pWinFlagAry     )
{
    ULONG nCurrPoly;
    for( nCurrPoly=0; nCurrPoly<nPoly; ++nCurrPoly )
    {
        const POINT* pCurrPoint = reinterpret_cast<const POINT*>( *pPtAry++ );
        const BYTE* pCurrFlag = *pFlgAry++;
        const ULONG nCurrPoints = *pPoints++;
        const bool bHaveFlagArray( pCurrFlag );
        ULONG nCurrPoint;

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
void ImplRenderPath( HDC hdc, ULONG nPoints, const SalPoint* pPtAry, const BYTE* pFlgAry )
{
    if( nPoints )
    {
        USHORT i;
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
        mhFonts[ i ] = 0;
    mhDC                = 0;
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
    // free obsolete GDI objekts
    for( int i = 0; i < MAX_FALLBACK; ++i )
        if( mhFonts[ i ] )
            DeleteFont( mhFonts[ i ] );

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

    // Cache-Daten zerstoeren
    if ( mpStdClipRgnData )
        delete [] mpStdClipRgnData;

    if ( mpLogFont )
        delete mpLogFont;

    if ( mpFontCharSets )
        delete mpFontCharSets;

    if ( mpFontKernPairs )
        delete mpFontKernPairs;
}

// -----------------------------------------------------------------------

void WinSalGraphics::GetResolution( long& rDPIX, long& rDPIY )
{
    rDPIX = GetDeviceCaps( mhDC, LOGPIXELSX );
    rDPIY = GetDeviceCaps( mhDC, LOGPIXELSY );
}

// -----------------------------------------------------------------------

void WinSalGraphics::GetScreenFontResolution( long& rDPIX, long& rDPIY )
{
    rDPIX = GetDeviceCaps( mhDC, LOGPIXELSX );
    rDPIY = GetDeviceCaps( mhDC, LOGPIXELSY );
}

// -----------------------------------------------------------------------

USHORT WinSalGraphics::GetBitCount()
{
    return (USHORT)GetDeviceCaps( mhDC, BITSPIXEL );
}

// -----------------------------------------------------------------------

long WinSalGraphics::GetGraphicsWidth()
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

    SelectClipRgn( mhDC, 0 );
}

// -----------------------------------------------------------------------

void WinSalGraphics::BeginSetClipRegion( ULONG nRectCount )
{
    if ( mhRegion )
    {
        DeleteRegion( mhRegion );
        mhRegion = 0;
    }

    ULONG nRectBufSize = sizeof(RECT)*nRectCount;
    if ( nRectCount < SAL_CLIPRECT_COUNT )
    {
        if ( !mpStdClipRgnData )
            mpStdClipRgnData = (RGNDATA*)new BYTE[sizeof(RGNDATA)-1+(SAL_CLIPRECT_COUNT*sizeof(RECT))];
        mpClipRgnData = mpStdClipRgnData;
    }
    else
        mpClipRgnData = (RGNDATA*)new BYTE[sizeof(RGNDATA)-1+nRectBufSize];
    mpClipRgnData->rdh.dwSize   = sizeof( RGNDATAHEADER );
    mpClipRgnData->rdh.iType    = RDH_RECTANGLES;
    mpClipRgnData->rdh.nCount   = nRectCount;
    mpClipRgnData->rdh.nRgnSize = nRectBufSize;
    SetRectEmpty( &(mpClipRgnData->rdh.rcBound) );
    mpNextClipRect          = (RECT*)(&(mpClipRgnData->Buffer));
    mbFirstClipRect         = TRUE;
}


// -----------------------------------------------------------------------

BOOL WinSalGraphics::unionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
    if ( nWidth && nHeight )
    {
        RECT*       pRect = mpNextClipRect;
        RECT*       pBoundRect = &(mpClipRgnData->rdh.rcBound);
        long        nRight = nX + nWidth;
        long        nBottom = nY + nHeight;

        if ( mbFirstClipRect )
        {
            pBoundRect->left    = nX;
            pBoundRect->top     = nY;
            pBoundRect->right   = nRight;
            pBoundRect->bottom  = nBottom;
            mbFirstClipRect = FALSE;
        }
        else
        {
            if ( nX < pBoundRect->left )
                pBoundRect->left = (int)nX;

            if ( nY < pBoundRect->top )
                pBoundRect->top = (int)nY;

            if ( nRight > pBoundRect->right )
                pBoundRect->right = (int)nRight;

            if ( nBottom > pBoundRect->bottom )
                pBoundRect->bottom = (int)nBottom;
        }

        pRect->left     = (int)nX;
        pRect->top      = (int)nY;
        pRect->right    = (int)nRight;
        pRect->bottom   = (int)nBottom;
        mpNextClipRect++;
    }
    else
    {
        mpClipRgnData->rdh.nCount--;
        mpClipRgnData->rdh.nRgnSize -= sizeof( RECT );
    }

    return TRUE;
}

// -----------------------------------------------------------------------

void WinSalGraphics::EndSetClipRegion()
{
    // create clip region from ClipRgnData
    if ( mpClipRgnData->rdh.nCount == 1 )
    {
        RECT* pRect = &(mpClipRgnData->rdh.rcBound);
        mhRegion = CreateRectRgn( pRect->left, pRect->top,
                                                 pRect->right, pRect->bottom );
    }
    else
    {
        ULONG nSize = mpClipRgnData->rdh.nRgnSize+sizeof(RGNDATAHEADER);
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

                for( ULONG n = 1; n < pHeader->nCount; n++, pRect++ )
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

    SelectClipRgn( mhDC, mhRegion );
}

// -----------------------------------------------------------------------

void WinSalGraphics::SetLineColor()
{
    // create and select new pen
    HPEN hNewPen = GetStockPen( NULL_PEN );
    HPEN hOldPen = SelectPen( mhDC, hNewPen );

    // destory or save old pen
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
    COLORREF    nPenColor = PALETTERGB( SALCOLOR_RED( nSalColor ),
                                        SALCOLOR_GREEN( nSalColor ),
                                        SALCOLOR_BLUE( nSalColor ) );
    HPEN        hNewPen = 0;
    BOOL        bStockPen;

    // search for stock pen (only screen, because printer have problems,
    // when we use stock objects)
    if ( !mbPrinter )
    {
        SalData* pSalData = GetSalData();
        for ( USHORT i = 0; i < pSalData->mnStockPenCount; i++ )
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
    HPEN hOldPen = SelectPen( mhDC, hNewPen );

    // destory or save old pen
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
    HBRUSH hOldBrush = SelectBrush( mhDC, hNewBrush );

    // destory or save old brush
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
    SalData*    pSalData    = GetSalData();
    BYTE        nRed        = SALCOLOR_RED( nSalColor );
    BYTE        nGreen      = SALCOLOR_GREEN( nSalColor );
    BYTE        nBlue       = SALCOLOR_BLUE( nSalColor );
    COLORREF    nBrushColor = PALETTERGB( nRed, nGreen, nBlue );
    HBRUSH      hNewBrush   = 0;
    BOOL        bStockBrush;

    // search for stock brush (only screen, because printer have problems,
    // when we use stock objects)
    if ( !mbPrinter )
    {
        for ( USHORT i = 0; i < pSalData->mnStockBrushCount; i++ )
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
    HBRUSH hOldBrush = SelectBrush( mhDC, hNewBrush );

    // destory or save old brush
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
    mbBrush     = FALSE;
    mbStockBrush = bStockBrush;
}

// -----------------------------------------------------------------------

void WinSalGraphics::SetXORMode( BOOL bSet )
{
    mbXORMode = bSet;
    ::SetROP2( mhDC, bSet ? R2_XORPEN : R2_COPYPEN );
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
        HBRUSH  hOldBrush = SelectBrush( mhDC, hBrush );
        PatBlt( mhDC, (int)nX, (int)nY, (int)1, (int)1, PATINVERT );
        SelectBrush( mhDC, hOldBrush );
        DeleteBrush( hBrush );
    }
    else
        SetPixel( mhDC, (int)nX, (int)nY, mnPenColor );
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
        HBRUSH  hOldBrush = SelectBrush( mhDC, hBrush );
        PatBlt( mhDC, (int)nX, (int)nY, (int)1, (int)1, PATINVERT );
        SelectBrush( mhDC, hOldBrush );
        DeleteBrush( hBrush );
    }
    else
        ::SetPixel( mhDC, (int)nX, (int)nY, nCol );
}

// -----------------------------------------------------------------------

void WinSalGraphics::drawLine( long nX1, long nY1, long nX2, long nY2 )
{
    MoveToEx( mhDC, (int)nX1, (int)nY1, NULL );

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

    LineTo( mhDC, (int)nX2, (int)nY2 );

    if ( bPaintEnd && !mbPrinter )
    {
        if ( mbXORMode )
        {
            HBRUSH  hBrush = CreateSolidBrush( mnPenColor );
            HBRUSH  hOldBrush = SelectBrush( mhDC, hBrush );
            PatBlt( mhDC, (int)nX2, (int)nY2, (int)1, (int)1, PATINVERT );
            SelectBrush( mhDC, hOldBrush );
            DeleteBrush( hBrush );
        }
        else
            SetPixel( mhDC, (int)nX2, (int)nY2, mnPenColor );
    }
}

// -----------------------------------------------------------------------

void WinSalGraphics::drawRect( long nX, long nY, long nWidth, long nHeight )
{
    if ( !mbPen )
    {
        if ( !mbPrinter )
        {
            PatBlt( mhDC, (int)nX, (int)nY, (int)nWidth, (int)nHeight,
                    mbXORMode ? PATINVERT : PATCOPY );
        }
        else
        {
            RECT aWinRect;
            aWinRect.left   = nX;
            aWinRect.top    = nY;
            aWinRect.right  = nX+nWidth;
            aWinRect.bottom = nY+nHeight;
            ::FillRect( mhDC, &aWinRect, mhBrush );
        }
    }
    else
        WIN_Rectangle( mhDC, (int)nX, (int)nY, (int)(nX+nWidth), (int)(nY+nHeight) );
}

// -----------------------------------------------------------------------

void WinSalGraphics::drawPolyLine( ULONG nPoints, const SalPoint* pPtAry )
{
    // Unter NT koennen wir das Array direkt weiterreichen
    DBG_ASSERT( sizeof( POINT ) == sizeof( SalPoint ),
                "WinSalGraphics::DrawPolyLine(): POINT != SalPoint" );

    POINT* pWinPtAry = (POINT*)pPtAry;
    // Wegen Windows 95 und der Beschraenkung auf eine maximale Anzahl
    // von Punkten
    if ( !Polyline( mhDC, pWinPtAry, (int)nPoints ) && (nPoints > MAX_64KSALPOINTS) )
        Polyline( mhDC, pWinPtAry, MAX_64KSALPOINTS );
}

// -----------------------------------------------------------------------

void WinSalGraphics::drawPolygon( ULONG nPoints, const SalPoint* pPtAry )
{
    // Unter NT koennen wir das Array direkt weiterreichen
    DBG_ASSERT( sizeof( POINT ) == sizeof( SalPoint ),
                "WinSalGraphics::DrawPolygon(): POINT != SalPoint" );

    POINT* pWinPtAry = (POINT*)pPtAry;
    // Wegen Windows 95 und der Beschraenkung auf eine maximale Anzahl
    // von Punkten
    if ( !WIN_Polygon( mhDC, pWinPtAry, (int)nPoints ) && (nPoints > MAX_64KSALPOINTS) )
        WIN_Polygon( mhDC, pWinPtAry, MAX_64KSALPOINTS );
}

// -----------------------------------------------------------------------

void WinSalGraphics::drawPolyPolygon( ULONG nPoly, const ULONG* pPoints,
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
    // Unter NT koennen wir das Array direkt weiterreichen
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

    if ( !WIN_PolyPolygon( mhDC, pWinPointAryAry, (int*)pWinPointAry, (UINT)nPoly ) &&
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
            WIN_Polygon( mhDC, pWinPointAryAry, *pWinPointAry );
        else
            WIN_PolyPolygon( mhDC, pWinPointAryAry, (int*)pWinPointAry, nPoly );
    }

    if ( pWinPointAry != aWinPointAry )
        delete [] pWinPointAry;
    if ( pWinPointAryAry != aWinPointAryAry )
        delete [] pWinPointAryAry;
}

// -----------------------------------------------------------------------

#define SAL_POLY_STACKBUF       32

// -----------------------------------------------------------------------

sal_Bool WinSalGraphics::drawPolyLineBezier( ULONG nPoints, const SalPoint* pPtAry, const BYTE* pFlgAry )
{
#ifdef USE_GDI_BEZIERS
    // Unter NT koennen wir das Array direkt weiterreichen
    DBG_ASSERT( sizeof( POINT ) == sizeof( SalPoint ),
                "WinSalGraphics::DrawPolyLineBezier(): POINT != SalPoint" );

    ImplRenderPath( mhDC, nPoints, pPtAry, pFlgAry );

    return sal_True;
#else
    return sal_False;
#endif
}

// -----------------------------------------------------------------------

sal_Bool WinSalGraphics::drawPolygonBezier( ULONG nPoints, const SalPoint* pPtAry, const BYTE* pFlgAry )
{
#ifdef USE_GDI_BEZIERS
    // Unter NT koennen wir das Array direkt weiterreichen
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

    ImplPreparePolyDraw(true, 1, &nPoints, &pPtAry, &pFlgAry, pWinPointAry, pWinFlagAry);

    sal_Bool bRet( sal_False );

    if( BeginPath( mhDC ) )
    {
        PolyDraw(mhDC, pWinPointAry, pWinFlagAry, nPoints);

        if( EndPath( mhDC ) )
        {
            if( StrokeAndFillPath( mhDC ) )
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

sal_Bool WinSalGraphics::drawPolyPolygonBezier( ULONG nPoly, const ULONG* pPoints,
                                             const SalPoint* const* pPtAry, const BYTE* const* pFlgAry )
{
#ifdef USE_GDI_BEZIERS
    // Unter NT koennen wir das Array direkt weiterreichen
    DBG_ASSERT( sizeof( POINT ) == sizeof( SalPoint ),
                "WinSalGraphics::DrawPolyPolygonBezier(): POINT != SalPoint" );

    ULONG nCurrPoly, nTotalPoints;
    const ULONG* pCurrPoints = pPoints;
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

    if( BeginPath( mhDC ) )
    {
        PolyDraw(mhDC, pWinPointAry, pWinFlagAry, nTotalPoints);

        if( EndPath( mhDC ) )
        {
            if( StrokeAndFillPath( mhDC ) )
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
#define POSTSCRIPT_BOUNDINGSEARCH 0x1000    // we only try to get the BoundingBox
                                            // in the first 4096 bytes

static BYTE* ImplSearchEntry( BYTE* pSource, BYTE* pDest, ULONG nComp, ULONG nSize )
{
    while ( nComp-- >= nSize )
    {
        for ( ULONG i = 0; i < nSize; i++ )
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

static BOOL ImplGetBoundingBox( double* nNumb, BYTE* pSource, ULONG nSize )
{
    BOOL    bRetValue = FALSE;
    BYTE* pDest = ImplSearchEntry( pSource, (BYTE*)"%%BoundingBox:", nSize, 14 );
    if ( pDest )
    {
        nNumb[0] = nNumb[1] = nNumb[2] = nNumb[3] = 0;
        pDest += 14;

        int nSizeLeft = nSize - ( pDest - pSource );
        if ( nSizeLeft > 100 )
            nSizeLeft = 100;    // only 100 bytes following the bounding box will be checked

        for ( int i = 0; ( i < 4 ) && nSizeLeft; i++ )
        {
            int     nDivision = 1;
            BOOL    bDivision = FALSE;
            BOOL    bNegative = FALSE;
            BOOL    bValid = TRUE;

            while ( ( --nSizeLeft ) && ( *pDest == ' ' ) || ( *pDest == 0x9 ) ) pDest++;
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

inline void ImplWriteDouble( BYTE** pBuf, double nNumb )
{
    *pBuf += sprintf( (char*)*pBuf, "%f", nNumb );
    *(*pBuf)++ = ' ';
}

inline void ImplWriteString( BYTE** pBuf, const char* sString )
{
    strcpy( (char*)*pBuf, sString );
    *pBuf += strlen( sString );
}

BOOL WinSalGraphics::drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, ULONG nSize )
{
    BOOL bRetValue = FALSE;

    if ( mbPrinter )
    {
        int nEscape = POSTSCRIPT_PASSTHROUGH;

        if ( Escape( mhDC, QUERYESCSUPPORT, sizeof( int ), ( LPSTR )&nEscape, 0 ) )
        {
            BYTE* pBuf = new BYTE[ POSTSCRIPT_BUFSIZE ];

            double  nBoundingBox[4];

            if ( pBuf && ImplGetBoundingBox( nBoundingBox, (BYTE*)pPtr, nSize ) )
            {
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
                BYTE*   pTemp = pBuf + 2;       // +2 because we want to insert the size later
                ImplWriteString( &pTemp, "\n\n/b4_Inc_state_salWin save def\n" );
                ImplWriteString( &pTemp, "/dict_count_salWin countdictstack def\n" );
                ImplWriteString( &pTemp, "/op_count_salWin count 1 sub def\n" );
                ImplWriteString( &pTemp, "userdict begin\n" );
                ImplWriteString( &pTemp, "/showpage {} def\n" );
                ImplWriteString( &pTemp, "0 setgray 0 setlinecap\n" );
                ImplWriteString( &pTemp, "1 setlinewidth 0 setlinejoin\n" );
                ImplWriteString( &pTemp, "10 setmiterlimit [] 0 setdash newpath\n" );
                ImplWriteString( &pTemp, "/languagelevel where\n" );
                ImplWriteString( &pTemp, "{\n" );
                ImplWriteString( &pTemp, "  pop languagelevel\n" );
                ImplWriteString( &pTemp, "  1 ne\n" );
                ImplWriteString( &pTemp, "  {\n" );
                ImplWriteString( &pTemp, "    false setstrokeadjust false setoverprint\n" );
                ImplWriteString( &pTemp, "  } if\n" );
                ImplWriteString( &pTemp, "} if\n\n" );
                *((USHORT*)pBuf) = (USHORT)( pTemp - pBuf - 2 );
                Escape ( mhDC, nEscape, pTemp - pBuf, (LPTSTR)((BYTE*)pBuf), 0 );


                // #107797# Write out EPS transformation code
                // ----------------------------------------------------------------------------------
                double  dM11 = nWidth / ( nBoundingBox[2] - nBoundingBox[0] );
                double  dM22 = nHeight / (nBoundingBox[1] - nBoundingBox[3] );
                pTemp = pBuf + 2;       // +2 because we want to insert the size later
                ImplWriteString( &pTemp, "\n\n[ " );
                ImplWriteDouble( &pTemp, dM11 );
                ImplWriteDouble( &pTemp, 0 );
                ImplWriteDouble( &pTemp, 0 );
                ImplWriteDouble( &pTemp, dM22 );
                ImplWriteDouble( &pTemp, nX - ( dM11 * nBoundingBox[0] ) );
                ImplWriteDouble( &pTemp, nY - ( dM22 * nBoundingBox[3] ) );
                ImplWriteString( &pTemp, "] concat\n" );
                ImplWriteString( &pTemp, "%%BeginDocument:\n" );
                *((USHORT*)pBuf) = (USHORT)( pTemp - pBuf - 2 );
                Escape ( mhDC, nEscape, pTemp - pBuf, (LPTSTR)((BYTE*)pBuf), 0 );


                // #107797# Write out actual EPS content
                // ----------------------------------------------------------------------------------
                ULONG   nToDo = nSize;
                ULONG   nDoNow;
                while ( nToDo )
                {
                    nDoNow = nToDo;
                    if ( nToDo > POSTSCRIPT_BUFSIZE - 2 )
                        nDoNow = POSTSCRIPT_BUFSIZE - 2;
                    *((USHORT*)pBuf) = (USHORT)nDoNow;
                    memcpy( pBuf + 2, (BYTE*)pPtr + nSize - nToDo, nDoNow );
                    ULONG nResult = Escape ( mhDC, nEscape, nDoNow + 2, (LPTSTR)((BYTE*)pBuf), 0 );
                    if (!nResult )
                        break;
                    nToDo -= nResult;
                }


                // #107797# Write out EPS encapsulation footer
                // ----------------------------------------------------------------------------------
                pTemp = pBuf + 2;
                ImplWriteString( &pTemp, "%%EndDocument\n" );
                ImplWriteString( &pTemp, "count op_count_salWin sub {pop} repeat\n" );
                ImplWriteString( &pTemp, "countdictstack dict_count_salWin sub {end} repeat\n" );
                ImplWriteString( &pTemp, "b4_Inc_state_salWin restore\n\n" );
                *((USHORT*)pBuf) = (USHORT)( pTemp - pBuf - 2 );
                Escape ( mhDC, nEscape, pTemp - pBuf, (LPTSTR)((BYTE*)pBuf), 0 );
                bRetValue = TRUE;
            }
            delete [] pBuf;
        }
    }

    return bRetValue;
}
