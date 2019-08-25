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
#include <vcl/drawables/B2DPolyPolyLineDrawable.hxx>

#include <salgdi.hxx>
#include <outdata.hxx>

#include <cassert>
#include <numeric>

namespace vcl
{
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
    basegfx::B2DPolyPolygon aFillPolyPolygon;
    const bool bDashUsed(LineStyle::Dash == rLineInfo.GetStyle());
    const bool bLineWidthUsed(rLineInfo.GetWidth() > 1);

    basegfx::B2DPolyPolygon aLinePolyPolygon(rLinePolyPolygon);

    if (bDashUsed && aLinePolyPolygon.count())
    {
        ::std::vector<double> fDotDashArray;
        const double fDashLen(rLineInfo.GetDashLen());
        const double fDotLen(rLineInfo.GetDotLen());
        const double fDistance(rLineInfo.GetDistance());

        for (sal_uInt16 a(0); a < rLineInfo.GetDashCount(); a++)
        {
            fDotDashArray.push_back(fDashLen);
            fDotDashArray.push_back(fDistance);
        }

        for (sal_uInt16 b(0); b < rLineInfo.GetDotCount(); b++)
        {
            fDotDashArray.push_back(fDotLen);
            fDotDashArray.push_back(fDistance);
        }

        const double fAccumulated(
            ::std::accumulate(fDotDashArray.begin(), fDotDashArray.end(), 0.0));

        if (fAccumulated > 0.0)
        {
            basegfx::B2DPolyPolygon aResult;

            for (auto const& rPolygon : aLinePolyPolygon)
            {
                basegfx::B2DPolyPolygon aLineTarget;
                basegfx::utils::applyLineDashing(rPolygon, fDotDashArray, &aLineTarget);
                aResult.append(aLineTarget);
            }

            aLinePolyPolygon = aResult;
        }
    }

    if (bLineWidthUsed && aLinePolyPolygon.count())
    {
        const double fHalfLineWidth((rLineInfo.GetWidth() * 0.5) + 0.5);

        if (aLinePolyPolygon.areControlPointsUsed())
        {
            // #i110768# When area geometry has to be created, do not
            // use the fallback bezier decomposition inside createAreaGeometry,
            // but one that is at least as good as ImplSubdivideBezier was.
            // There, Polygon::AdaptiveSubdivide was used with default parameter
            // 1.0 as quality index.
            aLinePolyPolygon = basegfx::utils::adaptiveSubdivideByDistance(aLinePolyPolygon, 1.0);
        }

        for (auto const& rPolygon : aLinePolyPolygon)
        {
            aFillPolyPolygon.append(basegfx::utils::createAreaGeometry(
                rPolygon, fHalfLineWidth, rLineInfo.GetLineJoin(), rLineInfo.GetLineCap()));
        }

        aLinePolyPolygon.clear();
    }

    GDIMetaFile* pOldMetaFile = pRenderContext->GetConnectMetaFile();
    pRenderContext->SetConnectMetaFile(nullptr);

    const bool bTryAA((pRenderContext->GetAntialiasing() & AntialiasingFlags::EnableB2dDraw)
                      && mpGraphics->supportsOperation(OutDevSupportType::B2DDraw)
                      && pRenderContext->GetRasterOp() == RasterOp::OverPaint
                      && pRenderContext->IsLineColor());

    if (aLinePolyPolygon.count())
    {
        for (auto const& rB2DPolygon : aLinePolyPolygon)
        {
            const bool bPixelSnapHairline(pRenderContext->GetAntialiasing()
                                          & AntialiasingFlags::PixelSnapHairline);
            bool bDone = false;

            if (bTryAA)
            {
                bDone = mpGraphics->DrawPolyLine(
                    basegfx::B2DHomMatrix(), rB2DPolygon, 0.0, basegfx::B2DVector(1.0, 1.0),
                    basegfx::B2DLineJoin::NONE, css::drawing::LineCap_BUTT,
                    basegfx::deg2rad(
                        15.0), // not used with B2DLineJoin::NONE, but the correct default
                    bPixelSnapHairline, pRenderContext);
            }

            if (!bDone)
            {
                tools::Polygon aPolygon(rB2DPolygon);
                mpGraphics->DrawPolyLine(aPolygon.GetSize(),
                                         reinterpret_cast<SalPoint*>(aPolygon.GetPointAry()),
                                         pRenderContext);
            }
        }
    }

    if (aFillPolyPolygon.count())
    {
        const Color aOldLineColor(pRenderContext->GetLineColor());
        const Color aOldFillColor(pRenderContext->GetFillColor());

        pRenderContext->SetLineColor();
        pRenderContext->InitLineColor();
        pRenderContext->SetFillColor(aOldLineColor);
        pRenderContext->InitFillColor();

        bool bDone = false;

        if (bTryAA)
        {
            bDone = mpGraphics->DrawPolyPolygon(basegfx::B2DHomMatrix(), aFillPolyPolygon, 0.0,
                                                pRenderContext);
        }

        if (!bDone)
        {
            for (auto const& rB2DPolygon : aFillPolyPolygon)
            {
                tools::Polygon aPolygon(rB2DPolygon);

                // need to subdivide, mpGraphics->DrawPolygon ignores curves
                aPolygon.AdaptiveSubdivide(aPolygon);
                mpGraphics->DrawPolygon(
                    aPolygon.GetSize(),
                    reinterpret_cast<const SalPoint*>(aPolygon.GetConstPointAry()), pRenderContext);
            }
        }

        pRenderContext->SetFillColor(aOldFillColor);
        pRenderContext->SetLineColor(aOldLineColor);
    }

    pRenderContext->SetConnectMetaFile(pOldMetaFile);

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

        if (pRenderContext->IsFillColor())
        {
            bSuccess
                = mpGraphics->DrawPolyPolygon(aTransform, aB2DPolyPolygon, 0.0, pRenderContext);
        }

        if (bSuccess && pRenderContext->IsLineColor())
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

        if (bSuccess)
            return true;
    }

    // fallback to old polygon drawing if needed
    const tools::PolyPolygon aToolsPolyPolygon(rLinePolyPolygon);
    const tools::PolyPolygon aPixelPolyPolygon
        = pRenderContext->ImplLogicToDevicePixel(aToolsPolyPolygon);
    pRenderContext->ImplDrawPolyPolygon(aPixelPolyPolygon.Count(), aPixelPolyPolygon);

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
