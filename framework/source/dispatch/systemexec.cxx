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

#include <dispatch/systemexec.hxx>
#include <threadhelp/readguard.hxx>
#include <general.h>
#include <services.h>

#include <com/sun/star/system/XSystemShellExecute.hpp>
#include <com/sun/star/util/PathSubstitution.hpp>
#include <com/sun/star/util/XStringSubstitution.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>

#include <vcl/svapp.hxx>
#include <comphelper/componentcontext.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  non exported const
//_________________________________________________________________________________________________________________

#define PROTOCOL_VALUE      "systemexecute:"
#define PROTOCOL_LENGTH     14

//_________________________________________________________________________________________________________________
//  non exported definitions
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
// XInterface, XTypeProvider, XServiceInfo

DEFINE_XINTERFACE_5(SystemExec                                      ,
                    OWeakObject                                     ,
                    DIRECT_INTERFACE(css::lang::XTypeProvider      ),
                    DIRECT_INTERFACE(css::lang::XServiceInfo       ),
                    DIRECT_INTERFACE(css::frame::XDispatchProvider ),
                    DIRECT_INTERFACE(css::frame::XNotifyingDispatch),
                    DIRECT_INTERFACE(css::frame::XDispatch         ))

DEFINE_XTYPEPROVIDER_5(SystemExec                    ,
                       css::lang::XTypeProvider      ,
                       css::lang::XServiceInfo       ,
                       css::frame::XDispatchProvider ,
                       css::frame::XNotifyingDispatch,
                       css::frame::XDispatch         )

DEFINE_XSERVICEINFO_MULTISERVICE(SystemExec                   ,
                                 ::cppu::OWeakObject          ,
                                 SERVICENAME_PROTOCOLHANDLER  ,
                                 IMPLEMENTATIONNAME_SYSTEMEXEC)

DEFINE_INIT_SERVICE(SystemExec,
                    {
                        /*Attention
                            I think we don't need any mutex or lock here ... because we are called by our own static method impl_createInstance()
                            to create a new instance of this class by our own supported service factory.
                            see macro DEFINE_XSERVICEINFO_MULTISERVICE and "impl_initService()" for further informations!
                        */
                    }
                   )

//_________________________________________________________________________________________________________________

SystemExec::SystemExec( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory )
        //  Init baseclasses first
        : ThreadHelpBase( &Application::GetSolarMutex() )
        , OWeakObject   (                               )
        // Init member
        , m_xFactory    ( xFactory                      )
{
}

//_________________________________________________________________________________________________________________

SystemExec::~SystemExec()
{
    m_xFactory = NULL;
}

//_________________________________________________________________________________________________________________

css::uno::Reference< css::frame::XDispatch > SAL_CALL SystemExec::queryDispatch( const css::util::URL&  aURL    ,
                                                                                 const ::rtl::OUString&,
                                                                                       sal_Int32 ) throw( css::uno::RuntimeException )
{
    css::uno::Reference< css::frame::XDispatch > xDispatcher;
    if (aURL.Complete.compareToAscii(PROTOCOL_VALUE,PROTOCOL_LENGTH)==0)
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
    ::rtl::OUString sSystemURLWithVariables = aURL.Complete.copy(PROTOCOL_LENGTH, c);

    // SAFE ->
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::lang::XMultiServiceFactory > xFactory = m_xFactory;
    aReadLock.unlock();
    // <- SAFE

    // TODO check security settings ...

    try
    {
        css::uno::Reference< css::uno::XComponentContext > xContext( comphelper::ComponentContext(xFactory).getUNOContext() );
        css::uno::Reference< css::util::XStringSubstitution > xPathSubst( css::util::PathSubstitution::create(xContext) );

        ::rtl::OUString sSystemURL = xPathSubst->substituteVariables(sSystemURLWithVariables, sal_True); // sal_True force an exception if unknown variables exists !

        css::uno::Reference< css::system::XSystemShellExecute > xShell(
            xFactory->createInstance(SERVICENAME_SYSTEMSHELLEXECUTE),
            css::uno::UNO_QUERY_THROW);

        xShell->execute(sSystemURL, ::rtl::OUString(), css::system::SystemShellExecuteFlags::URIS_ONLY);
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
