/*************************************************************************
 *
 *  $RCSfile: salgdi.cxx,v $
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

#include <stdio.h>
#include <string.h>

#ifndef _SVWIN_H
#include <tools/svwin.h>
#endif
#define _SV_SALGDI_CXX

#ifndef _SV_SALAQUA_HXX
#include <salaqua.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

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
#ifdef WIN
static PALETTEENTRY aImplExtraColor1 =
{
    0, 184, 255, 0
};
#endif

// -----------------------------------------------------------------------------

#ifdef WIN
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
#endif

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

#ifdef WIN
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
#endif

    // DC-Cache aufbauen
    pSalData->mpVCLVIEWCache = new VCLVIEWCache[ CACHESIZE_VCLVIEW ];
    memset( pSalData->mpVCLVIEWCache, 0, CACHESIZE_VCLVIEW * sizeof( VCLVIEWCache ) );

#ifdef WIN
    // Nur bei 256 Farben Displays, die Paletten unterstuetzen
    VCLVIEW hDC = GetDC( 0 );
    int nBitsPixel = GetDeviceCaps( hDC, BITSPIXEL );
    int nPlanes = GetDeviceCaps( hDC, PLANES );
    int nRasterCaps = GetDeviceCaps( hDC, RASTERCAPS );
    int nBitCount = nBitsPixel * nPlanes;

    if ( (nBitCount > 8) && (nBitCount < 24) )
    {
        // test, if we have to dither
        VCLVIEW         hMemDC = ::CreateCompatibleDC( hDC );
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
#endif
}

// -----------------------------------------------------------------------

void ImplFreeSalGDI()
{
    SalData*    pSalData = GetSalData();
    USHORT      i;

    // destroy stock objects
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

    ImplClearVCLVIEWCache( pSalData );
    delete[] pSalData->mpVCLVIEWCache;

    // Ditherpalette loeschen, wenn vorhanden
    if ( pSalData->mhDitherPal )
    {
#ifdef WIN
        DeleteObject( pSalData->mhDitherPal );
#endif
        pSalData->mhDitherPal = 0;
    }

    // delete buffers for dithering DIB patterns, if neccessary
    if ( pSalData->mhDitherDIB )
    {
#ifdef WIN
        GlobalUnlock( pSalData->mhDitherDIB );
        GlobalFree( pSalData->mhDitherDIB );
#endif
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
}

// -----------------------------------------------------------------------

static int ImplIsPaletteEntry( BYTE nRed, BYTE nGreen, BYTE nBlue )
{
    // dither color?
    if ( !(nRed % DITHER_PAL_DELTA) && !(nGreen % DITHER_PAL_DELTA) && !(nBlue % DITHER_PAL_DELTA) )
        return TRUE;

#ifdef WIN
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

#endif
    return FALSE;
}

// =======================================================================

int ImplIsSysColorEntry( SalColor nSalColor )
{
    SysColorEntry*  pEntry = pFirstSysColor;
#ifdef WIN
    const DWORD     nTestRGB = (DWORD)RGB( SALCOLOR_RED( nSalColor ),
                                           SALCOLOR_GREEN( nSalColor ),
                                           SALCOLOR_BLUE( nSalColor ) );

    while ( pEntry )
    {
        if ( pEntry->nRGB == nTestRGB )
            return TRUE;
        pEntry = pEntry->pNext;
    }
#endif

    return FALSE;
}

// =======================================================================

static void ImplInsertSysColorEntry( int nSysIndex )
{
#ifdef WIN
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
#endif
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

#ifdef WIN
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
#endif
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

void ImplSalInitGraphics( SalGraphicsData* pData )
{
#ifdef WIN
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
#endif
}

// -----------------------------------------------------------------------

void ImplSalDeInitGraphics( SalGraphicsData* pData )
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

VCLVIEW ImplGetCachedDC( ULONG nID, HBITMAP hBmp )
{
    SalData*    pSalData = GetSalData();
    VCLVIEWCache*   pC = &pSalData->mpVCLVIEWCache[ nID ];

#ifdef WIN
    if( !pC->mhDC )
    {
        VCLVIEW hDC = GetDC( 0 );

        // neuen DC mit DefaultBitmap anlegen
        pC->mhDC = CreateCompatibleDC( hDC );

        if( pSalData->mhDitherPal )
        {
            pC->mhDefPal = SelectPalette( pC->mhDC, pSalData->mhDitherPal, TRUE );
            RealizePalette( pC->mhDC );
        }

        pC->mhSelBmp = CreateCompatibleBitmap( hDC, CACHED_VCLVIEW_DEFEXT, CACHED_VCLVIEW_DEFEXT );
        pC->mhDefBmp = (HBITMAP) SelectObject( pC->mhDC, pC->mhSelBmp );

        ReleaseDC( 0, hDC );
    }

    if ( hBmp )
        SelectObject( pC->mhDC, pC->mhActBmp = hBmp );
    else
#endif
        pC->mhActBmp = 0;

    return pC->mhDC;
}

// =======================================================================

void ImplReleaseCachedDC( ULONG nID )
{
    SalData*    pSalData = GetSalData();
    VCLVIEWCache*   pC = &pSalData->mpVCLVIEWCache[ nID ];

#ifdef WIN
    if ( pC->mhActBmp )
        SelectObject( pC->mhDC, pC->mhSelBmp );
#endif
}

// =======================================================================

void ImplClearVCLVIEWCache( SalData* pData )
{
    for( ULONG i = 0; i < CACHESIZE_VCLVIEW; i++ )
    {
        VCLVIEWCache* pC = &pData->mpVCLVIEWCache[ i ];

#ifdef WIN
        if( pC->mhDC )
        {
            SelectObject( pC->mhDC, pC->mhDefBmp );

            if( pC->mhDefPal )
                SelectPalette( pC->mhDC, pC->mhDefPal, TRUE );

            DeleteDC( pC->mhDC );
            DeleteObject( pC->mhSelBmp );
        }
#endif
    }
}

// =======================================================================

SalGraphics::SalGraphics()
{
    maGraphicsData.mhDC                 = 0;
    maGraphicsData.mhPen                = 0;
    maGraphicsData.mhBrush              = 0;
    maGraphicsData.mhFont               = 0;
    maGraphicsData.mhRegion             = 0;
    maGraphicsData.mhDefPen             = 0;
    maGraphicsData.mhDefBrush           = 0;
    maGraphicsData.mhDefFont            = 0;
    maGraphicsData.mhDefPal             = 0;
#ifdef WIN
    maGraphicsData.mpStdClipRgnData     = NULL;
    maGraphicsData.mpLogFont            = NULL;
#endif
    maGraphicsData.mpFontCharSets       = NULL;
    maGraphicsData.mnFontCharSetCount   = 0;
#ifdef WIN
    maGraphicsData.mpFontKernPairs      = NULL;
#endif
    maGraphicsData.mnFontKernPairCount  = 0;
    maGraphicsData.mbFontKernInit       = FALSE;
    maGraphicsData.mnFontOverhang       = 0;
    maGraphicsData.mbXORMode            = FALSE;
    maGraphicsData.mnPenWidth           = GSL_PEN_WIDTH;
    maGraphicsData.mbCalcOverhang       = TRUE;
}

// -----------------------------------------------------------------------

SalGraphics::~SalGraphics()
{
    // Objekte zerstoeren
    if ( maGraphicsData.mhPen )
    {
        if ( !maGraphicsData.mbStockPen )
            DeletePen( maGraphicsData.mhPen );
    }
    if ( maGraphicsData.mhBrush )
    {
        if ( !maGraphicsData.mbStockBrush )
            DeleteBrush( maGraphicsData.mhBrush );
    }
    if ( maGraphicsData.mhFont )
        DeleteFont( maGraphicsData.mhFont );

    if ( maGraphicsData.mhRegion )
    {
        DeleteRegion( maGraphicsData.mhRegion );
        maGraphicsData.mhRegion = 0;
    }

    // Cache-Daten zerstoeren
#ifdef WIN
    if ( maGraphicsData.mpStdClipRgnData )
        delete maGraphicsData.mpStdClipRgnData;

    if ( maGraphicsData.mpLogFont )
        delete maGraphicsData.mpLogFont;
#endif

    if ( maGraphicsData.mpFontCharSets )
        delete maGraphicsData.mpFontCharSets;

#ifdef WIN
    if ( maGraphicsData.mpFontKernPairs )
        delete maGraphicsData.mpFontKernPairs;
#endif
}

// -----------------------------------------------------------------------

void SalGraphics::GetResolution( long& rDPIX, long& rDPIY )
{
#ifdef WIN
    rDPIX = GetDeviceCaps( maGraphicsData.mhDC, LOGPIXELSX );
    rDPIY = GetDeviceCaps( maGraphicsData.mhDC, LOGPIXELSY );
#endif
}

// -----------------------------------------------------------------------

void SalGraphics::GetScreenFontResolution( long& rDPIX, long& rDPIY )
{
#ifdef WIN
    rDPIX = GetDeviceCaps( maGraphicsData.mhDC, LOGPIXELSX );
    rDPIY = GetDeviceCaps( maGraphicsData.mhDC, LOGPIXELSY );
#endif
}

// -----------------------------------------------------------------------

USHORT SalGraphics::GetBitCount()
{
#ifdef WIN
    return (USHORT)GetDeviceCaps( maGraphicsData.mhDC, BITSPIXEL );
#else
    return 0;
#endif
}

// -----------------------------------------------------------------------

void SalGraphics::ResetClipRegion()
{
    if ( maGraphicsData.mhRegion )
    {
        DeleteRegion( maGraphicsData.mhRegion );
        maGraphicsData.mhRegion = 0;
    }

#ifdef WIN
    SelectClipRgn( maGraphicsData.mhDC, 0 );
#endif
}

// -----------------------------------------------------------------------

void SalGraphics::BeginSetClipRegion( ULONG nRectCount )
{
    if ( maGraphicsData.mhRegion )
    {
        DeleteRegion( maGraphicsData.mhRegion );
        maGraphicsData.mhRegion = 0;
    }

    ULONG nRectBufSize = sizeof(RECT)*nRectCount;
#ifdef WIN
    if ( nRectCount < SAL_CLIPRECT_COUNT )
    {
        if ( !maGraphicsData.mpStdClipRgnData )
            maGraphicsData.mpStdClipRgnData = (RGNDATA*)new BYTE[sizeof(RGNDATA)-1+(SAL_CLIPRECT_COUNT*sizeof(RECT))];
        maGraphicsData.mpClipRgnData = maGraphicsData.mpStdClipRgnData;
    }
    else
        maGraphicsData.mpClipRgnData = (RGNDATA*)new BYTE[sizeof(RGNDATA)-1+nRectBufSize];
    maGraphicsData.mpClipRgnData->rdh.dwSize    = sizeof( RGNDATAHEADER );
    maGraphicsData.mpClipRgnData->rdh.iType     = RDH_RECTANGLES;
    maGraphicsData.mpClipRgnData->rdh.nCount    = nRectCount;
    maGraphicsData.mpClipRgnData->rdh.nRgnSize  = nRectBufSize;
    SetRectEmpty( &(maGraphicsData.mpClipRgnData->rdh.rcBound) );
    maGraphicsData.mpNextClipRect           = (RECT*)(&(maGraphicsData.mpClipRgnData->Buffer));
    maGraphicsData.mbFirstClipRect          = TRUE;
#endif
}


// -----------------------------------------------------------------------

BOOL SalGraphics::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
#ifdef WIN
    if ( nWidth && nHeight )
    {
        RECT*       pRect = maGraphicsData.mpNextClipRect;
        RECT*       pBoundRect = &(maGraphicsData.mpClipRgnData->rdh.rcBound);
        long        nRight = nX + nWidth;
        long        nBottom = nY + nHeight;

        if ( maGraphicsData.mbFirstClipRect )
        {
            pBoundRect->left    = nX;
            pBoundRect->top     = nY;
            pBoundRect->right   = nRight;
            pBoundRect->bottom  = nBottom;
            maGraphicsData.mbFirstClipRect = FALSE;
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
        maGraphicsData.mpNextClipRect++;
    }
    else
    {
        maGraphicsData.mpClipRgnData->rdh.nCount--;
        maGraphicsData.mpClipRgnData->rdh.nRgnSize -= sizeof( RECT );
    }
#endif

    return TRUE;
}

// -----------------------------------------------------------------------

void SalGraphics::EndSetClipRegion()
{
#ifdef WIN
    // Aus den Region-Daten muessen wir jetzt eine ClipRegion erzeugen
    if ( maGraphicsData.mpClipRgnData->rdh.nCount == 1 )
    {
        RECT* pRect = &(maGraphicsData.mpClipRgnData->rdh.rcBound);
        maGraphicsData.mhRegion = CreateRectRgn( pRect->left, pRect->top,
                                                 pRect->right, pRect->bottom );
    }
    else
    {
        ULONG nSize = maGraphicsData.mpClipRgnData->rdh.nRgnSize+sizeof(RGNDATAHEADER);
        maGraphicsData.mhRegion = ExtCreateRegion( NULL, nSize, maGraphicsData.mpClipRgnData );

        // if ExtCreateRegion(...) is not supported
        if( !maGraphicsData.mhRegion )
        {
            RGNDATAHEADER* pHeader = (RGNDATAHEADER*) maGraphicsData.mpClipRgnData;

            if( pHeader->nCount )
            {
                RECT* pRect = (RECT*) maGraphicsData.mpClipRgnData->Buffer;
                maGraphicsData.mhRegion = CreateRectRgn( pRect->left, pRect->top, pRect->right, pRect->bottom );
                pRect++;

                for( ULONG n = 1; n < pHeader->nCount; n++, pRect++ )
                {
                    HRGN hRgn = CreateRectRgn( pRect->left, pRect->top, pRect->right, pRect->bottom );
                    CombineRgn( maGraphicsData.mhRegion, maGraphicsData.mhRegion, hRgn, RGN_OR );
                    DeleteRegion( hRgn );
                }
            }
        }

        if ( maGraphicsData.mpClipRgnData != maGraphicsData.mpStdClipRgnData )
            delete maGraphicsData.mpClipRgnData;
    }

    SelectClipRgn( maGraphicsData.mhDC, maGraphicsData.mhRegion );
#endif
}

// -----------------------------------------------------------------------

void SalGraphics::SetLineColor()
{
#ifdef WIN
    // create and select new pen
    HPEN hNewPen = GetStockPen( NULL_PEN );
    HPEN hOldPen = SelectPen( maGraphicsData.mhDC, hNewPen );

    // destory or save old pen
    if ( maGraphicsData.mhPen )
    {
        if ( !maGraphicsData.mbStockPen )
            DeletePen( maGraphicsData.mhPen );
    }
    else
        maGraphicsData.mhDefPen = hOldPen;

    // set new data
    maGraphicsData.mhPen        = hNewPen;
    maGraphicsData.mbPen        = FALSE;
    maGraphicsData.mbStockPen   = TRUE;
#endif
}

// -----------------------------------------------------------------------

void SalGraphics::SetLineColor( SalColor nSalColor )
{
#ifdef WIN
    COLORREF    nPenColor = PALETTERGB( SALCOLOR_RED( nSalColor ),
                                        SALCOLOR_GREEN( nSalColor ),
                                        SALCOLOR_BLUE( nSalColor ) );
    HPEN        hNewPen = 0;
    BOOL        bStockPen;

    // search for stock pen (only screen, because printer have problems,
    // when we use stock objects)
    if ( !maGraphicsData.mbPrinter )
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
        if ( !maGraphicsData.mbPrinter )
        {
            if ( GetSalData()->mhDitherPal && ImplIsSysColorEntry( nSalColor ) )
                nPenColor = PALRGB_TO_RGB( nPenColor );
        }

        hNewPen = CreatePen( PS_SOLID, maGraphicsData.mnPenWidth, nPenColor );
        bStockPen = FALSE;
    }

    // select new pen
    HPEN hOldPen = SelectPen( maGraphicsData.mhDC, hNewPen );

    // destory or save old pen
    if ( maGraphicsData.mhPen )
    {
        if ( !maGraphicsData.mbStockPen )
            DeletePen( maGraphicsData.mhPen );
    }
    else
        maGraphicsData.mhDefPen = hOldPen;

    // set new data
    maGraphicsData.mnPenColor   = nPenColor;
    maGraphicsData.mhPen        = hNewPen;
    maGraphicsData.mbPen        = TRUE;
    maGraphicsData.mbStockPen   = bStockPen;
#endif
}

// -----------------------------------------------------------------------

void SalGraphics::SetFillColor()
{
    // create and select new brush
#ifdef WIN
    HBRUSH hNewBrush = GetStockBrush( NULL_BRUSH );
    HBRUSH hOldBrush = SelectBrush( maGraphicsData.mhDC, hNewBrush );
#endif

    // destory or save old brush
    if ( maGraphicsData.mhBrush )
    {
        if ( !maGraphicsData.mbStockBrush )
            DeleteBrush( maGraphicsData.mhBrush );
    }
#ifdef WIN
    else
        maGraphicsData.mhDefBrush = hOldBrush;

    // set new data
    maGraphicsData.mhBrush      = hNewBrush;
#endif
    maGraphicsData.mbBrush      = FALSE;
    maGraphicsData.mbStockBrush = TRUE;
}

// -----------------------------------------------------------------------

void SalGraphics::SetFillColor( SalColor nSalColor )
{
#ifdef WIN
    SalData*    pSalData    = GetSalData();
    BYTE        nRed        = SALCOLOR_RED( nSalColor );
    BYTE        nGreen      = SALCOLOR_GREEN( nSalColor );
    BYTE        nBlue       = SALCOLOR_BLUE( nSalColor );
    COLORREF    nBrushColor = PALETTERGB( nRed, nGreen, nBlue );
    HBRUSH      hNewBrush   = 0;
    BOOL        bStockBrush;

    // search for stock brush (only screen, because printer have problems,
    // when we use stock objects)
    if ( !maGraphicsData.mbPrinter )
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
        if ( maGraphicsData.mbPrinter || !pSalData->mhDitherDIB )
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
    HBRUSH hOldBrush = SelectBrush( maGraphicsData.mhDC, hNewBrush );

    // destory or save old brush
    if ( maGraphicsData.mhBrush )
    {
        if ( !maGraphicsData.mbStockBrush )
            DeleteBrush( maGraphicsData.mhBrush );
    }
    else
        maGraphicsData.mhDefBrush = hOldBrush;

    // set new data
    maGraphicsData.mnBrushColor = nBrushColor;
    maGraphicsData.mhBrush      = hNewBrush;
    maGraphicsData.mbBrush      = FALSE;
    maGraphicsData.mbStockBrush = bStockBrush;
#endif
}

// -----------------------------------------------------------------------

void SalGraphics::SetXORMode( BOOL bSet )
{
    maGraphicsData.mbXORMode = bSet;
#ifdef WIn
    ::SetROP2( maGraphicsData.mhDC, bSet ? R2_XORPEN : R2_COPYPEN );
#endif
}

// -----------------------------------------------------------------------

void SalGraphics::SetROPLineColor( SalROPColor nROPColor )
{
    SetLineColor( ImplGetROPSalColor( nROPColor ) );
}

// -----------------------------------------------------------------------

void SalGraphics::SetROPFillColor( SalROPColor nROPColor )
{
    SetFillColor( ImplGetROPSalColor( nROPColor ) );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawPixel( long nX, long nY )
{
#ifdef WIN
    if ( maGraphicsData.mbXORMode )
    {
        HBRUSH  hBrush = CreateSolidBrush( maGraphicsData.mnPenColor );
        HBRUSH  hOldBrush = SelectBrush( maGraphicsData.mhDC, hBrush );
        PatBlt( maGraphicsData.mhDC, (int)nX, (int)nY, (int)1, (int)1, PATINVERT );
        SelectBrush( maGraphicsData.mhDC, hOldBrush );
        DeleteBrush( hBrush );
    }
    else
        SetPixel( maGraphicsData.mhDC, (int)nX, (int)nY, maGraphicsData.mnPenColor );
#endif
}

// -----------------------------------------------------------------------

void SalGraphics::DrawPixel( long nX, long nY, SalColor nSalColor )
{
#ifdef WIN
    COLORREF nCol = PALETTERGB( SALCOLOR_RED( nSalColor ),
                                SALCOLOR_GREEN( nSalColor ),
                                SALCOLOR_BLUE( nSalColor ) );

    if ( !maGraphicsData.mbPrinter &&
         GetSalData()->mhDitherPal &&
         ImplIsSysColorEntry( nSalColor ) )
        nCol = PALRGB_TO_RGB( nCol );

    if ( maGraphicsData.mbXORMode )
    {
        HBRUSH  hBrush = CreateSolidBrush( nCol );
        HBRUSH  hOldBrush = SelectBrush( maGraphicsData.mhDC, hBrush );
        PatBlt( maGraphicsData.mhDC, (int)nX, (int)nY, (int)1, (int)1, PATINVERT );
        SelectBrush( maGraphicsData.mhDC, hOldBrush );
        DeleteBrush( hBrush );
    }
    else
        ::SetPixel( maGraphicsData.mhDC, (int)nX, (int)nY, nCol );
#endif
}

// -----------------------------------------------------------------------

void SalGraphics::DrawLine( long nX1, long nY1, long nX2, long nY2 )
{
#ifdef WIN
    MoveToEx( maGraphicsData.mhDC, (int)nX1, (int)nY1, NULL );
#endif

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

#ifdef WIN
    LineTo( maGraphicsData.mhDC, (int)nX2, (int)nY2 );
#endif

    if ( bPaintEnd && !maGraphicsData.mbPrinter )
    {
#ifdef WIN
        if ( maGraphicsData.mbXORMode )
        {
            HBRUSH  hBrush = CreateSolidBrush( maGraphicsData.mnPenColor );
            HBRUSH  hOldBrush = SelectBrush( maGraphicsData.mhDC, hBrush );
            PatBlt( maGraphicsData.mhDC, (int)nX2, (int)nY2, (int)1, (int)1, PATINVERT );
            SelectBrush( maGraphicsData.mhDC, hOldBrush );
            DeleteBrush( hBrush );
        }
        else
            SetPixel( maGraphicsData.mhDC, (int)nX2, (int)nY2, maGraphicsData.mnPenColor );
#endif
    }
}

// -----------------------------------------------------------------------

void SalGraphics::DrawRect( long nX, long nY, long nWidth, long nHeight )
{
    if ( !maGraphicsData.mbPen )
    {
        if ( !maGraphicsData.mbPrinter )
        {
#ifdef WIN
            PatBlt( maGraphicsData.mhDC, (int)nX, (int)nY, (int)nWidth, (int)nHeight, maGraphicsData.mbXORMode ? PATINVERT : PATCOPY );
#endif
        }
        else
        {
            RECT aWinRect;
            aWinRect.left   = nX;
            aWinRect.top    = nY;
            aWinRect.right  = nX+nWidth;
            aWinRect.bottom = nY+nHeight;
#ifdef WIN
            ::FillRect( maGraphicsData.mhDC, &aWinRect, maGraphicsData.mhBrush );
#endif
        }
    }
#ifdef WIN
    else
        WIN_Rectangle( maGraphicsData.mhDC, (int)nX, (int)nY, (int)(nX+nWidth), (int)(nY+nHeight) );
#endif
}

// -----------------------------------------------------------------------

void SalGraphics::DrawPolyLine( ULONG nPoints, const SalPoint* pPtAry )
{
    // Unter NT koennen wir das Array direkt weiterreichen
    DBG_ASSERT( sizeof( POINT ) == sizeof( SalPoint ),
                "SalGraphics::DrawPolyLine(): POINT != SalPoint" );

    POINT* pWinPtAry = (POINT*)pPtAry;
    // Wegen Windows 95 und der Beschraenkung auf eine maximale Anzahl
    // von Punkten
#ifdef WIN
    if ( !Polyline( maGraphicsData.mhDC, pWinPtAry, (int)nPoints ) && (nPoints > MAX_64KSALPOINTS) )
        Polyline( maGraphicsData.mhDC, pWinPtAry, MAX_64KSALPOINTS );
#endif
}

// -----------------------------------------------------------------------

void SalGraphics::DrawPolygon( ULONG nPoints, const SalPoint* pPtAry )
{
    // Unter NT koennen wir das Array direkt weiterreichen
    DBG_ASSERT( sizeof( POINT ) == sizeof( SalPoint ),
                "SalGraphics::DrawPolygon(): POINT != SalPoint" );

    POINT* pWinPtAry = (POINT*)pPtAry;
#ifdef WIN
    // Wegen Windows 95 und der Beschraenkung auf eine maximale Anzahl
    // von Punkten
    if ( !WIN_Polygon( maGraphicsData.mhDC, pWinPtAry, (int)nPoints ) && (nPoints > MAX_64KSALPOINTS) )
        WIN_Polygon( maGraphicsData.mhDC, pWinPtAry, MAX_64KSALPOINTS );
#endif
}

// -----------------------------------------------------------------------

void SalGraphics::DrawPolyPolygon( ULONG nPoly, const ULONG* pPoints,
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
                "SalGraphics::DrawPolyPolygon(): POINT != SalPoint" );
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

#ifdef WIN
    if ( !WIN_PolyPolygon( maGraphicsData.mhDC, pWinPointAryAry, (int*)pWinPointAry, (UINT)nPoly ) &&
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
            WIN_Polygon( maGraphicsData.mhDC, pWinPointAryAry, *pWinPointAry );
        else
            WIN_PolyPolygon( maGraphicsData.mhDC, pWinPointAryAry, (int*)pWinPointAry, nPoly );
    }
#endif

    if ( pWinPointAry != aWinPointAry )
        delete pWinPointAry;
    if ( pWinPointAryAry != aWinPointAryAry )
        delete pWinPointAryAry;
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
    ULONG   nBytesRead;

    if ( nSize < 256 )      // we assume that the file is greater than 256 bytes
        return FALSE;

    if ( nSize < POSTSCRIPT_BOUNDINGSEARCH )
        nBytesRead = nSize;
    else
        nBytesRead = POSTSCRIPT_BOUNDINGSEARCH;

    BYTE* pDest = ImplSearchEntry( pSource, (BYTE*)"%%BoundingBox:", nBytesRead, 14 );
    if ( pDest )
    {
        int     nSecurityCount = 100;   // only 100 bytes following the bounding box will be checked
        nNumb[0] = nNumb[1] = nNumb[2] = nNumb[3] = 0;
        pDest += 14;
        for ( int i = 0; ( i < 4 ) && nSecurityCount; i++ )
        {
            int     nDivision = 1;
            BOOL    bDivision = FALSE;
            BOOL    bNegative = FALSE;
            BOOL    bValid = TRUE;

            while ( ( --nSecurityCount ) && ( *pDest == ' ' ) || ( *pDest == 0x9 ) ) pDest++;
            BYTE nByte = *pDest;
            while ( nSecurityCount && ( nByte != ' ' ) && ( nByte != 0x9 ) && ( nByte != 0xd ) && ( nByte != 0xa ) )
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
                            nSecurityCount = 1;     // error parsing the bounding box values
                        else if ( bValid )
                        {
                            if ( bDivision )
                                nDivision*=10;
                            nNumb[i] *= 10;
                            nNumb[i] += nByte - '0';
                        }
                        break;
                }
                nSecurityCount--;
                nByte = *(++pDest);
            }
            if ( bNegative )
                nNumb[i] = -nNumb[i];
            if ( bDivision && ( nDivision != 1 ) )
                nNumb[i] /= nDivision;
        }
        if ( nSecurityCount)
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

BOOL SalGraphics::DrawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, ULONG nSize )
{
    BOOL bRetValue = FALSE;

    if ( maGraphicsData.mbPrinter )
    {
#ifdef WIN
        int nEscape = POSTSCRIPT_PASSTHROUGH;

        if ( Escape( maGraphicsData.mhDC, QUERYESCSUPPORT, sizeof( int ), ( LPSTR )&nEscape, 0 ) )
        {
            BYTE* pBuf = new BYTE[ POSTSCRIPT_BUFSIZE ];

            double  nBoundingBox[4];

            if ( pBuf && ImplGetBoundingBox( nBoundingBox, (BYTE*)pPtr, nSize ) )
            {
                double  dM11 = nWidth / ( nBoundingBox[2] - nBoundingBox[0] );
                double  dM22 = nHeight / (nBoundingBox[1] - nBoundingBox[3] );
                BYTE*   pTemp = pBuf + 2;       // +2 because we want to insert the size later
                ImplWriteString( &pTemp, "\n\nsave\n[ " );
                ImplWriteDouble( &pTemp, dM11 );
                ImplWriteDouble( &pTemp, 0 );
                ImplWriteDouble( &pTemp, 0 );
                ImplWriteDouble( &pTemp, dM22 );
                ImplWriteDouble( &pTemp, nX - ( dM11 * nBoundingBox[0] ) );
                ImplWriteDouble( &pTemp, nY - ( dM22 * nBoundingBox[3] ) );
                ImplWriteString( &pTemp, "] concat /showpage {} def\n" );
                ImplWriteString( &pTemp, "%%BeginDocument:\n" );
                *((USHORT*)pBuf) = (USHORT)( pTemp - pBuf - 2 );
                Escape ( maGraphicsData.mhDC, nEscape, pTemp - pBuf, (LPTSTR)((BYTE*)pBuf), 0 );

                ULONG   nToDo = nSize;
                ULONG   nDoNow;
                while ( nToDo )
                {
                    nDoNow = nToDo;
                    if ( nToDo > POSTSCRIPT_BUFSIZE - 2 )
                        nDoNow = POSTSCRIPT_BUFSIZE - 2;
                    *((USHORT*)pBuf) = (USHORT)nDoNow;
                    memcpy( pBuf + 2, (BYTE*)pPtr + nSize - nToDo, nDoNow );
                    ULONG nResult = Escape ( maGraphicsData.mhDC, nEscape, nDoNow + 2, (LPTSTR)((BYTE*)pBuf), 0 );
                    if (!nResult )
                        break;
                    nToDo -= nResult;
                }
                pTemp = pBuf + 2;
                ImplWriteString( &pTemp, "%%EndDocument\n" );
                ImplWriteString( &pTemp, "restore\n\n" );
                *((USHORT*)pBuf) = (USHORT)( pTemp - pBuf - 2 );
                Escape ( maGraphicsData.mhDC, nEscape, pTemp - pBuf, (LPTSTR)((BYTE*)pBuf), 0 );
                bRetValue = TRUE;
            }
            delete pBuf;
        }
#endif
    }

    return bRetValue;
}
