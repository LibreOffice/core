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


#include <dispatch/startmoduledispatcher.hxx>

#include <pattern/frame.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>
#include <framework/framelistanalyzer.hxx>
#include <dispatchcommands.h>
#include <targets.h>
#include <services.h>
#include <general.h>

#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/CommandGroup.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include "com/sun/star/beans/XFastPropertySet.hpp"
#include <com/sun/star/frame/XModuleManager.hpp>

#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <unotools/moduleoptions.hxx>


namespace framework{

#ifdef fpf
    #error "Who uses \"fpf\" as define. It will overwrite my namespace alias ..."
#endif
namespace fpf = ::framework::pattern::frame;


DEFINE_XINTERFACE_4(StartModuleDispatcher                                     ,
                    OWeakObject                                               ,
                    DIRECT_INTERFACE(css::lang::XTypeProvider                ),
                    DIRECT_INTERFACE(css::frame::XNotifyingDispatch          ),
                    DIRECT_INTERFACE(css::frame::XDispatch                   ),
                    DIRECT_INTERFACE(css::frame::XDispatchInformationProvider))

// Note: XStatusListener is an implementation detail. Hide it for scripting!
DEFINE_XTYPEPROVIDER_4(StartModuleDispatcher                   ,
                       css::lang::XTypeProvider                ,
                       css::frame::XDispatchInformationProvider,
                       css::frame::XNotifyingDispatch          ,
                       css::frame::XDispatch                   )

//-----------------------------------------------
StartModuleDispatcher::StartModuleDispatcher(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR  ,
                                             const css::uno::Reference< css::frame::XFrame >&              xFrame ,
                                             const ::rtl::OUString&                                        sTarget)
    : ThreadHelpBase     (&Application::GetSolarMutex() )
    , ::cppu::OWeakObject(                              )
    , m_xSMGR            (xSMGR                         )
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
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    css::uno::Reference< css::frame::XFramesSupplier > xDesktop(
        xSMGR->createInstance(SERVICENAME_DESKTOP), css::uno::UNO_QUERY);

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
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR  = m_xSMGR;
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    css::uno::Reference< css::frame::XFrame > xDesktop         (xSMGR->createInstance(SERVICENAME_DESKTOP), css::uno::UNO_QUERY);
    css::uno::Reference< css::frame::XFrame > xFrame           = xDesktop->findFrame (SPECIALTARGET_BLANK, 0);
    css::uno::Reference< css::awt::XWindow  > xContainerWindow = xFrame->getContainerWindow ();

    css::uno::Sequence< css::uno::Any > lArgs(1);
    lArgs[0] <<= xContainerWindow;

    css::uno::Reference< css::frame::XController > xStartModule(
        xSMGR->createInstanceWithArguments(SERVICENAME_STARTMODULE, lArgs),
        css::uno::UNO_QUERY_THROW);

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
