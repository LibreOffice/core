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

#include <dispatch/closedispatcher.hxx>
#include <pattern/frame.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>
#include <framework/framelistanalyzer.hxx>
#include <services.h>
#include <general.h>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/CommandGroup.hpp>
#include <com/sun/star/frame/StartModule.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/document/XActionLockable.hpp>
#include "com/sun/star/beans/XFastPropertySet.hpp"
#include <toolkit/helper/vclunohelper.hxx>

#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include "vcl/syswin.hxx"
#include <osl/mutex.hxx>
#include <unotools/moduleoptions.hxx>
#include <comphelper/processfactory.hxx>


using namespace com::sun::star;

namespace framework{

#ifdef fpf
    #error "Who uses \"fpf\" as define. It will overwrite my namespace alias ..."
#endif
namespace fpf = ::framework::pattern::frame;

const char URL_CLOSEDOC[] = ".uno:CloseDoc";
const char URL_CLOSEWIN[] = ".uno:CloseWin";
const char URL_CLOSEFRAME[] = ".uno:CloseFrame";



CloseDispatcher::CloseDispatcher(const css::uno::Reference< css::uno::XComponentContext >& rxContext ,
                                 const css::uno::Reference< css::frame::XFrame >&          xFrame ,
                                 const OUString&                                           sTarget)
    : ThreadHelpBase     (&Application::GetSolarMutex()                   )
    , m_xContext         (rxContext                                       )
    , m_aAsyncCallback   (LINK( this, CloseDispatcher, impl_asyncCallback))
    , m_eOperation(E_CLOSE_DOC)
    , m_lStatusListener  (m_aLock.getShareableOslMutex()                  )
    , m_pSysWindow(NULL)
{
    uno::Reference<frame::XFrame> xTarget = static_impl_searchRightTargetFrame(xFrame, sTarget);
    m_xCloseFrame = xTarget;

    
    uno::Reference<awt::XWindow> xWindow = xTarget->getContainerWindow();
    if (xWindow.is())
    {
        Window* pWindow = VCLUnoHelper::GetWindow(xWindow);
        if (pWindow->IsSystemWindow())
            m_pSysWindow = dynamic_cast<SystemWindow*>(pWindow);
    }
}


CloseDispatcher::~CloseDispatcher()
{
}


void SAL_CALL CloseDispatcher::dispatch(const css::util::URL&                                  aURL      ,
                                        const css::uno::Sequence< css::beans::PropertyValue >& lArguments)
    throw(css::uno::RuntimeException)
{
    dispatchWithNotification(aURL, lArguments, css::uno::Reference< css::frame::XDispatchResultListener >());
}


css::uno::Sequence< sal_Int16 > SAL_CALL CloseDispatcher::getSupportedCommandGroups()
    throw(css::uno::RuntimeException)
{
    css::uno::Sequence< sal_Int16 > lGroups(2);
    lGroups[0] = css::frame::CommandGroup::VIEW;
    lGroups[1] = css::frame::CommandGroup::DOCUMENT;
    return lGroups;
}


css::uno::Sequence< css::frame::DispatchInformation > SAL_CALL CloseDispatcher::getConfigurableDispatchInformation(sal_Int16 nCommandGroup)
    throw(css::uno::RuntimeException)
{
    if (nCommandGroup == css::frame::CommandGroup::VIEW)
    {
        /* Attention: Dont add .uno:CloseFrame here. Because its not really
                      a configurable feature ... and further it does not have
                      a valid UIName entry inside the GenericCommands.xcu ... */
        css::uno::Sequence< css::frame::DispatchInformation > lViewInfos(1);
        lViewInfos[0].Command = OUString(URL_CLOSEWIN);
        lViewInfos[0].GroupId = css::frame::CommandGroup::VIEW;
        return lViewInfos;
    }
    else if (nCommandGroup == css::frame::CommandGroup::DOCUMENT)
    {
        css::uno::Sequence< css::frame::DispatchInformation > lDocInfos(1);
        lDocInfos[0].Command = OUString(URL_CLOSEDOC);
        lDocInfos[0].GroupId = css::frame::CommandGroup::DOCUMENT;
        return lDocInfos;
    }

    return css::uno::Sequence< css::frame::DispatchInformation >();
}


void SAL_CALL CloseDispatcher::addStatusListener(const css::uno::Reference< css::frame::XStatusListener >& /*xListener*/,
                                                 const css::util::URL&                                     /*aURL*/     )
    throw(css::uno::RuntimeException)
{
}


void SAL_CALL CloseDispatcher::removeStatusListener(const css::uno::Reference< css::frame::XStatusListener >& /*xListener*/,
                                                    const css::util::URL&                                     /*aURL*/     )
    throw(css::uno::RuntimeException)
{
}


void SAL_CALL CloseDispatcher::dispatchWithNotification(const css::util::URL&                                             aURL      ,
                                                        const css::uno::Sequence< css::beans::PropertyValue >&            lArguments,
                                                        const css::uno::Reference< css::frame::XDispatchResultListener >& xListener )
    throw(css::uno::RuntimeException)
{
    
    WriteGuard aWriteLock(m_aLock);

    
    
    
    
    
    
    if (m_xSelfHold.is())
    {
        aWriteLock.unlock();
        

        implts_notifyResultListener(
            xListener,
            css::frame::DispatchResultState::DONTKNOW,
            css::uno::Any());
        return;
    }

    
    
    
    
    if ( aURL.Complete == URL_CLOSEDOC )
        m_eOperation = E_CLOSE_DOC;
    else if ( aURL.Complete == URL_CLOSEWIN )
        m_eOperation = E_CLOSE_WIN;
    else if ( aURL.Complete == URL_CLOSEFRAME )
        m_eOperation = E_CLOSE_FRAME;
    else
    {
        aWriteLock.unlock();
        

        implts_notifyResultListener(
            xListener,
            css::frame::DispatchResultState::FAILURE,
            css::uno::Any());
        return;
    }

    if (m_pSysWindow && m_pSysWindow->GetCloseHdl().IsSet())
    {
        
        m_pSysWindow->GetCloseHdl().Call(m_pSysWindow);
        return;
    }

    
    
    
    
    

    
    
    

    m_xResultListener = xListener;
    m_xSelfHold       = css::uno::Reference< css::uno::XInterface >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);

    aWriteLock.unlock();
    

    sal_Bool bIsSynchron = sal_False;
    for (sal_Int32 nArgs=0; nArgs<lArguments.getLength(); nArgs++ )
    {
        if ( lArguments[nArgs].Name == "SynchronMode" )
        {
            lArguments[nArgs].Value >>= bIsSynchron;
            break;
        }
    }

    if ( bIsSynchron )
        impl_asyncCallback(0);
    else
        m_aAsyncCallback.Post(0);
}


/**
    @short      asynchronous callback
    @descr      We start all actions inside this object asnychronoue.
                (see comments there).
                Now we do the following:
                - close all views to the same document, if needed and possible
                - make the current frame empty
                  ! This step is necessary to handle errors during closing the
                    document inside the frame. May the document shows a dialog and
                    the user ignore it. Then the state of the office can be changed
                    during we try to close frame and document.
                - check the environment (menas count open frames - exlcuding our
                  current one)
                - decide then, if we must close this frame only, establish the backing mode
                  or shutdown the whole application.
*/
IMPL_LINK_NOARG(CloseDispatcher, impl_asyncCallback)
{
    try
    {

    
    
    
    sal_Bool bAllowSuspend        = sal_True;
    sal_Bool bControllerSuspended = sal_False;

    
    ReadGuard aReadLock(m_aLock);

    
    
    sal_Bool bCloseAllViewsToo = (m_eOperation == E_CLOSE_DOC);

    
    EOperation                                                  eOperation  = m_eOperation;
    css::uno::Reference< css::uno::XComponentContext >          xContext    = m_xContext;
    css::uno::Reference< css::frame::XFrame >                   xCloseFrame (m_xCloseFrame.get(), css::uno::UNO_QUERY);
    css::uno::Reference< css::frame::XDispatchResultListener >  xListener   = m_xResultListener;

    aReadLock.unlock();
    

    
    
    if (! xCloseFrame.is())
        return 0;

    sal_Bool bCloseFrame           = sal_False;
    sal_Bool bEstablishBackingMode = sal_False;
    sal_Bool bTerminateApp         = sal_False;

    
    
    
    css::uno::Reference< css::frame::XFramesSupplier > xDesktop( css::frame::Desktop::create(xContext), css::uno::UNO_QUERY_THROW);
    FrameListAnalyzer aCheck1(xDesktop, xCloseFrame, FrameListAnalyzer::E_HELP | FrameListAnalyzer::E_BACKINGCOMPONENT);

    
    
    
    
    
    
    if ( ! xCloseFrame->getCreator().is())
        bCloseFrame = sal_True;

    
    
    
    
    
    else if (aCheck1.m_bReferenceIsHelp)
        bCloseFrame = sal_True;

    
    
    
    
    
    else if (aCheck1.m_bReferenceIsBacking)
        bTerminateApp = sal_True;

    
    
    else
    {
        if (implts_prepareFrameForClosing(m_xCloseFrame, bAllowSuspend, bCloseAllViewsToo, bControllerSuspended))
        {
            
            
            FrameListAnalyzer aCheck2(xDesktop, xCloseFrame, FrameListAnalyzer::E_ALL);

            
            
            
            if (aCheck2.m_lOtherVisibleFrames.getLength()>0)
                bCloseFrame = sal_True;
            else

            
            
            
            
            if (
                (!bCloseAllViewsToo                    ) &&
                (aCheck2.m_lModelFrames.getLength() > 0)
               )
                bCloseFrame = sal_True;

            else
            
            
            
            
            
            {
                if (eOperation == E_CLOSE_FRAME)
                    bTerminateApp = sal_True;
                else if( SvtModuleOptions().IsModuleInstalled(SvtModuleOptions::E_SSTARTMODULE) )
                    bEstablishBackingMode = sal_True;
                else
                    bTerminateApp = sal_True;
            }
        }
    }

    
    sal_Bool bSuccess = sal_False;
    if (bCloseFrame)
        bSuccess = implts_closeFrame();
    else if (bEstablishBackingMode)
    #if defined MACOSX
    {
        
        
        

        bool bQuickstarterRunning = false;
        
        try
        {
            css::uno::Reference< css::beans::XFastPropertySet > xSet( xContext->getServiceManager()->createInstanceWithContext(IMPLEMENTATIONNAME_QUICKLAUNCHER, xContext), css::uno::UNO_QUERY_THROW );
            if( xSet.is() )
            {
                css::uno::Any aVal( xSet->getFastPropertyValue( 0 ) );
                sal_Bool bState = sal_False;
                if( aVal >>= bState )
                    bQuickstarterRunning = bState;
            }
        }
        catch( const css::uno::Exception& )
        {
        }
        bSuccess = bQuickstarterRunning ? implts_terminateApplication() : implts_establishBackingMode();
    }
    #else
        bSuccess = implts_establishBackingMode();
    #endif
    else if (bTerminateApp)
        bSuccess = implts_terminateApplication();

    if (
        ( ! bSuccess             ) &&
        (   bControllerSuspended )
       )
    {
        css::uno::Reference< css::frame::XController > xController = xCloseFrame->getController();
        if (xController.is())
            xController->suspend(sal_False);
    }

    
    sal_Int16 nState = css::frame::DispatchResultState::FAILURE;
    if (bSuccess)
        nState = css::frame::DispatchResultState::SUCCESS;
    implts_notifyResultListener(xListener, nState, css::uno::Any());

    
    WriteGuard aWriteLock(m_aLock);

    
    
    
    
    
    css::uno::Reference< css::uno::XInterface > xTempHold = m_xSelfHold;
    m_xSelfHold.clear();
    m_xResultListener.clear();

    aWriteLock.unlock();
    

    }
    catch(const css::lang::DisposedException&)
    {
    }

    return 0;
}


sal_Bool CloseDispatcher::implts_prepareFrameForClosing(const css::uno::Reference< css::frame::XFrame >& xFrame                ,
                                                              sal_Bool                                   bAllowSuspend         ,
                                                              sal_Bool                                   bCloseAllOtherViewsToo,
                                                              sal_Bool&                                  bControllerSuspended  )
{
    
    if (! xFrame.is())
        return sal_True;

    
    
    
    
    if (bCloseAllOtherViewsToo)
    {
        
        ReadGuard aReadLock(m_aLock);
        css::uno::Reference< css::uno::XComponentContext > xContext  = m_xContext;
        aReadLock.unlock();
        

        css::uno::Reference< css::frame::XFramesSupplier > xDesktop( css::frame::Desktop::create( xContext ), css::uno::UNO_QUERY_THROW);
        FrameListAnalyzer aCheck(xDesktop, xFrame, FrameListAnalyzer::E_ALL);

        sal_Int32 c = aCheck.m_lModelFrames.getLength();
        sal_Int32 i = 0;
        for (i=0; i<c; ++i)
        {
            if (!fpf::closeIt(aCheck.m_lModelFrames[i], sal_False))
                return sal_False;
        }
    }

    
    
    if (bAllowSuspend)
    {
        css::uno::Reference< css::frame::XController > xController = xFrame->getController();
        if (xController.is()) 
        {
            bControllerSuspended = xController->suspend(sal_True);
            if (! bControllerSuspended)
                return sal_False;
        }
    }

    
    
    
    return sal_True;
}


sal_Bool CloseDispatcher::implts_closeFrame()
{
    
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::frame::XFrame > xFrame (m_xCloseFrame.get(), css::uno::UNO_QUERY);
    aReadLock.unlock();
    

    
    if ( ! xFrame.is() )
        return sal_True;

    
    
    
    if (!fpf::closeIt(xFrame, sal_False))
        return sal_False;

    
    WriteGuard aWriteLock(m_aLock);
    m_xCloseFrame = css::uno::WeakReference< css::frame::XFrame >();
    aWriteLock.unlock();
    

    return sal_True;
}


sal_Bool CloseDispatcher::implts_establishBackingMode()
{
    
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::uno::XComponentContext > xContext  = m_xContext;
    css::uno::Reference< css::frame::XFrame >          xFrame (m_xCloseFrame.get(), css::uno::UNO_QUERY);
    aReadLock.unlock();
    

    if (!xFrame.is())
        return sal_False;

    css::uno::Reference < css::document::XActionLockable > xLock( xFrame, css::uno::UNO_QUERY );
    if ( xLock.is() && xLock->isActionLocked() )
        return sal_False;

    css::uno::Reference< css::awt::XWindow > xContainerWindow = xFrame->getContainerWindow();

    css::uno::Reference< css::frame::XController > xStartModule = css::frame::StartModule::createWithParentWindow(
                        xContext, xContainerWindow);

    
    css::uno::Reference< css::awt::XWindow > xBackingWin(xStartModule, css::uno::UNO_QUERY);
    xFrame->setComponent(xBackingWin, xStartModule);
    xStartModule->attachFrame(xFrame);
    xContainerWindow->setVisible(sal_True);

    return sal_True;
}


sal_Bool CloseDispatcher::implts_terminateApplication()
{
    
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;
    aReadLock.unlock();
    

    css::uno::Reference< css::frame::XDesktop2 > xDesktop = css::frame::Desktop::create( xContext );

    return xDesktop->terminate();
}


void CloseDispatcher::implts_notifyResultListener(const css::uno::Reference< css::frame::XDispatchResultListener >& xListener,
                                                        sal_Int16                                                   nState   ,
                                                  const css::uno::Any&                                              aResult  )
{
    if (!xListener.is())
        return;

    css::frame::DispatchResultEvent aEvent(
        css::uno::Reference< css::uno::XInterface >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY),
        nState,
        aResult);

    xListener->dispatchFinished(aEvent);
}


css::uno::Reference< css::frame::XFrame > CloseDispatcher::static_impl_searchRightTargetFrame(const css::uno::Reference< css::frame::XFrame >& xFrame ,
                                                                                              const OUString&                           sTarget)
{
    if (sTarget.equalsIgnoreAsciiCase("_self"))
        return xFrame;

    OSL_ENSURE(sTarget.isEmpty(), "CloseDispatch used for unexpected target. Magic things will happen now .-)");

    css::uno::Reference< css::frame::XFrame > xTarget = xFrame;
    while(true)
    {
        
        if (xTarget->isTop())
            return xTarget;

        
        css::uno::Reference< css::awt::XWindow >    xWindow        = xTarget->getContainerWindow();
        css::uno::Reference< css::awt::XTopWindow > xTopWindowCheck(xWindow, css::uno::UNO_QUERY);
        if (xTopWindowCheck.is())
        {
            
            
            
            
            
            SolarMutexGuard aSolarLock;
            Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
            if (
                (pWindow                  ) &&
                (pWindow->IsSystemWindow())
               )
                return xTarget;
        }

        
        
        
        css::uno::Reference< css::frame::XFrame > xParent(xTarget->getCreator(), css::uno::UNO_QUERY);
        if ( ! xParent.is())
            return xTarget;

        
        xTarget = xParent;
    }
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
