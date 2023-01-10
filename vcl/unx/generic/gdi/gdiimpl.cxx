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
    moBrushColor( std::in_place, 0xFF, 0xFF, 0XFF ),
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
    moPenColor( std::in_place, 0x00, 0x00, 0x00 ),
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
    mnPenPixel = moPenColor ? mrParent.GetPixel(*moPenColor) : 0;
    mnBrushPixel = moBrushColor ? mrParent.GetPixel(*moBrushColor) : 0;
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
        mrParent.m_aXRenderPicture = rRenderPeer.CreatePicture( mrParent.GetDrawable(), pXRenderFormat, 0, nullptr );
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
    values.foreground           = mrParent.GetColormap().GetBlackPixel()
                                  ^ mrParent.GetColormap().GetWhitePixel();
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
        values.foreground           = mrParent.GetColormap().GetBlackPixel()
                                      ^ mrParent.GetColormap().GetWhitePixel();
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
        values.foreground           = mrParent.GetColormap().GetWhitePixel();
        values.background           = mrParent.GetColormap().GetBlackPixel();
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

    SAL_WARN_IF( !moBrushColor, "vcl", "Brush Transparent" );

    if( !mpBrushGC )
    {
        XGCValues values;
        values.subwindow_mode       = ClipByChildren;
        values.fill_rule            = EvenOddRule;      // Pict import/ Gradient
        values.graphics_exposures   = False;

        mpBrushGC = XCreateGC( pDisplay, mrParent.GetDrawable(),
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

        mpPenGC = XCreateGC( pDisplay, mrParent.GetDrawable(),
                             GCSubwindowMode | GCFillRule | GCGraphicsExposures,
                             &values );
    }

    if( !mbPenGC )
    {
        if( moPenColor )
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
            internalDrawLine( rPoints[nPoints-1].x, rPoints[nPoints-1].y, rPoints[0].x, rPoints[0].y );
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
    ImplSalDDB* pSrcDDB = rSrcX11Bmp.ImplGetDDB( mrParent.GetDrawable(), mrParent.m_nXScreen, nDepth, rTR );
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

    Pixmap aAlphaPM = limitXCreatePixmap( pXDisplay, mrParent.GetDrawable(),
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

void X11SalGraphicsImpl::setClipRegion( const vcl::Region& i_rClip )
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
}

void X11SalGraphicsImpl::SetLineColor()
{
    if( moPenColor )
    {
        moPenColor      = std::nullopt;
        mbPenGC         = false;
    }
}

void X11SalGraphicsImpl::SetLineColor( Color nColor )
{
    if( moPenColor != nColor )
    {
        moPenColor      = nColor;
        mnPenPixel      = mrParent.GetPixel( nColor );
        mbPenGC         = false;
    }
}

void X11SalGraphicsImpl::SetFillColor()
{
    if( moBrushColor )
    {
        mbDitherBrush   = false;
        moBrushColor    = std::nullopt;
        mbBrushGC       = false;
    }
}

void X11SalGraphicsImpl::SetFillColor( Color nColor )
{
    if( moBrushColor == nColor )
        return;

    mbDitherBrush   = false;
    moBrushColor    = nColor;
    mnBrushPixel    = mrParent.GetPixel( nColor );
    if( TrueColor != mrParent.GetColormap().GetVisual().GetClass()
        && mrParent.GetColormap().GetColor( mnBrushPixel ) != moBrushColor
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
    moPenColor  = mrParent.GetColormap().GetColor( mnPenPixel );
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
    moBrushColor    = mrParent.GetColormap().GetColor( mnBrushPixel );
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

void X11SalGraphicsImpl::internalDrawLine( tools::Long nX1, tools::Long nY1, tools::Long nX2, tools::Long nY2 )
{
    if( moPenColor )
    {
        XDrawLine( mrParent.GetXDisplay(), mrParent.GetDrawable(),SelectPen(),
                   nX1, nY1, nX2, nY2 );
    }
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

tools::Long X11SalGraphicsImpl::GetGraphicsHeight() const
{
    if( mrParent.m_pFrame )
        return mrParent.m_pFrame->maGeometry.height();
    else if( mrParent.m_pVDev )
        return static_cast< X11SalVirtualDevice* >(mrParent.m_pVDev)->GetHeight();
    else
        return 0;
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
        return mrParent.m_pFrame->maGeometry.width();
    else if( mrParent.m_pVDev )
        return static_cast< X11SalVirtualDevice* >(mrParent.m_pVDev)->GetWidth();
    else
        return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
