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

#include <dispatch/oxt_handler.hxx>

#include <threadhelp/transactionguard.hxx>

#include <threadhelp/writeguard.hxx>

#include <threadhelp/readguard.hxx>

#include <macros/debug.hxx>

#include <services.h>

#include <comphelper/mediadescriptor.hxx>

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>
#include <com/sun/star/task/XJobExecutor.hpp>

#include <comphelper/sequenceashashmap.hxx>

#include <rtl/ustrbuf.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  non exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  non exported definitions
//_________________________________________________________________________________________________________________

#define UNISTRING(s) rtl::OUString(s)

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XINTERFACE_5                 (   Oxt_Handler                                                ,
                                        OWeakObject                                                ,
                                        DIRECT_INTERFACE( css::lang::XTypeProvider                ),
                                        DIRECT_INTERFACE( css::lang::XServiceInfo                 ),
                                        DIRECT_INTERFACE( css::frame::XNotifyingDispatch          ),
                                        DIRECT_INTERFACE( css::frame::XDispatch                   ),
                                        DIRECT_INTERFACE( css::document::XExtendedFilterDetection )
                                    )

DEFINE_XTYPEPROVIDER_5              (   Oxt_Handler                                                ,
                                        css::lang::XTypeProvider                                   ,
                                        css::lang::XServiceInfo                                    ,
                                        css::frame::XNotifyingDispatch                             ,
                                        css::frame::XDispatch                                      ,
                                        css::document::XExtendedFilterDetection
                                    )

DEFINE_XSERVICEINFO_MULTISERVICE    (   Oxt_Handler                                                ,
                                        ::cppu::OWeakObject                                        ,
                                        SERVICENAME_CONTENTHANDLER                                 ,
                                        IMPLEMENTATIONNAME_OXT_HANDLER
                                    )

DEFINE_INIT_SERVICE                 (   Oxt_Handler,
                                        {
                                        }
                                    )

/*-************************************************************************************************************//**
    @short      standard ctor
    @descr      These initialize a new instance of this class with needed informations for work.

    @seealso    using at owner

    @param      "xFactory", reference to service manager for creation of new services
    @return     -

    @onerror    Show an assertion and do nothing else.
    @threadsafe yes
*//*-*************************************************************************************************************/
Oxt_Handler::Oxt_Handler( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory )
        //  Init baseclasses first
        :   ThreadHelpBase      (          )
        ,   ::cppu::OWeakObject (          )
        // Init member
        ,   m_xFactory          ( xFactory )
{
}

/*-************************************************************************************************************//**
    @short      standard dtor
    @descr      -

    @seealso    -

    @param      -
    @return     -

    @onerror    -
    @threadsafe -
*//*-*************************************************************************************************************/
Oxt_Handler::~Oxt_Handler()
{
    if ( m_xListener.is() )
    {
        css::frame::DispatchResultEvent aEvent;
        aEvent.State = css::frame::DispatchResultState::FAILURE;
        m_xListener->dispatchFinished( aEvent );
        m_xListener = css::uno::Reference< css::frame::XDispatchResultListener >();
    }
}

/*-************************************************************************************************************//**
    @interface  ::com::sun::star::frame::XDispatch

    @short      try to load audio file
    @descr      This method try to load given audio file by URL and play it. We use vcl/Sound class to do that.
                Playing of sound is asynchron everytime.

    @attention  We must hold us alive by ourself ... because we use async. vcl sound player ... but playing is started
                in async interface call "dispatch()" too. And caller forget us imediatly. But then our uno ref count
                will decreased to 0 and will die. The only solution is to use own reference to our implementation.
                But we do it for realy started jobs only and release it during call back of vcl.

    @seealso    class vcl/Sound
    @seealso    method implts_PlayerNotify()

    @param      "aURL"      , URL to dispatch.
    @param      "lArguments", list of optional arguments.
    @return     -

    @onerror    We do nothing.
    @threadsafe yes
*//*-*************************************************************************************************************/
void SAL_CALL Oxt_Handler::dispatchWithNotification( const css::util::URL& aURL,
                                                     const css::uno::Sequence< css::beans::PropertyValue >&            /*lArguments*/,
                                                     const css::uno::Reference< css::frame::XDispatchResultListener >& xListener )
    throw( css::uno::RuntimeException )
{
    // SAFE {
    ResetableGuard aLock( m_aLock );

    rtl::OUString sServiceName = UNISTRING( "com.sun.star.deployment.ui.PackageManagerDialog" );
    css::uno::Sequence< css::uno::Any > lParams(1);
    lParams[0] <<= aURL.Main;

    css::uno::Reference< css::uno::XInterface > xService;

    xService = m_xFactory->createInstanceWithArguments( sServiceName, lParams );
    css::uno::Reference< css::task::XJobExecutor > xExecuteable( xService, css::uno::UNO_QUERY );
    if ( xExecuteable.is() )
        xExecuteable->trigger( rtl::OUString() );

    if ( xListener.is() )
    {
        css::frame::DispatchResultEvent aEvent;
        aEvent.State = css::frame::DispatchResultState::SUCCESS;
        xListener->dispatchFinished( aEvent );
    }

    // } SAFE
    aLock.unlock();
}

void SAL_CALL Oxt_Handler::dispatch( const css::util::URL&                                  aURL       ,
                                     const css::uno::Sequence< css::beans::PropertyValue >& lArguments )
    throw( css::uno::RuntimeException )
{
    dispatchWithNotification( aURL, lArguments, css::uno::Reference< css::frame::XDispatchResultListener >() );
}

/*-************************************************************************************************************//**
    @interface  ::com::sun::star::document::XExtendedFilterDetection

    @short      try to detect file (given as argument included in "lDescriptor")
    @descr      We try to detect, if given file could be handled by this class and is a well known one.
                If it is - we return right internal type name - otherwise we return nothing!
                So call can search for another detect service and ask him too.

    @attention  a) We don't need any mutex here ... because we don't use any member!
                b) Dont' use internal player instance "m_pPlayer" to detect given sound file!
                   It's not neccessary to do that ... and we can use temp. variable to do the same.
                   This way is easy - we don't must synchronize it with currently played sounds!
                   Another reason to do so ... We are a listener on our internal ma_Player object.
                   If you would call "IsSoundFile()" on this instance, he would call us back and
                   we make some uneccssary things ...

    @seealso    -

    @param      "lDescriptor", description of file to detect
    @return     Internal type name which match this file ... or nothing if it is unknown.

    @onerror    We return nothing.
    @threadsafe yes
*//*-*************************************************************************************************************/
::rtl::OUString SAL_CALL Oxt_Handler::detect( css::uno::Sequence< css::beans::PropertyValue >& lDescriptor )
    throw( css::uno::RuntimeException )
{
    // Our default is "nothing". So we can return it, if detection failed or fily type is realy unknown.
    ::rtl::OUString sTypeName;

    // Analyze given descriptor to find filename or input stream or ...
    ::comphelper::MediaDescriptor aDescriptor( lDescriptor );
    ::rtl::OUString               sURL       = aDescriptor.getUnpackedValueOrDefault( ::comphelper::MediaDescriptor::PROP_URL(), ::rtl::OUString() );

    long nLength = sURL.getLength();
    if ( ( nLength > 4 ) && sURL.matchIgnoreAsciiCase( UNISTRING(".oxt"), nLength-4 ) )
    {
        // "IsSoundFile" idffer between different "wav" and "au" file versions ...
        // couldn't return this information ... because: He use the OS to detect it!
        // I think we can the following ones:
        //  a) look for given extension of url to map our type decision HARD CODED!!!
        //  b) return preferred type every time... it's easy :-)
        sTypeName = ::rtl::OUString("oxt_OpenOffice_Extension");
        aDescriptor[::comphelper::MediaDescriptor::PROP_TYPENAME()] <<= sTypeName;
        aDescriptor >> lDescriptor;
    }

    // Return our decision.
    return sTypeName;
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
