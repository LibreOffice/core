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

cairo_surface_t* OpenGLX11CairoTextRender::getCairoSurface( const ServerFontLayout& rLayout )
{
    // static size_t id = 0;
    // OString aFileName = OString("/tmp/libo_logs/text_rendering") + OString::number(id++) + OString(".svg");
    // cairo_surface_t* surface = cairo_svg_surface_create(aFileName.getStr(), GetWidth(), GetHeight());
    cairo_surface_t* surface = NULL;
    Rectangle aTextBoundRect;
    OpenGLSalGraphicsImpl *pImpl = dynamic_cast< OpenGLSalGraphicsImpl* >(mrParent.GetImpl());

    if (pImpl)
    {
        Rectangle aClipRect = pImpl->getClipRegion().GetBoundRect();

        // no clipping area, we need to take the whole area
        if ( aClipRect.IsEmpty() )
        {
            aClipRect.setWidth(GetWidth());
            aClipRect.setHeight(GetHeight());
        }

        if (!rLayout.GetOrientation() )
        {
            // GetBoundRect can fail!
            if ( rLayout.GetBoundRect(mrParent, aTextBoundRect) )
            {
                // we need to take into account SalLayout's drawing offset
                int nOffsetX = rLayout.DrawOffset().X() - rLayout.DrawBase().X();
                int nOffsetY = rLayout.DrawOffset().Y() - rLayout.DrawBase().Y();

                aTextBoundRect.Left() += nOffsetX;
                aTextBoundRect.Top() += nOffsetY;

                aTextBoundRect = aTextBoundRect.Intersection(aClipRect);
            }
            else
            {
                SAL_WARN("vcl.layout", "GetBoundRect() failed");
            }
        }
        else
        {
            aTextBoundRect = aClipRect;
        }
    }
    else
    {
        SAL_WARN("vcl.opengl", "No OpenGLSalGraphicsImpl!");
        return NULL;
    }


    double nDX=0, nDY=0;
    getSurfaceOffset(nDX, nDY);

    // add cairo surface offsets to work out surface width and height
    surface = cairo_image_surface_create( CAIRO_FORMAT_ARGB32, aTextBoundRect.GetWidth() + nDX,
                                                               aTextBoundRect.GetHeight() + nDY );

    return surface;
}

void OpenGLX11CairoTextRender::getSurfaceOffset( double& nDX, double& nDY )
{
    OpenGLSalGraphicsImpl *pImpl = dynamic_cast< OpenGLSalGraphicsImpl* >(mrParent.GetImpl());
    if( pImpl )
    {
        Rectangle aClipRect = pImpl->getClipRegion().GetBoundRect();
        nDX = -aClipRect.Left();
        nDY = -aClipRect.Top();
    }
}

void OpenGLX11CairoTextRender::drawSurface(cairo_t* cr)
{
    // XXX: lfrb: GLES 2.0 doesn't support GL_UNSIGNED_INT_8_8_8_8_REV
    OpenGLSalGraphicsImpl *pImpl = dynamic_cast< OpenGLSalGraphicsImpl* >(mrParent.GetImpl());
    if(!pImpl)
        return;

    cairo_surface_t* pSurface = cairo_get_target(cr);
    int nWidth = cairo_image_surface_get_width( pSurface );
    int nHeight = cairo_image_surface_get_height( pSurface );
    cairo_surface_flush( pSurface );
    unsigned char *pSrc = cairo_image_surface_get_data( pSurface );

    // XXX: lfrb: GLES 2.0 doesn't support GL_UNSIGNED_INT_8_8_8_8_REV
    Rectangle aClipRect = pImpl->getClipRegion().GetBoundRect();

    SalTwoRect aRect(0, 0, nWidth, nHeight,
            aClipRect.Left(), aClipRect.Top(), nWidth, nHeight);

    // Cairo surface data is ARGB with premultiplied alpha and is Y-inverted
    OpenGLTexture aTexture( nWidth, nHeight, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, pSrc );
    pImpl->PreDraw();
    pImpl->DrawAlphaTexture( aTexture, aRect, true, true );
    pImpl->PostDraw();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
