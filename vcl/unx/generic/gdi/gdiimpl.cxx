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

#include <prex.h>
#include <X11/Xproto.h>
#include <postx.h>

#include "gdiimpl.hxx"

#include "vcl/salbtype.hxx"
#include <vcl/gradient.hxx>

#include "unx/salunx.h"
#include "unx/saldata.hxx"
#include "unx/saldisp.hxx"
#include "unx/salbmp.h"
#include "unx/salgdi.h"
#include "unx/salframe.h"
#include "unx/salvd.h"
#include <unx/x11/xlimits.hxx>
#include "xrender_peer.hxx"

#include "outdata.hxx"

#include "basegfx/polygon/b2dpolygon.hxx"
#include "basegfx/polygon/b2dpolypolygon.hxx"
#include "basegfx/polygon/b2dpolypolygontools.hxx"
#include "basegfx/polygon/b2dpolygontools.hxx"
#include "basegfx/polygon/b2dpolygonclipper.hxx"
#include "basegfx/polygon/b2dlinegeometry.hxx"
#include "basegfx/matrix/b2dhommatrix.hxx"
#include "basegfx/matrix/b2dhommatrixtools.hxx"
#include "basegfx/polygon/b2dpolypolygoncutter.hxx"
#include "basegfx/polygon/b2dtrapezoid.hxx"

#undef SALGDI2_TESTTRANS

#if (OSL_DEBUG_LEVEL > 1) && defined SALGDI2_TESTTRANS
#define DBG_TESTTRANS( _def_drawable )                              \
{                                                                   \
    XCopyArea( pXDisp, _def_drawable, aDrawable, GetCopyGC(),       \
               0, 0,                                                \
               rPosAry.mnDestWidth, rPosAry.mnDestHeight,         \
               0, 0 );                                              \
}
#else // (OSL_DEBUG_LEVEL > 1) && defined SALGDI2_TESTTRANS
#define DBG_TESTTRANS( _def_drawable )
#endif // (OSL_DEBUG_LEVEL > 1) && defined SALGDI2_TESTTRANS

#define STATIC_POINTS 64

class SalPolyLine
{
    XPoint Points_[STATIC_POINTS];
    XPoint *pFirst_;
public:
    SalPolyLine(sal_uLong nPoints, const SalPoint *p)
        : pFirst_(nPoints+1 > STATIC_POINTS ? new XPoint[nPoints+1] : Points_)
    {
        for( sal_uLong i = 0; i < nPoints; i++ )
        {
            pFirst_[i].x = (short)p[i].mnX;
            pFirst_[i].y = (short)p[i].mnY;
        }
        pFirst_[nPoints] = pFirst_[0]; // close polyline
    }

    ~SalPolyLine()
    {
        if( pFirst_ != Points_ )
            delete [] pFirst_;
    }

    XPoint &operator [] ( sal_uLong n ) const
    {
        return pFirst_[n];
    }
};

#undef STATIC_POINTS

namespace
{
    void setForeBack(XGCValues& rValues, const SalColormap& rColMap, const SalBitmap& rSalBitmap)
    {
        rValues.foreground = rColMap.GetWhitePixel();
        rValues.background = rColMap.GetBlackPixel();

        //fdo#33455 and fdo#80160 handle 1 bit depth pngs with palette entries
        //to set fore/back colors
        SalBitmap& rBitmap = const_cast<SalBitmap&>(rSalBitmap);
        if (BitmapBuffer* pBitmapBuffer = rBitmap.AcquireBuffer(BITMAP_READ_ACCESS))
        {
            const BitmapPalette& rPalette = pBitmapBuffer->maPalette;
            if (rPalette.GetEntryCount() == 2)
            {
                const BitmapColor aWhite(rPalette[rPalette.GetBestIndex(Color(COL_WHITE))]);
                rValues.foreground = rColMap.GetPixel(ImplColorToSal(aWhite));

                const BitmapColor aBlack(rPalette[rPalette.GetBestIndex(Color(COL_BLACK))]);
                rValues.background = rColMap.GetPixel(ImplColorToSal(aBlack));
            }
            rBitmap.ReleaseBuffer(pBitmapBuffer, BITMAP_READ_ACCESS);
        }
    }
}

X11SalGraphicsImpl::X11SalGraphicsImpl(X11SalGraphics& rParent):
    mrParent(rParent),
    mnBrushColor( MAKE_SALCOLOR( 0xFF, 0xFF, 0XFF ) ),
    mpBrushGC(NULL),
    mnBrushPixel(0),
    mbPenGC(false),
    mbBrushGC(false),
    mbMonoGC(false),
    mbCopyGC(false),
    mbInvertGC(false),
    mbInvert50GC(false),
    mbStippleGC(false),
    mbTrackingGC(false),
    mbDitherBrush(false),
    mbXORMode(false),
    mpPenGC(NULL),
    mnPenColor( MAKE_SALCOLOR( 0x00, 0x00, 0x00 ) ),
    mnPenPixel(0),
    mpMonoGC(NULL),
    mpCopyGC(NULL),
    mpMaskGC(NULL),
    mpInvertGC(NULL),
    mpInvert50GC(NULL),
    mpStippleGC(NULL),
    mpTrackingGC(NULL)
{
}

X11SalGraphicsImpl::~X11SalGraphicsImpl()
{
}

void X11SalGraphicsImpl::Init()
{
    mnPenPixel = mrParent.GetPixel( mnPenColor );
    mnBrushPixel = mrParent.GetPixel( mnBrushColor );
}

bool X11SalGraphicsImpl::FillPixmapFromScreen( X11Pixmap* pPixmap, int nX, int nY )
{
    //TODO lfrb: don't hardcode the depth
    Display* pDpy = mrParent.GetXDisplay();
    GC aTmpGC = XCreateGC( pDpy, pPixmap->GetPixmap(), 0, NULL );

    if( !aTmpGC )
    {
        SAL_WARN( "vcl", "Could not create GC from screen" );
        return false;
    }

    // Copy the background of the screen into a composite pixmap
    X11SalGraphics::CopyScreenArea( mrParent.GetXDisplay(),
                             mrParent.GetDrawable(), mrParent.GetScreenNumber(),
                             mrParent.GetVisual().GetDepth(),
                             pPixmap->GetDrawable(), pPixmap->GetScreen(),
                             pPixmap->GetDepth(),
                             aTmpGC,
                             nX, nY, pPixmap->GetWidth(), pPixmap->GetHeight(),
                             0, 0 );

    XFreeGC( pDpy, aTmpGC );
    return true;
}

bool X11SalGraphicsImpl::RenderPixmapToScreen( X11Pixmap* pPixmap, X11Pixmap* /*Mask*/, int nX, int nY )
{
    // TODO: lfrb: Use the mask
    GC aFontGC = mrParent.GetFontGC();

    // The GC can't be null, otherwise we'd have no clip region
    if( aFontGC == NULL )
    {
        SAL_WARN( "vcl", "no valid GC to render pixmap" );
        return false;
    }

    if( !pPixmap )
        return false;

    X11SalGraphics::CopyScreenArea( mrParent.GetXDisplay(),
                             pPixmap->GetDrawable(), pPixmap->GetScreen(),
                             pPixmap->GetDepth(),
                             mrParent.GetDrawable(), mrParent.m_nXScreen,
                             mrParent.GetVisual().GetDepth(),
                             aFontGC,
                             0, 0,
                             pPixmap->GetWidth(), pPixmap->GetHeight(),
                             nX, nY );
    return true;
}

bool X11SalGraphicsImpl::TryRenderCachedNativeControl(ControlCacheKey& /*rControlCacheKey*/, int /*nX*/, int /*nY*/)
{
    return false;
}

bool X11SalGraphicsImpl::RenderAndCacheNativeControl(X11Pixmap* pPixmap, X11Pixmap* pMask, int nX, int nY,
                                              ControlCacheKey& /*rControlCacheKey*/)
{
    return RenderPixmapToScreen(pPixmap, pMask, nX, nY);
}

XID X11SalGraphicsImpl::GetXRenderPicture()
{
    XRenderPeer& rRenderPeer = XRenderPeer::GetInstance();

    if( !mrParent.m_aXRenderPicture )
    {
        // check xrender support for matching visual
        XRenderPictFormat* pXRenderFormat = mrParent.GetXRenderFormat();
        if( !pXRenderFormat )
            return 0;
        // get the matching xrender target for drawable
        mrParent.m_aXRenderPicture = rRenderPeer.CreatePicture( mrParent.hDrawable_, pXRenderFormat, 0, NULL );
    }

    {
        // reset clip region
        // TODO: avoid clip reset if already done
        XRenderPictureAttributes aAttr;
        aAttr.clip_mask = None;
        rRenderPeer.ChangePicture( mrParent.m_aXRenderPicture, CPClipMask, &aAttr );
    }

    return mrParent.m_aXRenderPicture;
}

void X11SalGraphicsImpl::freeResources()
{
    Display *pDisplay = mrParent.GetXDisplay();

    if( mpPenGC ) XFreeGC( pDisplay, mpPenGC ), mpPenGC = None;
    if( mpBrushGC ) XFreeGC( pDisplay, mpBrushGC ), mpBrushGC = None;
    if( mpMonoGC ) XFreeGC( pDisplay, mpMonoGC ), mpMonoGC = None;
    if( mpTrackingGC ) XFreeGC( pDisplay, mpTrackingGC ), mpTrackingGC = None;
    if( mpCopyGC ) XFreeGC( pDisplay, mpCopyGC ), mpCopyGC = None;
    if( mpMaskGC ) XFreeGC( pDisplay, mpMaskGC ), mpMaskGC = None;
    if( mpInvertGC ) XFreeGC( pDisplay, mpInvertGC ), mpInvertGC = None;
    if( mpInvert50GC ) XFreeGC( pDisplay, mpInvert50GC ), mpInvert50GC = None;
    if( mpStippleGC ) XFreeGC( pDisplay, mpStippleGC ), mpStippleGC = None;
    mbTrackingGC = mbPenGC = mbBrushGC = mbMonoGC = mbCopyGC = mbInvertGC = mbInvert50GC = mbStippleGC = false;
}

GC X11SalGraphicsImpl::CreateGC( Drawable hDrawable, unsigned long nMask )
{
    XGCValues values;

    values.graphics_exposures   = False;
    values.foreground           = mrParent.m_pColormap->GetBlackPixel()
                                  ^ mrParent.m_pColormap->GetWhitePixel();
    values.function             = GXxor;
    values.line_width           = 1;
    values.fill_style           = FillStippled;
    values.stipple              = mrParent.GetDisplay()->GetInvert50( mrParent.m_nXScreen );
    values.subwindow_mode       = ClipByChildren;

    return XCreateGC( mrParent.GetXDisplay(), hDrawable, nMask | GCSubwindowMode, &values );
}

inline GC X11SalGraphicsImpl::GetCopyGC()
{
    if( mbXORMode ) return GetInvertGC();

    if( !mpCopyGC )
        mpCopyGC = CreateGC( mrParent.GetDrawable() );

    if( !mbCopyGC )
    {
        mrParent.SetClipRegion( mpCopyGC );
        mbCopyGC = true;
    }
    return mpCopyGC;
}

GC X11SalGraphicsImpl::GetTrackingGC()
{
    const char    dash_list[2] = {2, 2};

    if( !mpTrackingGC )
    {
        XGCValues     values;

        values.graphics_exposures   = False;
        values.foreground           = mrParent.m_pColormap->GetBlackPixel()
                                      ^ mrParent.m_pColormap->GetWhitePixel();
        values.function             = GXxor;
        values.line_width           = 1;
        values.line_style           = LineOnOffDash;

        mpTrackingGC = XCreateGC( mrParent.GetXDisplay(), mrParent.GetDrawable(),
                                  GCGraphicsExposures | GCForeground | GCFunction
                                  | GCLineWidth | GCLineStyle,
                                  &values );
        XSetDashes( mrParent.GetXDisplay(), mpTrackingGC, 0, dash_list, 2 );
    }

    if( !mbTrackingGC )
    {
        mrParent.SetClipRegion( mpTrackingGC );
        mbTrackingGC = true;
    }

    return mpTrackingGC;
}

GC X11SalGraphicsImpl::GetInvertGC()
{
    if( !mpInvertGC )
        mpInvertGC = CreateGC( mrParent.GetDrawable(),
                               GCGraphicsExposures
                               | GCForeground
                               | GCFunction
                               | GCLineWidth );

    if( !mbInvertGC )
    {
        mrParent.SetClipRegion( mpInvertGC );
        mbInvertGC = true;
    }
    return mpInvertGC;
}

GC X11SalGraphicsImpl::GetInvert50GC()
{
    if( !mpInvert50GC )
    {
        XGCValues values;

        values.graphics_exposures   = False;
        values.foreground           = mrParent.m_pColormap->GetWhitePixel();
        values.background           = mrParent.m_pColormap->GetBlackPixel();
        values.function             = GXinvert;
        values.line_width           = 1;
        values.line_style           = LineSolid;
        unsigned long nValueMask =
                                  GCGraphicsExposures
                                  | GCForeground
                                  | GCBackground
                                  | GCFunction
                                  | GCLineWidth
                                  | GCLineStyle
                                  | GCFillStyle
                                  | GCStipple;

        char* pEnv = getenv( "SAL_DO_NOT_USE_INVERT50" );
        if( pEnv && ! strcasecmp( pEnv, "true" ) )
        {
            values.fill_style = FillSolid;
            nValueMask &= ~ GCStipple;
        }
        else
        {
            values.fill_style           = FillStippled;
            values.stipple              = mrParent.GetDisplay()->GetInvert50( mrParent.m_nXScreen );
        }

        mpInvert50GC = XCreateGC( mrParent.GetXDisplay(), mrParent.GetDrawable(),
                                  nValueMask,
                                  &values );
    }

    if( !mbInvert50GC )
    {
        mrParent.SetClipRegion( mpInvert50GC );
        mbInvert50GC = true;
    }
    return mpInvert50GC;
}

inline GC X11SalGraphicsImpl::GetStippleGC()
{
    if( !mpStippleGC )
        mpStippleGC = CreateGC( mrParent.GetDrawable(),
                                GCGraphicsExposures
                                | GCFillStyle
                                | GCLineWidth );

    if( !mbStippleGC )
    {
        XSetFunction( mrParent.GetXDisplay(), mpStippleGC, mbXORMode ? GXxor : GXcopy );
        mrParent.SetClipRegion( mpStippleGC );
        mbStippleGC = true;
    }

    return mpStippleGC;
}

GC X11SalGraphicsImpl::SelectBrush()
{
    Display *pDisplay = mrParent.GetXDisplay();

    DBG_ASSERT( mnBrushColor != SALCOLOR_NONE, "Brush Transparent" );

    if( !mpBrushGC )
    {
        XGCValues values;
        values.subwindow_mode       = ClipByChildren;
        values.fill_rule            = EvenOddRule;      // Pict import/ Gradient
        values.graphics_exposures   = False;

        mpBrushGC = XCreateGC( pDisplay, mrParent.hDrawable_,
                               GCSubwindowMode | GCFillRule | GCGraphicsExposures,
                               &values );
    }

    if( !mbBrushGC )
    {
        if( !mbDitherBrush )
        {
            XSetFillStyle ( pDisplay, mpBrushGC, FillSolid );
            XSetForeground( pDisplay, mpBrushGC, mnBrushPixel );
            if( mrParent.bPrinter_ )
                XSetTile( pDisplay, mpBrushGC, None );
        }
        else
        {
            XSetFillStyle ( pDisplay, mpBrushGC, FillTiled );
            XSetTile      ( pDisplay, mpBrushGC, mrParent.hBrush_ );
        }
        XSetFunction  ( pDisplay, mpBrushGC, mbXORMode ? GXxor : GXcopy );
        mrParent.SetClipRegion( mpBrushGC );

        mbBrushGC = true;
    }

    return mpBrushGC;
}

GC X11SalGraphicsImpl::SelectPen()
{
    Display *pDisplay = mrParent.GetXDisplay();

    if( !mpPenGC )
    {
        XGCValues values;
        values.subwindow_mode       = ClipByChildren;
        values.fill_rule            = EvenOddRule;      // Pict import/ Gradient
        values.graphics_exposures   = False;

        mpPenGC = XCreateGC( pDisplay, mrParent.hDrawable_,
                             GCSubwindowMode | GCFillRule | GCGraphicsExposures,
                             &values );
    }

    if( !mbPenGC )
    {
        if( mnPenColor != SALCOLOR_NONE )
            XSetForeground( pDisplay, mpPenGC, mnPenPixel );
        XSetFunction  ( pDisplay, mpPenGC, mbXORMode ? GXxor : GXcopy );
        mrParent.SetClipRegion( mpPenGC );
        mbPenGC = true;
    }

    return mpPenGC;
}

void X11SalGraphicsImpl::DrawLines( sal_uLong              nPoints,
                                const SalPolyLine &rPoints,
                                GC                 pGC,
                                bool               bClose
                                )
{
    // calculate how many lines XWindow can draw in one go
    sal_uLong nMaxLines = (mrParent.GetDisplay()->GetMaxRequestSize() - sizeof(xPolyPointReq))
                      / sizeof(xPoint);
    if( nMaxLines > nPoints ) nMaxLines = nPoints;

    // print all lines that XWindows can draw
    sal_uLong n;
    for( n = 0; nPoints - n > nMaxLines; n += nMaxLines - 1 )
        XDrawLines( mrParent.GetXDisplay(),
                    mrParent.GetDrawable(),
                    pGC,
                    &rPoints[n],
                    nMaxLines,
                    CoordModeOrigin );

    if( n < nPoints )
        XDrawLines( mrParent.GetXDisplay(),
                    mrParent.GetDrawable(),
                    pGC,
                    &rPoints[n],
                    nPoints - n,
                    CoordModeOrigin );
    if( bClose )
    {
        if( rPoints[nPoints-1].x != rPoints[0].x || rPoints[nPoints-1].y != rPoints[0].y )
            drawLine( rPoints[nPoints-1].x, rPoints[nPoints-1].y, rPoints[0].x, rPoints[0].y );
    }
}

void X11SalGraphicsImpl::copyBits( const SalTwoRect& rPosAry,
                                  SalGraphics      *pSSrcGraphics )
{
    X11SalGraphics* pSrcGraphics = pSSrcGraphics
        ? static_cast<X11SalGraphics*>(pSSrcGraphics)
        : &mrParent;

    if( rPosAry.mnSrcWidth <= 0
        || rPosAry.mnSrcHeight <= 0
        || rPosAry.mnDestWidth <= 0
        || rPosAry.mnDestHeight <= 0 )
    {
        return;
    }

    int n;
    if( pSrcGraphics == &mrParent )
    {
        n = 2;
    }
    else if( pSrcGraphics->bWindow_ )
    {
        // window or compatible virtual device
        if( pSrcGraphics->GetDisplay() == mrParent.GetDisplay() &&
            pSrcGraphics->m_nXScreen == mrParent.m_nXScreen &&
            pSrcGraphics->GetVisual().GetDepth() == mrParent.GetVisual().GetDepth()
            )
            n = 2; // same Display
        else
            n = 1; // printer or other display
    }
    else if( pSrcGraphics->bVirDev_ )
    {
        // printer compatible virtual device
        if( mrParent.bPrinter_ )
            n = 2; // printer or compatible virtual device == same display
        else
            n = 1; // window or compatible virtual device
    }
    else
        n = 0;

    if( n == 2
        && rPosAry.mnSrcWidth   == rPosAry.mnDestWidth
        && rPosAry.mnSrcHeight == rPosAry.mnDestHeight
        )
    {
        // #i60699# Need to generate graphics exposures (to repaint
        // obscured areas beneath overlapping windows), src and dest
        // are the same window.
        const bool bNeedGraphicsExposures( pSrcGraphics == &mrParent &&
                                           !mrParent.bVirDev_ &&
                                           pSrcGraphics->bWindow_ );

        GC pCopyGC = GetCopyGC();

        if( bNeedGraphicsExposures )
            XSetGraphicsExposures( mrParent.GetXDisplay(),
                                   pCopyGC,
                                   True );

        XCopyArea( mrParent.GetXDisplay(),
                   pSrcGraphics->GetDrawable(),     // source
                   mrParent.GetDrawable(),                   // destination
                   pCopyGC,                         // destination clipping
                   rPosAry.mnSrcX,     rPosAry.mnSrcY,
                   rPosAry.mnSrcWidth, rPosAry.mnSrcHeight,
                   rPosAry.mnDestX,    rPosAry.mnDestY );

        if( bNeedGraphicsExposures )
        {
            mrParent.YieldGraphicsExpose();

            if( pCopyGC )
                XSetGraphicsExposures( mrParent.GetXDisplay(),
                                       pCopyGC,
                                       False );
        }
    }
    else if( n )
    {
        // #i60699# No chance to handle graphics exposures - we copy
        // to a temp bitmap first, into which no repaints are
        // technically possible.
        std::unique_ptr<SalBitmap> xDDB(pSrcGraphics->getBitmap( rPosAry.mnSrcX,
                                                                   rPosAry.mnSrcY,
                                                                   rPosAry.mnSrcWidth,
                                                                   rPosAry.mnSrcHeight ));

        if( !xDDB )
        {
            stderr0( "SalGraphics::CopyBits !pSrcGraphics->GetBitmap()\n" );
            return;
        }

        SalTwoRect aPosAry( rPosAry );

        aPosAry.mnSrcX = 0, aPosAry.mnSrcY = 0;
        drawBitmap( aPosAry, *xDDB );
    }
    else {
        stderr0( "X11SalGraphicsImpl::CopyBits from Printer not yet implemented\n" );
    }
}

void X11SalGraphicsImpl::copyArea ( long nDestX,    long nDestY,
                                long nSrcX,     long nSrcY,
                                long nSrcWidth, long nSrcHeight,
                                sal_uInt16 )
{
    SalTwoRect aPosAry(nSrcX, nSrcY, nSrcWidth, nSrcHeight, nDestX, nDestY, nSrcWidth, nSrcHeight);
    copyBits(aPosAry, 0);
}

void X11SalGraphicsImpl::drawBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap )
{
    const SalDisplay*   pSalDisp = mrParent.GetDisplay();
    Display*            pXDisp = pSalDisp->GetDisplay();
    const Drawable      aDrawable( mrParent.GetDrawable() );
    const SalColormap&  rColMap = pSalDisp->GetColormap( mrParent.m_nXScreen );
    const long          nDepth = mrParent.GetDisplay()->GetVisual( mrParent.m_nXScreen ).GetDepth();
    GC                  aGC( GetCopyGC() );
    XGCValues           aOldVal, aNewVal;
    int                 nValues = GCForeground | GCBackground;

    if( rSalBitmap.GetBitCount() == 1 )
    {
        // set foreground/background values for 1Bit bitmaps
        XGetGCValues( pXDisp, aGC, nValues, &aOldVal );
        setForeBack(aNewVal, rColMap, rSalBitmap);
        XChangeGC( pXDisp, aGC, nValues, &aNewVal );
    }

    static_cast<const X11SalBitmap&>(rSalBitmap).ImplDraw( aDrawable, mrParent.m_nXScreen, nDepth, rPosAry, aGC );

    if( rSalBitmap.GetBitCount() == 1 )
        XChangeGC( pXDisp, aGC, nValues, &aOldVal );
    XFlush( pXDisp );
}

void X11SalGraphicsImpl::drawBitmap( const SalTwoRect& rPosAry,
                                 const SalBitmap& rSrcBitmap,
                                 const SalBitmap& rMaskBitmap )
{
    DBG_ASSERT( !mrParent.bPrinter_, "Drawing of transparent bitmaps on printer devices is strictly forbidden" );

    // decide if alpha masking or transparency masking is needed
    BitmapBuffer* pAlphaBuffer = const_cast<SalBitmap&>(rMaskBitmap).AcquireBuffer( BITMAP_READ_ACCESS );
    if( pAlphaBuffer != NULL )
    {
        int nMaskFormat = pAlphaBuffer->mnFormat;
        const_cast<SalBitmap&>(rMaskBitmap).ReleaseBuffer( pAlphaBuffer, BITMAP_READ_ACCESS );
        if( nMaskFormat == BMP_FORMAT_8BIT_PAL )
            drawAlphaBitmap( rPosAry, rSrcBitmap, rMaskBitmap );
    }

    drawMaskedBitmap( rPosAry, rSrcBitmap, rMaskBitmap );
}

void X11SalGraphicsImpl::drawMaskedBitmap( const SalTwoRect& rPosAry,
                                       const SalBitmap& rSalBitmap,
                                       const SalBitmap& rTransBitmap )
{
    const SalDisplay*   pSalDisp = mrParent.GetDisplay();
    Display*            pXDisp = pSalDisp->GetDisplay();
    Drawable            aDrawable( mrParent.GetDrawable() );

    // figure work mode depth. If this is a VDev Drawable, use its
    // bitdepth to create pixmaps for, otherwise, XCopyArea will
    // refuse to work.
    const sal_uInt16    nDepth( mrParent.m_pVDev ?
                            static_cast< X11SalVirtualDevice* >(mrParent.m_pVDev)->GetDepth() :
                            pSalDisp->GetVisual( mrParent.m_nXScreen ).GetDepth() );
    Pixmap          aFG( limitXCreatePixmap( pXDisp, aDrawable, rPosAry.mnDestWidth,
                                        rPosAry.mnDestHeight, nDepth ) );
    Pixmap          aBG( limitXCreatePixmap( pXDisp, aDrawable, rPosAry.mnDestWidth,
                                        rPosAry.mnDestHeight, nDepth ) );

    if( aFG && aBG )
    {
        GC                  aTmpGC;
        XGCValues           aValues;
        setForeBack(aValues, pSalDisp->GetColormap(mrParent.m_nXScreen), rSalBitmap);
        const int           nValues = GCFunction | GCForeground | GCBackground;
        SalTwoRect          aTmpRect( rPosAry ); aTmpRect.mnDestX = aTmpRect.mnDestY = 0;

        // draw paint bitmap in pixmap #1
        aValues.function = GXcopy;
        aTmpGC = XCreateGC( pXDisp, aFG, nValues, &aValues );
        static_cast<const X11SalBitmap&>(rSalBitmap).ImplDraw( aFG, mrParent.m_nXScreen, nDepth, aTmpRect, aTmpGC );
        DBG_TESTTRANS( aFG );

        // draw background in pixmap #2
        XCopyArea( pXDisp, aDrawable, aBG, aTmpGC,
                   rPosAry.mnDestX, rPosAry.mnDestY,
                   rPosAry.mnDestWidth, rPosAry.mnDestHeight,
                   0, 0 );

        DBG_TESTTRANS( aBG );

        // mask out paint bitmap in pixmap #1 (transparent areas 0)
        aValues.function = GXand, aValues.foreground = 0x00000000, aValues.background = 0xffffffff;
        XChangeGC( pXDisp, aTmpGC, nValues, &aValues );
        static_cast<const X11SalBitmap&>(rTransBitmap).ImplDraw( aFG, mrParent.m_nXScreen, 1, aTmpRect, aTmpGC );

        DBG_TESTTRANS( aFG );

        // #105055# For XOR mode, keep background behind bitmap intact
        if( !mbXORMode )
        {
            // mask out background in pixmap #2 (nontransparent areas 0)
            aValues.function = GXand, aValues.foreground = 0xffffffff, aValues.background = 0x00000000;
            XChangeGC( pXDisp, aTmpGC, nValues, &aValues );
            static_cast<const X11SalBitmap&>(rTransBitmap).ImplDraw( aBG, mrParent.m_nXScreen, 1, aTmpRect, aTmpGC );

            DBG_TESTTRANS( aBG );
        }

        // merge pixmap #1 and pixmap #2 in pixmap #2
        aValues.function = GXxor, aValues.foreground = 0xffffffff, aValues.background = 0x00000000;
        XChangeGC( pXDisp, aTmpGC, nValues, &aValues );
        XCopyArea( pXDisp, aFG, aBG, aTmpGC,
                   0, 0,
                   rPosAry.mnDestWidth, rPosAry.mnDestHeight,
                   0, 0 );
        DBG_TESTTRANS( aBG );

        // #105055# Disable XOR temporarily
        bool bOldXORMode( mbXORMode );
        mbXORMode = false;

        // copy pixmap #2 (result) to background
        XCopyArea( pXDisp, aBG, aDrawable, GetCopyGC(),
                   0, 0,
                   rPosAry.mnDestWidth, rPosAry.mnDestHeight,
                   rPosAry.mnDestX, rPosAry.mnDestY );

        DBG_TESTTRANS( aBG );

        mbXORMode = bOldXORMode;

        XFreeGC( pXDisp, aTmpGC );
        XFlush( pXDisp );
    }
    else
        drawBitmap( rPosAry, rSalBitmap );

    if( aFG )
        XFreePixmap( pXDisp, aFG );

    if( aBG )
        XFreePixmap( pXDisp, aBG );
}

bool X11SalGraphicsImpl::blendBitmap( const SalTwoRect&,
    const SalBitmap& )
{
    return false;
}

bool X11SalGraphicsImpl::blendAlphaBitmap( const SalTwoRect&,
    const SalBitmap&, const SalBitmap&, const SalBitmap& )
{
    return false;
}

bool X11SalGraphicsImpl::drawAlphaBitmap( const SalTwoRect& rTR,
    const SalBitmap& rSrcBitmap, const SalBitmap& rAlphaBmp )
{
    // non 8-bit alpha not implemented yet
    if( rAlphaBmp.GetBitCount() != 8 )
        return false;

    // horizontal mirroring not implemented yet
    if( rTR.mnDestWidth < 0 )
        return false;

    // stretched conversion is not implemented yet
    if( rTR.mnDestWidth != rTR.mnSrcWidth )
        return false;
    if( rTR.mnDestHeight!= rTR.mnSrcHeight )
        return false;

    // create destination picture
    Picture aDstPic = GetXRenderPicture();
    if( !aDstPic )
        return false;

    const SalDisplay* pSalDisp = mrParent.GetDisplay();
    const SalVisual& rSalVis = pSalDisp->GetVisual( mrParent.m_nXScreen );
    Display* pXDisplay = pSalDisp->GetDisplay();

    // create source Picture
    int nDepth = mrParent.m_pVDev ? static_cast< X11SalVirtualDevice* >(mrParent.m_pVDev)->GetDepth() : rSalVis.GetDepth();
    const X11SalBitmap& rSrcX11Bmp = static_cast<const X11SalBitmap&>( rSrcBitmap );
    ImplSalDDB* pSrcDDB = rSrcX11Bmp.ImplGetDDB( mrParent.hDrawable_, mrParent.m_nXScreen, nDepth, rTR );
    if( !pSrcDDB )
        return false;

    //#i75249# workaround for ImplGetDDB() giving us back a different depth than
    // we requested. E.g. mask pixmaps are always compatible with the drawable
    // TODO: find an appropriate picture format for these cases
    //       then remove the workaround below and the one for #i75531#
    if( nDepth != pSrcDDB->ImplGetDepth() )
        return false;

    Pixmap aSrcPM = pSrcDDB->ImplGetPixmap();
    if( !aSrcPM )
        return false;

    // create source picture
    // TODO: use scoped picture
    Visual* pSrcXVisual = rSalVis.GetVisual();
    XRenderPeer& rPeer = XRenderPeer::GetInstance();
    XRenderPictFormat* pSrcVisFmt = rPeer.FindVisualFormat( pSrcXVisual );
    if( !pSrcVisFmt )
        return false;
    Picture aSrcPic = rPeer.CreatePicture( aSrcPM, pSrcVisFmt, 0, NULL );
    if( !aSrcPic )
        return false;

    // create alpha Picture

    // TODO: use SalX11Bitmap functionality and caching for the Alpha Pixmap
    // problem is that they don't provide an 8bit Pixmap on a non-8bit display
    BitmapBuffer* pAlphaBuffer = const_cast<SalBitmap&>(rAlphaBmp).AcquireBuffer( BITMAP_READ_ACCESS );

    // an XImage needs its data top_down
    // TODO: avoid wrongly oriented images in upper layers!
    const int nImageSize = pAlphaBuffer->mnHeight * pAlphaBuffer->mnScanlineSize;
    const char* pSrcBits = reinterpret_cast<char*>(pAlphaBuffer->mpBits);
    char* pAlphaBits = new char[ nImageSize ];
    if( BMP_SCANLINE_ADJUSTMENT( pAlphaBuffer->mnFormat ) == BMP_FORMAT_TOP_DOWN )
        memcpy( pAlphaBits, pSrcBits, nImageSize );
    else
    {
        char* pDstBits = pAlphaBits + nImageSize;
        const int nLineSize = pAlphaBuffer->mnScanlineSize;
        for(; (pDstBits -= nLineSize) >= pAlphaBits; pSrcBits += nLineSize )
            memcpy( pDstBits, pSrcBits, nLineSize );
    }

    // the alpha values need to be inverted for XRender
    // TODO: make upper layers use standard alpha
    long* pLDst = reinterpret_cast<long*>(pAlphaBits);
    for( int i = nImageSize/sizeof(long); --i >= 0; ++pLDst )
        *pLDst = ~*pLDst;

    char* pCDst = reinterpret_cast<char*>(pLDst);
    for( int i = nImageSize & (sizeof(long)-1); --i >= 0; ++pCDst )
        *pCDst = ~*pCDst;

    const XRenderPictFormat* pAlphaFormat = rPeer.GetStandardFormatA8();
    XImage* pAlphaImg = XCreateImage( pXDisplay, pSrcXVisual, 8, ZPixmap, 0,
        pAlphaBits, pAlphaBuffer->mnWidth, pAlphaBuffer->mnHeight,
        pAlphaFormat->depth, pAlphaBuffer->mnScanlineSize );

    Pixmap aAlphaPM = limitXCreatePixmap( pXDisplay, mrParent.hDrawable_,
        rTR.mnDestWidth, rTR.mnDestHeight, 8 );

    XGCValues aAlphaGCV;
    aAlphaGCV.function = GXcopy;
    GC aAlphaGC = XCreateGC( pXDisplay, aAlphaPM, GCFunction, &aAlphaGCV );
    XPutImage( pXDisplay, aAlphaPM, aAlphaGC, pAlphaImg,
        rTR.mnSrcX, rTR.mnSrcY, 0, 0, rTR.mnDestWidth, rTR.mnDestHeight );
    XFreeGC( pXDisplay, aAlphaGC );
    XFree( pAlphaImg );
    if( pAlphaBits != reinterpret_cast<char*>(pAlphaBuffer->mpBits) )
        delete[] pAlphaBits;

    const_cast<SalBitmap&>(rAlphaBmp).ReleaseBuffer( pAlphaBuffer, BITMAP_READ_ACCESS );

    XRenderPictureAttributes aAttr;
    aAttr.repeat = int(true);
    Picture aAlphaPic = rPeer.CreatePicture( aAlphaPM, pAlphaFormat, CPRepeat, &aAttr );
    if( !aAlphaPic )
        return false;

    // set clipping
    if( mrParent.mpClipRegion && !XEmptyRegion( mrParent.mpClipRegion ) )
        rPeer.SetPictureClipRegion( aDstPic, mrParent.mpClipRegion );

    // paint source * mask over destination picture
    rPeer.CompositePicture( PictOpOver, aSrcPic, aAlphaPic, aDstPic,
        rTR.mnSrcX, rTR.mnSrcY, 0, 0,
        rTR.mnDestX, rTR.mnDestY, rTR.mnDestWidth, rTR.mnDestHeight );

    rPeer.FreePicture( aAlphaPic );
    XFreePixmap(pXDisplay, aAlphaPM);
    rPeer.FreePicture( aSrcPic );
    return true;
}

bool X11SalGraphicsImpl::drawTransformedBitmap(
    const basegfx::B2DPoint& rNull,
    const basegfx::B2DPoint& rX,
    const basegfx::B2DPoint& rY,
    const SalBitmap& rSourceBitmap,
    const SalBitmap* pAlphaBitmap)
{
    // here direct support for transformed bitmaps can be implemented
    (void)rNull; (void)rX; (void)rY; (void)rSourceBitmap; (void)pAlphaBitmap;
    return false;
}

bool X11SalGraphicsImpl::drawAlphaRect( long nX, long nY, long nWidth,
                                    long nHeight, sal_uInt8 nTransparency )
{
    if( ! mrParent.m_pFrame && ! mrParent.m_pVDev )
        return false;

    if( mbPenGC || !mbBrushGC || mbXORMode )
        return false; // can only perform solid fills without XOR.

    if( mrParent.m_pVDev && static_cast< X11SalVirtualDevice* >(mrParent.m_pVDev)->GetDepth() < 8 )
        return false;

    Picture aDstPic = GetXRenderPicture();
    if( !aDstPic )
        return false;

    const double fTransparency = (100 - nTransparency) * (1.0/100);
    const XRenderColor aRenderColor = GetXRenderColor( mnBrushColor , fTransparency);

    XRenderPeer& rPeer = XRenderPeer::GetInstance();
    rPeer.FillRectangle( PictOpOver,
                         aDstPic,
                         &aRenderColor,
                         nX, nY,
                         nWidth, nHeight );

    return true;
}

void X11SalGraphicsImpl::drawMask( const SalTwoRect& rPosAry,
                               const SalBitmap &rSalBitmap,
                               SalColor nMaskColor )
{
    const SalDisplay*   pSalDisp = mrParent.GetDisplay();
    Display*            pXDisp = pSalDisp->GetDisplay();
    Drawable            aDrawable( mrParent.GetDrawable() );
    Pixmap              aStipple( limitXCreatePixmap( pXDisp, aDrawable,
                                                 rPosAry.mnDestWidth,
                                                 rPosAry.mnDestHeight, 1 ) );

    if( aStipple )
    {
        SalTwoRect  aTwoRect( rPosAry ); aTwoRect.mnDestX = aTwoRect.mnDestY = 0;
        GC          aTmpGC;
        XGCValues   aValues;

        // create a stipple bitmap first (set bits are changed to unset bits and vice versa)
        aValues.function = GXcopyInverted;
        aValues.foreground = 1, aValues.background = 0;
        aTmpGC = XCreateGC( pXDisp, aStipple, GCFunction | GCForeground | GCBackground, &aValues );
        static_cast<const X11SalBitmap&>(rSalBitmap).ImplDraw( aStipple, mrParent.m_nXScreen, 1, aTwoRect, aTmpGC );

        XFreeGC( pXDisp, aTmpGC );

        // Set stipple and draw rectangle
        GC  aStippleGC( GetStippleGC() );
        int nX = rPosAry.mnDestX, nY = rPosAry.mnDestY;

        XSetStipple( pXDisp, aStippleGC, aStipple );
        XSetTSOrigin( pXDisp, aStippleGC, nX, nY );
        XSetForeground( pXDisp, aStippleGC, mrParent.GetPixel( nMaskColor ) );
        XFillRectangle( pXDisp, aDrawable, aStippleGC,
                        nX, nY,
                        rPosAry.mnDestWidth, rPosAry.mnDestHeight );
        XFreePixmap( pXDisp, aStipple );
        XFlush( pXDisp );
    }
    else
        drawBitmap( rPosAry, rSalBitmap );
}

void X11SalGraphicsImpl::ResetClipRegion()
{
    if( mrParent.mpClipRegion )
    {
        mbPenGC         = false;
        mrParent.bFontGC_ = false;
        mbBrushGC       = false;
        mbMonoGC        = false;
        mbCopyGC        = false;
        mbInvertGC      = false;
        mbInvert50GC    = false;
        mbStippleGC     = false;
        mbTrackingGC    = false;

        XDestroyRegion( mrParent.mpClipRegion );
        mrParent.mpClipRegion    = NULL;
    }
}

bool X11SalGraphicsImpl::setClipRegion( const vcl::Region& i_rClip )
{
    if( mrParent.mpClipRegion )
        XDestroyRegion( mrParent.mpClipRegion );
    mrParent.mpClipRegion = XCreateRegion();

    RectangleVector aRectangles;
    i_rClip.GetRegionRectangles(aRectangles);

    for(RectangleVector::const_iterator aRectIter(aRectangles.begin()); aRectIter != aRectangles.end(); ++aRectIter)
    {
        const long nW(aRectIter->GetWidth());

        if(nW)
        {
            const long nH(aRectIter->GetHeight());

            if(nH)
            {
                XRectangle aRect;

                aRect.x = (short)aRectIter->Left();
                aRect.y = (short)aRectIter->Top();
                aRect.width = (unsigned short)nW;
                aRect.height = (unsigned short)nH;
                XUnionRectWithRegion(&aRect, mrParent.mpClipRegion, mrParent.mpClipRegion);
            }
        }
    }

    //ImplRegionInfo aInfo;
    //long nX, nY, nW, nH;
    //bool bRegionRect = i_rClip.ImplGetFirstRect(aInfo, nX, nY, nW, nH );
    //while( bRegionRect )
    //{
    //    if ( nW && nH )
    //    {
    //        XRectangle aRect;
    //        aRect.x           = (short)nX;
    //        aRect.y           = (short)nY;
    //        aRect.width       = (unsigned short)nW;
    //        aRect.height  = (unsigned short)nH;

    //        XUnionRectWithRegion( &aRect, mrParent.mpClipRegion, mrParent.mpClipRegion );
    //    }
    //    bRegionRect = i_rClip.ImplGetNextRect( aInfo, nX, nY, nW, nH );
    //}

    // done, invalidate GCs
    mbPenGC         = false;
    mrParent.bFontGC_ = false;
    mbBrushGC       = false;
    mbMonoGC        = false;
    mbCopyGC        = false;
    mbInvertGC      = false;
    mbInvert50GC    = false;
    mbStippleGC     = false;
    mbTrackingGC    = false;

    if( XEmptyRegion( mrParent.mpClipRegion ) )
    {
        XDestroyRegion( mrParent.mpClipRegion );
        mrParent.mpClipRegion= NULL;
    }
    return true;
}

void X11SalGraphicsImpl::SetLineColor()
{
    if( mnPenColor != SALCOLOR_NONE )
    {
        mnPenColor      = SALCOLOR_NONE;
        mbPenGC         = false;
    }
}

void X11SalGraphicsImpl::SetLineColor( SalColor nSalColor )
{
    if( mnPenColor != nSalColor )
    {
        mnPenColor      = nSalColor;
        mnPenPixel      = mrParent.GetPixel( nSalColor );
        mbPenGC         = false;
    }
}

void X11SalGraphicsImpl::SetFillColor()
{
    if( mnBrushColor != SALCOLOR_NONE )
    {
        mbDitherBrush   = false;
        mnBrushColor    = SALCOLOR_NONE;
        mbBrushGC       = false;
    }
}

void X11SalGraphicsImpl::SetFillColor( SalColor nSalColor )
{
    if( mnBrushColor != nSalColor )
    {
        mbDitherBrush   = false;
        mnBrushColor    = nSalColor;
        mnBrushPixel    = mrParent.GetPixel( nSalColor );
        if( TrueColor != mrParent.GetColormap().GetVisual().GetClass()
            && mrParent.GetColormap().GetColor( mnBrushPixel ) != mnBrushColor
            && nSalColor != MAKE_SALCOLOR( 0x00, 0x00, 0x00 ) // black
            && nSalColor != MAKE_SALCOLOR( 0x00, 0x00, 0x80 ) // blue
            && nSalColor != MAKE_SALCOLOR( 0x00, 0x80, 0x00 ) // green
            && nSalColor != MAKE_SALCOLOR( 0x00, 0x80, 0x80 ) // cyan
            && nSalColor != MAKE_SALCOLOR( 0x80, 0x00, 0x00 ) // red
            && nSalColor != MAKE_SALCOLOR( 0x80, 0x00, 0x80 ) // magenta
            && nSalColor != MAKE_SALCOLOR( 0x80, 0x80, 0x00 ) // brown
            && nSalColor != MAKE_SALCOLOR( 0x80, 0x80, 0x80 ) // gray
            && nSalColor != MAKE_SALCOLOR( 0xC0, 0xC0, 0xC0 ) // light gray
            && nSalColor != MAKE_SALCOLOR( 0x00, 0x00, 0xFF ) // light blue
            && nSalColor != MAKE_SALCOLOR( 0x00, 0xFF, 0x00 ) // light green
            && nSalColor != MAKE_SALCOLOR( 0x00, 0xFF, 0xFF ) // light cyan
            && nSalColor != MAKE_SALCOLOR( 0xFF, 0x00, 0x00 ) // light red
            && nSalColor != MAKE_SALCOLOR( 0xFF, 0x00, 0xFF ) // light magenta
            && nSalColor != MAKE_SALCOLOR( 0xFF, 0xFF, 0x00 ) // light brown
            && nSalColor != MAKE_SALCOLOR( 0xFF, 0xFF, 0xFF ) )
            mbDitherBrush = mrParent.GetDitherPixmap(nSalColor);
        mbBrushGC       = false;
    }
}

void X11SalGraphicsImpl::SetROPLineColor( SalROPColor nROPColor )
{
    switch( nROPColor )
    {
        case SAL_ROP_0 : // 0
            mnPenPixel = (Pixel)0;
            break;
        case SAL_ROP_1 : // 1
            mnPenPixel = (Pixel)(1 << mrParent.GetVisual().GetDepth()) - 1;
            break;
        case SAL_ROP_INVERT : // 2
            mnPenPixel = (Pixel)(1 << mrParent.GetVisual().GetDepth()) - 1;
            break;
    }
    mnPenColor  = mrParent.GetColormap().GetColor( mnPenPixel );
    mbPenGC     = false;
}

void X11SalGraphicsImpl::SetROPFillColor( SalROPColor nROPColor )
{
    switch( nROPColor )
    {
        case SAL_ROP_0 : // 0
            mnBrushPixel = (Pixel)0;
            break;
        case SAL_ROP_1 : // 1
            mnBrushPixel = (Pixel)(1 << mrParent.GetVisual().GetDepth()) - 1;
            break;
        case SAL_ROP_INVERT : // 2
            mnBrushPixel = (Pixel)(1 << mrParent.GetVisual().GetDepth()) - 1;
            break;
    }
    mbDitherBrush   = false;
    mnBrushColor    = mrParent.GetColormap().GetColor( mnBrushPixel );
    mbBrushGC       = false;
}

void X11SalGraphicsImpl::SetXORMode( bool bSet, bool )
{
    if (mbXORMode != bSet)
    {
        mbXORMode   = bSet;
        mbPenGC     = false;
        mrParent.bFontGC_ = false;
        mbBrushGC   = false;
        mbMonoGC        = false;
        mbCopyGC        = false;
        mbInvertGC  = false;
        mbInvert50GC    = false;
        mbStippleGC = false;
        mbTrackingGC    = false;
    }
}

void X11SalGraphicsImpl::drawPixel( long nX, long nY )
{
    if( mnPenColor !=  SALCOLOR_NONE )
        XDrawPoint( mrParent.GetXDisplay(), mrParent.GetDrawable(), SelectPen(), nX, nY );
}

void X11SalGraphicsImpl::drawPixel( long nX, long nY, SalColor nSalColor )
{
    if( nSalColor != SALCOLOR_NONE )
    {
        Display *pDisplay = mrParent.GetXDisplay();

        if( (mnPenColor == SALCOLOR_NONE) && !mbPenGC )
        {
            SetLineColor( nSalColor );
            XDrawPoint( pDisplay, mrParent.GetDrawable(), SelectPen(), nX, nY );
            mnPenColor = SALCOLOR_NONE;
            mbPenGC = False;
        }
        else
        {
            GC pGC = SelectPen();

            if( nSalColor != mnPenColor )
                XSetForeground( pDisplay, pGC, mrParent.GetPixel( nSalColor ) );

            XDrawPoint( pDisplay, mrParent.GetDrawable(), pGC, nX, nY );

            if( nSalColor != mnPenColor )
                XSetForeground( pDisplay, pGC, mnPenPixel );
        }
    }
}

void X11SalGraphicsImpl::drawLine( long nX1, long nY1, long nX2, long nY2 )
{
    if( mnPenColor != SALCOLOR_NONE )
    {
        XDrawLine( mrParent.GetXDisplay(), mrParent.GetDrawable(),SelectPen(),
                   nX1, nY1, nX2, nY2 );
    }
}

void X11SalGraphicsImpl::drawRect( long nX, long nY, long nDX, long nDY )
{
    if( mnBrushColor != SALCOLOR_NONE )
    {
        XFillRectangle( mrParent.GetXDisplay(),
                        mrParent.GetDrawable(),
                        SelectBrush(),
                        nX, nY, nDX, nDY );
    }
    // description DrawRect is wrong; thus -1
    if( mnPenColor != SALCOLOR_NONE )
        XDrawRectangle( mrParent.GetXDisplay(),
                        mrParent.GetDrawable(),
                        SelectPen(),
                        nX, nY, nDX-1, nDY-1 );
}

void X11SalGraphicsImpl::drawPolyLine( sal_uInt32 nPoints, const SalPoint *pPtAry )
{
    drawPolyLine( nPoints, pPtAry, false );
}

void X11SalGraphicsImpl::drawPolyLine( sal_uInt32 nPoints, const SalPoint *pPtAry, bool bClose )
{
    if( mnPenColor != SALCOLOR_NONE )
    {
        SalPolyLine Points( nPoints, pPtAry );

        DrawLines( nPoints, Points, SelectPen(), bClose );
    }
}

void X11SalGraphicsImpl::drawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry )
{
    if( nPoints == 0 )
        return;

    if( nPoints < 3 )
    {
        if( !mbXORMode )
        {
            if( 1 == nPoints  )
                drawPixel( pPtAry[0].mnX, pPtAry[0].mnY );
            else
                drawLine( pPtAry[0].mnX, pPtAry[0].mnY,
                          pPtAry[1].mnX, pPtAry[1].mnY );
        }
        return;
    }

    SalPolyLine Points( nPoints, pPtAry );

    nPoints++;

    /* WORKAROUND: some Xservers (Xorg, VIA chipset in this case)
     * do not draw the visible part of a polygon
     * if it overlaps to the left of screen 0,y.
     * This happens to be the case in the gradient drawn in the
     * menubar background. workaround for the special case of
     * of a rectangle overlapping to the left.
     */
    if( nPoints == 5 &&
    Points[ 0 ].x == Points[ 1 ].x &&
        Points[ 1 ].y == Points[ 2 ].y &&
        Points[ 2 ].x == Points[ 3 ].x &&
        Points[ 0 ].x == Points[ 4 ].x && Points[ 0 ].y == Points[ 4 ].y
       )
    {
        bool bLeft = false;
        bool bRight = false;
        for(unsigned int i = 0; i < nPoints; i++ )
    {
            if( Points[i].x < 0 )
                bLeft = true;
            else
                bRight= true;
    }
    if( bLeft && ! bRight )
        return;
    if( bLeft && bRight )
        {
            for( unsigned int i = 0; i < nPoints; i++ )
                if( Points[i].x < 0 )
                    Points[i].x = 0;
        }
    }

    if( mnBrushColor != SALCOLOR_NONE )
        XFillPolygon( mrParent.GetXDisplay(),
                      mrParent.GetDrawable(),
                      SelectBrush(),
                      &Points[0], nPoints,
                      Complex, CoordModeOrigin );

    if( mnPenColor != SALCOLOR_NONE )
        DrawLines( nPoints, Points, SelectPen(), true );
}

void X11SalGraphicsImpl::drawPolyPolygon( sal_uInt32 nPoly,
                                   const sal_uInt32    *pPoints,
                                   PCONSTSALPOINT  *pPtAry )
{
    if( mnBrushColor != SALCOLOR_NONE )
    {
        sal_uInt32      i, n;
        Region          pXRegA  = NULL;

        for( i = 0; i < nPoly; i++ ) {
            n = pPoints[i];
            SalPolyLine Points( n, pPtAry[i] );
            if( n > 2 )
            {
                Region pXRegB = XPolygonRegion( &Points[0], n+1, WindingRule );
                if( !pXRegA )
                    pXRegA = pXRegB;
                else
                {
                    XXorRegion( pXRegA, pXRegB, pXRegA );
                    XDestroyRegion( pXRegB );
                }
            }
        }

        if( pXRegA )
        {
            XRectangle aXRect;
            XClipBox( pXRegA, &aXRect );

            GC pGC = SelectBrush();
            mrParent.SetClipRegion( pGC, pXRegA ); // ??? twice
            XDestroyRegion( pXRegA );
            mbBrushGC = false;

            XFillRectangle( mrParent.GetXDisplay(),
                            mrParent.GetDrawable(),
                            pGC,
                            aXRect.x, aXRect.y, aXRect.width, aXRect.height );
        }
   }

   if( mnPenColor != SALCOLOR_NONE )
       for( sal_uInt32 i = 0; i < nPoly; i++ )
           drawPolyLine( pPoints[i], pPtAry[i], true );
}

bool X11SalGraphicsImpl::drawPolyLineBezier( sal_uInt32, const SalPoint*, const sal_uInt8* )
{
    return false;
}

bool X11SalGraphicsImpl::drawPolygonBezier( sal_uInt32, const SalPoint*, const sal_uInt8* )
{
    return false;
}

bool X11SalGraphicsImpl::drawPolyPolygonBezier( sal_uInt32, const sal_uInt32*,
                                                const SalPoint* const*, const sal_uInt8* const* )
{
    return false;
}

void X11SalGraphicsImpl::invert( long       nX,
                                long        nY,
                                long        nDX,
                                long        nDY,
                                SalInvert   nFlags )
{
    GC pGC;
    if( SAL_INVERT_50 & nFlags )
    {
        pGC = GetInvert50GC();
        XFillRectangle( mrParent.GetXDisplay(), mrParent.GetDrawable(), pGC, nX, nY, nDX, nDY );
    }
    else
    {
        if ( SAL_INVERT_TRACKFRAME & nFlags )
        {
            pGC = GetTrackingGC();
            XDrawRectangle( mrParent.GetXDisplay(), mrParent.GetDrawable(),  pGC, nX, nY, nDX, nDY );
        }
        else
        {
            pGC = GetInvertGC();
            XFillRectangle( mrParent.GetXDisplay(), mrParent.GetDrawable(),  pGC, nX, nY, nDX, nDY );
        }
    }
}

void X11SalGraphicsImpl::invert( sal_uInt32 nPoints,
                             const SalPoint* pPtAry,
                             SalInvert nFlags )
{
    SalPolyLine Points ( nPoints, pPtAry );

    GC pGC;
    if( SAL_INVERT_50 & nFlags )
        pGC = GetInvert50GC();
    else
        if ( SAL_INVERT_TRACKFRAME & nFlags )
            pGC = GetTrackingGC();
        else
            pGC = GetInvertGC();

    if( SAL_INVERT_TRACKFRAME & nFlags )
        DrawLines ( nPoints, Points, pGC, true );
    else
        XFillPolygon( mrParent.GetXDisplay(),
                      mrParent.GetDrawable(),
                      pGC,
                      &Points[0], nPoints,
                      Complex, CoordModeOrigin );
}

bool X11SalGraphicsImpl::drawEPS( long,long,long,long,void*,sal_uLong )
{
    return false;
}

// draw a poly-polygon
bool X11SalGraphicsImpl::drawPolyPolygon( const ::basegfx::B2DPolyPolygon& rOrigPolyPoly, double fTransparency )
{
    // nothing to do for empty polypolygons
    const int nOrigPolyCount = rOrigPolyPoly.count();
    if( nOrigPolyCount <= 0 )
        return true;

    // nothing to do if everything is transparent
    if( (mnBrushColor == SALCOLOR_NONE)
    &&  (mnPenColor == SALCOLOR_NONE) )
        return true;

    // cannot handle pencolor!=brushcolor yet
    if( (mnPenColor != SALCOLOR_NONE)
    &&  (mnPenColor != mnBrushColor) )
        return false;

    // TODO: remove the env-variable when no longer needed
    static const char* pRenderEnv = getenv( "SAL_DISABLE_RENDER_POLY" );
    if( pRenderEnv )
        return false;

    // snap to raster if requested
    basegfx::B2DPolyPolygon aPolyPoly = rOrigPolyPoly;
    const bool bSnapToRaster = !mrParent.getAntiAliasB2DDraw();
    if( bSnapToRaster )
        aPolyPoly = basegfx::tools::snapPointsOfHorizontalOrVerticalEdges( aPolyPoly );

    // don't bother with polygons outside of visible area
    const basegfx::B2DRange aViewRange( 0, 0, GetGraphicsWidth(), GetGraphicsHeight() );
    aPolyPoly = basegfx::tools::clipPolyPolygonOnRange( aPolyPoly, aViewRange, true, false );
    if( !aPolyPoly.count() )
        return true;

    // tesselate the polypolygon into trapezoids
    basegfx::B2DTrapezoidVector aB2DTrapVector;
    basegfx::tools::trapezoidSubdivide( aB2DTrapVector, aPolyPoly );
    const int nTrapCount = aB2DTrapVector.size();
    if( !nTrapCount )
        return true;
    const bool bDrawn = drawFilledTrapezoids( &aB2DTrapVector[0], nTrapCount, fTransparency );
    return bDrawn;
}

long X11SalGraphicsImpl::GetGraphicsHeight() const
{
    if( mrParent.m_pFrame )
        return mrParent.m_pFrame->maGeometry.nHeight;
    else if( mrParent.m_pVDev )
        return static_cast< X11SalVirtualDevice* >(mrParent.m_pVDev)->GetHeight();
    else
        return 0;
}

bool X11SalGraphicsImpl::drawFilledTrapezoids( const ::basegfx::B2DTrapezoid* pB2DTraps, int nTrapCount, double fTransparency )
{
    if( nTrapCount <= 0 )
        return true;

    Picture aDstPic = GetXRenderPicture();
    // check xrender support for this drawable
    if( !aDstPic )
        return false;

     // convert the B2DTrapezoids into XRender-Trapezoids
    typedef std::vector<XTrapezoid> TrapezoidVector;
    TrapezoidVector aTrapVector( nTrapCount );
    const basegfx::B2DTrapezoid* pB2DTrap = pB2DTraps;
    for( int i = 0; i < nTrapCount; ++pB2DTrap, ++i )
    {
        XTrapezoid& rTrap = aTrapVector[ i ] ;

         // set y-coordinates
        const double fY1 = pB2DTrap->getTopY();
        rTrap.left.p1.y = rTrap.right.p1.y = rTrap.top = XDoubleToFixed( fY1 );
        const double fY2 = pB2DTrap->getBottomY();
        rTrap.left.p2.y = rTrap.right.p2.y = rTrap.bottom = XDoubleToFixed( fY2 );

         // set x-coordinates
        const double fXL1 = pB2DTrap->getTopXLeft();
        rTrap.left.p1.x = XDoubleToFixed( fXL1 );
        const double fXR1 = pB2DTrap->getTopXRight();
        rTrap.right.p1.x = XDoubleToFixed( fXR1 );
        const double fXL2 = pB2DTrap->getBottomXLeft();
        rTrap.left.p2.x = XDoubleToFixed( fXL2 );
        const double fXR2 = pB2DTrap->getBottomXRight();
        rTrap.right.p2.x = XDoubleToFixed( fXR2 );
    }

    // get xrender Picture for polygon foreground
    // TODO: cache it like the target picture which uses GetXRenderPicture()
    XRenderPeer& rRenderPeer = XRenderPeer::GetInstance();
    SalDisplay::RenderEntry& rEntry = mrParent.GetDisplay()->GetRenderEntries( mrParent.m_nXScreen )[ 32 ];
    if( !rEntry.m_aPicture )
    {
        Display* pXDisplay = mrParent.GetXDisplay();

        rEntry.m_aPixmap = limitXCreatePixmap( pXDisplay, mrParent.hDrawable_, 1, 1, 32 );
        XRenderPictureAttributes aAttr;
        aAttr.repeat = int(true);

        XRenderPictFormat* pXRPF = rRenderPeer.FindStandardFormat( PictStandardARGB32 );
        rEntry.m_aPicture = rRenderPeer.CreatePicture( rEntry.m_aPixmap, pXRPF, CPRepeat, &aAttr );
    }

    // set polygon foreground color and opacity
    XRenderColor aRenderColor = GetXRenderColor( mnBrushColor , fTransparency );
    rRenderPeer.FillRectangle( PictOpSrc, rEntry.m_aPicture, &aRenderColor, 0, 0, 1, 1 );

    // set clipping
    // TODO: move into GetXRenderPicture?
    if( mrParent.mpClipRegion && !XEmptyRegion( mrParent.mpClipRegion ) )
        rRenderPeer.SetPictureClipRegion( aDstPic, mrParent.mpClipRegion );

    // render the trapezoids
    const XRenderPictFormat* pMaskFormat = rRenderPeer.GetStandardFormatA8();
    rRenderPeer.CompositeTrapezoids( PictOpOver,
        rEntry.m_aPicture, aDstPic, pMaskFormat, 0, 0, &aTrapVector[0], aTrapVector.size() );

    return true;
}

bool X11SalGraphicsImpl::drawPolyLine(
    const ::basegfx::B2DPolygon& rPolygon,
    double fTransparency,
    const ::basegfx::B2DVector& rLineWidth,
    basegfx::B2DLineJoin eLineJoin,
    com::sun::star::drawing::LineCap eLineCap)
{
    const bool bIsHairline = (rLineWidth.getX() == rLineWidth.getY()) && (rLineWidth.getX() <= 1.2);

    // #i101491#
    if( !bIsHairline && (rPolygon.count() > 1000) )
    {
        // the used basegfx::tools::createAreaGeometry is simply too
        // expensive with very big polygons; fallback to caller (who
        // should use ImplLineConverter normally)
        // AW: ImplLineConverter had to be removed since it does not even
        // know LineJoins, so the fallback will now prepare the line geometry
        // the same way.
        return false;
    }

    // temporarily adjust brush color to pen color
    // since the line is drawn as an area-polygon
    const SalColor aKeepBrushColor = mnBrushColor;
    mnBrushColor = mnPenColor;

    // #i11575#desc5#b adjust B2D tesselation result to raster positions
    basegfx::B2DPolygon aPolygon = rPolygon;
    const double fHalfWidth = 0.5 * rLineWidth.getX();

    // #i122456# This is probably thought to happen to align hairlines to pixel positions, so
    // it should be a 0.5 translation, not more. It will definitely go wrong with fat lines
    aPolygon.transform( basegfx::tools::createTranslateB2DHomMatrix(0.5, 0.5) );

    // shortcut for hairline drawing to improve performance
    bool bDrawnOk = true;
    if( bIsHairline )
    {
        // hairlines can benefit from a simplified tesselation
        // e.g. for hairlines the linejoin style can be ignored
        basegfx::B2DTrapezoidVector aB2DTrapVector;
        basegfx::tools::createLineTrapezoidFromB2DPolygon( aB2DTrapVector, aPolygon, rLineWidth.getX() );

        // draw tesselation result
        const int nTrapCount = aB2DTrapVector.size();
        if( nTrapCount > 0 )
            bDrawnOk = drawFilledTrapezoids( &aB2DTrapVector[0], nTrapCount, fTransparency );

        // restore the original brush GC
        mnBrushColor = aKeepBrushColor;
        return bDrawnOk;
    }

    // get the area polygon for the line polygon
    if( (rLineWidth.getX() != rLineWidth.getY())
    && !basegfx::fTools::equalZero( rLineWidth.getY() ) )
    {
        // prepare for createAreaGeometry() with anisotropic linewidth
        aPolygon.transform( basegfx::tools::createScaleB2DHomMatrix(1.0, rLineWidth.getX() / rLineWidth.getY()));
    }

    // create the area-polygon for the line
    const basegfx::B2DPolyPolygon aAreaPolyPoly( basegfx::tools::createAreaGeometry(aPolygon, fHalfWidth, eLineJoin, eLineCap) );

    if( (rLineWidth.getX() != rLineWidth.getY())
    && !basegfx::fTools::equalZero( rLineWidth.getX() ) )
    {
        // postprocess createAreaGeometry() for anisotropic linewidth
        aPolygon.transform(basegfx::tools::createScaleB2DHomMatrix(1.0, rLineWidth.getY() / rLineWidth.getX()));
    }

    // draw each area polypolygon component individually
    // to emulate the polypolygon winding rule "non-zero"
    const int nPolyCount = aAreaPolyPoly.count();
    for( int nPolyIdx = 0; nPolyIdx < nPolyCount; ++nPolyIdx )
    {
        const ::basegfx::B2DPolyPolygon aOnePoly( aAreaPolyPoly.getB2DPolygon( nPolyIdx ) );
        bDrawnOk = drawPolyPolygon( aOnePoly, fTransparency );
        if( !bDrawnOk )
            break;
    }

    // restore the original brush GC
    mnBrushColor = aKeepBrushColor;
    return bDrawnOk;
}

SalColor X11SalGraphicsImpl::getPixel( long nX, long nY )
{
    if( mrParent.bWindow_ && !mrParent.bVirDev_ )
    {
        XWindowAttributes aAttrib;

        XGetWindowAttributes( mrParent.GetXDisplay(), mrParent.GetDrawable(), &aAttrib );
        if( aAttrib.map_state != IsViewable )
        {
            stderr0( "X11SalGraphics::GetPixel drawable not viewable\n" );
            return 0;
        }
    }

    XImage *pXImage = XGetImage( mrParent.GetXDisplay(),
                                     mrParent.GetDrawable(),
                                 nX, nY,
                                 1,  1,
                                 AllPlanes,
                                 ZPixmap );
    if( !pXImage )
    {
        stderr0( "X11SalGraphics::GetPixel !XGetImage()\n" );
        return 0;
    }

    XColor aXColor;

    aXColor.pixel = XGetPixel( pXImage, 0, 0 );
    XDestroyImage( pXImage );

    return mrParent.GetColormap().GetColor( aXColor.pixel );
}

SalBitmap *X11SalGraphicsImpl::getBitmap( long nX, long nY, long nDX, long nDY )
{
    if( mrParent.bPrinter_ && !mrParent.bVirDev_ )
        return NULL;

    bool bFakeWindowBG = false;

    // normalize
    if( nDX < 0 )
    {
        nX += nDX;
        nDX = -nDX;
    }
    if ( nDY < 0 )
    {
        nY += nDY;
        nDY = -nDY;
    }

    if( mrParent.bWindow_ && !mrParent.bVirDev_ )
    {
        XWindowAttributes aAttrib;

        XGetWindowAttributes( mrParent.GetXDisplay(), mrParent.GetDrawable(), &aAttrib );
        if( aAttrib.map_state != IsViewable )
            bFakeWindowBG = true;
        else
        {
            long nOrgDX = nDX, nOrgDY = nDY;

            // clip to window size
            if ( nX < 0 )
            {
                nDX += nX;
                nX   = 0;
            }
            if ( nY < 0 )
            {
                nDY += nY;
                nY   = 0;
            }
            if( nX + nDX > aAttrib.width )
                nDX = aAttrib.width  - nX;
            if( nY + nDY > aAttrib.height )
                nDY = aAttrib.height - nY;

            // inside ?
            if( nDX <= 0 || nDY <= 0 )
            {
                bFakeWindowBG = true;
                nDX = nOrgDX;
                nDY = nOrgDY;
            }
        }
    }

    X11SalBitmap* pSalBitmap = new X11SalBitmap;
    sal_uInt16 nBitCount = GetBitCount();

    if( &mrParent.GetDisplay()->GetColormap( mrParent.m_nXScreen ) != &mrParent.GetColormap() )
        nBitCount = 1;

    if( ! bFakeWindowBG )
        pSalBitmap->ImplCreateFromDrawable( mrParent.GetDrawable(), mrParent.m_nXScreen, nBitCount, nX, nY, nDX, nDY );
    else
        pSalBitmap->Create( Size( nDX, nDY ), (nBitCount > 8) ? 24 : nBitCount, BitmapPalette( nBitCount > 8 ? nBitCount : 0 ) );

    return pSalBitmap;
}

sal_uInt16 X11SalGraphicsImpl::GetBitCount() const
{
    return mrParent.GetVisual().GetDepth();
}

long X11SalGraphicsImpl::GetGraphicsWidth() const
{
    if( mrParent.m_pFrame )
        return mrParent.m_pFrame->maGeometry.nWidth;
    else if( mrParent.m_pVDev )
        return static_cast< X11SalVirtualDevice* >(mrParent.m_pVDev)->GetWidth();
    else
        return 0;
}

bool X11SalGraphicsImpl::drawGradient(const tools::PolyPolygon& /*rPolygon*/, const Gradient& /*rGradient*/)
{
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
