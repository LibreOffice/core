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

#include <config_features.h>
#include <config_cairo_canvas.h>

#if ENABLE_CAIRO_CANVAS

#include <cairo-xlib.h>
#include <unx/salgdi.h>
#include "gdiimpl.hxx"
#include "cairo_xlib_cairo.hxx"

#include <headless/CairoCommon.hxx>

class X11CairoSalGraphicsImpl : public X11SalGraphicsImpl
{
private:
    X11Common& mrX11Common;
    vcl::Region maClipRegion;
    Color mnPenColor;
    Color mnFillColor;
    _cairo_fill_rule meFillRule;

    using X11SalGraphicsImpl::drawPolyPolygon;
    using X11SalGraphicsImpl::drawPolyLine;

public:
    X11CairoSalGraphicsImpl(X11SalGraphics& rParent, X11Common& rX11Common);

    void ResetClipRegion() override
    {
        maClipRegion.SetNull();
        X11SalGraphicsImpl::ResetClipRegion();
    }

    bool setClipRegion(const vcl::Region& i_rClip) override
    {
        maClipRegion = i_rClip;
        return X11SalGraphicsImpl::setClipRegion(i_rClip);
    }

    void SetLineColor() override
    {
        mnPenColor = SALCOLOR_NONE;
        X11SalGraphicsImpl::SetLineColor();
    }

    void SetLineColor(Color nColor) override
    {
        mnPenColor = nColor;
        X11SalGraphicsImpl::SetLineColor(nColor);
    }

    void SetFillColor() override
    {
        mnFillColor = SALCOLOR_NONE;
        X11SalGraphicsImpl::SetFillColor();
    }

    void SetFillColor(Color nColor) override
    {
        mnFillColor = nColor;
        X11SalGraphicsImpl::SetFillColor(nColor);
    }

    void SetFillRule() override
    {
        meFillRule = _cairo_fill_rule::CAIRO_FILL_RULE_EVEN_ODD;
        X11SalGraphicsImpl::SetFillRule();
    }

    void SetFillRule(PolyFillMode eFillRule) override
    {
        if (eFillRule == PolyFillMode::NON_ZERO_RULE_WINDING)
            meFillRule = _cairo_fill_rule::CAIRO_FILL_RULE_WINDING;
        else
            meFillRule = _cairo_fill_rule::CAIRO_FILL_RULE_EVEN_ODD;
        X11SalGraphicsImpl::SetFillRule(eFillRule);
    }

    void clipRegion(cairo_t* cr) { CairoCommon::clipRegion(cr, maClipRegion); }

    bool drawPolyPolygon(const basegfx::B2DHomMatrix& rObjectToDevice,
                         const basegfx::B2DPolyPolygon& rPolyPolygon,
                         double fTransparency) override;

    bool drawPolyLine(const basegfx::B2DHomMatrix& rObjectToDevice,
                      const basegfx::B2DPolygon& rPolygon, double fTransparency, double fLineWidth,
                      const std::vector<double>* pStroke, basegfx::B2DLineJoin eLineJoin,
                      css::drawing::LineCap eLineCap, double fMiterMinimumAngle,
                      bool bPixelSnapHairline) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
