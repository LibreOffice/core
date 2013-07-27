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

#include "PanelTitleBar.hxx"
#include "sfx2/sfxresid.hxx"
#include "Sidebar.hrc"

#include "Paint.hxx"
#include "Panel.hxx"
#include "sfx2/sidebar/Theme.hxx"
#include "sfx2/sidebar/ControllerFactory.hxx"
#include "sfx2/sidebar/Tools.hxx"
#include <tools/svborder.hxx>
#include <vcl/gradient.hxx>
#include <vcl/image.hxx>
#include <toolkit/helper/vclunohelper.hxx>

using namespace css;
using namespace cssu;

namespace sfx2 { namespace sidebar {


static const sal_Int32 gaLeftIconPadding (5);
static const sal_Int32 gaRightIconPadding (5);


PanelTitleBar::PanelTitleBar (
    const ::rtl::OUString& rsTitle,
    Window* pParentWindow,
    Panel* pPanel)
    : TitleBar(rsTitle, pParentWindow, GetBackgroundPaint()),
      mbIsLeftButtonDown(false),
      mpPanel(pPanel),
      mnMenuItemIndex(1),
      mxFrame(),
      msMoreOptionsCommand(),
      msAccessibleNamePrefix(SFX2_RESSTR(SFX_STR_SIDEBAR_ACCESSIBILITY_PANEL_PREFIX))
{
    OSL_ASSERT(mpPanel != NULL);

#ifdef DEBUG
    SetText(A2S("PanelTitleBar"));
#endif
}




PanelTitleBar::~PanelTitleBar (void)
{
}




void PanelTitleBar::SetMoreOptionsCommand (
    const ::rtl::OUString& rsCommandName,
    const ::cssu::Reference<css::frame::XFrame>& rxFrame)
{
    if ( ! rsCommandName.equals(msMoreOptionsCommand))
    {
        if (msMoreOptionsCommand.getLength() > 0)
            maToolBox.RemoveItem(maToolBox.GetItemPos(mnMenuItemIndex));

        msMoreOptionsCommand = rsCommandName;
        mxFrame = rxFrame;

        if (msMoreOptionsCommand.getLength() > 0)
        {
            maToolBox.InsertItem(
                mnMenuItemIndex,
                Theme::GetImage(Theme::Image_PanelMenu));
            Reference<frame::XToolbarController> xController (
                ControllerFactory::CreateToolBoxController(
                    &maToolBox,
                    mnMenuItemIndex,
                    msMoreOptionsCommand,
                    rxFrame,
                    VCLUnoHelper::GetInterface(&maToolBox),
                    0));
            maToolBox.SetController(mnMenuItemIndex, xController, msMoreOptionsCommand);
            maToolBox.SetOutStyle(TOOLBOX_STYLE_FLAT);
            maToolBox.SetQuickHelpText(
                mnMenuItemIndex,
                SFX2_RESSTR(SFX_STR_SIDEBAR_MORE_OPTIONS));
        }
    }
}




Rectangle PanelTitleBar::GetTitleArea (const Rectangle& rTitleBarBox)
{
    if (mpPanel != NULL)
    {
        Image aImage (mpPanel->IsExpanded()
            ? Theme::GetImage(Theme::Image_Expand)
            : Theme::GetImage(Theme::Image_Collapse));
        return Rectangle(
            aImage.GetSizePixel().Width() + gaLeftIconPadding + gaRightIconPadding,
            rTitleBarBox.Top(),
            rTitleBarBox.Right(),
            rTitleBarBox.Bottom());
    }
    else
        return rTitleBarBox;
}




void PanelTitleBar::PaintDecoration (const Rectangle& rTitleBarBox)
{
    (void)rTitleBarBox;

    if (mpPanel != NULL)
    {
        Image aImage (mpPanel->IsExpanded()
            ? Theme::GetImage(Theme::Image_Collapse)
            : Theme::GetImage(Theme::Image_Expand));
        const Point aTopLeft (
            gaLeftIconPadding,
            (GetSizePixel().Height()-aImage.GetSizePixel().Height())/2);
        DrawImage(aTopLeft, aImage);
    }
}




Paint PanelTitleBar::GetBackgroundPaint (void)
{
    return Theme::GetPaint(Theme::Paint_PanelTitleBarBackground);
}




Color PanelTitleBar::GetTextColor (void)
{
    return Theme::GetColor(Theme::Color_PanelTitleFont);
}




void PanelTitleBar::HandleToolBoxItemClick (const sal_uInt16 nItemIndex)
{
    if (nItemIndex == mnMenuItemIndex)
        if (msMoreOptionsCommand.getLength() > 0)
        {
            try
            {
                const util::URL aURL (Tools::GetURL(msMoreOptionsCommand));
                Reference<frame::XDispatch> xDispatch (Tools::GetDispatch(mxFrame, aURL));
                if (xDispatch.is())
                    xDispatch->dispatch(aURL, Sequence<beans::PropertyValue>());
            }
            catch(Exception& rException)
            {
                OSL_TRACE("caught exception: %s",
                    OUStringToOString(rException.Message, RTL_TEXTENCODING_ASCII_US).getStr());
            }
        }
}




Reference<accessibility::XAccessible> PanelTitleBar::CreateAccessible (void)
{
    const ::rtl::OUString sAccessibleName(msAccessibleNamePrefix + msTitle);
    SetAccessibleName(sAccessibleName);
    SetAccessibleDescription(sAccessibleName);
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
            if (mpPanel != NULL)
            {
                mpPanel->SetExpanded( ! mpPanel->IsExpanded());
                Invalidate();
            }
        }
    }
    if (mbIsLeftButtonDown)
        mbIsLeftButtonDown = false;
}




void PanelTitleBar::DataChanged (const DataChangedEvent& rEvent)
{
    maToolBox.SetItemImage(
        mnMenuItemIndex,
        Theme::GetImage(Theme::Image_PanelMenu));
    TitleBar::DataChanged(rEvent);
}

} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
