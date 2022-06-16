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

#include <config_features.h>
#include <vcl/skia/SkiaHelper.hxx>
#if HAVE_FEATURE_SKIA
#include <skia/x11/gdiimpl.hxx>
#include <skia/x11/textrender.hxx>
#endif

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrender.h>


#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>

#include <headless/svpgdi.hxx>

#include <vcl/sysdata.hxx>
#include <vcl/virdev.hxx>
#include <sal/log.hxx>

#include <unx/salunx.h>
#include <unx/saldisp.hxx>
#include <unx/salgdi.h>
#include <unx/x11/xlimits.hxx>

#include <salframe.hxx>
#include <salgdiimpl.hxx>
#include <textrender.hxx>
#include <salvd.hxx>
#include "gdiimpl.hxx"

#include <unx/x11/x11cairotextrender.hxx>
#include <unx/x11/xrender_peer.hxx>
#include "cairo_xlib_cairo.hxx"
#include <cairo-xlib.h>

#if ENABLE_CAIRO_CANVAS
#include "X11CairoSalGraphicsImpl.hxx"
#endif

// X11Common

X11Common::X11Common()
    : m_hDrawable(None)
    , m_pColormap(nullptr)
    , m_pExternalSurface(nullptr)
{}

cairo_t* X11Common::getCairoContext()
{
    if (m_pExternalSurface)
        return cairo_create(m_pExternalSurface);

    cairo_surface_t* surface = cairo_xlib_surface_create(GetXDisplay(), m_hDrawable, GetVisual().visual, SAL_MAX_INT16, SAL_MAX_INT16);

    cairo_t *cr = cairo_create(surface);
    cairo_surface_destroy(surface);

    return cr;
}

void X11Common::releaseCairoContext(cairo_t* cr)
{
   cairo_destroy(cr);
}

bool X11Common::SupportsCairo() const
{
    static bool bSupportsCairo = [this] {
        Display *pDisplay = GetXDisplay();
        int nDummy;
        return XQueryExtension(pDisplay, "RENDER", &nDummy, &nDummy, &nDummy);
    }();
    return bSupportsCairo;
}

// X11SalGraphics

X11SalGraphics::X11SalGraphics():
    m_pFrame(nullptr),
    m_pVDev(nullptr),
    m_nXScreen( 0 ),
    m_pXRenderFormat(nullptr),
    m_aXRenderPicture(0),
    mpClipRegion(nullptr),
    hBrush_(None),
    bWindow_(false)
{
#if HAVE_FEATURE_SKIA
    if (SkiaHelper::isVCLSkiaEnabled())
    {
        mxImpl.reset(new X11SkiaSalGraphicsImpl(*this));
        mxTextRenderImpl.reset(new SkiaTextRender);
    }
    else
#endif
    {
        mxTextRenderImpl.reset(new X11CairoTextRender(*this));
#if ENABLE_CAIRO_CANVAS
        mxImpl.reset(new X11CairoSalGraphicsImpl(*this, maX11Common));
#else
        mxImpl.reset(new X11SalGraphicsImpl(*this));
#endif
    }
}

X11SalGraphics::~X11SalGraphics() COVERITY_NOEXCEPT_FALSE
{
    DeInit();
    ReleaseFonts();
    freeResources();
}

void X11SalGraphics::freeResources()
{
    Display *pDisplay = GetXDisplay();

    if( mpClipRegion )
    {
        XDestroyRegion( mpClipRegion );
        mpClipRegion = nullptr;
    }

    mxImpl->freeResources();

    if( hBrush_ )
    {
        XFreePixmap( pDisplay, hBrush_ );
        hBrush_ = None;
    }
    if( m_pDeleteColormap )
    {
        m_pDeleteColormap.reset();
        maX11Common.m_pColormap = nullptr;
    }
    if( m_aXRenderPicture )
    {
        XRenderPeer::GetInstance().FreePicture( m_aXRenderPicture );
        m_aXRenderPicture = 0;
    }
}

SalGraphicsImpl* X11SalGraphics::GetImpl() const
{
    return mxImpl.get();
}

void X11SalGraphics::SetDrawable(Drawable aDrawable, cairo_surface_t* pExternalSurface, SalX11Screen nXScreen)
{
    maX11Common.m_pExternalSurface = pExternalSurface;

    // shortcut if nothing changed
    if( maX11Common.m_hDrawable == aDrawable )
        return;

    // free screen specific resources if needed
    if( nXScreen != m_nXScreen )
    {
        freeResources();
        maX11Common.m_pColormap = &vcl_sal::getSalDisplay(GetGenericUnixSalData())->GetColormap( nXScreen );
        m_nXScreen = nXScreen;
    }

    maX11Common.m_hDrawable = aDrawable;
    SetXRenderFormat( nullptr );
    if( m_aXRenderPicture )
    {
        XRenderPeer::GetInstance().FreePicture( m_aXRenderPicture );
        m_aXRenderPicture = 0;
    }
}

void X11SalGraphics::Init( SalFrame *pFrame, Drawable aTarget,
                           SalX11Screen nXScreen )
{
    maX11Common.m_pColormap = &vcl_sal::getSalDisplay(GetGenericUnixSalData())->GetColormap(nXScreen);
    m_nXScreen  = nXScreen;

    m_pFrame    = pFrame;
    m_pVDev     = nullptr;

    bWindow_    = true;

    SetDrawable(aTarget, nullptr, nXScreen);
    mxImpl->Init();
}

void X11SalGraphics::DeInit()
{
    mxImpl->DeInit();
    SetDrawable(None, nullptr, m_nXScreen);
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

bool X11SalGraphics::GetDitherPixmap( Color nColor )
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

    // Set the palette-entries for the dithering tile
    sal_uInt8 nColorRed   = nColor.GetRed();
    sal_uInt8 nColorGreen = nColor.GetGreen();
    sal_uInt8 nColorBlue  = nColor.GetBlue();

    for(auto & nY : nOrdDither8Bit)
    {
        for( int nX = 0; nX < 8; nX++ )
        {
            short nMagic = nY[nX];
            sal_uInt8 nR   = P_DELTA * DMAP( nColorRed,   nMagic );
            sal_uInt8 nG   = P_DELTA * DMAP( nColorGreen, nMagic );
            sal_uInt8 nB   = P_DELTA * DMAP( nColorBlue,  nMagic );

            *pBitsPtr++ = GetColormap().GetPixel( Color( nR, nG, nB ) );
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
    pImage->data = nullptr;
    XDestroyImage( pImage );

    return true;
}

sal_Int32 X11SalGraphics::GetSgpMetric(vcl::SGPmetric eMetric) const
{
    assert(m_pVDev || m_pFrame);
    if (m_pFrame && eMetric == vcl::SGPmetric::BitCount)
        return GetVisual().GetDepth();

    if (m_pVDev)
       return m_pVDev->GetSgpMetric(eMetric);
    else
        return m_pFrame->GetSgpMetric(eMetric);
}

XRenderPictFormat* X11SalGraphics::GetXRenderFormat() const
{
    if( m_pXRenderFormat == nullptr )
        m_pXRenderFormat = XRenderPeer::GetInstance().FindVisualFormat( GetVisual().visual );
    return m_pXRenderFormat;
}

SystemGraphicsData X11SalGraphics::GetGraphicsData() const
{
    SystemGraphicsData aRes;

    aRes.nSize = sizeof(aRes);
    aRes.pDisplay  = GetXDisplay();
    aRes.hDrawable = maX11Common.m_hDrawable;
    aRes.pVisual   = GetVisual().visual;
    aRes.nScreen   = m_nXScreen.getXScreen();
    aRes.pXRenderFormat = m_pXRenderFormat;
    return aRes;
}

void X11SalGraphics::Flush()
{
    mxImpl->Flush();
}

#if ENABLE_CAIRO_CANVAS

bool X11SalGraphics::SupportsCairo() const
{
    return maX11Common.SupportsCairo();
}

cairo::SurfaceSharedPtr X11SalGraphics::CreateSurface(const cairo::CairoSurfaceSharedPtr& rSurface) const
{
    return std::make_shared<cairo::X11Surface>(rSurface);
}

namespace
{
    cairo::X11SysData getSysData( const vcl::Window& rWindow )
    {
        const SystemEnvData* pSysData = rWindow.GetSystemData();

        if( !pSysData )
            return cairo::X11SysData();
        else
            return cairo::X11SysData(*pSysData, rWindow.ImplGetFrame());
    }

    cairo::X11SysData getSysData( const VirtualDevice& rVirDev )
    {
        return cairo::X11SysData( rVirDev.GetSystemGfxData() );
    }
}

cairo::SurfaceSharedPtr X11SalGraphics::CreateSurface( const OutputDevice& rRefDevice,
                                int x, int y, int width, int height ) const
{
    if( rRefDevice.GetOutDevType() == OUTDEV_WINDOW )
        return std::make_shared<cairo::X11Surface>(getSysData(*rRefDevice.GetOwnerWindow()),
                                               x,y,width,height);
    if( rRefDevice.IsVirtual() )
        return std::make_shared<cairo::X11Surface>(getSysData(static_cast<const VirtualDevice&>(rRefDevice)),
                                               x,y,width,height);
    return cairo::SurfaceSharedPtr();
}

cairo::SurfaceSharedPtr X11SalGraphics::CreateBitmapSurface( const OutputDevice&     rRefDevice,
                                      const BitmapSystemData& rData,
                                      const Size&             rSize ) const
{
    SAL_INFO("vcl", "requested size: " << rSize.Width() << " x " << rSize.Height()
              << " available size: " << rData.mnWidth << " x "
              << rData.mnHeight);
    if ( rData.mnWidth == rSize.Width() && rData.mnHeight == rSize.Height() )
    {
        if( rRefDevice.GetOutDevType() == OUTDEV_WINDOW )
            return std::make_shared<cairo::X11Surface>(getSysData(*rRefDevice.GetOwnerWindow()), rData );
        else if( rRefDevice.IsVirtual() )
            return std::make_shared<cairo::X11Surface>(getSysData(static_cast<const VirtualDevice&>(rRefDevice)), rData );
    }

    return cairo::SurfaceSharedPtr();
}

css::uno::Any X11SalGraphics::GetNativeSurfaceHandle(cairo::SurfaceSharedPtr& rSurface, const basegfx::B2ISize& /*rSize*/) const
{
    cairo::X11Surface& rXlibSurface=dynamic_cast<cairo::X11Surface&>(*rSurface);
    css::uno::Sequence< css::uno::Any > args{
        css::uno::Any(false), // do not call XFreePixmap on it
        css::uno::Any(sal_Int64(rXlibSurface.getPixmap()->mhDrawable)),
        css::uno::Any(sal_Int32( rXlibSurface.getDepth() ))
    };
    return css::uno::Any(args);
}

#endif // ENABLE_CAIRO_CANVAS

vcl::SalGeometryProvider *X11SalGraphics::GetGeometryProvider() const
{
    if (m_pFrame)
        return static_cast<vcl::SalGeometryProvider*>(m_pFrame);
    else
        return static_cast<vcl::SalGeometryProvider*>(m_pVDev);
}

cairo_t* X11SalGraphics::getCairoContext()
{
    return maX11Common.getCairoContext();
}

void X11SalGraphics::releaseCairoContext(cairo_t* cr)
{
   X11Common::releaseCairoContext(cr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
