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

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>

#include <vcl/gdimtf.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/window.hxx>
#include <vcl/drawables/PolyPolygonDrawable.hxx>
#include <vcl/drawables/B2DPolyPolyLineDrawable.hxx>

#include <salgdi.hxx>
#include <outdata.hxx>
#include <drawables/B2DPolyPolyLineDrawableHelper.hxx>

#include <cassert>
#include <numeric>

namespace vcl
{
B2DPolyPolyLineDrawable::B2DPolyPolyLineDrawable(basegfx::B2DPolyPolygon aLinePolyPolygon)
    : maLinePolyPolygon(aLinePolyPolygon)
    , mbUsesLineInfo(false)
{
}

B2DPolyPolyLineDrawable::B2DPolyPolyLineDrawable(basegfx::B2DPolyPolygon aLinePolyPolygon,
                                                 LineInfo const aLineInfo, bool bUsesScaffolding)
    : Drawable(bUsesScaffolding)
    , maLinePolyPolygon(aLinePolyPolygon)
    , maLineInfo(aLineInfo)
    , mbUsesLineInfo(true)
{
}

bool B2DPolyPolyLineDrawable::DrawCommand(OutputDevice* pRenderContext) const
{
    if (mbUsesLineInfo)
        return Draw(pRenderContext, maLinePolyPolygon, maLineInfo);
    else
        return Draw(pRenderContext, maLinePolyPolygon);
}

bool B2DPolyPolyLineDrawable::Draw(OutputDevice* pRenderContext,
                                   basegfx::B2DPolyPolygon const& rLinePolyPolygon,
                                   LineInfo const& rLineInfo) const
{
    basegfx::B2DPolyPolygon aLinePolyPolygon
        = B2DPolyPolyLineDrawableHelper::ApplyLineDashing(rLinePolyPolygon, rLineInfo);
    basegfx::B2DPolyPolygon aFillPolyPolygon
        = B2DPolyPolyLineDrawableHelper::CreateFillPolyPolygon(aLinePolyPolygon, rLineInfo);

    DisableMetafileProcessing aDisableMtf(pRenderContext);

    B2DPolyPolyLineDrawableHelper::DrawPolyLine(pRenderContext, mpGraphics, rLinePolyPolygon);
    B2DPolyPolyLineDrawableHelper::FillPolyPolygon(pRenderContext, mpGraphics, aFillPolyPolygon);

    return true;
}

bool B2DPolyPolyLineDrawable::Draw(OutputDevice* pRenderContext,
                                   basegfx::B2DPolyPolygon const& rLinePolyPolygon) const
{
    // Do not paint empty PolyPolygons
    if (!rLinePolyPolygon.count())
        return false;

    if ((pRenderContext->GetAntialiasing() & AntialiasingFlags::EnableB2dDraw)
        && mpGraphics->supportsOperation(OutDevSupportType::B2DDraw)
        && pRenderContext->GetRasterOp() == RasterOp::OverPaint
        && (pRenderContext->IsLineColor() || pRenderContext->IsFillColor()))
    {
        const basegfx::B2DHomMatrix aTransform(pRenderContext->ImplGetDeviceTransformation());
        basegfx::B2DPolyPolygon aB2DPolyPolygon(rLinePolyPolygon);

        // ensure closed - maybe assert, hinders buffering
        if (!aB2DPolyPolygon.isClosed())
            aB2DPolyPolygon.setClosed(true);

        bool bSuccess = true;

        if (pRenderContext->IsFillColor()
            && mpGraphics->DrawPolyPolygon(aTransform, aB2DPolyPolygon, 0.0, pRenderContext)
        {
            if (pRenderContext->IsLineColor())
            {
                const basegfx::B2DVector aB2DLineWidth(1.0, 1.0);
                const bool bPixelSnapHairline(pRenderContext->GetAntialiasing()
                                              & AntialiasingFlags::PixelSnapHairline);

                for (auto const& rPolygon : aB2DPolyPolygon)
                {
                    bSuccess = mpGraphics->DrawPolyLine(
                        aTransform, rPolygon, 0.0, aB2DLineWidth, basegfx::B2DLineJoin::NONE,
                        css::drawing::LineCap_BUTT,
                        basegfx::deg2rad(
                            15.0), // not used with B2DLineJoin::NONE, but the correct default
                        bPixelSnapHairline, pRenderContext);

                    if (!bSuccess)
                        break;
                }
            }
        }

        if (bSuccess)
            return true;
    }

    // fallback to old polygon drawing if needed
    const tools::PolyPolygon aToolsPolyPolygon(rLinePolyPolygon);
    const tools::PolyPolygon aPixelPolyPolygon
        = pRenderContext->ImplLogicToDevicePixel(aToolsPolyPolygon);
    pRenderContext->Draw(PolyPolygonDrawable(aPixelPolyPolygon.Count(), aPixelPolyPolygon));

    return true;
}

bool B2DPolyPolyLineDrawable::CanDraw(OutputDevice* pRenderContext) const
{
    return (!(!pRenderContext->IsDeviceOutputNecessary()
              || (!pRenderContext->IsLineColor() && !pRenderContext->IsFillColor())
              || pRenderContext->ImplIsRecordLayout()));
}
} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
