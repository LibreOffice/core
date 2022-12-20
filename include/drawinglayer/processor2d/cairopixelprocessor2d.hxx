/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <basegfx/color/bcolormodifier.hxx>
#include <sal/config.h>

// cairo-specific
#include <cairo.h>

namespace drawinglayer::primitive2d
{
class PolyPolygonColorPrimitive2D;
class PolygonHairlinePrimitive2D;
class BitmapPrimitive2D;
class UnifiedTransparencePrimitive2D;
class BackgroundColorPrimitive2D;
class TransparencePrimitive2D;
class MaskPrimitive2D;
class ModifiedColorPrimitive2D;
class TransformPrimitive2D;
class PointArrayPrimitive2D;
class MarkerArrayPrimitive2D;
class PolygonStrokePrimitive2D;
}

namespace drawinglayer::processor2d
{
class DRAWINGLAYER_DLLPUBLIC CairoPixelProcessor2D : public BaseProcessor2D
{
    // the modifiedColorPrimitive stack
    basegfx::BColorModifierStack maBColorModifierStack;

    // cairo specific data
    cairo_t* mpRT;

    // helpers for direct paints
    void processPolygonHairlinePrimitive2D(
        const primitive2d::PolygonHairlinePrimitive2D& rPolygonHairlinePrimitive2D);
    void processPolyPolygonColorPrimitive2D(
        const primitive2d::PolyPolygonColorPrimitive2D& rPolyPolygonColorPrimitive2D);
#if 0
    void processBitmapPrimitive2D(const primitive2d::BitmapPrimitive2D& rBitmapCandidate);
#endif
    void
    processTransparencePrimitive2D(const primitive2d::TransparencePrimitive2D& rTransCandidate);
#if 0
    void processUnifiedTransparencePrimitive2D(
        const primitive2d::UnifiedTransparencePrimitive2D& rTransCandidate);
#endif
    void processMaskPrimitive2DPixel(const primitive2d::MaskPrimitive2D& rMaskCandidate);
    void processModifiedColorPrimitive2D(
        const primitive2d::ModifiedColorPrimitive2D& rModifiedCandidate);
    void processTransformPrimitive2D(const primitive2d::TransformPrimitive2D& rTransformCandidate);
    void
    processPointArrayPrimitive2D(const primitive2d::PointArrayPrimitive2D& rPointArrayCandidate);
#if 0
    void
    processMarkerArrayPrimitive2D(const primitive2d::MarkerArrayPrimitive2D& rMarkerArrayCandidate);
    void processBackgroundColorPrimitive2D(
        const primitive2d::BackgroundColorPrimitive2D& rBackgroundColorCandidate);
#endif
    void processPolygonStrokePrimitive2D(
        const primitive2d::PolygonStrokePrimitive2D& rPolygonStrokeCandidate);
#if 0
    // common helpers
    ID2D1Bitmap* implCreateAlpha_Direct(const primitive2d::TransparencePrimitive2D& rTransCandidate,
                                        const basegfx::B2DRange& rVisibleRange);
    ID2D1Bitmap*
    implCreateAlpha_B2DBitmap(const primitive2d::TransparencePrimitive2D& rTransCandidate,
                              const basegfx::B2DRange& rVisibleRange,
                              D2D1_MATRIX_3X2_F& rMaskScale);
#endif

    /*  the local processor for BasePrimitive2D-Implementation based primitives,
        called from the common process()-implementation
     */
    virtual void processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate) override;

protected:
    // local protected minimal constructor for usage in derivates, e.g. helpers
    CairoPixelProcessor2D(const geometry::ViewInformation2D& rViewInformation);

    bool hasError() const { return cairo_status(mpRT) != CAIRO_STATUS_SUCCESS; }
    void setRenderTarget(cairo_t* mpNewRT) { mpRT = mpNewRT; }
    bool hasRenderTarget() const { return nullptr != mpRT; }

public:
    bool valid() const { return hasRenderTarget() && !hasError(); }
    CairoPixelProcessor2D(const geometry::ViewInformation2D& rViewInformation,
                          cairo_surface_t* pTarget);
    virtual ~CairoPixelProcessor2D() override;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
