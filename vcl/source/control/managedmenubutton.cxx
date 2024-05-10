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

#include <managedmenubutton.hxx>
#include <vcl/menu.hxx>

#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/theDesktop.hpp>
#include <com/sun/star/frame/thePopupMenuControllerFactory.hpp>

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
    css::uno::Reference<css::lang::XComponent> xComponent(m_xPopupController, css::uno::UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose();

    m_xPopupMenu.clear();
    m_xPopupController.clear();
    MenuButton::dispose();
}

void ManagedMenuButton::PrepareExecute()
{
    if (!GetPopupMenu())
        SetPopupMenu(VclPtr<PopupMenu>::Create());

    MenuButton::PrepareExecute();

    if (m_xPopupController.is())
    {
        m_xPopupController->updatePopupMenu();
        return;
    }

    if (!m_xPopupMenu.is())
        m_xPopupMenu = GetPopupMenu()->CreateMenuInterface();

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
        css::uno::Any(comphelper::makePropertyValue(u"ModuleIdentifier"_ustr, aModuleName)),
        css::uno::Any(comphelper::makePropertyValue(u"Frame"_ustr, css::uno::Any(xFrame))),
        css::uno::Any(comphelper::makePropertyValue(u"InToolbar"_ustr, css::uno::Any(true)))
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
            u"com.sun.star.comp.framework.ResourceMenuController"_ustr, aArgs, xContext), css::uno::UNO_QUERY);

    if (m_xPopupController.is())
        m_xPopupController->setPopupMenu(m_xPopupMenu);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
