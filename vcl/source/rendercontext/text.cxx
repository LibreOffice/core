/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <tools/fontenum.hxx>

#include <vcl/RenderContext.hxx>
#include <vcl/font.hxx>
#include <vcl/settings.hxx>

#include <drawmode.hxx>

namespace vcl
{
void RenderContext::SetTextAlign(TextAlign eAlign)
{
    if (maFont.GetAlignment() != eAlign)
    {
        maFont.SetAlignment(eAlign);
        mbNewFont = true;
    }

    if (mpAlphaVDev)
        mpAlphaVDev->SetTextAlign(eAlign);
}

void RenderContext::SetLayoutMode(ComplexTextLayoutFlags nTextLayoutMode)
{
    mnTextLayoutMode = nTextLayoutMode;

    if (mpAlphaVDev)
        mpAlphaVDev->SetLayoutMode(nTextLayoutMode);
}

void OutputDevice::SetDigitLanguage(LanguageType eTextLanguage)
{
    meTextLanguage = eTextLanguage;

    if (mpAlphaVDev)
        mpAlphaVDev->SetDigitLanguage(eTextLanguage);
}

void RenderContext2::SetTextColor(Color const& rColor)
{
    Color aColor
        = vcl::drawmode::GetTextColor(rColor, GetDrawMode(), GetSettings().GetStyleSettings());

    if (maTextColor != aColor)
    {
        maTextColor = aColor;
        mbInitTextColor = true;
    }

    if (mpAlphaVDev)
        mpAlphaVDev->SetTextColor(COL_BLACK);
}

void RenderContext2::SetTextLineColor()
{
    maTextLineColor = COL_TRANSPARENT;

    if (mpAlphaVDev)
        mpAlphaVDev->SetTextLineColor();
}

void RenderContext2::SetTextLineColor(Color const& rColor)
{
    maTextLineColor
        = vcl::drawmode::GetTextColor(rColor, GetDrawMode(), GetSettings().GetStyleSettings());

    if (mpAlphaVDev)
        mpAlphaVDev->SetTextLineColor(COL_BLACK);
}

Color RenderContext2::GetTextFillColor() const
{
    if (maFont.IsTransparent())
        return COL_TRANSPARENT;
    else
        return maFont.GetFillColor();
}

void RenderContext2::SetTextFillColor()
{
    if (maFont.GetColor() != COL_TRANSPARENT)
        maFont.SetFillColor(COL_TRANSPARENT);

    if (!maFont.IsTransparent())
        maFont.SetTransparent(true);

    if (mpAlphaVDev)
        mpAlphaVDev->SetTextFillColor();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
