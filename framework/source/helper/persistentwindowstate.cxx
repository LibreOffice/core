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

#include <helper/persistentwindowstate.hxx>

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>

#include <comphelper/lok.hxx>
#include <comphelper/configurationhelper.hxx>
#include <utility>
#include <vcl/window.hxx>
#include <vcl/syswin.hxx>

#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>

namespace framework{

PersistentWindowState::PersistentWindowState(css::uno::Reference< css::uno::XComponentContext >  xContext)
    : m_xContext              (std::move(xContext                     ))
    , m_bWindowStateAlreadySet(false                    )
{
}

PersistentWindowState::~PersistentWindowState()
{
}

void SAL_CALL PersistentWindowState::initialize(const css::uno::Sequence< css::uno::Any >& lArguments)
{
    // check arguments
    css::uno::Reference< css::frame::XFrame > xFrame;
    if (!lArguments.hasElements())
        throw css::lang::IllegalArgumentException(
                u"Empty argument list!"_ustr,
                static_cast< ::cppu::OWeakObject* >(this),
                1);

    lArguments[0] >>= xFrame;
    if (!xFrame.is())
        throw css::lang::IllegalArgumentException(
                u"No valid frame specified!"_ustr,
                static_cast< ::cppu::OWeakObject* >(this),
                1);

    {
        SolarMutexGuard g;
        m_xFrame = xFrame;
    }

    // start listening
    xFrame->addFrameActionListener(this);
}

void SAL_CALL PersistentWindowState::frameAction(const css::frame::FrameActionEvent& aEvent)
{
    // We don't want to do this stuff when being used through LibreOfficeKit
    if( comphelper::LibreOfficeKit::isActive() )
        return;

    css::uno::Reference< css::uno::XComponentContext >     xContext;
    css::uno::Reference< css::frame::XFrame >              xFrame;
    bool                                               bRestoreWindowState;
    {
        SolarMutexGuard g;
        xContext = m_xContext;
        xFrame.set(m_xFrame.get(), css::uno::UNO_QUERY);
        bRestoreWindowState = !m_bWindowStateAlreadySet;
    }

    // frame already gone ? We hold it weak only ...
    if (!xFrame.is())
        return;

    // no window -> no position and size available
    css::uno::Reference< css::awt::XWindow > xWindow = xFrame->getContainerWindow();
    if (!xWindow.is())
        return;

    // unknown module -> no configuration available!
    OUString sModuleName = PersistentWindowState::implst_identifyModule(xContext, xFrame);
    if (sModuleName.isEmpty())
        return;

    switch(aEvent.Action)
    {
        case css::frame::FrameAction_COMPONENT_ATTACHED :
            {
                if (bRestoreWindowState)
                {
                    OUString sWindowState = PersistentWindowState::implst_getWindowStateFromConfig(xContext, sModuleName);
                    PersistentWindowState::implst_setWindowStateOnWindow(xWindow,sWindowState);
                    SolarMutexGuard g;
                    m_bWindowStateAlreadySet = true;
                }
            }
            break;

        case css::frame::FrameAction_COMPONENT_REATTACHED :
            {
                // nothing todo here, because it's not allowed to change position and size
                // of an already existing frame!
            }
            break;

        case css::frame::FrameAction_COMPONENT_DETACHING :
            {
                OUString sWindowState = PersistentWindowState::implst_getWindowStateFromWindow(xWindow);
                PersistentWindowState::implst_setWindowStateOnConfig(xContext, sModuleName, sWindowState);
            }
            break;
        default:
            break;
    }
}

void SAL_CALL PersistentWindowState::disposing(const css::lang::EventObject&)
{
    css::uno::Reference< css::frame::XFrame > xFrame(m_xFrame.get(), css::uno::UNO_QUERY);
    if (xFrame.is())
        xFrame->removeFrameActionListener(this);

    // nothing todo here - because we hold the frame as weak reference only
}

OUString PersistentWindowState::implst_identifyModule(const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                                                             const css::uno::Reference< css::frame::XFrame >&              xFrame)
{
    OUString sModuleName;

    css::uno::Reference< css::frame::XModuleManager2 > xModuleManager =
        css::frame::ModuleManager::create( rxContext );

    try
    {
        sModuleName = xModuleManager->identify(xFrame);
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        { sModuleName.clear(); }

    return sModuleName;
}

OUString PersistentWindowState::implst_getWindowStateFromConfig(
        const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        std::u16string_view sModuleName)
{
    OUString sWindowState;
    try
    {
        ::comphelper::ConfigurationHelper::readDirectKey(rxContext,
            u"org.openoffice.Setup/"_ustr,
            OUString::Concat("Office/Factories/*[\"") + sModuleName + "\"]",
            u"ooSetupFactoryWindowAttributes"_ustr,
            ::comphelper::EConfigurationModes::ReadOnly) >>= sWindowState;
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        { sWindowState.clear(); }

    return sWindowState;
}

void PersistentWindowState::implst_setWindowStateOnConfig(
        const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        std::u16string_view sModuleName, const OUString& sWindowState)
{
    try
    {
        ::comphelper::ConfigurationHelper::writeDirectKey(rxContext,
            u"org.openoffice.Setup/"_ustr,
            OUString::Concat("Office/Factories/*[\"") + sModuleName + "\"]",
            u"ooSetupFactoryWindowAttributes"_ustr,
            css::uno::Any(sWindowState),
            ::comphelper::EConfigurationModes::Standard);
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        {}
}

OUString PersistentWindowState::implst_getWindowStateFromWindow(const css::uno::Reference< css::awt::XWindow >& xWindow)
{
    OUString sWindowState;

    if (xWindow.is())
    {
        // SOLAR SAFE -> ------------------------
        SolarMutexGuard aSolarGuard;

        VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow(xWindow);
        // check for system window is necessary to guarantee correct pointer cast!
        if ( pWindow && pWindow->IsSystemWindow() )
        {
            vcl::WindowDataMask const nMask = vcl::WindowDataMask::All & ~vcl::WindowDataMask::Minimized;
            sWindowState = static_cast<SystemWindow*>(pWindow.get())->GetWindowState(nMask);
        }
        // <- SOLAR SAFE ------------------------
    }

    return sWindowState;
}

void PersistentWindowState::implst_setWindowStateOnWindow(const css::uno::Reference< css::awt::XWindow >& xWindow     ,
                                                          std::u16string_view                      sWindowState)
{
    if (
        (!xWindow.is()                ) ||
        ( sWindowState.empty() )
       )
        return;

    // SOLAR SAFE -> ------------------------
    SolarMutexGuard aSolarGuard;

    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow(xWindow);
    if (!pWindow)
        return;

    // check for system and work window - it's necessary to guarantee correct pointer cast!
    bool bSystemWindow = pWindow->IsSystemWindow();
    bool bWorkWindow   = (pWindow->GetType() == WindowType::WORKWINDOW);

    if (!bSystemWindow && !bWorkWindow)
        return;

    SystemWindow* pSystemWindow = static_cast<SystemWindow*>(pWindow.get());
    WorkWindow*   pWorkWindow   = static_cast<WorkWindow*  >(pWindow.get());

    // don't save this special state!
    if (pWorkWindow->IsMinimized())
        return;

    OUString sOldWindowState = pSystemWindow->GetWindowState();
    if ( sOldWindowState != sWindowState )
        pSystemWindow->SetWindowState(sWindowState);
    // <- SOLAR SAFE ------------------------
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
