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

#include <sidebar/PanelTitleBar.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/strings.hrc>
#include <sidebar/Paint.hxx>
#include <sfx2/sidebar/Panel.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <sidebar/ControllerFactory.hxx>
#include <sidebar/Tools.hxx>
#include <vcl/event.hxx>
#include <vcl/image.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/diagnose_ex.h>

#include <com/sun/star/frame/XDispatch.hpp>

using namespace css;
using namespace css::uno;

namespace sfx2::sidebar {

const sal_Int32 gaLeftIconPadding (5);
const sal_Int32 gaRightIconPadding (5);

PanelTitleBar::PanelTitleBar(const OUString& rsTitle,
                             vcl::Window* pParentWindow,
                             Panel* pPanel)
    : TitleBar(rsTitle, pParentWindow, GetBackgroundPaint()),
      mbIsLeftButtonDown(false),
      mpPanel(pPanel),
      mxFrame(),
      msMoreOptionsCommand()
{
    assert(mpPanel);

    UpdateExpandedState();

    mxImage->set_margin_left(gaLeftIconPadding);
    mxImage->set_margin_right(gaRightIconPadding);

#ifdef DEBUG
    SetText(OUString("PanelTitleBar"));
#endif
}

void PanelTitleBar::UpdateExpandedState()
{
    if (mpPanel->IsExpanded())
        mxDecoration->set_from_icon_name("res/minus.png");
    else
        mxDecoration->set_from_icon_name("res/plus.png");
}

PanelTitleBar::~PanelTitleBar()
{
    disposeOnce();
}

void PanelTitleBar::dispose()
{
    mpPanel.clear();
    TitleBar::dispose();
}

void PanelTitleBar::SetMoreOptionsCommand(const OUString& rsCommandName,
                                          const css::uno::Reference<css::frame::XFrame>& rxFrame,
                                          const css::uno::Reference<css::frame::XController>& rxController)
{
    if (rsCommandName == msMoreOptionsCommand)
        return;

    if (msMoreOptionsCommand.getLength() > 0)
        mxToolBox->set_item_visible("button", false);

    msMoreOptionsCommand = rsCommandName;
    mxFrame = rxFrame;

    if (msMoreOptionsCommand.getLength() <= 0)
        return;

    mxToolBox->set_item_visible("button", true);
    mxToolBox->set_item_icon_name("button", "sfx2/res/symphony/morebutton.png");
    mxToolbarDispatch.reset(new ToolbarUnoDispatcher(*mxToolBox, *m_xBuilder, rxFrame));
#if 0
    Reference<frame::XToolbarController> xController (
        ControllerFactory::CreateToolBoxController(
            *mxToolBox, *m_xBuilder,
            msMoreOptionsCommand, rxFrame, true));
#endif
    mxToolBox->set_item_tooltip_text(
        "button",
        SfxResId(SFX_STR_SIDEBAR_MORE_OPTIONS));
}

#if 0
tools::Rectangle PanelTitleBar::GetTitleArea (const tools::Rectangle& rTitleBarBox)
{
    if (mpPanel != nullptr)
    {
        Image aImage (mpPanel->IsExpanded()
            ? Theme::GetImage(Theme::Image_Expand)
            : Theme::GetImage(Theme::Image_Collapse));
        return tools::Rectangle(
            aImage.GetSizePixel().Width() + gaLeftIconPadding + gaRightIconPadding,
            rTitleBarBox.Top(),
            rTitleBarBox.Right(),
            rTitleBarBox.Bottom());
    }
    else
        return rTitleBarBox;
}

void PanelTitleBar::PaintDecoration (vcl::RenderContext& rRenderContext)
{
    if (mpPanel != nullptr)
    {
        Image aImage (mpPanel->IsExpanded()
            ? Theme::GetImage(Theme::Image_Collapse)
            : Theme::GetImage(Theme::Image_Expand));
        const Point aTopLeft(gaLeftIconPadding,
                             (GetSizePixel().Height() - aImage.GetSizePixel().Height()) / 2);
        rRenderContext.DrawImage(aTopLeft, aImage);
    }
}
#endif

Paint PanelTitleBar::GetBackgroundPaint()
{
    return Theme::GetPaint(Theme::Paint_PanelTitleBarBackground);
}

void PanelTitleBar::HandleToolBoxItemClick()
{
    if (msMoreOptionsCommand.getLength() <= 0)
        return;

    try
    {
        const util::URL aURL (Tools::GetURL(msMoreOptionsCommand));
        Reference<frame::XDispatch> xDispatch (Tools::GetDispatch(mxFrame, aURL));
        if (xDispatch.is())
            xDispatch->dispatch(aURL, Sequence<beans::PropertyValue>());
    }
    catch(Exception const &)
    {
        DBG_UNHANDLED_EXCEPTION("sfx");
    }
}

Reference<accessibility::XAccessible> PanelTitleBar::CreateAccessible()
{
#if 0
    SetAccessibleName(msTitle);
    SetAccessibleDescription(msTitle);
#endif
    return TitleBar::CreateAccessible();
}

void PanelTitleBar::MouseButtonDown (const MouseEvent& rMouseEvent)
{
    if (rMouseEvent.IsLeft())
    {
        mbIsLeftButtonDown = true;
        CaptureMouse();
    }
}

void PanelTitleBar::MouseButtonUp (const MouseEvent& rMouseEvent)
{
    if (IsMouseCaptured())
        ReleaseMouse();

    if (rMouseEvent.IsLeft())
    {
        if (mbIsLeftButtonDown)
        {
            if (mpPanel != nullptr)
            {
                mpPanel->SetExpanded( ! mpPanel->IsExpanded());
                Invalidate();
                GrabFocus();
            }
        }
    }
    if (mbIsLeftButtonDown)
        mbIsLeftButtonDown = false;
}

void PanelTitleBar::DataChanged (const DataChangedEvent& rEvent)
{
    mxToolBox->set_item_icon_name("button", "sfx2/res/symphony/morebutton.png");
    TitleBar::DataChanged(rEvent);
}

} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
