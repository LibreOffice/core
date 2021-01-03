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

#include <vcl/RenderContext2.hxx>

#include <drawmode.hxx>

bool RenderContext2::IsInitFont() const { return mbInitFont; }
void RenderContext2::SetInitFontFlag(bool bFlag) { mbInitFont = bFlag; }
void RenderContext2::SetNewFontFlag(bool bFlag) { mbNewFont = bFlag; }

const vcl::Font& RenderContext2::GetFont() const { return maFont; }

void RenderContext2::SetFont(vcl::Font const& rNewFont)
{
    vcl::Font aFont(rNewFont);
    aFont = GetDrawModeFont(aFont, GetDrawMode(), GetSettings().GetStyleSettings());

    if (GetFont().IsSameInstance(aFont))
        return;

    // Optimization MT/HDU: COL_TRANSPARENT means SetFont should ignore the font color,
    // because SetTextColor() is used for this.
    // #i28759# maTextColor might have been changed behind our back, commit then, too.
    if (aFont.GetColor() != COL_TRANSPARENT
        && (aFont.GetColor() != GetFont().GetColor() || aFont.GetColor() != GetTextColor()))
    {
        maTextColor = aFont.GetColor();
        SetInitTextColorFlag(true);
    }

    maFont = aFont;
    mbNewFont = true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
