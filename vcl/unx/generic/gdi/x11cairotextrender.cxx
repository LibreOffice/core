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

#include "x11cairotextrender.hxx"
#include "unx/saldata.hxx"
#include "unx/saldisp.hxx"
#include "unx/salvd.h"

#include "gcach_xpeer.hxx"

#include <cairo.h>
#include <cairo-ft.h>

#include <cairo-xlib.h>
#include <cairo-xlib-xrender.h>

struct BOX
{
    short x1, x2, y1, y2;
};
struct _XRegion
{
    long size;
    long numRects;
    BOX *rects;
    BOX extents;
};

X11CairoTextRender::X11CairoTextRender(X11SalGraphics& rParent)
    : mrParent(rParent)
{
}

GlyphCache& X11CairoTextRender::getPlatformGlyphCache()
{
    return X11GlyphCache::GetInstance();
}

cairo_t* X11CairoTextRender::getCairoContext()
{
    // find a XRenderPictFormat compatible with the Drawable
    XRenderPictFormat* pVisualFormat = mrParent.GetXRenderFormat();

    Display* pDisplay = mrParent.GetXDisplay();

    cairo_surface_t* surface = nullptr;
    if (pVisualFormat)
    {
        surface = cairo_xlib_surface_create_with_xrender_format (
                        pDisplay, mrParent.hDrawable_,
                        ScreenOfDisplay(pDisplay, mrParent.m_nXScreen.getXScreen()),
                        pVisualFormat, SAL_MAX_INT16, SAL_MAX_INT16);
    }
    else
    {
        surface = cairo_xlib_surface_create(pDisplay, mrParent.hDrawable_,
            mrParent.GetVisual().visual, SAL_MAX_INT16, SAL_MAX_INT16);
    }

    if (!surface)
        return nullptr;

    cairo_t *cr = cairo_create(surface);
    cairo_surface_destroy(surface);
    return cr;
}

void X11CairoTextRender::getSurfaceOffset( double& nDX, double& nDY )
{
    nDX = 0;
    nDY = 0;
}

void X11CairoTextRender::clipRegion(cairo_t* cr)
{
    Region pClipRegion = mrParent.mpClipRegion;
    if( pClipRegion && !XEmptyRegion( pClipRegion ) )
    {
        for (long i = 0; i < pClipRegion->numRects; ++i)
        {
            cairo_rectangle(cr,
                pClipRegion->rects[i].x1,
                pClipRegion->rects[i].y1,
                pClipRegion->rects[i].x2 - pClipRegion->rects[i].x1,
                pClipRegion->rects[i].y2 - pClipRegion->rects[i].y1);
        }
        cairo_clip(cr);
    }
}

size_t X11CairoTextRender::GetWidth() const
{
    SalGeometryProvider *pProvider = mrParent.m_pFrame;
    if( !pProvider )
        pProvider = mrParent.m_pVDev;

    if( pProvider )
        return pProvider->GetWidth();
    else
        return 1;
}

size_t X11CairoTextRender::GetHeight() const
{
    SalGeometryProvider *pProvider = mrParent.m_pFrame;
    if( !pProvider )
        pProvider = mrParent.m_pVDev;

    if( pProvider )
        return pProvider->GetHeight();
    else
        return 1;
}

void X11CairoTextRender::drawSurface(cairo_t* /*cr*/)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
