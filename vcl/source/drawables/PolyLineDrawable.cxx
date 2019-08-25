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

#include <sal/types.h>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>

#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/outdev.hxx>
#include <vcl/settings.hxx>
#include <vcl/virdev.hxx>
#include <vcl/window.hxx>
#include <vcl/drawables/B2DPolyLineDrawable.hxx>
#include <vcl/drawables/B2DPolyPolyLineDrawable.hxx>
#include <vcl/drawables/PolyLineDrawable.hxx>
#include <vcl/drawables/PolyHairlineDrawable.hxx>

#include <salgdi.hxx>

#include <cassert>

namespace vcl
{
bool PolyLineDrawable::DrawCommand(OutputDevice* pRenderContext) const
{
    return Draw(pRenderContext, maPolygon, maLineInfo);
}

bool PolyLineDrawable::Draw(OutputDevice* pRenderContext, tools::Polygon const& rPoly,
                            LineInfo const& rLineInfo) const
{
    sal_uInt16 nPoints = rPoly.GetSize();
    if (nPoints < 2)
        return false;

    if (rLineInfo.IsDefault())
    {
        LineInfo aHairlineInfo;
        aHairlineInfo.SetLineJoin(basegfx::B2DLineJoin::NONE);
        aHairlineInfo.SetLineCap(css::drawing::LineCap_BUTT);

        // use b2dpolygon drawing if possible
        if (pRenderContext->Draw(vcl::PolyHairlineDrawable(basegfx::B2DHomMatrix(),
                                                           rPoly.getB2DPolygon(), aHairlineInfo)))
        {
            return true;
        }

        const basegfx::B2DPolygon aB2DPolyLine(rPoly.getB2DPolygon());
        const basegfx::B2DHomMatrix aTransform(pRenderContext->ImplGetDeviceTransformation());
        const basegfx::B2DVector aB2DLineWidth(1.0, 1.0);
        const bool bPixelSnapHairline(pRenderContext->GetAntialiasing()
                                      & AntialiasingFlags::PixelSnapHairline);

        if (mpGraphics->DrawPolyLine(
                aTransform, aB2DPolyLine, 0.0, aB2DLineWidth, basegfx::B2DLineJoin::NONE,
                css::drawing::LineCap_BUTT,
                basegfx::deg2rad(15.0) /*default fMiterMinimumAngle, not used*/, bPixelSnapHairline,
                pRenderContext))
        {
            return true;
        }

        tools::Polygon aPoly = pRenderContext->ImplLogicToDevicePixel(rPoly);
        SalPoint* pPtAry = reinterpret_cast<SalPoint*>(aPoly.GetPointAry());

        // #100127# Forward beziers to sal, if any
        if (aPoly.HasFlags())
        {
            const PolyFlags* pFlgAry = aPoly.GetConstFlagAry();
            if (!mpGraphics->DrawPolyLineBezier(nPoints, pPtAry, pFlgAry, pRenderContext))
            {
                aPoly = tools::Polygon::SubdivideBezier(aPoly);
                pPtAry = reinterpret_cast<SalPoint*>(aPoly.GetPointAry());
                mpGraphics->DrawPolyLine(aPoly.GetSize(), pPtAry, pRenderContext);
            }
        }
        else
        {
            mpGraphics->DrawPolyLine(nPoints, pPtAry, pRenderContext);
        }
    }
    else
    {
        tools::Polygon aPoly = pRenderContext->ImplLogicToDevicePixel(rPoly);

        // #i101491#
        // Try direct Fallback to B2D-Version of DrawPolyLine
        if ((pRenderContext->GetAntialiasing() & AntialiasingFlags::EnableB2dDraw)
            && rLineInfo.GetStyle() == LineStyle::Solid)
        {
            return pRenderContext->Draw(vcl::B2DPolyLineDrawable(
                rPoly.getB2DPolygon(), rLineInfo,
                basegfx::deg2rad(
                    15.0), /* default fMiterMinimumAngle, value not available in LineInfo */
                false));
        }

        const LineInfo aInfo(pRenderContext->ImplLogicToDevicePixel(rLineInfo));
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
    }

    DrawAlphaVirtDev(pRenderContext);
    return true;
}

bool PolyLineDrawable::CanDraw(OutputDevice* pRenderContext) const
{
    return (!(!pRenderContext->IsDeviceOutputNecessary()
              || (!pRenderContext->IsLineColor() && !pRenderContext->IsFillColor())
              || pRenderContext->ImplIsRecordLayout()));
}
} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
