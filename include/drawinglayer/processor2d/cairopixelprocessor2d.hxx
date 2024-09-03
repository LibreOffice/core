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
class InvertPrimitive2D;
class MaskPrimitive2D;
class ModifiedColorPrimitive2D;
class TransformPrimitive2D;
class PointArrayPrimitive2D;
class MarkerArrayPrimitive2D;
class PolygonStrokePrimitive2D;
class LineRectanglePrimitive2D;
class FilledRectanglePrimitive2D;
class SingleLinePrimitive2D;
class FillGradientPrimitive2D;
class PolyPolygonRGBAGradientPrimitive2D;
class FillGraphicPrimitive2D;
class PolyPolygonRGBAPrimitive2D;
class PolyPolygonAlphaGradientPrimitive2D;
class BitmapAlphaPrimitive2D;
class TextSimplePortionPrimitive2D;
class TextDecoratedPortionPrimitive2D;
class TextLayouterDevice;
class SvgLinearGradientPrimitive2D;
class SvgRadialGradientPrimitive2D;
class SvgGradientHelper;
}

namespace basegfx
{
class B2DPolyPolygon;
}

namespace basegfx::utils
{
class B2DHomMatrixBufferedOnDemandDecompose;
}

class BitmapEx;
class SalLayout;

namespace drawinglayer::processor2d
{
class UNLESS_MERGELIBS(DRAWINGLAYER_DLLPUBLIC) CairoPixelProcessor2D final : public BaseProcessor2D
{
    // the modifiedColorPrimitive stack
    basegfx::BColorModifierStack maBColorModifierStack;

    // cairo specific data
    cairo_t* mpRT;

    // get text render config settings
    bool mbRenderSimpleTextDirect;
    bool mbRenderDecoratedTextDirect;

    // recursion counter for CairoPixelProcessor2D::processMaskPrimitive2D,
    // see comment there
    sal_uInt16 mnClipRecurstionCount;

    // helpers for direct paints
    void paintPolyPoylgonRGBA(const basegfx::B2DPolyPolygon& rPolyPolygon,
                              const basegfx::BColor& rColor, double fTransparency = 0.0);
    void processPolygonHairlinePrimitive2D(
        const primitive2d::PolygonHairlinePrimitive2D& rPolygonHairlinePrimitive2D);
    void processPolyPolygonColorPrimitive2D(
        const primitive2d::PolyPolygonColorPrimitive2D& rPolyPolygonColorPrimitive2D);
    void processBitmapPrimitive2D(const primitive2d::BitmapPrimitive2D& rBitmapCandidate);
    void
    processTransparencePrimitive2D(const primitive2d::TransparencePrimitive2D& rTransCandidate);
    void processInvertPrimitive2D(const primitive2d::InvertPrimitive2D& rTransCandidate);
    void processUnifiedTransparencePrimitive2D(
        const primitive2d::UnifiedTransparencePrimitive2D& rTransCandidate);
    void processMaskPrimitive2D(const primitive2d::MaskPrimitive2D& rMaskCandidate);
    void processModifiedColorPrimitive2D(
        const primitive2d::ModifiedColorPrimitive2D& rModifiedCandidate);
    void processTransformPrimitive2D(const primitive2d::TransformPrimitive2D& rTransformCandidate);
    void
    processPointArrayPrimitive2D(const primitive2d::PointArrayPrimitive2D& rPointArrayCandidate);
    void
    processMarkerArrayPrimitive2D(const primitive2d::MarkerArrayPrimitive2D& rMarkerArrayCandidate);
    void processBackgroundColorPrimitive2D(
        const primitive2d::BackgroundColorPrimitive2D& rBackgroundColorCandidate);
    void processPolygonStrokePrimitive2D(
        const primitive2d::PolygonStrokePrimitive2D& rPolygonStrokeCandidate);
    void processLineRectanglePrimitive2D(
        const primitive2d::LineRectanglePrimitive2D& rLineRectanglePrimitive2D);
    void processFilledRectanglePrimitive2D(
        const primitive2d::FilledRectanglePrimitive2D& rFilledRectanglePrimitive2D);
    void
    processSingleLinePrimitive2D(const primitive2d::SingleLinePrimitive2D& rSingleLinePrimitive2D);
    void processFillGradientPrimitive2D(
        const primitive2d::FillGradientPrimitive2D& rFillGradientPrimitive2D);
    void processFillGraphicPrimitive2D(
        const primitive2d::FillGraphicPrimitive2D& rFillGraphicPrimitive2D);
    void processPolyPolygonRGBAPrimitive2D(
        const primitive2d::PolyPolygonRGBAPrimitive2D& rPolyPolygonRGBAPrimitive2D);
    void processPolyPolygonAlphaGradientPrimitive2D(
        const primitive2d::PolyPolygonAlphaGradientPrimitive2D&
            rPolyPolygonAlphaGradientPrimitive2D);
    void paintBitmapAlpha(const BitmapEx& rBitmapEx, const basegfx::B2DHomMatrix& rTransform,
                          double fTransparency = 0.0);
    void processBitmapAlphaPrimitive2D(
        const primitive2d::BitmapAlphaPrimitive2D& rBitmapAlphaPrimitive2D);

    void processTextSimplePortionPrimitive2D(
        const primitive2d::TextSimplePortionPrimitive2D& rCandidate);
    void processTextDecoratedPortionPrimitive2D(
        const primitive2d::TextDecoratedPortionPrimitive2D& rCandidate);

    // helpers for text rendering
    void renderTextSimpleOrDecoratedPortionPrimitive2D(
        const primitive2d::TextSimplePortionPrimitive2D& rTextCandidate,
        const primitive2d::TextDecoratedPortionPrimitive2D* pDecoratedCandidate);
    void renderTextBackground(const primitive2d::TextSimplePortionPrimitive2D& rTextCandidate,
                              double fAscent, double fDescent,
                              const basegfx::B2DHomMatrix& rTransform, double fTextWidth);
    void renderSalLayout(const std::unique_ptr<SalLayout>& rSalLayout,
                         const basegfx::BColor& rTextColor, const basegfx::B2DHomMatrix& rTransform,
                         bool bAntiAliase) const;
    void renderTextDecorationWithOptionalTransformAndColor(
        const primitive2d::TextDecoratedPortionPrimitive2D& rDecoratedCandidate,
        const basegfx::utils::B2DHomMatrixBufferedOnDemandDecompose& rDecTrans,
        const basegfx::B2DHomMatrix* pOptionalObjectTransform = nullptr,
        const basegfx::BColor* pReplacementColor = nullptr);

    // support for SVG gradients
    void processSvgLinearGradientPrimitive2D(
        const primitive2d::SvgLinearGradientPrimitive2D& rCandidate);
    void processSvgRadialGradientPrimitive2D(
        const primitive2d::SvgRadialGradientPrimitive2D& rCandidate);
    bool handleSvgGradientHelper(const primitive2d::SvgGradientHelper& rCandidate);

    /*  the local processor for BasePrimitive2D-Implementation based primitives,
        called from the common process()-implementation
     */
    virtual void processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate) override;

    // helpers for gradients
    void processFillGradientPrimitive2D_fallback_decompose(
        const primitive2d::FillGradientPrimitive2D& rFillGradientPrimitive2D);
    void processFillGradientPrimitive2D_drawOutputRange(
        const primitive2d::FillGradientPrimitive2D& rFillGradientPrimitive2D);
    bool processFillGradientPrimitive2D_isCompletelyBordered(
        const primitive2d::FillGradientPrimitive2D& rFillGradientPrimitive2D);
    void processFillGradientPrimitive2D_linear_axial(
        const primitive2d::FillGradientPrimitive2D& rFillGradientPrimitive2D);
    void processFillGradientPrimitive2D_radial_elliptical(
        const primitive2d::FillGradientPrimitive2D& rFillGradientPrimitive2D);
    void processFillGradientPrimitive2D_square_rect(
        const primitive2d::FillGradientPrimitive2D& rFillGradientPrimitive2D);

protected:
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
