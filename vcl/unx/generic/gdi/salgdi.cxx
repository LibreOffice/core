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

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>

#include <headless/svpgdi.hxx>

#include <vcl/sysdata.hxx>
#include <vcl/virdev.hxx>
#include <sal/log.hxx>
#include <o3tl/string_view.hxx>

#include <unx/salunx.h>
#include <unx/saldisp.hxx>
#include <unx/salgdi.h>
#include <unx/x11/xlimits.hxx>

#include <salframe.hxx>
#include <salgdiimpl.hxx>
#include <textrender.hxx>
#include <salvd.hxx>

#include <unx/salframe.h>
#include <unx/cairotextrender.hxx>
#include <cairo-xlib.h>

#include "X11CairoSalGraphicsImpl.hxx"


// X11Common

X11Common::X11Common()
    : m_hDrawable(None)
    , m_pColormap(nullptr)
{}

// X11SalGraphics

X11SalGraphics::X11SalGraphics():
    m_pFrame(nullptr),
    m_pVDev(nullptr),
    m_nXScreen( 0 )
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
        mxImpl.reset(new X11CairoSalGraphicsImpl(*this, maCairoCommon));
        mxTextRenderImpl.reset(new CairoTextRender(maCairoCommon));
    }
}

X11SalGraphics::~X11SalGraphics() COVERITY_NOEXCEPT_FALSE
{
    mxImpl->DeInit();
    SetDrawable(None, nullptr, m_nXScreen);
    ReleaseFonts();
    FreeColorMap();
}

void X11SalGraphics::FreeColorMap()
{
    if( m_pDeleteColormap )
    {
        m_pDeleteColormap.reset();
        maX11Common.m_pColormap = nullptr;
    }
}

SalGraphicsImpl* X11SalGraphics::GetImpl() const
{
    return mxImpl.get();
}

void X11SalGraphics::SetDrawable(Drawable aDrawable, cairo_surface_t* pSurface, SalX11Screen nXScreen)
{
    maCairoCommon.m_pSurface = pSurface;
    if (maCairoCommon.m_pSurface)
    {
        maCairoCommon.m_aFrameSize.setX(cairo_xlib_surface_get_width(pSurface));
        maCairoCommon.m_aFrameSize.setY(cairo_xlib_surface_get_height(pSurface));
        dl_cairo_surface_get_device_scale(pSurface, &maCairoCommon.m_fScale, nullptr);
    }

    // shortcut if nothing changed
    if( maX11Common.m_hDrawable == aDrawable )
        return;

    // free screen specific resources if needed
    if( nXScreen != m_nXScreen )
    {
        FreeColorMap();
        maX11Common.m_pColormap = &vcl_sal::getSalDisplay(GetGenericUnixSalData())->GetColormap( nXScreen );
        m_nXScreen = nXScreen;
    }

    maX11Common.m_hDrawable = aDrawable;
}

void X11SalGraphics::Init( X11SalFrame& rFrame, Drawable aTarget,
                           SalX11Screen nXScreen )
{
    maX11Common.m_pColormap = &vcl_sal::getSalDisplay(GetGenericUnixSalData())->GetColormap(nXScreen);
    m_nXScreen  = nXScreen;

    m_pFrame    = &rFrame;
    m_pVDev     = nullptr;

    SetDrawable(aTarget, rFrame.GetSurface(), nXScreen);
    mxImpl->UpdateX11GeometryProvider();
}

void X11SalGraphics::GetResolution( sal_Int32 &rDPIX, sal_Int32 &rDPIY ) // const
{
    char* pForceDpi;
    if ((pForceDpi = getenv("SAL_FORCEDPI")))
    {
        rDPIX = rDPIY = o3tl::toInt32(std::string_view(pForceDpi));
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
    if( rDPIX == rDPIY )
        return;

    // different x- and y- resolutions are usually artifacts of
    // a wrongly calculated screen size.
#if OSL_DEBUG_LEVEL >= 2
    SAL_INFO("vcl.gdi", "Forcing Resolution from "
        << std::hex << rDPIX
        << std::dec << rDPIX
        << " to "
        << std::hex << rDPIY
        << std::dec << rDPIY);
#endif
    rDPIX = rDPIY; // y-resolution is more trustworthy
}

SystemGraphicsData X11SalGraphics::GetGraphicsData() const
{
    SystemGraphicsData aRes;

    aRes.nSize = sizeof(aRes);
    return aRes;
}

void X11SalGraphics::Flush()
{
    if( X11GraphicsImpl* x11Impl = dynamic_cast< X11GraphicsImpl* >( mxImpl.get()))
        x11Impl->Flush();
}

SalGeometryProvider *X11SalGraphics::GetGeometryProvider() const
{
    if (m_pFrame)
        return static_cast< SalGeometryProvider * >(m_pFrame);
    else
        return static_cast< SalGeometryProvider * >(m_pVDev);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
