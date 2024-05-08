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

#include <dispatch/oxt_handler.hxx>
#include <unotools/mediadescriptor.hxx>

#include <com/sun/star/frame/DispatchResultState.hpp>
#include <com/sun/star/task/XJobExecutor.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <tools/long.hxx>
#include <utility>

namespace framework{

// XInterface, XTypeProvider, XServiceInfo

OUString SAL_CALL Oxt_Handler::getImplementationName()
{
    return u"com.sun.star.comp.framework.OXTFileHandler"_ustr;
}

sal_Bool SAL_CALL Oxt_Handler::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

css::uno::Sequence< OUString > SAL_CALL Oxt_Handler::getSupportedServiceNames()
{
    return { u"com.sun.star.frame.ContentHandler"_ustr };
}


/*-************************************************************************************************************
    @short      standard ctor
    @descr      These initialize a new instance of this class with needed information for work.

    @seealso    using at owner

    @param      "xFactory", reference to service manager for creation of new services
    @onerror    Show an assertion and do nothing else.
    @threadsafe yes
*//*-*************************************************************************************************************/
Oxt_Handler::Oxt_Handler( css::uno::Reference< css::uno::XComponentContext > xContext )
        :   m_xContext          (std::move( xContext ))
{
}

/*-************************************************************************************************************
    @short      standard dtor
*//*-*************************************************************************************************************/
Oxt_Handler::~Oxt_Handler()
{
}

/*-************************************************************************************************************
    @interface  css::frame::XDispatch

    @short      try to load audio file
    @descr      This method try to load given audio file by URL and play it. We use vcl/Sound class to do that.
                Playing of sound is asynchron every time.

    @attention  We must hold us alive by ourself ... because we use async. vcl sound player ... but playing is started
                in async interface call "dispatch()" too. And caller forget us immediately. But then our uno ref count
                will decreased to 0 and will die. The only solution is to use own reference to our implementation.
                But we do it for really started jobs only and release it during call back of vcl.

    @seealso    class vcl/Sound
    @seealso    method implts_PlayerNotify()

    @param      "aURL"      , URL to dispatch.
    @param      "lArguments", list of optional arguments.
    @onerror    We do nothing.
    @threadsafe yes
*//*-*************************************************************************************************************/
void SAL_CALL Oxt_Handler::dispatchWithNotification( const css::util::URL& aURL,
                                                     const css::uno::Sequence< css::beans::PropertyValue >&            /*lArguments*/,
                                                     const css::uno::Reference< css::frame::XDispatchResultListener >& xListener )
{
    std::unique_lock g(m_mutex);

    css::uno::Sequence< css::uno::Any > lParams{ css::uno::Any(aURL.Main) };

    css::uno::Reference< css::uno::XInterface > xService = m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext( u"com.sun.star.deployment.ui.PackageManagerDialog"_ustr, lParams, m_xContext );
    css::uno::Reference< css::task::XJobExecutor > xExecutable( xService, css::uno::UNO_QUERY );
    if ( xExecutable.is() )
        xExecutable->trigger( OUString() );

    if ( xListener.is() )
    {
        css::frame::DispatchResultEvent aEvent;
        aEvent.State = css::frame::DispatchResultState::SUCCESS;
        xListener->dispatchFinished( aEvent );
    }
}

void SAL_CALL Oxt_Handler::dispatch( const css::util::URL&                                  aURL       ,
                                     const css::uno::Sequence< css::beans::PropertyValue >& lArguments )
{
    dispatchWithNotification( aURL, lArguments, css::uno::Reference< css::frame::XDispatchResultListener >() );
}

/*-************************************************************************************************************
    @interface  css::document::XExtendedFilterDetection

    @short      try to detect file (given as argument included in "lDescriptor")
    @descr      We try to detect, if given file could be handled by this class and is a well known one.
                If it is - we return right internal type name - otherwise we return nothing!
                So call can search for another detect service and ask him too.

    @attention  a) We don't need any mutex here ... because we don't use any member!
                b) Don't use internal player instance "m_pPlayer" to detect given sound file!
                   It's not necessary to do that ... and we can use temp. variable to do the same.
                   This way is easy - we don't must synchronize it with currently played sounds!
                   Another reason to do so ... We are a listener on our internal ma_Player object.
                   If you would call "IsSoundFile()" on this instance, he would call us back and
                   we make some unnecessary things ...
    @param      "lDescriptor", description of file to detect
    @return     Internal type name which match this file ... or nothing if it is unknown.

    @onerror    We return nothing.
    @threadsafe yes
*//*-*************************************************************************************************************/
OUString SAL_CALL Oxt_Handler::detect( css::uno::Sequence< css::beans::PropertyValue >& lDescriptor )
{
    // Our default is "nothing". So we can return it, if detection failed or file type is really unknown.
    OUString sTypeName;

    // Analyze given descriptor to find filename or input stream or...
    utl::MediaDescriptor aDescriptor( lDescriptor );
    OUString               sURL       = aDescriptor.getUnpackedValueOrDefault( utl::MediaDescriptor::PROP_URL, OUString() );

    tools::Long nLength = sURL.getLength();
    if ( ( nLength > 4 ) && sURL.matchIgnoreAsciiCase( ".oxt", nLength-4 ) )
    {
        // "IsSoundFile" differs between different "wav" and "au" file versions...
        // couldn't return this information... because: it use the OS to detect it!
        // I think we can than following ones:
        //  a) look for given extension of url to map our type decision HARD CODED!!!
        //  b) return preferred type every time... it's easy :-)
        sTypeName = "oxt_OpenOffice_Extension";
        aDescriptor[utl::MediaDescriptor::PROP_TYPENAME] <<= sTypeName;
        aDescriptor >> lDescriptor;
    }

    // Return our decision.
    return sTypeName;
}

} // namespace framework


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
framework_Oxt_Handler_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(new framework::Oxt_Handler(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
