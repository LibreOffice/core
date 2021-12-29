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

#include <memory>
#include <numeric>

#include <headless/svpgdi.hxx>
#include <headless/svpbmp.hxx>
#include <headless/svpframe.hxx>
#include <headless/svpcairotextrender.hxx>
#include <headless/BitmapHelper.hxx>
#include <saldatabasic.hxx>

#include <sal/log.hxx>
#include <tools/helpers.hxx>
#include <o3tl/safeint.hxx>
#include <vcl/BitmapTools.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/gradient.hxx>
#include <config_cairo_canvas.h>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2ibox.hxx>
#include <basegfx/range/b2irange.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <comphelper/lok.hxx>
#include <unx/gendata.hxx>
#include <dlfcn.h>

#if ENABLE_CAIRO_CANVAS
#   if defined CAIRO_VERSION && CAIRO_VERSION < CAIRO_VERSION_ENCODE(1, 10, 0)
#      define CAIRO_OPERATOR_DIFFERENCE (static_cast<cairo_operator_t>(23))
#   endif
#endif

SvpSalGraphics::SvpSalGraphics()
    : m_aTextRenderImpl(*this)
    , m_pBackend(new SvpGraphicsBackend(m_aCairoCommon))
{
    bool bLOKActive = comphelper::LibreOfficeKit::isActive();
    initWidgetDrawBackends(bLOKActive);
}

SvpSalGraphics::~SvpSalGraphics()
{
    ReleaseFonts();
}

void SvpSalGraphics::setSurface(cairo_surface_t* pSurface, const basegfx::B2IVector& rSize)
{
    m_aCairoCommon.m_pSurface = pSurface;
    m_aCairoCommon.m_aFrameSize = rSize;
    dl_cairo_surface_get_device_scale(pSurface, &m_aCairoCommon.m_fScale, nullptr);
    ResetClipRegion();
}

void SvpSalGraphics::GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY )
{
    rDPIX = rDPIY = 96;
}

std::shared_ptr<SalBitmap> SvpSalGraphics::getBitmap( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight )
{
    std::shared_ptr<SvpSalBitmap> pBitmap = std::make_shared<SvpSalBitmap>();
    BitmapPalette aPal;
    vcl::PixelFormat ePixelFormat = vcl::PixelFormat::INVALID;
    if (GetBitCount() == 1)
    {
        ePixelFormat = vcl::PixelFormat::N1_BPP;
        aPal.SetEntryCount(2);
        aPal[0] = COL_BLACK;
        aPal[1] = COL_WHITE;
    }
    else
    {
        ePixelFormat = vcl::PixelFormat::N32_BPP;
    }

    if (!pBitmap->Create(Size(nWidth, nHeight), ePixelFormat, aPal))
    {
        SAL_WARN("vcl.gdi", "SvpSalGraphics::getBitmap, cannot create bitmap");
        return nullptr;
    }

    cairo_surface_t* target = CairoCommon::createCairoSurface(pBitmap->GetBuffer());
    if (!target)
    {
        SAL_WARN("vcl.gdi", "SvpSalGraphics::getBitmap, cannot create cairo surface");
        return nullptr;
    }
    cairo_t* cr = cairo_create(target);

    SalTwoRect aTR(nX, nY, nWidth, nHeight, 0, 0, nWidth, nHeight);
    CairoCommon::renderSource(cr, aTR, m_aCairoCommon.m_pSurface);

    cairo_destroy(cr);
    cairo_surface_destroy(target);

    Toggle1BitTransparency(*pBitmap->GetBuffer());

    return pBitmap;
}

#if ENABLE_CAIRO_CANVAS
bool SvpSalGraphics::SupportsCairo() const
{
    return false;
}

cairo::SurfaceSharedPtr SvpSalGraphics::CreateSurface(const cairo::CairoSurfaceSharedPtr& /*rSurface*/) const
{
    return cairo::SurfaceSharedPtr();
}

cairo::SurfaceSharedPtr SvpSalGraphics::CreateSurface(const OutputDevice& /*rRefDevice*/, int /*x*/, int /*y*/, int /*width*/, int /*height*/) const
{
    return cairo::SurfaceSharedPtr();
}

cairo::SurfaceSharedPtr SvpSalGraphics::CreateBitmapSurface(const OutputDevice& /*rRefDevice*/, const BitmapSystemData& /*rData*/, const Size& /*rSize*/) const
{
    return cairo::SurfaceSharedPtr();
}

css::uno::Any SvpSalGraphics::GetNativeSurfaceHandle(cairo::SurfaceSharedPtr& /*rSurface*/, const basegfx::B2ISize& /*rSize*/) const
{
    return css::uno::Any();
}

#endif // ENABLE_CAIRO_CANVAS

SystemGraphicsData SvpSalGraphics::GetGraphicsData() const
{
    return SystemGraphicsData();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
