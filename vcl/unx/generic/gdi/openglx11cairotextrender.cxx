/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "openglx11cairotextrender.hxx"

#include "openglgdiimpl.hxx"
#include "salbmp.hxx"
#include <vcl/salbtype.hxx>

#include <cairo-svg.h>

OpenGLX11CairoTextRender::OpenGLX11CairoTextRender(bool bPrinter, X11SalGraphics& rParent):
    X11CairoTextRender(bPrinter, rParent)
{
}

cairo_surface_t* OpenGLX11CairoTextRender::getCairoSurface()
{
    // static size_t id = 0;
    // OString aFileName = OString("/tmp/libo_logs/text_rendering") + OString::number(id++) + OString(".svg");
    // cairo_surface_t* surface = cairo_svg_surface_create(aFileName.getStr(), GetWidth(), GetHeight());
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, GetWidth(), GetHeight());
    return surface;
}

void OpenGLX11CairoTextRender::drawSurface(cairo_t* cr)
{
    cairo_surface_t* pSurface = cairo_get_target(cr);
    int nWidth = cairo_image_surface_get_width( pSurface );
    int nHeight = cairo_image_surface_get_height( pSurface );
    cairo_surface_flush( pSurface );
    unsigned char *pSrc = cairo_image_surface_get_data( pSurface );

    SalTwoRect aRect;
    aRect.mnSrcX = 0;
    aRect.mnSrcY = 0;
    aRect.mnSrcWidth = nWidth;
    aRect.mnSrcHeight = nHeight;
    aRect.mnDestX = 0;
    aRect.mnDestY = 0;
    aRect.mnDestWidth = nWidth;
    aRect.mnDestHeight = nHeight;

    // XXX: lfrb: GLES 2.0 doesn't support GL_UNSIGNED_INT_8_8_8_8_REV
    OpenGLSalGraphicsImpl *pImpl = dynamic_cast< OpenGLSalGraphicsImpl* >(mrParent.GetImpl());
    if( pImpl )
    {
        // Cairo surface data is ARGB with premultiplied alpha and is Y-inverted
        OpenGLTexture aTexture( nWidth, nHeight, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, pSrc );
        pImpl->PreDraw();
        pImpl->DrawAlphaTexture( aTexture, aRect, true, true );
        pImpl->PostDraw();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
