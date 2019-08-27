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

#include <vcl/drawables/ChordDrawable.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>

#include <salgdi.hxx>

namespace vcl
{
bool ChordDrawable::DrawCommand(OutputDevice* pRenderContext) const
{
    tools::Rectangle aRect(pRenderContext->ImplLogicToDevicePixel(maRect));
    if (aRect.IsEmpty())
        return false;

    const Point aStart(pRenderContext->ImplLogicToDevicePixel(maStartPt));
    const Point aEnd(pRenderContext->ImplLogicToDevicePixel(maEndPt));
    tools::Polygon aChordPoly(aRect, aStart, aEnd, PolyStyle::Chord);

    if (aChordPoly.GetSize() >= 2)
    {
        SalPoint* pPtAry = reinterpret_cast<SalPoint*>(aChordPoly.GetPointAry());
        if (!pRenderContext->IsFillColor())
        {
            mpGraphics->DrawPolyLine(aChordPoly.GetSize(), pPtAry, pRenderContext);
        }
        else
        {
            InitFillColor(pRenderContext);
            mpGraphics->DrawPolygon(aChordPoly.GetSize(), pPtAry, pRenderContext);
        }
    }

    return true;
}

bool ChordDrawable::CanDraw(OutputDevice* pRenderContext) const
{
    if (!pRenderContext->IsDeviceOutputNecessary()
        || (!pRenderContext->IsLineColor() && !pRenderContext->IsFillColor())
        || pRenderContext->ImplIsRecordLayout())
        return false;

    return true;
}

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
