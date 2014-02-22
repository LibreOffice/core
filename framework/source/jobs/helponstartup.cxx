/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */




#include <jobs/helponstartup.hxx>
#include <threadhelp/resetableguard.hxx>
#include <loadenv/targethelper.hxx>
#include <services.h>



#include <comphelper/configurationhelper.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <unotools/configmgr.hxx>
#include <vcl/svapp.hxx>
#include <vcl/help.hxx>
#include <rtl/ustrbuf.hxx>



#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/frame/Desktop.hpp>


namespace framework{

DEFINE_XSERVICEINFO_MULTISERVICE_2(HelpOnStartup                   ,
                                      ::cppu::OWeakObject             ,
                                      SERVICENAME_JOB                 ,
                                      IMPLEMENTATIONNAME_HELPONSTARTUP)

DEFINE_INIT_SERVICE(HelpOnStartup,
                    {
                        /*  Attention
                            I think we don't need any mutex or lock here ... because we are called by our own static method impl_createInstance()
                            to create a new instance of this class by our own supported service factory.
                            see macro DEFINE_XSERVICEINFO_MULTISERVICE and "impl_initService()" for further information!
                        */
                        
                        m_xModuleManager = css::frame::ModuleManager::create( m_xContext );

                        m_xDesktop = css::frame::Desktop::create(m_xContext);

                        m_xConfig = css::uno::Reference< css::container::XNameAccess >(
                            ::comphelper::ConfigurationHelper::openConfig(
                                m_xContext,
                                "/org.openoffice.Setup/Office/Factories",
                                ::comphelper::ConfigurationHelper::E_READONLY),
                            css::uno::UNO_QUERY_THROW);

                        
                        ::comphelper::ConfigurationHelper::readDirectKey(
                            m_xContext,
                            "/org.openoffice.Setup",
                            "L10N",
                            "ooLocale",
                            ::comphelper::ConfigurationHelper::E_READONLY) >>= m_sLocale;

                        
                        ::comphelper::ConfigurationHelper::readDirectKey(
                            m_xContext,
                            "/org.openoffice.Office.Common",
                            "Help",
                            "System",
                            ::comphelper::ConfigurationHelper::E_READONLY) >>= m_sSystem;

                        
                        
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


HelpOnStartup::HelpOnStartup(const css::uno::Reference< css::uno::XComponentContext >& xContext)
    : ThreadHelpBase(     )
    , m_xContext    (xContext)
{
}


HelpOnStartup::~HelpOnStartup()
{
}



css::uno::Any SAL_CALL HelpOnStartup::execute(const css::uno::Sequence< css::beans::NamedValue >& lArguments)
    throw(css::lang::IllegalArgumentException,
          css::uno::Exception                ,
          css::uno::RuntimeException         )
{
    
    
    OUString sModule = its_getModuleIdFromEnv(lArguments);

    
    
    if (sModule.isEmpty())
        return css::uno::Any();

    
    
    
    
    OUString sCurrentHelpURL                = its_getCurrentHelpURL();
    sal_Bool        bCurrentHelpURLIsAnyDefaultURL = its_isHelpUrlADefaultOne(sCurrentHelpURL);
    sal_Bool        bShowIt                        = sal_False;

    
    if (sCurrentHelpURL.isEmpty())
        bShowIt = sal_True;
    
    else if (bCurrentHelpURLIsAnyDefaultURL)
        bShowIt = sal_True;

    if (bShowIt)
    {
        
        OUString sModuleDependendHelpURL = its_checkIfHelpEnabledAndGetURL(sModule);
        if (!sModuleDependendHelpURL.isEmpty())
        {
            
            
            Help* pHelp = Application::GetHelp();
            if (pHelp)
                pHelp->Start(sModuleDependendHelpURL, 0);
        }
    }

    return css::uno::Any();
}


void SAL_CALL HelpOnStartup::disposing(const css::lang::EventObject& aEvent)
    throw(css::uno::RuntimeException)
{
    
    ResetableGuard aLock(m_aLock);

    if (aEvent.Source == m_xModuleManager)
        m_xModuleManager.clear();
    else if (aEvent.Source == m_xDesktop)
        m_xDesktop.clear();
    else if (aEvent.Source == m_xConfig)
        m_xConfig.clear();

    aLock.unlock();
    
}


OUString HelpOnStartup::its_getModuleIdFromEnv(const css::uno::Sequence< css::beans::NamedValue >& lArguments)
{
    ::comphelper::SequenceAsHashMap lArgs        (lArguments);
    ::comphelper::SequenceAsHashMap lEnvironment = lArgs.getUnpackedValueOrDefault("Environment", css::uno::Sequence< css::beans::NamedValue >());
    ::comphelper::SequenceAsHashMap lJobConfig   = lArgs.getUnpackedValueOrDefault("JobConfig", css::uno::Sequence< css::beans::NamedValue >());

    
    
    
    OUString sEnvType = lEnvironment.getUnpackedValueOrDefault("EnvType", OUString());
    if (sEnvType != "DOCUMENTEVENT")
        return OUString();

    css::uno::Reference< css::frame::XModel > xDoc = lEnvironment.getUnpackedValueOrDefault("Model", css::uno::Reference< css::frame::XModel >());
    if (!xDoc.is())
        return OUString();

    
    
    
    css::uno::Reference< css::frame::XDesktop >    xDesktopCheck;
    css::uno::Reference< css::frame::XFrame >      xFrame       ;
    css::uno::Reference< css::frame::XController > xController  = xDoc->getCurrentController();
    if (xController.is())
        xFrame = xController->getFrame();
    if (xFrame.is() && xFrame->isTop())
        xDesktopCheck = css::uno::Reference< css::frame::XDesktop >(xFrame->getCreator(), css::uno::UNO_QUERY);
    if (!xDesktopCheck.is())
        return OUString();

    
    
    
    ResetableGuard aLock(m_aLock);
    css::uno::Reference< css::frame::XModuleManager2 > xModuleManager = m_xModuleManager;
    aLock.unlock();
    

    OUString sModuleId;
    try
    {
        sModuleId = xModuleManager->identify(xDoc);
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        { sModuleId = OUString(); }

    return sModuleId;
}


OUString HelpOnStartup::its_getCurrentHelpURL()
{
    
    ResetableGuard aLock(m_aLock);
    css::uno::Reference< css::frame::XDesktop2 > xDesktop = m_xDesktop;
    aLock.unlock();
    

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
        { sCurrentHelpURL = OUString(); }

    return sCurrentHelpURL;
}


::sal_Bool HelpOnStartup::its_isHelpUrlADefaultOne(const OUString& sHelpURL)
{
    if (sHelpURL.isEmpty())
        return sal_False;

    
    ResetableGuard aLock(m_aLock);
    css::uno::Reference< css::container::XNameAccess >     xConfig = m_xConfig;
    OUString                                        sLocale = m_sLocale;
    OUString                                        sSystem = m_sSystem;
    aLock.unlock();
    

    if (!xConfig.is())
        return sal_False;

    
    const css::uno::Sequence< OUString > lModules = xConfig->getElementNames();
    const OUString*                      pModules = lModules.getConstArray();
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

            OUString sHelpBaseURL;
            xModuleConfig->getByName("ooSetupFactoryHelpBaseURL") >>= sHelpBaseURL;
            OUString sHelpURLForModule = HelpOnStartup::ist_createHelpURL(sHelpBaseURL, sLocale, sSystem);
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


OUString HelpOnStartup::its_checkIfHelpEnabledAndGetURL(const OUString& sModule)
{
    
    ResetableGuard aLock(m_aLock);
    css::uno::Reference< css::container::XNameAccess > xConfig = m_xConfig;
    OUString                                    sLocale = m_sLocale;
    OUString                                    sSystem = m_sSystem;
    aLock.unlock();
    

    OUString sHelpURL;

    try
    {
        css::uno::Reference< css::container::XNameAccess > xModuleConfig;
        if (xConfig.is())
            xConfig->getByName(sModule) >>= xModuleConfig;

        sal_Bool bHelpEnabled = sal_False;
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
        { sHelpURL = OUString(); }

    return sHelpURL;
}


OUString HelpOnStartup::ist_createHelpURL(const OUString& sBaseURL,
                                                 const OUString& sLocale ,
                                                 const OUString& sSystem )
{
    OUStringBuffer sHelpURL(256);
    sHelpURL.append     (sBaseURL    );
    sHelpURL.appendAscii("?Language=");
    sHelpURL.append     (sLocale     );
    sHelpURL.appendAscii("&System="  );
    sHelpURL.append     (sSystem     );

    return sHelpURL.makeStringAndClear();
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
