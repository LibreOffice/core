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
    std::optional<Color> moPenColor;
    std::optional<Color> moFillColor;

public:
    X11CairoSalGraphicsImpl(X11SalGraphics& rParent, X11Common& rX11Common);

    void ResetClipRegion() override
    {
        maClipRegion.SetNull();
        X11SalGraphicsImpl::ResetClipRegion();
    }

    void setClipRegion(const vcl::Region& i_rClip) override
    {
        maClipRegion = i_rClip;
        X11SalGraphicsImpl::setClipRegion(i_rClip);
    }

    void SetLineColor() override
    {
        moPenColor = std::nullopt;
        X11SalGraphicsImpl::SetLineColor();
    }

    void SetLineColor(Color nColor) override
    {
        moPenColor = nColor;
        X11SalGraphicsImpl::SetLineColor(nColor);
    }

    void SetFillColor() override
    {
        moFillColor = std::nullopt;
        X11SalGraphicsImpl::SetFillColor();
    }

    void SetFillColor(Color nColor) override
    {
        moFillColor = nColor;
        X11SalGraphicsImpl::SetFillColor(nColor);
    }

    void clipRegion(cairo_t* cr) { CairoCommon::clipRegion(cr, maClipRegion); }

    void drawPixel(tools::Long nX, tools::Long nY) override;
    void drawPixel(tools::Long nX, tools::Long nY, Color nColor) override;
    Color getPixel(tools::Long nX, tools::Long nY) override;

    void drawLine(tools::Long nX1, tools::Long nY1, tools::Long nX2, tools::Long nY2) override;

    void drawPolyPolygon(sal_uInt32 nPoly, const sal_uInt32* pPoints,
                         const Point** pPtAry) override;

    bool drawPolyPolygon(const basegfx::B2DHomMatrix& rObjectToDevice,
                         const basegfx::B2DPolyPolygon& rPolyPolygon,
                         double fTransparency) override;

    void drawPolyLine(sal_uInt32 nPoints, const Point* pPtAry) override;

    bool drawPolyLine(const basegfx::B2DHomMatrix& rObjectToDevice,
                      const basegfx::B2DPolygon& rPolygon, double fTransparency, double fLineWidth,
                      const std::vector<double>* pStroke, basegfx::B2DLineJoin eLineJoin,
                      css::drawing::LineCap eLineCap, double fMiterMinimumAngle,
                      bool bPixelSnapHairline) override;

    /** Render solid rectangle with given transparency

        @param nTransparency
        Transparency value (0-255) to use. 0 blits and opaque, 255 a
        fully transparent rectangle
     */
    bool drawAlphaRect(tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight,
                       sal_uInt8 nTransparency) override;

    bool drawGradient(const tools::PolyPolygon& rPolygon, const Gradient& rGradient) override;

    bool implDrawGradient(basegfx::B2DPolyPolygon const& rPolyPolygon,
                          SalGradient const& rGradient) override;

    virtual bool hasFastDrawTransformedBitmap() const override;

    virtual bool supportsOperation(OutDevSupportType eType) const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
