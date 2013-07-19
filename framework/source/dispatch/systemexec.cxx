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

#include <dispatch/systemexec.hxx>
#include <threadhelp/readguard.hxx>
#include <general.h>
#include <services.h>

#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/util/PathSubstitution.hpp>
#include <com/sun/star/util/XStringSubstitution.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>

#include <vcl/svapp.hxx>
#include <comphelper/processfactory.hxx>

namespace framework{

#define PROTOCOL_VALUE      "systemexecute:"
#define PROTOCOL_LENGTH     14

//_________________________________________________________________________________________________________________
// XInterface, XTypeProvider, XServiceInfo

DEFINE_XSERVICEINFO_MULTISERVICE_2(SystemExec                   ,
                                 ::cppu::OWeakObject          ,
                                 SERVICENAME_PROTOCOLHANDLER  ,
                                 IMPLEMENTATIONNAME_SYSTEMEXEC)

DEFINE_INIT_SERVICE(SystemExec,
                    {
                        /*Attention
                            I think we don't need any mutex or lock here ... because we are called by our own static method impl_createInstance()
                            to create a new instance of this class by our own supported service factory.
                            see macro DEFINE_XSERVICEINFO_MULTISERVICE and "impl_initService()" for further information!
                        */
                    }
                   )

//_________________________________________________________________________________________________________________

SystemExec::SystemExec( const css::uno::Reference< css::uno::XComponentContext >& rxContext )
        //  Init baseclasses first
        : ThreadHelpBase( &Application::GetSolarMutex() )
        // Init member
        , m_xContext    ( rxContext                     )
{
}

//_________________________________________________________________________________________________________________

SystemExec::~SystemExec()
{
    m_xContext = NULL;
}

//_________________________________________________________________________________________________________________

css::uno::Reference< css::frame::XDispatch > SAL_CALL SystemExec::queryDispatch( const css::util::URL&  aURL    ,
                                                                                 const OUString&,
                                                                                       sal_Int32 ) throw( css::uno::RuntimeException )
{
    css::uno::Reference< css::frame::XDispatch > xDispatcher;
    if (aURL.Complete.startsWith(PROTOCOL_VALUE))
        xDispatcher = this;
    return xDispatcher;
}

//_________________________________________________________________________________________________________________

css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL SystemExec::queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptor ) throw( css::uno::RuntimeException )
{
    sal_Int32 nCount = lDescriptor.getLength();
    css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > lDispatcher( nCount );
    for( sal_Int32 i=0; i<nCount; ++i )
    {
        lDispatcher[i] = this->queryDispatch(
                            lDescriptor[i].FeatureURL,
                            lDescriptor[i].FrameName,
                            lDescriptor[i].SearchFlags);
    }
    return lDispatcher;
}

//_________________________________________________________________________________________________________________

void SAL_CALL SystemExec::dispatch( const css::util::URL&                                  aURL       ,
                                    const css::uno::Sequence< css::beans::PropertyValue >& lArguments ) throw( css::uno::RuntimeException )
{
    dispatchWithNotification(aURL, lArguments, css::uno::Reference< css::frame::XDispatchResultListener >());
}

//_________________________________________________________________________________________________________________

void SAL_CALL SystemExec::dispatchWithNotification( const css::util::URL&                                             aURL      ,
                                                    const css::uno::Sequence< css::beans::PropertyValue >&,
                                                    const css::uno::Reference< css::frame::XDispatchResultListener >& xListener ) throw( css::uno::RuntimeException )
{
    // convert "systemexec:file:///c:/temp/test.html" => "file:///c:/temp/test.html"
    sal_Int32 c = aURL.Complete.getLength()-PROTOCOL_LENGTH;
    if (c<1) // we dont check for valid URLs here! The system will show an error message ...
    {
        impl_notifyResultListener(xListener, css::frame::DispatchResultState::FAILURE);
        return;
    }
    OUString sSystemURLWithVariables = aURL.Complete.copy(PROTOCOL_LENGTH, c);

    // SAFE ->
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;
    aReadLock.unlock();
    // <- SAFE

    // TODO check security settings ...

    try
    {
        css::uno::Reference< css::util::XStringSubstitution > xPathSubst( css::util::PathSubstitution::create(xContext) );

        OUString sSystemURL = xPathSubst->substituteVariables(sSystemURLWithVariables, sal_True); // sal_True force an exception if unknown variables exists !

        css::uno::Reference< css::system::XSystemShellExecute > xShell = css::system::SystemShellExecute::create( xContext );

        xShell->execute(sSystemURL, OUString(), css::system::SystemShellExecuteFlags::URIS_ONLY);
        impl_notifyResultListener(xListener, css::frame::DispatchResultState::SUCCESS);
    }
    catch(const css::uno::Exception&)
        {
            impl_notifyResultListener(xListener, css::frame::DispatchResultState::FAILURE);
        }
}

//_________________________________________________________________________________________________________________

void SAL_CALL SystemExec::addStatusListener( const css::uno::Reference< css::frame::XStatusListener >&,
                                             const css::util::URL& ) throw( css::uno::RuntimeException )
{
    // not suported yet
}

//_________________________________________________________________________________________________________________

void SAL_CALL SystemExec::removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >&,
                                                const css::util::URL& ) throw( css::uno::RuntimeException )
{
    // not suported yet
}

//_________________________________________________________________________________________________________________

void SystemExec::impl_notifyResultListener(const css::uno::Reference< css::frame::XDispatchResultListener >& xListener,
                                           const sal_Int16                                                   nState   )
{
    if (xListener.is())
    {
        css::frame::DispatchResultEvent aEvent;
        aEvent.State = nState;
        xListener->dispatchFinished(aEvent);
    }
}

}       //  namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
