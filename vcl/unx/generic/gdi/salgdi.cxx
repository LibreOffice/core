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

#include <config_graphite.h>

#include <vector>
#include <queue>
#include <set>

#include <prex.h>
#include <X11/Xproto.h>
#include <postx.h>

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

#include <vcl/jobdata.hxx>
#include <vcl/virdev.hxx>

#include "unx/salunx.h"
#include "unx/saldata.hxx"
#include "unx/saldisp.hxx"
#include "unx/salgdi.h"
#include "unx/salframe.h"
#include "unx/salvd.h"
#include "unx/x11/x11gdiimpl.h"
#include <unx/x11/xlimits.hxx>

#include "salgdiimpl.hxx"
#include "unx/x11windowprovider.hxx"
#include "textrender.hxx"
#include "gdiimpl.hxx"
#include "opengl/x11/gdiimpl.hxx"
#include "x11cairotextrender.hxx"
#include "openglx11cairotextrender.hxx"

#include "generic/printergfx.hxx"
#include "xrender_peer.hxx"
#include "cairo_cairo.hxx"
#include "cairo_xlib_cairo.hxx"

#include <vcl/opengl/OpenGLHelper.hxx>

X11SalGraphics::X11SalGraphics():
    m_pFrame(NULL),
    m_pVDev(NULL),
    m_pColormap(NULL),
    m_pDeleteColormap(NULL),
    hDrawable_(None),
    m_nXScreen( 0 ),
    m_pXRenderFormat(NULL),
    m_aXRenderPicture(0),
    pPaintRegion_(NULL),
    mpClipRegion(NULL),
    pFontGC_(NULL),
    nTextPixel_(0),
    hBrush_(None),
    bWindow_(false),
    bPrinter_(false),
    bVirDev_(false),
    bFontGC_(false)
{
    if (OpenGLHelper::isVCLOpenGLEnabled())
    {
        mxImpl.reset(new X11OpenGLSalGraphicsImpl(*this));
        mxTextRenderImpl.reset((new OpenGLX11CairoTextRender(*this)));
    }
    else
    {
        mxTextRenderImpl.reset((new X11CairoTextRender(*this)));
        mxImpl.reset(new X11SalGraphicsImpl(*this));
    }

}

X11SalGraphics::~X11SalGraphics()
{
    DeInit();
    ReleaseFonts();
    freeResources();
}

void X11SalGraphics::freeResources()
{
    Display *pDisplay = GetXDisplay();

    DBG_ASSERT( !pPaintRegion_, "pPaintRegion_" );
    if( mpClipRegion ) XDestroyRegion( mpClipRegion ), mpClipRegion = None;

    mxImpl->freeResources();

    if( hBrush_ )       XFreePixmap( pDisplay, hBrush_ ), hBrush_ = None;
    if( pFontGC_ ) XFreeGC( pDisplay, pFontGC_ ), pFontGC_ = None;
    if( m_pDeleteColormap )
        delete m_pDeleteColormap, m_pColormap = m_pDeleteColormap = NULL;

    if( m_aXRenderPicture )
        XRenderPeer::GetInstance().FreePicture( m_aXRenderPicture ), m_aXRenderPicture = 0;

    bFontGC_ = false;
}

SalGraphicsImpl* X11SalGraphics::GetImpl() const
{
    return mxImpl.get();
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
        m_pColormap = &vcl_sal::getSalDisplay(GetGenericData())->GetColormap( nXScreen );
        m_nXScreen = nXScreen;
    }

    hDrawable_ = aDrawable;
    SetXRenderFormat( NULL );
    if( m_aXRenderPicture )
    {
        XRenderPeer::GetInstance().FreePicture( m_aXRenderPicture );
        m_aXRenderPicture = 0;
    }

    // TODO: moggi: FIXME nTextPixel_     = GetPixel( nTextColor_ );
}

void X11SalGraphics::Init( SalFrame *pFrame, Drawable aTarget,
                           SalX11Screen nXScreen )
{
    m_pColormap = &vcl_sal::getSalDisplay(GetGenericData())->GetColormap(nXScreen);
    m_nXScreen  = nXScreen;

    m_pFrame    = pFrame;
    m_pVDev     = NULL;

    bWindow_    = true;
    bVirDev_    = false;

    SetDrawable( aTarget, nXScreen );
    mxImpl->Init();
}

void X11SalGraphics::DeInit()
{
    mxImpl->DeInit();
    SetDrawable( None, m_nXScreen );
}

void X11SalGraphics::SetClipRegion( GC pGC, Region pXReg ) const
{
    Display *pDisplay = GetXDisplay();

    int n = 0;
    Region Regions[3];

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
        Region pTmpRegion = XCreateRegion();
        XIntersectRegion( Regions[0], Regions[1], pTmpRegion );

        XSetRegion( pDisplay, pGC, pTmpRegion );
        XDestroyRegion( pTmpRegion );
    }
}

// Calculate a dither-pixmap and make a brush of it
#define P_DELTA         51
#define DMAP( v, m )    ((v % P_DELTA) > m ? (v / P_DELTA) + 1 : (v / P_DELTA))

bool X11SalGraphics::GetDitherPixmap( SalColor nSalColor )
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
        return false;

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

    if( !hBrush_ )
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

    return true;
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

    Pair dpi = pDisplay->GetResolution();
    rDPIX = dpi.A();
    rDPIY = dpi.B();

    if ( rDPIY > 200 )
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
    return mxImpl->GetBitCount();
}

long X11SalGraphics::GetGraphicsWidth() const
{
    return mxImpl->GetGraphicsWidth();
}

void X11SalGraphics::ResetClipRegion()
{
    mxImpl->ResetClipRegion();
}

bool X11SalGraphics::setClipRegion( const vcl::Region& i_rClip )
{
    return mxImpl->setClipRegion( i_rClip );
}

void X11SalGraphics::SetLineColor()
{
    mxImpl->SetLineColor();
}

void X11SalGraphics::SetLineColor( SalColor nSalColor )
{
    mxImpl->SetLineColor( nSalColor );
}

void X11SalGraphics::SetFillColor()
{
    mxImpl->SetFillColor();
}

void X11SalGraphics::SetFillColor( SalColor nSalColor )
{
    mxImpl->SetFillColor( nSalColor );
}

void X11SalGraphics::SetROPLineColor( SalROPColor nROPColor )
{
    mxImpl->SetROPLineColor( nROPColor );
}

void X11SalGraphics::SetROPFillColor( SalROPColor nROPColor )
{
    mxImpl->SetROPFillColor( nROPColor );
}

void X11SalGraphics::SetXORMode( bool bSet, bool bInvertOnly )
{
    mxImpl->SetXORMode( bSet, bInvertOnly );
}

void X11SalGraphics::drawPixel( long nX, long nY )
{
    mxImpl->drawPixel( nX, nY );
}

void X11SalGraphics::drawPixel( long nX, long nY, SalColor nSalColor )
{
    mxImpl->drawPixel( nX, nY, nSalColor );
}

void X11SalGraphics::drawLine( long nX1, long nY1, long nX2, long nY2 )
{
    mxImpl->drawLine( nX1, nY1, nX2, nY2 );
}

void X11SalGraphics::drawRect( long nX, long nY, long nDX, long nDY )
{
    mxImpl->drawRect( nX, nY, nDX, nDY );
}

void X11SalGraphics::drawPolyLine( sal_uInt32 nPoints, const SalPoint *pPtAry )
{
    mxImpl->drawPolyLine( nPoints, pPtAry );
}

void X11SalGraphics::drawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry )
{
    mxImpl->drawPolygon( nPoints, pPtAry );
}

void X11SalGraphics::drawPolyPolygon( sal_uInt32 nPoly,
                                   const sal_uInt32    *pPoints,
                                   PCONSTSALPOINT  *pPtAry )
{
    mxImpl->drawPolyPolygon( nPoly, pPoints, pPtAry );
}

bool X11SalGraphics::drawPolyLineBezier( sal_uInt32 nPoints, const SalPoint* pPtAry, const sal_uInt8* pFlgAry )
{
    return mxImpl->drawPolyLineBezier( nPoints, pPtAry, pFlgAry );
}

bool X11SalGraphics::drawPolygonBezier( sal_uInt32 nPoints, const SalPoint* pPtAry, const sal_uInt8* pFlgAry )
{
    return mxImpl->drawPolygonBezier( nPoints, pPtAry, pFlgAry );
}

bool X11SalGraphics::drawPolyPolygonBezier( sal_uInt32 nPoints, const sal_uInt32* pPoints,
                                                const SalPoint* const* pPtAry, const sal_uInt8* const* pFlgAry)
{
    return mxImpl->drawPolyPolygonBezier( nPoints, pPoints, pPtAry, pFlgAry );
}

void X11SalGraphics::invert( sal_uInt32 nPoints,
                             const SalPoint* pPtAry,
                             SalInvert nFlags )
{
    mxImpl->invert( nPoints, pPtAry, nFlags );
}

bool X11SalGraphics::drawEPS( long nX, long nY, long nWidth,
        long nHeight, void* pPtr, sal_uLong nSize )
{
    return mxImpl->drawEPS( nX, nY, nWidth, nHeight, pPtr, nSize );
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

bool X11SalGraphics::SupportsCairo() const
{
    Display *pDisplay = GetXDisplay();
    int nDummy;
    return XQueryExtension(pDisplay, "RENDER", &nDummy, &nDummy, &nDummy);
}

cairo::SurfaceSharedPtr X11SalGraphics::CreateSurface(const cairo::CairoSurfaceSharedPtr& rSurface) const
{
    return cairo::SurfaceSharedPtr(new cairo::X11Surface(rSurface));
}

namespace
{
    static cairo::X11SysData getSysData( const vcl::Window& rWindow )
    {
        const SystemEnvData* pSysData = cairo::GetSysData(&rWindow);

        if( !pSysData )
            return cairo::X11SysData();
        else
            return cairo::X11SysData(*pSysData);
    }

    static cairo::X11SysData getSysData( const VirtualDevice& rVirDev )
    {
        return cairo::X11SysData( rVirDev.GetSystemGfxData() );
    }
}

cairo::SurfaceSharedPtr X11SalGraphics::CreateSurface( const OutputDevice& rRefDevice,
                                int x, int y, int width, int height ) const
{
    if( rRefDevice.GetOutDevType() == OUTDEV_WINDOW )
        return cairo::SurfaceSharedPtr(new cairo::X11Surface(getSysData(static_cast<const vcl::Window&>(rRefDevice)),
                                               x,y,width,height));
    if( rRefDevice.GetOutDevType() == OUTDEV_VIRDEV )
        return cairo::SurfaceSharedPtr(new cairo::X11Surface(getSysData(static_cast<const VirtualDevice&>(rRefDevice)),
                                               x,y,width,height));
    return cairo::SurfaceSharedPtr();
}

cairo::SurfaceSharedPtr X11SalGraphics::CreateBitmapSurface( const OutputDevice&     rRefDevice,
                                      const BitmapSystemData& rData,
                                      const Size&             rSize ) const
{
    SAL_INFO(
        "canvas.cairo",
        "requested size: " << rSize.Width() << " x " << rSize.Height()
            << " available size: " << rData.mnWidth << " x "
            << rData.mnHeight);
    if ( rData.mnWidth == rSize.Width() && rData.mnHeight == rSize.Height() )
    {
        if( rRefDevice.GetOutDevType() == OUTDEV_WINDOW )
            return cairo::SurfaceSharedPtr(new cairo::X11Surface(getSysData(static_cast<const vcl::Window&>(rRefDevice)), rData ));
        else if( rRefDevice.GetOutDevType() == OUTDEV_VIRDEV )
            return cairo::SurfaceSharedPtr(new cairo::X11Surface(getSysData(static_cast<const VirtualDevice&>(rRefDevice)), rData ));
    }

    return cairo::SurfaceSharedPtr();
}

css::uno::Any X11SalGraphics::GetNativeSurfaceHandle(cairo::SurfaceSharedPtr& rSurface, const ::basegfx::B2ISize& /*rSize*/) const
{
    cairo::X11Surface& rXlibSurface=dynamic_cast<cairo::X11Surface&>(*rSurface.get());
    css::uno::Sequence< css::uno::Any > args( 3 );
    args[0] = css::uno::Any( false );  // do not call XFreePixmap on it
    args[1] = css::uno::Any( rXlibSurface.getPixmap()->mhDrawable );
    args[2] = css::uno::Any( sal_Int32( rXlibSurface.getDepth() ) );
    return css::uno::Any(args);
}

// draw a poly-polygon
bool X11SalGraphics::drawPolyPolygon( const ::basegfx::B2DPolyPolygon& rOrigPolyPoly, double fTransparency )
{
    return mxImpl->drawPolyPolygon( rOrigPolyPoly, fTransparency );
}

bool X11SalGraphics::drawPolyLine(
    const ::basegfx::B2DPolygon& rPolygon,
    double fTransparency,
    const ::basegfx::B2DVector& rLineWidth,
    basegfx::B2DLineJoin eLineJoin,
    com::sun::star::drawing::LineCap eLineCap)
{
    return mxImpl->drawPolyLine( rPolygon, fTransparency, rLineWidth,
            eLineJoin, eLineCap );
}

bool X11SalGraphics::drawGradient(const tools::PolyPolygon& rPoly, const Gradient& rGradient)
{
    return mxImpl->drawGradient(rPoly, rGradient);
}

OpenGLContext *X11SalGraphics::BeginPaint()
{
    return mxImpl->beginPaint();
}

SalGeometryProvider *X11SalGraphics::GetGeometryProvider() const
{
    if (m_pFrame)
        return static_cast< SalGeometryProvider * >(m_pFrame);
    else
        return static_cast< SalGeometryProvider * >(m_pVDev);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
