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

#ifndef INCLUDED_VCL_UNX_GENERIC_GDI_X11CAIROTEXTRENDER_HXX
#define INCLUDED_VCL_UNX_GENERIC_GDI_X11CAIROTEXTRENDER_HXX

#include "cairotextrender.hxx"

#include "unx/saldata.hxx"
#include "unx/saldisp.hxx"

#include "unx/salgdi.h"

class X11CairoTextRender : public CairoTextRender
{
protected:
    X11SalGraphics& mrParent;

protected:
    size_t GetWidth() const;
    size_t GetHeight() const;

public:
    explicit X11CairoTextRender(X11SalGraphics& rParent);

    virtual GlyphCache& getPlatformGlyphCache() override;
    virtual cairo_t* getCairoContext() override;
    virtual void getSurfaceOffset(double& nDX, double& nDY) override;
    virtual void clipRegion(cairo_t* cr) override;
    virtual void drawSurface(cairo_t* cr) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
