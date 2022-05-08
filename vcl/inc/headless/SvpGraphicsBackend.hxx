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

#pragma once

#include <vcl/dllapi.h>
#include <tools/long.hxx>
#include <tools/color.hxx>
#include <salgdiimpl.hxx>
#include <salgdi.hxx>

#include <headless/CairoCommon.hxx>

class VCL_DLLPUBLIC SvpGraphicsBackend : public SalGraphicsImpl
{
    CairoCommon& m_rCairoCommon;

public:
    SvpGraphicsBackend(CairoCommon& rCairoCommon);

    void Init() override;

    void freeResources() override;

    OUString getRenderBackendName() const override { return "svp"; }

    bool setClipRegion(vcl::Region const& rRegion) override;
    void ResetClipRegion() override;

    sal_uInt16 GetBitCount() const override;

    tools::Long GetGraphicsWidth() const override;

    void SetLineColor() override;
    void SetLineColor(Color nColor) override;
    void SetFillColor() override;
    void SetFillColor(Color nColor) override;
    void SetXORMode(bool bSet, bool bInvertOnly) override;
    void SetROPLineColor(SalROPColor nROPColor) override;
    void SetROPFillColor(SalROPColor nROPColor) override;

    void drawPixel(tools::Long nX, tools::Long nY) override;
    void drawPixel(tools::Long nX, tools::Long nY, Color nColor) override;

    void drawLine(tools::Long nX1, tools::Long nY1, tools::Long nX2, tools::Long nY2) override;
    void drawRect(tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight) override;
    void drawPolyLine(sal_uInt32 nPoints, const Point* pPointArray) override;
    void drawPolygon(sal_uInt32 nPoints, const Point* pPointArray) override;
    void drawPolyPolygon(sal_uInt32 nPoly, const sal_uInt32* pPoints,
                         const Point** pPointArray) override;

    bool drawPolyPolygon(const basegfx::B2DHomMatrix& rObjectToDevice,
                         const basegfx::B2DPolyPolygon&, double fTransparency) override;

    bool drawPolyLine(const basegfx::B2DHomMatrix& rObjectToDevice, const basegfx::B2DPolygon&,
                      double fTransparency, double fLineWidth, const std::vector<double>* pStroke,
                      basegfx::B2DLineJoin, css::drawing::LineCap, double fMiterMinimumAngle,
                      bool bPixelSnapHairline) override;

    bool drawPolyLineBezier(sal_uInt32 nPoints, const Point* pPointArray,
                            const PolyFlags* pFlagArray) override;

    bool drawPolygonBezier(sal_uInt32 nPoints, const Point* pPointArray,
                           const PolyFlags* pFlagArray) override;

    bool drawPolyPolygonBezier(sal_uInt32 nPoly, const sal_uInt32* pPoints,
                               const Point* const* pPointArray,
                               const PolyFlags* const* pFlagArray) override;

    void copyArea(tools::Long nDestX, tools::Long nDestY, tools::Long nSrcX, tools::Long nSrcY,
                  tools::Long nSrcWidth, tools::Long nSrcHeight, bool bWindowInvalidate) override;

    void copyBits(const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics) override;

    void drawBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap) override;

    void drawBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap,
                    const SalBitmap& rMaskBitmap) override;

    void drawMask(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap,
                  Color nMaskColor) override;

    std::shared_ptr<SalBitmap> getBitmap(tools::Long nX, tools::Long nY, tools::Long nWidth,
                                         tools::Long nHeight) override;

    Color getPixel(tools::Long nX, tools::Long nY) override;

    void invert(tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight,
                SalInvert nFlags) override;

    void invert(sal_uInt32 nPoints, const Point* pPtAry, SalInvert nFlags) override;

    bool drawEPS(tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight,
                 void* pPtr, sal_uInt32 nSize) override;

    bool blendBitmap(const SalTwoRect&, const SalBitmap& rBitmap) override;

    bool blendAlphaBitmap(const SalTwoRect&, const SalBitmap& rSrcBitmap,
                          const SalBitmap& rMaskBitmap, const SalBitmap& rAlphaBitmap) override;

    bool drawAlphaBitmap(const SalTwoRect&, const SalBitmap& rSourceBitmap,
                         const SalBitmap& rAlphaBitmap) override;

    bool drawTransformedBitmap(const basegfx::B2DPoint& rNull, const basegfx::B2DPoint& rX,
                               const basegfx::B2DPoint& rY, const SalBitmap& rSourceBitmap,
                               const SalBitmap* pAlphaBitmap, double fAlpha) override;

    bool hasFastDrawTransformedBitmap() const override;

    bool drawAlphaRect(tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight,
                       sal_uInt8 nTransparency) override;

    bool drawGradient(const tools::PolyPolygon& rPolygon, const Gradient& rGradient) override;
    bool implDrawGradient(basegfx::B2DPolyPolygon const& rPolyPolygon,
                          SalGradient const& rGradient) override;

    bool supportsOperation(OutDevSupportType eType) const override;

    void drawBitmapBuffer(const SalTwoRect& rPosAry, const BitmapBuffer* pBuffer,
                          cairo_operator_t eOp);

private:
    void implDrawRect(double nX, double nY, double nWidth, double nHeight);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
