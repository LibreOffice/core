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

#include <sidebar/SidebarToolBox.hxx>
#include <sidebar/ControllerFactory.hxx>
#include <sfx2/viewfrm.hxx>

#include <vcl/commandinfoprovider.hxx>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svtools/miscopt.hxx>
#include <com/sun/star/frame/XSubToolbarController.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XToolbarController.hpp>

using namespace css;
using namespace css::uno;

namespace {
    void lcl_RTLizeCommandURL( OUString& rCommandURL )
    {
        if (rCommandURL == ".uno:ParaLeftToRight")
            rCommandURL = ".uno:ParaRightToLeft";
        else if (rCommandURL == ".uno:ParaRightToLeft")
            rCommandURL = ".uno:ParaLeftToRight";
        else if (rCommandURL == ".uno:LeftPara")
            rCommandURL = ".uno:RightPara";
        else if (rCommandURL == ".uno:RightPara")
            rCommandURL = ".uno:LeftPara";
        else if (rCommandURL == ".uno:AlignLeft")
            rCommandURL = ".uno:AlignRight";
        else if (rCommandURL == ".uno:AlignRight")
            rCommandURL = ".uno:AlignLeft";
    }
}

namespace sfx2::sidebar {

SidebarToolBox::SidebarToolBox (vcl::Window* pParentWindow)
    : ToolBox(pParentWindow, 0),
      mbAreHandlersRegistered(false),
      mbUseDefaultButtonSize(true),
      mbSideBar(true)
{
    SetBackground(Wallpaper());
    SetPaintTransparent(true);
    SetToolboxButtonSize(GetDefaultButtonSize());

    SvtMiscOptions().AddListenerLink(LINK(this, SidebarToolBox, ChangedIconSizeHandler));

#ifdef DEBUG
    SetText(OUString("SidebarToolBox"));
#endif
}

SidebarToolBox::~SidebarToolBox()
{
    disposeOnce();
}

void SidebarToolBox::dispose()
{
    SvtMiscOptions().RemoveListenerLink(LINK(this, SidebarToolBox, ChangedIconSizeHandler));

    ControllerContainer aControllers;
    aControllers.swap(maControllers);
    for (auto const& controller : aControllers)
    {
        Reference<lang::XComponent> xComponent(controller.second, UNO_QUERY);
        if (xComponent.is())
            xComponent->dispose();
    }

    if (mbAreHandlersRegistered)
    {
        SetDropdownClickHdl(Link<ToolBox *, void>());
        SetClickHdl(Link<ToolBox *, void>());
        SetDoubleClickHdl(Link<ToolBox *, void>());
        SetSelectHdl(Link<ToolBox *, void>());
        SetActivateHdl(Link<ToolBox *, void>());
        SetDeactivateHdl(Link<ToolBox *, void>());
        mbAreHandlersRegistered = false;
    }

    ToolBox::dispose();
}

ToolBoxButtonSize SidebarToolBox::GetDefaultButtonSize() const
{
    return SvtMiscOptions().GetSidebarIconSize();
}

void SidebarToolBox::InsertItem(const OUString& rCommand,
        const css::uno::Reference<css::frame::XFrame>& rFrame,
        ToolBoxItemBits nBits, const Size& rRequestedSize, ImplToolItems::size_type nPos)
{
    OUString aCommand( rCommand );

    if( AllSettings::GetLayoutRTL() )
    {
        lcl_RTLizeCommandURL( aCommand );
    }

    ToolBox::InsertItem(aCommand, rFrame, nBits, rRequestedSize, nPos);

    CreateController(GetItemId(aCommand), rFrame, std::max(rRequestedSize.Width(), ::tools::Long(0)), mbSideBar);
    RegisterHandlers();
}

bool SidebarToolBox::EventNotify (NotifyEvent& rEvent)
{
    if (rEvent.GetType() == MouseNotifyEvent::KEYINPUT)
    {
        if (rEvent.GetKeyEvent()->GetKeyCode().GetCode() == KEY_TAB)
        {
            // Special handling for transferring handling of KEY_TAB
            // that becomes necessary because of our parent that is
            // not the dialog but a background control.
            return DockingWindow::EventNotify(rEvent);
        }
    }
    return ToolBox::EventNotify(rEvent);
}

void SidebarToolBox::KeyInput(const KeyEvent& rKEvt)
{
    if (KEY_ESCAPE != rKEvt.GetKeyCode().GetCode())
        ToolBox::KeyInput(rKEvt);
}

void SidebarToolBox::CreateController (
    const ToolBoxItemId nItemId,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    const sal_Int32 nItemWidth, bool bSideBar)
{
    const OUString sCommandName (GetItemCommand(nItemId));

    uno::Reference<frame::XToolbarController> xController(sfx2::sidebar::ControllerFactory::CreateToolBoxController(
            this, nItemId, sCommandName, rxFrame, rxFrame->getController(),
            VCLUnoHelper::GetInterface(this), nItemWidth, bSideBar));

    if (xController.is())
        maControllers.insert(std::make_pair(nItemId, xController));
}

Reference<frame::XToolbarController> SidebarToolBox::GetControllerForItemId (const ToolBoxItemId nItemId) const
{
    ControllerContainer::const_iterator iController (maControllers.find(nItemId));
    if (iController != maControllers.end())
        return iController->second;

    return Reference<frame::XToolbarController>();
}

void SidebarToolBox::RegisterHandlers()
{
    if ( ! mbAreHandlersRegistered)
    {
        mbAreHandlersRegistered = true;
        SetDropdownClickHdl(LINK(this, SidebarToolBox, DropDownClickHandler));
        SetClickHdl(LINK(this, SidebarToolBox, ClickHandler));
        SetDoubleClickHdl(LINK(this, SidebarToolBox, DoubleClickHandler));
        SetSelectHdl(LINK(this, SidebarToolBox, SelectHandler));
    }
}

IMPL_LINK(SidebarToolBox, DropDownClickHandler, ToolBox*, pToolBox, void)
{
    if (pToolBox != nullptr)
    {
        Reference<frame::XToolbarController> xController (GetControllerForItemId(pToolBox->GetCurItemId()));
        if (xController.is())
        {
            Reference<awt::XWindow> xWindow = xController->createPopupWindow();
            if (xWindow.is() )
                xWindow->setFocus();
        }
    }
}

IMPL_LINK(SidebarToolBox, ClickHandler, ToolBox*, pToolBox, void)
{
    if (pToolBox == nullptr)
        return;

    Reference<frame::XToolbarController> xController (GetControllerForItemId(pToolBox->GetCurItemId()));
    if (xController.is())
        xController->click();
}

IMPL_LINK(SidebarToolBox, DoubleClickHandler, ToolBox*, pToolBox, void)
{
    if (pToolBox == nullptr)
        return;

    Reference<frame::XToolbarController> xController (GetControllerForItemId(pToolBox->GetCurItemId()));
    if (xController.is())
        xController->doubleClick();
}

IMPL_LINK(SidebarToolBox, SelectHandler, ToolBox*, pToolBox, void)
{
    if (pToolBox == nullptr)
        return;

    Reference<frame::XToolbarController> xController (GetControllerForItemId(pToolBox->GetCurItemId()));
    if (xController.is())
        xController->execute(static_cast<sal_Int16>(pToolBox->GetModifier()));
}

IMPL_LINK_NOARG(SidebarToolBox, ChangedIconSizeHandler, LinkParamNone*, void)
{
    SolarMutexGuard g;

    if (mbUseDefaultButtonSize)
        SetToolboxButtonSize(GetDefaultButtonSize());

    for (auto const& it : maControllers)
    {
        Reference<frame::XSubToolbarController> xController(it.second, UNO_QUERY);
        if (xController.is() && xController->opensSubToolbar())
        {
            // The button should show the last function that was selected from the
            // dropdown. The controller should know better than us what it was.
            xController->updateImage();
        }
        else if (SfxViewFrame::Current())
        {
            OUString aCommandURL = GetItemCommand(it.first);
            css::uno::Reference<frame::XFrame> xFrame = SfxViewFrame::Current()->GetFrame().GetFrameInterface();
            Image aImage = vcl::CommandInfoProvider::GetImageForCommand(aCommandURL, xFrame, GetImageSize());
            SetItemImage(it.first, aImage);
        }
    }

    Resize();
    queue_resize();
}

void SidebarToolBox::InitToolBox(VclBuilder::stringmap& rMap)
{
    for (const auto& it : rMap)
    {
        if (it.first == "toolbar-style")
        {
            if (it.second == "text")
                SetButtonType(ButtonType::TEXT);
            else if (it.second == "both-horiz")
                SetButtonType(ButtonType::SYMBOLTEXT);
            else if (it.second == "both")
            {
                SetButtonType(ButtonType::SYMBOLTEXT);
                SetToolBoxTextPosition(ToolBoxTextPosition::Bottom);
            }
        }
        else if (it.first == "icon-size")
        {
            mbUseDefaultButtonSize = false;
            if (it.second == "1" || it.second == "2" || it.second == "4")
                SetToolboxButtonSize(ToolBoxButtonSize::Small);
            else if (it.second == "3")
                SetToolboxButtonSize(ToolBoxButtonSize::Large);
            else if (it.second == "5")
                SetToolboxButtonSize(ToolBoxButtonSize::Size32);
        }
        else if (it.first == "orientation" && it.second == "vertical")
            SetAlign(WindowAlign::Left);
    }
}

namespace {

class NotebookbarToolBox : public SidebarToolBox
{
public:
    explicit NotebookbarToolBox(vcl::Window* pParentWindow)
    : SidebarToolBox(pParentWindow)
    {
        mbSideBar = false;
        SetToolboxButtonSize(GetDefaultButtonSize());
    }

    virtual ToolBoxButtonSize GetDefaultButtonSize() const override
    {
        return SvtMiscOptions().GetNotebookbarIconSize();
    }
};

}

extern "C" SAL_DLLPUBLIC_EXPORT void makeNotebookbarToolBox(VclPtr<vcl::Window> & rRet, const VclPtr<vcl::Window> & pParent, VclBuilder::stringmap & rMap)
{
    static_assert(std::is_same_v<std::remove_pointer_t<VclBuilder::customMakeWidget>,
                                 decltype(makeNotebookbarToolBox)>);
    VclPtrInstance<NotebookbarToolBox> pBox(pParent);
    pBox->InitToolBox(rMap);
    rRet = pBox;
}

} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
