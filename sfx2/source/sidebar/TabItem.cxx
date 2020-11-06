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

#include <sidebar/TabItem.hxx>

#include <sidebar/DrawHelper.hxx>

#include <sfx2/sidebar/Theme.hxx>
#include <vcl/event.hxx>

using namespace css;
using namespace css::uno;

namespace sfx2::sidebar {

TabItem::TabItem (vcl::Window* pParentWindow)
    : RadioButton(pParentWindow, false, 0)
    , mbIsLeftButtonDown(false)
{
    SetStyle(GetStyle() | WB_TABSTOP | WB_DIALOGCONTROL | WB_NOPOINTERFOCUS);
    SetBackground(Theme::GetColor(Theme::Color_TabBarBackground));
#ifdef DEBUG
    SetText(OUString("TabItem"));
#endif
}

void TabItem::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& /*UpdateArea*/)
{
    const bool bIsSelected (IsChecked());
    const bool bIsHighlighted (IsMouseOver() || HasFocus());
    DrawHelper::DrawRoundedRectangle(
                rRenderContext,
                tools::Rectangle(Point(0,0), GetSizePixel()),
                Theme::GetInteger(Theme::Int_ButtonCornerRadius),
                bIsHighlighted||bIsSelected
                    ? Theme::GetColor(Theme::Color_TabItemBorder)
                    : COL_TRANSPARENT,
                bIsHighlighted
                    ? Theme::GetColor(Theme::Color_TabItemBackgroundHighlight)
                    : Theme::GetColor(Theme::Color_TabItemBackgroundNormal));

    const Image aIcon(Button::GetModeImage());
    const Size aIconSize (aIcon.GetSizePixel());
    const Point aIconLocation((GetSizePixel().Width() - aIconSize.Width()) / 2,
                              (GetSizePixel().Height() - aIconSize.Height()) / 2);
    rRenderContext.DrawImage(aIconLocation, aIcon, IsEnabled() ? DrawImageFlags::NONE : DrawImageFlags::Disable);
}

void TabItem::MouseMove(const MouseEvent& rEvent)
{
    if (rEvent.IsEnterWindow() || rEvent.IsLeaveWindow())
        Invalidate();
    RadioButton::MouseMove(rEvent);
}

void TabItem::MouseButtonDown(const MouseEvent& rMouseEvent)
{
    if (rMouseEvent.IsLeft())
    {
        mbIsLeftButtonDown = true;
        CaptureMouse();
        Invalidate();
    }
}

void TabItem::MouseButtonUp(const MouseEvent& rMouseEvent)
{
    if (IsMouseCaptured())
        ReleaseMouse();

    if (rMouseEvent.IsLeft())
    {
        if (mbIsLeftButtonDown)
        {
            Check();
            Click();
            vcl::Window* pParent = GetParent();
            if (pParent)
                pParent->Invalidate();
        }
    }

    if (mbIsLeftButtonDown)
    {
        mbIsLeftButtonDown = false;
        Invalidate();
    }
}

} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
