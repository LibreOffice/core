/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/rendersettings.hxx>

namespace vcl
{

void RenderSettings::SetLineColor(const Color& rColor)
{
    maOutDevState.mpLineColor = new Color(rColor);
    maOutDevState.mnFlags |= PushFlags::LINECOLOR;
}

void RenderSettings::SetFillColor(const Color& rColor)
{
    maOutDevState.mpFillColor = new Color(rColor);
    maOutDevState.mnFlags |= PushFlags::FILLCOLOR;
}

void RenderSettings::SetFont(const vcl::Font& rNewFont)
{
    maOutDevState.mpFont = new vcl::Font(rNewFont);
    maOutDevState.mnFlags |= PushFlags::FONT;
}

void RenderSettings::SetBackground(const Wallpaper& rBackground)
{
    mpBackground.reset(new Wallpaper(rBackground));
}

void RenderSettings::PushAndApply(vcl::RenderContext& rRenderContext)
{
    rRenderContext.Push(maOutDevState.mnFlags);
    Apply(rRenderContext);
}

void RenderSettings::Apply(vcl::RenderContext& rRenderContext)
{
    if (maOutDevState.mnFlags & PushFlags::LINECOLOR)
        rRenderContext.SetLineColor(*maOutDevState.mpLineColor);
    if (maOutDevState.mnFlags & PushFlags::FILLCOLOR)
        rRenderContext.SetFillColor(*maOutDevState.mpFillColor);
    if (maOutDevState.mnFlags & PushFlags::FONT)
        rRenderContext.SetFont(*maOutDevState.mpFont);
    if (mpBackground)
        rRenderContext.SetBackground(Wallpaper(*mpBackground.get()));
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
