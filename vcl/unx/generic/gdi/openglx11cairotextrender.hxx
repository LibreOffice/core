/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_UNX_GENERIC_GDI_OPENGLX11CAIROTEXTRENDER_HXX
#define INCLUDED_VCL_UNX_GENERIC_GDI_OPENGLX11CAIROTEXTRENDER_HXX

#include "x11cairotextrender.hxx"

class OpenGLX11CairoTextRender : public X11CairoTextRender
{
public:
    explicit OpenGLX11CairoTextRender(X11SalGraphics& rParent);

    virtual cairo_t* getCairoContext() override;
    virtual void getSurfaceOffset(double& nDX, double& nDY) override;
    virtual void drawSurface(cairo_t* cr) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
