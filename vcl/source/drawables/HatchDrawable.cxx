/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/settings.hxx>
#include <vcl/drawables/HatchDrawable.hxx>

#include <drawables/HatchDrawableHelper.hxx>

namespace vcl
{
bool HatchDrawable::DrawCommand(OutputDevice* pRenderContext) const
{
    if (mbScaffolding)
        return Draw(pRenderContext, maPolyPolygon, maHatch);
    else
        return HatchDrawableHelper::DrawDecomposedHatchLines(pRenderContext, maPolyPolygon, maHatch,
                                                             mbMtf);
}

bool HatchDrawable::Draw(OutputDevice* pRenderContext, tools::PolyPolygon const& rPolyPolygon,
                         Hatch const& rHatch) const
{
    Hatch aHatch(rHatch);

    if (pRenderContext->GetDrawMode()
        & (DrawModeFlags::BlackLine | DrawModeFlags::WhiteLine | DrawModeFlags::GrayLine
           | DrawModeFlags::SettingsLine))
    {
        Color aColor(rHatch.GetColor());

        if (pRenderContext->GetDrawMode() & DrawModeFlags::BlackLine)
        {
            aColor = COL_BLACK;
        }
        else if (pRenderContext->GetDrawMode() & DrawModeFlags::WhiteLine)
        {
            aColor = COL_WHITE;
        }
        else if (pRenderContext->GetDrawMode() & DrawModeFlags::GrayLine)
        {
            const sal_uInt8 cLum = aColor.GetLuminance();
            aColor = Color(cLum, cLum, cLum);
        }
        else if (pRenderContext->GetDrawMode() & DrawModeFlags::SettingsLine)
        {
            aColor = pRenderContext->GetSettings().GetStyleSettings().GetFontColor();
        }

        aHatch.SetColor(aColor);
    }

    mpMetaAction = new MetaHatchAction(rPolyPolygon, aHatch);
    AddAction(pRenderContext);

    if (!CanDraw(pRenderContext))
        return false;

    if (!InitClipRegion(pRenderContext))
        return false;

    if (rPolyPolygon.Count())
    {
        tools::PolyPolygon aPolyPoly(pRenderContext->LogicToPixel(rPolyPolygon));
        aPolyPoly.Optimize(PolyOptimizeFlags::NO_SAME);
        aHatch.SetDistance(pRenderContext->ImplLogicWidthToDevicePixel(aHatch.GetDistance()));

        vcl::RenderContextLineColorGuard aGuard(pRenderContext, aHatch.GetColor());
        pRenderContext->DrawHatch(aPolyPoly, aHatch, false);
    }

    DrawAlphaVirtDev(pRenderContext);

    return true;
}

bool HatchDrawable::CanDraw(OutputDevice* pRenderContext) const
{
    if (!pRenderContext->IsDeviceOutputNecessary() || pRenderContext->ImplIsRecordLayout())
        return false;

    return true;
}

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
