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

#include "Qt5Data.hxx"
#include "Qt5Graphics_Controls.hxx"

class PhysicalFontCollection;
class QImage;
class QPushButton;
class Qt5Font;
class Qt5FontFace;
class Qt5Frame;
class Qt5Painter;

class Qt5Graphics : public SalGraphics
{
    friend class Qt5Bitmap;
    friend class Qt5Painter;

    Qt5Graphics_Controls m_aControl;

    Qt5Frame* m_pFrame;
    QImage* m_pQImage;
    QRegion m_aClipRegion;
    QPainterPath m_aClipPath;
    Color m_aLineColor;
    Color m_aFillColor;
    QPainter::CompositionMode m_eCompositionMode;

    PhysicalFontCollection* m_pFontCollection;
    rtl::Reference<Qt5Font> m_pTextStyle[MAX_FALLBACK];
    Color m_aTextColor;
    std::unique_ptr<QPushButton> m_focusedButton;
    std::unique_ptr<QImage> m_image;
    QRect m_lastPopupRect;

    Qt5Graphics(Qt5Frame* pFrame, QImage* pQImage);

    void drawScaledImage(const SalTwoRect& rPosAry, const QImage& rImage);

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

    void ChangeQImage(QImage* pImage);

    virtual SalGraphicsImpl* GetImpl() const override;
    virtual SystemGraphicsData GetGraphicsData() const override;
    virtual bool supportsOperation(OutDevSupportType) const override;

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
    virtual SystemFontData GetSysFontData(int nFallbacklevel) const override;
#endif // ENABLE_CAIRO_CANVAS

    // GDI

    virtual bool setClipRegion(const vcl::Region&) override;
    virtual void ResetClipRegion() override;

    virtual void drawPixel(long nX, long nY) override;
    virtual void drawPixel(long nX, long nY, Color nColor) override;
    virtual void drawLine(long nX1, long nY1, long nX2, long nY2) override;
    virtual void drawRect(long nX, long nY, long nWidth, long nHeight) override;
    virtual void drawPolyLine(sal_uInt32 nPoints, const SalPoint* pPtAry) override;
    virtual void drawPolygon(sal_uInt32 nPoints, const SalPoint* pPtAry) override;
    virtual void drawPolyPolygon(sal_uInt32 nPoly, const sal_uInt32* pPoints,
                                 PCONSTSALPOINT* pPtAry) override;
    virtual bool drawPolyPolygon(const basegfx::B2DHomMatrix& rObjectToDevice,
                                 const basegfx::B2DPolyPolygon&, double fTransparency) override;
    virtual bool drawPolyLineBezier(sal_uInt32 nPoints, const SalPoint* pPtAry,
                                    const PolyFlags* pFlgAry) override;
    virtual bool drawPolygonBezier(sal_uInt32 nPoints, const SalPoint* pPtAry,
                                   const PolyFlags* pFlgAry) override;
    virtual bool drawPolyPolygonBezier(sal_uInt32 nPoly, const sal_uInt32* pPoints,
                                       const SalPoint* const* pPtAry,
                                       const PolyFlags* const* pFlgAry) override;
    virtual bool drawPolyLine(const basegfx::B2DHomMatrix& rObjectToDevice,
                              const basegfx::B2DPolygon&, double fTransparency,
                              const basegfx::B2DVector& rLineWidths, basegfx::B2DLineJoin,
                              css::drawing::LineCap eLineCap, double fMiterMinimumAngle,
                              bool bPixelSnapHairline) override;
    virtual bool drawGradient(const tools::PolyPolygon&, const Gradient&) override;

    virtual void copyArea(long nDestX, long nDestY, long nSrcX, long nSrcY, long nSrcWidth,
                          long nSrcHeight, bool bWindowInvalidate) override;

    virtual void copyBits(const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics) override;
    virtual void drawBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap) override;
    virtual void drawBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap,
                            const SalBitmap& rTransparentBitmap) override;
    virtual void drawMask(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap,
                          Color nMaskColor) override;

    virtual std::shared_ptr<SalBitmap> getBitmap(long nX, long nY, long nWidth,
                                                 long nHeight) override;
    virtual Color getPixel(long nX, long nY) override;

    virtual void invert(long nX, long nY, long nWidth, long nHeight, SalInvert nFlags) override;
    virtual void invert(sal_uInt32 nPoints, const SalPoint* pPtAry, SalInvert nFlags) override;

    virtual bool drawEPS(long nX, long nY, long nWidth, long nHeight, void* pPtr,
                         sal_uInt32 nSize) override;

    virtual bool blendBitmap(const SalTwoRect&, const SalBitmap& rBitmap) override;

    virtual bool blendAlphaBitmap(const SalTwoRect&, const SalBitmap& rSrcBitmap,
                                  const SalBitmap& rMaskBitmap,
                                  const SalBitmap& rAlphaBitmap) override;

    virtual bool drawAlphaBitmap(const SalTwoRect&, const SalBitmap& rSourceBitmap,
                                 const SalBitmap& rAlphaBitmap) override;

    bool drawTransformedBitmap(const basegfx::B2DPoint& rNull, const basegfx::B2DPoint& rX,
                               const basegfx::B2DPoint& rY, const SalBitmap& rSourceBitmap,
                               const SalBitmap* pAlphaBitmap) override;

    virtual bool drawAlphaRect(long nX, long nY, long nWidth, long nHeight,
                               sal_uInt8 nTransparency) override;

    virtual void GetResolution(sal_Int32& rDPIX, sal_Int32& rDPIY) override;
    virtual sal_uInt16 GetBitCount() const override;
    virtual long GetGraphicsWidth() const override;

    virtual void SetLineColor() override;
    virtual void SetLineColor(Color nColor) override;
    virtual void SetFillColor() override;
    virtual void SetFillColor(Color nColor) override;
    virtual void SetXORMode(bool bSet, bool bInvertOnly) override;
    virtual void SetROPLineColor(SalROPColor nROPColor) override;
    virtual void SetROPFillColor(SalROPColor nROPColor) override;

    // Text rendering + font support

    virtual void SetTextColor(Color nColor) override;
    virtual void SetFont(LogicalFontInstance*, int nFallbackLevel) override;
    virtual void GetFontMetric(ImplFontMetricDataRef&, int nFallbackLevel) override;
    virtual const FontCharMapRef GetFontCharMap() const override;
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

    virtual const void* GetEmbedFontData(const PhysicalFontFace*, long* pDataLen) override;
    virtual void FreeEmbedFontData(const void* pData, long nDataLen) override;

    virtual void GetGlyphWidths(const PhysicalFontFace*, bool bVertical,
                                std::vector<sal_Int32>& rWidths, Ucs2UIntMap& rUnicodeEnc) override;

    virtual std::unique_ptr<GenericSalLayout> GetTextLayout(int nFallbackLevel) override;
    virtual void DrawTextLayout(const GenericSalLayout&) override;

    // Native control support

    virtual bool IsNativeControlSupported(ControlType nType, ControlPart nPart) override;
    virtual bool hitTestNativeControl(ControlType nType, ControlPart nPart,
                                      const tools::Rectangle& rControlRegion, const Point& aPos,
                                      bool& rIsInside) override;
    virtual bool drawNativeControl(ControlType nType, ControlPart nPart,
                                   const tools::Rectangle& rControlRegion, ControlState nState,
                                   const ImplControlValue& aValue,
                                   const OUString& aCaption) override;
    virtual bool getNativeControlRegion(ControlType nType, ControlPart nPart,
                                        const tools::Rectangle& rControlRegion, ControlState nState,
                                        const ImplControlValue& aValue, const OUString& aCaption,
                                        tools::Rectangle& rNativeBoundingRegion,
                                        tools::Rectangle& rNativeContentRegion) override;
};

inline bool Qt5Graphics::IsNativeControlSupported(ControlType nType, ControlPart nPart)
{
    if (Qt5Data::noNativeControls())
        return false;
    return Qt5Graphics_Controls::IsNativeControlSupported(nType, nPart);
}

inline bool Qt5Graphics::hitTestNativeControl(ControlType nType, ControlPart nPart,
                                              const tools::Rectangle& rControlRegion,
                                              const Point& aPos, bool& rIsInside)
{
    return Qt5Graphics_Controls::hitTestNativeControl(nType, nPart, rControlRegion, aPos,
                                                      rIsInside);
}

inline bool Qt5Graphics::getNativeControlRegion(ControlType nType, ControlPart nPart,
                                                const tools::Rectangle& rControlRegion,
                                                ControlState nState, const ImplControlValue& aValue,
                                                const OUString& aCaption,
                                                tools::Rectangle& rNativeBoundingRegion,
                                                tools::Rectangle& rNativeContentRegion)
{
    return Qt5Graphics_Controls::getNativeControlRegion(nType, nPart, rControlRegion, nState,
                                                        aValue, aCaption, rNativeBoundingRegion,
                                                        rNativeContentRegion);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
