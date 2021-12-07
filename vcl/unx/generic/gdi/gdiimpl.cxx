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

#include <memory>
#include <numeric>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrender.h>
#include <X11/Xproto.h>

#include "gdiimpl.hxx"

#include <vcl/gradient.hxx>
#include <sal/log.hxx>

#include <unx/saldisp.hxx>
#include <unx/salbmp.h>
#include <unx/salgdi.h>
#include <unx/salvd.h>
#include <unx/x11/xlimits.hxx>
#include <salframe.hxx>
#include <unx/x11/xrender_peer.hxx>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dtrapezoid.hxx>
#include <basegfx/utils/systemdependentdata.hxx>

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

/* From <X11/Intrinsic.h> */
typedef unsigned long Pixel;

class SalPolyLine
{
    std::vector<XPoint> Points_;
public:
    SalPolyLine(sal_uLong nPoints, const Point *p)
        : Points_(nPoints+1)
    {
        for (sal_uLong i = 0; i < nPoints; ++i)
        {
            Points_[i].x = static_cast<short>(p[i].getX());
            Points_[i].y = static_cast<short>(p[i].getY());
        }
        Points_[nPoints] = Points_[0]; // close polyline
    }

    const XPoint &operator[](sal_uLong n) const
    {
        return Points_[n];
    }

    XPoint &operator[](sal_uLong n)
    {
        return Points_[n];
    }
};

namespace
{
    void setForeBack(XGCValues& rValues, const SalColormap& rColMap, const SalBitmap& rSalBitmap)
    {
        rValues.foreground = rColMap.GetWhitePixel();
        rValues.background = rColMap.GetBlackPixel();

        //fdo#33455 and fdo#80160 handle 1 bit depth pngs with palette entries
        //to set fore/back colors
        SalBitmap& rBitmap = const_cast<SalBitmap&>(rSalBitmap);
        BitmapBuffer* pBitmapBuffer = rBitmap.AcquireBuffer(BitmapAccessMode::Read);
        if (!pBitmapBuffer)
            return;

        const BitmapPalette& rPalette = pBitmapBuffer->maPalette;
        if (rPalette.GetEntryCount() == 2)
        {
            const BitmapColor aWhite(rPalette[rPalette.GetBestIndex(COL_WHITE)]);
            rValues.foreground = rColMap.GetPixel(aWhite);

            const BitmapColor aBlack(rPalette[rPalette.GetBestIndex(COL_BLACK)]);
            rValues.background = rColMap.GetPixel(aBlack);
        }
        rBitmap.ReleaseBuffer(pBitmapBuffer, BitmapAccessMode::Read);
    }
}

X11SalGraphicsImpl::X11SalGraphicsImpl(X11SalGraphics& rParent):
    mrParent(rParent),
    mnBrushColor( 0xFF, 0xFF, 0XFF ),
    mpBrushGC(nullptr),
    mnBrushPixel(0),
    mbPenGC(false),
    mbBrushGC(false),
    mbCopyGC(false),
    mbInvertGC(false),
    mbInvert50GC(false),
    mbStippleGC(false),
    mbTrackingGC(false),
    mbDitherBrush(false),
    mbXORMode(false),
    mpPenGC(nullptr),
    mnPenColor( 0x00, 0x00, 0x00 ),
    mnPenPixel(0),
    mpMonoGC(nullptr),
    mpCopyGC(nullptr),
    mpMaskGC(nullptr),
    mpInvertGC(nullptr),
    mpInvert50GC(nullptr),
    mpStippleGC(nullptr),
    mpTrackingGC(nullptr)
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
        mrParent.m_aXRenderPicture = rRenderPeer.CreatePicture( mrParent.hDrawable_, pXRenderFormat, 0, nullptr );
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

static void freeGC(Display *pDisplay, GC& rGC)
{
    if( rGC )
    {
        XFreeGC( pDisplay, rGC );
        rGC = None;
    }
}

void X11SalGraphicsImpl::freeResources()
{
    Display *pDisplay = mrParent.GetXDisplay();

    freeGC( pDisplay, mpPenGC );
    freeGC( pDisplay, mpBrushGC );
    freeGC( pDisplay, mpMonoGC );
    freeGC( pDisplay, mpTrackingGC );
    freeGC( pDisplay, mpCopyGC );
    freeGC( pDisplay, mpMaskGC );
    freeGC( pDisplay, mpInvertGC );
    freeGC( pDisplay, mpInvert50GC );
    freeGC( pDisplay, mpStippleGC );
    mbTrackingGC = mbPenGC = mbBrushGC = mbCopyGC = mbInvertGC = mbInvert50GC = mbStippleGC = false;
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
        const char dash_list[2] = {2, 2};
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
        unsigned long const nValueMask =
                                  GCGraphicsExposures
                                  | GCForeground
                                  | GCBackground
                                  | GCFunction
                                  | GCLineWidth
                                  | GCLineStyle
                                  | GCFillStyle
                                  | GCStipple;

        values.fill_style           = FillStippled;
        values.stipple              = mrParent.GetDisplay()->GetInvert50( mrParent.m_nXScreen );

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

    SAL_WARN_IF( mnBrushColor == SALCOLOR_NONE, "vcl", "Brush Transparent" );

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

void X11SalGraphicsImpl::DrawLines(sal_uInt32              nPoints,
                                   const SalPolyLine &rPoints,
                                   GC                 pGC,
                                   bool               bClose)
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
                    const_cast<XPoint*>(&rPoints[n]),
                    nMaxLines,
                    CoordModeOrigin );

    if( n < nPoints )
        XDrawLines( mrParent.GetXDisplay(),
                    mrParent.GetDrawable(),
                    pGC,
                    const_cast<XPoint*>(&rPoints[n]),
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
        std::shared_ptr<SalBitmap> xDDB(pSrcGraphics->getBitmap( rPosAry.mnSrcX,
                                                                   rPosAry.mnSrcY,
                                                                   rPosAry.mnSrcWidth,
                                                                   rPosAry.mnSrcHeight ));

        if( !xDDB )
        {
            SAL_WARN( "vcl", "SalGraphics::CopyBits !pSrcGraphics->GetBitmap()" );
            return;
        }

        SalTwoRect aPosAry( rPosAry );

        aPosAry.mnSrcX = 0;
        aPosAry.mnSrcY = 0;
        drawBitmap( aPosAry, *xDDB );
    }
    else {
        SAL_WARN( "vcl", "X11SalGraphicsImpl::CopyBits from Printer not yet implemented" );
    }
}

void X11SalGraphicsImpl::copyArea ( tools::Long nDestX,    tools::Long nDestY,
                                tools::Long nSrcX,     tools::Long nSrcY,
                                tools::Long nSrcWidth, tools::Long nSrcHeight,
                                bool /*bWindowInvalidate*/)
{
    SalTwoRect aPosAry(nSrcX, nSrcY, nSrcWidth, nSrcHeight, nDestX, nDestY, nSrcWidth, nSrcHeight);
    copyBits(aPosAry, nullptr);
}

void X11SalGraphicsImpl::drawBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap )
{
    const SalDisplay*   pSalDisp = mrParent.GetDisplay();
    Display*            pXDisp = pSalDisp->GetDisplay();
    const Drawable      aDrawable( mrParent.GetDrawable() );
    const SalColormap&  rColMap = pSalDisp->GetColormap( mrParent.m_nXScreen );
    const tools::Long          nDepth = mrParent.GetDisplay()->GetVisual( mrParent.m_nXScreen ).GetDepth();
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
    // decide if alpha masking or transparency masking is needed
    BitmapBuffer* pAlphaBuffer = const_cast<SalBitmap&>(rMaskBitmap).AcquireBuffer( BitmapAccessMode::Read );
    if( pAlphaBuffer != nullptr )
    {
        ScanlineFormat nMaskFormat = pAlphaBuffer->mnFormat;
        const_cast<SalBitmap&>(rMaskBitmap).ReleaseBuffer( pAlphaBuffer, BitmapAccessMode::Read );
        if( nMaskFormat == ScanlineFormat::N8BitPal )
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
        aValues.function = GXand;
        aValues.foreground = 0x00000000;
        aValues.background = 0xffffffff;
        XChangeGC( pXDisp, aTmpGC, nValues, &aValues );
        static_cast<const X11SalBitmap&>(rTransBitmap).ImplDraw( aFG, mrParent.m_nXScreen, 1, aTmpRect, aTmpGC );

        DBG_TESTTRANS( aFG );

        // #105055# For XOR mode, keep background behind bitmap intact
        if( !mbXORMode )
        {
            // mask out background in pixmap #2 (nontransparent areas 0)
            aValues.function = GXand;
            aValues.foreground = 0xffffffff;
            aValues.background = 0x00000000;
            XChangeGC( pXDisp, aTmpGC, nValues, &aValues );
            static_cast<const X11SalBitmap&>(rTransBitmap).ImplDraw( aBG, mrParent.m_nXScreen, 1, aTmpRect, aTmpGC );

            DBG_TESTTRANS( aBG );
        }

        // merge pixmap #1 and pixmap #2 in pixmap #2
        aValues.function = GXxor;
        aValues.foreground = 0xffffffff;
        aValues.background = 0x00000000;
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
    // #i75531# the workaround below can go when
    // X11SalGraphics::drawAlphaBitmap()'s render acceleration
    // can handle the bitmap depth mismatch directly
    if( rSrcBitmap.GetBitCount() < rAlphaBmp.GetBitCount() )
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
    Picture aSrcPic = rPeer.CreatePicture( aSrcPM, pSrcVisFmt, 0, nullptr );
    if( !aSrcPic )
        return false;

    // create alpha Picture

    // TODO: use SalX11Bitmap functionality and caching for the Alpha Pixmap
    // problem is that they don't provide an 8bit Pixmap on a non-8bit display
    BitmapBuffer* pAlphaBuffer = const_cast<SalBitmap&>(rAlphaBmp).AcquireBuffer( BitmapAccessMode::Read );

    // an XImage needs its data top_down
    // TODO: avoid wrongly oriented images in upper layers!
    const int nImageSize = pAlphaBuffer->mnHeight * pAlphaBuffer->mnScanlineSize;
    const char* pSrcBits = reinterpret_cast<char*>(pAlphaBuffer->mpBits);
    char* pAlphaBits = new char[ nImageSize ];
    if( pAlphaBuffer->mnFormat & ScanlineFormat::TopDown )
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
    tools::Long* pLDst = reinterpret_cast<long*>(pAlphaBits);
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

    const_cast<SalBitmap&>(rAlphaBmp).ReleaseBuffer( pAlphaBuffer, BitmapAccessMode::Read );

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
        rTR.mnSrcX, rTR.mnSrcY,
        rTR.mnDestX, rTR.mnDestY, rTR.mnDestWidth, rTR.mnDestHeight );

    rPeer.FreePicture( aAlphaPic );
    XFreePixmap(pXDisplay, aAlphaPM);
    rPeer.FreePicture( aSrcPic );
    return true;
}

bool X11SalGraphicsImpl::drawTransformedBitmap(
    const basegfx::B2DPoint&,
    const basegfx::B2DPoint&,
    const basegfx::B2DPoint&,
    const SalBitmap&,
    const SalBitmap*,
    double)
{
    // here direct support for transformed bitmaps can be implemented
    return false;
}

bool X11SalGraphicsImpl::drawAlphaRect( tools::Long nX, tools::Long nY, tools::Long nWidth,
                                    tools::Long nHeight, sal_uInt8 nTransparency )
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
                               Color nMaskColor )
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
        aValues.foreground = 1;
        aValues.background = 0;
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
    if( !mrParent.mpClipRegion )
        return;

    mbPenGC         = false;
    mbBrushGC       = false;
    mbCopyGC        = false;
    mbInvertGC      = false;
    mbInvert50GC    = false;
    mbStippleGC     = false;
    mbTrackingGC    = false;

    XDestroyRegion( mrParent.mpClipRegion );
    mrParent.mpClipRegion    = nullptr;
}

bool X11SalGraphicsImpl::setClipRegion( const vcl::Region& i_rClip )
{
    if( mrParent.mpClipRegion )
        XDestroyRegion( mrParent.mpClipRegion );
    mrParent.mpClipRegion = XCreateRegion();

    RectangleVector aRectangles;
    i_rClip.GetRegionRectangles(aRectangles);

    for (auto const& rectangle : aRectangles)
    {
        const tools::Long nW(rectangle.GetWidth());

        if(nW)
        {
            const tools::Long nH(rectangle.GetHeight());

            if(nH)
            {
                XRectangle aRect;

                aRect.x = static_cast<short>(rectangle.Left());
                aRect.y = static_cast<short>(rectangle.Top());
                aRect.width = static_cast<unsigned short>(nW);
                aRect.height = static_cast<unsigned short>(nH);
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
    mbBrushGC       = false;
    mbCopyGC        = false;
    mbInvertGC      = false;
    mbInvert50GC    = false;
    mbStippleGC     = false;
    mbTrackingGC    = false;

    if( XEmptyRegion( mrParent.mpClipRegion ) )
    {
        XDestroyRegion( mrParent.mpClipRegion );
        mrParent.mpClipRegion= nullptr;
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

void X11SalGraphicsImpl::SetLineColor( Color nColor )
{
    if( mnPenColor != nColor )
    {
        mnPenColor      = nColor;
        mnPenPixel      = mrParent.GetPixel( nColor );
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

void X11SalGraphicsImpl::SetFillColor( Color nColor )
{
    if( mnBrushColor == nColor )
        return;

    mbDitherBrush   = false;
    mnBrushColor    = nColor;
    mnBrushPixel    = mrParent.GetPixel( nColor );
    if( TrueColor != mrParent.GetColormap().GetVisual().GetClass()
        && mrParent.GetColormap().GetColor( mnBrushPixel ) != mnBrushColor
        && nColor != Color( 0x00, 0x00, 0x00 ) // black
        && nColor != Color( 0x00, 0x00, 0x80 ) // blue
        && nColor != Color( 0x00, 0x80, 0x00 ) // green
        && nColor != Color( 0x00, 0x80, 0x80 ) // cyan
        && nColor != Color( 0x80, 0x00, 0x00 ) // red
        && nColor != Color( 0x80, 0x00, 0x80 ) // magenta
        && nColor != Color( 0x80, 0x80, 0x00 ) // brown
        && nColor != Color( 0x80, 0x80, 0x80 ) // gray
        && nColor != Color( 0xC0, 0xC0, 0xC0 ) // light gray
        && nColor != Color( 0x00, 0x00, 0xFF ) // light blue
        && nColor != Color( 0x00, 0xFF, 0x00 ) // light green
        && nColor != Color( 0x00, 0xFF, 0xFF ) // light cyan
        && nColor != Color( 0xFF, 0x00, 0x00 ) // light red
        && nColor != Color( 0xFF, 0x00, 0xFF ) // light magenta
        && nColor != Color( 0xFF, 0xFF, 0x00 ) // light brown
        && nColor != Color( 0xFF, 0xFF, 0xFF ) )
        mbDitherBrush = mrParent.GetDitherPixmap(nColor);
    mbBrushGC       = false;
}

void X11SalGraphicsImpl::SetROPLineColor( SalROPColor nROPColor )
{
    switch( nROPColor )
    {
        case SalROPColor::N0 : // 0
            mnPenPixel = Pixel(0);
            break;
        case SalROPColor::N1 : // 1
            mnPenPixel = static_cast<Pixel>(1 << mrParent.GetVisual().GetDepth()) - 1;
            break;
        case SalROPColor::Invert : // 2
            mnPenPixel = static_cast<Pixel>(1 << mrParent.GetVisual().GetDepth()) - 1;
            break;
    }
    mnPenColor  = mrParent.GetColormap().GetColor( mnPenPixel );
    mbPenGC     = false;
}

void X11SalGraphicsImpl::SetROPFillColor( SalROPColor nROPColor )
{
    switch( nROPColor )
    {
        case SalROPColor::N0 : // 0
            mnBrushPixel = Pixel(0);
            break;
        case SalROPColor::N1 : // 1
            mnBrushPixel = static_cast<Pixel>(1 << mrParent.GetVisual().GetDepth()) - 1;
            break;
        case SalROPColor::Invert : // 2
            mnBrushPixel = static_cast<Pixel>(1 << mrParent.GetVisual().GetDepth()) - 1;
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
        mbBrushGC   = false;
        mbCopyGC        = false;
        mbInvertGC  = false;
        mbInvert50GC    = false;
        mbStippleGC = false;
        mbTrackingGC    = false;
    }
}

void X11SalGraphicsImpl::drawPixel( tools::Long nX, tools::Long nY )
{
    if( mnPenColor !=  SALCOLOR_NONE )
        XDrawPoint( mrParent.GetXDisplay(), mrParent.GetDrawable(), SelectPen(), nX, nY );
}

void X11SalGraphicsImpl::drawPixel( tools::Long nX, tools::Long nY, Color nColor )
{
    if( nColor == SALCOLOR_NONE )
        return;

    Display *pDisplay = mrParent.GetXDisplay();

    if( (mnPenColor == SALCOLOR_NONE) && !mbPenGC )
    {
        SetLineColor( nColor );
        XDrawPoint( pDisplay, mrParent.GetDrawable(), SelectPen(), nX, nY );
        mnPenColor = SALCOLOR_NONE;
        mbPenGC = False;
    }
    else
    {
        GC pGC = SelectPen();

        if( nColor != mnPenColor )
            XSetForeground( pDisplay, pGC, mrParent.GetPixel( nColor ) );

        XDrawPoint( pDisplay, mrParent.GetDrawable(), pGC, nX, nY );

        if( nColor != mnPenColor )
            XSetForeground( pDisplay, pGC, mnPenPixel );
    }
}

void X11SalGraphicsImpl::drawLine( tools::Long nX1, tools::Long nY1, tools::Long nX2, tools::Long nY2 )
{
    if( mnPenColor != SALCOLOR_NONE )
    {
        XDrawLine( mrParent.GetXDisplay(), mrParent.GetDrawable(),SelectPen(),
                   nX1, nY1, nX2, nY2 );
    }
}

void X11SalGraphicsImpl::drawRect( tools::Long nX, tools::Long nY, tools::Long nDX, tools::Long nDY )
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

void X11SalGraphicsImpl::drawPolyLine( sal_uInt32 nPoints, const Point *pPtAry )
{
    drawPolyLine( nPoints, pPtAry, false );
}

void X11SalGraphicsImpl::drawPolyLine( sal_uInt32 nPoints, const Point *pPtAry, bool bClose )
{
    if( mnPenColor != SALCOLOR_NONE )
    {
        SalPolyLine Points( nPoints, pPtAry );

        DrawLines( nPoints, Points, SelectPen(), bClose );
    }
}

void X11SalGraphicsImpl::drawPolygon( sal_uInt32 nPoints, const Point* pPtAry )
{
    if( nPoints == 0 )
        return;

    if( nPoints < 3 )
    {
        if( !mbXORMode )
        {
            if( 1 == nPoints  )
                drawPixel( pPtAry[0].getX(), pPtAry[0].getY() );
            else
                drawLine( pPtAry[0].getX(), pPtAry[0].getY(),
                          pPtAry[1].getX(), pPtAry[1].getY() );
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
    if (nPoints == 5 &&
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
                                   const Point*  *pPtAry )
{
    if( mnBrushColor != SALCOLOR_NONE )
    {
        sal_uInt32      i, n;
        Region          pXRegA  = nullptr;

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

bool X11SalGraphicsImpl::drawPolyLineBezier( sal_uInt32, const Point*, const PolyFlags* )
{
    return false;
}

bool X11SalGraphicsImpl::drawPolygonBezier( sal_uInt32, const Point*, const PolyFlags* )
{
    return false;
}

bool X11SalGraphicsImpl::drawPolyPolygonBezier( sal_uInt32, const sal_uInt32*,
                                                const Point* const*, const PolyFlags* const* )
{
    return false;
}

void X11SalGraphicsImpl::invert( tools::Long       nX,
                                tools::Long        nY,
                                tools::Long        nDX,
                                tools::Long        nDY,
                                SalInvert   nFlags )
{
    GC pGC;
    if( SalInvert::N50 & nFlags )
    {
        pGC = GetInvert50GC();
        XFillRectangle( mrParent.GetXDisplay(), mrParent.GetDrawable(), pGC, nX, nY, nDX, nDY );
    }
    else
    {
        if ( SalInvert::TrackFrame & nFlags )
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
                             const Point* pPtAry,
                             SalInvert nFlags )
{
    SalPolyLine Points ( nPoints, pPtAry );

    GC pGC;
    if( SalInvert::N50 & nFlags )
        pGC = GetInvert50GC();
    else
        if ( SalInvert::TrackFrame & nFlags )
            pGC = GetTrackingGC();
        else
            pGC = GetInvertGC();

    if( SalInvert::TrackFrame & nFlags )
        DrawLines ( nPoints, Points, pGC, true );
    else
        XFillPolygon( mrParent.GetXDisplay(),
                      mrParent.GetDrawable(),
                      pGC,
                      &Points[0], nPoints,
                      Complex, CoordModeOrigin );
}

bool X11SalGraphicsImpl::drawEPS( tools::Long,tools::Long,tools::Long,tools::Long,void*,sal_uInt32 )
{
    return false;
}

// draw a poly-polygon
bool X11SalGraphicsImpl::drawPolyPolygon(
    const basegfx::B2DHomMatrix& rObjectToDevice,
    const basegfx::B2DPolyPolygon& rPolyPolygon,
    double fTransparency)
{
    // nothing to do for empty polypolygons
    const int nOrigPolyCount = rPolyPolygon.count();
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

    // Fallback: Transform to DeviceCoordinates
    basegfx::B2DPolyPolygon aPolyPolygon(rPolyPolygon);
    aPolyPolygon.transform(rObjectToDevice);

    // snap to raster if requested
    const bool bSnapToRaster = !mrParent.getAntiAlias();
    if( bSnapToRaster )
        aPolyPolygon = basegfx::utils::snapPointsOfHorizontalOrVerticalEdges( aPolyPolygon );

    // don't bother with polygons outside of visible area
    const basegfx::B2DRange aViewRange( 0, 0, GetGraphicsWidth(), GetGraphicsHeight() );
    aPolyPolygon = basegfx::utils::clipPolyPolygonOnRange( aPolyPolygon, aViewRange, true, false );
    if( !aPolyPolygon.count() )
        return true;

    // tessellate the polypolygon into trapezoids
    basegfx::B2DTrapezoidVector aB2DTrapVector;
    basegfx::utils::trapezoidSubdivide( aB2DTrapVector, aPolyPolygon );
    const int nTrapCount = aB2DTrapVector.size();
    if( !nTrapCount )
        return true;
    const bool bDrawn = drawFilledTrapezoids( aB2DTrapVector.data(), nTrapCount, fTransparency );
    return bDrawn;
}

tools::Long X11SalGraphicsImpl::GetGraphicsHeight() const
{
    if( mrParent.m_pFrame )
        return mrParent.m_pFrame->maGeometry.nHeight;
    else if( mrParent.m_pVDev )
        return static_cast< X11SalVirtualDevice* >(mrParent.m_pVDev)->GetHeight();
    else
        return 0;
}

bool X11SalGraphicsImpl::drawFilledTrapezoids( const basegfx::B2DTrapezoid* pB2DTraps, int nTrapCount, double fTransparency )
{
    if( nTrapCount <= 0 )
        return true;

    Picture aDstPic = GetXRenderPicture();
    // check xrender support for this drawable
    if( !aDstPic )
        return false;

     // convert the B2DTrapezoids into XRender-Trapezoids
    std::vector<XTrapezoid> aTrapVector( nTrapCount );
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
        rEntry.m_aPicture, aDstPic, pMaskFormat, 0, 0, aTrapVector.data(), aTrapVector.size() );

    return true;
}

bool X11SalGraphicsImpl::drawFilledTriangles(
    const basegfx::B2DHomMatrix& rObjectToDevice,
     const basegfx::triangulator::B2DTriangleVector& rTriangles,
     double fTransparency)
{
    if(rTriangles.empty())
        return true;

    Picture aDstPic = GetXRenderPicture();
    // check xrender support for this drawable
    if( !aDstPic )
    {
        return false;
    }

    // prepare transformation for ObjectToDevice coordinate system
    basegfx::B2DHomMatrix aObjectToDevice = basegfx::utils::createTranslateB2DHomMatrix(0.5, 0.5) * rObjectToDevice;

     // convert the Triangles into XRender-Triangles
    std::vector<XTriangle> aTriVector(rTriangles.size());
    sal_uInt32 nIndex(0);

    for(const auto& rCandidate : rTriangles)
    {
        const basegfx::B2DPoint aP1(aObjectToDevice * rCandidate.getA());
        const basegfx::B2DPoint aP2(aObjectToDevice * rCandidate.getB());
        const basegfx::B2DPoint aP3(aObjectToDevice * rCandidate.getC());
        XTriangle& rTri(aTriVector[nIndex++]);

        rTri.p1.x = XDoubleToFixed(aP1.getX());
        rTri.p1.y = XDoubleToFixed(aP1.getY());

        rTri.p2.x = XDoubleToFixed(aP2.getX());
        rTri.p2.y = XDoubleToFixed(aP2.getY());

        rTri.p3.x = XDoubleToFixed(aP3.getX());
        rTri.p3.y = XDoubleToFixed(aP3.getY());
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
    rRenderPeer.CompositeTriangles( PictOpOver,
        rEntry.m_aPicture, aDstPic, pMaskFormat, 0, 0, aTriVector.data(), aTriVector.size() );

    return true;
}

namespace {

class SystemDependentData_Triangulation : public basegfx::SystemDependentData
{
private:
    // the triangulation itself
    basegfx::triangulator::B2DTriangleVector    maTriangles;

    // all other values the triangulation is based on and
    // need to be compared with to check for data validity
    double                                      mfLineWidth;
    basegfx::B2DLineJoin                        meJoin;
    css::drawing::LineCap                       meCap;
    double                                      mfMiterMinimumAngle;
    std::vector< double >                       maStroke;

public:
    SystemDependentData_Triangulation(
        basegfx::SystemDependentDataManager& rSystemDependentDataManager,
        basegfx::triangulator::B2DTriangleVector&& rTriangles,
        double fLineWidth,
        basegfx::B2DLineJoin eJoin,
        css::drawing::LineCap eCap,
        double fMiterMinimumAngle,
        const std::vector< double >* pStroke); // MM01

    // read access
    const basegfx::triangulator::B2DTriangleVector& getTriangles() const { return maTriangles; }
    double getLineWidth() const { return mfLineWidth; }
    const basegfx::B2DLineJoin& getJoin() const { return meJoin; }
    const css::drawing::LineCap& getCap() const { return meCap; }
    double getMiterMinimumAngle() const { return mfMiterMinimumAngle; }
    const std::vector< double >& getStroke() const { return maStroke; }

    virtual sal_Int64 estimateUsageInBytes() const override;
};

}

SystemDependentData_Triangulation::SystemDependentData_Triangulation(
    basegfx::SystemDependentDataManager& rSystemDependentDataManager,
    basegfx::triangulator::B2DTriangleVector&& rTriangles,
    double fLineWidth,
    basegfx::B2DLineJoin eJoin,
    css::drawing::LineCap eCap,
    double fMiterMinimumAngle,
    const std::vector< double >* pStroke)
:   basegfx::SystemDependentData(rSystemDependentDataManager),
    maTriangles(std::move(rTriangles)),
    mfLineWidth(fLineWidth),
    meJoin(eJoin),
    meCap(eCap),
    mfMiterMinimumAngle(fMiterMinimumAngle)
{
    if(nullptr != pStroke)
    {
        maStroke = *pStroke;
    }
}

sal_Int64 SystemDependentData_Triangulation::estimateUsageInBytes() const
{
    sal_Int64 nRetval(0);

    if(!maTriangles.empty())
    {
        nRetval = maTriangles.size() * sizeof(basegfx::triangulator::B2DTriangle);
    }

    return nRetval;
}

bool X11SalGraphicsImpl::drawPolyLine(
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
    // short circuit if there is nothing to do
    if(0 == rPolygon.count() || fTransparency < 0.0 || fTransparency >= 1.0)
    {
        return true;
    }

    // need to check/handle LineWidth when ObjectToDevice transformation is used
    const bool bObjectToDeviceIsIdentity(rObjectToDevice.isIdentity());
    basegfx::B2DHomMatrix aObjectToDeviceInv;

    // tdf#124848 calculate-back logical LineWidth for a hairline.
    // This implementation does not hand over the transformation to
    // the graphic sub-system, but the triangulation data is prepared
    // view-independent based on the logic LineWidth, so we need to
    // know it
    if(fLineWidth == 0)
    {
        fLineWidth = 1.0;

        if(!bObjectToDeviceIsIdentity)
        {
            if(aObjectToDeviceInv.isIdentity())
            {
                aObjectToDeviceInv = rObjectToDevice;
                aObjectToDeviceInv.invert();
            }

            fLineWidth = (aObjectToDeviceInv * basegfx::B2DVector(fLineWidth, 0)).getLength();
        }
    }

    // try to access buffered data
    std::shared_ptr<SystemDependentData_Triangulation> pSystemDependentData_Triangulation(
        rPolygon.getSystemDependentData<SystemDependentData_Triangulation>());

    // MM01 need to do line dashing as fallback stuff here now
    const double fDotDashLength(nullptr != pStroke ? std::accumulate(pStroke->begin(), pStroke->end(), 0.0) : 0.0);
    const bool bStrokeUsed(0.0 != fDotDashLength);
    assert(!bStrokeUsed || (bStrokeUsed && pStroke));

    if(pSystemDependentData_Triangulation)
    {
        // MM01 - check on stroke change. Used against not used, or if oth used,
        // equal or different? Triangulation geometry creation depends heavily
        // on stroke, independent of being transformation independent
        const bool bStrokeWasUsed(!pSystemDependentData_Triangulation->getStroke().empty());

        if(bStrokeWasUsed != bStrokeUsed
        || (bStrokeUsed && *pStroke != pSystemDependentData_Triangulation->getStroke()))
        {
            // data invalid, forget
            pSystemDependentData_Triangulation.reset();
        }
    }

    if(pSystemDependentData_Triangulation)
    {
        // check data validity (I)
        if(pSystemDependentData_Triangulation->getJoin() != eLineJoin
        || pSystemDependentData_Triangulation->getCap() != eLineCap
        || pSystemDependentData_Triangulation->getMiterMinimumAngle() != fMiterMinimumAngle)
        {
            // data invalid, forget
            pSystemDependentData_Triangulation.reset();
        }
    }

    if(pSystemDependentData_Triangulation)
    {
        // check data validity (II)
        if(pSystemDependentData_Triangulation->getLineWidth() != fLineWidth)
        {
            // sometimes small inconsistencies, use a percentage tolerance
            const double fFactor(basegfx::fTools::equalZero(fLineWidth)
                ? 0.0
                : fabs(1.0 - (pSystemDependentData_Triangulation->getLineWidth() / fLineWidth)));
            // compare with 5.0% tolerance
            if(basegfx::fTools::more(fFactor, 0.05))
            {
                // data invalid, forget
                pSystemDependentData_Triangulation.reset();
            }
        }
    }

    if(!pSystemDependentData_Triangulation)
    {
        // MM01 need to do line dashing as fallback stuff here now
        basegfx::B2DPolyPolygon aPolyPolygonLine;

        if(bStrokeUsed)
        {
            // apply LineStyle
            basegfx::utils::applyLineDashing(
                rPolygon, // source
                *pStroke, // pattern
                &aPolyPolygonLine, // target for lines
                nullptr, // target for gaps
                fDotDashLength); // full length if available
        }
        else
        {
            // no line dashing, just copy
            aPolyPolygonLine.append(rPolygon);
        }

        // try to create data
        if(bPixelSnapHairline)
        {
            // Do NOT transform, but keep device-independent. To
            // do so, transform to device for snap, but back again after
            if(!bObjectToDeviceIsIdentity)
            {
                aPolyPolygonLine.transform(rObjectToDevice);
            }

            aPolyPolygonLine = basegfx::utils::snapPointsOfHorizontalOrVerticalEdges(aPolyPolygonLine);

            if(!bObjectToDeviceIsIdentity)
            {
                if(aObjectToDeviceInv.isIdentity())
                {
                    aObjectToDeviceInv = rObjectToDevice;
                    aObjectToDeviceInv.invert();
                }

                aPolyPolygonLine.transform(aObjectToDeviceInv);
            }
        }

        basegfx::triangulator::B2DTriangleVector aTriangles;

        // MM01 checked/verified for X11 (linux)
        for(sal_uInt32 a(0); a < aPolyPolygonLine.count(); a++)
        {
            const basegfx::B2DPolygon aPolyLine(aPolyPolygonLine.getB2DPolygon(a));
            // MM01 upps - commit 51b5b93092d6231615de470c62494c24e54828a1 removed
            // this *central* geometry-creating lines (!) probably due to aAreaPolyPoly
            // *not* being used - that's true, but the work is inside of filling
            // aTriangles data (!)
            basegfx::utils::createAreaGeometry(
                aPolyLine,
                0.5 * fLineWidth,
                eLineJoin,
                eLineCap,
                basegfx::deg2rad(12.5),
                0.4,
                fMiterMinimumAngle,
                &aTriangles); // CAUTION! This is *needed* since it creates the data!
        }

        if(!aTriangles.empty())
        {
            // Add to buffering mechanism
            // Add all values the triangulation is based off, too, to check for
            // validity (see above)
            pSystemDependentData_Triangulation = rPolygon.addOrReplaceSystemDependentData<SystemDependentData_Triangulation>(
                ImplGetSystemDependentDataManager(),
                std::move(aTriangles),
                fLineWidth,
                eLineJoin,
                eLineCap,
                fMiterMinimumAngle,
                pStroke);
        }
    }

    if(!pSystemDependentData_Triangulation)
    {
        return false;
    }

    // temporarily adjust brush color to pen color
    // since the line is drawn as an area-polygon
    const Color aKeepBrushColor = mnBrushColor;
    mnBrushColor = mnPenColor;

    // create the area-polygon for the line
    const bool bDrawnOk(
        drawFilledTriangles(
            rObjectToDevice,
            pSystemDependentData_Triangulation->getTriangles(),
            fTransparency));

    // restore the original brush GC
    mnBrushColor = aKeepBrushColor;
    return bDrawnOk;
}

Color X11SalGraphicsImpl::getPixel( tools::Long nX, tools::Long nY )
{
    if( mrParent.bWindow_ && !mrParent.bVirDev_ )
    {
        XWindowAttributes aAttrib;

        XGetWindowAttributes( mrParent.GetXDisplay(), mrParent.GetDrawable(), &aAttrib );
        if( aAttrib.map_state != IsViewable )
        {
            SAL_WARN( "vcl", "X11SalGraphics::GetPixel drawable not viewable" );
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
        SAL_WARN( "vcl", "X11SalGraphics::GetPixel !XGetImage()" );
        return 0;
    }

    XColor aXColor;

    aXColor.pixel = XGetPixel( pXImage, 0, 0 );
    XDestroyImage( pXImage );

    return mrParent.GetColormap().GetColor( aXColor.pixel );
}

std::shared_ptr<SalBitmap> X11SalGraphicsImpl::getBitmap( tools::Long nX, tools::Long nY, tools::Long nDX, tools::Long nDY )
{
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
            tools::Long nOrgDX = nDX, nOrgDY = nDY;

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

    std::shared_ptr<X11SalBitmap> pSalBitmap = std::make_shared<X11SalBitmap>();
    sal_uInt16 nBitCount = GetBitCount();
    vcl::PixelFormat ePixelFormat = vcl::bitDepthToPixelFormat(nBitCount);

    if( &mrParent.GetDisplay()->GetColormap( mrParent.m_nXScreen ) != &mrParent.GetColormap() )
    {
        ePixelFormat = vcl::PixelFormat::N1_BPP;
        nBitCount = 1;
    }

    if (nBitCount > 8)
        ePixelFormat = vcl::PixelFormat::N24_BPP;

    if( ! bFakeWindowBG )
        pSalBitmap->ImplCreateFromDrawable( mrParent.GetDrawable(), mrParent.m_nXScreen, nBitCount, nX, nY, nDX, nDY );
    else
        pSalBitmap->Create( Size( nDX, nDY ), ePixelFormat, BitmapPalette( nBitCount > 8 ? nBitCount : 0 ) );

    return pSalBitmap;
}

sal_uInt16 X11SalGraphicsImpl::GetBitCount() const
{
    return mrParent.GetVisual().GetDepth();
}

tools::Long X11SalGraphicsImpl::GetGraphicsWidth() const
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

bool X11SalGraphicsImpl::implDrawGradient(basegfx::B2DPolyPolygon const & /*rPolyPolygon*/, SalGradient const & /*rGradient*/)
{
    return false;
}

bool X11SalGraphicsImpl::supportsOperation(OutDevSupportType eType) const
{
    bool bRet = false;
    switch (eType)
    {
        case OutDevSupportType::TransparentRect:
        case OutDevSupportType::B2DDraw:
        {
            XRenderPeer& rPeer = XRenderPeer::GetInstance();
            const SalDisplay* pSalDisp = mrParent.GetDisplay();
            const SalVisual& rSalVis = pSalDisp->GetVisual(mrParent.GetScreenNumber());

            Visual* pDstXVisual = rSalVis.GetVisual();
            XRenderPictFormat* pDstVisFmt = rPeer.FindVisualFormat(pDstXVisual);
            if (pDstVisFmt)
                bRet = true;
        }
        break;
        default:
            break;
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
