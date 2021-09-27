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

#include <drawmode.hxx>

namespace vcl
{
void RenderContext2::SetFont(vcl::Font const& rNewFont)
{
    vcl::Font aFont
        = vcl::drawmode::GetFont(rNewFont, GetDrawMode(), GetSettings().GetStyleSettings());

    // Optimization MT/HDU: COL_TRANSPARENT means SetFont should ignore the font color,
    // because SetTextColor() is used for this.
    // #i28759# maTextColor might have been changed behind our back, commit then, too.
    if (aFont.GetColor() != COL_TRANSPARENT
        && (aFont.GetColor() != maFont.GetColor() || aFont.GetColor() != maTextColor))
    {
        maTextColor = aFont.GetColor();
        mbInitTextColor = true;
    }

    if (maFont.IsSameInstance(aFont))
        return;

    maFont = aFont;
    mbNewFont = true;

    if (!mpAlphaVDev)
        return;

    // #i30463#
    // Since SetFont might change the text color, apply that only
    // selectively to alpha vdev (which normally paints opaque text
    // with COL_BLACK)
    if (aFont.GetColor() != COL_TRANSPARENT)
    {
        mpAlphaVDev->SetTextColor(COL_BLACK);
        aFont.SetColor(COL_TRANSPARENT);
    }

    mpAlphaVDev->SetFont(aFont);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
