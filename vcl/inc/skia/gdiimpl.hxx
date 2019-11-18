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

#ifndef INCLUDED_VCL_SKIA_GDIIMPL_HXX
#define INCLUDED_VCL_SKIA_GDIIMPL_HXX

#include <vcl/dllapi.h>

#include <salgdiimpl.hxx>
#include <salgeom.hxx>

#include <SkSurface.h>

class SkiaFlushIdle;

class VCL_DLLPUBLIC SkiaSalGraphicsImpl : public SalGraphicsImpl
{
public:
    SkiaSalGraphicsImpl(SalGraphics& pParent, SalGeometryProvider* pProvider);
    virtual ~SkiaSalGraphicsImpl() override;

    virtual void Init() override;

    virtual void DeInit() override;

    virtual OUString getRenderBackendName() const override { return "skia"; }

    const vcl::Region& getClipRegion() const;
    virtual bool setClipRegion(const vcl::Region&) override;

    //
    // get the depth of the device
    virtual sal_uInt16 GetBitCount() const override;

    // get the width of the device
    virtual long GetGraphicsWidth() const override;

    // set the clip region to empty
    virtual void ResetClipRegion() override;

    // set the line color to transparent (= don't draw lines)

    virtual void SetLineColor() override;

    // set the line color to a specific color
    virtual void SetLineColor(Color nColor) override;

    // set the fill color to transparent (= don't fill)
    virtual void SetFillColor() override;

    // set the fill color to a specific color, shapes will be
    // filled accordingly
    virtual void SetFillColor(Color nColor) override;

    // enable/disable XOR drawing
    virtual void SetXORMode(bool bSet, bool bInvertOnly) override;

    // set line color for raster operations
    virtual void SetROPLineColor(SalROPColor nROPColor) override;

    // set fill color for raster operations
    virtual void SetROPFillColor(SalROPColor nROPColor) override;

    // draw --> LineColor and FillColor and RasterOp and ClipRegion
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

    virtual bool drawPolyLine(const basegfx::B2DHomMatrix& rObjectToDevice,
                              const basegfx::B2DPolygon&, double fTransparency,
                              const basegfx::B2DVector& rLineWidths, basegfx::B2DLineJoin,
                              css::drawing::LineCap, double fMiterMinimumAngle,
                              bool bPixelSnapHairline) override;

    virtual bool drawPolyLineBezier(sal_uInt32 nPoints, const SalPoint* pPtAry,
                                    const PolyFlags* pFlgAry) override;

    virtual bool drawPolygonBezier(sal_uInt32 nPoints, const SalPoint* pPtAry,
                                   const PolyFlags* pFlgAry) override;

    virtual bool drawPolyPolygonBezier(sal_uInt32 nPoly, const sal_uInt32* pPoints,
                                       const SalPoint* const* pPtAry,
                                       const PolyFlags* const* pFlgAry) override;

    // CopyArea --> No RasterOp, but ClipRegion
    virtual void copyArea(long nDestX, long nDestY, long nSrcX, long nSrcY, long nSrcWidth,
                          long nSrcHeight, bool bWindowInvalidate) override;

    virtual void copyBits(const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics) override;

    virtual bool blendBitmap(const SalTwoRect&, const SalBitmap& rBitmap) override;

    virtual bool blendAlphaBitmap(const SalTwoRect&, const SalBitmap& rSrcBitmap,
                                  const SalBitmap& rMaskBitmap,
                                  const SalBitmap& rAlphaBitmap) override;

    virtual void drawBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap) override;

    virtual void drawBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap,
                            const SalBitmap& rMaskBitmap) override;

    virtual void drawMask(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap,
                          Color nMaskColor) override;

    virtual std::shared_ptr<SalBitmap> getBitmap(long nX, long nY, long nWidth,
                                                 long nHeight) override;

    virtual Color getPixel(long nX, long nY) override;

    // invert --> ClipRegion (only Windows or VirDevs)
    virtual void invert(long nX, long nY, long nWidth, long nHeight, SalInvert nFlags) override;

    virtual void invert(sal_uInt32 nPoints, const SalPoint* pPtAry, SalInvert nFlags) override;

    virtual bool drawEPS(long nX, long nY, long nWidth, long nHeight, void* pPtr,
                         sal_uInt32 nSize) override;

    /** Render bitmap with alpha channel

        @param rSourceBitmap
        Source bitmap to blit

        @param rAlphaBitmap
        Alpha channel to use for blitting

        @return true, if the operation succeeded, and false
        otherwise. In this case, clients should try to emulate alpha
        compositing themselves
     */
    virtual bool drawAlphaBitmap(const SalTwoRect&, const SalBitmap& rSourceBitmap,
                                 const SalBitmap& rAlphaBitmap) override;

    /** draw transformed bitmap (maybe with alpha) where Null, X, Y define the coordinate system */
    virtual bool drawTransformedBitmap(const basegfx::B2DPoint& rNull, const basegfx::B2DPoint& rX,
                                       const basegfx::B2DPoint& rY, const SalBitmap& rSourceBitmap,
                                       const SalBitmap* pAlphaBitmap) override;

    /** Render solid rectangle with given transparency

      @param nX             Top left coordinate of rectangle

      @param nY             Bottom right coordinate of rectangle

      @param nWidth         Width of rectangle

      @param nHeight        Height of rectangle

      @param nTransparency  Transparency value (0-255) to use. 0 blits and opaque, 255 a
                            fully transparent rectangle

      @returns true if successfully drawn, false if not able to draw rectangle
     */
    virtual bool drawAlphaRect(long nX, long nY, long nWidth, long nHeight,
                               sal_uInt8 nTransparency) override;

    virtual bool drawGradient(const tools::PolyPolygon& rPolygon,
                              const Gradient& rGradient) override;

    void drawBitmap(const SalTwoRect& rPosAry, const SkBitmap& bitmap);

    virtual bool supportsOperation(OutDevSupportType eType) const override;

#ifdef DBG_UTIL
    void dump(const char* file) const;
    static void dump(const SkBitmap& bitmap, const char* file);
#endif

protected:
    // To be called before any drawing.
    void preDraw();
    // To be called after any drawing.
    void postDraw();

    virtual void createSurface();
    // Call to ensure that mSurface is valid. If mSurface is going to be modified,
    // use preDraw() instead of this.
    void checkSurface();
    void resetSurface();

    void setProvider(SalGeometryProvider* provider) { mProvider = provider; }

    bool isOffscreen() const { return mProvider == nullptr || mProvider->IsOffScreen(); }

    void invert(basegfx::B2DPolygon const& rPoly, SalInvert eFlags);

    // Called by SkiaFlushIdle.
    virtual void performFlush() = 0;
    friend class SkiaFlushIdle;

    // get the width of the device
    int GetWidth() const { return mProvider ? mProvider->GetWidth() : 1; }
    // get the height of the device
    int GetHeight() const { return mProvider ? mProvider->GetHeight() : 1; }

    void drawMask(const SalTwoRect& rPosAry, const SkBitmap& rBitmap, Color nMaskColor);

    // Which Skia backend to use.
    enum RenderMethod
    {
        RenderRaster,
        RenderVulkan
    };
    static RenderMethod renderMethodToUse();

    SalGraphics& mParent;
    /// Pointer to the SalFrame or SalVirtualDevice
    SalGeometryProvider* mProvider;
    // The Skia surface that is target of all the rendering.
    sk_sp<SkSurface> mSurface;
    vcl::Region mClipRegion;
    Color mLineColor;
    Color mFillColor;
    std::unique_ptr<SkiaFlushIdle> mFlush;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
