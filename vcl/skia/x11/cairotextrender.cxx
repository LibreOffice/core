/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <skia/x11/cairotextrender.hxx>

#include <salbmp.hxx>
#include <skia/gdiimpl.hxx>

#include <cairo.h>

SkiaX11CairoTextRender::SkiaX11CairoTextRender(X11SalGraphics& rParent)
    : X11CairoTextRender(rParent)
{
}

cairo_t* SkiaX11CairoTextRender::getCairoContext()
{
    cairo_surface_t* surface = nullptr;
    SkiaSalGraphicsImpl* pImpl = dynamic_cast<SkiaSalGraphicsImpl*>(mrParent.GetImpl());
    if (pImpl)
    {
        tools::Rectangle aClipRect = pImpl->getClipRegion().GetBoundRect();
        if (aClipRect.GetWidth() == 0 || aClipRect.GetHeight() == 0)
        {
            aClipRect.setWidth(GetWidth());
            aClipRect.setHeight(GetHeight());
        }
        surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, aClipRect.GetWidth(),
                                             aClipRect.GetHeight());
    }
    if (!surface)
        return nullptr;
    cairo_t* cr = cairo_create(surface);
    // TODO SKIA
    cairo_surface_destroy(surface);
    return cr;
}

void SkiaX11CairoTextRender::getSurfaceOffset(double& nDX, double& nDY)
{
    SkiaSalGraphicsImpl* pImpl = dynamic_cast<SkiaSalGraphicsImpl*>(mrParent.GetImpl());
    if (pImpl)
    {
        tools::Rectangle aClipRect = pImpl->getClipRegion().GetBoundRect();
        nDX = -aClipRect.Left();
        nDY = -aClipRect.Top();
    }
}

void SkiaX11CairoTextRender::releaseCairoContext(cairo_t* cr)
{
    SkiaSalGraphicsImpl* pImpl = dynamic_cast<SkiaSalGraphicsImpl*>(mrParent.GetImpl());
    if (!pImpl)
    {
        cairo_destroy(cr);
        return;
    }

    cairo_surface_t* pSurface = cairo_get_target(cr);
    int nWidth = cairo_image_surface_get_width(pSurface);
    int nHeight = cairo_image_surface_get_height(pSurface);
    cairo_surface_flush(pSurface);
    unsigned char* pSrc = cairo_image_surface_get_data(pSurface);

    tools::Rectangle aClipRect = pImpl->getClipRegion().GetBoundRect();

    SalTwoRect aRect(0, 0, nWidth, nHeight, aClipRect.Left(), aClipRect.Top(), nWidth, nHeight);

    // Cairo surface data is ARGB with premultiplied alpha and is Y-inverted
    // TODO SKIA
    //    SkiaTexture aTexture( nWidth, nHeight, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, pSrc );
    //    pImpl->PreDraw();
    //    pImpl->DrawAlphaTexture( aTexture, aRect, true, true );
    //    pImpl->PostDraw();
    //    abort();
    (void)pSrc;

    cairo_destroy(cr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
