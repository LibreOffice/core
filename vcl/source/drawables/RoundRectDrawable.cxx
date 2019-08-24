/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/drawables/RoundRectDrawable.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>

#include <salgdi.hxx>

#include <cassert>

namespace vcl
{
bool RoundRectDrawable::DrawCommand(OutputDevice* pRenderContext) const
{
    tools::Rectangle aRect(pRenderContext->ImplLogicToDevicePixel(maRect));

    if (aRect.IsEmpty())
        return false;

    aRect.Justify();

    sal_uLong nHorzRadius = pRenderContext->ImplLogicWidthToDevicePixel(mnHorzRadius);
    sal_uLong nVertRadius = pRenderContext->ImplLogicWidthToDevicePixel(mnVertRadius);

    if (!nHorzRadius && !nVertRadius)
    {
        mpGraphics->DrawRect(aRect.Left(), aRect.Top(), aRect.GetWidth(), aRect.GetHeight(),
                             pRenderContext);
    }
    else
    {
        tools::Polygon aRoundRectPoly(aRect, nHorzRadius, nVertRadius);

        if (aRoundRectPoly.GetSize() >= 2)
        {
            SalPoint* pPtAry = reinterpret_cast<SalPoint*>(aRoundRectPoly.GetPointAry());

            if (!pRenderContext->IsFillColor())
                mpGraphics->DrawPolyLine(aRoundRectPoly.GetSize(), pPtAry, pRenderContext);
            else
                mpGraphics->DrawPolygon(aRoundRectPoly.GetSize(), pPtAry, pRenderContext);
        }
    }

    return true;
}

bool RoundRectDrawable::CanDraw(OutputDevice* pRenderContext) const
{
    if (!pRenderContext->IsDeviceOutputNecessary()
        || (!pRenderContext->IsLineColor() && !pRenderContext->IsFillColor())
        || pRenderContext->ImplIsRecordLayout())
        return false;

    return true;
}

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
