/*************************************************************************
 *
 *  $RCSfile: salgdi2.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:49 $
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
#include <stdlib.h>

#ifndef _SVWIN_H
#include <tools/svwin.h>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#define _SV_SALGDI2_CXX

#ifndef _SV_WINCOMP_HXX
#include <wincomp.hxx>
#endif
#ifndef _SV_SALBMP_HXX
#include <salbmp.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALIDS_HRC
#include <salids.hrc>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif

// =======================================================================

BOOL bFastTransparent = FALSE;

// =======================================================================

void SalGraphics::CopyBits( const SalTwoRect* pPosAry, SalGraphics* pSrcGraphics )
{
    HDC     hSrcDC;
    DWORD   nRop;

    if ( pSrcGraphics )
        hSrcDC = pSrcGraphics->maGraphicsData.mhDC;
    else
        hSrcDC = maGraphicsData.mhDC;

    if ( maGraphicsData.mbXORMode )
        nRop = SRCINVERT;
    else
        nRop = SRCCOPY;

    if ( (pPosAry->mnSrcWidth  == pPosAry->mnDestWidth) &&
         (pPosAry->mnSrcHeight == pPosAry->mnDestHeight) )
    {
        BitBlt( maGraphicsData.mhDC,
                (int)pPosAry->mnDestX, (int)pPosAry->mnDestY,
                (int)pPosAry->mnDestWidth, (int)pPosAry->mnDestHeight,
                hSrcDC,
                (int)pPosAry->mnSrcX, (int)pPosAry->mnSrcY,
                nRop );
    }
    else
    {
        int nOldStretchMode = SetStretchBltMode( maGraphicsData.mhDC, STRETCH_DELETESCANS );
        StretchBlt( maGraphicsData.mhDC,
                    (int)pPosAry->mnDestX, (int)pPosAry->mnDestY,
                    (int)pPosAry->mnDestWidth, (int)pPosAry->mnDestHeight,
                    hSrcDC,
                    (int)pPosAry->mnSrcX, (int)pPosAry->mnSrcY,
                    (int)pPosAry->mnSrcWidth, (int)pPosAry->mnSrcHeight,
                    nRop );
        SetStretchBltMode( maGraphicsData.mhDC, nOldStretchMode );
    }
}

// -----------------------------------------------------------------------

void ImplCalcOutSideRgn( const RECT& rSrcRect,
                         int nLeft, int nTop, int nRight, int nBottom,
                         HRGN& rhInvalidateRgn )
{
    HRGN hTempRgn;

    // Bereiche ausserhalb des sichtbaren Bereiches berechnen
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

// -----------------------------------------------------------------------

void SalGraphics::CopyArea( long nDestX, long nDestY,
                            long nSrcX, long nSrcY,
                            long nSrcWidth, long nSrcHeight,
                            USHORT nFlags )
{
    BitBlt( maGraphicsData.mhDC,
            (int)nDestX, (int)nDestY,
            (int)nSrcWidth, (int)nSrcHeight,
            maGraphicsData.mhDC,
            (int)nSrcX, (int)nSrcY,
            SRCCOPY );

    // Muessen die ueberlappenden Bereiche auch invalidiert werden?
    if ( (nFlags & SAL_COPYAREA_WINDOWINVALIDATE) && maGraphicsData.mbWindow )
    {
        // Overlap-Bereich berechnen und invalidieren
        RECT    aSrcRect;
        RECT    aClipRect;
        RECT    aTempRect;
        RECT    aTempRect2;
        HRGN    hInvalidateRgn;
        HRGN    hTempRgn;
        HWND    hWnd;
        int     nRgnType;

        aSrcRect.left   = (int)nSrcX;
        aSrcRect.top    = (int)nSrcY;
        aSrcRect.right  = aSrcRect.left+(int)nSrcWidth;
        aSrcRect.bottom = aSrcRect.top+(int)nSrcHeight;
        GetClientRect( maGraphicsData.mhWnd, &aClipRect );
        if ( IntersectRect( &aSrcRect, &aSrcRect, &aClipRect ) )
        {
            // Rechteck in Screen-Koordinaaten umrechnen
            POINT aPt;
            int nScreenDX = GetSystemMetrics( SM_CXSCREEN );
            int nScreenDY = GetSystemMetrics( SM_CYSCREEN );
            aPt.x = 0;
            aPt.y = 0;
            ClientToScreen( maGraphicsData.mhWnd, &aPt );
            aSrcRect.left   += aPt.x;
            aSrcRect.top    += aPt.y;
            aSrcRect.right  += aPt.x;
            aSrcRect.bottom += aPt.y;
            hInvalidateRgn = 0;
            // Bereiche ausserhalb des sichtbaren Bereiches berechnen
            ImplCalcOutSideRgn( aSrcRect, 0, 0, nScreenDX, nScreenDY, hInvalidateRgn );

            // Bereiche die von anderen Fenstern ueberlagert werden berechnen
            HRGN hTempRgn2 = 0;
            HWND hWndTopWindow = maGraphicsData.mhWnd;
            // Find the TopLevel Window, because only Windows which are in
            // in the foreground of our TopLevel window must be considered
            if ( GetWindowStyle( hWndTopWindow ) & WS_CHILD )
            {
                RECT aTempRect3 = aSrcRect;
                do
                {
                    hWndTopWindow = ::GetParent( hWndTopWindow );

                    // Test, if the Parent clip our window
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
                        if ( !hInvalidateRgn )
                            hInvalidateRgn = CreateRectRgnIndirect( &aSrcRect );
                        hTempRgn = CreateRectRgnIndirect( &aTempRect );
                        if ( !hTempRgn2 )
                            hTempRgn2 = CreateRectRgn( 0, 0, 0, 0 );
                        nRgnType = GetWindowRgn( hWnd, hTempRgn2 );
                        if ( (nRgnType != ERROR) && (nRgnType != NULLREGION) )
                        {
                            OffsetRgn( hTempRgn2, aTempRect.left, aTempRect.top );
                            CombineRgn( hTempRgn, hTempRgn, hTempRgn2, RGN_AND );
                        }
                        CombineRgn( hInvalidateRgn, hInvalidateRgn, hTempRgn, RGN_DIFF );
                        DeleteRegion( hTempRgn );
                    }
                }
                hWnd = GetWindow( hWnd, GW_HWNDNEXT );
            }
            if ( hTempRgn2 )
                DeleteRegion( hTempRgn2 );
            if ( hInvalidateRgn )
            {
                hTempRgn = CreateRectRgnIndirect( &aSrcRect );
                nRgnType = CombineRgn( hInvalidateRgn, hTempRgn, hInvalidateRgn, RGN_DIFF );
                if ( (nRgnType != ERROR) && (nRgnType != NULLREGION) )
                {
                    int nOffX = (int)(nDestX-nSrcX);
                    int nOffY = (int)(nDestY-nSrcY);
                    OffsetRgn( hInvalidateRgn, nOffX-aPt.x, nOffY-aPt.y );
                    // Combine Invalidate Region with existing ClipRegion
                    if ( GetClipRgn( maGraphicsData.mhDC, hTempRgn ) == 1 )
                        nRgnType = CombineRgn( hInvalidateRgn, hTempRgn, hInvalidateRgn, RGN_AND );
                    if ( (nRgnType != ERROR) && (nRgnType != NULLREGION) )
                    {
                        InvalidateRgn( maGraphicsData.mhWnd, hInvalidateRgn, TRUE );
                        // Hier loesen wir nur ein Update aus, wenn es der
                        // MainThread ist, damit es beim Bearbeiten der
                        // Paint-Message keinen Deadlock gibt, da der
                        // SolarMutex durch diesen Thread schon gelockt ist
                        SalData*    pSalData = GetSalData();
                        DWORD       nCurThreadId = GetCurrentThreadId();
                        if ( pSalData->mnAppThreadId == nCurThreadId )
                            UpdateWindow( maGraphicsData.mhWnd );
                    }
                }
                DeleteRegion( hTempRgn );
                DeleteRegion( hInvalidateRgn );
            }
        }
    }
}

// -----------------------------------------------------------------------

void ImplDrawBitmap( HDC hDC,
                     const SalTwoRect* pPosAry, const SalBitmap& rSalBitmap,
                     BOOL bPrinter, int nDrawMode )
{
    if( hDC )
    {
        HGLOBAL     hDrawDIB;
        HBITMAP     hDrawDDB = rSalBitmap.ImplGethDDB();
        SalBitmap*  pTmpSalBmp;
        BOOL        bPrintDDB = ( bPrinter && hDrawDDB );

        if( bPrintDDB )
        {
            pTmpSalBmp = new SalBitmap;
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

            int nCount = StretchDIBits( hDC,
                                        (int)pPosAry->mnDestX, (int)pPosAry->mnDestY,
                                        (int)pPosAry->mnDestWidth, (int)pPosAry->mnDestHeight,
                                        (int)pPosAry->mnSrcX, (int)(pBIH->biHeight - pPosAry->mnSrcHeight - pPosAry->mnSrcY),
                                        (int)pPosAry->mnSrcWidth, (int)pPosAry->mnSrcHeight,
                                        pBits, pBI, DIB_RGB_COLORS, nDrawMode );

            GlobalUnlock( hDrawDIB );
            SetStretchBltMode( hDC, nOldStretchMode );
        }
        else if( hDrawDDB && !bPrintDDB )
        {
            HDC         hBmpDC = ImplGetCachedDC( CACHED_HDC_DRAW, hDrawDDB );
            COLORREF    nOldBkColor;
            COLORREF    nOldTextColor;
            BOOL        bMono = ( rSalBitmap.GetBitCount() == 1 );

            if( bMono )
            {
                nOldBkColor = SetBkColor( hDC, RGB( 0xFF, 0xFF, 0xFF ) );
                nOldTextColor = ::SetTextColor( hDC, RGB( 0x00, 0x00, 0x00 ) );
            }

            if ( (pPosAry->mnSrcWidth  == pPosAry->mnDestWidth) &&
                 (pPosAry->mnSrcHeight == pPosAry->mnDestHeight) )
            {
                BitBlt( hDC,
                        (int)pPosAry->mnDestX, (int)pPosAry->mnDestY,
                        (int)pPosAry->mnDestWidth, (int)pPosAry->mnDestHeight,
                        hBmpDC,
                        (int)pPosAry->mnSrcX, (int)pPosAry->mnSrcY,
                        nDrawMode );
            }
            else
            {
                const int nOldStretchMode = SetStretchBltMode( hDC, STRETCH_DELETESCANS );

                StretchBlt( hDC,
                            (int)pPosAry->mnDestX, (int)pPosAry->mnDestY,
                            (int)pPosAry->mnDestWidth, (int)pPosAry->mnDestHeight,
                            hBmpDC,
                            (int)pPosAry->mnSrcX, (int)pPosAry->mnSrcY,
                            (int)pPosAry->mnSrcWidth, (int)pPosAry->mnSrcHeight,
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

// -----------------------------------------------------------------------

void SalGraphics::DrawBitmap( const SalTwoRect* pPosAry,
                              const SalBitmap& rSalBitmap )
{
    ImplDrawBitmap( maGraphicsData.mhDC, pPosAry, rSalBitmap,
                    maGraphicsData.mbPrinter,
                    maGraphicsData.mbXORMode ? SRCINVERT : SRCCOPY );
}

// -----------------------------------------------------------------------

void SalGraphics::DrawBitmap( const SalTwoRect* pPosAry,
                              const SalBitmap& rSalBitmap,
                              SalColor nTransparentColor )
{
    DBG_ASSERT( !maGraphicsData.mbPrinter, "No transparency print possible!" );

    SalBitmap*  pMask = new SalBitmap;
    HDC         hDC = maGraphicsData.mhDC;
    const Point aPoint;
    const Size  aSize( rSalBitmap.GetSize() );
    HBITMAP     hMaskBitmap = CreateBitmap( (int) aSize.Width(), (int) aSize.Height(), 1, 1, NULL );
    HDC         hMaskDC = ImplGetCachedDC( CACHED_HDC_1, hMaskBitmap );
    const BYTE  cRed = SALCOLOR_RED( nTransparentColor );
    const BYTE  cGreen = SALCOLOR_GREEN( nTransparentColor );
    const BYTE  cBlue = SALCOLOR_BLUE( nTransparentColor );

    if( rSalBitmap.ImplGethDDB() )
    {
        HDC         hSrcDC = ImplGetCachedDC( CACHED_HDC_2, rSalBitmap.ImplGethDDB() );
        COLORREF    aOldCol = SetBkColor( hSrcDC, RGB( cRed, cGreen, cBlue ) );

        BitBlt( hMaskDC, 0, 0, (int) aSize.Width(), (int) aSize.Height(), hSrcDC, 0, 0, SRCCOPY );

        SetBkColor( hSrcDC, aOldCol );
        ImplReleaseCachedDC( CACHED_HDC_2 );
    }
    else
    {
        SalBitmap*  pTmpSalBmp = new SalBitmap;

        if( pTmpSalBmp->Create( rSalBitmap, this ) )
        {
            HDC         hSrcDC = ImplGetCachedDC( CACHED_HDC_2, pTmpSalBmp->ImplGethDDB() );
            COLORREF    aOldCol = SetBkColor( hSrcDC, RGB( cRed, cGreen, cBlue ) );

            BitBlt( hMaskDC, 0, 0, (int) aSize.Width(), (int) aSize.Height(), hSrcDC, 0, 0, SRCCOPY );

            SetBkColor( hSrcDC, aOldCol );
            ImplReleaseCachedDC( CACHED_HDC_2 );
        }

        delete pTmpSalBmp;
    }

    ImplReleaseCachedDC( CACHED_HDC_1 );

    // hMaskBitmap is destroyed by new SalBitmap 'pMask' ( bDIB==FALSE, bCopy == FALSE )
    if( pMask->Create( hMaskBitmap, FALSE, FALSE ) )
        DrawBitmap( pPosAry, rSalBitmap, *pMask );

    delete pMask;
}

// -----------------------------------------------------------------------

void SalGraphics::DrawBitmap( const SalTwoRect* pPosAry,
                              const SalBitmap& rSalBitmap,
                              const SalBitmap& rTransparentBitmap )
{
    DBG_ASSERT( !maGraphicsData.mbPrinter, "No transparency print possible!" );

    if( bFastTransparent )
    {
        // bei Paletten-Displays hat WIN/WNT offenbar ein kleines Problem,
        // die Farben der Maske richtig auf die Palette abzubilden,
        // wenn wir die DIB direkt ausgeben => DDB-Ausgabe
        if( ( GetBitCount() <= 8 ) && rTransparentBitmap.ImplGethDIB() && rTransparentBitmap.GetBitCount() == 1 )
        {
            SalBitmap aTmp;
            if( aTmp.Create( rTransparentBitmap, this ) )
                ImplDrawBitmap( maGraphicsData.mhDC, pPosAry, aTmp, FALSE, SRCAND );
        }
        else
            ImplDrawBitmap( maGraphicsData.mhDC, pPosAry, rTransparentBitmap, FALSE, SRCAND );

        // bei Paletten-Displays hat WIN/WNT offenbar ein kleines Problem,
        // die Farben der Maske richtig auf die Palette abzubilden,
        // wenn wir die DIB direkt ausgeben => DDB-Ausgabe
        if( ( GetBitCount() <= 8 ) && rSalBitmap.ImplGethDIB() && rSalBitmap.GetBitCount() == 1 )
        {
            SalBitmap aTmp;
            if( aTmp.Create( rSalBitmap, this ) )
                ImplDrawBitmap( maGraphicsData.mhDC, pPosAry, aTmp, FALSE, SRCPAINT );
        }
        else
            ImplDrawBitmap( maGraphicsData.mhDC, pPosAry, rSalBitmap, FALSE, SRCPAINT );
    }
    else
    {
        SalTwoRect  aPosAry = *pPosAry;
        int         nDstX = (int)aPosAry.mnDestX;
        int         nDstY = (int)aPosAry.mnDestY;
        int         nDstWidth = (int)aPosAry.mnDestWidth;
        int         nDstHeight = (int)aPosAry.mnDestHeight;
        HDC         hDC = maGraphicsData.mhDC;
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

        // bei Paletten-Displays hat WIN/WNT offenbar ein kleines Problem,
        // die Farben der Maske richtig auf die Palette abzubilden,
        // wenn wir die DIB direkt ausgeben => DDB-Ausgabe
        if( ( GetBitCount() <= 8 ) && rTransparentBitmap.ImplGethDIB() && rTransparentBitmap.GetBitCount() == 1 )
        {
            SalBitmap aTmp;

            if( aTmp.Create( rTransparentBitmap, this ) )
                ImplDrawBitmap( hMaskDC, &aPosAry, aTmp, FALSE, SRCCOPY );
        }
        else
            ImplDrawBitmap( hMaskDC, &aPosAry, rTransparentBitmap, FALSE, SRCCOPY );

        BitBlt( hMemDC, 0, 0, nDstWidth, nDstHeight, hMaskDC, 0, 0, SRCAND );
        ImplDrawBitmap( hMaskDC, &aPosAry, rSalBitmap, FALSE, SRCERASE );
        BitBlt( hMemDC, 0, 0, nDstWidth, nDstHeight, hMaskDC, 0, 0, SRCPAINT );
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
}

// -----------------------------------------------------------------------

void SalGraphics::DrawMask( const SalTwoRect* pPosAry,
                            const SalBitmap& rSalBitmap,
                            SalColor nMaskColor )
{
    DBG_ASSERT( !maGraphicsData.mbPrinter, "No transparency print possible!" );

    SalTwoRect  aPosAry = *pPosAry;
    const BYTE  cRed = SALCOLOR_RED( nMaskColor );
    const BYTE  cGreen = SALCOLOR_GREEN( nMaskColor );
    const BYTE  cBlue = SALCOLOR_BLUE( nMaskColor );
    HDC         hDC = maGraphicsData.mhDC;
    HBRUSH      hMaskBrush = CreateSolidBrush( RGB( cRed, cGreen, cBlue ) );
    HBRUSH      hOldBrush = SelectBrush( hDC, hMaskBrush );

    // bei Paletten-Displays hat WIN/WNT offenbar ein kleines Problem,
    // die Farben der Maske richtig auf die Palette abzubilden,
    // wenn wir die DIB direkt ausgeben => DDB-Ausgabe
    if( ( GetBitCount() <= 8 ) && rSalBitmap.ImplGethDIB() && rSalBitmap.GetBitCount() == 1 )
    {
        SalBitmap aTmp;

        if( aTmp.Create( rSalBitmap, this ) )
            ImplDrawBitmap( hDC, &aPosAry, aTmp, FALSE, 0x00B8074AUL );
    }
    else
        ImplDrawBitmap( hDC, &aPosAry, rSalBitmap, FALSE, 0x00B8074AUL );

    SelectBrush( hDC, hOldBrush );
    DeleteBrush( hMaskBrush );
}

// -----------------------------------------------------------------------

SalBitmap* SalGraphics::GetBitmap( long nX, long nY, long nDX, long nDY )
{
    DBG_ASSERT( !maGraphicsData.mbPrinter, "No ::GetBitmap() from printer possible!" );

    SalBitmap* pSalBitmap = NULL;

    nDX = labs( nDX );
    nDY = labs( nDY );

    HDC     hDC = maGraphicsData.mhDC;
    HBITMAP hBmpBitmap = CreateCompatibleBitmap( hDC, nDX, nDY );
    HDC     hBmpDC = ImplGetCachedDC( CACHED_HDC_1, hBmpBitmap );
    BOOL    bRet;

    bRet = BitBlt( hBmpDC, 0, 0, (int) nDX, (int) nDY, hDC, (int) nX, (int) nY, SRCCOPY );
    ImplReleaseCachedDC( CACHED_HDC_1 );

    if( bRet )
    {
        pSalBitmap = new SalBitmap;

        if( !pSalBitmap->Create( hBmpBitmap, FALSE, FALSE ) )
        {
            delete pSalBitmap;
            pSalBitmap = NULL;
        }
    }

    return pSalBitmap;
}

// -----------------------------------------------------------------------

SalColor SalGraphics::GetPixel( long nX, long nY )
{
    COLORREF aWinCol = ::GetPixel( maGraphicsData.mhDC, (int) nX, (int) nY );

#ifdef WIN
    if ( -1 == aWinCol )
#else
    if ( CLR_INVALID == aWinCol )
#endif
        return MAKE_SALCOLOR( 0, 0, 0 );
    else
        return MAKE_SALCOLOR( GetRValue( aWinCol ),
                              GetGValue( aWinCol ),
                              GetBValue( aWinCol ) );
}

// -----------------------------------------------------------------------

void SalGraphics::Invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags )
{
    if ( nFlags & SAL_INVERT_TRACKFRAME )
    {
        HPEN    hDotPen = CreatePen( PS_DOT, 0, 0 );
        HPEN    hOldPen = SelectPen( maGraphicsData.mhDC, hDotPen );
        HBRUSH  hOldBrush = SelectBrush( maGraphicsData.mhDC, GetStockBrush( NULL_BRUSH ) );
        int     nOldROP = SetROP2( maGraphicsData.mhDC, R2_NOT );

        WIN_Rectangle( maGraphicsData.mhDC, (int)nX, (int)nY, (int)(nX+nWidth), (int)(nY+nHeight) );

        SetROP2( maGraphicsData.mhDC, nOldROP );
        SelectPen( maGraphicsData.mhDC, hOldPen );
        SelectBrush( maGraphicsData.mhDC, hOldBrush );
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

        COLORREF nOldTextColor = ::SetTextColor( maGraphicsData.mhDC, 0 );
        HBRUSH hOldBrush = SelectBrush( maGraphicsData.mhDC, pSalData->mh50Brush );
        PatBlt( maGraphicsData.mhDC, nX, nY, nWidth, nHeight, PATINVERT );
        ::SetTextColor( maGraphicsData.mhDC, nOldTextColor );
        SelectBrush( maGraphicsData.mhDC, hOldBrush );
    }
    else
    {
         RECT aRect;
         aRect.left      = (int)nX;
         aRect.top       = (int)nY;
         aRect.right     = (int)nX+nWidth;
         aRect.bottom    = (int)nY+nHeight;
         ::InvertRect( maGraphicsData.mhDC, &aRect );
    }
}

// -----------------------------------------------------------------------

void SalGraphics::Invert( ULONG nPoints, const SalPoint* pPtAry, SalInvert nSalFlags )
{
    HPEN        hPen;
    HPEN        hOldPen;
    HBRUSH      hBrush;
    HBRUSH      hOldBrush;
    COLORREF    nOldTextColor;
    int         nOldROP = SetROP2( maGraphicsData.mhDC, R2_NOT );

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
        nOldTextColor = ::SetTextColor( maGraphicsData.mhDC, 0 );
        hOldBrush = SelectBrush( maGraphicsData.mhDC, hBrush );
    }
    hOldPen = SelectPen( maGraphicsData.mhDC, hPen );

    POINT* pWinPtAry;
#ifdef WIN
    if ( nPoints > MAX_64KSALPOINTS )
        nPoints = MAX_64KSALPOINTS;

    pWinPtAry = new POINT[(USHORT)nPoints];
    const SalPoint huge* pHugePtAry = (const SalPoint huge*)pPtAry;
    for( USHORT i=0; i < (USHORT)nPoints ; i++ )
    {
        pWinPtAry[i].x = (int)pHugePtAry[i].mnX;
        pWinPtAry[i].y = (int)pHugePtAry[i].mnY;
    }
    if ( nSalFlags & SAL_INVERT_TRACKFRAME )
        Polyline( maGraphicsData.mhDC, pWinPtAry, (int)nPoints );
    else
        WIN_Polygon( maGraphicsData.mhDC, pWinPtAry, (int)nPoints );
    delete pWinPtAry;
#else
    // Unter NT koennen wir das Array direkt weiterreichen
    DBG_ASSERT( sizeof( POINT ) == sizeof( SalPoint ),
                "SalGraphics::DrawPolyLine(): POINT != SalPoint" );

    pWinPtAry = (POINT*)pPtAry;
    // Wegen Windows 95 und der Beschraenkung auf eine maximale Anzahl
    // von Punkten
    if ( nSalFlags & SAL_INVERT_TRACKFRAME )
    {
        if ( !Polyline( maGraphicsData.mhDC, pWinPtAry, (int)nPoints ) && (nPoints > MAX_64KSALPOINTS) )
            Polyline( maGraphicsData.mhDC, pWinPtAry, MAX_64KSALPOINTS );
    }
    else
    {
        if ( !WIN_Polygon( maGraphicsData.mhDC, pWinPtAry, (int)nPoints ) && (nPoints > MAX_64KSALPOINTS) )
            WIN_Polygon( maGraphicsData.mhDC, pWinPtAry, MAX_64KSALPOINTS );
    }
#endif

    SetROP2( maGraphicsData.mhDC, nOldROP );
    SelectPen( maGraphicsData.mhDC, hOldPen );

    if ( nSalFlags & SAL_INVERT_TRACKFRAME )
        DeletePen( hPen );
    else
    {
        ::SetTextColor( maGraphicsData.mhDC, nOldTextColor );
        SelectBrush( maGraphicsData.mhDC, hOldBrush );
    }
}
