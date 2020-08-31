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
#include <services.h>

#include <com/sun/star/frame/DispatchResultState.hpp>
#include <com/sun/star/task/XJobExecutor.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <tools/diagnose_ex.h>
#include <cppuhelper/supportsservice.hxx>

namespace framework{

#define PROTOCOL_VALUE      "service:"
#define PROTOCOL_LENGTH     8

// XInterface, XTypeProvider, XServiceInfo

OUString SAL_CALL ServiceHandler::getImplementationName()
{
    return "com.sun.star.comp.framework.ServiceHandler";
}

sal_Bool SAL_CALL ServiceHandler::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

css::uno::Sequence< OUString > SAL_CALL ServiceHandler::getSupportedServiceNames()
{
    return { SERVICENAME_PROTOCOLHANDLER };
}


/**
    @short      standard ctor
    @descr      This initializes a new instance of this class with needed information for work.

    @param      xFactory
                reference to uno servicemanager for creation of new services
*/
ServiceHandler::ServiceHandler( const css::uno::Reference< css::uno::XComponentContext >& xContext )
        : m_xContext    ( xContext                      )
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
    @descr      A protocol handler is registered for a URL pattern inside configuration and will
                be asked by the generic dispatch mechanism inside framework, if he can handle this
                special URL which match his registration. He can agree by returning of a valid dispatch
                instance or disagree by returning <NULL/>.
                We don't create new dispatch instances here really - we return THIS as result to handle it
                at the same implementation.
*/
css::uno::Reference< css::frame::XDispatch > SAL_CALL ServiceHandler::queryDispatch( const css::util::URL&  aURL    ,
                                                                                     const OUString& /*sTarget*/ ,
                                                                                           sal_Int32        /*nFlags*/  )
{
    css::uno::Reference< css::frame::XDispatch > xDispatcher;
    if (aURL.Complete.startsWith(PROTOCOL_VALUE))
        xDispatcher = this;
    return xDispatcher;
}

/**
    @short      do the same like dispatch() but for multiple requests at the same time
*/
css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL ServiceHandler::queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptor )
{
    sal_Int32 nCount = lDescriptor.getLength();
    css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > lDispatcher( nCount );
    for( sal_Int32 i=0; i<nCount; ++i )
    {
        lDispatcher[i] = queryDispatch(
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
                                        const css::uno::Sequence< css::beans::PropertyValue >& /*lArguments*/ )
{
    // dispatch() is an [oneway] call ... and may our user release his reference to us immediately.
    // So we should hold us self alive till this call ends.
    css::uno::Reference< css::frame::XNotifyingDispatch > xSelfHold(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);
    implts_dispatch(aURL);
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
                                                        const css::uno::Sequence< css::beans::PropertyValue >&            /*lArguments*/,
                                                        const css::uno::Reference< css::frame::XDispatchResultListener >& xListener )
{
    // This class was designed to die by reference. And if user release his reference to us immediately after calling this method
    // we can run into some problems. So we hold us self alive till this method ends.
    // Another reason: We can use this reference as source of sending event at the end too.
    css::uno::Reference< css::frame::XNotifyingDispatch > xThis(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);

    css::uno::Reference< css::uno::XInterface > xService = implts_dispatch(aURL);
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

    @return     <NULL/> if requested service couldn't be created successfully;
                a valid reference otherwise. This return value can be used to indicate,
                if dispatch was successful.
*/
css::uno::Reference< css::uno::XInterface > ServiceHandler::implts_dispatch( const css::util::URL& aURL )
{
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
        xService = m_xContext->getServiceManager()->createInstanceWithContext(sServiceName, m_xContext);
        // or b) he implements the right interface and starts there (may with optional parameters)
        css::uno::Reference< css::task::XJobExecutor > xExecutable(xService, css::uno::UNO_QUERY);
        if (xExecutable.is())
            xExecutable->trigger(sArguments);
    }
    // ignore all errors - inclusive runtime errors!
    // E.g. a script based service (written in Python) could not be executed
    // because it contains syntax errors, which was detected at runtime...
    catch(const css::uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("fwk.dispatch", "ignored");
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
                                                 const css::util::URL&                                     /*aURL*/      )
{
    // not supported yet
}

void SAL_CALL ServiceHandler::removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& /*xListener*/ ,
                                                    const css::util::URL&                                     /*aURL*/      )
{
    // not supported yet
}

}       //  namespace framework


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
framework_ServiceHandler_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(new framework::ServiceHandler(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
