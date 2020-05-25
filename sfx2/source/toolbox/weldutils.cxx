/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/frame/XSubToolbarController.hpp>
#include <sidebar/ControllerFactory.hxx>
#include <sfx2/weldutils.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/settings.hxx>
#include <vcl/weld.hxx>

namespace
{
bool lcl_RTLizeCommandURL(OUString& rCommandURL)
{
    if (rCommandURL == ".uno:ParaLeftToRight")
    {
        rCommandURL = ".uno:ParaRightToLeft";
        return true;
    }
    if (rCommandURL == ".uno:ParaRightToLeft")
    {
        rCommandURL = ".uno:ParaLeftToRight";
        return true;
    }
    if (rCommandURL == ".uno:LeftPara")
    {
        rCommandURL = ".uno:RightPara";
        return true;
    }
    if (rCommandURL == ".uno:RightPara")
    {
        rCommandURL = ".uno:LeftPara";
        return true;
    }
    if (rCommandURL == ".uno:AlignLeft")
    {
        rCommandURL = ".uno:AlignRight";
        return true;
    }
    if (rCommandURL == ".uno:AlignRight")
    {
        rCommandURL = ".uno:AlignLeft";
        return true;
    }
    return false;
}
}

// for now all controllers are in the sidebar
vcl::ImageType ToolbarUnoDispatcher::GetIconSize() const
{
    vcl::ImageType eType = vcl::ImageType::Size16;
    switch (m_aToolbarOptions.GetSidebarIconSize())
    {
        case ToolBoxButtonSize::Large:
            eType = vcl::ImageType::Size26;
            break;
        case ToolBoxButtonSize::Size32:
            eType = vcl::ImageType::Size32;
            break;
        case ToolBoxButtonSize::DontCare:
        case ToolBoxButtonSize::Small:
            break;
    }
    return eType;
}

ToolbarUnoDispatcher::ToolbarUnoDispatcher(weld::Toolbar& rToolbar, weld::Builder& rBuilder,
                                           const css::uno::Reference<css::frame::XFrame>& rFrame,
                                           bool bSideBar)
    : m_xFrame(rFrame)
    , m_pToolbar(&rToolbar)
    , m_pBuilder(&rBuilder)
    , m_bSideBar(bSideBar)
{
    rToolbar.connect_clicked(LINK(this, ToolbarUnoDispatcher, SelectHdl));
    rToolbar.connect_menu_toggled(LINK(this, ToolbarUnoDispatcher, ToggleMenuHdl));

    OUString aModuleName(vcl::CommandInfoProvider::GetModuleIdentifier(rFrame));
    vcl::ImageType eSize = GetIconSize();
    rToolbar.set_icon_size(eSize);

    bool bRTL = AllSettings::GetLayoutRTL();

    for (int i = 0, nItems = rToolbar.get_n_items(); i < nItems; ++i)
    {
        OString sIdent(rToolbar.get_item_ident(i));
        if (!sIdent.startsWith(".uno:"))
            continue;
        OUString sCommand = OUString::fromUtf8(sIdent);
        if (bRTL && lcl_RTLizeCommandURL(sCommand))
            rToolbar.set_item_ident(i, sCommand.toUtf8());

        auto aProperties = vcl::CommandInfoProvider::GetCommandProperties(sCommand, aModuleName);
        OUString aLabel(vcl::CommandInfoProvider::GetLabelForCommand(aProperties));
        rToolbar.set_item_label(i, aLabel);
        OUString aTooltip(
            vcl::CommandInfoProvider::GetTooltipForCommand(sCommand, aProperties, rFrame));
        rToolbar.set_item_tooltip_text(i, aTooltip);
        auto xImage(vcl::CommandInfoProvider::GetXGraphicForCommand(sCommand, rFrame, eSize));
        rToolbar.set_item_image(i, xImage);

        CreateController(sCommand);
    }

    m_aToolbarOptions.AddListenerLink(LINK(this, ToolbarUnoDispatcher, ChangedIconSizeHandler));
}

void ToolbarUnoDispatcher::CreateController(const OUString& rCommand)
{
    css::uno::Reference<css::frame::XToolbarController> xController(
        sfx2::sidebar::ControllerFactory::CreateToolBoxController(*m_pToolbar, *m_pBuilder,
                                                                  rCommand, m_xFrame, m_bSideBar));

    if (xController.is())
        maControllers.insert(std::make_pair(rCommand, xController));
}

css::uno::Reference<css::frame::XToolbarController>
ToolbarUnoDispatcher::GetControllerForCommand(const OUString& rCommand) const
{
    ControllerContainer::const_iterator iController(maControllers.find(rCommand));
    if (iController != maControllers.end())
        return iController->second;

    return css::uno::Reference<css::frame::XToolbarController>();
}

IMPL_LINK(ToolbarUnoDispatcher, SelectHdl, const OString&, rCommand, void)
{
    css::uno::Reference<css::frame::XToolbarController> xController(
        GetControllerForCommand(OUString::fromUtf8(rCommand)));

    if (xController.is())
        xController->execute(0);
}

IMPL_LINK(ToolbarUnoDispatcher, ToggleMenuHdl, const OString&, rCommand, void)
{
    css::uno::Reference<css::frame::XToolbarController> xController(
        GetControllerForCommand(OUString::fromUtf8(rCommand)));

    if (xController.is())
        xController->click();
}

IMPL_LINK_NOARG(ToolbarUnoDispatcher, ChangedIconSizeHandler, LinkParamNone*, void)
{
    vcl::ImageType eSize = GetIconSize();
    m_pToolbar->set_icon_size(eSize);

    for (int i = 0, nItems = m_pToolbar->get_n_items(); i < nItems; ++i)
    {
        OUString sCommand = OUString::fromUtf8(m_pToolbar->get_item_ident(i));
        auto xImage(vcl::CommandInfoProvider::GetXGraphicForCommand(sCommand, m_xFrame, eSize));
        m_pToolbar->set_item_image(i, xImage);
    }

    for (auto const& it : maControllers)
    {
        css::uno::Reference<css::frame::XSubToolbarController> xController(it.second,
                                                                           css::uno::UNO_QUERY);
        if (xController.is() && xController->opensSubToolbar())
        {
            // The button should show the last function that was selected from the
            // dropdown. The controller should know better than us what it was.
            xController->updateImage();
        }
    }
}

void ToolbarUnoDispatcher::dispose()
{
    if (!m_pToolbar)
        return;

    m_aToolbarOptions.RemoveListenerLink(LINK(this, ToolbarUnoDispatcher, ChangedIconSizeHandler));

    ControllerContainer aControllers;
    aControllers.swap(maControllers);
    for (auto const& controller : aControllers)
    {
        css::uno::Reference<css::lang::XComponent> xComponent(controller.second,
                                                              css::uno::UNO_QUERY);
        if (xComponent.is())
            xComponent->dispose();
    }

    m_pToolbar->connect_clicked(Link<const OString&, void>());
    m_pToolbar = nullptr;
    m_pBuilder = nullptr;
}

ToolbarUnoDispatcher::~ToolbarUnoDispatcher() { dispose(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
