/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/drawables/EllipseDrawable.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>

#include <salgdi.hxx>

namespace vcl
{
bool EllipseDrawable::DrawCommand(OutputDevice* pRenderContext) const
{
    tools::Rectangle aRect(pRenderContext->ImplLogicToDevicePixel(maRect));
    if (aRect.IsEmpty())
        return false;

    tools::Polygon aRectPoly(aRect.Center(), aRect.GetWidth() >> 1, aRect.GetHeight() >> 1);
    if (aRectPoly.GetSize() >= 2)
    {
        SalPoint* pPtAry = reinterpret_cast<SalPoint*>(aRectPoly.GetPointAry());
        if (!pRenderContext->IsFillColor())
        {
            mpGraphics->DrawPolyLine(aRectPoly.GetSize(), pPtAry, pRenderContext);
        }
        else
        {
            InitFillColor(pRenderContext);
            mpGraphics->DrawPolygon(aRectPoly.GetSize(), pPtAry, pRenderContext);
        }
    }
    return true;
}

bool EllipseDrawable::CanDraw(OutputDevice* pRenderContext) const
{
    if (!pRenderContext->IsDeviceOutputNecessary()
        || (!pRenderContext->IsLineColor() && !pRenderContext->IsFillColor())
        || pRenderContext->ImplIsRecordLayout())
        return false;

    return true;
}

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
