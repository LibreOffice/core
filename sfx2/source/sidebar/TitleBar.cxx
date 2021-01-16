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

#include <sfx2/sidebar/TitleBar.hxx>
#include <sfx2/sidebar/Paint.hxx>
#include <sfx2/sidebar/Accessible.hxx>
#include <sfx2/sidebar/AccessibleTitleBar.hxx>

#include <tools/svborder.hxx>
#include <vcl/gradient.hxx>
#include <vcl/lineinfo.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>

namespace
{
    const static sal_Int32 gnLeftIconSpace (3);
    const static sal_Int32 gnRightIconSpace (3);
}

namespace sfx2 { namespace sidebar {

TitleBar::TitleBar(const OUString& rsTitle,
                   vcl::Window* pParentWindow,
                   const sidebar::Paint& rInitialBackgroundPaint)
    : Window(pParentWindow)
    , maToolBox(VclPtr<SidebarToolBox>::Create(this))
    , msTitle(rsTitle)
    , maIcon()
    , maBackgroundPaint(rInitialBackgroundPaint)
{
    maToolBox->SetSelectHdl(LINK(this, TitleBar, SelectionHandler));
}

TitleBar::~TitleBar()
{
    disposeOnce();
}

void TitleBar::dispose()
{
    maToolBox.disposeAndClear();
    vcl::Window::dispose();
}

void TitleBar::SetTitle(const OUString& rsTitle)
{
    if (msTitle != rsTitle)
    {
        msTitle = rsTitle;
        Invalidate();
    }
}

void TitleBar::SetIcon(const Image& rIcon)
{
    if (maIcon != rIcon)
    {
        maIcon = rIcon;
        Invalidate();
    }
}

void TitleBar::ApplySettings(vcl::RenderContext& rRenderContext)
{
    rRenderContext.SetBackground(maBackgroundPaint.GetWallpaper());
}

void TitleBar::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& /*rUpdateArea*/)
{
    // Paint title bar background.
    Size aWindowSize (GetSizePixel());
    tools::Rectangle aTitleBarBox(0,0, aWindowSize.Width(), aWindowSize.Height());

    PaintDecoration(rRenderContext);
    const tools::Rectangle aTitleBox(GetTitleArea(aTitleBarBox));
    PaintTitle(rRenderContext, aTitleBox);
    PaintFocus(rRenderContext, aTitleBox);
}

void TitleBar::DataChanged (const DataChangedEvent& /*rEvent*/)
{
    maBackgroundPaint = GetBackgroundPaint();
    Invalidate();
}

void TitleBar::setPosSizePixel (long nX, long nY, long nWidth, long nHeight, PosSizeFlags nFlags)
{
    Window::setPosSizePixel(nX, nY, nWidth, nHeight, nFlags);

    // Place the toolbox.
    const sal_Int32 nToolBoxWidth (maToolBox->GetItemPosRect(0).GetWidth());
    maToolBox->setPosSizePixel(nWidth - nToolBoxWidth,0, nToolBoxWidth, nHeight);
    maToolBox->Show();
}

void TitleBar::HandleToolBoxItemClick(const sal_uInt16 /*nItemIndex*/)
{
    // Any real processing has to be done in derived class.
}

css::uno::Reference<css::accessibility::XAccessible> TitleBar::CreateAccessible()
{
    SetAccessibleRole(css::accessibility::AccessibleRole::PANEL);
    return AccessibleTitleBar::Create(*this);
}

void TitleBar::PaintTitle(vcl::RenderContext& rRenderContext, const tools::Rectangle& rTitleBox)
{
    rRenderContext.Push(PushFlags::FONT | PushFlags::TEXTCOLOR);

    tools::Rectangle aTitleBox(rTitleBox);

    // When there is an icon then paint it at the left of the given
    // box.
    if (!!maIcon)
    {
        rRenderContext.DrawImage(Point(aTitleBox.Left() + gnLeftIconSpace,
                                       aTitleBox.Top() + (aTitleBox.GetHeight() - maIcon.GetSizePixel().Height()) / 2),
                                 maIcon);
        aTitleBox.AdjustLeft(gnLeftIconSpace + maIcon.GetSizePixel().Width() + gnRightIconSpace );
    }

    vcl::Font aFont(rRenderContext.GetFont());
    aFont.SetWeight(WEIGHT_BOLD);
    rRenderContext.SetFont(aFont);

    // Paint title bar text.
    rRenderContext.SetTextColor(rRenderContext.GetTextColor());
    rRenderContext.DrawText(aTitleBox, msTitle, DrawTextFlags::Left | DrawTextFlags::VCenter);
    rRenderContext.Pop();
}

void TitleBar::PaintFocus(vcl::RenderContext& rRenderContext, const tools::Rectangle& rFocusBox)
{
    rRenderContext.Push(PushFlags::FONT | PushFlags::TEXTCOLOR);

    vcl::Font aFont(rRenderContext.GetFont());
    aFont.SetWeight(WEIGHT_BOLD);
    rRenderContext.SetFont(aFont);

    const tools::Rectangle aTextBox(rRenderContext.GetTextRect(rFocusBox, msTitle, DrawTextFlags::Left | DrawTextFlags::VCenter));

    const tools::Rectangle aLargerTextBox(aTextBox.Left() - 2,
                                   aTextBox.Top() - 2,
                                   aTextBox.Right() + 2,
                                   aTextBox.Bottom() + 2);

    if (HasFocus())
        Window::ShowFocus(aLargerTextBox);
    else
        Window::HideFocus();

    rRenderContext.Pop();
}

IMPL_LINK(TitleBar, SelectionHandler, ToolBox*, pToolBox, void)
{
    OSL_ASSERT(maToolBox.get()==pToolBox);
    const sal_uInt16 nItemId (maToolBox->GetHighlightItemId());

    HandleToolBoxItemClick(nItemId);
}

} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
