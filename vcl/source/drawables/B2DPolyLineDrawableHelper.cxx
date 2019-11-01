/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with pRenderContext
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with pRenderContext work for additional information regarding copyright
 *   ownership. The ASF licenses pRenderContext file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use pRenderContext file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>

#include <vcl/lineinfo.hxx>
#include <vcl/outdev.hxx>
#include <vcl/drawables/PolyHairlineDrawable.hxx>
#include <vcl/drawables/B2DPolyPolyLineDrawable.hxx>

#include <drawables/B2DPolyLineDrawableHelper.hxx>
#include <salgdi.hxx>

namespace vcl
{
basegfx::B2DPolyPolygon
B2DPolyLineDrawableHelper::CreateFilledPolygon(basegfx::B2DPolygon const& rB2DPolygon,
                                               LineInfo const& rLineInfo, double fMiterMinimumAngle)
{
    const double fHalfLineWidth((rLineInfo.GetWidth() * 0.5) + 0.5);
    return basegfx::utils::createAreaGeometry(rB2DPolygon, fHalfLineWidth, rLineInfo.GetLineJoin(),
                                              rLineInfo.GetLineCap(), fMiterMinimumAngle);
}

void B2DPolyLineDrawableHelper::DrawPolyPolygonOutline(
    OutputDevice* pRenderContext, basegfx::B2DPolyPolygon const& rAreaPolyPolygon)
{
    SetFillColor fillcolor(pRenderContext);

    // draw using a loop; else the topology will paint a PolyPolygon
    for (auto const& rPolygon : rAreaPolyPolygon)
    {
        pRenderContext->Draw(vcl::B2DPolyPolyLineDrawable(basegfx::B2DPolyPolygon(rPolygon)));
    }
}

void B2DPolyLineDrawableHelper::FillPolyPolygon(OutputDevice* pRenderContext,
                                                basegfx::B2DPolyPolygon const& rAreaPolyPolygon,
                                                LineInfo const& rLineInfo)
{
    // when AA it is necessary to also paint the filled polygon's outline
    // to avoid optical gaps
    for (auto const& rPolygon : rAreaPolyPolygon)
    {
        LineInfo aHairlineInfo;
        aHairlineInfo.SetWidth(rLineInfo.GetWidth());
        aHairlineInfo.SetLineJoin(basegfx::B2DLineJoin::NONE);
        aHairlineInfo.SetLineCap(css::drawing::LineCap_BUTT);

        pRenderContext->Draw(
            vcl::PolyHairlineDrawable(basegfx::B2DHomMatrix(), rPolygon, aHairlineInfo));
    }
}

bool B2DPolyLineDrawableHelper::DrawB2DPolyLine(OutputDevice* pRenderContext,
                                                basegfx::B2DPolygon const& rB2DPolygon,
                                                LineInfo const& rLineInfo,
                                                double fMiterMinimumAngle)
{
    if (rLineInfo.GetWidth() >= 2.5 && rB2DPolygon.count() && rB2DPolygon.count() <= 1000)
    {
        const basegfx::B2DPolyPolygon aAreaPolyPolygon
            = B2DPolyLineDrawableHelper::CreateFilledPolygon(rB2DPolygon, rLineInfo,
                                                             fMiterMinimumAngle);

        B2DPolyLineDrawableHelper::DrawPolyPolygonOutline(pRenderContext, aAreaPolyPolygon);
        B2DPolyLineDrawableHelper::FillPolyPolygon(pRenderContext, aAreaPolyPolygon, rLineInfo);

        return true;
    }

    return false;
}

bool B2DPolyLineDrawableHelper::DrawFallbackPolyLine(OutputDevice* pRenderContext,
                                                     basegfx::B2DPolygon const& rB2DPolygon,
                                                     LineInfo const& rLineInfo)
{
    const tools::Polygon aToolsPolygon(rB2DPolygon);
    LineInfo aLineInfo;
    if (rLineInfo.GetWidth() != 0.0)
        aLineInfo.SetWidth(static_cast<long>(rLineInfo.GetWidth() + 0.5));

    sal_uInt16 nPoints(aToolsPolygon.GetSize());

    if (nPoints < 2 || aLineInfo.GetStyle() == LineStyle::NONE)
        return false;

    tools::Polygon aPoly = pRenderContext->ImplLogicToDevicePixel(aToolsPolygon);

    const LineInfo aInfo(pRenderContext->ImplLogicToDevicePixel(aLineInfo));
    const bool bDashUsed(aInfo.GetStyle() == LineStyle::Dash);
    const bool bLineWidthUsed(aInfo.GetWidth() > 1);

    if (bDashUsed || bLineWidthUsed)
    {
        return pRenderContext->Draw(
            vcl::B2DPolyPolyLineDrawable(basegfx::B2DPolyPolygon(aPoly.getB2DPolygon()), aInfo));
    }
    else
    {
        // #100127# the subdivision HAS to be done here since only a pointer
        // to an array of points is given to the DrawPolyLine method, there is
        // NO way to find out there that it's a curve.
        if (aPoly.HasFlags())
        {
            aPoly = tools::Polygon::SubdivideBezier(aPoly);
            nPoints = aPoly.GetSize();
        }

        SalGraphics* pGraphics = pRenderContext->GetGraphics();
        pGraphics->DrawPolyLine(nPoints, reinterpret_cast<SalPoint*>(aPoly.GetPointAry()),
                                pRenderContext);
    }

    return true;
}
} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
