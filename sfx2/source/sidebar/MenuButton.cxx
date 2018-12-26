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

#include <sfx2/sidebar/MenuButton.hxx>

#include <sfx2/sidebar/DrawHelper.hxx>
#include <sfx2/sidebar/Paint.hxx>
#include <sfx2/sidebar/Tools.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <vcl/event.hxx>

using namespace css;
using namespace css::uno;

namespace sfx2 { namespace sidebar {

MenuButton::MenuButton (vcl::Window* pParentWindow)
    : CheckBox(pParentWindow),
      mbIsLeftButtonDown(false)
{
#ifdef DEBUG
    SetText(OUString("MenuButton"));
#endif
}

void MenuButton::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& /*rUpdateArea*/)
{
    const bool bIsSelected (IsChecked());
    const bool bIsHighlighted (IsMouseOver() || HasFocus());
    DrawHelper::DrawRoundedRectangle(
                rRenderContext,
                tools::Rectangle(Point(0,0), GetSizePixel()),
                3,
                (bIsHighlighted || bIsSelected
                    ? Theme::GetColor(Theme::Color_TabItemBorder)
                    : Color(0xffffffff)),
                (bIsHighlighted
                    ? Theme::GetPaint(Theme::Paint_TabItemBackgroundHighlight)
                    : Theme::GetPaint(Theme::Paint_TabItemBackgroundNormal)));

    const Image aIcon(Button::GetModeImage());
    const Size aIconSize(aIcon.GetSizePixel());
    const Point aIconLocation((GetSizePixel().Width() - aIconSize.Width()) / 2,
                              (GetSizePixel().Height() - aIconSize.Height()) / 2);
    rRenderContext.DrawImage(aIconLocation, aIcon);
}

void MenuButton::MouseMove (const MouseEvent& rEvent)
{
    if (rEvent.IsEnterWindow() || rEvent.IsLeaveWindow())
        Invalidate();
    CheckBox::MouseMove(rEvent);
}

void MenuButton::MouseButtonDown (const MouseEvent& rMouseEvent)
{
    if (rMouseEvent.IsLeft())
    {
        mbIsLeftButtonDown = true;
        CaptureMouse();
        Invalidate();
    }
}

void MenuButton::MouseButtonUp (const MouseEvent& rMouseEvent)
{
    if (IsMouseCaptured())
        ReleaseMouse();

    if (rMouseEvent.IsLeft())
    {
        if (mbIsLeftButtonDown)
        {
            Check();
            Click();
            GetParent()->Invalidate();
        }
    }
    if (mbIsLeftButtonDown)
    {
        mbIsLeftButtonDown = false;
        Invalidate();
    }
}

} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
