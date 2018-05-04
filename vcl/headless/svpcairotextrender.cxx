/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <headless/svpcairotextrender.hxx>
#include <headless/svpgdi.hxx>
#include <cairo.h>

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
    cairo_t* cr = mrParent.getCairoContext(false);

    //rhbz#1283420 tdf#117413 bodge to force a read from the underlying surface which has
    //the side effect of making the mysterious xrender related problem go away
    {
        cairo_surface_t *target = cairo_get_target(cr);
        cairo_surface_t *throw_away = cairo_surface_create_similar(target, cairo_surface_get_content(target), 1, 1);
        cairo_t *force_read_cr = cairo_create(throw_away);
        cairo_set_source_surface(force_read_cr, target, 0, 0);
        cairo_paint(force_read_cr);
        cairo_destroy(force_read_cr);
        cairo_surface_destroy(throw_away);
    }

    return cr;
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

void SvpCairoTextRender::releaseCairoContext(cairo_t* cr)
{
    mrParent.releaseCairoContext(cr, false, basegfx::B2DRange());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
