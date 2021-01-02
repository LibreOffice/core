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

#include <vcl/settings.hxx>
#include <vcl/DrawModeFlags.hxx>
#include <vcl/RenderContext2.hxx>

#include <drawmode.hxx>

Color GetDrawModeLineColor(Color const& rColor, DrawModeFlags nDrawMode,
                           StyleSettings const& rStyleSettings)
{
    Color aColor(rColor);

    if (nDrawMode
        & (DrawModeFlags::BlackLine | DrawModeFlags::WhiteLine | DrawModeFlags::GrayLine
           | DrawModeFlags::SettingsLine))
    {
        if (!aColor.IsTransparent())
        {
            if (nDrawMode & DrawModeFlags::BlackLine)
            {
                aColor = COL_BLACK;
            }
            else if (nDrawMode & DrawModeFlags::WhiteLine)
            {
                aColor = COL_WHITE;
            }
            else if (nDrawMode & DrawModeFlags::GrayLine)
            {
                const sal_uInt8 cLum = aColor.GetLuminance();
                aColor = Color(cLum, cLum, cLum);
            }
            else if (nDrawMode & DrawModeFlags::SettingsLine)
            {
                aColor = rStyleSettings.GetFontColor();
            }
        }
    }

    return aColor;
}

Color GetDrawModeFillColor(Color const& rColor, DrawModeFlags nDrawMode,
                           StyleSettings const& rStyleSettings)
{
    Color aColor(rColor);

    if (nDrawMode
        & (DrawModeFlags::BlackFill | DrawModeFlags::WhiteFill | DrawModeFlags::GrayFill
           | DrawModeFlags::NoFill | DrawModeFlags::SettingsFill))
    {
        if (!aColor.IsTransparent())
        {
            if (nDrawMode & DrawModeFlags::BlackFill)
            {
                aColor = COL_BLACK;
            }
            else if (nDrawMode & DrawModeFlags::WhiteFill)
            {
                aColor = COL_WHITE;
            }
            else if (nDrawMode & DrawModeFlags::GrayFill)
            {
                const sal_uInt8 cLum = aColor.GetLuminance();
                aColor = Color(cLum, cLum, cLum);
            }
            else if (nDrawMode & DrawModeFlags::NoFill)
            {
                aColor = COL_TRANSPARENT;
            }
            else if (nDrawMode & DrawModeFlags::SettingsFill)
            {
                aColor = rStyleSettings.GetWindowColor();
            }
        }
    }

    return aColor;
}

Color GetDrawModeTextColor(Color const& rColor, DrawModeFlags nDrawMode,
                           StyleSettings const& rStyleSettings)
{
    Color aColor(rColor);

    if (nDrawMode
        & (DrawModeFlags::BlackText | DrawModeFlags::WhiteText | DrawModeFlags::GrayText
           | DrawModeFlags::SettingsText))
    {
        if (!aColor.IsTransparent())
        {
            if (nDrawMode & DrawModeFlags::BlackText)
            {
                aColor = COL_BLACK;
            }
            else if (nDrawMode & DrawModeFlags::WhiteText)
            {
                aColor = COL_WHITE;
            }
            else if (nDrawMode & DrawModeFlags::GrayText)
            {
                const sal_uInt8 cLum = aColor.GetLuminance();
                aColor = Color(cLum, cLum, cLum);
            }
            else if (nDrawMode & DrawModeFlags::SettingsText)
            {
                aColor = rStyleSettings.GetFontColor();
            }
        }
    }

    return aColor;
}

DrawModeFlags RenderContext2::GetDrawMode() const { return mnDrawMode; }
void RenderContext2::SetDrawMode(DrawModeFlags nDrawMode) { mnDrawMode = nDrawMode; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
