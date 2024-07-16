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

#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <basegfx/color/bcolormodifier.hxx>
#include <systools/win32/comtools.hxx>
#include <sal/config.h>

// win-specific
#include <d2d1.h>

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
class LineRectanglePrimitive2D;
class FilledRectanglePrimitive2D;
class SingleLinePrimitive2D;
class FillGraphicPrimitive2D;
class InvertPrimitive2D;
class FillGradientPrimitive2D;
}

namespace basegfx
{
class B2DHomMatrix;
class B2DPolyPolygon;
class BColor;
}

struct ID2D1RenderTarget;
struct ID2D1Factory;

namespace drawinglayer::processor2d
{
class DRAWINGLAYER_DLLPUBLIC D2DPixelProcessor2D : public BaseProcessor2D
{
    // the modifiedColorPrimitive stack
    basegfx::BColorModifierStack maBColorModifierStack;

    // win and render specific data
    sal::systools::COMReference<ID2D1RenderTarget> mpRT;
    sal_uInt32 mnRecursionCounter;
    sal_uInt32 mnErrorCounter;

    // helpers for direct paints
    void processPolygonHairlinePrimitive2D(
        const primitive2d::PolygonHairlinePrimitive2D& rPolygonHairlinePrimitive2D);
    void processPolyPolygonColorPrimitive2D(
        const primitive2d::PolyPolygonColorPrimitive2D& rPolyPolygonColorPrimitive2D);
    void processBitmapPrimitive2D(const primitive2d::BitmapPrimitive2D& rBitmapCandidate);
    void
    processTransparencePrimitive2D(const primitive2d::TransparencePrimitive2D& rTransCandidate);
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
    void processFillGraphicPrimitive2D(
        const primitive2d::FillGraphicPrimitive2D& rFillGraphicPrimitive2D);
    void processInvertPrimitive2D(const primitive2d::InvertPrimitive2D& rInvertPrimitive2D);
    void processFillGradientPrimitive2D(
        const primitive2d::FillGradientPrimitive2D& rFillGradientPrimitive2D);

    // common helpers
    sal::systools::COMReference<ID2D1Bitmap>
    implCreateAlpha_Direct(const primitive2d::TransparencePrimitive2D& rTransCandidate);
    sal::systools::COMReference<ID2D1Bitmap>
    implCreateAlpha_B2DBitmap(const primitive2d::TransparencePrimitive2D& rTransCandidate,
                              const basegfx::B2DRange& rVisibleRange,
                              D2D1_MATRIX_3X2_F& rMaskScale);
    bool drawPolyPolygonColorTransformed(const basegfx::B2DHomMatrix& rTansformation,
                                         const basegfx::B2DPolyPolygon& rPolyPolygon,
                                         const basegfx::BColor& rColor);

    /*  the local processor for BasePrimitive2D-Implementation based primitives,
        called from the common process()-implementation
     */
    virtual void processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate) override;

protected:
    // local protected minimal constructor for usage in derivates, e.g. helpers
    D2DPixelProcessor2D(const geometry::ViewInformation2D& rViewInformation);

    // local protected minimal accessors for usage in derivates, e.g. helpers
    void increaseError() { mnErrorCounter++; }
    bool hasError() const { return 0 != mnErrorCounter; }
    bool hasRenderTarget() const { return mpRT.is(); }

    void setRenderTarget(const sal::systools::COMReference<ID2D1RenderTarget>& rNewRT)
    {
        mpRT = rNewRT;
    }
    sal::systools::COMReference<ID2D1RenderTarget>& getRenderTarget() { return mpRT; }

public:
    bool valid() const { return hasRenderTarget() && !hasError(); }
    D2DPixelProcessor2D(const geometry::ViewInformation2D& rViewInformation, HDC aHdc);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
