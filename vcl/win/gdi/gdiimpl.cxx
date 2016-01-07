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

#include <svsys.h>

#include "gdiimpl.hxx"

#include <string.h>
#include <rtl/strbuf.hxx>
#include <tools/debug.hxx>
#include <tools/poly.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <win/wincomp.hxx>
#include <win/saldata.hxx>
#include <win/salgdi.h>
#include "win/salbmp.h"
#include <vcl/salbtype.hxx>
#include <win/salframe.h>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include "outdata.hxx"
#include "win/salids.hrc"

#if defined _MSC_VER
#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#endif
#endif

#include "prewin.h"

#ifdef __MINGW32__
#ifdef GetObject
#undef GetObject
#endif
#endif

#include <gdiplus.h>
#include <gdiplusenums.h>
#include <gdipluscolor.h>

#include "postwin.h"

#define SAL_POLYPOLYCOUNT_STACKBUF          8
#define SAL_POLYPOLYPOINTS_STACKBUF         64

#define DITHER_PAL_DELTA                51
#define DITHER_PAL_STEPS                6
#define DITHER_PAL_COUNT                (DITHER_PAL_STEPS*DITHER_PAL_STEPS*DITHER_PAL_STEPS)
#define DITHER_MAX_SYSCOLOR             16
#define DITHER_EXTRA_COLORS             1
#define DMAP( _def_nVal, _def_nThres )  ((pDitherDiff[_def_nVal]>(_def_nThres))?pDitherHigh[_def_nVal]:pDitherLow[_def_nVal])

#define SAL_POLY_STACKBUF       32
#define USE_GDI_BEZIERS

namespace {

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

static PALETTEENTRY aImplExtraColor1 =
{
    0, 184, 255, 0
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

SalColor ImplGetROPSalColor( SalROPColor nROPColor )
{
    SalColor nSalColor;
    if ( nROPColor == SAL_ROP_0 )
        nSalColor = MAKE_SALCOLOR( 0, 0, 0 );
    else
        nSalColor = MAKE_SALCOLOR( 255, 255, 255 );
    return nSalColor;
}

int ImplIsPaletteEntry( BYTE nRed, BYTE nGreen, BYTE nBlue )
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

}

WinSalGraphicsImpl::WinSalGraphicsImpl(WinSalGraphics& rParent):
    mrParent(rParent),
    mbXORMode(false),
    mhPen(0),
    mhBrush(0)
{
}

WinSalGraphicsImpl::~WinSalGraphicsImpl()
{
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

}

void WinSalGraphicsImpl::Init()
{
}

void WinSalGraphicsImpl::freeResources()
{
}

bool WinSalGraphicsImpl::drawEPS(long, long, long, long, void*, sal_uLong)
{
    return false;
}

void WinSalGraphicsImpl::copyBits( const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics )
{
    HDC     hSrcDC;
    DWORD   nRop;

    if ( pSrcGraphics )
        hSrcDC = static_cast<WinSalGraphics*>(pSrcGraphics)->getHDC();
    else
        hSrcDC = mrParent.getHDC();

    if ( mbXORMode )
        nRop = SRCINVERT;
    else
        nRop = SRCCOPY;

    if ( (rPosAry.mnSrcWidth  == rPosAry.mnDestWidth) &&
         (rPosAry.mnSrcHeight == rPosAry.mnDestHeight) )
    {
        BitBlt( mrParent.getHDC(),
                (int)rPosAry.mnDestX, (int)rPosAry.mnDestY,
                (int)rPosAry.mnDestWidth, (int)rPosAry.mnDestHeight,
                hSrcDC,
                (int)rPosAry.mnSrcX, (int)rPosAry.mnSrcY,
                nRop );
    }
    else
    {
        int nOldStretchMode = SetStretchBltMode( mrParent.getHDC(), STRETCH_DELETESCANS );
        StretchBlt( mrParent.getHDC(),
                    (int)rPosAry.mnDestX, (int)rPosAry.mnDestY,
                    (int)rPosAry.mnDestWidth, (int)rPosAry.mnDestHeight,
                    hSrcDC,
                    (int)rPosAry.mnSrcX, (int)rPosAry.mnSrcY,
                    (int)rPosAry.mnSrcWidth, (int)rPosAry.mnSrcHeight,
                    nRop );
        SetStretchBltMode( mrParent.getHDC(), nOldStretchMode );
    }
}

void ImplCalcOutSideRgn( const RECT& rSrcRect,
                         int nLeft, int nTop, int nRight, int nBottom,
                         HRGN& rhInvalidateRgn )
{
    HRGN hTempRgn;

    // calculate area outside the visible region
    if ( rSrcRect.left < nLeft )
    {
        if ( !rhInvalidateRgn )
            rhInvalidateRgn = CreateRectRgnIndirect( &rSrcRect );
        hTempRgn = CreateRectRgn( -31999, 0, nLeft, 31999 );
        CombineRgn( rhInvalidateRgn, rhInvalidateRgn, hTempRgn, RGN_DIFF );
        DeleteRegion( hTempRgn );
    }
    if ( rSrcRect.top < nTop )
    {
        if ( !rhInvalidateRgn )
            rhInvalidateRgn = CreateRectRgnIndirect( &rSrcRect );
        hTempRgn = CreateRectRgn( 0, -31999, 31999, nTop );
        CombineRgn( rhInvalidateRgn, rhInvalidateRgn, hTempRgn, RGN_DIFF );
        DeleteRegion( hTempRgn );
    }
    if ( rSrcRect.right > nRight )
    {
        if ( !rhInvalidateRgn )
            rhInvalidateRgn = CreateRectRgnIndirect( &rSrcRect );
        hTempRgn = CreateRectRgn( nRight, 0, 31999, 31999 );
        CombineRgn( rhInvalidateRgn, rhInvalidateRgn, hTempRgn, RGN_DIFF );
        DeleteRegion( hTempRgn );
    }
    if ( rSrcRect.bottom > nBottom )
    {
        if ( !rhInvalidateRgn )
            rhInvalidateRgn = CreateRectRgnIndirect( &rSrcRect );
        hTempRgn = CreateRectRgn( 0, nBottom, 31999, 31999 );
        CombineRgn( rhInvalidateRgn, rhInvalidateRgn, hTempRgn, RGN_DIFF );
        DeleteRegion( hTempRgn );
    }
}

void WinSalGraphicsImpl::copyArea( long nDestX, long nDestY,
                            long nSrcX, long nSrcY,
                            long nSrcWidth, long nSrcHeight,
                            sal_uInt16 nFlags )
{
    bool    bRestoreClipRgn = false;
    HRGN    hOldClipRgn = 0;
    int     nOldClipRgnType = ERROR;
    HRGN    hInvalidateRgn = 0;

    // do we have to invalidate also the overlapping regions?
    if ( (nFlags & SAL_COPYAREA_WINDOWINVALIDATE) && mrParent.isWindow() )
    {
        // compute and invalidate those parts that were either off-screen or covered by other windows
        //  while performing the above BitBlt
        // those regions then have to be invalidated as they contain useless/wrong data
        RECT    aSrcRect;
        RECT    aClipRect;
        RECT    aTempRect;
        RECT    aTempRect2;
        HRGN    hTempRgn;
        HWND    hWnd;

        // restrict srcRect to this window (calc intersection)
        aSrcRect.left   = (int)nSrcX;
        aSrcRect.top    = (int)nSrcY;
        aSrcRect.right  = aSrcRect.left+(int)nSrcWidth;
        aSrcRect.bottom = aSrcRect.top+(int)nSrcHeight;
        GetClientRect( mrParent.gethWnd(), &aClipRect );
        if ( IntersectRect( &aSrcRect, &aSrcRect, &aClipRect ) )
        {
            // transform srcRect to screen coordinates
            POINT aPt;
            aPt.x = 0;
            aPt.y = 0;
            ClientToScreen( mrParent.gethWnd(), &aPt );
            aSrcRect.left   += aPt.x;
            aSrcRect.top    += aPt.y;
            aSrcRect.right  += aPt.x;
            aSrcRect.bottom += aPt.y;
            hInvalidateRgn = 0;

            // compute the parts that are off screen (ie invisible)
            RECT theScreen;
            ImplSalGetWorkArea( NULL, &theScreen, NULL );  // find the screen area taking multiple monitors into account
            ImplCalcOutSideRgn( aSrcRect, theScreen.left, theScreen.top, theScreen.right, theScreen.bottom, hInvalidateRgn );

            // calculate regions that are covered by other windows
            HRGN hTempRgn2 = 0;
            HWND hWndTopWindow = mrParent.gethWnd();
            // Find the TopLevel Window, because only Windows which are in
            // in the foreground of our TopLevel window must be considered
            if ( GetWindowStyle( hWndTopWindow ) & WS_CHILD )
            {
                RECT aTempRect3 = aSrcRect;
                do
                {
                    hWndTopWindow = ::GetParent( hWndTopWindow );

                    // Test if the Parent clips our window
                    GetClientRect( hWndTopWindow, &aTempRect );
                    POINT aPt2;
                    aPt2.x = 0;
                    aPt2.y = 0;
                    ClientToScreen( hWndTopWindow, &aPt2 );
                    aTempRect.left   += aPt2.x;
                    aTempRect.top    += aPt2.y;
                    aTempRect.right  += aPt2.x;
                    aTempRect.bottom += aPt2.y;
                    IntersectRect( &aTempRect3, &aTempRect3, &aTempRect );
                }
                while ( GetWindowStyle( hWndTopWindow ) & WS_CHILD );

                // If one or more Parents clip our window, than we must
                // calculate the outside area
                if ( !EqualRect( &aSrcRect, &aTempRect3 ) )
                {
                    ImplCalcOutSideRgn( aSrcRect,
                                        aTempRect3.left, aTempRect3.top,
                                        aTempRect3.right, aTempRect3.bottom,
                                        hInvalidateRgn );
                }
            }
            // retrieve the top-most (z-order) child window
            hWnd = GetWindow( GetDesktopWindow(), GW_CHILD );
            while ( hWnd )
            {
                if ( hWnd == hWndTopWindow )
                    break;
                if ( IsWindowVisible( hWnd ) && !IsIconic( hWnd ) )
                {
                    GetWindowRect( hWnd, &aTempRect );
                    if ( IntersectRect( &aTempRect2, &aSrcRect, &aTempRect ) )
                    {
                        // hWnd covers part or all of aSrcRect
                        if ( !hInvalidateRgn )
                            hInvalidateRgn = CreateRectRgnIndirect( &aSrcRect );

                        // get full bounding box of hWnd
                        hTempRgn = CreateRectRgnIndirect( &aTempRect );

                        // get region of hWnd (the window may be shaped)
                        if ( !hTempRgn2 )
                            hTempRgn2 = CreateRectRgn( 0, 0, 0, 0 );
                        int nRgnType = GetWindowRgn( hWnd, hTempRgn2 );
                        if ( (nRgnType != ERROR) && (nRgnType != NULLREGION) )
                        {
                            // convert window region to screen coordinates
                            OffsetRgn( hTempRgn2, aTempRect.left, aTempRect.top );
                            // and intersect with the window's bounding box
                            CombineRgn( hTempRgn, hTempRgn, hTempRgn2, RGN_AND );
                        }
                        // finally compute that part of aSrcRect which is not covered by any parts of hWnd
                        CombineRgn( hInvalidateRgn, hInvalidateRgn, hTempRgn, RGN_DIFF );
                        DeleteRegion( hTempRgn );
                    }
                }
                // retrieve the next window in the z-order, i.e. the window below hwnd
                hWnd = GetWindow( hWnd, GW_HWNDNEXT );
            }
            if ( hTempRgn2 )
                DeleteRegion( hTempRgn2 );
            if ( hInvalidateRgn )
            {
                // hInvalidateRgn contains the fully visible parts of the original srcRect
                hTempRgn = CreateRectRgnIndirect( &aSrcRect );
                // subtract it from the original rect to get the occluded parts
                int nRgnType = CombineRgn( hInvalidateRgn, hTempRgn, hInvalidateRgn, RGN_DIFF );
                DeleteRegion( hTempRgn );

                if ( (nRgnType != ERROR) && (nRgnType != NULLREGION) )
                {
                    // move the occluded parts to the destination pos
                    int nOffX = (int)(nDestX-nSrcX);
                    int nOffY = (int)(nDestY-nSrcY);
                    OffsetRgn( hInvalidateRgn, nOffX-aPt.x, nOffY-aPt.y );

                    // by excluding hInvalidateRgn from the system's clip region
                    // we will prevent bitblt from copying useless data
                    // especially now shadows from overlapping windows will appear (#i36344)
                    hOldClipRgn = CreateRectRgn( 0, 0, 0, 0 );
                    nOldClipRgnType = GetClipRgn( mrParent.getHDC(), hOldClipRgn );

                    bRestoreClipRgn = TRUE; // indicate changed clipregion and force invalidate
                    ExtSelectClipRgn( mrParent.getHDC(), hInvalidateRgn, RGN_DIFF );
                }
            }
        }
    }

    BitBlt( mrParent.getHDC(),
            (int)nDestX, (int)nDestY,
            (int)nSrcWidth, (int)nSrcHeight,
            mrParent.getHDC(),
            (int)nSrcX, (int)nSrcY,
            SRCCOPY );

    if( bRestoreClipRgn )
    {
        // restore old clip region
        if( nOldClipRgnType != ERROR )
            SelectClipRgn( mrParent.getHDC(), hOldClipRgn);
        DeleteRegion( hOldClipRgn );

        // invalidate regions that were not copied
        bool    bInvalidate = true;

        // Combine Invalidate vcl::Region with existing ClipRegion
        HRGN    hTempRgn = CreateRectRgn( 0, 0, 0, 0 );
        if ( GetClipRgn( mrParent.getHDC(), hTempRgn ) == 1 )
        {
            int nRgnType = CombineRgn( hInvalidateRgn, hTempRgn, hInvalidateRgn, RGN_AND );
            if ( (nRgnType == ERROR) || (nRgnType == NULLREGION) )
                bInvalidate = false;
        }
        DeleteRegion( hTempRgn );

        if ( bInvalidate )
        {
            InvalidateRgn( mrParent.gethWnd(), hInvalidateRgn, TRUE );
            // here we only initiate an update if this is the MainThread,
            // so that there is no deadlock when handling the Paint event,
            // as the SolarMutex is already held by this Thread
            SalData*    pSalData = GetSalData();
            DWORD       nCurThreadId = GetCurrentThreadId();
            if ( pSalData->mnAppThreadId == nCurThreadId )
                UpdateWindow( mrParent.gethWnd() );
        }

        DeleteRegion( hInvalidateRgn );
    }

}

namespace {

void ImplDrawBitmap( HDC hDC, const SalTwoRect& rPosAry, const WinSalBitmap& rSalBitmap,
                     bool bPrinter, int nDrawMode )
{
    if( hDC )
    {
        HGLOBAL     hDrawDIB;
        HBITMAP     hDrawDDB = rSalBitmap.ImplGethDDB();
        WinSalBitmap*   pTmpSalBmp = NULL;
        bool        bPrintDDB = ( bPrinter && hDrawDDB );

        if( bPrintDDB )
        {
            pTmpSalBmp = new WinSalBitmap;
            pTmpSalBmp->Create( rSalBitmap, rSalBitmap.GetBitCount() );
            hDrawDIB = pTmpSalBmp->ImplGethDIB();
        }
        else
            hDrawDIB = rSalBitmap.ImplGethDIB();

        if( hDrawDIB )
        {
            PBITMAPINFO         pBI = (PBITMAPINFO) GlobalLock( hDrawDIB );
            PBITMAPINFOHEADER   pBIH = (PBITMAPINFOHEADER) pBI;
            PBYTE               pBits = (PBYTE) pBI + *(DWORD*) pBI +
                                        rSalBitmap.ImplGetDIBColorCount( hDrawDIB ) * sizeof( RGBQUAD );
            const int           nOldStretchMode = SetStretchBltMode( hDC, STRETCH_DELETESCANS );

            StretchDIBits( hDC,
                           (int)rPosAry.mnDestX, (int)rPosAry.mnDestY,
                           (int)rPosAry.mnDestWidth, (int)rPosAry.mnDestHeight,
                           (int)rPosAry.mnSrcX, (int)(pBIH->biHeight - rPosAry.mnSrcHeight - rPosAry.mnSrcY),
                           (int)rPosAry.mnSrcWidth, (int)rPosAry.mnSrcHeight,
                           pBits, pBI, DIB_RGB_COLORS, nDrawMode );

            GlobalUnlock( hDrawDIB );
            SetStretchBltMode( hDC, nOldStretchMode );
        }
        else if( hDrawDDB && !bPrintDDB )
        {
            HDC         hBmpDC = ImplGetCachedDC( CACHED_HDC_DRAW, hDrawDDB );
            COLORREF    nOldBkColor = RGB(0xFF,0xFF,0xFF);
            COLORREF    nOldTextColor = RGB(0,0,0);
            bool        bMono = ( rSalBitmap.GetBitCount() == 1 );

            if( bMono )
            {
                COLORREF nBkColor = RGB( 0xFF, 0xFF, 0xFF );
                COLORREF nTextColor = RGB( 0x00, 0x00, 0x00 );
                //fdo#33455 handle 1 bit depth pngs with palette entries
                //to set fore/back colors
                if (BitmapBuffer* pBitmapBuffer = const_cast<WinSalBitmap&>(rSalBitmap).AcquireBuffer(BITMAP_INFO_ACCESS))
                {
                    const BitmapPalette& rPalette = pBitmapBuffer->maPalette;
                    if (rPalette.GetEntryCount() == 2)
                    {
                        SalColor nCol;
                        nCol = ImplColorToSal(rPalette[0]);
                        nTextColor = RGB( SALCOLOR_RED(nCol), SALCOLOR_GREEN(nCol), SALCOLOR_BLUE(nCol) );
                        nCol = ImplColorToSal(rPalette[1]);
                        nBkColor = RGB( SALCOLOR_RED(nCol), SALCOLOR_GREEN(nCol), SALCOLOR_BLUE(nCol) );
                    }
                    const_cast<WinSalBitmap&>(rSalBitmap).ReleaseBuffer(pBitmapBuffer, BITMAP_INFO_ACCESS);
                }
                nOldBkColor = SetBkColor( hDC, nBkColor );
                nOldTextColor = ::SetTextColor( hDC, nTextColor );
            }

            if ( (rPosAry.mnSrcWidth  == rPosAry.mnDestWidth) &&
                 (rPosAry.mnSrcHeight == rPosAry.mnDestHeight) )
            {
                BitBlt( hDC,
                        (int)rPosAry.mnDestX, (int)rPosAry.mnDestY,
                        (int)rPosAry.mnDestWidth, (int)rPosAry.mnDestHeight,
                        hBmpDC,
                        (int)rPosAry.mnSrcX, (int)rPosAry.mnSrcY,
                        nDrawMode );
            }
            else
            {
                const int nOldStretchMode = SetStretchBltMode( hDC, STRETCH_DELETESCANS );

                StretchBlt( hDC,
                            (int)rPosAry.mnDestX, (int)rPosAry.mnDestY,
                            (int)rPosAry.mnDestWidth, (int)rPosAry.mnDestHeight,
                            hBmpDC,
                            (int)rPosAry.mnSrcX, (int)rPosAry.mnSrcY,
                            (int)rPosAry.mnSrcWidth, (int)rPosAry.mnSrcHeight,
                            nDrawMode );

                SetStretchBltMode( hDC, nOldStretchMode );
            }

            if( bMono )
            {
                SetBkColor( hDC, nOldBkColor );
                ::SetTextColor( hDC, nOldTextColor );
            }

            ImplReleaseCachedDC( CACHED_HDC_DRAW );
        }

        if( bPrintDDB )
            delete pTmpSalBmp;
    }
}

}

void WinSalGraphicsImpl::drawBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap)
{
    bool bTryDirectPaint(!mrParent.isPrinter() && !mbXORMode);

    if(bTryDirectPaint)
    {
        // only paint direct when no scaling and no MapMode, else the
        // more expensive conversions may be done for short-time Bitmap/BitmapEx
        // used for buffering only
        if(rPosAry.mnSrcWidth == rPosAry.mnDestWidth && rPosAry.mnSrcHeight == rPosAry.mnDestHeight)
        {
            bTryDirectPaint = false;
        }
    }

    // try to draw using GdiPlus directly
    if(bTryDirectPaint && tryDrawBitmapGdiPlus(rPosAry, rSalBitmap))
    {
        return;
    }

    // fall back old stuff
    assert(dynamic_cast<const WinSalBitmap*>(&rSalBitmap));

    ImplDrawBitmap(mrParent.getHDC(), rPosAry, static_cast<const WinSalBitmap&>(rSalBitmap),
        mrParent.isPrinter(),
        mbXORMode ? SRCINVERT : SRCCOPY );
}

void WinSalGraphicsImpl::drawBitmap( const SalTwoRect& rPosAry,
                              const SalBitmap& rSSalBitmap,
                              const SalBitmap& rSTransparentBitmap )
{
    DBG_ASSERT( !mrParent.isPrinter(), "No transparency print possible!" );
    bool bTryDirectPaint(!mrParent.isPrinter() && !mbXORMode);

    // try to draw using GdiPlus directly
    if(bTryDirectPaint && drawAlphaBitmap(rPosAry, rSSalBitmap, rSTransparentBitmap))
    {
        return;
    }

    assert(dynamic_cast<const WinSalBitmap*>(&rSSalBitmap));
    assert(dynamic_cast<const WinSalBitmap*>(&rSTransparentBitmap));

    const WinSalBitmap& rSalBitmap = static_cast<const WinSalBitmap&>(rSSalBitmap);
    const WinSalBitmap& rTransparentBitmap = static_cast<const WinSalBitmap&>(rSTransparentBitmap);

    SalTwoRect  aPosAry = rPosAry;
    int         nDstX = (int)aPosAry.mnDestX;
    int         nDstY = (int)aPosAry.mnDestY;
    int         nDstWidth = (int)aPosAry.mnDestWidth;
    int         nDstHeight = (int)aPosAry.mnDestHeight;
    HDC         hDC = mrParent.getHDC();
    HBITMAP     hMemBitmap = 0;
    HBITMAP     hMaskBitmap = 0;

    if( ( nDstWidth > CACHED_HDC_DEFEXT ) || ( nDstHeight > CACHED_HDC_DEFEXT ) )
    {
        hMemBitmap = CreateCompatibleBitmap( hDC, nDstWidth, nDstHeight );
        hMaskBitmap = CreateCompatibleBitmap( hDC, nDstWidth, nDstHeight );
    }

    HDC hMemDC = ImplGetCachedDC( CACHED_HDC_1, hMemBitmap );
    HDC hMaskDC = ImplGetCachedDC( CACHED_HDC_2, hMaskBitmap );

    aPosAry.mnDestX = aPosAry.mnDestY = 0;
    BitBlt( hMemDC, 0, 0, nDstWidth, nDstHeight, hDC, nDstX, nDstY, SRCCOPY );

    // WIN/WNT seems to have a minor problem mapping the correct color of the
    // mask to the palette if we draw the DIB directly ==> draw DDB
    if( ( GetBitCount() <= 8 ) && rTransparentBitmap.ImplGethDIB() && rTransparentBitmap.GetBitCount() == 1 )
    {
        WinSalBitmap aTmp;

        if( aTmp.Create( rTransparentBitmap, &mrParent ) )
            ImplDrawBitmap( hMaskDC, aPosAry, aTmp, FALSE, SRCCOPY );
    }
    else
        ImplDrawBitmap( hMaskDC, aPosAry, rTransparentBitmap, FALSE, SRCCOPY );

    // now MemDC contains background, MaskDC the transparency mask

    // #105055# Respect XOR mode
    if( mbXORMode )
    {
        ImplDrawBitmap( hMaskDC, aPosAry, rSalBitmap, FALSE, SRCERASE );
        // now MaskDC contains the bitmap area with black background
        BitBlt( hMemDC, 0, 0, nDstWidth, nDstHeight, hMaskDC, 0, 0, SRCINVERT );
        // now MemDC contains background XORed bitmap area ontop
    }
    else
    {
        BitBlt( hMemDC, 0, 0, nDstWidth, nDstHeight, hMaskDC, 0, 0, SRCAND );
        // now MemDC contains background with masked-out bitmap area
        ImplDrawBitmap( hMaskDC, aPosAry, rSalBitmap, FALSE, SRCERASE );
        // now MaskDC contains the bitmap area with black background
        BitBlt( hMemDC, 0, 0, nDstWidth, nDstHeight, hMaskDC, 0, 0, SRCPAINT );
        // now MemDC contains background and bitmap merged together
    }
    // copy to output DC
    BitBlt( hDC, nDstX, nDstY, nDstWidth, nDstHeight, hMemDC, 0, 0, SRCCOPY );

    ImplReleaseCachedDC( CACHED_HDC_1 );
    ImplReleaseCachedDC( CACHED_HDC_2 );

    // hMemBitmap != 0 ==> hMaskBitmap != 0
    if( hMemBitmap )
    {
        DeleteObject( hMemBitmap );
        DeleteObject( hMaskBitmap );
    }
}

bool WinSalGraphicsImpl::drawAlphaRect( long nX, long nY, long nWidth,
                                    long nHeight, sal_uInt8 nTransparency )
{
    if( mbPen || !mbBrush || mbXORMode )
        return false; // can only perform solid fills without XOR.

    HDC hMemDC = ImplGetCachedDC( CACHED_HDC_1, 0 );
    SetPixel( hMemDC, (int)0, (int)0, mnBrushColor );

    BLENDFUNCTION aFunc = {
        AC_SRC_OVER,
        0,
        sal::static_int_cast<sal_uInt8>(255 - 255L*nTransparency/100),
        0
    };

    // hMemDC contains a 1x1 bitmap of the right color - stretch-blit
    // that to dest hdc
    bool bRet = AlphaBlend( mrParent.getHDC(), nX, nY, nWidth, nHeight,
                            hMemDC, 0,0,1,1,
                            aFunc ) == TRUE;

    ImplReleaseCachedDC( CACHED_HDC_1 );

    return bRet;
}

void WinSalGraphicsImpl::drawMask( const SalTwoRect& rPosAry,
                            const SalBitmap& rSSalBitmap,
                            SalColor nMaskColor )
{
    DBG_ASSERT( !mrParent.isPrinter(), "No transparency print possible!" );

    assert(dynamic_cast<const WinSalBitmap*>(&rSSalBitmap));

    const WinSalBitmap& rSalBitmap = static_cast<const WinSalBitmap&>(rSSalBitmap);

    SalTwoRect  aPosAry = rPosAry;
    const BYTE  cRed = SALCOLOR_RED( nMaskColor );
    const BYTE  cGreen = SALCOLOR_GREEN( nMaskColor );
    const BYTE  cBlue = SALCOLOR_BLUE( nMaskColor );
    HDC         hDC = mrParent.getHDC();
    HBRUSH      hMaskBrush = CreateSolidBrush( RGB( cRed, cGreen, cBlue ) );
    HBRUSH      hOldBrush = SelectBrush( hDC, hMaskBrush );

    // WIN/WNT seems to have a minor problem mapping the correct color of the
    // mask to the palette if we draw the DIB directly ==> draw DDB
    if( ( GetBitCount() <= 8 ) && rSalBitmap.ImplGethDIB() && rSalBitmap.GetBitCount() == 1 )
    {
        WinSalBitmap aTmp;

        if( aTmp.Create( rSalBitmap, &mrParent ) )
            ImplDrawBitmap( hDC, aPosAry, aTmp, FALSE, 0x00B8074AUL );
    }
    else
        ImplDrawBitmap( hDC, aPosAry, rSalBitmap, FALSE, 0x00B8074AUL );

    SelectBrush( hDC, hOldBrush );
    DeleteBrush( hMaskBrush );
}

SalBitmap* WinSalGraphicsImpl::getBitmap( long nX, long nY, long nDX, long nDY )
{
    DBG_ASSERT( !mrParent.isPrinter(), "No ::GetBitmap() from printer possible!" );

    WinSalBitmap* pSalBitmap = NULL;

    nDX = labs( nDX );
    nDY = labs( nDY );

    HDC     hDC = mrParent.getHDC();
    HBITMAP hBmpBitmap = CreateCompatibleBitmap( hDC, nDX, nDY );
    HDC     hBmpDC = ImplGetCachedDC( CACHED_HDC_1, hBmpBitmap );
    bool    bRet;

    bRet = BitBlt( hBmpDC, 0, 0, (int) nDX, (int) nDY, hDC, (int) nX, (int) nY, SRCCOPY ) ? TRUE : FALSE;
    ImplReleaseCachedDC( CACHED_HDC_1 );

    if( bRet )
    {
        pSalBitmap = new WinSalBitmap;

        if( !pSalBitmap->Create( hBmpBitmap, FALSE, FALSE ) )
        {
            delete pSalBitmap;
            pSalBitmap = NULL;
        }
    }
    else
    {
        // #124826# avoid resource leak! Happens when running without desktop access (remote desktop, service, may be screensavers)
        DeleteBitmap( hBmpBitmap );
    }

    return pSalBitmap;
}

SalColor WinSalGraphicsImpl::getPixel( long nX, long nY )
{
    COLORREF aWinCol = ::GetPixel( mrParent.getHDC(), (int) nX, (int) nY );

    if ( CLR_INVALID == aWinCol )
        return MAKE_SALCOLOR( 0, 0, 0 );
    else
        return MAKE_SALCOLOR( GetRValue( aWinCol ),
                              GetGValue( aWinCol ),
                              GetBValue( aWinCol ) );
}

void WinSalGraphicsImpl::invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags )
{
    if ( nFlags & SAL_INVERT_TRACKFRAME )
    {
        HPEN    hDotPen = CreatePen( PS_DOT, 0, 0 );
        HPEN    hOldPen = SelectPen( mrParent.getHDC(), hDotPen );
        HBRUSH  hOldBrush = SelectBrush( mrParent.getHDC(), GetStockBrush( NULL_BRUSH ) );
        int     nOldROP = SetROP2( mrParent.getHDC(), R2_NOT );

        WIN_Rectangle( mrParent.getHDC(), (int)nX, (int)nY, (int)(nX+nWidth), (int)(nY+nHeight) );

        SetROP2( mrParent.getHDC(), nOldROP );
        SelectPen( mrParent.getHDC(), hOldPen );
        SelectBrush( mrParent.getHDC(), hOldBrush );
        DeletePen( hDotPen );
    }
    else if ( nFlags & SAL_INVERT_50 )
    {
        SalData* pSalData = GetSalData();
        if ( !pSalData->mh50Brush )
        {
            if ( !pSalData->mh50Bmp )
                pSalData->mh50Bmp = ImplLoadSalBitmap( SAL_RESID_BITMAP_50 );
            pSalData->mh50Brush = CreatePatternBrush( pSalData->mh50Bmp );
        }

        COLORREF nOldTextColor = ::SetTextColor( mrParent.getHDC(), 0 );
        HBRUSH hOldBrush = SelectBrush( mrParent.getHDC(), pSalData->mh50Brush );
        PatBlt( mrParent.getHDC(), nX, nY, nWidth, nHeight, PATINVERT );
        ::SetTextColor( mrParent.getHDC(), nOldTextColor );
        SelectBrush( mrParent.getHDC(), hOldBrush );
    }
    else
    {
         RECT aRect;
         aRect.left      = (int)nX;
         aRect.top       = (int)nY;
         aRect.right     = (int)nX+nWidth;
         aRect.bottom    = (int)nY+nHeight;
         ::InvertRect( mrParent.getHDC(), &aRect );
    }
}

void WinSalGraphicsImpl::invert( sal_uInt32 nPoints, const SalPoint* pPtAry, SalInvert nSalFlags )
{
    HPEN        hPen;
    HPEN        hOldPen;
    HBRUSH      hBrush;
    HBRUSH      hOldBrush = 0;
    COLORREF    nOldTextColor RGB(0,0,0);
    int         nOldROP = SetROP2( mrParent.getHDC(), R2_NOT );

    if ( nSalFlags & SAL_INVERT_TRACKFRAME )
        hPen = CreatePen( PS_DOT, 0, 0 );
    else
    {

        if ( nSalFlags & SAL_INVERT_50 )
        {
            SalData* pSalData = GetSalData();
            if ( !pSalData->mh50Brush )
            {
                if ( !pSalData->mh50Bmp )
                    pSalData->mh50Bmp = ImplLoadSalBitmap( SAL_RESID_BITMAP_50 );
                pSalData->mh50Brush = CreatePatternBrush( pSalData->mh50Bmp );
            }

            hBrush = pSalData->mh50Brush;
        }
        else
            hBrush = GetStockBrush( BLACK_BRUSH );

        hPen = GetStockPen( NULL_PEN );
        nOldTextColor = ::SetTextColor( mrParent.getHDC(), 0 );
        hOldBrush = SelectBrush( mrParent.getHDC(), hBrush );
    }
    hOldPen = SelectPen( mrParent.getHDC(), hPen );

    POINT* pWinPtAry;
    // for NT, we can handover the array directly
    static_assert( sizeof( POINT ) == sizeof( SalPoint ), "must be the same size" );

    pWinPtAry = (POINT*)pPtAry;
    // for Windows 95 and its maximum number of points
    if ( nSalFlags & SAL_INVERT_TRACKFRAME )
    {
        if ( !Polyline( mrParent.getHDC(), pWinPtAry, (int)nPoints ) && (nPoints > MAX_64KSALPOINTS) )
            Polyline( mrParent.getHDC(), pWinPtAry, MAX_64KSALPOINTS );
    }
    else
    {
        if ( !Polygon( mrParent.getHDC(), pWinPtAry, (int)nPoints ) && (nPoints > MAX_64KSALPOINTS) )
            Polygon( mrParent.getHDC(), pWinPtAry, MAX_64KSALPOINTS );
    }

    SetROP2( mrParent.getHDC(), nOldROP );
    SelectPen( mrParent.getHDC(), hOldPen );

    if ( nSalFlags & SAL_INVERT_TRACKFRAME )
        DeletePen( hPen );
    else
    {
        ::SetTextColor( mrParent.getHDC(), nOldTextColor );
        SelectBrush( mrParent.getHDC(), hOldBrush );
    }
}

sal_uInt16 WinSalGraphicsImpl::GetBitCount() const
{
    return (sal_uInt16)GetDeviceCaps( mrParent.getHDC(), BITSPIXEL );
}

long WinSalGraphicsImpl::GetGraphicsWidth() const
{
    if( mrParent.gethWnd() && IsWindow( mrParent.gethWnd() ) )
    {
        WinSalFrame* pFrame = GetWindowPtr( mrParent.gethWnd() );
        if( pFrame )
        {
            if( pFrame->maGeometry.nWidth )
                return pFrame->maGeometry.nWidth;
            else
            {
                // TODO: perhaps not needed, maGeometry should always be up-to-date
                RECT aRect;
                GetClientRect( mrParent.gethWnd(), &aRect );
                return aRect.right;
            }
        }
    }

    return 0;
}

void WinSalGraphicsImpl::ResetClipRegion()
{
    if ( mrParent.mhRegion )
    {
        DeleteRegion( mrParent.mhRegion );
        mrParent.mhRegion = 0;
    }

    SelectClipRgn( mrParent.getHDC(), 0 );
}

static bool containsOnlyHorizontalAndVerticalEdges(const basegfx::B2DPolygon& rCandidate)
{
    if(rCandidate.areControlPointsUsed())
    {
        return false;
    }

    const sal_uInt32 nPointCount(rCandidate.count());

    if(nPointCount < 2)
    {
        return true;
    }

    const sal_uInt32 nEdgeCount(rCandidate.isClosed() ? nPointCount + 1 : nPointCount);
    basegfx::B2DPoint aLast(rCandidate.getB2DPoint(0));

    for(sal_uInt32 a(1); a < nEdgeCount; a++)
    {
        const sal_uInt32 nNextIndex(a % nPointCount);
        const basegfx::B2DPoint aCurrent(rCandidate.getB2DPoint(nNextIndex));

        if(!basegfx::fTools::equal(aLast.getX(), aCurrent.getX()) && !basegfx::fTools::equal(aLast.getY(), aCurrent.getY()))
        {
            return false;
        }

        aLast = aCurrent;
    }

    return true;
}

static bool containsOnlyHorizontalAndVerticalEdges(const basegfx::B2DPolyPolygon& rCandidate)
{
    if(rCandidate.areControlPointsUsed())
    {
        return false;
    }

    for(sal_uInt32 a(0); a < rCandidate.count(); a++)
    {
        if(!containsOnlyHorizontalAndVerticalEdges(rCandidate.getB2DPolygon(a)))
        {
            return false;
        }
    }

    return true;
}

bool WinSalGraphicsImpl::setClipRegion( const vcl::Region& i_rClip )
{
    if ( mrParent.mhRegion )
    {
        DeleteRegion( mrParent.mhRegion );
        mrParent.mhRegion = 0;
    }

    bool bUsePolygon(i_rClip.HasPolyPolygonOrB2DPolyPolygon());
    static bool bTryToAvoidPolygon(true);

    // #i122149# try to avoid usage of tools::PolyPolygon ClipRegions when tools::PolyPolygon is no curve
    // and only contains horizontal/vertical edges. In that case, use the fallback
    // in GetRegionRectangles which will use vcl::Region::GetAsRegionBand() which will do
    // the correct polygon-to-RegionBand transformation.
    // Background is that when using the same Rectangle as rectangle or as Polygon
    // clip region will lead to different results; the polygon-based one will be
    // one pixel less to the right and down (see GDI docu for CreatePolygonRgn). This
    // again is because of the polygon-nature and it's classic handling when filling.
    // This also means that all cases which use a 'true' polygon-based incarnation of
    // a vcl::Region should know what they do - it may lead to repaint errors.
    if(bUsePolygon && bTryToAvoidPolygon)
    {
        const basegfx::B2DPolyPolygon aPolyPolygon( i_rClip.GetAsB2DPolyPolygon() );

        if(!aPolyPolygon.areControlPointsUsed())
        {
            if(containsOnlyHorizontalAndVerticalEdges(aPolyPolygon))
            {
                bUsePolygon = false;
            }
        }
    }

    if(bUsePolygon)
    {
        // #i122149# check the comment above to know that this may lead to potential repaint
        // problems. It may be solved (if needed) by scaling the polygon by one in X
        // and Y. Currently the workaround to only use it if really unavoidable will
        // solve most cases. When someone is really using polygon-based Regions he
        // should know what he is doing.
        // Added code to do that scaling to check if it works, testing it.
        const basegfx::B2DPolyPolygon aPolyPolygon( i_rClip.GetAsB2DPolyPolygon() );
        const sal_uInt32 nCount(aPolyPolygon.count());

        if( nCount )
        {
            std::vector< POINT > aPolyPoints;
            aPolyPoints.reserve( 1024 );
            std::vector< INT > aPolyCounts( nCount, 0 );
            basegfx::B2DHomMatrix aExpand;
            static bool bExpandByOneInXandY(true);

            if(bExpandByOneInXandY)
            {
                const basegfx::B2DRange aRangeS(aPolyPolygon.getB2DRange());
                const basegfx::B2DRange aRangeT(aRangeS.getMinimum(), aRangeS.getMaximum() + basegfx::B2DTuple(1.0, 1.0));
                aExpand = basegfx::B2DHomMatrix(basegfx::tools::createSourceRangeTargetRangeTransform(aRangeS, aRangeT));
            }

            for(sal_uInt32 a(0); a < nCount; a++)
            {
                const basegfx::B2DPolygon aPoly(
                    basegfx::tools::adaptiveSubdivideByDistance(
                        aPolyPolygon.getB2DPolygon(a),
                        1));
                const sal_uInt32 nPoints(aPoly.count());
                aPolyCounts[a] = nPoints;

                for( sal_uInt32 b = 0; b < nPoints; b++ )
                {
                    basegfx::B2DPoint aPt(aPoly.getB2DPoint(b));

                    if(bExpandByOneInXandY)
                    {
                        aPt = aExpand * aPt;
                    }

                    POINT aPOINT;
                    // #i122149# do correct rounding
                    aPOINT.x = basegfx::fround(aPt.getX());
                    aPOINT.y = basegfx::fround(aPt.getY());
                    aPolyPoints.push_back( aPOINT );
                }
            }

            mrParent.mhRegion = CreatePolyPolygonRgn( &aPolyPoints[0], &aPolyCounts[0], nCount, ALTERNATE );
        }
    }
    else
    {
        RectangleVector aRectangles;
        i_rClip.GetRegionRectangles(aRectangles);

        sal_uLong nRectBufSize = sizeof(RECT)*aRectangles.size();
        if ( aRectangles.size() < SAL_CLIPRECT_COUNT )
        {
            if ( !mrParent.mpStdClipRgnData )
                mrParent.mpStdClipRgnData = (RGNDATA*)new BYTE[sizeof(RGNDATA)-1+(SAL_CLIPRECT_COUNT*sizeof(RECT))];
            mrParent.mpClipRgnData = mrParent.mpStdClipRgnData;
        }
        else
            mrParent.mpClipRgnData = (RGNDATA*)new BYTE[sizeof(RGNDATA)-1+nRectBufSize];
        mrParent.mpClipRgnData->rdh.dwSize   = sizeof( RGNDATAHEADER );
        mrParent.mpClipRgnData->rdh.iType    = RDH_RECTANGLES;
        mrParent.mpClipRgnData->rdh.nCount   = aRectangles.size();
        mrParent.mpClipRgnData->rdh.nRgnSize = nRectBufSize;
        RECT*       pBoundRect = &(mrParent.mpClipRgnData->rdh.rcBound);
        SetRectEmpty( pBoundRect );
        RECT* pNextClipRect         = (RECT*)(&(mrParent.mpClipRgnData->Buffer));
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
                mrParent.mpClipRgnData->rdh.nCount--;
                mrParent.mpClipRgnData->rdh.nRgnSize -= sizeof( RECT );
            }
        }

        // create clip region from ClipRgnData
        if(0 == mrParent.mpClipRgnData->rdh.nCount)
        {
            // #i123585# region is empty; this may happen when e.g. a tools::PolyPolygon is given
            // that contains no polygons or only empty ones (no width/height). This is
            // perfectly fine and we are done, except setting it (see end of method)
        }
        else if(1 == mrParent.mpClipRgnData->rdh.nCount)
        {
            RECT* pRect = &(mrParent.mpClipRgnData->rdh.rcBound);
            mrParent.mhRegion = CreateRectRgn( pRect->left, pRect->top,
                                                     pRect->right, pRect->bottom );
        }
        else if(mrParent.mpClipRgnData->rdh.nCount > 1)
        {
            sal_uLong nSize = mrParent.mpClipRgnData->rdh.nRgnSize+sizeof(RGNDATAHEADER);
            mrParent.mhRegion = ExtCreateRegion( NULL, nSize, mrParent.mpClipRgnData );

            // if ExtCreateRegion(...) is not supported
            if( !mrParent.mhRegion )
            {
                RGNDATAHEADER* pHeader = (RGNDATAHEADER*) mrParent.mpClipRgnData;

                if( pHeader->nCount )
                {
                    RECT* pRect = (RECT*) mrParent.mpClipRgnData->Buffer;
                    mrParent.mhRegion = CreateRectRgn( pRect->left, pRect->top, pRect->right, pRect->bottom );
                    pRect++;

                    for( sal_uLong n = 1; n < pHeader->nCount; n++, pRect++ )
                    {
                        HRGN hRgn = CreateRectRgn( pRect->left, pRect->top, pRect->right, pRect->bottom );
                        CombineRgn( mrParent.mhRegion, mrParent.mhRegion, hRgn, RGN_OR );
                        DeleteRegion( hRgn );
                    }
                }
            }

            if ( mrParent.mpClipRgnData != mrParent.mpStdClipRgnData )
                delete [] (BYTE*)mrParent.mpClipRgnData;
        }
    }

    if( mrParent.mhRegion )
    {
        SelectClipRgn( mrParent.getHDC(), mrParent.mhRegion );

        // debug code if you want to check range of the newly applied ClipRegion
        //RECT aBound;
        //const int aRegionType = GetRgnBox(mrParent.mhRegion, &aBound);

        //bool bBla = true;
    }
    else
    {
        // #i123585# See above, this is a valid case, execute it
        SelectClipRgn( mrParent.getHDC(), 0 );
    }

    // #i123585# retval no longer dependent of mrParent.mhRegion, see TaskId comments above
    return true;
}

void WinSalGraphicsImpl::SetLineColor()
{
    // create and select new pen
    HPEN hNewPen = GetStockPen( NULL_PEN );
    HPEN hOldPen = SelectPen( mrParent.getHDC(), hNewPen );

    // destroy or save old pen
    if ( mhPen )
    {
        if ( !mbStockPen )
            DeletePen( mhPen );
    }
    else
        mrParent.mhDefPen = hOldPen;

    // set new data
    mhPen       = hNewPen;
    mbPen       = FALSE;
    mbStockPen  = TRUE;
}

void WinSalGraphicsImpl::SetLineColor( SalColor nSalColor )
{
    maLineColor = nSalColor;
    COLORREF    nPenColor = PALETTERGB( SALCOLOR_RED( nSalColor ),
                                        SALCOLOR_GREEN( nSalColor ),
                                        SALCOLOR_BLUE( nSalColor ) );
    HPEN        hNewPen = 0;
    bool        bStockPen = FALSE;

    // search for stock pen (only screen, because printer have problems,
    // when we use stock objects)
    if ( !mrParent.isPrinter() )
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
        if ( !mrParent.isPrinter() )
        {
            if ( GetSalData()->mhDitherPal && ImplIsSysColorEntry( nSalColor ) )
                nPenColor = PALRGB_TO_RGB( nPenColor );
        }

        hNewPen = CreatePen( PS_SOLID, mrParent.mnPenWidth, nPenColor );
        bStockPen = FALSE;
    }

    // select new pen
    HPEN hOldPen = SelectPen( mrParent.getHDC(), hNewPen );

    // destroy or save old pen
    if ( mhPen )
    {
        if ( !mbStockPen )
            DeletePen( mhPen );
    }
    else
        mrParent.mhDefPen = hOldPen;

    // set new data
    mnPenColor  = nPenColor;
    mhPen       = hNewPen;
    mbPen       = TRUE;
    mbStockPen  = bStockPen;
}

void WinSalGraphicsImpl::SetFillColor()
{
    // create and select new brush
    HBRUSH hNewBrush = GetStockBrush( NULL_BRUSH );
    HBRUSH hOldBrush = SelectBrush( mrParent.getHDC(), hNewBrush );

    // destroy or save old brush
    if ( mhBrush )
    {
        if ( !mbStockBrush )
            DeleteBrush( mhBrush );
    }
    else
        mrParent.mhDefBrush = hOldBrush;

    // set new data
    mhBrush     = hNewBrush;
    mbBrush     = FALSE;
    mbStockBrush = TRUE;
}

void WinSalGraphicsImpl::SetFillColor( SalColor nSalColor )
{
    maFillColor = nSalColor;
    SalData*    pSalData    = GetSalData();
    BYTE        nRed        = SALCOLOR_RED( nSalColor );
    BYTE        nGreen      = SALCOLOR_GREEN( nSalColor );
    BYTE        nBlue       = SALCOLOR_BLUE( nSalColor );
    COLORREF    nBrushColor = PALETTERGB( nRed, nGreen, nBlue );
    HBRUSH      hNewBrush   = 0;
    bool        bStockBrush = FALSE;

    // search for stock brush (only screen, because printer have problems,
    // when we use stock objects)
    if ( !mrParent.isPrinter() )
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
        if ( mrParent.isPrinter() || !pSalData->mhDitherDIB )
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
    HBRUSH hOldBrush = SelectBrush( mrParent.getHDC(), hNewBrush );

    // destroy or save old brush
    if ( mhBrush )
    {
        if ( !mbStockBrush )
            DeleteBrush( mhBrush );
    }
    else
        mrParent.mhDefBrush = hOldBrush;

    // set new data
    mnBrushColor = nBrushColor;
    mhBrush     = hNewBrush;
    mbBrush     = TRUE;
    mbStockBrush = bStockBrush;
}

void WinSalGraphicsImpl::SetXORMode( bool bSet, bool )
{
    mbXORMode = bSet;
    ::SetROP2( mrParent.getHDC(), bSet ? R2_XORPEN : R2_COPYPEN );
}

void WinSalGraphicsImpl::SetROPLineColor( SalROPColor nROPColor )
{
    SetLineColor( ImplGetROPSalColor( nROPColor ) );
}

void WinSalGraphicsImpl::SetROPFillColor( SalROPColor nROPColor )
{
    SetFillColor( ImplGetROPSalColor( nROPColor ) );
}

void WinSalGraphicsImpl::drawPixel( long nX, long nY )
{
    if ( mbXORMode )
    {
        HBRUSH  hBrush = CreateSolidBrush( mnPenColor );
        HBRUSH  hOldBrush = SelectBrush( mrParent.getHDC(), hBrush );
        PatBlt( mrParent.getHDC(), (int)nX, (int)nY, (int)1, (int)1, PATINVERT );
        SelectBrush( mrParent.getHDC(), hOldBrush );
        DeleteBrush( hBrush );
    }
    else
        SetPixel( mrParent.getHDC(), (int)nX, (int)nY, mnPenColor );
}

void WinSalGraphicsImpl::drawPixel( long nX, long nY, SalColor nSalColor )
{
    COLORREF nCol = PALETTERGB( SALCOLOR_RED( nSalColor ),
                                SALCOLOR_GREEN( nSalColor ),
                                SALCOLOR_BLUE( nSalColor ) );

    if ( !mrParent.isPrinter() &&
         GetSalData()->mhDitherPal &&
         ImplIsSysColorEntry( nSalColor ) )
        nCol = PALRGB_TO_RGB( nCol );

    if ( mbXORMode )
    {
        HBRUSH  hBrush = CreateSolidBrush( nCol );
        HBRUSH  hOldBrush = SelectBrush( mrParent.getHDC(), hBrush );
        PatBlt( mrParent.getHDC(), (int)nX, (int)nY, (int)1, (int)1, PATINVERT );
        SelectBrush( mrParent.getHDC(), hOldBrush );
        DeleteBrush( hBrush );
    }
    else
        ::SetPixel( mrParent.getHDC(), (int)nX, (int)nY, nCol );
}

void WinSalGraphicsImpl::drawLine( long nX1, long nY1, long nX2, long nY2 )
{
    MoveToEx( mrParent.getHDC(), (int)nX1, (int)nY1, NULL );

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

    LineTo( mrParent.getHDC(), (int)nX2, (int)nY2 );

    if ( bPaintEnd && !mrParent.isPrinter() )
    {
        if ( mbXORMode )
        {
            HBRUSH  hBrush = CreateSolidBrush( mnPenColor );
            HBRUSH  hOldBrush = SelectBrush( mrParent.getHDC(), hBrush );
            PatBlt( mrParent.getHDC(), (int)nX2, (int)nY2, (int)1, (int)1, PATINVERT );
            SelectBrush( mrParent.getHDC(), hOldBrush );
            DeleteBrush( hBrush );
        }
        else
            SetPixel( mrParent.getHDC(), (int)nX2, (int)nY2, mnPenColor );
    }
}

void WinSalGraphicsImpl::drawRect( long nX, long nY, long nWidth, long nHeight )
{
    if ( !mbPen )
    {
        if ( !mrParent.isPrinter() )
        {
            PatBlt( mrParent.getHDC(), (int)nX, (int)nY, (int)nWidth, (int)nHeight,
                    mbXORMode ? PATINVERT : PATCOPY );
        }
        else
        {
            RECT aWinRect;
            aWinRect.left   = nX;
            aWinRect.top    = nY;
            aWinRect.right  = nX+nWidth;
            aWinRect.bottom = nY+nHeight;
            ::FillRect( mrParent.getHDC(), &aWinRect, mhBrush );
        }
    }
    else
        WIN_Rectangle( mrParent.getHDC(), (int)nX, (int)nY, (int)(nX+nWidth), (int)(nY+nHeight) );
}

void WinSalGraphicsImpl::drawPolyLine( sal_uInt32 nPoints, const SalPoint* pPtAry )
{
    // for NT, we can handover the array directly
    static_assert( sizeof( POINT ) == sizeof( SalPoint ), "must be the same size" );

    POINT* pWinPtAry = (POINT*)pPtAry;

    // we assume there are at least 2 points (Polyline requires at least 2 point, see MSDN)
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
    if ( !Polyline( mrParent.getHDC(), pWinPtAry, (int)nPoints ) && (nPoints > MAX_64KSALPOINTS) )
        Polyline( mrParent.getHDC(), pWinPtAry, MAX_64KSALPOINTS );

    if ( bPaintEnd && !mrParent.isPrinter() )
    {
        if ( mbXORMode )
        {
            HBRUSH     hBrush = CreateSolidBrush( mnPenColor );
            HBRUSH     hOldBrush = SelectBrush( mrParent.getHDC(), hBrush );
            PatBlt( mrParent.getHDC(), (int)(pWinPtAry[nPoints-1].x), (int)(pWinPtAry[nPoints-1].y), (int)1, (int)1, PATINVERT );
            SelectBrush( mrParent.getHDC(), hOldBrush );
            DeleteBrush( hBrush );
        }
        else
            SetPixel( mrParent.getHDC(), (int)(pWinPtAry[nPoints-1].x), (int)(pWinPtAry[nPoints-1].y), mnPenColor );
    }
}

void WinSalGraphicsImpl::drawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry )
{
    // for NT, we can handover the array directly
    static_assert( sizeof( POINT ) == sizeof( SalPoint ), "must be the same size" );

    POINT* pWinPtAry = (POINT*)pPtAry;
    // for Windows 95 and its maximum number of points
    if ( !Polygon( mrParent.getHDC(), pWinPtAry, (int)nPoints ) && (nPoints > MAX_64KSALPOINTS) )
        Polygon( mrParent.getHDC(), pWinPtAry, MAX_64KSALPOINTS );
}

void WinSalGraphicsImpl::drawPolyPolygon( sal_uInt32 nPoly, const sal_uInt32* pPoints,
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
    static_assert( sizeof( POINT ) == sizeof( SalPoint ), "must be the same size" );
    UINT            n = 0;
    for ( i = 0; i < (UINT)nPoly; i++ )
    {
        nPoints = pWinPointAry[i];
        const SalPoint* pPolyAry = pPtAry[i];
        memcpy( pWinPointAryAry+n, pPolyAry, (nPoints-1)*sizeof(POINT) );
        pWinPointAryAry[n+nPoints-1] = pWinPointAryAry[n];
        n += nPoints;
    }

    if ( !PolyPolygon( mrParent.getHDC(), pWinPointAryAry, (int*)pWinPointAry, (UINT)nPoly ) &&
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
            Polygon( mrParent.getHDC(), pWinPointAryAry, *pWinPointAry );
        else
            PolyPolygon( mrParent.getHDC(), pWinPointAryAry, (int*)pWinPointAry, nPoly );
    }

    if ( pWinPointAry != aWinPointAry )
        delete [] pWinPointAry;
    if ( pWinPointAryAry != aWinPointAryAry )
        delete [] pWinPointAryAry;
}

bool WinSalGraphicsImpl::drawPolyLineBezier( sal_uInt32 nPoints, const SalPoint* pPtAry, const BYTE* pFlgAry )
{
#ifdef USE_GDI_BEZIERS
    // for NT, we can handover the array directly
    static_assert( sizeof( POINT ) == sizeof( SalPoint ), "must be the same size" );

    ImplRenderPath( mrParent.getHDC(), nPoints, pPtAry, pFlgAry );

    return true;
#else
    return false;
#endif
}

bool WinSalGraphicsImpl::drawPolygonBezier( sal_uInt32 nPoints, const SalPoint* pPtAry, const BYTE* pFlgAry )
{
#ifdef USE_GDI_BEZIERS
    // for NT, we can handover the array directly
    static_assert( sizeof( POINT ) == sizeof( SalPoint ), "must be the same size" );

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

    bool bRet( false );

    if( BeginPath( mrParent.getHDC() ) )
    {
        PolyDraw(mrParent.getHDC(), pWinPointAry, pWinFlagAry, nPoints);

        if( EndPath( mrParent.getHDC() ) )
        {
            if( StrokeAndFillPath( mrParent.getHDC() ) )
                bRet = true;
        }
    }

    if( pWinPointAry != aStackAry1 )
    {
        delete [] pWinPointAry;
        delete [] pWinFlagAry;
    }

    return bRet;
#else
    return false;
#endif
}

bool WinSalGraphicsImpl::drawPolyPolygonBezier( sal_uInt32 nPoly, const sal_uInt32* pPoints,
                                             const SalPoint* const* pPtAry, const BYTE* const* pFlgAry )
{
#ifdef USE_GDI_BEZIERS
    // for NT, we can handover the array directly
    static_assert( sizeof( POINT ) == sizeof( SalPoint ), "must be the same size" );

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

    bool bRet( false );

    if( BeginPath( mrParent.getHDC() ) )
    {
        PolyDraw(mrParent.getHDC(), pWinPointAry, pWinFlagAry, nTotalPoints);

        if( EndPath( mrParent.getHDC() ) )
        {
            if( StrokeAndFillPath( mrParent.getHDC() ) )
                bRet = true;
        }
    }

    if( pWinPointAry != aStackAry1 )
    {
        delete [] pWinPointAry;
        delete [] pWinFlagAry;
    }

    return bRet;
#else
    return false;
#endif
}

void impAddB2DPolygonToGDIPlusGraphicsPathReal(Gdiplus::GpPath *pPath, const basegfx::B2DPolygon& rPolygon, bool bNoLineJoin)
{
    sal_uInt32 nCount(rPolygon.count());

    if(nCount)
    {
        const sal_uInt32 nEdgeCount(rPolygon.isClosed() ? nCount : nCount - 1);
        const bool bControls(rPolygon.areControlPointsUsed());
        basegfx::B2DPoint aCurr(rPolygon.getB2DPoint(0));

        for(sal_uInt32 a(0); a < nEdgeCount; a++)
        {
            const sal_uInt32 nNextIndex((a + 1) % nCount);
            const basegfx::B2DPoint aNext(rPolygon.getB2DPoint(nNextIndex));

            if(bControls && (rPolygon.isNextControlPointUsed(a) || rPolygon.isPrevControlPointUsed(nNextIndex)))
            {
                const basegfx::B2DPoint aCa(rPolygon.getNextControlPoint(a));
                const basegfx::B2DPoint aCb(rPolygon.getPrevControlPoint(nNextIndex));

                Gdiplus::DllExports::GdipAddPathBezier(pPath,
                    aCurr.getX(), aCurr.getY(),
                    aCa.getX(), aCa.getY(),
                    aCb.getX(), aCb.getY(),
                    aNext.getX(), aNext.getY());
            }
            else
            {
                Gdiplus::DllExports::GdipAddPathLine(pPath, aCurr.getX(), aCurr.getY(), aNext.getX(), aNext.getY());
            }

            if(a + 1 < nEdgeCount)
            {
                aCurr = aNext;

                if(bNoLineJoin)
                {
                    Gdiplus::DllExports::GdipStartPathFigure(pPath);
                }
            }
        }
    }
}

void impAddB2DPolygonToGDIPlusGraphicsPathInteger(Gdiplus::GpPath *pPath, const basegfx::B2DPolygon& rPolygon, bool bNoLineJoin)
{
    sal_uInt32 nCount(rPolygon.count());

    if(nCount)
    {
        const sal_uInt32 nEdgeCount(rPolygon.isClosed() ? nCount : nCount - 1);
        const bool bControls(rPolygon.areControlPointsUsed());
        basegfx::B2DPoint aCurr(rPolygon.getB2DPoint(0));

        for(sal_uInt32 a(0); a < nEdgeCount; a++)
        {
            const sal_uInt32 nNextIndex((a + 1) % nCount);
            const basegfx::B2DPoint aNext(rPolygon.getB2DPoint(nNextIndex));

            if(bControls && (rPolygon.isNextControlPointUsed(a) || rPolygon.isPrevControlPointUsed(nNextIndex)))
            {
                const basegfx::B2DPoint aCa(rPolygon.getNextControlPoint(a));
                const basegfx::B2DPoint aCb(rPolygon.getPrevControlPoint(nNextIndex));

                Gdiplus::DllExports::GdipAddPathBezier(
                    pPath,
                    aCurr.getX(), aCurr.getY(),
                    aCa.getX(), aCa.getY(),
                    aCb.getX(), aCb.getY(),
                    aNext.getX(), aNext.getY());
            }
            else
            {
                Gdiplus::DllExports::GdipAddPathLine(pPath, aCurr.getX(), aCurr.getY(), aNext.getX(), aNext.getY());
            }

            if(a + 1 < nEdgeCount)
            {
                aCurr = aNext;

                if(bNoLineJoin)
                {
                    Gdiplus::DllExports::GdipStartPathFigure(pPath);
                }
            }
        }
    }
}

bool WinSalGraphicsImpl::drawPolyPolygon( const basegfx::B2DPolyPolygon& rPolyPolygon, double fTransparency)
{
    const sal_uInt32 nCount(rPolyPolygon.count());

    if(mbBrush && nCount && (fTransparency >= 0.0 && fTransparency < 1.0))
    {
        Gdiplus::GpGraphics *pGraphics = NULL;
        Gdiplus::DllExports::GdipCreateFromHDC(mrParent.getHDC(), &pGraphics);
        const sal_uInt8 aTrans((sal_uInt8)255 - (sal_uInt8)basegfx::fround(fTransparency * 255.0));
        Gdiplus::Color aTestColor(aTrans, SALCOLOR_RED(maFillColor), SALCOLOR_GREEN(maFillColor), SALCOLOR_BLUE(maFillColor));
        Gdiplus::GpSolidFill *pTestBrush;
        Gdiplus::DllExports::GdipCreateSolidFill(aTestColor.GetValue(), &pTestBrush);
        Gdiplus::GpPath *pPath = NULL;
        Gdiplus::DllExports::GdipCreatePath(Gdiplus::FillModeAlternate, &pPath);

        for(sal_uInt32 a(0); a < nCount; a++)
        {
            if(0 != a)
            {
                Gdiplus::DllExports::GdipStartPathFigure(pPath); // #i101491# not needed for first run
            }

            impAddB2DPolygonToGDIPlusGraphicsPathReal(pPath, rPolyPolygon.getB2DPolygon(a), false);
            Gdiplus::DllExports::GdipClosePathFigure(pPath);
        }

        if(mrParent.getAntiAliasB2DDraw())
        {
            Gdiplus::DllExports::GdipSetSmoothingMode(pGraphics, Gdiplus::SmoothingModeAntiAlias);
        }
        else
        {
            Gdiplus::DllExports::GdipSetSmoothingMode(pGraphics, Gdiplus::SmoothingModeNone);
        }

        if(mrParent.isPrinter())
        {
            // #i121591#
            // Normally GdiPlus should not be used for printing at all since printers cannot
            // print transparent filled polygon geometry and normally this does not happen
            // since OutputDevice::RemoveTransparenciesFromMetaFile is used as preparation
            // and no transparent parts should remain for printing. But this can be overridden
            // by the user and thus happens. This call can only come (currently) from
            // OutputDevice::DrawTransparent, see comments there with the same TaskID.
            // If it is used, the mapping for the printer is wrong and needs to be corrected. I
            // checked that there is *no* transformation set and estimated that a stable factor
            // dependent of the printer's DPI is used. Create and set a transformation here to
            // correct this.
            Gdiplus::REAL aDpiX;
            Gdiplus::DllExports::GdipGetDpiX(pGraphics, &aDpiX);
            Gdiplus::REAL aDpiY;
            Gdiplus::DllExports::GdipGetDpiY(pGraphics, &aDpiY);

            Gdiplus::DllExports::GdipResetWorldTransform(pGraphics);
            Gdiplus::DllExports::GdipScaleWorldTransform(pGraphics, Gdiplus::REAL(100.0) / aDpiX, Gdiplus::REAL(100.0) / aDpiY, Gdiplus::MatrixOrderAppend);
        }

        Gdiplus::DllExports::GdipFillPath(pGraphics, pTestBrush, pPath);

        Gdiplus::DllExports::GdipDeletePath(pPath);
        Gdiplus::DllExports::GdipDeleteGraphics(pGraphics);
    }

     return true;
}

bool WinSalGraphicsImpl::drawPolyLine(
    const basegfx::B2DPolygon& rPolygon,
    double fTransparency,
    const basegfx::B2DVector& rLineWidths,
    basegfx::B2DLineJoin eLineJoin,
    css::drawing::LineCap eLineCap)
{
    const sal_uInt32 nCount(rPolygon.count());

    if(mbPen && nCount)
    {
        Gdiplus::GpGraphics *pGraphics = NULL;
        Gdiplus::DllExports::GdipCreateFromHDC(mrParent.getHDC(), &pGraphics);
        const sal_uInt8 aTrans = (sal_uInt8)basegfx::fround( 255 * (1.0 - fTransparency) );
        Gdiplus::Color aTestColor(aTrans, SALCOLOR_RED(maLineColor), SALCOLOR_GREEN(maLineColor), SALCOLOR_BLUE(maLineColor));
        Gdiplus::GpPen *pTestPen = NULL;
        Gdiplus::DllExports::GdipCreatePen1(aTestColor.GetValue(), Gdiplus::REAL(rLineWidths.getX()), Gdiplus::UnitWorld, &pTestPen);
        Gdiplus::GpPath *pPath;
        Gdiplus::DllExports::GdipCreatePath(Gdiplus::FillModeAlternate, &pPath);
        bool bNoLineJoin(false);

        switch(eLineJoin)
        {
            default : // basegfx::B2DLineJoin::NONE :
            {
                if(basegfx::fTools::more(rLineWidths.getX(), 0.0))
                {
                    bNoLineJoin = true;
                }
                break;
            }
            case basegfx::B2DLineJoin::Bevel :
            {
                Gdiplus::DllExports::GdipSetPenLineJoin(pTestPen, Gdiplus::LineJoinBevel);
                break;
            }
            case basegfx::B2DLineJoin::Middle :
            case basegfx::B2DLineJoin::Miter :
            {
                const Gdiplus::REAL aMiterLimit(15.0);
                Gdiplus::DllExports::GdipSetPenMiterLimit(pTestPen, aMiterLimit);
                Gdiplus::DllExports::GdipSetPenLineJoin(pTestPen, Gdiplus::LineJoinMiter);
                break;
            }
            case basegfx::B2DLineJoin::Round :
            {
                Gdiplus::DllExports::GdipSetPenLineJoin(pTestPen, Gdiplus::LineJoinRound);
                break;
            }
        }

        switch(eLineCap)
        {
            default: /*css::drawing::LineCap_BUTT*/
            {
                // nothing to do
                break;
            }
            case css::drawing::LineCap_ROUND:
            {
                Gdiplus::DllExports::GdipSetPenStartCap(pTestPen, Gdiplus::LineCapRound);
                Gdiplus::DllExports::GdipSetPenEndCap(pTestPen, Gdiplus::LineCapRound);
                break;
            }
            case css::drawing::LineCap_SQUARE:
            {
                Gdiplus::DllExports::GdipSetPenStartCap(pTestPen, Gdiplus::LineCapSquare);
                Gdiplus::DllExports::GdipSetPenEndCap(pTestPen, Gdiplus::LineCapSquare);
                break;
            }
        }

        if(nCount > 250 && basegfx::fTools::more(rLineWidths.getX(), 1.5))
        {
            impAddB2DPolygonToGDIPlusGraphicsPathInteger(pPath, rPolygon, bNoLineJoin);
        }
        else
        {
            impAddB2DPolygonToGDIPlusGraphicsPathReal(pPath, rPolygon, bNoLineJoin);
        }

        if(rPolygon.isClosed() && !bNoLineJoin)
        {
            // #i101491# needed to create the correct line joins
            Gdiplus::DllExports::GdipClosePathFigure(pPath);
        }

        if(mrParent.getAntiAliasB2DDraw())
        {
            Gdiplus::DllExports::GdipSetSmoothingMode(pGraphics, Gdiplus::SmoothingModeAntiAlias);
        }
        else
        {
            Gdiplus::DllExports::GdipSetSmoothingMode(pGraphics, Gdiplus::SmoothingModeNone);
        }

        Gdiplus::DllExports::GdipDrawPath(pGraphics, pTestPen, pPath);

        Gdiplus::DllExports::GdipDeletePath(pPath);
        Gdiplus::DllExports::GdipDeletePen(pTestPen);
        Gdiplus::DllExports::GdipDeleteGraphics(pGraphics);
    }

    return true;
}

void paintToGdiPlus(
    Gdiplus::Graphics& rGraphics,
    const SalTwoRect& rTR,
    Gdiplus::Bitmap& rBitmap)
{
    // only parts of source are used
    Gdiplus::PointF aDestPoints[3];
    Gdiplus::ImageAttributes aAttributes;

    // define target region as paralellogram
    aDestPoints[0].X = Gdiplus::REAL(rTR.mnDestX);
    aDestPoints[0].Y = Gdiplus::REAL(rTR.mnDestY);
    aDestPoints[1].X = Gdiplus::REAL(rTR.mnDestX + rTR.mnDestWidth);
    aDestPoints[1].Y = Gdiplus::REAL(rTR.mnDestY);
    aDestPoints[2].X = Gdiplus::REAL(rTR.mnDestX);
    aDestPoints[2].Y = Gdiplus::REAL(rTR.mnDestY + rTR.mnDestHeight);

    aAttributes.SetWrapMode(Gdiplus::WrapModeTileFlipXY);

    rGraphics.DrawImage(
        &rBitmap,
        aDestPoints,
        3,
        Gdiplus::REAL(rTR.mnSrcX),
        Gdiplus::REAL(rTR.mnSrcY),
        Gdiplus::REAL(rTR.mnSrcWidth),
        Gdiplus::REAL(rTR.mnSrcHeight),
        Gdiplus::UnitPixel,
        &aAttributes,
        0,
        0);
}

void setInterpolationMode(
    Gdiplus::Graphics& rGraphics,
    const long& rSrcWidth,
    const long& rDestWidth,
    const long& rSrcHeight,
    const long& rDestHeight)
{
    const bool bSameWidth(rSrcWidth == rDestWidth);
    const bool bSameHeight(rSrcHeight == rDestHeight);

    if(bSameWidth && bSameHeight)
    {
#ifdef __MINGW32__
        //Gdiplus::InterpolationModeInvalid is missing on mingw
        rGraphics.SetInterpolationMode(Gdiplus::InterpolationModeDefault);
#else
        rGraphics.SetInterpolationMode(Gdiplus::InterpolationModeInvalid);
#endif
    }
    else if(rDestWidth > rSrcWidth && rDestHeight > rSrcHeight)
    {
        rGraphics.SetInterpolationMode(Gdiplus::InterpolationModeDefault);
    }
    else if(rDestWidth < rSrcWidth && rDestHeight < rSrcHeight)
    {
        rGraphics.SetInterpolationMode(Gdiplus::InterpolationModeBicubic);
    }
    else
    {
        rGraphics.SetInterpolationMode(Gdiplus::InterpolationModeDefault);
    }
}

bool WinSalGraphicsImpl::tryDrawBitmapGdiPlus(const SalTwoRect& rTR, const SalBitmap& rSrcBitmap)
{
    if(rTR.mnSrcWidth && rTR.mnSrcHeight && rTR.mnDestWidth && rTR.mnDestHeight)
    {
        assert(dynamic_cast<const WinSalBitmap*>(&rSrcBitmap));

        const WinSalBitmap& rSalBitmap = static_cast< const WinSalBitmap& >(rSrcBitmap);
        GdiPlusBmpPtr aARGB(rSalBitmap.ImplGetGdiPlusBitmap());

        if(aARGB.get())
        {
            Gdiplus::Graphics aGraphics(mrParent.getHDC());

            setInterpolationMode(
                aGraphics,
                rTR.mnSrcWidth,
                rTR.mnDestWidth,
                rTR.mnSrcHeight,
                rTR.mnDestHeight);

            paintToGdiPlus(
                aGraphics,
                rTR,
                *aARGB.get());

            return true;
        }
    }

    return false;
}

bool WinSalGraphicsImpl::blendBitmap(
    const SalTwoRect&,
    const SalBitmap&)
{
    return false;
}

bool WinSalGraphicsImpl::blendAlphaBitmap(
    const SalTwoRect&,
    const SalBitmap&,
    const SalBitmap&,
    const SalBitmap&)
{
    return false;
}

bool WinSalGraphicsImpl::drawAlphaBitmap(
    const SalTwoRect& rTR,
    const SalBitmap& rSrcBitmap,
    const SalBitmap& rAlphaBmp)
{
    if(rTR.mnSrcWidth && rTR.mnSrcHeight && rTR.mnDestWidth && rTR.mnDestHeight)
    {
        assert(dynamic_cast<const WinSalBitmap*>(&rSrcBitmap));
        assert(dynamic_cast<const WinSalBitmap*>(&rAlphaBmp));

        const WinSalBitmap& rSalBitmap = static_cast< const WinSalBitmap& >(rSrcBitmap);
        const WinSalBitmap& rSalAlpha = static_cast< const WinSalBitmap& >(rAlphaBmp);
        GdiPlusBmpPtr aARGB(rSalBitmap.ImplGetGdiPlusBitmap(&rSalAlpha));

        if(aARGB.get())
        {
            Gdiplus::Graphics aGraphics(mrParent.getHDC());

            setInterpolationMode(
                aGraphics,
                rTR.mnSrcWidth,
                rTR.mnDestWidth,
                rTR.mnSrcHeight,
                rTR.mnDestHeight);

            paintToGdiPlus(
                aGraphics,
                rTR,
                *aARGB.get());

            return true;
        }
    }

    return false;
}

bool WinSalGraphicsImpl::drawTransformedBitmap(
    const basegfx::B2DPoint& rNull,
    const basegfx::B2DPoint& rX,
    const basegfx::B2DPoint& rY,
    const SalBitmap& rSourceBitmap,
    const SalBitmap* pAlphaBitmap)
{
    assert(dynamic_cast<const WinSalBitmap*>(&rSourceBitmap));
    assert(!pAlphaBitmap || dynamic_cast<const WinSalBitmap*>(pAlphaBitmap));

    const WinSalBitmap& rSalBitmap = static_cast< const WinSalBitmap& >(rSourceBitmap);
    const WinSalBitmap* pSalAlpha = static_cast< const WinSalBitmap* >(pAlphaBitmap);
    GdiPlusBmpPtr aARGB(rSalBitmap.ImplGetGdiPlusBitmap(pSalAlpha));

    if(aARGB.get())
    {
        const long nSrcWidth(aARGB->GetWidth());
        const long nSrcHeight(aARGB->GetHeight());

        if(nSrcWidth && nSrcHeight)
        {
            const long nDestWidth(basegfx::fround(basegfx::B2DVector(rX - rNull).getLength()));
            const long nDestHeight(basegfx::fround(basegfx::B2DVector(rY - rNull).getLength()));

            if(nDestWidth && nDestHeight)
            {
                Gdiplus::Graphics aGraphics(mrParent.getHDC());
                Gdiplus::PointF aDestPoints[3];
                Gdiplus::ImageAttributes aAttributes;

                setInterpolationMode(
                    aGraphics,
                    nSrcWidth,
                    nDestWidth,
                    nSrcHeight,
                    nDestHeight);

                // this mode is only capable of drawing the whole bitmap to a paralellogram
                aDestPoints[0].X = Gdiplus::REAL(rNull.getX());
                aDestPoints[0].Y = Gdiplus::REAL(rNull.getY());
                aDestPoints[1].X = Gdiplus::REAL(rX.getX());
                aDestPoints[1].Y = Gdiplus::REAL(rX.getY());
                aDestPoints[2].X = Gdiplus::REAL(rY.getX());
                aDestPoints[2].Y = Gdiplus::REAL(rY.getY());

                aAttributes.SetWrapMode(Gdiplus::WrapModeTileFlipXY);

                aGraphics.DrawImage(
                    aARGB.get(),
                    aDestPoints,
                    3,
                    Gdiplus::REAL(0.0),
                    Gdiplus::REAL(0.0),
                    Gdiplus::REAL(nSrcWidth),
                    Gdiplus::REAL(nSrcHeight),
                    Gdiplus::UnitPixel,
                    &aAttributes,
                    0,
                    0);
            }
        }

        return true;
    }

    return false;
}

bool WinSalGraphicsImpl::drawGradient(const tools::PolyPolygon& /*rPolygon*/,
        const Gradient& /*rGradient*/)
{
    return false;
}

bool WinSalGraphicsImpl::TryRenderCachedNativeControl(ControlCacheKey& /*rControlCacheKey*/, int /*nX*/, int /*nY*/)
{
    return false;
}

bool WinSalGraphicsImpl::RenderAndCacheNativeControl(OpenGLCompatibleDC& /*rWhite*/, OpenGLCompatibleDC& /*rBlack*/,
        int /*nX*/, int /*nY*/ , ControlCacheKey& /*aControlCacheKey*/)
{
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
