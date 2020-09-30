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

#include "canvashelper.hxx"

#include <vcl/outdev.hxx>
#include <vcl/skia/interface.hxx>
#include <vcl/canvastools.hxx>
#include <canvas/canvastools.hxx>
#include <com/sun/star/rendering/CompositeOperation.hpp>

#include <SkMatrix.h>
#include <SkPaint.h>

#include "impltools.hxx"

using namespace ::com::sun::star;

namespace vclcanvas
{
static void setupPaint(SkPaint* paint, SkMatrix* matrix, const rendering::ViewState& viewState,
                       const rendering::RenderState& renderState, int* transparency = nullptr)
{
    SkMatrix viewMatrix
        = SkMatrix::MakeAll(viewState.AffineTransform.m00, viewState.AffineTransform.m01,
                            viewState.AffineTransform.m02, viewState.AffineTransform.m10,
                            viewState.AffineTransform.m11, viewState.AffineTransform.m12, 0, 0, 1);
    SkMatrix renderMatrix = SkMatrix::MakeAll(
        renderState.AffineTransform.m00, renderState.AffineTransform.m01,
        renderState.AffineTransform.m02, renderState.AffineTransform.m10,
        renderState.AffineTransform.m11, renderState.AffineTransform.m12, 0, 0, 1);
    *matrix = viewMatrix * renderMatrix;

    paint->setAntiAlias(true);

    Color color(COL_WHITE);
    if (renderState.DeviceColor.getLength() > 2)
        color = vcl::unotools::stdColorSpaceSequenceToColor(renderState.DeviceColor);

    // SetupOutDev() does this, probably because of the 2nd alpha device.
    if (transparency)
        *transparency = color.GetTransparency();
    color.SetTransparency(0);
    paint->setColor(SkColorSetRGB(color.GetRed(), color.GetGreen(), color.GetBlue()));

    switch (renderState.CompositeOperation)
    {
        case rendering::CompositeOperation::CLEAR:
            paint->setBlendMode(SkBlendMode::kClear);
            break;
        case rendering::CompositeOperation::SOURCE:
            paint->setBlendMode(SkBlendMode::kSrc);
            break;
        case rendering::CompositeOperation::DESTINATION:
            paint->setBlendMode(SkBlendMode::kDst);
            break;
        case rendering::CompositeOperation::OVER:
            paint->setBlendMode(SkBlendMode::kSrcOver);
            break;
        case rendering::CompositeOperation::UNDER:
            paint->setBlendMode(SkBlendMode::kDstOver);
            break;
        case rendering::CompositeOperation::INSIDE:
            paint->setBlendMode(SkBlendMode::kSrcIn);
            break;
        case rendering::CompositeOperation::INSIDE_REVERSE:
            paint->setBlendMode(SkBlendMode::kSrcOut);
            break;
        case rendering::CompositeOperation::OUTSIDE:
            paint->setBlendMode(SkBlendMode::kDstIn);
            break;
        case rendering::CompositeOperation::OUTSIDE_REVERSE:
            paint->setBlendMode(SkBlendMode::kDstOut);
            break;
        case rendering::CompositeOperation::ATOP:
            paint->setBlendMode(SkBlendMode::kSrcATop);
            break;
        case rendering::CompositeOperation::ATOP_REVERSE:
            paint->setBlendMode(SkBlendMode::kDstATop);
            break;
        case rendering::CompositeOperation::XOR:
            paint->setBlendMode(SkBlendMode::kXor);
            break;
        case rendering::CompositeOperation::ADD:
            paint->setBlendMode(SkBlendMode::kPlus);
            break;
        case rendering::CompositeOperation::SATURATE:
            paint->setBlendMode(SkBlendMode::kModulate);
            break;
    }
}

namespace
{
// Calling OutDev::SetClipRegion() wouldn't work with SkiaOutDevInterface calls,
// because OutDev actually applies the clip region on-demand only when making
// a drawing call, which is bypassed here. So use the interface setClipRegion() call.
// Constructor sets the clip region, destructor restores the original one.
class TemporaryClipper
{
public:
    TemporaryClipper(SkiaOutDevInterface* interface_, const vcl::Region& region)
        : interface(interface_)
        , originalRegion(interface->getClipRegion())
    {
        interface->setClipRegion(region);
    }
    ~TemporaryClipper() { interface->setClipRegion(originalRegion); }

private:
    SkiaOutDevInterface* interface;
    vcl::Region originalRegion;
};
} // namespace

void CanvasHelper::initSkia(OutputDevice& outputDevice)
{
    mpSkiaInterface = outputDevice.GetSkiaInterface();
}

void CanvasHelper::init2ndSkia(OutputDevice& outputDevice)
{
    mp2ndSkiaInterface = outputDevice.GetSkiaInterface();
}

bool CanvasHelper::implDrawBitmapSkia(const rendering::XCanvas*,
                                      const uno::Reference<rendering::XBitmap>& xBitmap,
                                      const rendering::ViewState& viewState,
                                      const rendering::RenderState& renderState,
                                      bool bModulateColors)
{
    if (!mpSkiaInterface)
        return false;
    vcl::Region clipRegion = canvas::tools::stateToClipRegion(viewState, renderState);
    SkPaint paint;
    SkMatrix matrix;
    setupPaint(&paint, &matrix, viewState, renderState);
    // If we're painting with the alpha outdev hack, then the alpha needs to be handled
    // as kMultiply (see below and SkiaSalGraphicsImpl::blendBitmap()). But for that
    // the 1st outdev mode needs to be kSrcOver (i.e. the normal one).
    if (mp2ndSkiaInterface && paint.getBlendMode() != SkBlendMode::kSrcOver)
    {
        SAL_WARN("canvas.skia", "Unimplemented alpha handling for "
                                    << SkBlendMode_Name(paint.getBlendMode()) << "mode");
        return false;
    }
    BitmapEx bitmap(tools::bitmapExFromXBitmap(xBitmap));
    double alphaModulation = 1;
    if (bModulateColors && renderState.DeviceColor.getLength() > 3)
        alphaModulation = renderState.DeviceColor[3];
    TemporaryClipper clip(mpSkiaInterface, clipRegion);
    if (!mpSkiaInterface->drawBitmap(bitmap, paint, matrix, alphaModulation,
                                     SkiaOutDevInterface::FailIfSlow))
        return false;
    if (mp2ndSkiaInterface)
    {
        BitmapEx alpha;
        if (bitmap.IsAlpha())
            alpha = bitmap.GetAlpha();
        else
            alpha = bitmap.GetMask();
        if (!!alpha)
        {
            // Alpha outdev hack. See above and SkiaSalGraphicsImpl::blendBitmap().
            paint.setBlendMode(SkBlendMode::kMultiply);
            TemporaryClipper clip2nd(mp2ndSkiaInterface, clipRegion);
            if (!mp2ndSkiaInterface->drawBitmap(alpha, paint, matrix, alphaModulation,
                                                SkiaOutDevInterface::FailIfSlow))
            {
                assert(false); // Should not happen, only half is painted.
                return false;
            }
        }
    }
    return true;
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
