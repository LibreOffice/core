/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


//_______________________________________________
// include own header
#include <jobs/helponstartup.hxx>
#include <threadhelp/resetableguard.hxx>
#include <loadenv/targethelper.hxx>
#include <services.h>

//_______________________________________________
// include others
#include <comphelper/configurationhelper.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <unotools/configmgr.hxx>
#include <vcl/svapp.hxx>
#include <vcl/help.hxx>
#include <rtl/ustrbuf.hxx>

//_______________________________________________
// include interfaces
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/frame/XDesktop.hpp>


namespace framework{


// path to module config
static ::rtl::OUString CFG_PACKAGE_MODULES     ("/org.openoffice.Setup/Office/Factories");
static ::rtl::OUString CFG_PACKAGE_SETUP       ("/org.openoffice.Setup");
static ::rtl::OUString CFG_PACKAGE_COMMON      ("/org.openoffice.Office.Common");
static ::rtl::OUString CFG_PATH_L10N           ("L10N");
static ::rtl::OUString CFG_PATH_HELP           ("Help");
static ::rtl::OUString CFG_KEY_LOCALE          ("ooLocale");
static ::rtl::OUString CFG_KEY_HELPSYSTEM      ("System");

// props of job environment
static ::rtl::OUString PROP_ENVIRONMENT        ("Environment");
static ::rtl::OUString PROP_JOBCONFIG          ("JobConfig");
static ::rtl::OUString PROP_ENVTYPE            ("EnvType");
static ::rtl::OUString PROP_MODEL              ("Model");

// props of module config
static ::rtl::OUString PROP_HELP_BASEURL       ("ooSetupFactoryHelpBaseURL");
static ::rtl::OUString PROP_AUTOMATIC_HELP     ("ooSetupFactoryHelpOnOpen");

// special value of job environment
static ::rtl::OUString ENVTYPE_DOCUMENTEVENT   ("DOCUMENTEVENT");

//-----------------------------------------------

DEFINE_XSERVICEINFO_MULTISERVICE(HelpOnStartup                   ,
                                      ::cppu::OWeakObject             ,
                                      SERVICENAME_JOB                 ,
                                      IMPLEMENTATIONNAME_HELPONSTARTUP)

DEFINE_INIT_SERVICE(HelpOnStartup,
                    {
                        /*  Attention
                            I think we don't need any mutex or lock here ... because we are called by our own static method impl_createInstance()
                            to create a new instance of this class by our own supported service factory.
                            see macro DEFINE_XSERVICEINFO_MULTISERVICE and "impl_initService()" for further informations!
                        */
                        // create some needed uno services and cache it
                        m_xModuleManager = css::uno::Reference< css::frame::XModuleManager >(
                            m_xSMGR->createInstance(SERVICENAME_MODULEMANAGER),
                            css::uno::UNO_QUERY_THROW);

                        m_xDesktop = css::uno::Reference< css::frame::XFrame >(
                            m_xSMGR->createInstance(SERVICENAME_DESKTOP),
                            css::uno::UNO_QUERY_THROW);

                        m_xConfig = css::uno::Reference< css::container::XNameAccess >(
                            ::comphelper::ConfigurationHelper::openConfig(
                                m_xSMGR,
                                CFG_PACKAGE_MODULES,
                                ::comphelper::ConfigurationHelper::E_READONLY),
                            css::uno::UNO_QUERY_THROW);

                        // ask for office locale
                        ::comphelper::ConfigurationHelper::readDirectKey(
                            m_xSMGR,
                            CFG_PACKAGE_SETUP,
                            CFG_PATH_L10N,
                            CFG_KEY_LOCALE,
                            ::comphelper::ConfigurationHelper::E_READONLY) >>= m_sLocale;

                        // detect system
                        ::comphelper::ConfigurationHelper::readDirectKey(
                            m_xSMGR,
                            CFG_PACKAGE_COMMON,
                            CFG_PATH_HELP,
                            CFG_KEY_HELPSYSTEM,
                            ::comphelper::ConfigurationHelper::E_READONLY) >>= m_sSystem;

                        // Start listening for disposing events of these services,
                        // so we can react e.g. for an office shutdown
                        css::uno::Reference< css::lang::XComponent > xComponent;
                        xComponent = css::uno::Reference< css::lang::XComponent >(m_xModuleManager, css::uno::UNO_QUERY);
                        if (xComponent.is())
                            xComponent->addEventListener(static_cast< css::lang::XEventListener* >(this));
                        xComponent = css::uno::Reference< css::lang::XComponent >(m_xDesktop, css::uno::UNO_QUERY);
                        if (xComponent.is())
                            xComponent->addEventListener(static_cast< css::lang::XEventListener* >(this));
                        xComponent = css::uno::Reference< css::lang::XComponent >(m_xConfig, css::uno::UNO_QUERY);
                        if (xComponent.is())
                            xComponent->addEventListener(static_cast< css::lang::XEventListener* >(this));
                    }
                   )

//-----------------------------------------------
HelpOnStartup::HelpOnStartup(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
    : ThreadHelpBase(     )
    , m_xSMGR       (xSMGR)
{
}

//-----------------------------------------------
HelpOnStartup::~HelpOnStartup()
{
}

//-----------------------------------------------
// css.task.XJob
css::uno::Any SAL_CALL HelpOnStartup::execute(const css::uno::Sequence< css::beans::NamedValue >& lArguments)
    throw(css::lang::IllegalArgumentException,
          css::uno::Exception                ,
          css::uno::RuntimeException         )
{
    // Analyze the given arguments; try to locate a model there and
    // classify it's used application module.
    ::rtl::OUString sModule = its_getModuleIdFromEnv(lArguments);

    // Attention: We are bound to events for openeing any document inside the office.
    // That includes e.g. the help module itself. But we have to do nothing then!
    if (sModule.isEmpty())
        return css::uno::Any();

    // check current state of the help module
    // a) help isnt open                       => show default page for the detected module
    // b) help shows any other default page(!) => show default page for the detected module
    // c) help shows any other content         => do nothing (user travelled to any other content and leaved the set of default pages)
    ::rtl::OUString sCurrentHelpURL                = its_getCurrentHelpURL();
    sal_Bool        bCurrentHelpURLIsAnyDefaultURL = its_isHelpUrlADefaultOne(sCurrentHelpURL);
    sal_Bool        bShowIt                        = sal_False;

    // a)
    if (sCurrentHelpURL.isEmpty())
        bShowIt = sal_True;
    else
    // b)
    if (bCurrentHelpURLIsAnyDefaultURL)
        bShowIt = sal_True;

    if (bShowIt)
    {
        // retrieve the help URL for the detected application module
        ::rtl::OUString sModuleDependendHelpURL = its_checkIfHelpEnabledAndGetURL(sModule);
        if (!sModuleDependendHelpURL.isEmpty())
        {
            // Show this help page.
            // Note: The help window brings itself to front ...
            Help* pHelp = Application::GetHelp();
            if (pHelp)
                pHelp->Start(sModuleDependendHelpURL, 0);
        }
    }

    return css::uno::Any();
}

//-----------------------------------------------
void SAL_CALL HelpOnStartup::disposing(const css::lang::EventObject& aEvent)
    throw(css::uno::RuntimeException)
{
    // SAFE ->
    ResetableGuard aLock(m_aLock);

    if (aEvent.Source == m_xModuleManager)
        m_xModuleManager.clear();
    else
    if (aEvent.Source == m_xDesktop)
        m_xDesktop.clear();
    else
    if (aEvent.Source == m_xConfig)
        m_xConfig.clear();

    aLock.unlock();
    // <- SAFE
}

//-----------------------------------------------
::rtl::OUString HelpOnStartup::its_getModuleIdFromEnv(const css::uno::Sequence< css::beans::NamedValue >& lArguments)
{
    ::comphelper::SequenceAsHashMap lArgs        (lArguments);
    ::comphelper::SequenceAsHashMap lEnvironment = lArgs.getUnpackedValueOrDefault(PROP_ENVIRONMENT, css::uno::Sequence< css::beans::NamedValue >());
    ::comphelper::SequenceAsHashMap lJobConfig   = lArgs.getUnpackedValueOrDefault(PROP_JOBCONFIG  , css::uno::Sequence< css::beans::NamedValue >());

    // check for right environment.
    // If its not a DocumentEvent, which triggered this job,
    // we cant work correctly! => return immediatly and do nothing
    ::rtl::OUString sEnvType = lEnvironment.getUnpackedValueOrDefault(PROP_ENVTYPE, ::rtl::OUString());
    if (!sEnvType.equals(ENVTYPE_DOCUMENTEVENT))
        return ::rtl::OUString();

    css::uno::Reference< css::frame::XModel > xDoc = lEnvironment.getUnpackedValueOrDefault(PROP_MODEL, css::uno::Reference< css::frame::XModel >());
    if (!xDoc.is())
        return ::rtl::OUString();

    // be sure that we work on top level documents only, which are registered
    // on the desktop instance. Ignore e.g. life previews, which are top frames too ...
    // but not registered at this global desktop instance.
    css::uno::Reference< css::frame::XDesktop >    xDesktopCheck;
    css::uno::Reference< css::frame::XFrame >      xFrame       ;
    css::uno::Reference< css::frame::XController > xController  = xDoc->getCurrentController();
    if (xController.is())
        xFrame = xController->getFrame();
    if (xFrame.is() && xFrame->isTop())
        xDesktopCheck = css::uno::Reference< css::frame::XDesktop >(xFrame->getCreator(), css::uno::UNO_QUERY);
    if (!xDesktopCheck.is())
        return ::rtl::OUString();

    // OK - now we are sure this document is a top level document.
    // Classify it.
    // SAFE ->
    ResetableGuard aLock(m_aLock);
    css::uno::Reference< css::frame::XModuleManager > xModuleManager = m_xModuleManager;
    aLock.unlock();
    // <- SAFE

    if (!xModuleManager.is())
        return ::rtl::OUString();

    ::rtl::OUString sModuleId;
    try
    {
        sModuleId = xModuleManager->identify(xDoc);
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        { sModuleId = ::rtl::OUString(); }

    return sModuleId;
}

//-----------------------------------------------
::rtl::OUString HelpOnStartup::its_getCurrentHelpURL()
{
    // SAFE ->
    ResetableGuard aLock(m_aLock);
    css::uno::Reference< css::frame::XFrame > xDesktop = m_xDesktop;
    aLock.unlock();
    // <- SAFE

    if (!xDesktop.is())
        return ::rtl::OUString();

    css::uno::Reference< css::frame::XFrame > xHelp = xDesktop->findFrame(SPECIALTARGET_HELPTASK, css::frame::FrameSearchFlag::CHILDREN);
    if (!xHelp.is())
        return ::rtl::OUString();

    ::rtl::OUString sCurrentHelpURL;
    try
    {
        css::uno::Reference< css::frame::XFramesSupplier >  xHelpRoot  (xHelp                 , css::uno::UNO_QUERY_THROW);
        css::uno::Reference< css::container::XIndexAccess > xHelpChildren(xHelpRoot->getFrames(), css::uno::UNO_QUERY_THROW);

        css::uno::Reference< css::frame::XFrame >      xHelpChild  ;
        css::uno::Reference< css::frame::XController > xHelpView   ;
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
        { sCurrentHelpURL = ::rtl::OUString(); }

    return sCurrentHelpURL;
}

//-----------------------------------------------
::sal_Bool HelpOnStartup::its_isHelpUrlADefaultOne(const ::rtl::OUString& sHelpURL)
{
    if (sHelpURL.isEmpty())
        return sal_False;

    // SAFE ->
    ResetableGuard aLock(m_aLock);
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR   (m_xSMGR, css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::container::XNameAccess >     xConfig = m_xConfig;
    ::rtl::OUString                                        sLocale = m_sLocale;
    ::rtl::OUString                                        sSystem = m_sSystem;
    aLock.unlock();
    // <- SAFE

    if (!xConfig.is())
        return sal_False;

    // check given help url against all default ones
    const css::uno::Sequence< ::rtl::OUString > lModules = xConfig->getElementNames();
    const ::rtl::OUString*                      pModules = lModules.getConstArray();
          ::sal_Int32                           c        = lModules.getLength();
          ::sal_Int32                           i        = 0;

    for (i=0; i<c; ++i)
    {
        try
        {
            css::uno::Reference< css::container::XNameAccess > xModuleConfig;
            xConfig->getByName(pModules[i]) >>= xModuleConfig;
            if (!xModuleConfig.is())
                continue;

            ::rtl::OUString sHelpBaseURL;
            xModuleConfig->getByName(PROP_HELP_BASEURL) >>= sHelpBaseURL;
            ::rtl::OUString sHelpURLForModule = HelpOnStartup::ist_createHelpURL(sHelpBaseURL, sLocale, sSystem);
            if (sHelpURL.equals(sHelpURLForModule))
                return sal_True;
        }
        catch(const css::uno::RuntimeException&)
            { throw; }
        catch(const css::uno::Exception&)
            {}
    }

    return sal_False;
}

//-----------------------------------------------
::rtl::OUString HelpOnStartup::its_checkIfHelpEnabledAndGetURL(const ::rtl::OUString& sModule)
{
    // SAFE ->
    ResetableGuard aLock(m_aLock);
    css::uno::Reference< css::container::XNameAccess > xConfig = m_xConfig;
    ::rtl::OUString                                    sLocale = m_sLocale;
    ::rtl::OUString                                    sSystem = m_sSystem;
    aLock.unlock();
    // <- SAFE

    ::rtl::OUString sHelpURL;

    try
    {
        css::uno::Reference< css::container::XNameAccess > xModuleConfig;
        if (xConfig.is())
            xConfig->getByName(sModule) >>= xModuleConfig;

        sal_Bool bHelpEnabled = sal_False;
        if (xModuleConfig.is())
            xModuleConfig->getByName(PROP_AUTOMATIC_HELP) >>= bHelpEnabled;

        if (bHelpEnabled)
        {
            ::rtl::OUString sHelpBaseURL;
            xModuleConfig->getByName(PROP_HELP_BASEURL) >>= sHelpBaseURL;
            sHelpURL = HelpOnStartup::ist_createHelpURL(sHelpBaseURL, sLocale, sSystem);
        }
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        { sHelpURL = ::rtl::OUString(); }

    return sHelpURL;
}

//-----------------------------------------------
::rtl::OUString HelpOnStartup::ist_createHelpURL(const ::rtl::OUString& sBaseURL,
                                                 const ::rtl::OUString& sLocale ,
                                                 const ::rtl::OUString& sSystem )
{
    ::rtl::OUStringBuffer sHelpURL(256);
    sHelpURL.append     (sBaseURL    );
    sHelpURL.appendAscii("?Language=");
    sHelpURL.append     (sLocale     );
    sHelpURL.appendAscii("&System="  );
    sHelpURL.append     (sSystem     );

    return sHelpURL.makeStringAndClear();
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
