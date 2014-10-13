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

#include "TitleBar.hxx"
#include "Paint.hxx"
#include "Accessible.hxx"
#include "AccessibleTitleBar.hxx"

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

TitleBar::TitleBar (
    const ::rtl::OUString& rsTitle,
    vcl::Window* pParentWindow,
    const sidebar::Paint& rInitialBackgroundPaint)
    : Window(pParentWindow),
      maToolBox(this),
      msTitle(rsTitle),
      maIcon()
{
    SetBackground(rInitialBackgroundPaint.GetWallpaper());

    maToolBox.SetSelectHdl(LINK(this, TitleBar, SelectionHandler));
}




TitleBar::~TitleBar (void)
{
}




void TitleBar::SetTitle (const ::rtl::OUString& rsTitle)
{
    msTitle = rsTitle;
    Invalidate();
}




void TitleBar::SetIcon (const Image& rIcon)
{
    maIcon = rIcon;
    Invalidate();
}




void TitleBar::Paint (const Rectangle& rUpdateArea)
{
    (void)rUpdateArea;

    // Paint title bar background.
    Size aWindowSize (GetOutputSizePixel());
    Rectangle aTitleBarBox(
        0,
        0,
        aWindowSize.Width(),
        aWindowSize.Height()
        );

    PaintDecoration(aTitleBarBox);
    const Rectangle aTitleBox (GetTitleArea(aTitleBarBox));
    PaintTitle(aTitleBox);
    PaintFocus(aTitleBox);
}




void TitleBar::DataChanged (const DataChangedEvent& rEvent)
{
    (void)rEvent;

    SetBackground(GetBackgroundPaint().GetWallpaper());
}




void TitleBar::setPosSizePixel (
    long nX,
    long nY,
    long nWidth,
    long nHeight,
    sal_uInt16 nFlags)
{
    Window::setPosSizePixel(nX,nY,nWidth,nHeight,nFlags);

    // Place the toolbox.
    const sal_Int32 nToolBoxWidth (maToolBox.GetItemPosRect(0).GetWidth());
    maToolBox.setPosSizePixel(nWidth-nToolBoxWidth,0, nToolBoxWidth,nHeight, WINDOW_POSSIZE_POSSIZE);
    maToolBox.Show();
}












void TitleBar::HandleToolBoxItemClick (const sal_uInt16 nItemIndex)
{
    (void)nItemIndex;
    // Any real processing has to be done in derived class.
}




css::uno::Reference<css::accessibility::XAccessible> TitleBar::CreateAccessible (void)
{
    SetAccessibleRole(css::accessibility::AccessibleRole::PANEL);
    return AccessibleTitleBar::Create(*this);
}




void TitleBar::PaintTitle (const Rectangle& rTitleBox)
{
    Push(PushFlags::FONT | PushFlags::TEXTCOLOR);

    Rectangle aTitleBox (rTitleBox);

    // When there is an icon then paint it at the left of the given
    // box.
    if ( !! maIcon)
    {
        DrawImage(
            Point(
                aTitleBox.Left() + gnLeftIconSpace,
                aTitleBox.Top() + (aTitleBox.GetHeight()-maIcon.GetSizePixel().Height())/2),
            maIcon);
        aTitleBox.Left() += gnLeftIconSpace + maIcon.GetSizePixel().Width() + gnRightIconSpace;
    }

    vcl::Font aFont(GetFont());
    aFont.SetWeight(WEIGHT_BOLD);
    SetFont(aFont);

    // Paint title bar text.
    SetTextColor(GetTextColor());
    DrawText(
        aTitleBox,
        msTitle,
        TEXT_DRAW_LEFT | TEXT_DRAW_VCENTER);

    Pop();
}




void TitleBar::PaintFocus (const Rectangle& rFocusBox)
{
    Push(PushFlags::FONT | PushFlags::TEXTCOLOR);

    vcl::Font aFont(GetFont());
    aFont.SetWeight(WEIGHT_BOLD);
    SetFont(aFont);

    const Rectangle aTextBox (
        GetTextRect(
            rFocusBox,
            msTitle,
            TEXT_DRAW_LEFT | TEXT_DRAW_VCENTER));
    const Rectangle aLargerTextBox (
        aTextBox.Left() - 2,
        aTextBox.Top() - 2,
        aTextBox.Right() + 2,
        aTextBox.Bottom() + 2);

    if (HasFocus())
        Window::ShowFocus(aLargerTextBox);
    else
        Window::HideFocus();

    Pop();
}




IMPL_LINK(TitleBar, SelectionHandler, ToolBox*, pToolBox)
{
    (void)pToolBox;
    OSL_ASSERT(&maToolBox==pToolBox);
    const sal_uInt16 nItemId (maToolBox.GetHighlightItemId());

    HandleToolBoxItemClick(nItemId);

    return sal_True;
}

} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
