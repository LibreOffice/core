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
#include <vcl/drawables/LineDrawable.hxx>
#include <vcl/drawables/B2DPolyLineDrawable.hxx>

#include <salgdi.hxx>
#include <outdata.hxx>

#include <cassert>
#include <numeric>

namespace vcl
{
bool LineDrawable::DrawCommand(OutputDevice* pRenderContext) const
{
    return Draw(pRenderContext, maStartPt, maEndPt, maLineInfo);
}

bool LineDrawable::Draw(OutputDevice* pRenderContext, Point const& rStartPt, Point const& rEndPt,
                        LineInfo const& rLineInfo) const
{
    if (rLineInfo.IsDefault())
    {
        // #i101598# support AA and snap for lines, too
        if ((pRenderContext->GetAntialiasing() & AntialiasingFlags::EnableB2dDraw)
            && mpGraphics->supportsOperation(OutDevSupportType::B2DDraw)
            && pRenderContext->GetRasterOp() == RasterOp::OverPaint
            && pRenderContext->IsLineColor())
        {
            // at least transform with double precision to device coordinates; this will
            // avoid pixel snap of single, appended lines
            const basegfx::B2DHomMatrix aTransform(pRenderContext->ImplGetDeviceTransformation());
            const basegfx::B2DVector aB2DLineWidth(1.0, 1.0);
            basegfx::B2DPolygon aB2DPolyLine;

            aB2DPolyLine.append(basegfx::B2DPoint(rStartPt.X(), rStartPt.Y()));
            aB2DPolyLine.append(basegfx::B2DPoint(rEndPt.X(), rEndPt.Y()));
            aB2DPolyLine.transform(aTransform);

            const bool bPixelSnapHairline(pRenderContext->GetAntialiasing()
                                          & AntialiasingFlags::PixelSnapHairline);

            if (mpGraphics->DrawPolyLine(
                    basegfx::B2DHomMatrix(), aB2DPolyLine, 0.0, aB2DLineWidth,
                    basegfx::B2DLineJoin::NONE, css::drawing::LineCap_BUTT,
                    basegfx::deg2rad(
                        15.0), // not used with B2DLineJoin::NONE, but the correct default
                    bPixelSnapHairline, pRenderContext))
            {
                return true;
            }
        }

        const Point aStartPt(pRenderContext->ImplLogicToDevicePixel(rStartPt));
        const Point aEndPt(pRenderContext->ImplLogicToDevicePixel(rEndPt));

        mpGraphics->DrawLine(aStartPt.X(), aStartPt.Y(), aEndPt.X(), aEndPt.Y(), pRenderContext);
    }
    else
    {
        const Point aStartPt(pRenderContext->ImplLogicToDevicePixel(rStartPt));
        const Point aEndPt(pRenderContext->ImplLogicToDevicePixel(rEndPt));
        const LineInfo aInfo(pRenderContext->ImplLogicToDevicePixel(rLineInfo));
        const bool bDashUsed(aInfo.GetStyle() == LineStyle::Dash);
        const bool bLineWidthUsed(aInfo.GetWidth() > 1);

        if (bDashUsed || bLineWidthUsed)
        {
            basegfx::B2DPolygon aLinePolygon;
            aLinePolygon.append(basegfx::B2DPoint(aStartPt.X(), aStartPt.Y()));
            aLinePolygon.append(basegfx::B2DPoint(aEndPt.X(), aEndPt.Y()));

            pRenderContext->Draw(
                B2DPolyLineDrawable(basegfx::B2DPolyPolygon(aLinePolygon), aInfo, false));
        }
        else
        {
            mpGraphics->DrawLine(aStartPt.X(), aStartPt.Y(), aEndPt.X(), aEndPt.Y(),
                                 pRenderContext);
        }
    }

    return true;
}

bool LineDrawable::CanDraw(OutputDevice* pRenderContext) const
{
    return (!(!pRenderContext->IsDeviceOutputNecessary()
              || (!pRenderContext->IsLineColor() && !pRenderContext->IsFillColor())
              || pRenderContext->ImplIsRecordLayout()));
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
