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

#include <salgdi.hxx>

#include <memory>

#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QRegion>

#include "Qt5GraphicsBase.hxx"

class PhysicalFontCollection;
class QImage;
class QPushButton;
class Qt5Font;
class Qt5FontFace;
class Qt5Frame;
class Qt5Painter;

class Qt5GraphicsBackend final : public SalGraphicsImpl, public Qt5GraphicsBase
{
    friend class Qt5Painter;

    Qt5Frame* m_pFrame;
    QImage* m_pQImage;
    QRegion m_aClipRegion;
    QPainterPath m_aClipPath;
    Color m_aLineColor;
    Color m_aFillColor;
    QPainter::CompositionMode m_eCompositionMode;

public:
    Qt5GraphicsBackend(Qt5Frame* pFrame, QImage* pQImage);
    ~Qt5GraphicsBackend() override;

    void Init() override {}

    QImage* getQImage() { return m_pQImage; }

    void setQImage(QImage* pQImage) { m_pQImage = pQImage; }

    void freeResources() override {}

    OUString getRenderBackendName() const override { return "qt5"; }

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

private:
    void drawScaledImage(const SalTwoRect& rPosAry, const QImage& rImage);
};

class Qt5Graphics final : public SalGraphicsAutoDelegateToImpl, public Qt5GraphicsBase
{
    friend class Qt5Bitmap;

    std::unique_ptr<Qt5GraphicsBackend> m_pBackend;

    Qt5Frame* m_pFrame;

    rtl::Reference<Qt5Font> m_pTextStyle[MAX_FALLBACK];
    Color m_aTextColor;

    Qt5Graphics(Qt5Frame* pFrame, QImage* pQImage);

    void drawScaledImage(const SalTwoRect& rPosAry, const QImage& rImage);

    void handleDamage(const tools::Rectangle&) override;

public:
    Qt5Graphics(Qt5Frame* pFrame)
        : Qt5Graphics(pFrame, nullptr)
    {
    }
    Qt5Graphics(QImage* pQImage)
        : Qt5Graphics(nullptr, pQImage)
    {
    }
    virtual ~Qt5Graphics() override;

    QImage* getQImage() { return m_pBackend->getQImage(); }

    void ChangeQImage(QImage* pImage);

    virtual SalGraphicsImpl* GetImpl() const override;
    virtual SystemGraphicsData GetGraphicsData() const override;
    virtual OUString getRenderBackendName() const override
    {
        return m_pBackend->getRenderBackendName();
    }

#if ENABLE_CAIRO_CANVAS
    virtual bool SupportsCairo() const override;
    virtual cairo::SurfaceSharedPtr
    CreateSurface(const cairo::CairoSurfaceSharedPtr& rSurface) const override;
    virtual cairo::SurfaceSharedPtr CreateSurface(const OutputDevice& rRefDevice, int x, int y,
                                                  int width, int height) const override;
    virtual cairo::SurfaceSharedPtr CreateBitmapSurface(const OutputDevice& rRefDevice,
                                                        const BitmapSystemData& rData,
                                                        const Size& rSize) const override;
    virtual css::uno::Any GetNativeSurfaceHandle(cairo::SurfaceSharedPtr& rSurface,
                                                 const basegfx::B2ISize& rSize) const override;
#endif // ENABLE_CAIRO_CANVAS

    // GDI

    virtual void GetResolution(sal_Int32& rDPIX, sal_Int32& rDPIY) override;

    // Text rendering + font support

    virtual void SetTextColor(Color nColor) override;
    virtual void SetFont(LogicalFontInstance*, int nFallbackLevel) override;
    virtual void GetFontMetric(ImplFontMetricDataRef&, int nFallbackLevel) override;
    virtual FontCharMapRef GetFontCharMap() const override;
    virtual bool GetFontCapabilities(vcl::FontCapabilities& rFontCapabilities) const override;
    virtual void GetDevFontList(PhysicalFontCollection*) override;
    virtual void ClearDevFontCache() override;
    virtual bool AddTempDevFont(PhysicalFontCollection*, const OUString& rFileURL,
                                const OUString& rFontName) override;
    virtual bool CreateFontSubset(const OUString& rToFile, const PhysicalFontFace* pFont,
                                  const sal_GlyphId* pGlyphIds, const sal_uInt8* pEncoding,
                                  sal_Int32* pWidths, int nGlyphs,
                                  FontSubsetInfo& rInfo // out parameter
                                  ) override;

    virtual const void* GetEmbedFontData(const PhysicalFontFace*, tools::Long* pDataLen) override;
    virtual void FreeEmbedFontData(const void* pData, tools::Long nDataLen) override;

    virtual void GetGlyphWidths(const PhysicalFontFace*, bool bVertical,
                                std::vector<sal_Int32>& rWidths, Ucs2UIntMap& rUnicodeEnc) override;

    virtual std::unique_ptr<GenericSalLayout> GetTextLayout(int nFallbackLevel) override;
    virtual void DrawTextLayout(const GenericSalLayout&) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
