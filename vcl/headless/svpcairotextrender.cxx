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

cairo_t* SvpCairoTextRender::getCairoContext() { return mrParent.getCairoContext(false); }

void SvpCairoTextRender::getSurfaceOffset(double& nDX, double& nDY)
{
    nDX = 0;
    nDY = 0;
}

void SvpCairoTextRender::clipRegion(cairo_t* cr) { mrParent.clipRegion(cr); }

void SvpCairoTextRender::releaseCairoContext(cairo_t* cr)
{
    mrParent.releaseCairoContext(cr, false, basegfx::B2DRange());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
