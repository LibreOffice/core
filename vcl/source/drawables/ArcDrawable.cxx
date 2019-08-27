/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <tools/poly.hxx>

#include <vcl/drawables/ArcDrawable.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>

#include <salgdi.hxx>

namespace vcl
{
bool ArcDrawable::DrawCommand(OutputDevice* pRenderContext) const
{
    tools::Rectangle aRect(pRenderContext->ImplLogicToDevicePixel(maRect));
    if (aRect.IsEmpty())
        return false;

    const Point aStart(pRenderContext->ImplLogicToDevicePixel(maStartPt));
    const Point aEnd(pRenderContext->ImplLogicToDevicePixel(maEndPt));
    tools::Polygon aArcPoly(aRect, aStart, aEnd, PolyStyle::Arc);

    if (aArcPoly.GetSize() >= 2)
    {
        SalPoint* pPtAry = reinterpret_cast<SalPoint*>(aArcPoly.GetPointAry());
        mpGraphics->DrawPolyLine(aArcPoly.GetSize(), pPtAry, pRenderContext);
        return true;
    }

    return false;
}

bool ArcDrawable::CanDraw(OutputDevice* pRenderContext) const
{
    if (!pRenderContext->IsDeviceOutputNecessary() || !pRenderContext->IsLineColor()
        || pRenderContext->ImplIsRecordLayout())
        return false;

    return true;
}

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
