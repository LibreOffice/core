/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <tools/debug.hxx>

#include <vcl/RenderContext2.hxx>

#include <drawmode.hxx>
#include <salgdi.hxx>

bool RenderContext2::IsInitTextColor() const { return mbInitTextColor; }
Color const& RenderContext2::GetTextColor() const { return maTextColor; }
void RenderContext2::SetInitTextColorFlag(bool bFlag) { mbInitTextColor = bFlag; }

void RenderContext2::InitTextColor()
{
    DBG_TESTSOLARMUTEX();

    if (mbInitTextColor)
    {
        mpGraphics->SetTextColor(GetTextColor());
        mbInitTextColor = false;
    }
}

void RenderContext2::SetTextColor(Color const& rColor)
{
    Color aColor(rColor);
    aColor = GetDrawModeTextColor(aColor, GetDrawMode(), GetSettings().GetStyleSettings());

    if (maTextColor != aColor)
    {
        maTextColor = aColor;
        mbInitTextColor = true;
    }
}

Color RenderContext2::GetTextLineColor() const { return maTextLineColor; }

bool RenderContext2::IsOpaqueTextLineColor() const
{
    return (maTextLineColor.GetTransparency() == 0);
}

void RenderContext2::SetTextLineColor(Color const& rColor)
{
    if (rColor.IsTransparent())
        maTextLineColor = COL_TRANSPARENT;
    else
        maTextLineColor
            = GetDrawModeTextColor(Color(rColor), GetDrawMode(), GetSettings().GetStyleSettings());
}

bool RenderContext2::IsOpaqueTextFillColor() const { return !maFont.IsTransparent(); }

Color RenderContext2::GetTextFillColor() const
{
    if (maFont.IsTransparent())
        return COL_TRANSPARENT;
    else
        return maFont.GetFillColor();
}

void RenderContext2::SetTextFillColor(Color const& rColor)
{
    Color aColor(rColor);
    aColor = GetDrawModeFillColor(aColor, GetDrawMode(), GetSettings().GetStyleSettings());

    if (rColor.IsTransparent())
    {
        if (maFont.GetFillColor() != COL_TRANSPARENT)
            maFont.SetFillColor(COL_TRANSPARENT);

        if (!maFont.IsTransparent())
            maFont.SetTransparent(true);
    }
    else
    {
        if (maFont.GetFillColor() != aColor)
            maFont.SetFillColor(aColor);

        if (maFont.IsTransparent() != rColor.IsTransparent())
            maFont.SetTransparent(rColor.IsTransparent());
    }
}

bool RenderContext2::IsOpaqueOverlineColor() const { return !maOverlineColor.IsTransparent(); }

Color RenderContext2::GetOverlineColor() const { return maOverlineColor; }

void RenderContext2::SetOverlineColor(Color const& rColor)
{
    if (rColor.IsTransparent())
        maOverlineColor = COL_TRANSPARENT;
    else
        maOverlineColor
            = GetDrawModeTextColor(rColor, GetDrawMode(), GetSettings().GetStyleSettings());
}

ComplexTextLayoutFlags RenderContext2::GetLayoutMode() const { return mnTextLayoutMode; }

void RenderContext2::SetLayoutMode(ComplexTextLayoutFlags nTextLayoutMode)
{
    mnTextLayoutMode = nTextLayoutMode;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
