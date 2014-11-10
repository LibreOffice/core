/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "openglx11cairotextrender.hxx"

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
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, GetWidth(), GetHeight());
    return surface;
}

void OpenGLX11CairoTextRender::drawSurface(cairo_t* cr)
{
    cairo_surface_t* surface = cairo_get_target(cr);
    int width = cairo_image_surface_get_width(surface);
    int height = cairo_image_surface_get_height(surface);
    SalBitmap* pBitmap = ImplGetSVData()->mpDefInst->CreateSalBitmap();
    pBitmap->Create(Size(width, height), 24, BitmapPalette());
    BitmapBuffer* pBuffer = pBitmap->AcquireBuffer(false);
    std::memcpy(pBuffer->mpBits, cairo_image_surface_get_data(surface), width*height*3);
    pBitmap->ReleaseBuffer(pBuffer, false);
    SalTwoRect aRect;
    aRect.mnSrcX = 0;
    aRect.mnSrcY = 0;
    aRect.mnSrcWidth = width;
    aRect.mnSrcHeight = height;
    mrParent.drawBitmap(aRect, *pBitmap);
    delete pBitmap;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
