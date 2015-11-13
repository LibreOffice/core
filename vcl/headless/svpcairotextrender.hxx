/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_UNX_GTK3_GDI_GTK3CAIROTEXTRENDER_HXX
#define INCLUDED_VCL_UNX_GTK3_GDI_GTK3CAIROTEXTRENDER_HXX

#include "cairotextrender.hxx"

class SvpSalGraphics;

class SvpCairoTextRender : public CairoTextRender
{
protected:
    basebmp::BitmapDeviceSharedPtr mxTmpSurface;
    SvpSalGraphics& mrParent;

public:
    explicit SvpCairoTextRender(SvpSalGraphics& rParent);

    virtual GlyphCache& getPlatformGlyphCache() override;
    virtual cairo_t* getCairoContext() override;
    virtual void getSurfaceOffset(double& nDX, double& nDY) override;
    virtual void clipRegion(cairo_t* cr) override;
    virtual void drawSurface(cairo_t* cr) override;
    virtual basebmp::BitmapDeviceSharedPtr createSimpleMask() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
