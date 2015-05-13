/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "svx/sidebar/PopupControl.hxx"
#include <sfx2/sidebar/Theme.hxx>
#include <vcl/gradient.hxx>

using ::sfx2::sidebar::Theme;

namespace svx { namespace sidebar {

PopupControl::PopupControl (
    vcl::Window* pParent,
    const ResId& rResId)
    : Control( pParent,rResId)
{
    EnableChildTransparentMode(true);
    SetControlBackground();

    SetBackground(Theme::GetWallpaper(Theme::Paint_DropDownBackground));
}

void PopupControl::Paint (vcl::RenderContext& rRenderContext, const Rectangle& rRect)
{
    Control::Paint(rRenderContext, rRect);

    // The background is taken care of by setting the background color
    // in the constructor.  Here we just paint the border.
    rRenderContext.SetFillColor();
    rRenderContext.SetLineColor(Theme::GetColor(Theme::Color_DropDownBorder));
    rRenderContext.DrawRect(Rectangle(Point(0,0), GetOutputSizePixel()));
}

} } // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
