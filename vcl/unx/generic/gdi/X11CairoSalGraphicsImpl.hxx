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
#include <unx/x11/x11gdiimpl.h>
#include "cairo_xlib_cairo.hxx"

#include <headless/CairoCommon.hxx>

class X11CairoSalGraphicsImpl : public SalGraphicsImpl, public X11GraphicsImpl
{
private:
    X11SalGraphics& mrParent;
    CairoCommon& mrCairoCommon;

public:
    X11CairoSalGraphicsImpl(X11SalGraphics& rParent, CairoCommon& rCairoCommon);

    void Init() override;

    OUString getRenderBackendName() const override { return u"gen"_ustr; }

    // get the depth of the device
    sal_uInt16 GetBitCount() const override { return mrParent.GetVisual().GetDepth(); }

    // get the width of the device
    tools::Long GetGraphicsWidth() const override;

    void ResetClipRegion() override { mrCairoCommon.m_aClipRegion.SetNull(); }

    void setClipRegion(const vcl::Region& i_rClip) override
    {
        mrCairoCommon.m_aClipRegion = i_rClip;
    }

    void SetLineColor() override { mrCairoCommon.m_oLineColor = std::nullopt; }

    void SetLineColor(Color nColor) override { mrCairoCommon.m_oLineColor = nColor; }

    void SetFillColor() override { mrCairoCommon.m_oFillColor = std::nullopt; }

    void SetFillColor(Color nColor) override { mrCairoCommon.m_oFillColor = nColor; }

    void SetXORMode(bool bSet, bool bInvertOnly) override
    {
        mrCairoCommon.SetXORMode(bSet, bInvertOnly);
    }

    void SetROPLineColor(SalROPColor nROPColor) override
    {
        mrCairoCommon.SetROPLineColor(nROPColor);
    }

    void SetROPFillColor(SalROPColor nROPColor) override
    {
        mrCairoCommon.SetROPFillColor(nROPColor);
    }

    void clipRegion(cairo_t* cr) { CairoCommon::clipRegion(cr, mrCairoCommon.m_aClipRegion); }

    void drawPixel(tools::Long nX, tools::Long nY) override;
    void drawPixel(tools::Long nX, tools::Long nY, Color nColor) override;
    Color getPixel(tools::Long nX, tools::Long nY) override;

    void drawLine(tools::Long nX1, tools::Long nY1, tools::Long nX2, tools::Long nY2) override;

    void drawRect(tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight) override;

    void drawPolygon(sal_uInt32 nPoints, const Point* pPtAry) override;

    void drawPolyPolygon(sal_uInt32 nPoly, const sal_uInt32* pPoints,
                         const Point** pPtAry) override;

    void drawPolyPolygon(const basegfx::B2DHomMatrix& rObjectToDevice,
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

    void invert(tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight,
                SalInvert nFlags) override;

    void invert(sal_uInt32 nPoints, const Point* pPtAry, SalInvert nFlags) override;

    // CopyArea --> No RasterOp, but ClipRegion
    void copyArea(tools::Long nDestX, tools::Long nDestY, tools::Long nSrcX, tools::Long nSrcY,
                  tools::Long nSrcWidth, tools::Long nSrcHeight, bool bWindowInvalidate) override;

    // CopyBits and DrawBitmap --> RasterOp and ClipRegion
    // CopyBits() --> pSrcGraphics == NULL, then CopyBits on same Graphics
    void copyBits(const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics) override;

    void drawBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap) override;

    void drawBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap,
                    const SalBitmap& rMaskBitmap) override;

    /** Render bitmap with alpha channel

        @param rSourceBitmap
        Source bitmap to blit

        @param rAlphaBitmap
        Alpha channel to use for blitting

        @return true, if the operation succeeded, and false
        otherwise. In this case, clients should try to emulate alpha
        compositing themselves
     */
    bool drawAlphaBitmap(const SalTwoRect&, const SalBitmap& rSourceBitmap,
                         const SalBitmap& rAlphaBitmap) override;

    void drawMask(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap,
                  Color nMaskColor) override;

    std::shared_ptr<SalBitmap> getBitmap(tools::Long nX, tools::Long nY, tools::Long nWidth,
                                         tools::Long nHeight) override;

    bool drawPolyLineBezier(sal_uInt32 nPoints, const Point* pPtAry,
                            const PolyFlags* pFlgAry) override;

    bool drawPolygonBezier(sal_uInt32 nPoints, const Point* pPtAry,
                           const PolyFlags* pFlgAry) override;

    bool drawPolyPolygonBezier(sal_uInt32 nPoly, const sal_uInt32* pPoints,
                               const Point* const* pPtAry,
                               const PolyFlags* const* pFlgAry) override;

    bool drawEPS(tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight,
                 void* pPtr, sal_uInt32 nSize) override;

    bool hasFastDrawTransformedBitmap() const override;

    /** draw transformed bitmap (maybe with alpha) where Null, X, Y define the coordinate system */
    bool drawTransformedBitmap(const basegfx::B2DPoint& rNull, const basegfx::B2DPoint& rX,
                               const basegfx::B2DPoint& rY, const SalBitmap& rSourceBitmap,
                               const SalBitmap* pAlphaBitmap, double fAlpha) override;

    /** Blend bitmap with color channels */
    bool blendBitmap(const SalTwoRect&, const SalBitmap& rBitmap) override;

    /** Render bitmap by blending using the mask and alpha channel */
    bool blendAlphaBitmap(const SalTwoRect&, const SalBitmap& rSrcBitmap,
                          const SalBitmap& rMaskBitmap, const SalBitmap& rAlphaBitmap) override;

    bool supportsOperation(OutDevSupportType eType) const override;

    void freeResources() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
