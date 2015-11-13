/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "svpcairotextrender.hxx"
#include "headless/svpgdi.hxx"

SvpCairoTextRender::SvpCairoTextRender(SvpSalGraphics& rParent)
    : mrParent(rParent)
{
}

GlyphCache& SvpCairoTextRender::getPlatformGlyphCache()
{
    return SvpSalGraphics::getPlatformGlyphCache();
}

cairo_t* SvpCairoTextRender::getCairoContext()
{
    cairo_t* pRet = mrParent.getCairoContext();
    assert(pRet);
    if (!pRet)
    {
        mxTmpSurface = mrParent.createSimpleMask();
        pRet = SvpSalGraphics::createCairoContext(mxTmpSurface);
    }
    return pRet;
}

void SvpCairoTextRender::getSurfaceOffset(double& nDX, double& nDY)
{
    nDX = 0;
    nDY = 0;
}

void SvpCairoTextRender::clipRegion(cairo_t* cr)
{
    mrParent.clipRegion(cr);
}

basebmp::BitmapDeviceSharedPtr SvpCairoTextRender::createSimpleMask()
{
    return mrParent.createSimpleMask();
}

void SvpCairoTextRender::drawSurface(cairo_t*)
{
    //typically we have drawn directly to the real surface, in edge-cases of
    //strange surface depths, we'll have drawn into a tmp surface, so flush
    //it
    if (mxTmpSurface)
    {
        // blend text color into target using the glyph's mask
        mrParent.BlendTextColor(basebmp::Color(GetTextColor()), mxTmpSurface, basegfx::B2IPoint(0, 0));
        mxTmpSurface.reset();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
