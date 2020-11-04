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

#include <sidebar/DeckTitleBar.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/strings.hrc>

#include <vcl/event.hxx>
#include <vcl/image.hxx>
#include <vcl/ptrstyle.hxx>

#ifdef DEBUG
#include <sfx2/sidebar/Tools.hxx>
#endif

namespace sfx2::sidebar {

DeckTitleBar::DeckTitleBar (const OUString& rsTitle,
                            vcl::Window* pParentWindow,
                            const std::function<void()>& rCloserAction)
    : TitleBar(pParentWindow, "sfx/ui/decktitlebar.ui", "DeckTitleBar",
               Theme::Color_DeckTitleBarBackground)
    , mxGrip(m_xBuilder->weld_image("grip"))
    , mxLabel(m_xBuilder->weld_label("label"))
    , maCloserAction(rCloserAction)
    , mbIsCloserVisible(false)
{
    mxLabel->set_label(rsTitle);

    OSL_ASSERT(pParentWindow != nullptr);

    if (maCloserAction)
        SetCloserVisible(true);

#ifdef DEBUG
    SetText(OUString("DeckTitleBar"));
#endif
}

DeckTitleBar::~DeckTitleBar()
{
    disposeOnce();
}

void DeckTitleBar::dispose()
{
    mxLabel.reset();
    mxGrip.reset();
    TitleBar::dispose();
}

void DeckTitleBar::SetTitle(const OUString& rsTitle)
{
    mxLabel->set_label(rsTitle);
}

OUString DeckTitleBar::GetTitle() const
{
    return mxLabel->get_label();
}

void DeckTitleBar::SetCloserVisible (const bool bIsCloserVisible)
{
    if (mbIsCloserVisible == bIsCloserVisible)
        return;

    mbIsCloserVisible = bIsCloserVisible;

    if (mbIsCloserVisible)
    {
        mxToolBox->set_item_visible("button", true);
        mxToolBox->set_item_icon_name("button", "sfx2/res/closedoc.png");
        mxToolBox->set_item_tooltip_text("button",
                                    SfxResId(SFX_STR_SIDEBAR_CLOSE_DECK));
    }
    else
    {
        mxToolBox->set_item_visible("button", false);
    }
}

#if 0

tools::Rectangle DeckTitleBar::GetDragArea()
{
    Image aGripImage (Theme::GetImage(Theme::Image_Grip));
    return tools::Rectangle(0,0,
               aGripImage.GetSizePixel().Width() + gaLeftGripPadding + gaRightGripPadding,
               aGripImage.GetSizePixel().Height()
    );
}

#endif

void DeckTitleBar::HandleToolBoxItemClick()
{
    if (maCloserAction)
        maCloserAction();
}

void DeckTitleBar::DataChanged (const DataChangedEvent& rEvent)
{
    mxToolBox->set_item_icon_name("button", "sfx2/res/closedoc.png");
    TitleBar::DataChanged(rEvent);
}

#if 0
void DeckTitleBar::MouseMove (const MouseEvent& rMouseEvent)
{
    tools::Rectangle aGrip = GetDragArea();
    PointerStyle eStyle = PointerStyle::Arrow;

    if ( aGrip.IsInside( rMouseEvent.GetPosPixel() ) )
        eStyle = PointerStyle::Move;

    SetPointer( eStyle );

    Window::MouseMove( rMouseEvent );
}
#endif

} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
