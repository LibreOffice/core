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

#include <skia/utils.hxx>

#include <SkPaint.h>

class SkiaFlushIdle;
class GenericSalLayout;
class SkFont;
class SkiaSalBitmap;

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
    virtual tools::Long GetGraphicsWidth() const override;

    // set the clip region to empty
    virtual void ResetClipRegion() override;

    // set the line color to transparent (= don't draw lines)

    virtual void SetLineColor() override;

    // set the line color to a specific color
    virtual void SetLineColor(Color nColor) override;

    // set the fill rule to default (= FillRule_NON_ZERO)
    virtual void SetFillRule() override;

    // set the fill rule to specific rule (FillRule_NON_ZERO OR FillRule_EVEN_ODD)
    virtual void SetFillRule(PolyFillMode eFillRule) override;

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
    virtual void drawPixel(tools::Long nX, tools::Long nY) override;
    virtual void drawPixel(tools::Long nX, tools::Long nY, Color nColor) override;

    virtual void drawLine(tools::Long nX1, tools::Long nY1, tools::Long nX2,
                          tools::Long nY2) override;

    virtual void drawRect(tools::Long nX, tools::Long nY, tools::Long nWidth,
                          tools::Long nHeight) override;

    virtual void drawPolyLine(sal_uInt32 nPoints, const Point* pPtAry) override;

    virtual void drawPolygon(sal_uInt32 nPoints, const Point* pPtAry) override;

    virtual void drawPolyPolygon(sal_uInt32 nPoly, const sal_uInt32* pPoints,
                                 const Point** pPtAry) override;

    virtual bool drawPolyPolygon(const basegfx::B2DHomMatrix& rObjectToDevice,
                                 const basegfx::B2DPolyPolygon&, double fTransparency) override;

    virtual bool drawPolyLine(const basegfx::B2DHomMatrix& rObjectToDevice,
                              const basegfx::B2DPolygon&, double fTransparency, double fLineWidth,
                              const std::vector<double>* pStroke, basegfx::B2DLineJoin,
                              css::drawing::LineCap, double fMiterMinimumAngle,
                              bool bPixelSnapHairline) override;

    virtual bool drawPolyLineBezier(sal_uInt32 nPoints, const Point* pPtAry,
                                    const PolyFlags* pFlgAry) override;

    virtual bool drawPolygonBezier(sal_uInt32 nPoints, const Point* pPtAry,
                                   const PolyFlags* pFlgAry) override;

    virtual bool drawPolyPolygonBezier(sal_uInt32 nPoly, const sal_uInt32* pPoints,
                                       const Point* const* pPtAry,
                                       const PolyFlags* const* pFlgAry) override;

    // CopyArea --> No RasterOp, but ClipRegion
    virtual void copyArea(tools::Long nDestX, tools::Long nDestY, tools::Long nSrcX,
                          tools::Long nSrcY, tools::Long nSrcWidth, tools::Long nSrcHeight,
                          bool bWindowInvalidate) override;

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

    virtual std::shared_ptr<SalBitmap> getBitmap(tools::Long nX, tools::Long nY, tools::Long nWidth,
                                                 tools::Long nHeight) override;

    virtual Color getPixel(tools::Long nX, tools::Long nY) override;

    // invert --> ClipRegion (only Windows or VirDevs)
    virtual void invert(tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight,
                        SalInvert nFlags) override;

    virtual void invert(sal_uInt32 nPoints, const Point* pPtAry, SalInvert nFlags) override;

    virtual bool drawEPS(tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight,
                         void* pPtr, sal_uInt32 nSize) override;

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
                                       const SalBitmap* pAlphaBitmap, double fAlpha) override;

    /** Render solid rectangle with given transparency

      @param nX             Top left coordinate of rectangle

      @param nY             Bottom right coordinate of rectangle

      @param nWidth         Width of rectangle

      @param nHeight        Height of rectangle

      @param nTransparency  Transparency value (0-255) to use. 0 blits and opaque, 255 a
                            fully transparent rectangle

      @returns true if successfully drawn, false if not able to draw rectangle
     */
    virtual bool drawAlphaRect(tools::Long nX, tools::Long nY, tools::Long nWidth,
                               tools::Long nHeight, sal_uInt8 nTransparency) override;

    virtual bool drawGradient(const tools::PolyPolygon& rPolygon,
                              const Gradient& rGradient) override;
    virtual bool implDrawGradient(const basegfx::B2DPolyPolygon& rPolyPolygon,
                                  const SalGradient& rGradient) override;

    virtual bool supportsOperation(OutDevSupportType eType) const override;

    // Dump contents to a file for debugging.
    void dump(const char* file) const;

    // Default blend mode for SkPaint is SkBlendMode::kSrcOver
    void drawBitmap(const SalTwoRect& rPosAry, const SkiaSalBitmap& bitmap,
                    SkBlendMode blendMode = SkBlendMode::kSrcOver);

    void drawImage(const SalTwoRect& rPosAry, const sk_sp<SkImage>& aImage, int srcScaling = 1,
                   SkBlendMode eBlendMode = SkBlendMode::kSrcOver);

    void drawShader(const SalTwoRect& rPosAry, const sk_sp<SkShader>& shader,
                    SkBlendMode blendMode = SkBlendMode::kSrcOver);

    void drawGenericLayout(const GenericSalLayout& layout, Color textColor, const SkFont& font,
                           const SkFont& verticalFont);

protected:
    // To be called before any drawing.
    void preDraw();
    // To be called after any drawing.
    void postDraw();
    // The canvas to draw to.
    SkCanvas* getDrawCanvas() { return mSurface->getCanvas(); }
    // Call before makeImageSnapshot(), ensures the content is up to date.
    void flushDrawing();

    virtual void createSurface();
    // Call to ensure that mSurface is valid. If mSurface is going to be modified,
    // use preDraw() instead of this.
    void checkSurface();
    void destroySurface();
    // Reimplemented for X11.
    virtual bool avoidRecreateByResize() const;
    void createWindowSurface(bool forceRaster = false);
    virtual void createWindowSurfaceInternal(bool forceRaster = false) = 0;
    void createOffscreenSurface();
    virtual void flushSurfaceToWindowContext();

    void privateDrawAlphaRect(tools::Long nX, tools::Long nY, tools::Long nWidth,
                              tools::Long nHeight, double nTransparency, bool blockAA = false);
    void privateCopyBits(const SalTwoRect& rPosAry, SkiaSalGraphicsImpl* src);

    void setProvider(SalGeometryProvider* provider) { mProvider = provider; }

    bool isOffscreen() const;
    bool isGPU() const { return mIsGPU; }

    void invert(basegfx::B2DPolygon const& rPoly, SalInvert eFlags);

    // Called by SkiaFlushIdle.
    void performFlush();
    void scheduleFlush();
    friend class SkiaFlushIdle;

    // get the width of the device
    int GetWidth() const { return mProvider ? mProvider->GetWidth() : 1; }
    // get the height of the device
    int GetHeight() const { return mProvider ? mProvider->GetHeight() : 1; }
    // Get the global HiDPI scaling factor.
    virtual int getWindowScaling() const;

    void addUpdateRegion(const SkRect& rect)
    {
        // Make slightly larger, just in case (rounding, antialiasing,...).
        SkIRect addedRect = rect.makeOutset(2, 2).round();
        // Using SkIRect should be enough, SkRegion would be too slow with many operations
        // and swapping to the screen is not _that_slow.
        mDirtyRect.join(addedRect);
    }
    void setCanvasScalingAndClipping();
    void resetCanvasScalingAndClipping();
    static void setCanvasClipRegion(SkCanvas* canvas, const vcl::Region& region);
    sk_sp<SkImage> mergeCacheBitmaps(const SkiaSalBitmap& bitmap, const SkiaSalBitmap* alphaBitmap,
                                     const Size& targetSize);
    using DirectImage = SkiaHelper::DirectImage;
    static OString makeCachedImageKey(const SkiaSalBitmap& bitmap, const SkiaSalBitmap* alphaBitmap,
                                      const Size& targetSize, DirectImage bitmapType,
                                      DirectImage alphaBitmapType);

    // Skia uses floating point coordinates, so when we use integer coordinates, sometimes
    // rounding results in off-by-one errors (down), especially when drawing using GPU,
    // see https://bugs.chromium.org/p/skia/issues/detail?id=9611 . Compensate for
    // it by using centers of pixels. Using 0.5 may sometimes round up, so go with 0.495 .
    static constexpr SkScalar toSkX(tools::Long x) { return x + 0.495; }
    static constexpr SkScalar toSkY(tools::Long y) { return y + 0.495; }
    // Value to add to be exactly in the middle of the pixel.
    static constexpr SkScalar toSkXYFix = SkScalar(0.005);

    // Perform any pending drawing such as delayed merging of polygons. Called by preDraw()
    // and anything that means the next operation cannot be another one in a series (e.g.
    // changing colors).
    void checkPendingDrawing();
    bool delayDrawPolyPolygon(const basegfx::B2DPolyPolygon& polygon, double transparency);
    void performDrawPolyPolygon(const basegfx::B2DPolyPolygon& polygon, double transparency,
                                bool useAA);

    BmpScaleFlag goodScalingQuality() const { return SkiaHelper::goodScalingQuality(isGPU()); }
    SkSamplingOptions makeSamplingOptions(const SalTwoRect& rPosAry, int scalingFactor,
                                          int srcScalingFactor = 1)
    {
        return SkiaHelper::makeSamplingOptions(rPosAry, scalingFactor, srcScalingFactor, isGPU());
    }
    SkSamplingOptions makeSamplingOptions(const SkMatrix& matrix, int scalingFactor)
    {
        return SkiaHelper::makeSamplingOptions(goodScalingQuality(), matrix, scalingFactor);
    }

    // Create SkPaint to use when drawing to the surface. It is not to be used
    // when doing internal drawing such as when merging two bitmaps together.
    // This may apply some default settings to the paint as necessary.
    SkPaint makePaintInternal() const;
    // Create SkPaint set up for drawing lines (using mLineColor etc.).
    SkPaint makeLinePaint(double transparency = 0) const;
    // Create SkPaint set up for filling (using mFillColor etc.).
    SkPaint makeFillPaint(double transparency = 0) const;
    // Create SkPaint set up for bitmap drawing.
    SkPaint makeBitmapPaint() const;
    // Create SkPaint set up for gradient drawing.
    SkPaint makeGradientPaint() const;
    // Create SkPaint set up for text drawing.
    SkPaint makeTextPaint(Color color) const;
    // Create SkPaint for unspecified pixel drawing. Avoid if possible.
    SkPaint makePixelPaint(Color color) const;

    template <typename charT, typename traits>
    friend inline std::basic_ostream<charT, traits>&
    operator<<(std::basic_ostream<charT, traits>& stream, const SkiaSalGraphicsImpl* graphics)
    {
        if (graphics == nullptr)
            return stream << "(null)";
        // O - offscreen, G - GPU-based, R - raster
        stream << static_cast<const void*>(graphics) << " "
               << Size(graphics->GetWidth(), graphics->GetHeight());
        if (graphics->mScaling != 1)
            stream << "*" << graphics->mScaling;
        stream << (graphics->isGPU() ? "G" : "R") << (graphics->isOffscreen() ? "O" : "");
        return stream;
    }

    SalGraphics& mParent;
    /// Pointer to the SalFrame or SalVirtualDevice
    SalGeometryProvider* mProvider;
    // The Skia surface that is target of all the rendering.
    sk_sp<SkSurface> mSurface;
    // Note that mSurface may be a proxy surface and not the one from the window context.
    std::unique_ptr<sk_app::WindowContext> mWindowContext;
    bool mIsGPU; // whether the surface is GPU-backed
    // Note that we generally use VCL coordinates, which is not mSurface coordinates if mScaling!=1.
    SkIRect mDirtyRect; // The area that has been changed since the last performFlush().
    vcl::Region mClipRegion;
    Color mLineColor;
    Color mFillColor;
    enum class XorMode
    {
        None,
        Invert,
        Xor
    };
    XorMode mXorMode;
    PolyFillMode meFillRule;
    SkBitmap mXorBitmap;
    std::unique_ptr<SkCanvas> mXorCanvas;
    SkRegion mXorRegion; // the area that needs updating for the xor operation
    std::unique_ptr<SkiaFlushIdle> mFlush;
    // Info about pending polygons to draw (we try to merge adjacent polygons into one).
    struct LastPolyPolygonInfo
    {
        basegfx::B2DPolyPolygonVector polygons;
        basegfx::B2DRange bounds;
        double transparency;
    };
    LastPolyPolygonInfo mLastPolyPolygonInfo;
    int mPendingOperationsToFlush;
    int mScaling; // The scale factor for HiDPI screens.
};

inline SkPaint SkiaSalGraphicsImpl::makePaintInternal() const
{
    SkPaint paint;
    // Invert could be done using a blend mode like invert() does, but
    // intentionally use SkBlender to make sure it's not overwritten
    // by a blend mode set later (which would be probably a mistake),
    // and so that the drawing color does not actually matter.
    if (mXorMode == XorMode::Invert)
        SkiaHelper::setBlenderInvert(&paint);
    else if (mXorMode == XorMode::Xor)
        SkiaHelper::setBlenderXor(&paint);
    return paint;
}

inline SkPaint SkiaSalGraphicsImpl::makeLinePaint(double transparency) const
{
    assert(mLineColor != SALCOLOR_NONE);
    SkPaint paint = makePaintInternal();
    paint.setColor(transparency == 0
                       ? SkiaHelper::toSkColor(mLineColor)
                       : SkiaHelper::toSkColorWithTransparency(mLineColor, transparency));
    paint.setStyle(SkPaint::kStroke_Style);
    return paint;
}

inline SkPaint SkiaSalGraphicsImpl::makeFillPaint(double transparency) const
{
    assert(mFillColor != SALCOLOR_NONE);
    SkPaint paint = makePaintInternal();
    paint.setColor(transparency == 0
                       ? SkiaHelper::toSkColor(mFillColor)
                       : SkiaHelper::toSkColorWithTransparency(mFillColor, transparency));
    if (mLineColor == mFillColor)
        paint.setStyle(SkPaint::kStrokeAndFill_Style);
    else
        paint.setStyle(SkPaint::kFill_Style);
    return paint;
}

inline SkPaint SkiaSalGraphicsImpl::makeBitmapPaint() const { return makePaintInternal(); }

inline SkPaint SkiaSalGraphicsImpl::makeGradientPaint() const { return makePaintInternal(); }

inline SkPaint SkiaSalGraphicsImpl::makeTextPaint(Color color) const
{
    assert(color != SALCOLOR_NONE);
    SkPaint paint = makePaintInternal();
    paint.setColor(SkiaHelper::toSkColor(color));
    return paint;
}

inline SkPaint SkiaSalGraphicsImpl::makePixelPaint(Color color) const
{
    assert(color != SALCOLOR_NONE);
    SkPaint paint = makePaintInternal();
    paint.setColor(SkiaHelper::toSkColor(color));
    return paint;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
