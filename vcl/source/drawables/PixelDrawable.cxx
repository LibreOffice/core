/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/drawables/PixelDrawable.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>

#include <salgdi.hxx>

namespace vcl
{
bool PixelDrawable::DrawCommand(OutputDevice* pRenderContext) const
{
    Color aColor;
    if (mbUsesColor)
        aColor = pRenderContext->ImplDrawModeToColor(maColor);

    Point aPt = pRenderContext->ImplLogicToDevicePixel(maPt);

    if (mbUsesColor)
        mpGraphics->DrawPixel(aPt.X(), aPt.Y(), aColor, pRenderContext);
    else
        mpGraphics->DrawPixel(aPt.X(), aPt.Y(), pRenderContext);

    return true;
}

bool PixelDrawable::CanDraw(OutputDevice* pRenderContext) const
{
    if (!pRenderContext->IsDeviceOutputNecessary() || !pRenderContext->IsLineColor()
        || pRenderContext->ImplIsRecordLayout())
        return false;

    return true;
}

bool PixelDrawable::DrawAlphaVirtDev(OutputDevice* pRenderContext) const
{
    Color aColor;
    if (mbUsesColor)
        aColor = pRenderContext->ImplDrawModeToColor(maColor);

    VirtualDevice* pAlphaVDev = pRenderContext->GetAlphaVirtDev();
    if (pAlphaVDev)
    {
        if (mbUsesColor)
        {
            Color aAlphaColor(aColor.GetTransparency(), aColor.GetTransparency(),
                              aColor.GetTransparency());
            pAlphaVDev->Draw(PixelDrawable(maPt, aAlphaColor));
        }
        else
        {
            pAlphaVDev->Draw(PixelDrawable(maPt));
        }
    }

    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
