/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/debug.hxx>

#include <vcl/RenderContext2.hxx>

#include <drawmode.hxx>
#include <salgdi.hxx>

bool RenderContext2::IsInitLineColor() const { return mbInitLineColor; }
bool RenderContext2::IsOpaqueLineColor() const { return mbOpaqueLineColor; }
Color const& RenderContext2::GetLineColor() const { return maLineColor; }
void RenderContext2::SetInitLineColorFlag(bool bFlag) { mbInitLineColor = bFlag; }
void RenderContext2::FlagLineColorAsTransparent() { mbOpaqueLineColor = false; }
void RenderContext2::FlagLineColorAsOpaque() { mbOpaqueLineColor = true; }

void RenderContext2::InitLineColor()
{
    DBG_TESTSOLARMUTEX();

    if (mbOpaqueLineColor)
    {
        if (meRasterOp == RasterOp::N0)
            mpGraphics->SetROPLineColor(SalROPColor::N0);
        else if (meRasterOp == RasterOp::N1)
            mpGraphics->SetROPLineColor(SalROPColor::N1);
        else if (meRasterOp == RasterOp::Invert)
            mpGraphics->SetROPLineColor(SalROPColor::Invert);
        else
            mpGraphics->SetLineColor(maLineColor);
    }
    else
    {
        mpGraphics->SetLineColor();
    }

    mbInitLineColor = false;
}

void RenderContext2::SetLineColor(Color const& rColor)
{
    if (rColor.IsTransparent())
    {
        if (mbOpaqueLineColor)
        {
            mbInitLineColor = true;
            mbOpaqueLineColor = false;
            maLineColor = COL_TRANSPARENT;
        }
    }
    else
    {
        Color aColor
            = GetDrawModeLineColor(rColor, GetDrawMode(), GetSettings().GetStyleSettings());

        if (mbOpaqueLineColor)
        {
            mbInitLineColor = true;
            mbOpaqueLineColor = false;
            maLineColor = COL_TRANSPARENT;
        }
        else
        {
            if (maLineColor != aColor)
            {
                mbInitLineColor = true;
                mbOpaqueLineColor = true;
                maLineColor = aColor;
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
