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

#include <vector>
#include <queue>
#include <set>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrender.h>
#include <X11/Xproto.h>


#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <basegfx/polygon/b2dtrapezoid.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>

#include <headless/svpgdi.hxx>

#include <vcl/jobdata.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/virdev.hxx>
#include <sal/log.hxx>

#include <unx/salunx.h>
#include <unx/saldisp.hxx>
#include <unx/salgdi.h>
#include <unx/salvd.h>
#include <unx/x11/x11gdiimpl.h>
#include <unx/x11/xlimits.hxx>

#include <salgdiimpl.hxx>
#include <unx/nativewindowhandleprovider.hxx>
#include <textrender.hxx>
#include "gdiimpl.hxx"
#include <opengl/x11/gdiimpl.hxx>
#include "x11cairotextrender.hxx"
#include "openglx11cairotextrender.hxx"

#include <unx/printergfx.hxx>
#include <unx/x11/xrender_peer.hxx>
#include "cairo_xlib_cairo.hxx"
#include <cairo-xlib.h>

#include <vcl/opengl/OpenGLHelper.hxx>

X11SalGraphics::X11SalGraphics():
    m_pFrame(nullptr),
    m_pVDev(nullptr),
    m_pColormap(nullptr),
    hDrawable_(None),
    m_nXScreen( 0 ),
    m_pXRenderFormat(nullptr),
    m_aXRenderPicture(0),
    mpClipRegion(nullptr),
#if ENABLE_CAIRO_CANVAS
    maClipRegion(),
    mnPenColor(SALCOLOR_NONE),
    mnFillColor(SALCOLOR_NONE),
#endif // ENABLE_CAIRO_CANVAS
    pFontGC_(nullptr),
    nTextPixel_(0),
    hBrush_(None),
    bWindow_(false),
    bVirDev_(false),
    bFontGC_(false),
    m_bOpenGL(OpenGLHelper::isVCLOpenGLEnabled())
{
    if (m_bOpenGL)
    {
        mxImpl.reset(new X11OpenGLSalGraphicsImpl(*this));
        mxTextRenderImpl.reset(new OpenGLX11CairoTextRender(*this));
    }
    else
    {
        mxTextRenderImpl.reset(new X11CairoTextRender(*this));
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

    if( mpClipRegion )
    {
        XDestroyRegion( mpClipRegion );
        mpClipRegion = None;
    }

    mxImpl->freeResources();

    if( hBrush_ )
    {
        XFreePixmap( pDisplay, hBrush_ );
        hBrush_ = None;
    }
    if( pFontGC_ )
    {
        XFreeGC( pDisplay, pFontGC_ );
        pFontGC_ = None;
    }
    if( m_pDeleteColormap )
    {
        m_pDeleteColormap.reset();
        m_pColormap = nullptr;
    }
    if( m_aXRenderPicture )
    {
        XRenderPeer::GetInstance().FreePicture( m_aXRenderPicture );
        m_aXRenderPicture = 0;
    }

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
        m_pColormap = &vcl_sal::getSalDisplay(GetGenericUnixSalData())->GetColormap( nXScreen );
        m_nXScreen = nXScreen;
    }

    hDrawable_ = aDrawable;
    SetXRenderFormat( nullptr );
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
    m_pColormap = &vcl_sal::getSalDisplay(GetGenericUnixSalData())->GetColormap(nXScreen);
    m_nXScreen  = nXScreen;

    m_pFrame    = pFrame;
    m_pVDev     = nullptr;

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

void X11SalGraphics::GetResolution( sal_Int32 &rDPIX, sal_Int32 &rDPIY ) // const
{
    char* pForceDpi;
    if ((pForceDpi = getenv("SAL_FORCEDPI")))
    {
        OString sForceDPI(pForceDpi);
        rDPIX = rDPIY = sForceDPI.toInt32();
        return;
    }

    const SalDisplay *pDisplay = GetDisplay();
    if (!pDisplay)
    {
        SAL_WARN( "vcl", "Null display");
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
#if ENABLE_CAIRO_CANVAS
    maClipRegion.SetNull();
#endif
    mxImpl->ResetClipRegion();
}

bool X11SalGraphics::setClipRegion( const vcl::Region& i_rClip )
{
#if ENABLE_CAIRO_CANVAS
    maClipRegion = i_rClip;
#endif
    return mxImpl->setClipRegion( i_rClip );
}

void X11SalGraphics::SetLineColor()
{
#if ENABLE_CAIRO_CANVAS
    mnPenColor = SALCOLOR_NONE;
#endif // ENABLE_CAIRO_CANVAS

    mxImpl->SetLineColor();
}

void X11SalGraphics::SetLineColor( Color nColor )
{
#if ENABLE_CAIRO_CANVAS
    mnPenColor = nColor;
#endif // ENABLE_CAIRO_CANVAS

    mxImpl->SetLineColor( nColor );
}

void X11SalGraphics::SetFillColor()
{
#if ENABLE_CAIRO_CANVAS
    mnFillColor = SALCOLOR_NONE;
#endif // ENABLE_CAIRO_CANVAS

    mxImpl->SetFillColor();
}

void X11SalGraphics::SetFillColor( Color nColor )
{
#if ENABLE_CAIRO_CANVAS
    mnFillColor = nColor;
#endif // ENABLE_CAIRO_CANVAS

    mxImpl->SetFillColor( nColor );
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

void X11SalGraphics::drawPixel( long nX, long nY, Color nColor )
{
    mxImpl->drawPixel( nX, nY, nColor );
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

bool X11SalGraphics::drawPolyLineBezier( sal_uInt32 nPoints, const SalPoint* pPtAry, const PolyFlags* pFlgAry )
{
    return mxImpl->drawPolyLineBezier( nPoints, pPtAry, pFlgAry );
}

bool X11SalGraphics::drawPolygonBezier( sal_uInt32 nPoints, const SalPoint* pPtAry, const PolyFlags* pFlgAry )
{
    return mxImpl->drawPolygonBezier( nPoints, pPtAry, pFlgAry );
}

bool X11SalGraphics::drawPolyPolygonBezier( sal_uInt32 nPoints, const sal_uInt32* pPoints,
                                                const SalPoint* const* pPtAry, const PolyFlags* const* pFlgAry)
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
        long nHeight, void* pPtr, sal_uInt32 nSize )
{
    return mxImpl->drawEPS( nX, nY, nWidth, nHeight, pPtr, nSize );
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
    aRes.hDrawable = hDrawable_;
    aRes.pVisual   = GetVisual().visual;
    aRes.nScreen   = m_nXScreen.getXScreen();
    aRes.pXRenderFormat = m_pXRenderFormat;
    return aRes;
}

#if ENABLE_CAIRO_CANVAS

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
    cairo::X11SysData getSysData( const vcl::Window& rWindow )
    {
        const SystemEnvData* pSysData = rWindow.GetSystemData();

        if( !pSysData )
            return cairo::X11SysData();
        else
            return cairo::X11SysData(*pSysData);
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
        return cairo::SurfaceSharedPtr(new cairo::X11Surface(getSysData(static_cast<const vcl::Window&>(rRefDevice)),
                                               x,y,width,height));
    if( rRefDevice.IsVirtual() )
        return cairo::SurfaceSharedPtr(new cairo::X11Surface(getSysData(static_cast<const VirtualDevice&>(rRefDevice)),
                                               x,y,width,height));
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
            return cairo::SurfaceSharedPtr(new cairo::X11Surface(getSysData(static_cast<const vcl::Window&>(rRefDevice)), rData ));
        else if( rRefDevice.IsVirtual() )
            return cairo::SurfaceSharedPtr(new cairo::X11Surface(getSysData(static_cast<const VirtualDevice&>(rRefDevice)), rData ));
    }

    return cairo::SurfaceSharedPtr();
}

css::uno::Any X11SalGraphics::GetNativeSurfaceHandle(cairo::SurfaceSharedPtr& rSurface, const basegfx::B2ISize& /*rSize*/) const
{
    cairo::X11Surface& rXlibSurface=dynamic_cast<cairo::X11Surface&>(*rSurface);
    css::uno::Sequence< css::uno::Any > args( 3 );
    args[0] <<= false;  // do not call XFreePixmap on it
    args[1] <<= rXlibSurface.getPixmap()->mhDrawable;
    args[2] <<= sal_Int32( rXlibSurface.getDepth() );
    return css::uno::Any(args);
}

#endif // ENABLE_CAIRO_CANVAS

// draw a poly-polygon
bool X11SalGraphics::drawPolyPolygon(
    const basegfx::B2DHomMatrix& rObjectToDevice,
    const basegfx::B2DPolyPolygon& rPolyPolygon,
    double fTransparency)
{
    if(fTransparency >= 1.0)
    {
        return true;
    }

    if(rPolyPolygon.count() == 0)
    {
        return true;
    }

#if ENABLE_CAIRO_CANVAS
    // Fallback: Transform to DeviceCoordinates
    basegfx::B2DPolyPolygon aPolyPolygon(rPolyPolygon);
    aPolyPolygon.transform(rObjectToDevice);

    if(SALCOLOR_NONE == mnFillColor && SALCOLOR_NONE == mnPenColor)
    {
        return true;
    }

    // enable by setting to something
    static const char* pUseCairoForPolygons(getenv("SAL_ENABLE_USE_CAIRO_FOR_POLYGONS"));

    if (!m_bOpenGL && nullptr != pUseCairoForPolygons && SupportsCairo())
    {
        // snap to raster if requested
        const bool bSnapPoints(!getAntiAliasB2DDraw());

        if(bSnapPoints)
        {
            aPolyPolygon = basegfx::utils::snapPointsOfHorizontalOrVerticalEdges(aPolyPolygon);
        }

        cairo_t* cr = getCairoContext();
        clipRegion(cr);

        for(auto const& rPolygon : aPolyPolygon)
        {
            const sal_uInt32 nPointCount(rPolygon.count());

            if(nPointCount)
            {
                const sal_uInt32 nEdgeCount(rPolygon.isClosed() ? nPointCount : nPointCount - 1);

                if(nEdgeCount)
                {
                    basegfx::B2DCubicBezier aEdge;

                    for(sal_uInt32 b = 0; b < nEdgeCount; ++b)
                    {
                        rPolygon.getBezierSegment(b, aEdge);

                        if(!b)
                        {
                            const basegfx::B2DPoint aStart(aEdge.getStartPoint());
                            cairo_move_to(cr, aStart.getX(), aStart.getY());
                        }

                        const basegfx::B2DPoint aEnd(aEdge.getEndPoint());

                        if(aEdge.isBezier())
                        {
                            const basegfx::B2DPoint aCP1(aEdge.getControlPointA());
                            const basegfx::B2DPoint aCP2(aEdge.getControlPointB());
                            cairo_curve_to(cr,
                                aCP1.getX(), aCP1.getY(),
                                aCP2.getX(), aCP2.getY(),
                                aEnd.getX(), aEnd.getY());
                        }
                        else
                        {
                            cairo_line_to(cr, aEnd.getX(), aEnd.getY());
                        }
                    }

                    cairo_close_path(cr);
                }
            }
        }

        if(SALCOLOR_NONE != mnFillColor)
        {
            cairo_set_source_rgba(cr,
                mnFillColor.GetRed()/255.0,
                mnFillColor.GetGreen()/255.0,
                mnFillColor.GetBlue()/255.0,
                1.0 - fTransparency);
            cairo_set_fill_rule(cr, CAIRO_FILL_RULE_EVEN_ODD);
            cairo_fill_preserve(cr);
        }

        if(SALCOLOR_NONE != mnPenColor)
        {
            cairo_set_source_rgba(cr,
                    mnPenColor.GetRed()/255.0,
                    mnPenColor.GetGreen()/255.0,
                    mnPenColor.GetBlue()/255.0,
                    1.0 - fTransparency);
            cairo_stroke_preserve(cr);
        }

        releaseCairoContext(cr);
        return true;
    }
#endif // ENABLE_CAIRO_CANVAS

    return mxImpl->drawPolyPolygon(
        rObjectToDevice,
        rPolyPolygon,
        fTransparency);
}

#if ENABLE_CAIRO_CANVAS
void X11SalGraphics::clipRegion(cairo_t* cr)
{
    SvpSalGraphics::clipRegion(cr, maClipRegion);
}
#endif // ENABLE_CAIRO_CANVAS

bool X11SalGraphics::drawPolyLine(
    const basegfx::B2DHomMatrix& rObjectToDevice,
    const basegfx::B2DPolygon& rPolygon,
    double fTransparency,
    const basegfx::B2DVector& rLineWidth,
    basegfx::B2DLineJoin eLineJoin,
    css::drawing::LineCap eLineCap,
    double fMiterMinimumAngle,
    bool bPixelSnapHairline)
{
    if(0 == rPolygon.count())
    {
        return true;
    }

    if(fTransparency >= 1.0)
    {
        return true;
    }

#if ENABLE_CAIRO_CANVAS
    // disable by setting to something
    static const char* pUseCairoForFatLines(getenv("SAL_DISABLE_USE_CAIRO_FOR_FATLINES"));

    if (!m_bOpenGL && nullptr == pUseCairoForFatLines && SupportsCairo())
    {
        cairo_t* cr = getCairoContext();
        clipRegion(cr);

        // Use the now available static drawPolyLine from the Cairo-Headless-Fallback
        // that will take care of all needed stuff
        const bool bRetval(
            SvpSalGraphics::drawPolyLine(
                cr,
                nullptr,
                mnPenColor,
                getAntiAliasB2DDraw(),
                rObjectToDevice,
                rPolygon,
                fTransparency,
                rLineWidth,
                eLineJoin,
                eLineCap,
                fMiterMinimumAngle,
                bPixelSnapHairline));

        releaseCairoContext(cr);

        if(bRetval)
        {
            return true;
        }
    }
#endif // ENABLE_CAIRO_CANVAS

    return mxImpl->drawPolyLine(
        rObjectToDevice,
        rPolygon,
        fTransparency,
        rLineWidth,
        eLineJoin,
        eLineCap,
        fMiterMinimumAngle,
        bPixelSnapHairline);
}

bool X11SalGraphics::drawGradient(const tools::PolyPolygon& rPoly, const Gradient& rGradient)
{
    return mxImpl->drawGradient(rPoly, rGradient);
}

SalGeometryProvider *X11SalGraphics::GetGeometryProvider() const
{
    if (m_pFrame)
        return static_cast< SalGeometryProvider * >(m_pFrame);
    else
        return static_cast< SalGeometryProvider * >(m_pVDev);
}

cairo_t* X11SalGraphics::getCairoContext()
{
    cairo_surface_t* surface = cairo_xlib_surface_create(GetXDisplay(), hDrawable_,
            GetVisual().visual, SAL_MAX_INT16, SAL_MAX_INT16);

    cairo_t *cr = cairo_create(surface);
    cairo_surface_destroy(surface);

    return cr;
}

void X11SalGraphics::releaseCairoContext(cairo_t* cr)
{
   cairo_destroy(cr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
