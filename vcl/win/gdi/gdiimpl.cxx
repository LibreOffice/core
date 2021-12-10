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

#include <sal/config.h>

#include <cstdlib>
#include <memory>
#include <numeric>

#include <svsys.h>

#include "gdiimpl.hxx"

#include <string.h>
#include <rtl/strbuf.hxx>
#include <sal/log.hxx>
#include <tools/poly.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <win/wincomp.hxx>
#include <win/saldata.hxx>
#include <win/salgdi.h>
#include <win/salbmp.h>
#include <win/scoped_gdi.hxx>
#include <vcl/BitmapAccessMode.hxx>
#include <vcl/BitmapBuffer.hxx>
#include <vcl/BitmapPalette.hxx>
#include <win/salframe.h>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/utils/systemdependentdata.hxx>

#include <win/salids.hrc>
#include <ControlCacheKey.hxx>

#if defined _MSC_VER
#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#endif
#endif

#include <prewin.h>

#include <gdiplus.h>
#include <gdiplusenums.h>
#include <gdipluscolor.h>

#include <postwin.h>

#define SAL_POLYPOLYCOUNT_STACKBUF          8
#define SAL_POLYPOLYPOINTS_STACKBUF         64

#define SAL_POLY_STACKBUF       32

namespace {

// #100127# Fill point and flag memory from array of points which
// might also contain bezier control points for the PolyDraw() GDI method
// Make sure pWinPointAry and pWinFlagAry are big enough
void ImplPreparePolyDraw( bool                      bCloseFigures,
                          sal_uLong                 nPoly,
                          const sal_uInt32*         pPoints,
                          const Point* const*    pPtAry,
                          const PolyFlags* const*   pFlgAry,
                          POINT*                    pWinPointAry,
                          BYTE*                     pWinFlagAry     )
{
    sal_uLong nCurrPoly;
    for( nCurrPoly=0; nCurrPoly<nPoly; ++nCurrPoly )
    {
        const Point* pCurrPoint = *pPtAry++;
        const PolyFlags* pCurrFlag = *pFlgAry++;
        const sal_uInt32 nCurrPoints = *pPoints++;
        const bool bHaveFlagArray( pCurrFlag );
        sal_uLong nCurrPoint;

        if( nCurrPoints )
        {
            // start figure
            *pWinPointAry++ = POINT { static_cast<LONG>(pCurrPoint->getX()), static_cast<LONG>(pCurrPoint->getY()) };
            pCurrPoint++;
            *pWinFlagAry++  = PT_MOVETO;
            ++pCurrFlag;

            for( nCurrPoint=1; nCurrPoint<nCurrPoints; )
            {
                // #102067# Check existence of flag array
                if( bHaveFlagArray &&
                    ( nCurrPoint + 2 ) < nCurrPoints )
                {
                    PolyFlags P4( pCurrFlag[ 2 ] );

                    if( ( PolyFlags::Control == pCurrFlag[ 0 ] ) &&
                        ( PolyFlags::Control == pCurrFlag[ 1 ] ) &&
                        ( PolyFlags::Normal == P4 || PolyFlags::Smooth == P4 || PolyFlags::Symmetric == P4 ) )
                    {
                        // control point one
                        *pWinPointAry++ = POINT { static_cast<LONG>(pCurrPoint->getX()), static_cast<LONG>(pCurrPoint->getY()) };
                        pCurrPoint++;
                        *pWinFlagAry++  = PT_BEZIERTO;

                        // control point two
                        *pWinPointAry++ = POINT { static_cast<LONG>(pCurrPoint->getX()), static_cast<LONG>(pCurrPoint->getY()) };
                        pCurrPoint++;
                        *pWinFlagAry++  = PT_BEZIERTO;

                        // end point
                        *pWinPointAry++ = POINT { static_cast<LONG>(pCurrPoint->getX()), static_cast<LONG>(pCurrPoint->getY()) };
                        pCurrPoint++;
                        *pWinFlagAry++  = PT_BEZIERTO;

                        nCurrPoint += 3;
                        pCurrFlag += 3;
                        continue;
                    }
                }

                // regular line point
                *pWinPointAry++ = POINT { static_cast<LONG>(pCurrPoint->getX()), static_cast<LONG>(pCurrPoint->getY()) };
                pCurrPoint++;
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

Color ImplGetROPColor( SalROPColor nROPColor )
{
    Color nColor;
    if ( nROPColor == SalROPColor::N0 )
        nColor = Color( 0, 0, 0 );
    else
        nColor = Color( 255, 255, 255 );
    return nColor;
}

bool IsDitherColor(BYTE nRed, BYTE nGreen, BYTE nBlue)
{
    constexpr sal_uInt8 DITHER_PAL_DELTA = 51;

    return !(nRed % DITHER_PAL_DELTA) &&
           !(nGreen % DITHER_PAL_DELTA) &&
           !(nBlue % DITHER_PAL_DELTA);
}

bool IsPaletteColor(BYTE nRed, BYTE nGreen, BYTE nBlue)
{
    static const PALETTEENTRY aImplSalSysPalEntryAry[] =
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

    for (const auto& rPalEntry : aImplSalSysPalEntryAry)
    {
        if(rPalEntry.peRed == nRed &&
           rPalEntry.peGreen == nGreen &&
           rPalEntry.peBlue == nBlue)
        {
            return true;
        }
    }

    return false;
}

bool IsExtraColor(BYTE nRed, BYTE nGreen, BYTE nBlue)
{
    return (nRed == 0) && (nGreen == 184) && (nBlue == 255);
}

bool ImplIsPaletteEntry(BYTE nRed, BYTE nGreen, BYTE nBlue)
{
    return IsDitherColor(nRed, nGreen, nBlue) ||
           IsPaletteColor(nRed, nGreen, nBlue) ||
           IsExtraColor(nRed, nGreen, nBlue);
}

} // namespace

WinSalGraphicsImpl::WinSalGraphicsImpl(WinSalGraphics& rParent):
    mrParent(rParent),
    mbXORMode(false),
    mbPen(false),
    mhPen(nullptr),
    mbStockPen(false),
    mbBrush(false),
    mbStockBrush(false),
    mhBrush(nullptr)
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

bool WinSalGraphicsImpl::drawEPS(tools::Long, tools::Long, tools::Long, tools::Long, void*, sal_uInt32)
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
                static_cast<int>(rPosAry.mnDestX), static_cast<int>(rPosAry.mnDestY),
                static_cast<int>(rPosAry.mnDestWidth), static_cast<int>(rPosAry.mnDestHeight),
                hSrcDC,
                static_cast<int>(rPosAry.mnSrcX), static_cast<int>(rPosAry.mnSrcY),
                nRop );
    }
    else
    {
        int nOldStretchMode = SetStretchBltMode( mrParent.getHDC(), STRETCH_DELETESCANS );
        StretchBlt( mrParent.getHDC(),
                    static_cast<int>(rPosAry.mnDestX), static_cast<int>(rPosAry.mnDestY),
                    static_cast<int>(rPosAry.mnDestWidth), static_cast<int>(rPosAry.mnDestHeight),
                    hSrcDC,
                    static_cast<int>(rPosAry.mnSrcX), static_cast<int>(rPosAry.mnSrcY),
                    static_cast<int>(rPosAry.mnSrcWidth), static_cast<int>(rPosAry.mnSrcHeight),
                    nRop );
        SetStretchBltMode( mrParent.getHDC(), nOldStretchMode );
    }
}

namespace
{

void MakeInvisibleArea(const RECT& rSrcRect,
                       int nLeft, int nTop, int nRight, int nBottom,
                       HRGN& rhInvalidateRgn)
{
    if (!rhInvalidateRgn)
    {
        rhInvalidateRgn = CreateRectRgnIndirect(&rSrcRect);
    }

    ScopedHRGN hTempRgn(CreateRectRgn(nLeft, nTop, nRight, nBottom));
    CombineRgn(rhInvalidateRgn, rhInvalidateRgn, hTempRgn.get(), RGN_DIFF);
}

void ImplCalcOutSideRgn( const RECT& rSrcRect,
                         int nLeft, int nTop, int nRight, int nBottom,
                         HRGN& rhInvalidateRgn )
{
    // calculate area outside the visible region
    if (rSrcRect.left < nLeft)
    {
        MakeInvisibleArea(rSrcRect, -31999, 0, nLeft, 31999, rhInvalidateRgn);
    }
    if (rSrcRect.top < nTop)
    {
        MakeInvisibleArea(rSrcRect, 0, -31999, 31999, nTop, rhInvalidateRgn);
    }
    if (rSrcRect.right > nRight)
    {
        MakeInvisibleArea(rSrcRect, nRight, 0, 31999, 31999, rhInvalidateRgn);
    }
    if (rSrcRect.bottom > nBottom)
    {
        MakeInvisibleArea(rSrcRect, 0, nBottom, 31999, 31999, rhInvalidateRgn);
    }
}

} // namespace

void WinSalGraphicsImpl::copyArea( tools::Long nDestX, tools::Long nDestY,
                            tools::Long nSrcX, tools::Long nSrcY,
                            tools::Long nSrcWidth, tools::Long nSrcHeight,
                            bool bWindowInvalidate )
{
    bool    bRestoreClipRgn = false;
    HRGN    hOldClipRgn = nullptr;
    int     nOldClipRgnType = ERROR;
    HRGN    hInvalidateRgn = nullptr;

    // do we have to invalidate also the overlapping regions?
    if ( bWindowInvalidate && mrParent.isWindow() )
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
        aSrcRect.left   = static_cast<int>(nSrcX);
        aSrcRect.top    = static_cast<int>(nSrcY);
        aSrcRect.right  = aSrcRect.left+static_cast<int>(nSrcWidth);
        aSrcRect.bottom = aSrcRect.top+static_cast<int>(nSrcHeight);
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
            hInvalidateRgn = nullptr;

            // compute the parts that are off screen (ie invisible)
            RECT theScreen;
            ImplSalGetWorkArea( nullptr, &theScreen, nullptr );  // find the screen area taking multiple monitors into account
            ImplCalcOutSideRgn( aSrcRect, theScreen.left, theScreen.top, theScreen.right, theScreen.bottom, hInvalidateRgn );

            // calculate regions that are covered by other windows
            HRGN hTempRgn2 = nullptr;
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

                // If one or more Parents clip our window, then we must
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
                    int nOffX = static_cast<int>(nDestX-nSrcX);
                    int nOffY = static_cast<int>(nDestY-nSrcY);
                    OffsetRgn( hInvalidateRgn, nOffX-aPt.x, nOffY-aPt.y );

                    // by excluding hInvalidateRgn from the system's clip region
                    // we will prevent bitblt from copying useless data
                    // especially now shadows from overlapping windows will appear (#i36344)
                    hOldClipRgn = CreateRectRgn( 0, 0, 0, 0 );
                    nOldClipRgnType = GetClipRgn( mrParent.getHDC(), hOldClipRgn );

                    bRestoreClipRgn = true; // indicate changed clipregion and force invalidate
                    ExtSelectClipRgn( mrParent.getHDC(), hInvalidateRgn, RGN_DIFF );
                }
            }
        }
    }

    BitBlt( mrParent.getHDC(),
            static_cast<int>(nDestX), static_cast<int>(nDestY),
            static_cast<int>(nSrcWidth), static_cast<int>(nSrcHeight),
            mrParent.getHDC(),
            static_cast<int>(nSrcX), static_cast<int>(nSrcY),
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
        std::unique_ptr<WinSalBitmap> xTmpSalBmp;
        bool        bPrintDDB = ( bPrinter && hDrawDDB );

        if( bPrintDDB )
        {
            xTmpSalBmp.reset(new WinSalBitmap);
            xTmpSalBmp->Create(rSalBitmap, vcl::bitDepthToPixelFormat(rSalBitmap.GetBitCount()));
            hDrawDIB = xTmpSalBmp->ImplGethDIB();
        }
        else
            hDrawDIB = rSalBitmap.ImplGethDIB();

        if( hDrawDIB )
        {
            PBITMAPINFO         pBI = static_cast<PBITMAPINFO>(GlobalLock( hDrawDIB ));
            PBYTE               pBits = reinterpret_cast<PBYTE>(pBI) + pBI->bmiHeader.biSize +
                                        WinSalBitmap::ImplGetDIBColorCount( hDrawDIB ) * sizeof( RGBQUAD );
            const int           nOldStretchMode = SetStretchBltMode( hDC, STRETCH_DELETESCANS );

            StretchDIBits( hDC,
                           static_cast<int>(rPosAry.mnDestX), static_cast<int>(rPosAry.mnDestY),
                           static_cast<int>(rPosAry.mnDestWidth), static_cast<int>(rPosAry.mnDestHeight),
                           static_cast<int>(rPosAry.mnSrcX), static_cast<int>(pBI->bmiHeader.biHeight - rPosAry.mnSrcHeight - rPosAry.mnSrcY),
                           static_cast<int>(rPosAry.mnSrcWidth), static_cast<int>(rPosAry.mnSrcHeight),
                           pBits, pBI, DIB_RGB_COLORS, nDrawMode );

            GlobalUnlock( hDrawDIB );
            SetStretchBltMode( hDC, nOldStretchMode );
        }
        else if( hDrawDDB && !bPrintDDB )
        {
            ScopedCachedHDC<CACHED_HDC_DRAW> hBmpDC(hDrawDDB);

            COLORREF    nOldBkColor = RGB(0xFF,0xFF,0xFF);
            COLORREF    nOldTextColor = RGB(0,0,0);
            bool        bMono = ( rSalBitmap.GetBitCount() == 1 );

            if( bMono )
            {
                COLORREF nBkColor = RGB( 0xFF, 0xFF, 0xFF );
                COLORREF nTextColor = RGB( 0x00, 0x00, 0x00 );
                //fdo#33455 handle 1 bit depth pngs with palette entries
                //to set fore/back colors
                if (BitmapBuffer* pBitmapBuffer = const_cast<WinSalBitmap&>(rSalBitmap).AcquireBuffer(BitmapAccessMode::Info))
                {
                    const BitmapPalette& rPalette = pBitmapBuffer->maPalette;
                    if (rPalette.GetEntryCount() == 2)
                    {
                        Color nCol = rPalette[0];
                        nTextColor = RGB( nCol.GetRed(), nCol.GetGreen(), nCol.GetBlue() );
                        nCol = rPalette[1];
                        nBkColor = RGB( nCol.GetRed(), nCol.GetGreen(), nCol.GetBlue() );
                    }
                    const_cast<WinSalBitmap&>(rSalBitmap).ReleaseBuffer(pBitmapBuffer, BitmapAccessMode::Info);
                }
                nOldBkColor = SetBkColor( hDC, nBkColor );
                nOldTextColor = ::SetTextColor( hDC, nTextColor );
            }

            if ( (rPosAry.mnSrcWidth  == rPosAry.mnDestWidth) &&
                 (rPosAry.mnSrcHeight == rPosAry.mnDestHeight) )
            {
                BitBlt( hDC,
                        static_cast<int>(rPosAry.mnDestX), static_cast<int>(rPosAry.mnDestY),
                        static_cast<int>(rPosAry.mnDestWidth), static_cast<int>(rPosAry.mnDestHeight),
                        hBmpDC.get(),
                        static_cast<int>(rPosAry.mnSrcX), static_cast<int>(rPosAry.mnSrcY),
                        nDrawMode );
            }
            else
            {
                const int nOldStretchMode = SetStretchBltMode( hDC, STRETCH_DELETESCANS );

                StretchBlt( hDC,
                            static_cast<int>(rPosAry.mnDestX), static_cast<int>(rPosAry.mnDestY),
                            static_cast<int>(rPosAry.mnDestWidth), static_cast<int>(rPosAry.mnDestHeight),
                            hBmpDC.get(),
                            static_cast<int>(rPosAry.mnSrcX), static_cast<int>(rPosAry.mnSrcY),
                            static_cast<int>(rPosAry.mnSrcWidth), static_cast<int>(rPosAry.mnSrcHeight),
                            nDrawMode );

                SetStretchBltMode( hDC, nOldStretchMode );
            }

            if( bMono )
            {
                SetBkColor( hDC, nOldBkColor );
                ::SetTextColor( hDC, nOldTextColor );
            }
        }
    }
}

} // namespace

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
    if(bTryDirectPaint && TryDrawBitmapGDIPlus(rPosAry, rSalBitmap))
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
    SAL_WARN_IF( mrParent.isPrinter(), "vcl", "No transparency print possible!" );
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
    int         nDstX = static_cast<int>(aPosAry.mnDestX);
    int         nDstY = static_cast<int>(aPosAry.mnDestY);
    int         nDstWidth = static_cast<int>(aPosAry.mnDestWidth);
    int         nDstHeight = static_cast<int>(aPosAry.mnDestHeight);
    HDC         hDC = mrParent.getHDC();

    ScopedHBITMAP hMemBitmap;
    ScopedHBITMAP hMaskBitmap;

    if( ( nDstWidth > CACHED_HDC_DEFEXT ) || ( nDstHeight > CACHED_HDC_DEFEXT ) )
    {
        hMemBitmap.reset(CreateCompatibleBitmap(hDC, nDstWidth, nDstHeight));
        hMaskBitmap.reset(CreateCompatibleBitmap(hDC, nDstWidth, nDstHeight));
    }

    ScopedCachedHDC<CACHED_HDC_1> hMemDC(hMemBitmap.get());
    ScopedCachedHDC<CACHED_HDC_2> hMaskDC(hMaskBitmap.get());

    aPosAry.mnDestX = aPosAry.mnDestY = 0;
    BitBlt( hMemDC.get(), 0, 0, nDstWidth, nDstHeight, hDC, nDstX, nDstY, SRCCOPY );

    // WIN/WNT seems to have a minor problem mapping the correct color of the
    // mask to the palette if we draw the DIB directly ==> draw DDB
    if( ( GetBitCount() <= 8 ) && rTransparentBitmap.ImplGethDIB() && rTransparentBitmap.GetBitCount() == 1 )
    {
        WinSalBitmap aTmp;

        if( aTmp.Create( rTransparentBitmap, &mrParent ) )
            ImplDrawBitmap( hMaskDC.get(), aPosAry, aTmp, false, SRCCOPY );
    }
    else
        ImplDrawBitmap( hMaskDC.get(), aPosAry, rTransparentBitmap, false, SRCCOPY );

    // now MemDC contains background, MaskDC the transparency mask

    // #105055# Respect XOR mode
    if( mbXORMode )
    {
        ImplDrawBitmap( hMaskDC.get(), aPosAry, rSalBitmap, false, SRCERASE );
        // now MaskDC contains the bitmap area with black background
        BitBlt( hMemDC.get(), 0, 0, nDstWidth, nDstHeight, hMaskDC.get(), 0, 0, SRCINVERT );
        // now MemDC contains background XORed bitmap area on top
    }
    else
    {
        BitBlt( hMemDC.get(), 0, 0, nDstWidth, nDstHeight, hMaskDC.get(), 0, 0, SRCAND );
        // now MemDC contains background with masked-out bitmap area
        ImplDrawBitmap( hMaskDC.get(), aPosAry, rSalBitmap, false, SRCERASE );
        // now MaskDC contains the bitmap area with black background
        BitBlt( hMemDC.get(), 0, 0, nDstWidth, nDstHeight, hMaskDC.get(), 0, 0, SRCPAINT );
        // now MemDC contains background and bitmap merged together
    }
    // copy to output DC
    BitBlt( hDC, nDstX, nDstY, nDstWidth, nDstHeight, hMemDC.get(), 0, 0, SRCCOPY );
}

bool WinSalGraphicsImpl::drawAlphaRect( tools::Long nX, tools::Long nY, tools::Long nWidth,
                                    tools::Long nHeight, sal_uInt8 nTransparency )
{
    if( mbPen || !mbBrush || mbXORMode )
        return false; // can only perform solid fills without XOR.

    ScopedCachedHDC<CACHED_HDC_1> hMemDC(nullptr);
    SetPixel( hMemDC.get(), int(0), int(0), mnBrushColor );

    BLENDFUNCTION aFunc = {
        AC_SRC_OVER,
        0,
        sal::static_int_cast<sal_uInt8>(255 - 255L*nTransparency/100),
        0
    };

    // hMemDC contains a 1x1 bitmap of the right color - stretch-blit
    // that to dest hdc
    bool bRet = GdiAlphaBlend(mrParent.getHDC(), nX, nY, nWidth, nHeight,
                              hMemDC.get(), 0,0,1,1,
                              aFunc ) == TRUE;

    return bRet;
}

void WinSalGraphicsImpl::drawMask(const SalTwoRect& rPosAry,
                                  const SalBitmap& rSSalBitmap,
                                  Color nMaskColor)
{
    SAL_WARN_IF( mrParent.isPrinter(), "vcl", "No transparency print possible!" );

    assert(dynamic_cast<const WinSalBitmap*>(&rSSalBitmap));

    const WinSalBitmap& rSalBitmap = static_cast<const WinSalBitmap&>(rSSalBitmap);

    SalTwoRect  aPosAry = rPosAry;
    const HDC hDC = mrParent.getHDC();

    ScopedSelectedHBRUSH hBrush(hDC, CreateSolidBrush(RGB(nMaskColor.GetRed(),
                                                          nMaskColor.GetGreen(),
                                                          nMaskColor.GetBlue())));

    // WIN/WNT seems to have a minor problem mapping the correct color of the
    // mask to the palette if we draw the DIB directly ==> draw DDB
    if( ( GetBitCount() <= 8 ) && rSalBitmap.ImplGethDIB() && rSalBitmap.GetBitCount() == 1 )
    {
        WinSalBitmap aTmp;

        if( aTmp.Create( rSalBitmap, &mrParent ) )
            ImplDrawBitmap( hDC, aPosAry, aTmp, false, 0x00B8074AUL );
    }
    else
        ImplDrawBitmap( hDC, aPosAry, rSalBitmap, false, 0x00B8074AUL );
}

std::shared_ptr<SalBitmap> WinSalGraphicsImpl::getBitmap( tools::Long nX, tools::Long nY, tools::Long nDX, tools::Long nDY )
{
    SAL_WARN_IF( mrParent.isPrinter(), "vcl", "No ::GetBitmap() from printer possible!" );

    std::shared_ptr<WinSalBitmap> pSalBitmap;

    nDX = std::abs( nDX );
    nDY = std::abs( nDY );

    HDC     hDC = mrParent.getHDC();
    HBITMAP hBmpBitmap = CreateCompatibleBitmap( hDC, nDX, nDY );
    bool    bRet;

    {
        ScopedCachedHDC<CACHED_HDC_1> hBmpDC(hBmpBitmap);

        bRet = BitBlt(hBmpDC.get(), 0, 0,
                      static_cast<int>(nDX), static_cast<int>(nDY), hDC,
                      static_cast<int>(nX), static_cast<int>(nY), SRCCOPY) ? TRUE : FALSE;
    }

    if( bRet )
    {
        pSalBitmap = std::make_shared<WinSalBitmap>();

        if( !pSalBitmap->Create( hBmpBitmap ) )
        {
            pSalBitmap.reset();
        }
    }
    else
    {
        // #124826# avoid resource leak! Happens when running without desktop access (remote desktop, service, may be screensavers)
        DeleteBitmap( hBmpBitmap );
    }

    return pSalBitmap;
}

Color WinSalGraphicsImpl::getPixel( tools::Long nX, tools::Long nY )
{
    COLORREF aWinCol = ::GetPixel( mrParent.getHDC(), static_cast<int>(nX), static_cast<int>(nY) );

    if ( CLR_INVALID == aWinCol )
        return Color( 0, 0, 0 );
    else
        return Color( GetRValue( aWinCol ),
                              GetGValue( aWinCol ),
                              GetBValue( aWinCol ) );
}

namespace
{

HBRUSH Get50PercentBrush()
{
    SalData* pSalData = GetSalData();
    if ( !pSalData->mh50Brush )
    {
        if ( !pSalData->mh50Bmp )
            pSalData->mh50Bmp = ImplLoadSalBitmap( SAL_RESID_BITMAP_50 );
        pSalData->mh50Brush = CreatePatternBrush( pSalData->mh50Bmp );
    }

    return pSalData->mh50Brush;
}

} // namespace

void WinSalGraphicsImpl::invert( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight, SalInvert nFlags )
{
    if ( nFlags & SalInvert::TrackFrame )
    {
        HPEN    hDotPen = CreatePen( PS_DOT, 0, 0 );
        HPEN    hOldPen = SelectPen( mrParent.getHDC(), hDotPen );
        HBRUSH  hOldBrush = SelectBrush( mrParent.getHDC(), GetStockBrush( NULL_BRUSH ) );
        int     nOldROP = SetROP2( mrParent.getHDC(), R2_NOT );

        Rectangle( mrParent.getHDC(), static_cast<int>(nX), static_cast<int>(nY), static_cast<int>(nX+nWidth), static_cast<int>(nY+nHeight) );

        SetROP2( mrParent.getHDC(), nOldROP );
        SelectPen( mrParent.getHDC(), hOldPen );
        SelectBrush( mrParent.getHDC(), hOldBrush );
        DeletePen( hDotPen );
    }
    else if ( nFlags & SalInvert::N50 )
    {
        COLORREF nOldTextColor = ::SetTextColor( mrParent.getHDC(), 0 );
        HBRUSH hOldBrush = SelectBrush( mrParent.getHDC(), Get50PercentBrush() );
        PatBlt( mrParent.getHDC(), nX, nY, nWidth, nHeight, PATINVERT );
        ::SetTextColor( mrParent.getHDC(), nOldTextColor );
        SelectBrush( mrParent.getHDC(), hOldBrush );
    }
    else
    {
         RECT aRect;
         aRect.left      = static_cast<int>(nX);
         aRect.top       = static_cast<int>(nY);
         aRect.right     = static_cast<int>(nX)+nWidth;
         aRect.bottom    = static_cast<int>(nY)+nHeight;
         ::InvertRect( mrParent.getHDC(), &aRect );
    }
}

void WinSalGraphicsImpl::invert( sal_uInt32 nPoints, const Point* pPtAry, SalInvert nSalFlags )
{
    HPEN        hPen;
    HPEN        hOldPen;
    HBRUSH      hBrush;
    HBRUSH      hOldBrush = nullptr;
    COLORREF    nOldTextColor RGB(0,0,0);
    int         nOldROP = SetROP2( mrParent.getHDC(), R2_NOT );

    if ( nSalFlags & SalInvert::TrackFrame )
        hPen = CreatePen( PS_DOT, 0, 0 );
    else
    {

        if ( nSalFlags & SalInvert::N50 )
            hBrush = Get50PercentBrush();
        else
            hBrush = GetStockBrush( BLACK_BRUSH );

        hPen = GetStockPen( NULL_PEN );
        nOldTextColor = ::SetTextColor( mrParent.getHDC(), 0 );
        hOldBrush = SelectBrush( mrParent.getHDC(), hBrush );
    }
    hOldPen = SelectPen( mrParent.getHDC(), hPen );

    std::unique_ptr<POINT[]> pWinPtAry(new POINT[nPoints]);
    for (sal_uInt32 i=0; i<nPoints; ++i)
        pWinPtAry[i] = POINT { static_cast<LONG>(pPtAry[i].getX()), static_cast<LONG>(pPtAry[i].getY()) };

    // for Windows 95 and its maximum number of points
    if ( nSalFlags & SalInvert::TrackFrame )
    {
        if ( !Polyline( mrParent.getHDC(), pWinPtAry.get(), static_cast<int>(nPoints) ) && (nPoints > MAX_64KSALPOINTS) )
            Polyline( mrParent.getHDC(), pWinPtAry.get(), MAX_64KSALPOINTS );
    }
    else
    {
        if ( !Polygon( mrParent.getHDC(), pWinPtAry.get(), static_cast<int>(nPoints) ) && (nPoints > MAX_64KSALPOINTS) )
            Polygon( mrParent.getHDC(), pWinPtAry.get(), MAX_64KSALPOINTS );
    }

    SetROP2( mrParent.getHDC(), nOldROP );
    SelectPen( mrParent.getHDC(), hOldPen );

    if ( nSalFlags & SalInvert::TrackFrame )
        DeletePen( hPen );
    else
    {
        ::SetTextColor( mrParent.getHDC(), nOldTextColor );
        SelectBrush( mrParent.getHDC(), hOldBrush );
    }
}

sal_uInt16 WinSalGraphicsImpl::GetBitCount() const
{
    return static_cast<sal_uInt16>(GetDeviceCaps( mrParent.getHDC(), BITSPIXEL ));
}

tools::Long WinSalGraphicsImpl::GetGraphicsWidth() const
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
        mrParent.mhRegion = nullptr;
    }

    SelectClipRgn( mrParent.getHDC(), nullptr );
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

    for(auto const& rPolygon : rCandidate)
    {
        if(!containsOnlyHorizontalAndVerticalEdges(rPolygon))
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
        mrParent.mhRegion = nullptr;
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
            sal_uInt32 nTargetCount(0);
            static bool bExpandByOneInXandY(true);

            if(bExpandByOneInXandY)
            {
                const basegfx::B2DRange aRangeS(aPolyPolygon.getB2DRange());
                const basegfx::B2DRange aRangeT(aRangeS.getMinimum(), aRangeS.getMaximum() + basegfx::B2DTuple(1.0, 1.0));
                aExpand = basegfx::utils::createSourceRangeTargetRangeTransform(aRangeS, aRangeT);
            }

            for(auto const& rPolygon : aPolyPolygon)
            {
                const basegfx::B2DPolygon aPoly(
                    basegfx::utils::adaptiveSubdivideByDistance(
                        rPolygon,
                        1));
                const sal_uInt32 nPoints(aPoly.count());

                // tdf#40863 For CustomShapes there is a hack (see
                // f64ef72743e55389e446e0d4bc6febd475011023) that adds polygons
                // with a single point in top-left and bottom-right corner
                // of the BoundRect to be able to determine the correct BoundRect
                // in the slideshow. Unfortunately, CreatePolyPolygonRgn below
                // fails with polygons containing a single pixel, so clipping is
                // lost. For now, use only polygons with more than two points - the
                // ones that may have an area.
                // Note: polygons with one point which are curves may have an area,
                // but the polygon is already subdivided here, so no need to test
                // this.
                if(nPoints > 2)
                {
                    aPolyCounts[nTargetCount] = nPoints;
                    nTargetCount++;

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
            }

            if(nTargetCount)
            {
                mrParent.mhRegion = CreatePolyPolygonRgn( aPolyPoints.data(), aPolyCounts.data(), nTargetCount, ALTERNATE );
            }
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
                mrParent.mpStdClipRgnData = reinterpret_cast<RGNDATA*>(new BYTE[sizeof(RGNDATA)-1+(SAL_CLIPRECT_COUNT*sizeof(RECT))]);
            mrParent.mpClipRgnData = mrParent.mpStdClipRgnData;
        }
        else
            mrParent.mpClipRgnData = reinterpret_cast<RGNDATA*>(new BYTE[sizeof(RGNDATA)-1+nRectBufSize]);
        mrParent.mpClipRgnData->rdh.dwSize   = sizeof( RGNDATAHEADER );
        mrParent.mpClipRgnData->rdh.iType    = RDH_RECTANGLES;
        mrParent.mpClipRgnData->rdh.nCount   = aRectangles.size();
        mrParent.mpClipRgnData->rdh.nRgnSize = nRectBufSize;
        RECT*       pBoundRect = &(mrParent.mpClipRgnData->rdh.rcBound);
        SetRectEmpty( pBoundRect );
        RECT* pNextClipRect         = reinterpret_cast<RECT*>(&(mrParent.mpClipRgnData->Buffer));
        bool bFirstClipRect         = true;

        for (auto const& rectangle : aRectangles)
        {
            const tools::Long nW(rectangle.GetWidth());
            const tools::Long nH(rectangle.GetHeight());

            if(nW && nH)
            {
                const tools::Long nRight(rectangle.Left() + nW);
                const tools::Long nBottom(rectangle.Top() + nH);

                if(bFirstClipRect)
                {
                    pBoundRect->left = rectangle.Left();
                    pBoundRect->top = rectangle.Top();
                    pBoundRect->right = nRight;
                    pBoundRect->bottom = nBottom;
                    bFirstClipRect = false;
                }
                else
                {
                    if(rectangle.Left() < pBoundRect->left)
                    {
                        pBoundRect->left = static_cast<int>(rectangle.Left());
                    }

                    if(rectangle.Top() < pBoundRect->top)
                    {
                        pBoundRect->top = static_cast<int>(rectangle.Top());
                    }

                    if(nRight > pBoundRect->right)
                    {
                        pBoundRect->right = static_cast<int>(nRight);
                    }

                    if(nBottom > pBoundRect->bottom)
                    {
                        pBoundRect->bottom = static_cast<int>(nBottom);
                    }
                }

                pNextClipRect->left = static_cast<int>(rectangle.Left());
                pNextClipRect->top = static_cast<int>(rectangle.Top());
                pNextClipRect->right = static_cast<int>(nRight);
                pNextClipRect->bottom = static_cast<int>(nBottom);
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
            mrParent.mhRegion = ExtCreateRegion( nullptr, nSize, mrParent.mpClipRgnData );

            // if ExtCreateRegion(...) is not supported
            if( !mrParent.mhRegion )
            {
                RGNDATAHEADER const & pHeader = mrParent.mpClipRgnData->rdh;

                if( pHeader.nCount )
                {
                    RECT* pRect = reinterpret_cast<RECT*>(mrParent.mpClipRgnData->Buffer);
                    mrParent.mhRegion = CreateRectRgn( pRect->left, pRect->top, pRect->right, pRect->bottom );
                    pRect++;

                    for( sal_uLong n = 1; n < pHeader.nCount; n++, pRect++ )
                    {
                        ScopedHRGN hRgn(CreateRectRgn(pRect->left, pRect->top, pRect->right, pRect->bottom));
                        CombineRgn( mrParent.mhRegion, mrParent.mhRegion, hRgn.get(), RGN_OR );
                    }
                }
            }

            if ( mrParent.mpClipRgnData != mrParent.mpStdClipRgnData )
                delete [] reinterpret_cast<BYTE*>(mrParent.mpClipRgnData);
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
        SelectClipRgn( mrParent.getHDC(), nullptr );
    }

    // #i123585# retval no longer dependent of mrParent.mhRegion, see TaskId comments above
    return true;
}

void WinSalGraphicsImpl::SetLineColor()
{
    ResetPen(GetStockPen(NULL_PEN));

    // set new data
    mbPen       = false;
    mbStockPen  = true;
}

void WinSalGraphicsImpl::SetLineColor(Color nColor)
{
    COLORREF nPenColor = PALETTERGB(nColor.GetRed(),
                                    nColor.GetGreen(),
                                    nColor.GetBlue());
    bool bStockPen = false;

    HPEN hNewPen = SearchStockPen(nPenColor);
    if (hNewPen)
        bStockPen = true;
    else
        hNewPen = MakePen(nColor);

    ResetPen(hNewPen);

    // set new data
    mnPenColor  = nPenColor;
    maLineColor = nColor;
    mbPen       = true;
    mbStockPen  = bStockPen;
}

HPEN WinSalGraphicsImpl::SearchStockPen(COLORREF nPenColor)
{
    // Only screen, because printer has problems, when we use stock objects.
    if (!mrParent.isPrinter())
    {
        const SalData* pSalData = GetSalData();

        for (sal_uInt16 i = 0; i < pSalData->mnStockPenCount; i++)
        {
            if (nPenColor == pSalData->maStockPenColorAry[i])
                return pSalData->mhStockPenAry[i];
        }
    }

    return nullptr;
}

HPEN WinSalGraphicsImpl::MakePen(Color nColor)
{
    COLORREF nPenColor = PALETTERGB(nColor.GetRed(),
                                    nColor.GetGreen(),
                                    nColor.GetBlue());

    if (!mrParent.isPrinter())
    {
        if (GetSalData()->mhDitherPal && ImplIsSysColorEntry(nColor))
        {
            nPenColor = PALRGB_TO_RGB(nPenColor);
        }
    }

    return CreatePen(PS_SOLID, mrParent.mnPenWidth, nPenColor);
}

void WinSalGraphicsImpl::ResetPen(HPEN hNewPen)
{
    HPEN hOldPen = SelectPen(mrParent.getHDC(), hNewPen);

    if (mhPen)
    {
        if (!mbStockPen)
        {
            DeletePen(mhPen);
        }
    }
    else
    {
        mrParent.mhDefPen = hOldPen;
    }

    mhPen = hNewPen;
}

void WinSalGraphicsImpl::SetFillColor()
{
    ResetBrush(GetStockBrush(NULL_BRUSH));

    // set new data
    mbBrush     = false;
    mbStockBrush = true;
}

void WinSalGraphicsImpl::SetFillColor(Color nColor)
{
    COLORREF nBrushColor = PALETTERGB(nColor.GetRed(),
                                      nColor.GetGreen(),
                                      nColor.GetBlue());
    bool bStockBrush = false;

    HBRUSH hNewBrush = SearchStockBrush(nBrushColor);
    if (hNewBrush)
        bStockBrush = true;
    else
        hNewBrush = MakeBrush(nColor);

    ResetBrush(hNewBrush);

    // set new data
    mnBrushColor = nBrushColor;
    maFillColor = nColor;
    mbBrush     = true;
    mbStockBrush = bStockBrush;
}

HBRUSH WinSalGraphicsImpl::SearchStockBrush(COLORREF nBrushColor)
{
    // Only screen, because printer has problems, when we use stock objects.
    if (!mrParent.isPrinter())
    {
        const SalData* pSalData = GetSalData();

        for (sal_uInt16 i = 0; i < pSalData->mnStockBrushCount; i++)
        {
            if (nBrushColor == pSalData->maStockBrushColorAry[i])
                return pSalData->mhStockBrushAry[i];
        }
    }

    return nullptr;
}

namespace
{

BYTE GetDitherMappingValue(BYTE nVal, BYTE nThres, const SalData* pSalData)
{
    return (pSalData->mpDitherDiff[nVal] > nThres) ?
        pSalData->mpDitherHigh[nVal] : pSalData->mpDitherLow[nVal];
}

HBRUSH Make16BitDIBPatternBrush(Color nColor)
{
    const SalData* pSalData = GetSalData();

    const BYTE nRed   = nColor.GetRed();
    const BYTE nGreen = nColor.GetGreen();
    const BYTE nBlue  = nColor.GetBlue();

    static const BYTE aOrdDither16Bit[8][8] =
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

    BYTE* pTmp = pSalData->mpDitherDIBData;

    for(int nY = 0; nY < 8; ++nY)
    {
        for(int nX = 0; nX < 8; ++nX)
        {
            const BYTE nThres = aOrdDither16Bit[nY][nX];
            *pTmp++ = GetDitherMappingValue(nBlue, nThres, pSalData);
            *pTmp++ = GetDitherMappingValue(nGreen, nThres, pSalData);
            *pTmp++ = GetDitherMappingValue(nRed, nThres, pSalData);
        }
    }

    return CreateDIBPatternBrush(pSalData->mhDitherDIB, DIB_RGB_COLORS);
}

HBRUSH Make8BitDIBPatternBrush(Color nColor)
{
    const SalData* pSalData = GetSalData();

    const BYTE nRed   = nColor.GetRed();
    const BYTE nGreen = nColor.GetGreen();
    const BYTE nBlue  = nColor.GetBlue();

    static const BYTE aOrdDither8Bit[8][8] =
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

    BYTE* pTmp = pSalData->mpDitherDIBData;

    for (int nY = 0; nY < 8; ++nY)
    {
        for (int nX = 0; nX < 8; ++nX)
        {
            const BYTE nThres = aOrdDither8Bit[nY][nX];
            *pTmp = GetDitherMappingValue(nRed, nThres, pSalData) +
                    GetDitherMappingValue(nGreen, nThres, pSalData) * 6 +
                    GetDitherMappingValue(nBlue, nThres, pSalData) * 36;
            pTmp++;
        }
    }

    return CreateDIBPatternBrush(pSalData->mhDitherDIB, DIB_PAL_COLORS);
}

} // namespace

HBRUSH WinSalGraphicsImpl::MakeBrush(Color nColor)
{
    const SalData* pSalData = GetSalData();

    const BYTE        nRed        = nColor.GetRed();
    const BYTE        nGreen      = nColor.GetGreen();
    const BYTE        nBlue       = nColor.GetBlue();
    const COLORREF    nBrushColor = PALETTERGB(nRed, nGreen, nBlue);

    if (mrParent.isPrinter() || !pSalData->mhDitherDIB)
        return CreateSolidBrush(nBrushColor);

    if (24 == reinterpret_cast<BITMAPINFOHEADER*>(pSalData->mpDitherDIB)->biBitCount)
        return Make16BitDIBPatternBrush(nColor);

    if (ImplIsSysColorEntry(nColor))
        return CreateSolidBrush(PALRGB_TO_RGB(nBrushColor));

    if (ImplIsPaletteEntry(nRed, nGreen, nBlue))
        return CreateSolidBrush(nBrushColor);

    return Make8BitDIBPatternBrush(nColor);
}

void WinSalGraphicsImpl::ResetBrush(HBRUSH hNewBrush)
{
    HBRUSH hOldBrush = SelectBrush(mrParent.getHDC(), hNewBrush);

    if (mhBrush)
    {
        if (!mbStockBrush)
        {
            DeleteBrush(mhBrush);
        }
    }
    else
    {
        mrParent.mhDefBrush = hOldBrush;
    }

    mhBrush = hNewBrush;
}

void WinSalGraphicsImpl::SetXORMode( bool bSet, bool )
{
    mbXORMode = bSet;
    ::SetROP2( mrParent.getHDC(), bSet ? R2_XORPEN : R2_COPYPEN );
}

void WinSalGraphicsImpl::SetROPLineColor( SalROPColor nROPColor )
{
    SetLineColor( ImplGetROPColor( nROPColor ) );
}

void WinSalGraphicsImpl::SetROPFillColor( SalROPColor nROPColor )
{
    SetFillColor( ImplGetROPColor( nROPColor ) );
}

void WinSalGraphicsImpl::DrawPixelImpl( tools::Long nX, tools::Long nY, COLORREF crColor )
{
    const HDC hDC = mrParent.getHDC();

    if (!mbXORMode)
    {
        SetPixel(hDC, static_cast<int>(nX), static_cast<int>(nY), crColor);
        return;
    }

    ScopedSelectedHBRUSH hBrush(hDC, CreateSolidBrush(crColor));
    PatBlt(hDC, static_cast<int>(nX), static_cast<int>(nY), int(1), int(1), PATINVERT);
}

void WinSalGraphicsImpl::drawPixel( tools::Long nX, tools::Long nY )
{
    DrawPixelImpl( nX, nY, mnPenColor );
}

void WinSalGraphicsImpl::drawPixel( tools::Long nX, tools::Long nY, Color nColor )
{
    COLORREF nCol = PALETTERGB( nColor.GetRed(),
                                nColor.GetGreen(),
                                nColor.GetBlue() );

    if ( !mrParent.isPrinter() &&
         GetSalData()->mhDitherPal &&
         ImplIsSysColorEntry( nColor ) )
        nCol = PALRGB_TO_RGB( nCol );

    DrawPixelImpl( nX, nY, nCol );
}

void WinSalGraphicsImpl::drawLine( tools::Long nX1, tools::Long nY1, tools::Long nX2, tools::Long nY2 )
{
    MoveToEx( mrParent.getHDC(), static_cast<int>(nX1), static_cast<int>(nY1), nullptr );

    LineTo( mrParent.getHDC(), static_cast<int>(nX2), static_cast<int>(nY2) );

    // LineTo doesn't draw the last pixel
    if ( !mrParent.isPrinter() )
        DrawPixelImpl( nX2, nY2, mnPenColor );
}

void WinSalGraphicsImpl::drawRect( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight )
{
    if ( !mbPen )
    {
        if ( !mrParent.isPrinter() )
        {
            PatBlt( mrParent.getHDC(), static_cast<int>(nX), static_cast<int>(nY), static_cast<int>(nWidth), static_cast<int>(nHeight),
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
        Rectangle( mrParent.getHDC(), static_cast<int>(nX), static_cast<int>(nY), static_cast<int>(nX+nWidth), static_cast<int>(nY+nHeight) );
}

void WinSalGraphicsImpl::drawPolyLine( sal_uInt32 nPoints, const Point* pPtAry )
{
    std::unique_ptr<POINT[]> pWinPtAry(new POINT[nPoints]);
    for (sal_uInt32 i=0; i<nPoints; ++i)
        pWinPtAry[i] = POINT { static_cast<LONG>(pPtAry[i].getX()), static_cast<LONG>(pPtAry[i].getY()) };

    // for Windows 95 and its maximum number of points
    if ( !Polyline( mrParent.getHDC(), pWinPtAry.get(), static_cast<int>(nPoints) ) && (nPoints > MAX_64KSALPOINTS) )
        Polyline( mrParent.getHDC(), pWinPtAry.get(), MAX_64KSALPOINTS );

    // Polyline seems to uses LineTo, which doesn't paint the last pixel (see 87eb8f8ee)
    if ( !mrParent.isPrinter() )
        DrawPixelImpl( pWinPtAry[nPoints-1].x, pWinPtAry[nPoints-1].y, mnPenColor );
}

void WinSalGraphicsImpl::drawPolygon( sal_uInt32 nPoints, const Point* pPtAry )
{
    std::unique_ptr<POINT[]> pWinPtAry(new POINT[nPoints]);
    for (sal_uInt32 i=0; i<nPoints; ++i)
        pWinPtAry[i] = POINT { static_cast<LONG>(pPtAry[i].getX()), static_cast<LONG>(pPtAry[i].getY()) };

    // for Windows 95 and its maximum number of points
    if ( !Polygon( mrParent.getHDC(), pWinPtAry.get(), static_cast<int>(nPoints) ) && (nPoints > MAX_64KSALPOINTS) )
        Polygon( mrParent.getHDC(), pWinPtAry.get(), MAX_64KSALPOINTS );
}

void WinSalGraphicsImpl::drawPolyPolygon( sal_uInt32 nPoly, const sal_uInt32* pPoints,
                                   const Point** pPtAry )
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

    for ( i = 0; i < static_cast<UINT>(nPoly); i++ )
    {
        nPoints = static_cast<UINT>(pPoints[i])+1;
        pWinPointAry[i] = nPoints;
        nPolyPolyPoints += nPoints;
    }

    POINT  aWinPointAryAry[SAL_POLYPOLYPOINTS_STACKBUF];
    POINT* pWinPointAryAry;
    if ( nPolyPolyPoints <= SAL_POLYPOLYPOINTS_STACKBUF )
        pWinPointAryAry = aWinPointAryAry;
    else
        pWinPointAryAry = new POINT[nPolyPolyPoints];
    UINT            n = 0;
    for ( i = 0; i < static_cast<UINT>(nPoly); i++ )
    {
        nPoints = pWinPointAry[i];
        const Point* pPolyAry = pPtAry[i];
        for (sal_uInt32 j=0; j<nPoints-1; ++j)
            pWinPointAryAry[n+j] = POINT { static_cast<LONG>(pPolyAry[j].getX()), static_cast<LONG>(pPolyAry[j].getY()) };
        pWinPointAryAry[n+nPoints-1] = pWinPointAryAry[n];
        n += nPoints;
    }

    if ( !PolyPolygon( mrParent.getHDC(), pWinPointAryAry, reinterpret_cast<int*>(pWinPointAry), static_cast<UINT>(nPoly) ) &&
         (nPolyPolyPoints > MAX_64KSALPOINTS) )
    {
        nPolyPolyPoints  = 0;
        nPoly = 0;
        do
        {
            nPolyPolyPoints += pWinPointAry[static_cast<UINT>(nPoly)];
            nPoly++;
        }
        while ( nPolyPolyPoints < MAX_64KSALPOINTS );
        nPoly--;
        if ( pWinPointAry[static_cast<UINT>(nPoly)] > MAX_64KSALPOINTS )
            pWinPointAry[static_cast<UINT>(nPoly)] = MAX_64KSALPOINTS;
        if ( nPoly == 1 )
            Polygon( mrParent.getHDC(), pWinPointAryAry, *pWinPointAry );
        else
            PolyPolygon( mrParent.getHDC(), pWinPointAryAry, reinterpret_cast<int*>(pWinPointAry), nPoly );
    }

    if ( pWinPointAry != aWinPointAry )
        delete [] pWinPointAry;
    if ( pWinPointAryAry != aWinPointAryAry )
        delete [] pWinPointAryAry;
}

bool WinSalGraphicsImpl::drawPolyLineBezier( sal_uInt32 nPoints, const Point* pPtAry, const PolyFlags* pFlgAry )
{
    // #100127# draw an array of points which might also contain bezier control points
    if (!nPoints)
        return true;

    const HDC hdc = mrParent.getHDC();

    // TODO: profile whether the following options are faster:
    // a) look ahead and draw consecutive bezier or line segments by PolyBezierTo/PolyLineTo resp.
    // b) convert our flag array to window's and use PolyDraw
    MoveToEx(hdc, static_cast<LONG>(pPtAry->getX()), static_cast<LONG>(pPtAry->getY()), nullptr);
    ++pPtAry;
    ++pFlgAry;

    for(sal_uInt32 i = 1; i < nPoints; ++i)
    {
        if(*pFlgAry != PolyFlags::Control)
        {
            LineTo(hdc, pPtAry->getX(), pPtAry->getY());
        }
        else if(nPoints - i > 2)
        {
            POINT bezierPoints[] = {
                POINT { static_cast<LONG>(pPtAry[0].getX()), static_cast<LONG>(pPtAry[0].getY()) },
                POINT { static_cast<LONG>(pPtAry[1].getX()), static_cast<LONG>(pPtAry[1].getY()) },
                POINT { static_cast<LONG>(pPtAry[2].getX()), static_cast<LONG>(pPtAry[2].getY()) },
            };
            PolyBezierTo(hdc, bezierPoints, 3);
            i += 2;
            pPtAry += 2;
            pFlgAry += 2;
        }

        ++pPtAry;
        ++pFlgAry;
    }

    return true;
}

bool WinSalGraphicsImpl::drawPolygonBezier( sal_uInt32 nPoints, const Point* pPtAry, const PolyFlags* pFlgAry )
{
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
}

bool WinSalGraphicsImpl::drawPolyPolygonBezier( sal_uInt32 nPoly, const sal_uInt32* pPoints,
                                             const Point* const* pPtAry, const PolyFlags* const* pFlgAry )
{
    sal_uLong nCurrPoly, nTotalPoints;
    const sal_uInt32* pCurrPoints = pPoints;
    for( nCurrPoly=0, nTotalPoints=0; nCurrPoly<nPoly; ++nCurrPoly )
        nTotalPoints += *pCurrPoints++;

    POINT      aStackAry1[SAL_POLY_STACKBUF];
    BYTE       aStackAry2[SAL_POLY_STACKBUF];
    POINT*     pWinPointAry;
    BYTE*      pWinFlagAry;
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
}

static basegfx::B2DPoint impPixelSnap(
    const basegfx::B2DPolygon& rPolygon,
    const basegfx::B2DHomMatrix& rObjectToDevice,
    basegfx::B2DHomMatrix& rObjectToDeviceInv,
    sal_uInt32 nIndex)
{
    const sal_uInt32 nCount(rPolygon.count());

    // get the data
    const basegfx::B2ITuple aPrevTuple(basegfx::fround(rObjectToDevice * rPolygon.getB2DPoint((nIndex + nCount - 1) % nCount)));
    const basegfx::B2DPoint aCurrPoint(rObjectToDevice * rPolygon.getB2DPoint(nIndex));
    const basegfx::B2ITuple aCurrTuple(basegfx::fround(aCurrPoint));
    const basegfx::B2ITuple aNextTuple(basegfx::fround(rObjectToDevice * rPolygon.getB2DPoint((nIndex + 1) % nCount)));

    // get the states
    const bool bPrevVertical(aPrevTuple.getX() == aCurrTuple.getX());
    const bool bNextVertical(aNextTuple.getX() == aCurrTuple.getX());
    const bool bPrevHorizontal(aPrevTuple.getY() == aCurrTuple.getY());
    const bool bNextHorizontal(aNextTuple.getY() == aCurrTuple.getY());
    const bool bSnapX(bPrevVertical || bNextVertical);
    const bool bSnapY(bPrevHorizontal || bNextHorizontal);

    if(bSnapX || bSnapY)
    {
        basegfx::B2DPoint aSnappedPoint(
            bSnapX ? aCurrTuple.getX() : aCurrPoint.getX(),
            bSnapY ? aCurrTuple.getY() : aCurrPoint.getY());

        if(rObjectToDeviceInv.isIdentity())
        {
            rObjectToDeviceInv = rObjectToDevice;
            rObjectToDeviceInv.invert();
        }

        aSnappedPoint *= rObjectToDeviceInv;

        return aSnappedPoint;
    }

    return rPolygon.getB2DPoint(nIndex);
}

static void impAddB2DPolygonToGDIPlusGraphicsPathReal(
    Gdiplus::GraphicsPath& rGraphicsPath,
    const basegfx::B2DPolygon& rPolygon,
    const basegfx::B2DHomMatrix& rObjectToDevice,
    bool bNoLineJoin,
    bool bPixelSnapHairline)
{
    sal_uInt32 nCount(rPolygon.count());

    if(nCount)
    {
        const sal_uInt32 nEdgeCount(rPolygon.isClosed() ? nCount : nCount - 1);

        if(nEdgeCount)
        {
            const bool bControls(rPolygon.areControlPointsUsed());
            basegfx::B2DPoint aCurr(rPolygon.getB2DPoint(0));
            basegfx::B2DHomMatrix aObjectToDeviceInv;

            if(bPixelSnapHairline)
            {
                aCurr = impPixelSnap(rPolygon, rObjectToDevice, aObjectToDeviceInv, 0);
            }

            for(sal_uInt32 a(0); a < nEdgeCount; a++)
            {
                const sal_uInt32 nNextIndex((a + 1) % nCount);
                basegfx::B2DPoint aNext(rPolygon.getB2DPoint(nNextIndex));
                const bool b1stControlPointUsed(bControls && rPolygon.isNextControlPointUsed(a));
                const bool b2ndControlPointUsed(bControls && rPolygon.isPrevControlPointUsed(nNextIndex));

                if(bPixelSnapHairline)
                {
                    aNext = impPixelSnap(rPolygon, rObjectToDevice, aObjectToDeviceInv, nNextIndex);
                }

                if(b1stControlPointUsed || b2ndControlPointUsed)
                {
                    basegfx::B2DPoint aCa(rPolygon.getNextControlPoint(a));
                    basegfx::B2DPoint aCb(rPolygon.getPrevControlPoint(nNextIndex));

                    // tdf#99165 MS Gdiplus cannot handle creating correct extra geometry for fat lines
                    // with LineCap or LineJoin when a bezier segment starts or ends trivial, e.g. has
                    // no 1st or 2nd control point, despite that these are mathematically correct definitions
                    // (basegfx can handle that).
                    // Caution: This error (and it's correction) might be necessary for other graphical
                    // sub-systems in a similar way.
                    // tdf#101026 The 1st attempt to create a mathematically correct replacement control
                    // vector was wrong. Best alternative is one as close as possible which means short.
                    if(!b1stControlPointUsed)
                    {
                        aCa = aCurr + ((aCb - aCurr) * 0.0005);
                    }
                    else if(!b2ndControlPointUsed)
                    {
                        aCb = aNext + ((aCa - aNext) * 0.0005);
                    }

                    rGraphicsPath.AddBezier(
                        static_cast< Gdiplus::REAL >(aCurr.getX()), static_cast< Gdiplus::REAL >(aCurr.getY()),
                        static_cast< Gdiplus::REAL >(aCa.getX()), static_cast< Gdiplus::REAL >(aCa.getY()),
                        static_cast< Gdiplus::REAL >(aCb.getX()), static_cast< Gdiplus::REAL >(aCb.getY()),
                        static_cast< Gdiplus::REAL >(aNext.getX()), static_cast< Gdiplus::REAL >(aNext.getY()));
                }
                else
                {
                    rGraphicsPath.AddLine(
                        static_cast< Gdiplus::REAL >(aCurr.getX()), static_cast< Gdiplus::REAL >(aCurr.getY()),
                        static_cast< Gdiplus::REAL >(aNext.getX()), static_cast< Gdiplus::REAL >(aNext.getY()));
                }

                if(a + 1 < nEdgeCount)
                {
                    aCurr = aNext;

                    if(bNoLineJoin)
                    {
                        rGraphicsPath.StartFigure();
                    }
                }
            }
        }
    }
}

namespace {

class SystemDependentData_GraphicsPath : public basegfx::SystemDependentData
{
private:
    // the path data itself
    std::shared_ptr<Gdiplus::GraphicsPath>  mpGraphicsPath;

    // all other values the triangulation is based on and
    // need to be compared with to check for data validity
    bool                                    mbNoLineJoin;
    std::vector< double >                       maStroke;

public:
    SystemDependentData_GraphicsPath(
        basegfx::SystemDependentDataManager& rSystemDependentDataManager,
        std::shared_ptr<Gdiplus::GraphicsPath>& rpGraphicsPath,
        bool bNoLineJoin,
        const std::vector< double >* pStroke); // MM01

    // read access
    std::shared_ptr<Gdiplus::GraphicsPath>& getGraphicsPath() { return mpGraphicsPath; }
    bool getNoLineJoin() const { return mbNoLineJoin; }
    const std::vector< double >& getStroke() const { return maStroke; }

    virtual sal_Int64 estimateUsageInBytes() const override;
};

}

SystemDependentData_GraphicsPath::SystemDependentData_GraphicsPath(
    basegfx::SystemDependentDataManager& rSystemDependentDataManager,
    std::shared_ptr<Gdiplus::GraphicsPath>& rpGraphicsPath,
    bool bNoLineJoin,
    const std::vector< double >* pStroke)
:   basegfx::SystemDependentData(rSystemDependentDataManager),
    mpGraphicsPath(rpGraphicsPath),
    mbNoLineJoin(bNoLineJoin),
    maStroke()
{
    if(nullptr != pStroke)
    {
        maStroke = *pStroke;
    }
}

sal_Int64 SystemDependentData_GraphicsPath::estimateUsageInBytes() const
{
    sal_Int64 nRetval(0);

    if(mpGraphicsPath)
    {
        const INT nPointCount(mpGraphicsPath->GetPointCount());

        if(0 != nPointCount)
        {
            // Each point has
            // - 2 x sizeof(Gdiplus::REAL)
            // - 1 byte (see GetPathTypes in docu)
            nRetval = nPointCount * ((2 * sizeof(Gdiplus::REAL)) + 1);
        }
    }

    return nRetval;
}

bool WinSalGraphicsImpl::drawPolyPolygon(
    const basegfx::B2DHomMatrix& rObjectToDevice,
    const basegfx::B2DPolyPolygon& rPolyPolygon,
    double fTransparency)
{
    const sal_uInt32 nCount(rPolyPolygon.count());

    if(!mbBrush || 0 == nCount || fTransparency < 0.0 || fTransparency > 1.0)
    {
        return true;
    }

    Gdiplus::Graphics aGraphics(mrParent.getHDC());
    const sal_uInt8 aTrans(sal_uInt8(255) - static_cast<sal_uInt8>(basegfx::fround(fTransparency * 255.0)));
    const Gdiplus::Color aTestColor(aTrans, maFillColor.GetRed(), maFillColor.GetGreen(), maFillColor.GetBlue());
    const Gdiplus::SolidBrush aSolidBrush(aTestColor.GetValue());

    // Set full (Object-to-Device) transformation - if used
    if(rObjectToDevice.isIdentity())
    {
        aGraphics.ResetTransform();
    }
    else
    {
        Gdiplus::Matrix aMatrix;

        aMatrix.SetElements(
            rObjectToDevice.get(0, 0),
            rObjectToDevice.get(1, 0),
            rObjectToDevice.get(0, 1),
            rObjectToDevice.get(1, 1),
            rObjectToDevice.get(0, 2),
            rObjectToDevice.get(1, 2));
        aGraphics.SetTransform(&aMatrix);
    }

    // prepare local instance of Gdiplus::GraphicsPath
    std::shared_ptr<Gdiplus::GraphicsPath> pGraphicsPath;

    // try to access buffered data
    std::shared_ptr<SystemDependentData_GraphicsPath> pSystemDependentData_GraphicsPath(
        rPolyPolygon.getSystemDependentData<SystemDependentData_GraphicsPath>());

    if(pSystemDependentData_GraphicsPath)
    {
        // copy buffered data
        pGraphicsPath = pSystemDependentData_GraphicsPath->getGraphicsPath();
    }
    else
    {
        // Note: In principle we could use the same buffered geometry at line
        // and fill polygons. Checked that in a first try, used
        // GraphicsPath::AddPath from Gdiplus combined with below used
        // StartFigure/CloseFigure, worked well (thus the line-draw version
        // may create non-closed partial Polygon data).
        //
        // But in current reality it gets not used due to e.g.
        // SdrPathPrimitive2D::create2DDecomposition creating transformed
        // line and fill polygon-primitives (what could be changed).
        //
        // There will probably be more hindrances here in other rendering paths
        // which could all be found - intention to do this would be: Use more
        // transformations, less modifications of B2DPolygons/B2DPolyPolygons.
        //
        // A fix for SdrPathPrimitive2D would be to create the sub-geometry
        // and embed into a TransformPrimitive2D containing the transformation.
        //
        // A 2nd problem is that the NoLineJoin mode (basegfx::B2DLineJoin::NONE
        // && !bIsHairline) creates polygon fill infos that are not reusable
        // for the fill case (see ::drawPolyLine below) - thus we would need a
        // bool and/or two system-dependent paths buffered - doable, but complicated.
        //
        // All in all: Make B2DPolyPolygon a SystemDependentDataProvider and buffer
        // the whole to-be-filled PolyPolygon independent from evtl. line-polygon
        // (at least for now...)

        // create data
        pGraphicsPath = std::make_shared<Gdiplus::GraphicsPath>();

        for(sal_uInt32 a(0); a < nCount; a++)
        {
            if(0 != a)
            {
                // #i101491# not needed for first run
                pGraphicsPath->StartFigure();
            }

            impAddB2DPolygonToGDIPlusGraphicsPathReal(
                *pGraphicsPath,
                rPolyPolygon.getB2DPolygon(a),
                rObjectToDevice, // not used due to the two 'false' values below, but to not forget later
                false,
                false);

            pGraphicsPath->CloseFigure();
        }

        // add to buffering mechanism
        rPolyPolygon.addOrReplaceSystemDependentData<SystemDependentData_GraphicsPath>(
            ImplGetSystemDependentDataManager(),
            pGraphicsPath,
            false,
            nullptr);
    }

    if(mrParent.getAntiAlias())
    {
        aGraphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    }
    else
    {
        aGraphics.SetSmoothingMode(Gdiplus::SmoothingModeNone);
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
        const Gdiplus::REAL aDpiX(aGraphics.GetDpiX());
        const Gdiplus::REAL aDpiY(aGraphics.GetDpiY());

        // Now the transformation maybe/is already used (see above), so do
        // modify it without resetting to not destroy it.
        // I double-checked with MS docu that Gdiplus::MatrixOrderAppend does what
        // we need - in our notation, would be a multiply from left to execute
        // current transform first and this scale last.
        // I tried to trigger this code using Print from the menu and various
        // targets, but got no hit, thus maybe obsolete anyways. If someone knows
        // more, feel free to remove it.
        // One more hint: This *may* also be needed now in ::drawPolyLine below
        // since it also uses transformations now.
        //
        // aGraphics.ResetTransform();

        aGraphics.ScaleTransform(
            Gdiplus::REAL(100.0) / aDpiX,
            Gdiplus::REAL(100.0) / aDpiY,
            Gdiplus::MatrixOrderAppend);
    }

    // use created or buffered data
    aGraphics.FillPath(
        &aSolidBrush,
        &(*pGraphicsPath));

    return true;
}

bool WinSalGraphicsImpl::drawPolyLine(
    const basegfx::B2DHomMatrix& rObjectToDevice,
    const basegfx::B2DPolygon& rPolygon,
    double fTransparency,
    double fLineWidth,
    const std::vector< double >* pStroke, // MM01
    basegfx::B2DLineJoin eLineJoin,
    css::drawing::LineCap eLineCap,
    double fMiterMinimumAngle,
    bool bPixelSnapHairline)
{
    // MM01 check done for simple reasons
    if(!mbPen || !rPolygon.count() || fTransparency < 0.0 || fTransparency > 1.0)
    {
        return true;
    }

    // need to check/handle LineWidth when ObjectToDevice transformation is used
    const bool bObjectToDeviceIsIdentity(rObjectToDevice.isIdentity());
    const bool bIsHairline(fLineWidth == 0);

    // tdf#124848 calculate-back logical LineWidth for a hairline
    // since this implementation hands over the transformation to
    // the graphic sub-system
    if(bIsHairline)
    {
        fLineWidth = 1.0;

        if(!bObjectToDeviceIsIdentity)
        {
            basegfx::B2DHomMatrix aObjectToDeviceInv(rObjectToDevice);
            aObjectToDeviceInv.invert();
            fLineWidth = (aObjectToDeviceInv * basegfx::B2DVector(fLineWidth, 0)).getLength();
        }
    }

    Gdiplus::Graphics aGraphics(mrParent.getHDC());
    const sal_uInt8 aTrans = static_cast<sal_uInt8>(basegfx::fround( 255 * (1.0 - fTransparency) ));
    const Gdiplus::Color aTestColor(aTrans, maLineColor.GetRed(), maLineColor.GetGreen(), maLineColor.GetBlue());
    Gdiplus::Pen aPen(aTestColor.GetValue(), Gdiplus::REAL(fLineWidth));
    bool bNoLineJoin(false);

    // Set full (Object-to-Device) transformation - if used
    if(bObjectToDeviceIsIdentity)
    {
        aGraphics.ResetTransform();
    }
    else
    {
        Gdiplus::Matrix aMatrix;

        aMatrix.SetElements(
            rObjectToDevice.get(0, 0),
            rObjectToDevice.get(1, 0),
            rObjectToDevice.get(0, 1),
            rObjectToDevice.get(1, 1),
            rObjectToDevice.get(0, 2),
            rObjectToDevice.get(1, 2));
        aGraphics.SetTransform(&aMatrix);
    }

    switch(eLineJoin)
    {
        case basegfx::B2DLineJoin::NONE :
        {
            if(!bIsHairline)
            {
                bNoLineJoin = true;
            }
            break;
        }
        case basegfx::B2DLineJoin::Bevel :
        {
            aPen.SetLineJoin(Gdiplus::LineJoinBevel);
            break;
        }
        case basegfx::B2DLineJoin::Miter :
        {
            const Gdiplus::REAL aMiterLimit(1.0/sin(fMiterMinimumAngle/2.0));

            aPen.SetMiterLimit(aMiterLimit);
            // tdf#99165 MS's LineJoinMiter creates non standard conform miter additional
            // graphics, somewhere clipped in some distance from the edge point, dependent
            // of MiterLimit. The more default-like option is LineJoinMiterClipped, so use
            // that instead
            aPen.SetLineJoin(Gdiplus::LineJoinMiterClipped);
            break;
        }
        case basegfx::B2DLineJoin::Round :
        {
            aPen.SetLineJoin(Gdiplus::LineJoinRound);
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
            aPen.SetStartCap(Gdiplus::LineCapRound);
            aPen.SetEndCap(Gdiplus::LineCapRound);
            break;
        }
        case css::drawing::LineCap_SQUARE:
        {
            aPen.SetStartCap(Gdiplus::LineCapSquare);
            aPen.SetEndCap(Gdiplus::LineCapSquare);
            break;
        }
    }

    // prepare local instance of Gdiplus::GraphicsPath
    std::shared_ptr<Gdiplus::GraphicsPath> pGraphicsPath;

    // try to access buffered data
    std::shared_ptr<SystemDependentData_GraphicsPath> pSystemDependentData_GraphicsPath(
        rPolygon.getSystemDependentData<SystemDependentData_GraphicsPath>());

    // MM01 need to do line dashing as fallback stuff here now
    const double fDotDashLength(nullptr != pStroke ? std::accumulate(pStroke->begin(), pStroke->end(), 0.0) : 0.0);
    const bool bStrokeUsed(0.0 != fDotDashLength);
    assert(!bStrokeUsed || (bStrokeUsed && pStroke));

    // MM01 decide if to stroke directly
    static bool bDoDirectGDIPlusStroke(true);

    // activate to stroke directly
    if(bDoDirectGDIPlusStroke && bStrokeUsed)
    {
        // tdf#124848 the fix of tdf#130478 that was needed here before
        // gets much easier when already handling the hairline case above,
        // the back-calculated logical linewidth is already here, just use it.
        // Still be careful - a zero LineWidth *should* not happen, but...
        std::vector<Gdiplus::REAL> aDashArray(pStroke->size());
        const double fFactor(fLineWidth == 0 ? 1.0 : 1.0 / fLineWidth);

        // tdf#134128. ODF adds caps to the dashes and dots, but GDI makes caps from the
        // dash or dot themselves. We tweak aDashArray to look the same in GDI (e.g. Impress edit mode)
        // and other renders (e.g. Impress slide show), while keeping the total length of the
        // pattern.
        // Patterns are always a sequence dash space dash space ...
        if (eLineCap != css::drawing::LineCap_BUTT)
        {
            size_t nSize = pStroke->size();
            // We want to treat dash and space in pairs. There should be no odd size. If so, we ignore
            // last item.
            nSize /= 2;
            for(size_t a(0); a < nSize; a++)
            {
                double fDashLengthRel = (*pStroke)[2 * a] * fFactor;
                double fSpaceLengthRel = (*pStroke)[2 * a + 1] * fFactor;
                // GDI allows only positive lengths for space, Skia negative lengths too. Thus the
                // appearance is different, in case space is too small.
                double fCorrect = fSpaceLengthRel - 1.0 <= 0 ? fSpaceLengthRel - 0.01 : 1.0;
                aDashArray[2 * a] = Gdiplus::REAL(fDashLengthRel + fCorrect);
                aDashArray[2 * a + 1] = Gdiplus::REAL(fSpaceLengthRel - fCorrect);
            }
        }
        else
        {
            for(size_t a(0); a < pStroke->size(); a++)
            {
                aDashArray[a] = Gdiplus::REAL((*pStroke)[a] * fFactor);
            }
        }
        if (eLineCap == css::drawing::LineCap_ROUND)
            aPen.SetDashCap(Gdiplus::DashCapRound);
        else
            aPen.SetDashCap(Gdiplus::DashCapFlat); // "square" doesn't exist in Gdiplus
        aPen.SetDashOffset(Gdiplus::REAL(0.0));
        aPen.SetDashPattern(aDashArray.data(), aDashArray.size());
    }

    if(!bDoDirectGDIPlusStroke && pSystemDependentData_GraphicsPath)
    {
        // MM01 - check on stroke change. Used against not used, or if oth used,
        // equal or different? Triangulation geometry creation depends heavily
        // on stroke, independent of being transformation independent
        const bool bStrokeWasUsed(!pSystemDependentData_GraphicsPath->getStroke().empty());

        if(bStrokeWasUsed != bStrokeUsed
        || (bStrokeUsed && *pStroke != pSystemDependentData_GraphicsPath->getStroke()))
        {
            // data invalid, forget
            pSystemDependentData_GraphicsPath.reset();
        }
    }

    if(pSystemDependentData_GraphicsPath)
    {
        // check data validity
        if (pSystemDependentData_GraphicsPath->getNoLineJoin() != bNoLineJoin
            || bPixelSnapHairline /*tdf#124700*/)
        {
            // data invalid, forget
            pSystemDependentData_GraphicsPath.reset();
        }
    }

    if(pSystemDependentData_GraphicsPath)
    {
        // copy buffered data
        pGraphicsPath = pSystemDependentData_GraphicsPath->getGraphicsPath();
    }
    else
    {
        // fill data of buffered data
        pGraphicsPath = std::make_shared<Gdiplus::GraphicsPath>();

        if(!bDoDirectGDIPlusStroke && bStrokeUsed)
        {
            // MM01 need to do line dashing as fallback stuff here now
            basegfx::B2DPolyPolygon aPolyPolygonLine;

            // apply LineStyle
            basegfx::utils::applyLineDashing(
                rPolygon, // source
                *pStroke, // pattern
                &aPolyPolygonLine, // target for lines
                nullptr, // target for gaps
                fDotDashLength); // full length if available

            // MM01 checked/verified, ok
            for(sal_uInt32 a(0); a < aPolyPolygonLine.count(); a++)
            {
                const basegfx::B2DPolygon aPolyLine(aPolyPolygonLine.getB2DPolygon(a));
                pGraphicsPath->StartFigure();
                impAddB2DPolygonToGDIPlusGraphicsPathReal(
                    *pGraphicsPath,
                    aPolyLine,
                    rObjectToDevice,
                    bNoLineJoin,
                    bPixelSnapHairline);
            }
        }
        else
        {
            // no line dashing or direct stroke, just copy
            impAddB2DPolygonToGDIPlusGraphicsPathReal(
                *pGraphicsPath,
                rPolygon,
                rObjectToDevice,
                bNoLineJoin,
                bPixelSnapHairline);

            if(rPolygon.isClosed() && !bNoLineJoin)
            {
                // #i101491# needed to create the correct line joins
                pGraphicsPath->CloseFigure();
            }
        }

        // add to buffering mechanism
        if (!bPixelSnapHairline /*tdf#124700*/)
        {
            rPolygon.addOrReplaceSystemDependentData<SystemDependentData_GraphicsPath>(
                ImplGetSystemDependentDataManager(),
                pGraphicsPath,
                bNoLineJoin,
                pStroke);
        }
    }

    if(mrParent.getAntiAlias())
    {
        aGraphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    }
    else
    {
        aGraphics.SetSmoothingMode(Gdiplus::SmoothingModeNone);
    }

    if(mrParent.isPrinter())
    {
        // tdf#122384 As mentioned above in WinSalGraphicsImpl::drawPolyPolygon
        // (look for 'One more hint: This *may* also be needed now in'...).
        // See comments in same spot above *urgently* before doing changes here,
        // these comments are *still fully valid* at this place (!)
        const Gdiplus::REAL aDpiX(aGraphics.GetDpiX());
        const Gdiplus::REAL aDpiY(aGraphics.GetDpiY());

        aGraphics.ScaleTransform(
            Gdiplus::REAL(100.0) / aDpiX,
            Gdiplus::REAL(100.0) / aDpiY,
            Gdiplus::MatrixOrderAppend);
    }

    aGraphics.DrawPath(
        &aPen,
        &(*pGraphicsPath));

    return true;
}

static void paintToGdiPlus(
    Gdiplus::Graphics& rGraphics,
    const SalTwoRect& rTR,
    Gdiplus::Bitmap& rBitmap)
{
    // only parts of source are used
    Gdiplus::PointF aDestPoints[3];
    Gdiplus::ImageAttributes aAttributes;

    // define target region as parallelogram
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
        &aAttributes);
}

static void setInterpolationMode(
    Gdiplus::Graphics& rGraphics,
    tools::Long rSrcWidth,
    tools::Long rDestWidth,
    tools::Long rSrcHeight,
    tools::Long rDestHeight)
{
    const bool bSameWidth(rSrcWidth == rDestWidth);
    const bool bSameHeight(rSrcHeight == rDestHeight);

    if(bSameWidth && bSameHeight)
    {
        rGraphics.SetInterpolationMode(Gdiplus::InterpolationModeInvalid);
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

bool WinSalGraphicsImpl::TryDrawBitmapGDIPlus(const SalTwoRect& rTR, const SalBitmap& rSrcBitmap)
{
    if(rTR.mnSrcWidth && rTR.mnSrcHeight && rTR.mnDestWidth && rTR.mnDestHeight)
    {
        assert(dynamic_cast<const WinSalBitmap*>(&rSrcBitmap));

        const WinSalBitmap& rSalBitmap = static_cast< const WinSalBitmap& >(rSrcBitmap);
        std::shared_ptr< Gdiplus::Bitmap > aARGB(rSalBitmap.ImplGetGdiPlusBitmap());

        if(aARGB)
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
                *aARGB);

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
        std::shared_ptr< Gdiplus::Bitmap > aARGB(rSalBitmap.ImplGetGdiPlusBitmap(&rSalAlpha));

        if(aARGB)
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
                *aARGB);

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
    const SalBitmap* pAlphaBitmap,
    double fAlpha)
{
    assert(dynamic_cast<const WinSalBitmap*>(&rSourceBitmap));
    assert(!pAlphaBitmap || dynamic_cast<const WinSalBitmap*>(pAlphaBitmap));

    if( fAlpha != 1.0 )
        return false;

    const WinSalBitmap& rSalBitmap = static_cast< const WinSalBitmap& >(rSourceBitmap);
    const WinSalBitmap* pSalAlpha = static_cast< const WinSalBitmap* >(pAlphaBitmap);
    std::shared_ptr< Gdiplus::Bitmap > aARGB(rSalBitmap.ImplGetGdiPlusBitmap(pSalAlpha));

    if(aARGB)
    {
        const tools::Long nSrcWidth(aARGB->GetWidth());
        const tools::Long nSrcHeight(aARGB->GetHeight());

        if(nSrcWidth && nSrcHeight)
        {
            const tools::Long nDestWidth(basegfx::fround(basegfx::B2DVector(rX - rNull).getLength()));
            const tools::Long nDestHeight(basegfx::fround(basegfx::B2DVector(rY - rNull).getLength()));

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

                // this mode is only capable of drawing the whole bitmap to a parallelogram
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
                    &aAttributes);
            }
        }

        return true;
    }

    return false;
}

bool WinSalGraphicsImpl::hasFastDrawTransformedBitmap() const
{
    return false;
}

bool WinSalGraphicsImpl::drawGradient(const tools::PolyPolygon& /*rPolygon*/,
        const Gradient& /*rGradient*/)
{
    return false;
}

bool WinSalGraphicsImpl::implDrawGradient(basegfx::B2DPolyPolygon const & /*rPolyPolygon*/,
    SalGradient const & /*rGradient*/)
{
    return false;
}

bool WinSalGraphicsImpl::supportsOperation(OutDevSupportType eType) const
{
    static bool bAllowForTest(true);
    bool bRet = false;

    switch (eType)
    {
        case OutDevSupportType::TransparentRect:
            bRet = mrParent.mbVirDev || mrParent.mbWindow;
            break;
        case OutDevSupportType::B2DDraw:
            bRet = bAllowForTest;
            break;
        default:
            break;
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
