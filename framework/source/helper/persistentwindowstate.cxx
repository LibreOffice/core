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

#include <pattern/window.hxx>
#include <helper/persistentwindowstate.hxx>
#include <threadhelp/writeguard.hxx>
#include <threadhelp/readguard.hxx>
#include <macros/generic.hxx>
#include <services.h>

#include <com/sun/star/awt/XWindow.hpp>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/configurationhelper.hxx>
#include <vcl/window.hxx>
#include <vcl/syswin.hxx>

#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <rtl/string.hxx>


namespace framework{



PersistentWindowState::PersistentWindowState(const css::uno::Reference< css::uno::XComponentContext >& xContext)
    : ThreadHelpBase          (&Application::GetSolarMutex())
    , m_xContext              (xContext                     )
    , m_bWindowStateAlreadySet(sal_False                    )
{
}


PersistentWindowState::~PersistentWindowState()
{
}


void SAL_CALL PersistentWindowState::initialize(const css::uno::Sequence< css::uno::Any >& lArguments)
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
    
    css::uno::Reference< css::frame::XFrame > xFrame;
    if (lArguments.getLength() < 1)
        throw css::lang::IllegalArgumentException(
                "Empty argument list!",
                static_cast< ::cppu::OWeakObject* >(this),
                1);

    lArguments[0] >>= xFrame;
    if (!xFrame.is())
        throw css::lang::IllegalArgumentException(
                "No valid frame specified!",
                static_cast< ::cppu::OWeakObject* >(this),
                1);

    
    WriteGuard aWriteLock(m_aLock);
    
    m_xFrame = xFrame;
    aWriteLock.unlock();
    

    
    xFrame->addFrameActionListener(this);
}


void SAL_CALL PersistentWindowState::frameAction(const css::frame::FrameActionEvent& aEvent)
    throw(css::uno::RuntimeException)
{
    
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::uno::XComponentContext >     xContext = m_xContext;
    css::uno::Reference< css::frame::XFrame >              xFrame(m_xFrame.get(), css::uno::UNO_QUERY);
    sal_Bool                                               bRestoreWindowState = !m_bWindowStateAlreadySet;
    aReadLock.unlock();
    

    
    if (!xFrame.is())
        return;

    
    css::uno::Reference< css::awt::XWindow > xWindow = xFrame->getContainerWindow();
    if (!xWindow.is())
        return;

    
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
                    
                    WriteGuard aWriteLock(m_aLock);
                    m_bWindowStateAlreadySet = sal_True;
                    aWriteLock.unlock();
                    
                }
            }
            break;

        case css::frame::FrameAction_COMPONENT_REATTACHED :
            {
                
                
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
    throw(css::uno::RuntimeException)
{
    
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
        { sModuleName = OUString(); }

    return sModuleName;
}


OUString PersistentWindowState::implst_getWindowStateFromConfig(const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                                                                       const OUString&                                    sModuleName)
{
    OUString sWindowState;

    OUStringBuffer sRelPathBuf(256);
    sRelPathBuf.appendAscii("Office/Factories/*[\"");
    sRelPathBuf.append     (sModuleName            );
    sRelPathBuf.appendAscii("\"]"                  );

    OUString sPackage("org.openoffice.Setup/");
    OUString sRelPath = sRelPathBuf.makeStringAndClear();
    OUString sKey("ooSetupFactoryWindowAttributes");

    try
    {
        ::comphelper::ConfigurationHelper::readDirectKey(rxContext,
                                                                                      sPackage,
                                                                                      sRelPath,
                                                                                      sKey,
                                                                                      ::comphelper::ConfigurationHelper::E_READONLY) >>= sWindowState;
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        { sWindowState = OUString(); }

    return sWindowState;
}


void PersistentWindowState::implst_setWindowStateOnConfig(const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                                                          const OUString&                                    sModuleName ,
                                                          const OUString&                                    sWindowState)
{
    OUStringBuffer sRelPathBuf(256);
    sRelPathBuf.appendAscii("Office/Factories/*[\"");
    sRelPathBuf.append     (sModuleName            );
    sRelPathBuf.appendAscii("\"]"                  );

    OUString sPackage("org.openoffice.Setup/");
    OUString sRelPath = sRelPathBuf.makeStringAndClear();
    OUString sKey("ooSetupFactoryWindowAttributes");

    try
    {
        ::comphelper::ConfigurationHelper::writeDirectKey(rxContext,
                                                          sPackage,
                                                          sRelPath,
                                                          sKey,
                                                          css::uno::makeAny(sWindowState),
                                                          ::comphelper::ConfigurationHelper::E_STANDARD);
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
        
        SolarMutexGuard aSolarGuard;

        Window* pWindow = VCLUnoHelper::GetWindow(xWindow);
        
        if (
            (pWindow                  ) &&
            (pWindow->IsSystemWindow())
           )
        {
            sal_uLong nMask  =   WINDOWSTATE_MASK_ALL;
                  nMask &= ~(WINDOWSTATE_MASK_MINIMIZED);
            sWindowState = OStringToOUString(
                            ((SystemWindow*)pWindow)->GetWindowState(nMask),
                            RTL_TEXTENCODING_UTF8);
        }
        
    }

    return sWindowState;
}



void PersistentWindowState::implst_setWindowStateOnWindow(const css::uno::Reference< css::awt::XWindow >& xWindow     ,
                                                          const OUString&                          sWindowState)
{
    if (
        (!xWindow.is()                ) ||
        ( sWindowState.isEmpty() )
       )
        return;

    
    SolarMutexGuard aSolarGuard;

    Window* pWindow = VCLUnoHelper::GetWindow(xWindow);
    if (!pWindow)
        return;

    
    sal_Bool bSystemWindow = pWindow->IsSystemWindow();
    sal_Bool bWorkWindow   = (pWindow->GetType() == WINDOW_WORKWINDOW);

    if (!bSystemWindow && !bWorkWindow)
        return;

    SystemWindow* pSystemWindow = (SystemWindow*)pWindow;
    WorkWindow*   pWorkWindow   = (WorkWindow*  )pWindow;

    
    if (pWorkWindow->IsMinimized())
        return;

    OUString sOldWindowState = OStringToOUString( pSystemWindow->GetWindowState(), RTL_TEXTENCODING_ASCII_US );
    if ( sOldWindowState != sWindowState )
        pSystemWindow->SetWindowState(OUStringToOString(sWindowState,RTL_TEXTENCODING_UTF8));
    
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
