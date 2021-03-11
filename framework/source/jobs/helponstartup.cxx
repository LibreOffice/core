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

// include own header
#include <jobs/helponstartup.hxx>
#include <services.h>
#include <targets.h>

#include <officecfg/Office/Common.hxx>
#include <officecfg/Setup.hxx>

// include others
#include <comphelper/sequenceashashmap.hxx>
#include <vcl/svapp.hxx>
#include <vcl/help.hxx>

// include interfaces
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <cppuhelper/supportsservice.hxx>

namespace framework{

// XInterface, XTypeProvider, XServiceInfo

OUString SAL_CALL HelpOnStartup::getImplementationName()
{
    return "com.sun.star.comp.framework.HelpOnStartup";
}

sal_Bool SAL_CALL HelpOnStartup::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

css::uno::Sequence< OUString > SAL_CALL HelpOnStartup::getSupportedServiceNames()
{
    return { SERVICENAME_JOB };
}

HelpOnStartup::HelpOnStartup(const css::uno::Reference< css::uno::XComponentContext >& xContext)
    : m_xContext    (xContext)
{
    // create some needed uno services and cache it
    m_xModuleManager = css::frame::ModuleManager::create( m_xContext );

    m_xDesktop = css::frame::Desktop::create(m_xContext);

    // ask for office locale
    m_sLocale = officecfg::Setup::L10N::ooLocale::get(m_xContext);

    // detect system
    m_sSystem = officecfg::Office::Common::Help::System::get(m_xContext);

    // Start listening for disposing events of these services,
    // so we can react e.g. for an office shutdown
    css::uno::Reference< css::lang::XComponent > xComponent;
    xComponent.set(m_xModuleManager, css::uno::UNO_QUERY);
    if (xComponent.is())
        xComponent->addEventListener(static_cast< css::lang::XEventListener* >(this));
    if (m_xDesktop.is())
        m_xDesktop->addEventListener(static_cast< css::lang::XEventListener* >(this));
    xComponent.set(m_xConfig, css::uno::UNO_QUERY);
    if (xComponent.is())
        xComponent->addEventListener(static_cast< css::lang::XEventListener* >(this));
}

HelpOnStartup::~HelpOnStartup()
{
}

// css.task.XJob
css::uno::Any SAL_CALL HelpOnStartup::execute(const css::uno::Sequence< css::beans::NamedValue >& lArguments)
{
    // Analyze the given arguments; try to locate a model there and
    // classify it's used application module.
    OUString sModule = its_getModuleIdFromEnv(lArguments);

    // Attention: we are bound to events for opening any document inside the office.
    // That includes e.g. the help module itself. But we have to do nothing then!
    if (sModule.isEmpty())
        return css::uno::Any();

    // check current state of the help module
    // a) help isn't open                       => show default page for the detected module
    // b) help shows any other default page(!) => show default page for the detected module
    // c) help shows any other content         => do nothing (user travelled to any other content and leaved the set of default pages)
    OUString sCurrentHelpURL                = its_getCurrentHelpURL();
    bool        bCurrentHelpURLIsAnyDefaultURL = its_isHelpUrlADefaultOne(sCurrentHelpURL);
    bool        bShowIt                        = false;

    // a)
    if (sCurrentHelpURL.isEmpty())
        bShowIt = true;
    // b)
    else if (bCurrentHelpURLIsAnyDefaultURL)
        bShowIt = true;

    if (bShowIt)
    {
        // retrieve the help URL for the detected application module
        OUString sModuleDependentHelpURL = its_checkIfHelpEnabledAndGetURL(sModule);
        if (!sModuleDependentHelpURL.isEmpty())
        {
            // Show this help page.
            // Note: The help window brings itself to front ...
            Help* pHelp = Application::GetHelp();
            if (pHelp)
                pHelp->Start(sModuleDependentHelpURL);
        }
    }

    return css::uno::Any();
}

void SAL_CALL HelpOnStartup::disposing(const css::lang::EventObject& aEvent)
{
    osl::MutexGuard g(m_mutex);
    if (aEvent.Source == m_xModuleManager)
        m_xModuleManager.clear();
    else if (aEvent.Source == m_xDesktop)
        m_xDesktop.clear();
    else if (aEvent.Source == m_xConfig)
        m_xConfig.clear();
}

OUString HelpOnStartup::its_getModuleIdFromEnv(const css::uno::Sequence< css::beans::NamedValue >& lArguments)
{
    ::comphelper::SequenceAsHashMap lArgs        (lArguments);
    ::comphelper::SequenceAsHashMap lEnvironment = lArgs.getUnpackedValueOrDefault("Environment", css::uno::Sequence< css::beans::NamedValue >());

    // check for right environment.
    // If it's not a DocumentEvent, which triggered this job,
    // we can't work correctly! => return immediately and do nothing
    OUString sEnvType = lEnvironment.getUnpackedValueOrDefault("EnvType", OUString());
    if (sEnvType != "DOCUMENTEVENT")
        return OUString();

    css::uno::Reference< css::frame::XModel > xDoc = lEnvironment.getUnpackedValueOrDefault("Model", css::uno::Reference< css::frame::XModel >());
    if (!xDoc.is())
        return OUString();

    // be sure that we work on top level documents only, which are registered
    // on the desktop instance. Ignore e.g. life previews, which are top frames too ...
    // but not registered at this global desktop instance.
    css::uno::Reference< css::frame::XDesktop >    xDesktopCheck;
    css::uno::Reference< css::frame::XFrame >      xFrame;
    css::uno::Reference< css::frame::XController > xController  = xDoc->getCurrentController();
    if (xController.is())
        xFrame = xController->getFrame();
    if (xFrame.is() && xFrame->isTop())
        xDesktopCheck.set(xFrame->getCreator(), css::uno::UNO_QUERY);
    if (!xDesktopCheck.is())
        return OUString();

    // OK - now we are sure this document is a top level document.
    // Classify it.
    // SAFE ->
    osl::ClearableMutexGuard aLock(m_mutex);
    css::uno::Reference< css::frame::XModuleManager2 > xModuleManager = m_xModuleManager;
    aLock.clear();
    // <- SAFE

    OUString sModuleId;
    try
    {
        sModuleId = xModuleManager->identify(xDoc);
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        { sModuleId.clear(); }

    return sModuleId;
}

OUString HelpOnStartup::its_getCurrentHelpURL()
{
    // SAFE ->
    osl::ClearableMutexGuard aLock(m_mutex);
    css::uno::Reference< css::frame::XDesktop2 > xDesktop = m_xDesktop;
    aLock.clear();
    // <- SAFE

    if (!xDesktop.is())
        return OUString();

    css::uno::Reference< css::frame::XFrame > xHelp = xDesktop->findFrame(SPECIALTARGET_HELPTASK, css::frame::FrameSearchFlag::CHILDREN);
    if (!xHelp.is())
        return OUString();

    OUString sCurrentHelpURL;
    try
    {
        css::uno::Reference< css::frame::XFramesSupplier >  xHelpRoot  (xHelp                 , css::uno::UNO_QUERY_THROW);
        css::uno::Reference< css::container::XIndexAccess > xHelpChildren(xHelpRoot->getFrames(), css::uno::UNO_QUERY_THROW);

        css::uno::Reference< css::frame::XFrame >      xHelpChild;
        css::uno::Reference< css::frame::XController > xHelpView;
        css::uno::Reference< css::frame::XModel >      xHelpContent;

        xHelpChildren->getByIndex(0) >>= xHelpChild;
        if (xHelpChild.is())
            xHelpView = xHelpChild->getController();
        if (xHelpView.is())
            xHelpContent = xHelpView->getModel();
        if (xHelpContent.is())
            sCurrentHelpURL = xHelpContent->getURL();
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        { sCurrentHelpURL.clear(); }

    return sCurrentHelpURL;
}

bool HelpOnStartup::its_isHelpUrlADefaultOne(std::u16string_view sHelpURL)
{
    if (sHelpURL.empty())
        return false;

    // SAFE ->
    osl::ClearableMutexGuard aLock(m_mutex);
    css::uno::Reference< css::container::XNameAccess >     xConfig = m_xConfig;
    OUString                                        sLocale = m_sLocale;
    OUString                                        sSystem = m_sSystem;
    aLock.clear();
    // <- SAFE

    if (!xConfig.is())
        return false;

    // check given help url against all default ones
    const css::uno::Sequence< OUString > lModules = xConfig->getElementNames();
    const OUString*                      pModules = lModules.getConstArray();
    ::sal_Int32                          c        = lModules.getLength();
    ::sal_Int32                          i        = 0;

    for (i=0; i<c; ++i)
    {
        try
        {
            css::uno::Reference< css::container::XNameAccess > xModuleConfig;
            xConfig->getByName(pModules[i]) >>= xModuleConfig;
            if (!xModuleConfig.is())
                continue;

            OUString sHelpBaseURL;
            xModuleConfig->getByName("ooSetupFactoryHelpBaseURL") >>= sHelpBaseURL;
            OUString sHelpURLForModule = HelpOnStartup::ist_createHelpURL(sHelpBaseURL, sLocale, sSystem);
            if (sHelpURL == sHelpURLForModule)
                return true;
        }
        catch(const css::uno::RuntimeException&)
            { throw; }
        catch(const css::uno::Exception&)
            {}
    }

    return false;
}

OUString HelpOnStartup::its_checkIfHelpEnabledAndGetURL(const OUString& sModule)
{
    // SAFE ->
    osl::ClearableMutexGuard aLock(m_mutex);
    css::uno::Reference< css::container::XNameAccess > xConfig = m_xConfig;
    OUString                                    sLocale = m_sLocale;
    OUString                                    sSystem = m_sSystem;
    aLock.clear();
    // <- SAFE

    OUString sHelpURL;

    try
    {
        css::uno::Reference< css::container::XNameAccess > xModuleConfig;
        if (xConfig.is())
            xConfig->getByName(sModule) >>= xModuleConfig;

        bool bHelpEnabled = false;
        if (xModuleConfig.is())
            xModuleConfig->getByName("ooSetupFactoryHelpOnOpen") >>= bHelpEnabled;

        if (bHelpEnabled)
        {
            OUString sHelpBaseURL;
            xModuleConfig->getByName("ooSetupFactoryHelpBaseURL") >>= sHelpBaseURL;
            sHelpURL = HelpOnStartup::ist_createHelpURL(sHelpBaseURL, sLocale, sSystem);
        }
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        { sHelpURL.clear(); }

    return sHelpURL;
}

OUString HelpOnStartup::ist_createHelpURL(std::u16string_view sBaseURL,
                                                 std::u16string_view sLocale ,
                                                 std::u16string_view sSystem )
{
    return OUString::Concat(sBaseURL) + "?Language=" + sLocale + "&System=" + sSystem;
}

} // namespace framework

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
framework_HelpOnStartup_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(new framework::HelpOnStartup(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
