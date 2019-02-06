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

#include <sfx2/sidebar/PanelTitleBar.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/strings.hrc>
#include <sfx2/sidebar/Paint.hxx>
#include <sfx2/sidebar/Panel.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/sidebar/ControllerFactory.hxx>
#include <sfx2/sidebar/Tools.hxx>
#include <tools/svborder.hxx>
#include <vcl/event.hxx>
#include <vcl/gradient.hxx>
#include <vcl/image.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/diagnose_ex.h>

using namespace css;
using namespace css::uno;

namespace sfx2 { namespace sidebar {

static const sal_Int32 gaLeftIconPadding (5);
static const sal_Int32 gaRightIconPadding (5);

PanelTitleBar::PanelTitleBar(const OUString& rsTitle,
                             vcl::Window* pParentWindow,
                             Panel* pPanel)
    : TitleBar(rsTitle, pParentWindow, GetBackgroundPaint()),
      mbIsLeftButtonDown(false),
      mpPanel(pPanel),
      mxFrame(),
      msMoreOptionsCommand()
{
    OSL_ASSERT(mpPanel != nullptr);

#ifdef DEBUG
    SetText(OUString("PanelTitleBar"));
#endif
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
        maToolBox->RemoveItem(maToolBox->GetItemPos(mnMenuItemIndex));

    msMoreOptionsCommand = rsCommandName;
    mxFrame = rxFrame;

    if (msMoreOptionsCommand.getLength() <= 0)
        return;

    maToolBox->InsertItem(
        mnMenuItemIndex,
        Theme::GetImage(Theme::Image_PanelMenu));
    Reference<frame::XToolbarController> xController (
        ControllerFactory::CreateToolBoxController(
            maToolBox.get(),
            mnMenuItemIndex,
            msMoreOptionsCommand,
            rxFrame, rxController,
            VCLUnoHelper::GetInterface(maToolBox.get()),
            0));
    maToolBox->SetController(mnMenuItemIndex, xController);
    maToolBox->SetOutStyle(TOOLBOX_STYLE_FLAT);
    maToolBox->SetQuickHelpText(
        mnMenuItemIndex,
        SfxResId(SFX_STR_SIDEBAR_MORE_OPTIONS));
}

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

void PanelTitleBar::PaintDecoration (vcl::RenderContext& rRenderContext, const tools::Rectangle& /*rTitleBarBox*/)
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

Paint PanelTitleBar::GetBackgroundPaint()
{
    return Theme::GetPaint(Theme::Paint_PanelTitleBarBackground);
}

void PanelTitleBar::HandleToolBoxItemClick (const sal_uInt16 nItemIndex)
{
    if (nItemIndex != mnMenuItemIndex)
        return;

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
    SetAccessibleName(msTitle);
    SetAccessibleDescription(msTitle);
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
    maToolBox->SetItemImage(
        mnMenuItemIndex,
        Theme::GetImage(Theme::Image_PanelMenu));
    TitleBar::DataChanged(rEvent);
}

} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
