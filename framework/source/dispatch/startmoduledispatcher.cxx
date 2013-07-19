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


#include <dispatch/startmoduledispatcher.hxx>

#include <pattern/frame.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>
#include <framework/framelistanalyzer.hxx>
#include <dispatchcommands.h>
#include <targets.h>
#include <services.h>
#include <general.h>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/CommandGroup.hpp>
#include <com/sun/star/frame/StartModule.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include "com/sun/star/beans/XFastPropertySet.hpp"
#include <com/sun/star/frame/XModuleManager.hpp>

#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <unotools/moduleoptions.hxx>
#include <comphelper/processfactory.hxx>


namespace framework{

#ifdef fpf
    #error "Who uses \"fpf\" as define. It will overwrite my namespace alias ..."
#endif
namespace fpf = ::framework::pattern::frame;


//-----------------------------------------------
StartModuleDispatcher::StartModuleDispatcher(const css::uno::Reference< css::uno::XComponentContext >&     rxContext,
                                             const css::uno::Reference< css::frame::XFrame >&              xFrame ,
                                             const OUString&                                        sTarget)
    : ThreadHelpBase     (&Application::GetSolarMutex() )
    , m_xContext         (rxContext                         )
    , m_xOwner           (xFrame                        )
    , m_sDispatchTarget  (sTarget                       )
    , m_lStatusListener  (m_aLock.getShareableOslMutex())
{
}

//-----------------------------------------------
StartModuleDispatcher::~StartModuleDispatcher()
{
}

//-----------------------------------------------
void SAL_CALL StartModuleDispatcher::dispatch(const css::util::URL&                                  aURL      ,
                                              const css::uno::Sequence< css::beans::PropertyValue >& lArguments)
    throw(css::uno::RuntimeException)
{
    dispatchWithNotification(aURL, lArguments, css::uno::Reference< css::frame::XDispatchResultListener >());
}

//-----------------------------------------------
void SAL_CALL StartModuleDispatcher::dispatchWithNotification(const css::util::URL&                                             aURL      ,
                                                              const css::uno::Sequence< css::beans::PropertyValue >&            /*lArguments*/,
                                                              const css::uno::Reference< css::frame::XDispatchResultListener >& xListener )
    throw(css::uno::RuntimeException)
{
    ::sal_Int16 nResult = css::frame::DispatchResultState::DONTKNOW;
    if ( aURL.Complete == CMD_UNO_SHOWSTARTMODULE )
    {
        nResult = css::frame::DispatchResultState::FAILURE;
        if (implts_isBackingModePossible ())
        {
            if (implts_establishBackingMode ())
                nResult = css::frame::DispatchResultState::SUCCESS;
        }
    }

    implts_notifyResultListener(xListener, nResult, css::uno::Any());
}

//-----------------------------------------------
css::uno::Sequence< ::sal_Int16 > SAL_CALL StartModuleDispatcher::getSupportedCommandGroups()
    throw(css::uno::RuntimeException)
{
    return css::uno::Sequence< ::sal_Int16 >();
}

//-----------------------------------------------
css::uno::Sequence< css::frame::DispatchInformation > SAL_CALL StartModuleDispatcher::getConfigurableDispatchInformation(::sal_Int16 /*nCommandGroup*/)
    throw(css::uno::RuntimeException)
{
    return css::uno::Sequence< css::frame::DispatchInformation >();
}

//-----------------------------------------------
void SAL_CALL StartModuleDispatcher::addStatusListener(const css::uno::Reference< css::frame::XStatusListener >& /*xListener*/,
                                                       const css::util::URL&                                     /*aURL*/     )
    throw(css::uno::RuntimeException)
{
}

//-----------------------------------------------
void SAL_CALL StartModuleDispatcher::removeStatusListener(const css::uno::Reference< css::frame::XStatusListener >& /*xListener*/,
                                                          const css::util::URL&                                     /*aURL*/     )
    throw(css::uno::RuntimeException)
{
}

//-----------------------------------------------
::sal_Bool StartModuleDispatcher::implts_isBackingModePossible ()
{
    if ( ! SvtModuleOptions().IsModuleInstalled(SvtModuleOptions::E_SSTARTMODULE))
        return sal_False;

    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    css::uno::Reference< css::frame::XFramesSupplier > xDesktop(
        css::frame::Desktop::create( xContext ), css::uno::UNO_QUERY);

    FrameListAnalyzer aCheck(
        xDesktop,
        css::uno::Reference< css::frame::XFrame >(),
        FrameListAnalyzer::E_HELP | FrameListAnalyzer::E_BACKINGCOMPONENT);

    ::sal_Bool  bIsPossible    = sal_False;
    ::sal_Int32 nVisibleFrames = aCheck.m_lOtherVisibleFrames.getLength ();

    if (
        ( ! aCheck.m_xBackingComponent.is ()) &&
        (   nVisibleFrames < 1              )
       )
    {
        bIsPossible = sal_True;
    }

    return bIsPossible;
}

//-----------------------------------------------
::sal_Bool StartModuleDispatcher::implts_establishBackingMode()
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::uno::XComponentContext > xContext  = m_xContext;
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    css::uno::Reference< css::frame::XDesktop2> xDesktop       = css::frame::Desktop::create( xContext );
    css::uno::Reference< css::frame::XFrame > xFrame           = xDesktop->findFrame(SPECIALTARGET_BLANK, 0);
    css::uno::Reference< css::awt::XWindow  > xContainerWindow = xFrame->getContainerWindow();

    css::uno::Reference< css::frame::XController > xStartModule = css::frame::StartModule::createWithParentWindow(xContext, xContainerWindow);
    css::uno::Reference< css::awt::XWindow > xComponentWindow(xStartModule, css::uno::UNO_QUERY);
    xFrame->setComponent(xComponentWindow, xStartModule);
    xStartModule->attachFrame(xFrame);
    xContainerWindow->setVisible(sal_True);

    return sal_True;
}

//-----------------------------------------------
void StartModuleDispatcher::implts_notifyResultListener(const css::uno::Reference< css::frame::XDispatchResultListener >& xListener,
                                                              ::sal_Int16                                                 nState   ,
                                                        const css::uno::Any&                                              aResult  )
{
    if ( ! xListener.is())
        return;

    css::frame::DispatchResultEvent aEvent(
        css::uno::Reference< css::uno::XInterface >(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY),
        nState,
        aResult);

    xListener->dispatchFinished(aEvent);
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
