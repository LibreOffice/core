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

#include <dispatch/servicehandler.hxx>
#include <general.h>
#include <services.h>

#include <com/sun/star/frame/DispatchResultState.hpp>
#include <com/sun/star/task/XJobExecutor.hpp>

#include <vcl/svapp.hxx>

namespace framework{

#define PROTOCOL_VALUE      "service:"
#define PROTOCOL_LENGTH     8

// XInterface, XTypeProvider, XServiceInfo

DEFINE_XSERVICEINFO_MULTISERVICE(ServiceHandler                   ,
                                 ::cppu::OWeakObject              ,
                                 SERVICENAME_PROTOCOLHANDLER      ,
                                 IMPLEMENTATIONNAME_SERVICEHANDLER)

DEFINE_INIT_SERVICE(ServiceHandler,
                    {
                        /*Attention
                            I think we don't need any mutex or lock here ... because we are called by our own static method impl_createInstance()
                            to create a new instance of this class by our own supported service factory.
                            see macro DEFINE_XSERVICEINFO_MULTISERVICE and "impl_initService()" for further information!
                        */
                    }
                   )

/**
    @short      standard ctor
    @descr      This initializes a new instance of ths class with needed information for work.

    @param      xFactory
                reference to uno servicemanager for creation of new services
*/
ServiceHandler::ServiceHandler( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory )
        : m_xFactory    ( xFactory                      )
{
}

/**
    @short      standard dtor
*/
ServiceHandler::~ServiceHandler()
{
}

/**
    @short      decide if this dispatch implementation can be used for requested URL or not
    @descr      A protocol handler is registered for an URL pattern inside configuration and will
                be asked by the generic dispatch mechanism inside framework, if he can handle this
                special URL which match his registration. He can agree by returning of a valid dispatch
                instance or disagree by returning <NULL/>.
                We don't create new dispatch instances here really - we return THIS as result to handle it
                at the same implementation.
*/
css::uno::Reference< css::frame::XDispatch > SAL_CALL ServiceHandler::queryDispatch( const css::util::URL&  aURL    ,
                                                                                     const OUString& /*sTarget*/ ,
                                                                                           sal_Int32        /*nFlags*/  ) throw( css::uno::RuntimeException, std::exception )
{
    css::uno::Reference< css::frame::XDispatch > xDispatcher;
    if (aURL.Complete.startsWith(PROTOCOL_VALUE))
        xDispatcher = this;
    return xDispatcher;
}

/**
    @short      do the same like dispatch() but for multiple requests at the same time
*/
css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL ServiceHandler::queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptor ) throw( css::uno::RuntimeException, std::exception )
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

/**
    @short      dispatch URL with arguments
    @descr      We use threadsafe internal method to do so. It returns a state value - but we ignore it.
                Because we don't support status listener notifications here.

    @param      aURL
                    uno URL which should be executed
    @param      lArguments
                    list of optional arguments for this request
*/
void SAL_CALL ServiceHandler::dispatch( const css::util::URL&                                  aURL       ,
                                    const css::uno::Sequence< css::beans::PropertyValue >& lArguments ) throw( css::uno::RuntimeException, std::exception )
{
    // dispatch() is an [oneway] call ... and may our user release his reference to us immediately.
    // So we should hold us self alive till this call ends.
    css::uno::Reference< css::frame::XNotifyingDispatch > xSelfHold(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);
    implts_dispatch(aURL,lArguments);
    // No notification for status listener!
}

/**
    @short      dispatch with guaranteed notifications about success
    @descr      We use threadsafe internal method to do so. Return state of this function will be used
                for notification if an optional listener is given.

    @param      aURL
                    uno URL which should be executed
    @param      lArguments
                    list of optional arguments for this request
    @param      xListener
                    optional listener for state events
*/
void SAL_CALL ServiceHandler::dispatchWithNotification( const css::util::URL&                                             aURL      ,
                                                        const css::uno::Sequence< css::beans::PropertyValue >&            lArguments,
                                                        const css::uno::Reference< css::frame::XDispatchResultListener >& xListener ) throw( css::uno::RuntimeException, std::exception )
{
    // This class was designed to die by reference. And if user release his reference to us immediately after calling this method
    // we can run into some problems. So we hold us self alive till this method ends.
    // Another reason: We can use this reference as source of sending event at the end too.
    css::uno::Reference< css::frame::XNotifyingDispatch > xThis(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);

    css::uno::Reference< css::uno::XInterface > xService = implts_dispatch(aURL,lArguments);
    if (xListener.is())
    {
        css::frame::DispatchResultEvent aEvent;
        if (xService.is())
            aEvent.State = css::frame::DispatchResultState::SUCCESS;
        else
            aEvent.State = css::frame::DispatchResultState::FAILURE;
        aEvent.Result <<= xService; // may NULL for state=FAILED!
        aEvent.Source = xThis;

        xListener->dispatchFinished( aEvent );
    }
}

/**
    @short      threadsafe helper for dispatch calls
    @descr      We support two interfaces for the same process - dispatch URLs. That the reason for this internal
                function. It implements the real dispatch operation and returns a state value which inform caller
                about success. He can notify listener then by using this return value.

    @param      aURL
                    uno URL which should be executed
    @param      lArguments
                    list of optional arguments for this request

    @return     <NULL/> if requested service couldn't be created successullfy;
                a valid reference otherwise. This return value can be used to indicate,
                if dispatch was successfully or not.
*/
css::uno::Reference< css::uno::XInterface > ServiceHandler::implts_dispatch( const css::util::URL&                                  aURL       ,
                                                                             const css::uno::Sequence< css::beans::PropertyValue >& /*lArguments*/ ) throw( css::uno::RuntimeException )
{
    if (!m_xFactory.is())
        return css::uno::Reference< css::uno::XInterface >();

    // extract service name and may optional given parameters from given URL
    // and use it to create and start the component
    OUString sServiceAndArguments = aURL.Complete.copy(PROTOCOL_LENGTH);
    OUString sServiceName;
    OUString sArguments;

    sal_Int32 nArgStart = sServiceAndArguments.indexOf('?');
    if (nArgStart!=-1)
    {
        sServiceName = sServiceAndArguments.copy(0,nArgStart);
        ++nArgStart; // ignore '?'!
        sArguments   = sServiceAndArguments.copy(nArgStart);
    }
    else
    {
        sServiceName = sServiceAndArguments;
    }

    if (sServiceName.isEmpty())
        return css::uno::Reference< css::uno::XInterface >();

    // If a service doesn't support an optional job executor interface - he can't get
    // any given parameters!
    // Because we can't know if we must call createInstanceWithArguments() or XJobExecutor::trigger() ...

    css::uno::Reference< css::uno::XInterface > xService;
    try
    {
        // => a) a service starts running inside his own ctor and we create it only
        xService = m_xFactory->createInstance(sServiceName);
        // or b) he implements the right interface and starts there (may with optional parameters)
        css::uno::Reference< css::task::XJobExecutor > xExecuteable(xService, css::uno::UNO_QUERY);
        if (xExecuteable.is())
            xExecuteable->trigger(sArguments);
    }
    // ignore all errors - inclusive runtime errors!
    // E.g. a script based service (written in Python) could not be executed
    // because it contains syntax errors, which was detected at runtime...
    catch(const css::uno::Exception& e)
    {
        SAL_WARN(
            "fwk.dispatch", "ignored UNO Exception \"" << e.Message << '"');
        xService.clear();
    }

    return xService;
}

/**
    @short      add/remove listener for state events
    @descr      We use an internal container to hold such registered listener. This container lives if we live.
                And if call pass registration as non breakable transaction - we can accept the request without
                any explicit lock. Because we share our mutex with this container.

    @param      xListener
                    reference to a valid listener for state events
    @param      aURL
                    URL about listener will be informed, if something occurred
*/
void SAL_CALL ServiceHandler::addStatusListener( const css::uno::Reference< css::frame::XStatusListener >& /*xListener*/ ,
                                                 const css::util::URL&                                     /*aURL*/      ) throw( css::uno::RuntimeException, std::exception )
{
    // not supported yet
}

void SAL_CALL ServiceHandler::removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& /*xListener*/ ,
                                                    const css::util::URL&                                     /*aURL*/      ) throw( css::uno::RuntimeException, std::exception )
{
    // not supported yet
}

}       //  namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
