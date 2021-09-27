/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <vcl/RenderContext.hxx>

void RenderContext2::SetLineColor()
{
    if (mbLineColor)
    {
        mbInitLineColor = true;
        mbLineColor = false;
        maLineColor = COL_TRANSPARENT;
    }

    if (mpAlphaVDev)
        mpAlphaVDev->SetLineColor();
}

void RenderContext2::SetLineColor(const Color& rColor)
{
    Color aColor;

    if (rColor.IsTransparent())
        aColor = rColor;
    else
        aColor
            = vcl::drawmode::GetLineColor(rColor, GetDrawMode(), GetSettings().GetStyleSettings());

    if (aColor.IsTransparent())
    {
        if (mbLineColor)
        {
            mbInitLineColor = true;
            mbLineColor = false;
            maLineColor = COL_TRANSPARENT;
        }
    }
    else
    {
        if (maLineColor != aColor)
        {
            mbInitLineColor = true;
            mbLineColor = true;
            maLineColor = aColor;
        }
    }

    if (mpAlphaVDev)
        mpAlphaVDev->SetLineColor(COL_BLACK);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
