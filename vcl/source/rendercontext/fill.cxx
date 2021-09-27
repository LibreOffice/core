/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/debug.hxx>

#include <vcl/RenderContext.hxx>
#include <vcl/rendercontext/RasterOp.hxx>

#include <drawmode>
#include <salgdi.hxx>

namespace vcl
{
void RenderContext::SetFillColor()
{
    if (mbFillColor)
    {
        mbInitFillColor = true;
        mbFillColor = false;
        maFillColor = COL_TRANSPARENT;
    }

    if (mpAlphaVDev)
        mpAlphaVDev->SetFillColor();
}

void RenderContext::SetFillColor(Color const& rColor)
{
    Color aColor = GetDrawModeFillColor(rColor, GetDrawMode(), GetSettings().GetStyleSettings());

    if (aColor.IsTransparent())
    {
        if (mbFillColor)
        {
            mbInitFillColor = true;
            mbFillColor = false;
            maFillColor = COL_TRANSPARENT;
        }
    }
    else
    {
        if (maFillColor != aColor)
        {
            mbInitFillColor = true;
            mbFillColor = true;
            maFillColor = aColor;
        }
    }

    if (mpAlphaVDev)
        mpAlphaVDev->SetFillColor(COL_BLACK);
}

void RenderContext::InitFillColor()
{
    DBG_TESTSOLARMUTEX();

    if (mbFillColor)
    {
        if (RasterOp::N0 == meRasterOp)
            mpGraphics->SetROPFillColor(SalROPColor::N0);
        else if (RasterOp::N1 == meRasterOp)
            mpGraphics->SetROPFillColor(SalROPColor::N1);
        else if (RasterOp::Invert == meRasterOp)
            mpGraphics->SetROPFillColor(SalROPColor::Invert);
        else
            mpGraphics->SetFillColor(maFillColor);
    }
    else
    {
        mpGraphics->SetFillColor();
    }

    mbInitFillColor = false;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
