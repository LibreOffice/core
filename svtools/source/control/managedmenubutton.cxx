/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/menu.hxx>
#include <vcl/menubtn.hxx>

#include <com/sun/star/frame/theDesktop.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/thePopupMenuControllerFactory.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>

namespace {

class ManagedMenuButton : public MenuButton
{
public:
    ManagedMenuButton(vcl::Window* pParent, WinBits nStyle);
    ~ManagedMenuButton();

    void Activate() override;
    void dispose() override;

private:
    css::uno::Reference<css::frame::XPopupMenuController> m_xPopupController;
};

ManagedMenuButton::ManagedMenuButton(vcl::Window* pParent, WinBits nStyle)
    : MenuButton(pParent, nStyle)
{
    SetImageAlign(ImageAlign::Left);
}

ManagedMenuButton::~ManagedMenuButton()
{
    disposeOnce();
}

void ManagedMenuButton::dispose()
{
    // Disposing the controller will delete the VCLXPopupMenu, which in turn will also
    // dispose the VCL Menu instance. This is needed if the menu was created in Activate().
    css::uno::Reference<css::lang::XComponent> xComponent(m_xPopupController, css::uno::UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose();

    m_xPopupController.clear();
    MenuButton::dispose();
}

void ManagedMenuButton::Activate()
{
    if (!GetPopupMenu())
        SetPopupMenu(VclPtr<PopupMenu>::Create());

    MenuButton::Activate();

    if (m_xPopupController.is())
    {
        m_xPopupController->updatePopupMenu();
        return;
    }

    // FIXME: get the frame from the parent VclBuilder.
    css::uno::Reference<css::uno::XComponentContext> xContext(comphelper::getProcessComponentContext());
    css::uno::Reference<css::frame::XDesktop2> xDesktop(css::frame::theDesktop::get(xContext));
    css::uno::Reference<css::frame::XFrame> xFrame(xDesktop->getActiveFrame());
    if (!xFrame.is())
        return;

    OUString aModuleName;
    try
    {
        css::uno::Reference<css::frame::XModuleManager> xModuleManager(css::frame::ModuleManager::create(xContext));
        aModuleName = xModuleManager->identify(xFrame);
    }
    catch( const css::uno::Exception& )
    {}

    css::uno::Sequence<css::uno::Any> aArgs {
        css::uno::makeAny(comphelper::makePropertyValue("ModuleIdentifier", aModuleName)),
        css::uno::makeAny(comphelper::makePropertyValue("Frame", css::uno::makeAny(xFrame))),
        css::uno::makeAny(comphelper::makePropertyValue("InToolbar", css::uno::makeAny(true)))
    };

    const OUString aCommand(GetCommand());
    if (!aCommand.isEmpty() && GetPopupMenu()->GetItemCount() == 0)
    {
        css::uno::Reference<css::frame::XUIControllerFactory> xPopupMenuControllerFactory =
            css::frame::thePopupMenuControllerFactory::get(xContext);

        if (xPopupMenuControllerFactory->hasController(aCommand, aModuleName))
            m_xPopupController.set(xPopupMenuControllerFactory->createInstanceWithArgumentsAndContext(
                aCommand, aArgs, xContext), css::uno::UNO_QUERY);
    }

    // No registered controller found, use one the can handle arbitrary menus (e.g. defined in .ui file).
    if (!m_xPopupController.is())
        m_xPopupController.set(xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
            "com.sun.star.comp.framework.ResourceMenuController", aArgs, xContext), css::uno::UNO_QUERY);

    rtl::Reference<VCLXPopupMenu> xPopupMenu(new VCLXPopupMenu(GetPopupMenu()));
    if (m_xPopupController.is())
        m_xPopupController->setPopupMenu(xPopupMenu.get());
}

}

VCL_BUILDER_FACTORY_ARGS(ManagedMenuButton, WB_CLIPCHILDREN|WB_CENTER|WB_VCENTER|WB_FLATBUTTON)

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
