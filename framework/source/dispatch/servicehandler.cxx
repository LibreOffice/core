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

#include <dispatch/servicehandler.hxx>
#include <threadhelp/readguard.hxx>
#include <general.h>
#include <services.h>

#include <com/sun/star/frame/DispatchResultState.hpp>
#include <com/sun/star/task/XJobExecutor.hpp>

#include <vcl/svapp.hxx>

namespace framework{

#define PROTOCOL_VALUE      "service:"
#define PROTOCOL_LENGTH     8

//_________________________________________________________________________________________________________________
// XInterface, XTypeProvider, XServiceInfo

DEFINE_XINTERFACE_5(ServiceHandler                                  ,
                    OWeakObject                                     ,
                    DIRECT_INTERFACE(css::lang::XTypeProvider      ),
                    DIRECT_INTERFACE(css::lang::XServiceInfo       ),
                    DIRECT_INTERFACE(css::frame::XDispatchProvider ),
                    DIRECT_INTERFACE(css::frame::XNotifyingDispatch),
                    DIRECT_INTERFACE(css::frame::XDispatch         ))

DEFINE_XTYPEPROVIDER_5(ServiceHandler                ,
                       css::lang::XTypeProvider      ,
                       css::lang::XServiceInfo       ,
                       css::frame::XDispatchProvider ,
                       css::frame::XNotifyingDispatch,
                       css::frame::XDispatch         )

DEFINE_XSERVICEINFO_MULTISERVICE(ServiceHandler                   ,
                                 ::cppu::OWeakObject              ,
                                 SERVICENAME_PROTOCOLHANDLER      ,
                                 IMPLEMENTATIONNAME_SERVICEHANDLER)

DEFINE_INIT_SERVICE(ServiceHandler,
                    {
                        /*Attention
                            I think we don't need any mutex or lock here ... because we are called by our own static method impl_createInstance()
                            to create a new instance of this class by our own supported service factory.
                            see macro DEFINE_XSERVICEINFO_MULTISERVICE and "impl_initService()" for further informations!
                        */
                    }
                   )

//_________________________________________________________________________________________________________________

/**
    @short      standard ctor
    @descr      These initialize a new instance of ths class with needed informations for work.

    @param      xFactory
                reference to uno servicemanager for creation of new services
*/
ServiceHandler::ServiceHandler( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory )
        //  Init baseclasses first
        : ThreadHelpBase( &Application::GetSolarMutex() )
        , OWeakObject   (                               )
        // Init member
        , m_xFactory    ( xFactory                      )
{
}

//_________________________________________________________________________________________________________________

/**
    @short      standard dtor
    @descr      -
*/
ServiceHandler::~ServiceHandler()
{
    m_xFactory = NULL;
}

//_________________________________________________________________________________________________________________

/**
    @short      decide if this dispatch implementation can be used for requested URL or not
    @descr      A protocol handler is registerd for an URL pattern inside configuration and will
                be asked by the generic dispatch mechanism inside framework, if he can handle this
                special URL wich match his registration. He can agree by returning of a valid dispatch
                instance or disagree by returning <NULL/>.
                We don't create new dispatch instances here realy - we return THIS as result to handle it
                at the same implementation.
*/
css::uno::Reference< css::frame::XDispatch > SAL_CALL ServiceHandler::queryDispatch( const css::util::URL&  aURL    ,
                                                                                     const ::rtl::OUString& /*sTarget*/ ,
                                                                                           sal_Int32        /*nFlags*/  ) throw( css::uno::RuntimeException )
{
    css::uno::Reference< css::frame::XDispatch > xDispatcher;
    if (aURL.Complete.compareToAscii(PROTOCOL_VALUE,PROTOCOL_LENGTH)==0)
        xDispatcher = this;
    return xDispatcher;
}

//_________________________________________________________________________________________________________________

/**
    @short      do the same like dispatch() but for multiple requests at the same time
    @descr      -
*/
css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL ServiceHandler::queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptor ) throw( css::uno::RuntimeException )
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

/**
    @short      dispatch URL with arguments
    @descr      We use threadsafe internal method to do so. It returns a state value - but we ignore it.
                Because we doesn't support status listener notifications here.

    @param      aURL
                    uno URL which should be executed
    @param      lArguments
                    list of optional arguments for this request
*/
void SAL_CALL ServiceHandler::dispatch( const css::util::URL&                                  aURL       ,
                                    const css::uno::Sequence< css::beans::PropertyValue >& lArguments ) throw( css::uno::RuntimeException )
{
    // dispatch() is an [oneway] call ... and may our user release his reference to us immediatly.
    // So we should hold us self alive till this call ends.
    css::uno::Reference< css::frame::XNotifyingDispatch > xSelfHold(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);
    implts_dispatch(aURL,lArguments);
    // No notification for status listener!
}

//_________________________________________________________________________________________________________________

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
                                                        const css::uno::Reference< css::frame::XDispatchResultListener >& xListener ) throw( css::uno::RuntimeException )
{
    // This class was designed to die by reference. And if user release his reference to us immediatly after calling this method
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

//_________________________________________________________________________________________________________________

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
    /* SAFE */
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::lang::XMultiServiceFactory > xFactory = m_xFactory;
    aReadLock.unlock();
    /* SAFE */

    if (!xFactory.is())
        return css::uno::Reference< css::uno::XInterface >();

    // extract service name and may optional given parameters from given URL
    // and use it to create and start the component
    ::rtl::OUString sServiceAndArguments = aURL.Complete.copy(PROTOCOL_LENGTH);
    ::rtl::OUString sServiceName;
    ::rtl::OUString sArguments  ;

    sal_Int32 nArgStart = sServiceAndArguments.indexOf('?',0);
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

    // If a service doesnt support an optional job executor interface - he can't get
    // any given parameters!
    // Because we can't know if we must call createInstanceWithArguments() or XJobExecutor::trigger() ...

    css::uno::Reference< css::uno::XInterface > xService;
    try
    {
        // => a) a service starts running inside his own ctor and we create it only
        xService = xFactory->createInstance(sServiceName);
        // or b) he implements the right interface and starts there (may with optional parameters)
        css::uno::Reference< css::task::XJobExecutor > xExecuteable(xService, css::uno::UNO_QUERY);
        if (xExecuteable.is())
            xExecuteable->trigger(sArguments);
    }
    // ignore all errors - inclusive runtime errors!
    // E.g. a script based service (written in phyton) could not be executed
    // because it contains syntax errors, which was detected at runtime ...
    catch(const css::uno::Exception&)
        { xService.clear(); }

    return xService;
}

//_________________________________________________________________________________________________________________

/**
    @short      add/remove listener for state events
    @descr      We use an internal container to hold such registered listener. This container lives if we live.
                And if call pas registration as non breakable transaction - we can accept the request without
                any explicit lock. Because we share our mutex with this container.

    @param      xListener
                    reference to a valid listener for state events
    @param      aURL
                    URL about listener will be informed, if something occurred
*/
void SAL_CALL ServiceHandler::addStatusListener( const css::uno::Reference< css::frame::XStatusListener >& /*xListener*/ ,
                                                 const css::util::URL&                                     /*aURL*/      ) throw( css::uno::RuntimeException )
{
    // not suported yet
}

//_________________________________________________________________________________________________________________

void SAL_CALL ServiceHandler::removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& /*xListener*/ ,
                                                    const css::util::URL&                                     /*aURL*/      ) throw( css::uno::RuntimeException )
{
    // not suported yet
}

}       //  namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
