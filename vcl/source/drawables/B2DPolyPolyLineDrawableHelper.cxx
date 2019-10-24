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

#include <vcl/lineinfo.hxx>
#include <vcl/outdev.hxx>
#include <vcl/drawables/PolyPolygonDrawable.hxx>

#include <salgdi.hxx>
#include <drawables/B2DPolyPolyLineDrawableHelper.hxx>

#include <cassert>
#include <numeric>

namespace vcl
{
bool B2DPolyPolyLineDrawableHelper::CanApplyDashes(basegfx::B2DPolyPolygon const& rLinePolyPolygon,
                                                   LineInfo const& rLineInfo)
{
    return (rLineInfo.GetStyle() == LineStyle::Dash && rLinePolyPolygon.count());
}

static void PushDashDotElements(::std::vector<double>& fDotDashArray, sal_uInt16 nElements,
                                double fLen, double fDistance)
{
    for (sal_uInt16 i(0); i < nElements; i++)
    {
        fDotDashArray.push_back(fLen);
        fDotDashArray.push_back(fDistance);
    }
}

::std::vector<double> B2DPolyPolyLineDrawableHelper::GenerateDotDashArray(LineInfo const& rLineInfo)
{
    ::std::vector<double> fDotDashArray;

    PushDashDotElements(fDotDashArray, rLineInfo.GetDashCount(), rLineInfo.GetDashLen(),
                        rLineInfo.GetDistance());
    PushDashDotElements(fDotDashArray, rLineInfo.GetDotCount(), rLineInfo.GetDotLen(),
                        rLineInfo.GetDistance());

    return fDotDashArray;
}

bool B2DPolyPolyLineDrawableHelper::DashDotArrayHasLength(
    ::std::vector<double> const& rDotDashArray)
{
    const double fAccumulated(::std::accumulate(rDotDashArray.begin(), rDotDashArray.end(), 0.0));

    return (fAccumulated > 0.0);
}

basegfx::B2DPolyPolygon
B2DPolyPolyLineDrawableHelper::ApplyLineDashing(basegfx::B2DPolyPolygon const& rLinePolyPolygon,
                                                LineInfo const& rLineInfo)
{
    assert(rLinePolyPolygon.count());

    basegfx::B2DPolyPolygon aLinePolyPolygon(rLinePolyPolygon);

    if (CanApplyDashes(aLinePolyPolygon, rLineInfo))
    {
        ::std::vector<double> fDotDashArray = GenerateDotDashArray(rLineInfo);

        if (DashDotArrayHasLength(fDotDashArray))
        {
            basegfx::B2DPolyPolygon aResult;

            for (auto const& rPolygon : rLinePolyPolygon)
            {
                basegfx::B2DPolyPolygon aLineTarget;
                basegfx::utils::applyLineDashing(rPolygon, fDotDashArray, &aLineTarget);
                aResult.append(aLineTarget);
            }

            aLinePolyPolygon = aResult;
        }
    }

    return aLinePolyPolygon;
}

bool B2DPolyPolyLineDrawableHelper::UseLineWidth(basegfx::B2DPolyPolygon const& rLinePolyPolygon,
                                                 LineInfo const& rLineInfo)
{
    return (rLineInfo.GetWidth() > 1 && rLinePolyPolygon.count());
}

// TODO: rLinePolyPolygon is changed, probably need to prevent this somehow
basegfx::B2DPolyPolygon
B2DPolyPolyLineDrawableHelper::CreateFillPolyPolygon(basegfx::B2DPolyPolygon& rLinePolyPolygon,
                                                     LineInfo const& rLineInfo)
{
    assert(rLinePolyPolygon.count());

    basegfx::B2DPolyPolygon aFillPolyPolygon;

    if (UseLineWidth(rLinePolyPolygon, rLineInfo))
    {
        const double fHalfLineWidth((rLineInfo.GetWidth() * 0.5) + 0.5);

        if (rLinePolyPolygon.areControlPointsUsed())
        {
            // #i110768# When area geometry has to be created, do not
            // use the fallback bezier decomposition inside createAreaGeometry,
            // but one that is at least as good as ImplSubdivideBezier was.
            // There, Polygon::AdaptiveSubdivide was used with default parameter
            // 1.0 as quality index.
            rLinePolyPolygon = basegfx::utils::adaptiveSubdivideByDistance(rLinePolyPolygon, 1.0);
        }

        for (auto const& rPolygon : rLinePolyPolygon)
        {
            aFillPolyPolygon.append(basegfx::utils::createAreaGeometry(
                rPolygon, fHalfLineWidth, rLineInfo.GetLineJoin(), rLineInfo.GetLineCap()));
        }

        rLinePolyPolygon.clear();
    }

    return aFillPolyPolygon;
}

bool B2DPolyPolyLineDrawableHelper::CanAntialiasLine(OutputDevice* pRenderContext,
                                                     SalGraphics* pGraphics)
{
    return (pRenderContext->GetAntialiasing() & AntialiasingFlags::EnableB2dDraw)
           && pGraphics->supportsOperation(OutDevSupportType::B2DDraw)
           && pRenderContext->GetRasterOp() == RasterOp::OverPaint && pRenderContext->IsLineColor();
}

void B2DPolyPolyLineDrawableHelper::DrawPolyPolyLine(
    OutputDevice* pRenderContext, SalGraphics* const pGraphics,
    basegfx::B2DPolyPolygon const& rLinePolyPolygon)
{
    assert(rLinePolyPolygon.count());

    for (auto const& rB2DPolygon : rLinePolyPolygon)
    {
        if (!DrawPolyLine(pRenderContext, pGraphics, rB2DPolygon))
            DrawPolyLineFallback(pRenderContext, pGraphics, rB2DPolygon);
    }
}

void B2DPolyPolyLineDrawableHelper::DrawPolyLineFallback(OutputDevice* pRenderContext,
                                                         SalGraphics* const pGraphics,
                                                         basegfx::B2DPolygon const& rB2DPolygon)
{
    tools::Polygon aPolygon(rB2DPolygon);
    pGraphics->DrawPolyLine(aPolygon.GetSize(), reinterpret_cast<SalPoint*>(aPolygon.GetPointAry()),
                            pRenderContext);
}

bool B2DPolyPolyLineDrawableHelper::DrawPolyLine(OutputDevice* pRenderContext,
                                                 SalGraphics* const pGraphics,
                                                 basegfx::B2DPolygon const& rB2DPolygon)
{
    return CanAntialiasLine(pRenderContext, pGraphics)
           && pGraphics->DrawPolyLine(
               basegfx::B2DHomMatrix(), rB2DPolygon, 0.0, basegfx::B2DVector(1.0, 1.0),
               basegfx::B2DLineJoin::NONE, css::drawing::LineCap_BUTT,
               basegfx::deg2rad(15.0), // not used with B2DLineJoin::NONE, but the correct default
               bool(pRenderContext->GetAntialiasing() & AntialiasingFlags::PixelSnapHairline),
               pRenderContext);
}

void B2DPolyPolyLineDrawableHelper::FillPolyPolygon(OutputDevice* pRenderContext,
                                                    SalGraphics* const pGraphics,
                                                    basegfx::B2DPolyPolygon const& rFillPolyPolygon)
{
    assert(rFillPolyPolygon.count());

    ColorFillPolyPolygon aColorFillPolyPolygon(pRenderContext);

    if (CanAntialiasLine(pRenderContext, pGraphics)
        && !pGraphics->DrawPolyPolygon(basegfx::B2DHomMatrix(), rFillPolyPolygon, 0.0,
                                       pRenderContext))
    {
        for (auto const& rB2DPolygon : rFillPolyPolygon)
        {
            tools::Polygon aPolygon(rB2DPolygon);

            // need to subdivide, pGraphics->DrawPolygon ignores curves
            aPolygon.AdaptiveSubdivide(aPolygon);
            pGraphics->DrawPolygon(aPolygon.GetSize(),
                                   reinterpret_cast<const SalPoint*>(aPolygon.GetConstPointAry()),
                                   pRenderContext);
        }
    }
}

void B2DPolyPolyLineDrawableHelper::DrawPolyPolygonFallback(
    OutputDevice* pRenderContext, basegfx::B2DPolyPolygon const& rLinePolyPolygon)
{
    const tools::PolyPolygon aToolsPolyPolygon(rLinePolyPolygon);
    const tools::PolyPolygon aPixelPolyPolygon
        = pRenderContext->ImplLogicToDevicePixel(aToolsPolyPolygon);

    pRenderContext->Draw(PolyPolygonDrawable(aPixelPolyPolygon.Count(), aPixelPolyPolygon));
}

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
