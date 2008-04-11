/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salgdi2.cxx,v $
 * $Revision: 1.7 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <string.h>
#include <svpm.h>

#define _SV_SALGDI2_CXX
#include <salbmp.h>
#include <saldata.hxx>
#ifndef _SV_SALIDS_HRC
#include <salids.hrc>
#endif
#include <salgdi.h>
#include <salvd.h>
#include <vcl/salbtype.hxx>

#ifndef __H_FT2LIB
#include <wingdi.h>
#include <ft2lib.h>
#endif

BOOL bFastTransparent = FALSE;

// -----------
// - Defines -
// -----------

#define RGBCOLOR( r, g, b ) ((ULONG)(((BYTE)(b)|((USHORT)(g)<<8))|(((ULONG)(BYTE)(r))<<16)))
#define TY( y )             (mnHeight-(y)-1)

// ---------------
// - SalGraphics -
// ---------------

bool Os2SalGraphics::supportsOperation( OutDevSupportType ) const
{
    return false;
}


void Os2SalGraphics::copyBits( const SalTwoRect* pPosAry, SalGraphics* pSrcGraphics )
{
    HPS     hSrcPS;
    POINTL  thePoints[4];
    long    nSrcHeight;

    if ( pSrcGraphics )
    {
        //hSrcPS = pSrcGraphics->mhPS;
        //nSrcHeight = pSrcGraphics->mnHeight;
        hSrcPS = static_cast<Os2SalGraphics*>(pSrcGraphics)->mhPS;
        nSrcHeight = static_cast<Os2SalGraphics*>(pSrcGraphics)->mnHeight;
    }
    else
    {
        hSrcPS = mhPS;
        nSrcHeight = mnHeight;
    }

    // lower-left corner of target
    thePoints[0].x = pPosAry->mnDestX;
    thePoints[0].y = TY( pPosAry->mnDestY + pPosAry->mnDestHeight - 1 );

    // upper-right corner of target
    thePoints[1].x = pPosAry->mnDestX + pPosAry->mnDestWidth;
    thePoints[1].y = TY( pPosAry->mnDestY - 1 );

    // lower-left corner of source
    thePoints[2].x = pPosAry->mnSrcX;
    thePoints[2].y = nSrcHeight - ( pPosAry->mnSrcY + pPosAry->mnSrcHeight );

    if ( ( pPosAry->mnDestWidth != pPosAry->mnSrcWidth ) || ( pPosAry->mnDestHeight != pPosAry->mnSrcHeight ) )
    {
        // upper-right corner of Source
        thePoints[3].x = pPosAry->mnSrcX + pPosAry->mnSrcWidth;
        thePoints[3].y = nSrcHeight - pPosAry->mnSrcY + pPosAry->mnSrcHeight;

        GpiBitBlt( mhPS, hSrcPS, 4, thePoints,
                   mbXORMode ? ROP_SRCINVERT : ROP_SRCCOPY, BBO_IGNORE );
    }
    else
    {
        GpiBitBlt( mhPS, hSrcPS, 3, thePoints,
                   mbXORMode ? ROP_SRCINVERT : ROP_SRCCOPY, BBO_IGNORE );
    }
}

// -----------------------------------------------------------------------

void Os2SalGraphics::copyArea( long nDestX, long nDestY,
                            long nSrcX, long nSrcY,
                            long nSrcWidth, long nSrcHeight,
                            USHORT nFlags )
{
    POINTL thePoints[3];

    // lower-left corner of target
    thePoints[0].x = nDestX;
    thePoints[0].y = TY( nDestY + nSrcHeight - 1 );

    // upper-right corner of target
    thePoints[1].x = nDestX + nSrcWidth;
    thePoints[1].y = TY( nDestY - 1 );

    // lower-left corner of source
    thePoints[2].x = nSrcX;
    thePoints[2].y = TY( nSrcY + nSrcHeight - 1);

    if ( (nFlags & SAL_COPYAREA_WINDOWINVALIDATE) && mbWindow )
    {
        // Overlap-Bereich berechnen und invalidieren
        Point       aVCLSrcPos( nSrcX, nSrcY );
        Size        aVCLSrcSize( nSrcWidth, nSrcHeight );
        Rectangle   aVCLSrcRect( aVCLSrcPos, aVCLSrcSize );
        Rectangle   aVCLClipRect;
        SWP         aSWP;

        WinQueryWindowPos( mhWnd, &aSWP );
        aVCLClipRect.Right()    = aSWP.cx-1;
        aVCLClipRect.Bottom()   = aSWP.cy-1;
        if ( !aVCLSrcRect.Intersection( aVCLClipRect ).IsEmpty() )
        {
            RECTL   aSrcRect;
            RECTL   aTempRect;
            HRGN    hInvalidateRgn;
            HRGN    hTempRgn;
            HWND    hWnd;
            long    nRgnType;

            long nVCLScrHeight  = aVCLSrcRect.GetHeight();
            aSrcRect.xLeft      = aVCLSrcRect.Left();
            aSrcRect.yBottom    = TY( aVCLSrcRect.Top()+nVCLScrHeight-1 );
            aSrcRect.xRight     = aSrcRect.xLeft+aVCLSrcRect.GetWidth();
            aSrcRect.yTop       = aSrcRect.yBottom+nVCLScrHeight;

            // Rechteck in Screen-Koordinaaten umrechnen
            POINTL  aPt;
            long    nScreenDX = WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN );
            long    nScreenDY = WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN );
            aPt.x = 0;
            aPt.y = 0;
            WinMapWindowPoints( mhWnd, HWND_DESKTOP, &aPt, 1 );
            aSrcRect.xLeft   += aPt.x;
            aSrcRect.yTop    += aPt.y;
            aSrcRect.xRight  += aPt.x;
            aSrcRect.yBottom += aPt.y;
            hInvalidateRgn = 0;
            // Bereiche ausserhalb des sichtbaren Bereiches berechnen
            if ( aSrcRect.xLeft < 0 )
            {
                if ( !hInvalidateRgn )
                    hInvalidateRgn = GpiCreateRegion( mhPS, 1, &aSrcRect );
                aTempRect.xLeft     = -31999;
                aTempRect.yBottom   = 0;
                aTempRect.xRight    = 0;
                aTempRect.yTop      = 31999;
                hTempRgn = GpiCreateRegion( mhPS, 1, &aTempRect );
                GpiCombineRegion( mhPS, hInvalidateRgn, hInvalidateRgn, hTempRgn, CRGN_DIFF );
                GpiDestroyRegion( mhPS, hTempRgn );
            }
            if ( aSrcRect.yBottom < 0 )
            {
                if ( !hInvalidateRgn )
                    hInvalidateRgn = GpiCreateRegion( mhPS, 1, &aSrcRect );
                aTempRect.xLeft     = 0;
                aTempRect.yBottom   = -31999;
                aTempRect.xRight    = 31999;
                aTempRect.yTop      = 0;
                hTempRgn = GpiCreateRegion( mhPS, 1, &aTempRect );
                GpiCombineRegion( mhPS, hInvalidateRgn, hInvalidateRgn, hTempRgn, CRGN_DIFF );
                GpiDestroyRegion( mhPS, hTempRgn );
            }
            if ( aSrcRect.xRight > nScreenDX )
            {
                if ( !hInvalidateRgn )
                    hInvalidateRgn = GpiCreateRegion( mhPS, 1, &aSrcRect );
                aTempRect.xLeft     = nScreenDX;
                aTempRect.yBottom   = 0;
                aTempRect.xRight    = 31999;
                aTempRect.yTop      = 31999;
                hTempRgn = GpiCreateRegion( mhPS, 1, &aTempRect );
                GpiCombineRegion( mhPS, hInvalidateRgn, hInvalidateRgn, hTempRgn, CRGN_DIFF );
                GpiDestroyRegion( mhPS, hTempRgn );
            }
            if ( aSrcRect.yTop > nScreenDY )
            {
                if ( !hInvalidateRgn )
                    hInvalidateRgn = GpiCreateRegion( mhPS, 1, &aSrcRect );
                aTempRect.xLeft     = 0;
                aTempRect.yBottom   = nScreenDY;
                aTempRect.xRight    = 31999;
                aTempRect.yTop      = 31999;
                hTempRgn = GpiCreateRegion( mhPS, 1, &aTempRect );
                GpiCombineRegion( mhPS, hInvalidateRgn, hInvalidateRgn, hTempRgn, CRGN_DIFF );
                GpiDestroyRegion( mhPS, hTempRgn );
            }

            // Bereiche die von anderen Fenstern ueberlagert werden berechnen
            // Calculate areas that are overlapped by other windows
            HWND hWndParent = WinQueryWindow( mhWnd, QW_PARENT );
            hWnd = WinQueryWindow( HWND_DESKTOP, QW_TOP );
            aVCLSrcRect = Rectangle( aSrcRect.xLeft, aSrcRect.yBottom, aSrcRect.xRight, aSrcRect.yTop );
            while ( hWnd )
            {
                if ( hWnd == hWndParent )
                    break;
                if ( WinIsWindowVisible( hWnd ) )
                {
                    WinQueryWindowPos( hWnd, &aSWP );
                    if ( !(aSWP.fl & SWP_MINIMIZE) )
                    {
                        aVCLClipRect = Rectangle( Point( aSWP.x, aSWP.y ), Size( aSWP.cx, aSWP.cy ) );
                        if ( aVCLSrcRect.IsOver( aVCLClipRect ) )
                        {
                            if ( !hInvalidateRgn )
                                hInvalidateRgn = GpiCreateRegion( mhPS, 1, &aSrcRect );
                            aTempRect.xLeft     = aSWP.x;
                            aTempRect.yBottom   = aSWP.y;
                            aTempRect.xRight    = aTempRect.xLeft+aSWP.cx;
                            aTempRect.yTop      = aTempRect.yBottom+aSWP.cy;
                            hTempRgn = GpiCreateRegion( mhPS, 1, &aTempRect );
                            GpiCombineRegion( mhPS, hInvalidateRgn, hInvalidateRgn, hTempRgn, CRGN_DIFF );
                            GpiDestroyRegion( mhPS, hTempRgn );
                        }
                    }
                }
                hWnd = WinQueryWindow( hWnd, QW_NEXT );
            }

            if ( hInvalidateRgn )
            {
                hTempRgn = GpiCreateRegion( mhPS, 1, &aSrcRect );
                nRgnType = GpiCombineRegion( mhPS, hInvalidateRgn, hTempRgn, hInvalidateRgn, CRGN_DIFF );
                GpiDestroyRegion( mhPS, hTempRgn );
                if ( (nRgnType != RGN_ERROR) && (nRgnType != RGN_NULL) )
                {
                    long nOffX = (nDestX-nSrcX);
                    long nOffY = (nSrcY-nDestY);
                    aPt.x = nOffX-aPt.x;
                    aPt.y = nOffY-aPt.y;
                    GpiOffsetRegion( mhPS, hInvalidateRgn, &aPt );
                    WinInvalidateRegion( mhWnd, hInvalidateRgn, TRUE );
                    // Hier loesen wir nur ein Update aus, wenn es der
                    // MainThread ist, damit es beim Bearbeiten der
                    // Paint-Message keinen Deadlock gibt, da der
                    // SolarMutex durch diesen Thread schon gelockt ist
                    SalData*    pSalData = GetSalData();
                    ULONG       nCurThreadId = GetCurrentThreadId();
                    if ( pSalData->mnAppThreadId == nCurThreadId )
                        WinUpdateWindow( mhWnd );
                }
                GpiDestroyRegion( mhPS, hInvalidateRgn );
            }
        }
    }

    GpiBitBlt( mhPS, mhPS, 3, thePoints,
               ROP_SRCCOPY, BBO_IGNORE );

}

// -----------------------------------------------------------------------

void ImplDrawBitmap( HPS hPS, long nScreenHeight,
                     const SalTwoRect* pPosAry, const Os2SalBitmap& rSalBitmap,
                     BOOL bPrinter, int nDrawMode )
{
    if( hPS )
    {
        HANDLE      hDrawDIB;
        HBITMAP     hDrawDDB = rSalBitmap.ImplGethDDB();
        Os2SalBitmap*   pTmpSalBmp = NULL;
        BOOL        bPrintDDB = ( bPrinter && hDrawDDB );
        BOOL        bDrawDDB1 = ( ( rSalBitmap.GetBitCount() == 1 ) && hDrawDDB );

        if( bPrintDDB || bDrawDDB1 )
        {
            pTmpSalBmp = new Os2SalBitmap;
            pTmpSalBmp->Create( rSalBitmap, rSalBitmap.GetBitCount() );
            hDrawDIB = pTmpSalBmp->ImplGethDIB();
        }
        else
            hDrawDIB = rSalBitmap.ImplGethDIB();

        if( hDrawDIB )
        {
            HANDLE              hSubst = rSalBitmap.ImplGethDIB1Subst();
            POINTL              pts[ 4 ];
            BITMAPINFO2*        pBI = (BITMAPINFO2*) hDrawDIB;
            BITMAPINFOHEADER2*  pBIH = (BITMAPINFOHEADER2*) pBI;
            const long          nHeight = pBIH->cy;
            long                nInfoSize = *(ULONG*) pBI + rSalBitmap.ImplGetDIBColorCount( hDrawDIB ) * sizeof( RGB2 );
            BYTE*               pBits = (BYTE*) pBI + nInfoSize;

            pts[0].x = pPosAry->mnDestX;
            pts[0].y = nScreenHeight - pPosAry->mnDestY - pPosAry->mnDestHeight;
            pts[1].x = pPosAry->mnDestX + pPosAry->mnDestWidth - 1;
            pts[1].y = nScreenHeight - pPosAry->mnDestY - 1;

            pts[2].x = pPosAry->mnSrcX;
            pts[2].y = nHeight - ( pPosAry->mnSrcY + pPosAry->mnSrcHeight );
            pts[3].x = pPosAry->mnSrcX + pPosAry->mnSrcWidth;
            pts[3].y = nHeight - pPosAry->mnSrcY;

            // if we've got a 1Bit DIB, we create a 4Bit substitute
            if( ( pBIH->cBitCount == 1 ) && !hSubst )
            {
                // create 4Bit substitute
                hSubst = Os2SalBitmap::ImplCreateDIB4FromDIB1( hDrawDIB );

                // replace substitute only, if it is no temporary SalBitmap
                if( !( bPrintDDB || bDrawDDB1 ) )
                    ( (Os2SalBitmap&) rSalBitmap ).ImplReplacehDIB1Subst( hSubst );
            }

            if( hSubst )
            {
                pBI = (BITMAPINFO2*) hSubst;
                pBIH = (BITMAPINFOHEADER2*) pBI;
                nInfoSize = *(ULONG*) pBI + rSalBitmap.ImplGetDIBColorCount( hSubst ) * sizeof( RGB2 );
                pBits = (BYTE*) pBI + nInfoSize;
            }

            if( bPrinter )
            {
                BYTE* pDummy;

                // expand 8Bit-DIB's to 24Bit-DIB's, because some printer drivers
                // have problems to print these DIB's (strange)
                if( pBIH->cBitCount == 8 && pBIH->ulCompression == BCA_UNCOMP )
                {
                    const long          nWidth = pBIH->cx;
                    const long          nHeight = pBIH->cy;
                    const long          nWidthAl8 = AlignedWidth4Bytes( nWidth * 8 );
                    const long          nWidthAl24 = AlignedWidth4Bytes( nWidth * 24 );
                    const long          nNewImageSize = nHeight * nWidthAl24;
                    BITMAPINFOHEADER2*  pNewInfo;

                    pDummy = new BYTE[ sizeof( BITMAPINFO2 ) + nNewImageSize ];
                    memset( pDummy, 0, sizeof( BITMAPINFO2 ) );

                    pNewInfo = (BITMAPINFOHEADER2*) pDummy;
                    pNewInfo->cbFix = sizeof( BITMAPINFOHEADER2 );
                    pNewInfo->cx = nWidth;
                    pNewInfo->cy = nHeight;
                    pNewInfo->cPlanes = 1;
                    pNewInfo->cBitCount = 24;
                    pNewInfo->ulCompression = BCA_UNCOMP;
                    pNewInfo->cbImage = nNewImageSize;

                    BYTE* pBitsSrc = (BYTE*) pBIH + nInfoSize;
                    BYTE* pBitsDst = pDummy + sizeof( BITMAPINFO2 );

                    for( long nY = 0UL; nY < nHeight; nY++ )
                    {
                        BYTE* pSrcLine = pBitsSrc + nY * nWidthAl8;
                        BYTE* pDstLine = pBitsDst + nY * nWidthAl24;

                        for( long nX = 0UL; nX < nWidth; nX++ )
                        {
                            const RGB2& rQuad = pBI->argbColor[ *pSrcLine++ ];

                            *pDstLine++ = rQuad.bBlue;
                            *pDstLine++ = rQuad.bGreen;
                            *pDstLine++ = rQuad.bRed;
                        }
                    }

                    nInfoSize = sizeof( BITMAPINFO2 );
                }
                else
                {
                    const long nImageSize = ( pBIH->cbImage ? pBIH->cbImage : ( pBIH->cy * AlignedWidth4Bytes( pBIH->cx * pBIH->cBitCount ) ) );
                    const long nTotalSize = nInfoSize + nImageSize;

                    pDummy = new BYTE[ nTotalSize ];
                    memcpy( pDummy, pBI, nTotalSize );
                }

                GpiDrawBits( hPS, pDummy + nInfoSize, (BITMAPINFO2*) pDummy, 4L, pts, nDrawMode, BBO_IGNORE );
                delete[] pDummy;
            }
            else
                GpiDrawBits( hPS, pBits, pBI, 4L, pts, nDrawMode, BBO_IGNORE );
        }
        else if( hDrawDDB && !bPrintDDB )
        {
            POINTL pts[ 4 ];

            pts[0].x = pPosAry->mnDestX;
            pts[0].y = nScreenHeight - pPosAry->mnDestY - pPosAry->mnDestHeight;
            pts[1].x = pPosAry->mnDestX + pPosAry->mnDestWidth - 1;
            pts[1].y = nScreenHeight - pPosAry->mnDestY - 1;

            pts[2].x = pPosAry->mnSrcX;
            pts[2].y = rSalBitmap.GetSize().Height() - ( pPosAry->mnSrcY + pPosAry->mnSrcHeight );
            pts[3].x = pPosAry->mnSrcX + pPosAry->mnSrcWidth;
            pts[3].y = rSalBitmap.GetSize().Height() - pPosAry->mnSrcY;

            GpiWCBitBlt( hPS, hDrawDDB, 4L, pts, nDrawMode, BBO_IGNORE );
/*
            HPS hDrawPS = ImplGetCachedPS( CACHED_HPS_DRAW, hDrawDDB );
            Ft2BitBlt( hPS, hDrawPS, 4, pts, nDrawMode, BBO_IGNORE );
            ImplReleaseCachedPS( CACHED_HPS_DRAW );
*/
        }

        if( bPrintDDB || bDrawDDB1 )
            delete pTmpSalBmp;
    }
}

// -----------------------------------------------------------------------

void Os2SalGraphics::drawBitmap( const SalTwoRect* pPosAry,
                              const SalBitmap& rSalBitmap )
{
    ImplDrawBitmap( mhPS, mnHeight,
                    pPosAry, static_cast<const Os2SalBitmap&>(rSalBitmap),
                    mbPrinter,
                    mbXORMode ? ROP_SRCINVERT : ROP_SRCCOPY );
}

// -----------------------------------------------------------------------

void Os2SalGraphics::drawBitmap( const SalTwoRect* pPosAry,
                              const SalBitmap& rSalBitmap,
                              SalColor nTransparentColor )
{
    DBG_ASSERT( !mbPrinter, "No transparency print possible!" );
    //const Os2SalBitmap& rSalBitmap = static_cast<const Os2SalBitmap&>(rSSalBitmap);
    // an FM: kann erst einmal unberuecksichtigt bleiben
    drawBitmap( pPosAry, rSalBitmap );
}

// -----------------------------------------------------------------------

void Os2SalGraphics::drawBitmap( const SalTwoRect* pPosAry,
                              const SalBitmap& rSSalBitmap,
                              const SalBitmap& rSTransparentBitmap )
{
    DBG_ASSERT( !mbPrinter, "No transparency print possible!" );

    const Os2SalBitmap& rSalBitmap = static_cast<const Os2SalBitmap&>(rSSalBitmap);
    const Os2SalBitmap& rTransparentBitmap = static_cast<const Os2SalBitmap&>(rSTransparentBitmap);

    if( bFastTransparent )
    {
        ImplDrawBitmap( mhPS, mnHeight, pPosAry, rTransparentBitmap, FALSE, ROP_SRCAND );
        ImplDrawBitmap( mhPS, mnHeight, pPosAry, rSalBitmap, FALSE, ROP_SRCPAINT );
    }
    else
    {
        SalTwoRect      aPosAry = *pPosAry;
        int             nDstX = (int) aPosAry.mnDestX;
        int             nDstY = (int) aPosAry.mnDestY;
        int             nDstWidth = (int) aPosAry.mnDestWidth;
        int             nDstHeight = (int) aPosAry.mnDestHeight;
        HAB             hAB = GetSalData()->mhAB;
        HPS             hPS = mhPS;
        DEVOPENSTRUC    aDevOpenStruc = { NULL, (PSZ)"DISPLAY", NULL, NULL, NULL, NULL, NULL, NULL, NULL };
        SIZEL           aSizeL = { nDstWidth, nDstHeight };
        POINTL          aPtL[ 3 ];

        HDC                hMemDC = DevOpenDC( hAB, OD_MEMORY, (PSZ)"*", 5L, (PDEVOPENDATA)&aDevOpenStruc, 0 );
        HPS                hMemPS = Ft2CreatePS( hAB, hMemDC, &aSizeL, GPIT_MICRO | GPIA_ASSOC | PU_PELS );
        HBITMAP            hMemBitmap = ImplCreateVirDevBitmap( hMemDC, hMemPS, nDstWidth, nDstHeight, 0 );
        HBITMAP            hMemOld = (HBITMAP) Ft2SetBitmap( hMemPS, hMemBitmap );
        HDC                hMaskDC = DevOpenDC( hAB, OD_MEMORY, (PSZ)"*", 5L, (PDEVOPENDATA)&aDevOpenStruc, 0 );
        HPS                hMaskPS = Ft2CreatePS( hAB, hMaskDC, &aSizeL, GPIT_MICRO | GPIA_ASSOC | PU_PELS );
        HBITMAP            hMaskBitmap = ImplCreateVirDevBitmap( hMaskDC, hMaskPS, nDstWidth, nDstHeight, 0 );
        HBITMAP            hMaskOld = (HBITMAP) Ft2SetBitmap( hMaskPS, hMaskBitmap );
/*
        HPS hMemPS = ImplGetCachedPS( CACHED_HPS_1, 0 );
        HPS hMaskPS = ImplGetCachedPS( CACHED_HPS_2, 0 );
*/
        aPosAry.mnDestX = aPosAry.mnDestY = 0L;

        aPtL[ 0 ].x = 0;
        aPtL[ 0 ].y = 0;
        aPtL[ 1 ].x = nDstWidth;
        aPtL[ 1 ].y = nDstHeight;
        aPtL[ 2 ].x = nDstX;
        aPtL[ 2 ].y = TY( nDstY + nDstHeight - 1 );

        GpiBitBlt( hMemPS, hPS, 3, aPtL, ROP_SRCCOPY, BBO_IGNORE );
        ImplDrawBitmap( hMaskPS, nDstHeight, &aPosAry, rTransparentBitmap, FALSE, ROP_SRCCOPY );

        aPtL[ 2 ].x = 0;
        aPtL[ 2 ].y = 0;

        GpiBitBlt( hMemPS, hMaskPS, 3, aPtL, ROP_SRCAND, BBO_IGNORE );
        ImplDrawBitmap( hMaskPS, nDstHeight, &aPosAry, rSalBitmap, FALSE, ROP_SRCERASE );
        GpiBitBlt( hMemPS, hMaskPS, 3, aPtL, ROP_SRCPAINT, BBO_IGNORE );

        aPtL[ 0 ].x = nDstX;
        aPtL[ 0 ].y = TY( nDstY + nDstHeight - 1 );
        aPtL[ 1 ].x = nDstX + nDstWidth;
        aPtL[ 1 ].y = TY( nDstY - 1 );

        GpiBitBlt( hPS, hMemPS, 3, aPtL, ROP_SRCCOPY, BBO_IGNORE );

        Ft2SetBitmap( hMaskPS, hMaskOld );
        Ft2DestroyPS( hMaskPS );
        DevCloseDC( hMaskDC );
        GpiDeleteBitmap( hMaskBitmap );

        Ft2SetBitmap( hMemPS, hMemOld );
        Ft2DestroyPS( hMemPS );
        DevCloseDC( hMemDC );
        GpiDeleteBitmap( hMemBitmap );

/*
        ImplReleaseCachedPS( CACHED_HPS_1 );
        ImplReleaseCachedPS( CACHED_HPS_2 );
*/
    }
}

// -----------------------------------------------------------------------

bool Os2SalGraphics::drawAlphaBitmap( const SalTwoRect& rTR,
                      const SalBitmap&  rSrcBitmap,
                      const SalBitmap&  rAlphaBmp )
{
    // TODO(P3) implement alpha blending
    return false;
}

// -----------------------------------------------------------------------

bool Os2SalGraphics::drawAlphaRect( long nX, long nY, long nWidth,
                                    long nHeight, sal_uInt8 nTransparency )
{
    // TODO(P3) implement alpha blending
    return false;
}

// -----------------------------------------------------------------------

void Os2SalGraphics::drawMask( const SalTwoRect* pPosAry,
                            const SalBitmap& rSSalBitmap,
                            SalColor nMaskColor )
{
    DBG_ASSERT( !mbPrinter, "No transparency print possible!" );

    const Os2SalBitmap& rSalBitmap = static_cast<const Os2SalBitmap&>(rSSalBitmap);

    SalTwoRect  aPosAry = *pPosAry;
    HPS         hPS = mhPS;
    IMAGEBUNDLE aBundle, aOldBundle;
    AREABUNDLE  aAreaBundle, aOldAreaBundle;
    const ULONG    nColor = RGBCOLOR( SALCOLOR_RED( nMaskColor ),
                                    SALCOLOR_GREEN( nMaskColor ),
                                    SALCOLOR_BLUE( nMaskColor ) );

    GpiQueryAttrs( hPS, PRIM_IMAGE, IBB_COLOR | IBB_BACK_COLOR, &aOldBundle );
    aBundle.lColor = RGBCOLOR( 0, 0, 0 );
    aBundle.lBackColor = RGBCOLOR( 0xFF, 0xFF, 0xFF );
    Ft2SetAttrs( hPS, PRIM_IMAGE, IBB_COLOR | IBB_BACK_COLOR, 0, &aBundle );

    GpiQueryAttrs( hPS, PRIM_AREA, ABB_COLOR | ABB_BACK_COLOR | ABB_SYMBOL |
                   ABB_MIX_MODE | ABB_BACK_MIX_MODE, &aOldAreaBundle );
    aAreaBundle.lColor = nColor;
    aAreaBundle.lBackColor = nColor;
    aAreaBundle.usSymbol = PATSYM_SOLID;
    aAreaBundle.usMixMode = FM_OVERPAINT;
    aAreaBundle.usBackMixMode = BM_OVERPAINT;
    Ft2SetAttrs( hPS, PRIM_AREA, ABB_COLOR | ABB_BACK_COLOR | ABB_SYMBOL |
                 ABB_MIX_MODE | ABB_BACK_MIX_MODE, 0, &aAreaBundle );

    ImplDrawBitmap( hPS, mnHeight, &aPosAry, rSalBitmap, FALSE, 0x00B8L );

    Ft2SetAttrs( hPS, PRIM_IMAGE, IBB_COLOR | IBB_BACK_COLOR, 0, &aOldBundle );
    Ft2SetAttrs( hPS, PRIM_AREA, ABB_COLOR | ABB_BACK_COLOR | ABB_SYMBOL |
                 ABB_MIX_MODE | ABB_BACK_MIX_MODE, 0, &aOldAreaBundle );
}

// -----------------------------------------------------------------------

SalBitmap* Os2SalGraphics::getBitmap( long nX, long nY, long nDX, long nDY )
{
    HAB            hAB = GetSalData()->mhAB;
    SIZEL        size = { nDX, nDY };
    Os2SalBitmap*     pSalBitmap = NULL;

    // create device context (at this time allways display compatible)
    DEVOPENSTRUC    aDevOpenStruc = { NULL, (PSZ)"DISPLAY", NULL, NULL, NULL, NULL, NULL, NULL, NULL };
    HDC             hMemDC = DevOpenDC( hAB, OD_MEMORY, (PSZ)"*", 5L, (PDEVOPENDATA)&aDevOpenStruc, 0 );
    HPS             hMemPS = Ft2CreatePS( hAB, hMemDC, &size, GPIT_MICRO | GPIA_ASSOC | PU_PELS );
    HBITMAP         hMemBmp = ImplCreateVirDevBitmap( hMemDC, hMemPS, nDX, nDY, 0 );
    HBITMAP         hMemOld = Ft2SetBitmap( hMemPS, hMemBmp );

    // creation successfull?
    if( hMemDC && hMemPS && hMemBmp )
    {
        POINTL thePoints[ 3 ];

        // lower-left corner of target
        thePoints[ 0 ].x = 0;
        thePoints[ 0 ].y = 0;

        // upper-right corner of target
        thePoints[ 1 ].x = nDX;
        thePoints[ 1 ].y = nDY;

        // lower-left corner of source
        thePoints[ 2 ].x = nX;
        thePoints[ 2 ].y = TY( nY + nDY - 1 );

        long lHits = GpiBitBlt( hMemPS, mhPS, 3, thePoints,
                                mbXORMode ? ROP_SRCINVERT : ROP_SRCCOPY, BBO_IGNORE );

        if( hMemPS )
        {
            Ft2SetBitmap( hMemPS, hMemOld );
            Ft2DestroyPS( hMemPS );
        }

        if( hMemDC )
            DevCloseDC( hMemDC );

        if( lHits == GPI_OK )
        {
            pSalBitmap = new Os2SalBitmap;

            if( !pSalBitmap->Create( hMemBmp, FALSE, FALSE ) )
            {
                delete pSalBitmap;
                pSalBitmap = NULL;
            }
        }
    }

    if( !pSalBitmap )
        GpiDeleteBitmap( hMemBmp );

    // return pointer to SAL-Bitmap
    return pSalBitmap;
}

// -----------------------------------------------------------------------

SalColor Os2SalGraphics::getPixel( long nX, long nY )
{
    POINTL    aPt = { nX, TY( nY ) };
    LONG    nColor = Ft2QueryPel( mhPS, &aPt );

    return MAKE_SALCOLOR( (BYTE) ( nColor >> 16 ), (BYTE) ( nColor >> 8 ), (BYTE) nColor );
}

// -----------------------------------------------------------------------

void Os2SalGraphics::invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags )
{
    if( nFlags & SAL_INVERT_TRACKFRAME )
    {
        // save old vylues
        LINEBUNDLE oldLb;
        LINEBUNDLE lb;
        GpiQueryAttrs( mhPS, PRIM_LINE, LBB_MIX_MODE | LBB_TYPE | LBB_COLOR, &oldLb );

        // set linetype to short dash
        lb.lColor = RGBCOLOR( 255, 255, 255 );
        lb.usMixMode = FM_XOR;
        lb.usType = LINETYPE_ALTERNATE;
        Ft2SetAttrs( mhPS, PRIM_LINE, LBB_MIX_MODE | LBB_TYPE | LBB_COLOR, 0, &lb );

        // draw inverted box
        POINTL aPt;

        aPt.x = nX;
        aPt.y = TY( nY );

        Ft2Move( mhPS, &aPt );

        aPt.x = nX + nWidth - 1;
        aPt.y = TY( nY + nHeight - 1 );

        Ft2Box( mhPS, DRO_OUTLINE, &aPt, 0, 0 );

        // restore old values
        Ft2SetAttrs( mhPS, PRIM_LINE, LBB_MIX_MODE | LBB_TYPE | LBB_COLOR, 0, &oldLb );

    }
    else
    {
        // save old values
        AREABUNDLE oldAb;
        AREABUNDLE ab;

        GpiQueryAttrs( mhPS, PRIM_AREA, ABB_COLOR | ABB_MIX_MODE | ABB_SYMBOL, &oldAb );

        // set fill color to black
        ab.lColor = RGBCOLOR( 255, 255, 255 );
        ab.usMixMode = FM_XOR;
        ab.usSymbol = (nFlags & SAL_INVERT_50) ? PATSYM_DENSE5 : PATSYM_SOLID;
        Ft2SetAttrs( mhPS, PRIM_AREA, ABB_COLOR | ABB_MIX_MODE | ABB_SYMBOL, 0, &ab );

        // draw inverted box
        POINTL aPt;

        aPt.x = nX;
        aPt.y = TY( nY );

        Ft2Move( mhPS, &aPt );

        aPt.x = nX + nWidth - 1;
        aPt.y = TY( nY + nHeight - 1 );

        Ft2Box( mhPS, DRO_FILL, &aPt, 0, 0 );

        // restore old values
        Ft2SetAttrs( mhPS, PRIM_AREA, ABB_COLOR | ABB_MIX_MODE | ABB_SYMBOL, 0, &oldAb );
    }
}

// -----------------------------------------------------------------------

void Os2SalGraphics::invert( ULONG nPoints, const SalPoint* pPtAry, SalInvert nFlags )
{
    if( nFlags & SAL_INVERT_TRACKFRAME )
    {
        // save old vylues
        LINEBUNDLE oldLb;
        LINEBUNDLE lb;
        GpiQueryAttrs( mhPS, PRIM_LINE, LBB_MIX_MODE | LBB_TYPE | LBB_COLOR, &oldLb );

        // set linetype to short dash
        lb.lColor = RGBCOLOR( 255, 255, 255 );
        lb.usMixMode = FM_XOR;
        lb.usType = LINETYPE_ALTERNATE;
        Ft2SetAttrs( mhPS, PRIM_LINE, LBB_MIX_MODE | LBB_TYPE | LBB_COLOR, 0, &lb );

        // Draw Polyline
        drawPolyLine( nPoints, pPtAry );

        // restore old values
        Ft2SetAttrs( mhPS, PRIM_LINE, LBB_MIX_MODE | LBB_TYPE | LBB_COLOR, 0, &oldLb );
    }
    else
    {
        // save old values
        AREABUNDLE oldAb;
        AREABUNDLE ab;

        GpiQueryAttrs( mhPS, PRIM_AREA, ABB_COLOR | ABB_MIX_MODE | ABB_SYMBOL, &oldAb );

        // set fill color to black
        ab.lColor = RGBCOLOR( 255, 255, 255 );
        ab.usMixMode = FM_XOR;
        ab.usSymbol = (nFlags & SAL_INVERT_50) ? PATSYM_DENSE5 : PATSYM_SOLID;
        Ft2SetAttrs( mhPS, PRIM_AREA, ABB_COLOR | ABB_MIX_MODE | ABB_SYMBOL, 0, &ab );

        // Draw Polyline
        drawPolygon( nPoints, pPtAry );

        // restore old values
        Ft2SetAttrs( mhPS, PRIM_AREA, ABB_COLOR | ABB_MIX_MODE | ABB_SYMBOL, 0, &oldAb );
    }
}

