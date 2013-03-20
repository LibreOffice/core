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

#include "tools/debug.hxx"

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

#include "vcl/jobdata.hxx"

#include "unx/Xproto.h"
#include "unx/salunx.h"
#include "unx/saldata.hxx"
#include "unx/saldisp.hxx"
#include "unx/salgdi.h"
#include "unx/salframe.h"
#include "unx/salvd.h"
#include <unx/x11/xlimits.hxx>

#include "generic/printergfx.hxx"
#include "xrender_peer.hxx"
#include "region.h"

#include <vector>
#include <queue>
#include <set>

#include <config_graphite.h>

#define STATIC_POINTS 64

class SalPolyLine
{
            XPoint              Points_[STATIC_POINTS];
            XPoint             *pFirst_;
public:
    inline                      SalPolyLine( sal_uLong nPoints );
    inline                      SalPolyLine( sal_uLong nPoints, const SalPoint *p );
    inline                      ~SalPolyLine();
    inline  XPoint             &operator [] ( sal_uLong n ) const
                                { return pFirst_[n]; }
};

inline SalPolyLine::SalPolyLine( sal_uLong nPoints )
    : pFirst_( nPoints+1 > STATIC_POINTS ? new XPoint[nPoints+1] : Points_ )
{}

inline SalPolyLine::SalPolyLine( sal_uLong nPoints, const SalPoint *p )
    : pFirst_( nPoints+1 > STATIC_POINTS ? new XPoint[nPoints+1] : Points_ )
{
    for( sal_uLong i = 0; i < nPoints; i++ )
    {
        pFirst_[i].x = (short)p[i].mnX;
        pFirst_[i].y = (short)p[i].mnY;
    }
    pFirst_[nPoints] = pFirst_[0]; // close polyline
}

inline SalPolyLine::~SalPolyLine()
{ if( pFirst_ != Points_ ) delete [] pFirst_; }

#undef STATIC_POINTS

X11SalGraphics::X11SalGraphics()
    : m_nXScreen( 0 )
{
    m_pFrame            = NULL;
    m_pVDev             = NULL;
    m_pDeleteColormap   = NULL;
    hDrawable_          = None;
    m_aXRenderPicture    = 0;
    m_pXRenderFormat     = NULL;

    mpClipRegion            = NULL;
    pPaintRegion_       = NULL;

    pPenGC_         = NULL;
    nPenPixel_          = 0;
    nPenColor_          = MAKE_SALCOLOR( 0x00, 0x00, 0x00 ); // Black

    pFontGC_            = NULL;
    for( int i = 0; i < MAX_FALLBACK; ++i )
        mpServerFont[i] = NULL;

    nTextPixel_         = 0;
    nTextColor_         = MAKE_SALCOLOR( 0x00, 0x00, 0x00 ); // Black

#if ENABLE_GRAPHITE
    // check if graphite fonts have been disabled
    static const char* pDisableGraphiteStr = getenv( "SAL_DISABLE_GRAPHITE" );
    bDisableGraphite_       = pDisableGraphiteStr ? (pDisableGraphiteStr[0]!='0') : sal_False;
#endif

    pBrushGC_           = NULL;
    nBrushPixel_            = 0;
    nBrushColor_        = MAKE_SALCOLOR( 0xFF, 0xFF, 0xFF ); // White
    hBrush_             = None;

    pMonoGC_            = NULL;
    pCopyGC_            = NULL;
    pMaskGC_            = NULL;
    pInvertGC_          = NULL;
    pInvert50GC_        = NULL;
    pStippleGC_         = NULL;
    pTrackingGC_        = NULL;

    bWindow_            = sal_False;
    bPrinter_           = sal_False;
    bVirDev_            = sal_False;
    bPenGC_         = sal_False;
    bFontGC_            = sal_False;
    bBrushGC_           = sal_False;
    bMonoGC_            = sal_False;
    bCopyGC_            = sal_False;
    bInvertGC_          = sal_False;
    bInvert50GC_        = sal_False;
    bStippleGC_         = sal_False;
    bTrackingGC_        = sal_False;
    bXORMode_           = sal_False;
    bDitherBrush_       = sal_False;
}

X11SalGraphics::~X11SalGraphics()
{
    ReleaseFonts();
    freeResources();
}

void X11SalGraphics::freeResources()
{
    Display *pDisplay = GetXDisplay();

    DBG_ASSERT( !pPaintRegion_, "pPaintRegion_" );
    if( mpClipRegion ) XDestroyRegion( mpClipRegion ), mpClipRegion = None;

    if( hBrush_ )       XFreePixmap( pDisplay, hBrush_ ), hBrush_ = None;
    if( pPenGC_ )       XFreeGC( pDisplay, pPenGC_ ), pPenGC_ = None;
    if( pFontGC_ )      XFreeGC( pDisplay, pFontGC_ ), pFontGC_ = None;
    if( pBrushGC_ )     XFreeGC( pDisplay, pBrushGC_ ), pBrushGC_ = None;
    if( pMonoGC_ )      XFreeGC( pDisplay, pMonoGC_ ), pMonoGC_ = None;
    if( pCopyGC_ )      XFreeGC( pDisplay, pCopyGC_ ), pCopyGC_ = None;
    if( pMaskGC_ )      XFreeGC( pDisplay, pMaskGC_ ), pMaskGC_ = None;
    if( pInvertGC_ )    XFreeGC( pDisplay, pInvertGC_ ), pInvertGC_ = None;
    if( pInvert50GC_ )  XFreeGC( pDisplay, pInvert50GC_ ), pInvert50GC_ = None;
    if( pStippleGC_ )   XFreeGC( pDisplay, pStippleGC_ ), pStippleGC_ = None;
    if( pTrackingGC_ )  XFreeGC( pDisplay, pTrackingGC_ ), pTrackingGC_ = None;
    if( m_pDeleteColormap )
        delete m_pDeleteColormap, m_pColormap = m_pDeleteColormap = NULL;

    if( m_aXRenderPicture )
        XRenderPeer::GetInstance().FreePicture( m_aXRenderPicture ), m_aXRenderPicture = 0;

    bPenGC_ = bFontGC_ = bBrushGC_ = bMonoGC_ = bCopyGC_ = bInvertGC_ = bInvert50GC_ = bStippleGC_ = bTrackingGC_ = false;
}

void X11SalGraphics::SetDrawable( Drawable aDrawable, SalX11Screen nXScreen )
{
    // shortcut if nothing changed
    if( hDrawable_ == aDrawable )
        return;

    // free screen specific resources if needed
    if( nXScreen != m_nXScreen )
    {
        freeResources();
        m_pColormap = &GetGenericData()->GetSalDisplay()->GetColormap( nXScreen );
        m_nXScreen = nXScreen;
    }

    hDrawable_ = aDrawable;
    SetXRenderFormat( NULL );
    if( m_aXRenderPicture )
    {
        XRenderPeer::GetInstance().FreePicture( m_aXRenderPicture );
        m_aXRenderPicture = 0;
    }

    if( hDrawable_ )
    {
        nPenPixel_      = GetPixel( nPenColor_ );
        nTextPixel_     = GetPixel( nTextColor_ );
        nBrushPixel_    = GetPixel( nBrushColor_ );
    }
}

void X11SalGraphics::Init( SalFrame *pFrame, Drawable aTarget,
                           SalX11Screen nXScreen )
{
    m_pColormap = &GetGenericData()->GetSalDisplay()->GetColormap(nXScreen);
    m_nXScreen  = nXScreen;
    SetDrawable( aTarget, nXScreen );

    bWindow_    = sal_True;
    m_pFrame    = pFrame;
    m_pVDev     = NULL;
}

void X11SalGraphics::DeInit()
{
    SetDrawable( None, m_nXScreen );
}

void X11SalGraphics::SetClipRegion( GC pGC, XLIB_Region pXReg ) const
{
    Display *pDisplay = GetXDisplay();

    int n = 0;
    XLIB_Region Regions[3];

    if( mpClipRegion )
        Regions[n++] = mpClipRegion;

    if( pXReg && !XEmptyRegion( pXReg ) )
        Regions[n++] = pXReg;

    if( 0 == n )
        XSetClipMask( pDisplay, pGC, None );
    else if( 1 == n )
        XSetRegion( pDisplay, pGC, Regions[0] );
    else
    {
        XLIB_Region pTmpRegion = XCreateRegion();
        XIntersectRegion( Regions[0], Regions[1], pTmpRegion );

        XSetRegion( pDisplay, pGC, pTmpRegion );
        XDestroyRegion( pTmpRegion );
    }
}

GC X11SalGraphics::SelectPen()
{
    Display *pDisplay = GetXDisplay();

    if( !pPenGC_ )
    {
        XGCValues values;
        values.subwindow_mode       = ClipByChildren;
        values.fill_rule            = EvenOddRule;      // Pict import/ Gradient
        values.graphics_exposures   = False;

        pPenGC_ = XCreateGC( pDisplay, hDrawable_,
                             GCSubwindowMode | GCFillRule | GCGraphicsExposures,
                             &values );
    }

    if( !bPenGC_ )
    {
        if( nPenColor_ != SALCOLOR_NONE )
            XSetForeground( pDisplay, pPenGC_, nPenPixel_ );
        XSetFunction  ( pDisplay, pPenGC_, bXORMode_ ? GXxor : GXcopy );
        SetClipRegion( pPenGC_ );
        bPenGC_ = sal_True;
    }

    return pPenGC_;
}

GC X11SalGraphics::SelectBrush()
{
    Display *pDisplay = GetXDisplay();

    DBG_ASSERT( nBrushColor_ != SALCOLOR_NONE, "Brush Transparent" );

    if( !pBrushGC_ )
    {
        XGCValues values;
        values.subwindow_mode       = ClipByChildren;
        values.fill_rule            = EvenOddRule;      // Pict import/ Gradient
        values.graphics_exposures   = False;

        pBrushGC_ = XCreateGC( pDisplay, hDrawable_,
                               GCSubwindowMode | GCFillRule | GCGraphicsExposures,
                               &values );
    }

    if( !bBrushGC_ )
    {
        if( !bDitherBrush_ )
        {
            XSetFillStyle ( pDisplay, pBrushGC_, FillSolid );
            XSetForeground( pDisplay, pBrushGC_, nBrushPixel_ );
            if( bPrinter_ )
                XSetTile( pDisplay, pBrushGC_, None );
        }
        else
        {
            // Bug in Sun Solaris 2.5.1, XFillPolygon doesn't always reflect
            // changes of the tile. PROPERTY_BUG_Tile doesn't fix this !
            if (GetDisplay()->GetProperties() & PROPERTY_BUG_FillPolygon_Tile)
                XSetFillStyle ( pDisplay, pBrushGC_, FillSolid );

            XSetFillStyle ( pDisplay, pBrushGC_, FillTiled );
            XSetTile      ( pDisplay, pBrushGC_, hBrush_ );
        }
        XSetFunction  ( pDisplay, pBrushGC_, bXORMode_ ? GXxor : GXcopy );
        SetClipRegion( pBrushGC_ );

        bBrushGC_ = sal_True;
    }

    return pBrushGC_;
}

GC X11SalGraphics::GetTrackingGC()
{
    const char    dash_list[2] = {2, 2};

    if( !pTrackingGC_ )
    {
        XGCValues     values;

        values.graphics_exposures   = False;
        values.foreground           = m_pColormap->GetBlackPixel()
                                      ^ m_pColormap->GetWhitePixel();
        values.function             = GXxor;
        values.line_width           = 1;
        values.line_style           = LineOnOffDash;

        pTrackingGC_ = XCreateGC( GetXDisplay(), GetDrawable(),
                                  GCGraphicsExposures | GCForeground | GCFunction
                                  | GCLineWidth | GCLineStyle,
                                  &values );
        XSetDashes( GetXDisplay(), pTrackingGC_, 0, dash_list, 2 );
    }

    if( !bTrackingGC_ )
    {
        SetClipRegion( pTrackingGC_ );
        bTrackingGC_ = sal_True;
    }

    return pTrackingGC_;
}

void X11SalGraphics::DrawLines( sal_uLong              nPoints,
                                const SalPolyLine &rPoints,
                                GC                 pGC,
                                bool               bClose
                                )
{
    // calculate how many lines XWindow can draw in one go
    sal_uLong nMaxLines = (GetDisplay()->GetMaxRequestSize() - sizeof(xPolyPointReq))
                      / sizeof(xPoint);
    if( nMaxLines > nPoints ) nMaxLines = nPoints;

    // print all lines that XWindows can draw
    sal_uLong n;
    for( n = 0; nPoints - n > nMaxLines; n += nMaxLines - 1 )
        XDrawLines( GetXDisplay(),
                    GetDrawable(),
                    pGC,
                    &rPoints[n],
                    nMaxLines,
                    CoordModeOrigin );

    if( n < nPoints )
        XDrawLines( GetXDisplay(),
                    GetDrawable(),
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

// Calculate a dither-pixmap and make a brush of it
#define P_DELTA         51
#define DMAP( v, m )    ((v % P_DELTA) > m ? (v / P_DELTA) + 1 : (v / P_DELTA))

BOOL X11SalGraphics::GetDitherPixmap( SalColor nSalColor )
{
    static const short nOrdDither8Bit[ 8 ][ 8 ] =
    {
        { 0, 38,  9, 48,  2, 40, 12, 50},
        {25, 12, 35, 22, 28, 15, 37, 24},
        { 6, 44,  3, 41,  8, 47,  5, 44},
        {32, 19, 28, 16, 34, 21, 31, 18},
        { 1, 40, 11, 49,  0, 39, 10, 48},
        {27, 14, 36, 24, 26, 13, 36, 23},
        { 8, 46,  4, 43,  7, 45,  4, 42},
        {33, 20, 30, 17, 32, 20, 29, 16}
    };

    // test for correct depth (8bit)
    if( GetColormap().GetVisual().GetDepth() != 8 )
        return sal_False;

    char    pBits[64];
    char   *pBitsPtr = pBits;

    // Set the pallette-entries for the dithering tile
    sal_uInt8 nSalColorRed   = SALCOLOR_RED   ( nSalColor );
    sal_uInt8 nSalColorGreen = SALCOLOR_GREEN ( nSalColor );
    sal_uInt8 nSalColorBlue  = SALCOLOR_BLUE  ( nSalColor );

    for( int nY = 0; nY < 8; nY++ )
    {
        for( int nX = 0; nX < 8; nX++ )
        {
            short nMagic = nOrdDither8Bit[nY][nX];
            sal_uInt8 nR   = P_DELTA * DMAP( nSalColorRed,   nMagic );
            sal_uInt8 nG   = P_DELTA * DMAP( nSalColorGreen, nMagic );
            sal_uInt8 nB   = P_DELTA * DMAP( nSalColorBlue,  nMagic );

            *pBitsPtr++ = GetColormap().GetPixel( MAKE_SALCOLOR( nR, nG, nB ) );
        }
    }

    // create the tile as ximage and an according pixmap -> caching
    XImage *pImage = XCreateImage( GetXDisplay(),
                                   GetColormap().GetXVisual(),
                                   8,
                                   ZPixmap,
                                   0,               // offset
                                   pBits,           // data
                                   8, 8,            // width & height
                                   8,               // bitmap_pad
                                   0 );             // (default) bytes_per_line

    if ( GetDisplay()->GetProperties() & PROPERTY_BUG_Tile )
    {
        if (hBrush_)
            XFreePixmap (GetXDisplay(), hBrush_);
        hBrush_ = limitXCreatePixmap( GetXDisplay(), GetDrawable(), 8, 8, 8 );
    }
    else if( !hBrush_ )
        hBrush_ = limitXCreatePixmap( GetXDisplay(), GetDrawable(), 8, 8, 8 );

    // put the ximage to the pixmap
    XPutImage( GetXDisplay(),
               hBrush_,
               GetDisplay()->GetCopyGC( m_nXScreen ),
               pImage,
               0, 0,                        // Source
               0, 0,                        // Destination
               8, 8 );                      // width & height

    // destroy image-frame but not palette-data
    pImage->data = NULL;
    XDestroyImage( pImage );

    return sal_True;
}

void X11SalGraphics::GetResolution( sal_Int32 &rDPIX, sal_Int32 &rDPIY ) // const
{
    const SalDisplay *pDisplay = GetDisplay();
    if (!pDisplay)
    {
        OSL_TRACE("Null display");
        rDPIX = rDPIY = 96;
        return;
    }

    rDPIX = pDisplay->GetResolution().A();
    rDPIY = pDisplay->GetResolution().B();
    if( !pDisplay->GetExactResolution() && rDPIY < 96 )
    {
        rDPIX = Divide( rDPIX * 96, rDPIY );
            rDPIY = 96;
    }
    else if ( rDPIY > 200 )
    {
        rDPIX = Divide( rDPIX * 200, rDPIY );
        rDPIY = 200;
    }

    // #i12705# equalize x- and y-resolution if they are close enough
    if( rDPIX != rDPIY )
    {
        // different x- and y- resolutions are usually artifacts of
        // a wrongly calculated screen size.
#ifdef DEBUG
        printf("Forcing Resolution from %" SAL_PRIdINT32 "x%" SAL_PRIdINT32 " to %" SAL_PRIdINT32 "x%" SAL_PRIdINT32 "\n",
                rDPIX,rDPIY,rDPIY,rDPIY);
#endif
        rDPIX = rDPIY; // y-resolution is more trustworthy
    }
}

sal_uInt16 X11SalGraphics::GetBitCount() const
{
    return GetVisual().GetDepth();
}

long X11SalGraphics::GetGraphicsWidth() const
{
    if( m_pFrame )
        return m_pFrame->maGeometry.nWidth;
    else if( m_pVDev )
        return m_pVDev->GetWidth();
    else
        return 0;
}

long X11SalGraphics::GetGraphicsHeight() const
{
    if( m_pFrame )
        return m_pFrame->maGeometry.nHeight;
    else if( m_pVDev )
        return m_pVDev->GetHeight();
    else
        return 0;
}

void X11SalGraphics::ResetClipRegion()
{
    if( mpClipRegion )
    {
        bPenGC_         = sal_False;
        bFontGC_        = sal_False;
        bBrushGC_       = sal_False;
        bMonoGC_        = sal_False;
        bCopyGC_        = sal_False;
        bInvertGC_      = sal_False;
        bInvert50GC_    = sal_False;
        bStippleGC_     = sal_False;
        bTrackingGC_    = sal_False;

        XDestroyRegion( mpClipRegion );
        mpClipRegion    = NULL;
    }
}

bool X11SalGraphics::setClipRegion( const Region& i_rClip )
{
    if( mpClipRegion )
        XDestroyRegion( mpClipRegion );
    mpClipRegion = XCreateRegion();

    ImplRegionInfo aInfo;
    long nX, nY, nW, nH;
    bool bRegionRect = i_rClip.ImplGetFirstRect(aInfo, nX, nY, nW, nH );
    while( bRegionRect )
    {
        if ( nW && nH )
        {
            XRectangle aRect;
            aRect.x         = (short)nX;
            aRect.y         = (short)nY;
            aRect.width     = (unsigned short)nW;
            aRect.height    = (unsigned short)nH;

            XUnionRectWithRegion( &aRect, mpClipRegion, mpClipRegion );
        }
        bRegionRect = i_rClip.ImplGetNextRect( aInfo, nX, nY, nW, nH );
    }

    // done, invalidate GCs
    bPenGC_         = sal_False;
    bFontGC_        = sal_False;
    bBrushGC_       = sal_False;
    bMonoGC_        = sal_False;
    bCopyGC_        = sal_False;
    bInvertGC_      = sal_False;
    bInvert50GC_    = sal_False;
    bStippleGC_     = sal_False;
    bTrackingGC_    = sal_False;

    if( XEmptyRegion( mpClipRegion ) )
    {
        XDestroyRegion( mpClipRegion );
        mpClipRegion= NULL;
    }
    return true;
}

void X11SalGraphics::SetLineColor()
{
    if( nPenColor_ != SALCOLOR_NONE )
    {
        nPenColor_      = SALCOLOR_NONE;
        bPenGC_         = sal_False;
    }
}

void X11SalGraphics::SetLineColor( SalColor nSalColor )
{
    if( nPenColor_ != nSalColor )
    {
        nPenColor_      = nSalColor;
        nPenPixel_      = GetPixel( nSalColor );
        bPenGC_         = sal_False;
    }
}

void X11SalGraphics::SetFillColor()
{
    if( nBrushColor_ != SALCOLOR_NONE )
    {
        bDitherBrush_   = sal_False;
        nBrushColor_    = SALCOLOR_NONE;
        bBrushGC_       = sal_False;
    }
}

void X11SalGraphics::SetFillColor( SalColor nSalColor )
{
    if( nBrushColor_ != nSalColor )
    {
        bDitherBrush_   = sal_False;
        nBrushColor_    = nSalColor;
        nBrushPixel_    = GetPixel( nSalColor );
        if( TrueColor != GetColormap().GetVisual().GetClass()
            && GetColormap().GetColor( nBrushPixel_ ) != nBrushColor_
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
            bDitherBrush_ = GetDitherPixmap(nSalColor);
        bBrushGC_       = sal_False;
    }
}

void X11SalGraphics::SetROPLineColor( SalROPColor nROPColor )
{
    switch( nROPColor )
    {
        case SAL_ROP_0 : // 0
            nPenPixel_ = (Pixel)0;
            break;
        case SAL_ROP_1 : // 1
            nPenPixel_ = (Pixel)(1 << GetVisual().GetDepth()) - 1;
            break;
        case SAL_ROP_INVERT : // 2
            nPenPixel_ = (Pixel)(1 << GetVisual().GetDepth()) - 1;
            break;
    }
    nPenColor_  = GetColormap().GetColor( nPenPixel_ );
    bPenGC_     = sal_False;
}

void X11SalGraphics::SetROPFillColor( SalROPColor nROPColor )
{
    switch( nROPColor )
    {
        case SAL_ROP_0 : // 0
            nBrushPixel_ = (Pixel)0;
            break;
        case SAL_ROP_1 : // 1
            nBrushPixel_ = (Pixel)(1 << GetVisual().GetDepth()) - 1;
            break;
        case SAL_ROP_INVERT : // 2
            nBrushPixel_ = (Pixel)(1 << GetVisual().GetDepth()) - 1;
            break;
    }
    bDitherBrush_   = sal_False;
    nBrushColor_    = GetColormap().GetColor( nBrushPixel_ );
    bBrushGC_       = sal_False;
}

void X11SalGraphics::SetXORMode( bool bSet, bool )
{
    if( !bXORMode_ == bSet )
    {
        bXORMode_   = bSet;
        bPenGC_     = sal_False;
        bFontGC_    = sal_False;
        bBrushGC_   = sal_False;
        bMonoGC_        = sal_False;
        bCopyGC_        = sal_False;
        bInvertGC_  = sal_False;
        bInvert50GC_    = sal_False;
        bStippleGC_ = sal_False;
        bTrackingGC_    = sal_False;
    }
}

void X11SalGraphics::drawPixel( long nX, long nY )
{
    if( nPenColor_ !=  SALCOLOR_NONE )
        XDrawPoint( GetXDisplay(), GetDrawable(), SelectPen(), nX, nY );
}

void X11SalGraphics::drawPixel( long nX, long nY, SalColor nSalColor )
{
    if( nSalColor != SALCOLOR_NONE )
    {
        Display *pDisplay = GetXDisplay();

        if( (nPenColor_ == SALCOLOR_NONE) && !bPenGC_ )
        {
            SetLineColor( nSalColor );
            XDrawPoint( pDisplay, GetDrawable(), SelectPen(), nX, nY );
            nPenColor_ = SALCOLOR_NONE;
            bPenGC_ = False;
        }
        else
        {
            GC pGC = SelectPen();

            if( nSalColor != nPenColor_ )
                XSetForeground( pDisplay, pGC, GetPixel( nSalColor ) );

            XDrawPoint( pDisplay, GetDrawable(), pGC, nX, nY );

            if( nSalColor != nPenColor_ )
                XSetForeground( pDisplay, pGC, nPenPixel_ );
        }
    }
}

void X11SalGraphics::drawLine( long nX1, long nY1, long nX2, long nY2 )
{
    if( nPenColor_ != SALCOLOR_NONE )
    {
        if ( GetDisplay()->GetProperties() & PROPERTY_BUG_DrawLine )
        {
            GC aGC = SelectPen();
            XDrawPoint (GetXDisplay(), GetDrawable(), aGC, (int)nX1, (int)nY1);
            XDrawPoint (GetXDisplay(), GetDrawable(), aGC, (int)nX2, (int)nY2);
            XDrawLine  (GetXDisplay(), GetDrawable(), aGC, nX1, nY1, nX2, nY2 );
        }
        else
            XDrawLine( GetXDisplay(), GetDrawable(),SelectPen(),
                       nX1, nY1, nX2, nY2 );
    }
}

void X11SalGraphics::drawRect( long nX, long nY, long nDX, long nDY )
{
    if( nBrushColor_ != SALCOLOR_NONE )
    {
        XFillRectangle( GetXDisplay(),
                        GetDrawable(),
                        SelectBrush(),
                        nX, nY, nDX, nDY );
    }
    // description DrawRect is wrong; thus -1
    if( nPenColor_ != SALCOLOR_NONE )
        XDrawRectangle( GetXDisplay(),
                        GetDrawable(),
                        SelectPen(),
                        nX, nY, nDX-1, nDY-1 );
}

void X11SalGraphics::drawPolyLine( sal_uLong nPoints, const SalPoint *pPtAry )
{
    drawPolyLine( nPoints, pPtAry, false );
}

void X11SalGraphics::drawPolyLine( sal_uLong nPoints, const SalPoint *pPtAry, bool bClose )
{
    if( nPenColor_ != SALCOLOR_NONE )
    {
        SalPolyLine Points( nPoints, pPtAry );

        DrawLines( nPoints, Points, SelectPen(), bClose );
    }
}

void X11SalGraphics::drawPolygon( sal_uLong nPoints, const SalPoint* pPtAry )
{
    if( nPoints == 0 )
        return;

    if( nPoints < 3 )
    {
        if( !bXORMode_ )
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

    if( nBrushColor_ != SALCOLOR_NONE )
        XFillPolygon( GetXDisplay(),
                      GetDrawable(),
                      SelectBrush(),
                      &Points[0], nPoints,
                      Complex, CoordModeOrigin );

    if( nPenColor_ != SALCOLOR_NONE )
        DrawLines( nPoints, Points, SelectPen(), true );
}

void X11SalGraphics::drawPolyPolygon( sal_uInt32        nPoly,
                                   const sal_uInt32    *pPoints,
                                   PCONSTSALPOINT  *pPtAry )
{
    if( nBrushColor_ != SALCOLOR_NONE )
    {
        sal_uInt32      i, n;
        XLIB_Region pXRegA  = NULL;

        for( i = 0; i < nPoly; i++ ) {
            n = pPoints[i];
            SalPolyLine Points( n, pPtAry[i] );
            if( n > 2 )
            {
                XLIB_Region pXRegB = XPolygonRegion( &Points[0], n+1, WindingRule );
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
            SetClipRegion( pGC, pXRegA ); // ??? twice
            XDestroyRegion( pXRegA );
            bBrushGC_ = sal_False;

            XFillRectangle( GetXDisplay(),
                            GetDrawable(),
                            pGC,
                            aXRect.x, aXRect.y, aXRect.width, aXRect.height );
        }
   }

   if( nPenColor_ != SALCOLOR_NONE )
       for( sal_uInt32 i = 0; i < nPoly; i++ )
           drawPolyLine( pPoints[i], pPtAry[i], true );
}

sal_Bool X11SalGraphics::drawPolyLineBezier( sal_uLong, const SalPoint*, const BYTE* )
{
    return sal_False;
}

sal_Bool X11SalGraphics::drawPolygonBezier( sal_uLong, const SalPoint*, const BYTE* )
{
    return sal_False;
}

sal_Bool X11SalGraphics::drawPolyPolygonBezier( sal_uInt32, const sal_uInt32*,
                                                const SalPoint* const*, const BYTE* const* )
{
    return sal_False;
}

void X11SalGraphics::invert( sal_uLong nPoints,
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
        XFillPolygon( GetXDisplay(),
                      GetDrawable(),
                      pGC,
                      &Points[0], nPoints,
                      Complex, CoordModeOrigin );
}

BOOL X11SalGraphics::drawEPS( long,long,long,long,void*,sal_uLong )
{
    return sal_False;
}

XID X11SalGraphics::GetXRenderPicture()
{
    XRenderPeer& rRenderPeer = XRenderPeer::GetInstance();

    if( !m_aXRenderPicture )
    {
        // check xrender support for matching visual
        XRenderPictFormat* pXRenderFormat = GetXRenderFormat();
        if( !pXRenderFormat )
            return 0;
        // get the matching xrender target for drawable
        m_aXRenderPicture = rRenderPeer.CreatePicture( hDrawable_, pXRenderFormat, 0, NULL );
    }

    {
        // reset clip region
        // TODO: avoid clip reset if already done
        XRenderPictureAttributes aAttr;
        aAttr.clip_mask = None;
        rRenderPeer.ChangePicture( m_aXRenderPicture, CPClipMask, &aAttr );
    }

    return m_aXRenderPicture;
}

XRenderPictFormat* X11SalGraphics::GetXRenderFormat() const
{
    if( m_pXRenderFormat == NULL )
        m_pXRenderFormat = XRenderPeer::GetInstance().FindVisualFormat( GetVisual().visual );
    return m_pXRenderFormat;
}

SystemGraphicsData X11SalGraphics::GetGraphicsData() const
{
    SystemGraphicsData aRes;

    aRes.nSize = sizeof(aRes);
    aRes.pDisplay  = GetXDisplay();
    aRes.hDrawable = hDrawable_;
    aRes.pVisual   = GetVisual().visual;
    aRes.nScreen   = m_nXScreen.getXScreen();
    aRes.nDepth    = GetBitCount();
    aRes.aColormap = GetColormap().GetXColormap();
    aRes.pXRenderFormat = m_pXRenderFormat;
    return aRes;
}

// draw a poly-polygon
bool X11SalGraphics::drawPolyPolygon( const ::basegfx::B2DPolyPolygon& rOrigPolyPoly, double fTransparency )
{
    // nothing to do for empty polypolygons
    const int nOrigPolyCount = rOrigPolyPoly.count();
    if( nOrigPolyCount <= 0 )
        return sal_True;

    // nothing to do if everything is transparent
    if( (nBrushColor_ == SALCOLOR_NONE)
    &&  (nPenColor_ == SALCOLOR_NONE) )
        return sal_True;

    // cannot handle pencolor!=brushcolor yet
    if( (nPenColor_ != SALCOLOR_NONE)
    &&  (nPenColor_ != nBrushColor_) )
        return sal_False;

    // TODO: remove the env-variable when no longer needed
    static const char* pRenderEnv = getenv( "SAL_DISABLE_RENDER_POLY" );
    if( pRenderEnv )
        return sal_False;

    // snap to raster if requested
    basegfx::B2DPolyPolygon aPolyPoly = rOrigPolyPoly;
    const bool bSnapToRaster = !getAntiAliasB2DDraw();
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

bool X11SalGraphics::drawFilledTrapezoids( const ::basegfx::B2DTrapezoid* pB2DTraps, int nTrapCount, double fTransparency )
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
    SalDisplay::RenderEntry& rEntry = GetDisplay()->GetRenderEntries( m_nXScreen )[ 32 ];
    if( !rEntry.m_aPicture )
    {
        Display* pXDisplay = GetXDisplay();

        rEntry.m_aPixmap = limitXCreatePixmap( pXDisplay, hDrawable_, 1, 1, 32 );
        XRenderPictureAttributes aAttr;
        aAttr.repeat = true;

        XRenderPictFormat* pXRPF = rRenderPeer.FindStandardFormat( PictStandardARGB32 );
        rEntry.m_aPicture = rRenderPeer.CreatePicture( rEntry.m_aPixmap, pXRPF, CPRepeat, &aAttr );
    }

    // set polygon foreground color and opacity
    XRenderColor aRenderColor = GetXRenderColor( nBrushColor_ , fTransparency );
    rRenderPeer.FillRectangle( PictOpSrc, rEntry.m_aPicture, &aRenderColor, 0, 0, 1, 1 );

    // set clipping
    // TODO: move into GetXRenderPicture?
    if( mpClipRegion && !XEmptyRegion( mpClipRegion ) )
        rRenderPeer.SetPictureClipRegion( aDstPic, mpClipRegion );

    // render the trapezoids
    const XRenderPictFormat* pMaskFormat = rRenderPeer.GetStandardFormatA8();
    rRenderPeer.CompositeTrapezoids( PictOpOver,
        rEntry.m_aPicture, aDstPic, pMaskFormat, 0, 0, &aTrapVector[0], aTrapVector.size() );

    return true;
}

bool X11SalGraphics::drawPolyLine(
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
    const SalColor aKeepBrushColor = nBrushColor_;
    nBrushColor_ = nPenColor_;

    // #i11575#desc5#b adjust B2D tesselation result to raster positions
    basegfx::B2DPolygon aPolygon = rPolygon;
    const double fHalfWidth = 0.5 * rLineWidth.getX();
    aPolygon.transform( basegfx::tools::createTranslateB2DHomMatrix(+fHalfWidth,+fHalfWidth) );

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
        nBrushColor_ = aKeepBrushColor;
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
    nBrushColor_ = aKeepBrushColor;
    return bDrawnOk;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
