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

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>

#include <vcl/gdimtf.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/window.hxx>
#include <vcl/drawables/B2DPolyLineDrawable.hxx>
#include <vcl/drawables/B2DPolyPolyLineDrawable.hxx>
#include <vcl/drawables/PolyHairlineDrawable.hxx>

#include <salgdi.hxx>
#include <outdata.hxx>

#include <cassert>
#include <numeric>

namespace vcl
{
bool B2DPolyLineDrawable::DrawCommand(OutputDevice* pRenderContext) const
{
    return Draw(pRenderContext, maPolyLine, maLineInfo, mfMiterMinimumAngle);
}

bool B2DPolyLineDrawable::Draw(OutputDevice* pRenderContext, basegfx::B2DPolygon const& rB2DPolygon,
                               LineInfo const& rLineInfo, double fMiterMinimumAngle) const
{
    {
        LineInfo aHairlineInfo;
        aHairlineInfo.SetWidth(rLineInfo.GetWidth());
        aHairlineInfo.SetLineJoin(rLineInfo.GetLineJoin());
        aHairlineInfo.SetLineCap(rLineInfo.GetLineCap());

        // use b2dpolygon drawing if possible
        if (pRenderContext->Draw(vcl::PolyHairlineDrawable(basegfx::B2DHomMatrix(), rB2DPolygon,
                                                           aHairlineInfo, 0.0, fMiterMinimumAngle)))
        {
            return true;
        }
    }

    // #i101491#
    // no output yet; fallback to geometry decomposition and use filled polygon paint
    // when line is fat and not too complex. ImplDrawPolyPolygonWithB2DPolyPolygon
    // will do internal needed AA checks etc.
    if (rLineInfo.GetWidth() >= 2.5 && rB2DPolygon.count() && rB2DPolygon.count() <= 1000)
    {
        const double fHalfLineWidth((rLineInfo.GetWidth() * 0.5) + 0.5);
        const basegfx::B2DPolyPolygon aAreaPolyPolygon(
            basegfx::utils::createAreaGeometry(rB2DPolygon, fHalfLineWidth, rLineInfo.GetLineJoin(),
                                               rLineInfo.GetLineCap(), fMiterMinimumAngle));
        const Color aOldLineColor(pRenderContext->GetLineColor());
        const Color aOldFillColor(pRenderContext->GetFillColor());

        pRenderContext->SetLineColor();
        pRenderContext->InitLineColor();
        pRenderContext->SetFillColor(aOldLineColor);
        pRenderContext->InitFillColor();

        // draw using a loop; else the topology will paint a PolyPolygon
        for (auto const& rPolygon : aAreaPolyPolygon)
        {
            pRenderContext->Draw(vcl::B2DPolyPolyLineDrawable(basegfx::B2DPolyPolygon(rPolygon)));
        }

        pRenderContext->SetLineColor(aOldLineColor);
        pRenderContext->InitLineColor();
        pRenderContext->SetFillColor(aOldFillColor);
        pRenderContext->InitFillColor();

        // when AA it is necessary to also paint the filled polygon's outline
        // to avoid optical gaps
        for (auto const& rPolygon : aAreaPolyPolygon)
        {
            LineInfo aHairlineInfo;
            aHairlineInfo.SetWidth(rLineInfo.GetWidth());
            aHairlineInfo.SetLineJoin(basegfx::B2DLineJoin::NONE);
            aHairlineInfo.SetLineCap(css::drawing::LineCap_BUTT);

            pRenderContext->Draw(
                vcl::PolyHairlineDrawable(basegfx::B2DHomMatrix(), rPolygon, aHairlineInfo));
        }
    }
    else
    {
        // fallback to old polygon drawing if needed
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
            pRenderContext->Draw(vcl::B2DPolyPolyLineDrawable(
                basegfx::B2DPolyPolygon(aPoly.getB2DPolygon()), aInfo));
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

            mpGraphics->DrawPolyLine(nPoints, reinterpret_cast<SalPoint*>(aPoly.GetPointAry()),
                                     pRenderContext);
        }

        DrawAlphaVirtDev(pRenderContext);
    }

    return true;
}

bool B2DPolyLineDrawable::CanDraw(OutputDevice* pRenderContext) const
{
    return (!(!pRenderContext->IsDeviceOutputNecessary()
              || (!pRenderContext->IsLineColor() && !pRenderContext->IsFillColor())
              || pRenderContext->ImplIsRecordLayout()));
}
} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
