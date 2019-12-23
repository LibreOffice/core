/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/dispatchcommand.hxx>
#include <sfx2/sidebar/ControllerFactory.hxx>
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

ToolbarUnoDispatcher::ToolbarUnoDispatcher(weld::Toolbar& rToolbar,
                                           const css::uno::Reference<css::frame::XFrame>& rFrame)
    : m_xFrame(rFrame)
    , m_pToolbar(&rToolbar)
{
    OUString aModuleName(vcl::CommandInfoProvider::GetModuleIdentifier(rFrame));
    vcl::ImageType eSize = rToolbar.get_icon_size();

    bool bRTL = AllSettings::GetLayoutRTL();

    for (int i = 0, nItems = rToolbar.get_n_items(); i < nItems; ++i)
    {
        OUString sCommand = OUString::fromUtf8(rToolbar.get_item_ident(i));
        if (bRTL && lcl_RTLizeCommandURL(sCommand))
            rToolbar.set_item_ident(i, sCommand.toUtf8());

        auto aProperties = vcl::CommandInfoProvider::GetCommandProperties(sCommand, aModuleName);
        OUString aLabel(vcl::CommandInfoProvider::GetLabelForCommand(aProperties));
        rToolbar.set_item_label(i, aLabel);
        OUString aTooltip(
            vcl::CommandInfoProvider::GetTooltipForCommand(sCommand, aProperties, rFrame));
        rToolbar.set_item_tooltip_text(i, aTooltip);
        auto xImage(vcl::CommandInfoProvider::GetXGraphicForCommand(sCommand, rFrame, eSize));
        rToolbar.set_item_icon(i, xImage);

        CreateController(sCommand);
    }

    rToolbar.connect_clicked(LINK(this, ToolbarUnoDispatcher, SelectHdl));
}

void ToolbarUnoDispatcher::CreateController(const OUString& rCommand)
{
    css::uno::Reference<css::frame::XToolbarController> xController(
        sfx2::sidebar::ControllerFactory::CreateToolBoxController(*m_pToolbar, rCommand, m_xFrame));

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

void ToolbarUnoDispatcher::dispose()
{
    if (!m_pToolbar)
        return;

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
}

ToolbarUnoDispatcher::~ToolbarUnoDispatcher() { dispose(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
