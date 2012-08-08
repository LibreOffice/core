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

#include <services/dispatchhelper.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>
#include <services.h>

#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>

#include <comphelper/componentcontext.hxx>

namespace framework{

//_______________________________________________
// XInterface, XTypeProvider, XServiceInfo

DEFINE_XSERVICEINFO_MULTISERVICE(DispatchHelper                   ,
                                 ::cppu::OWeakObject              ,
                                 SERVICENAME_DISPATCHHELPER       ,
                                 IMPLEMENTATIONNAME_DISPATCHHELPER)

DEFINE_INIT_SERVICE( DispatchHelper, {} )

//_______________________________________________

/** ctor.

    @param xSMGR    the global uno service manager, which can be used to create own needed services.
*/
DispatchHelper::DispatchHelper( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR )
        :   ThreadHelpBase(     )
        // Init member
        ,   m_xSMGR       (xSMGR)
{
}

//_______________________________________________

/** dtor.
*/
DispatchHelper::~DispatchHelper()
{
}

//_______________________________________________

/** capsulate all steps of a dispatch request and provide so an easy way for dispatches.

    @param xDispatchProvider
                identifies the object, which provides may be valid dispatch objects for this execute.

    @param sURL
                describes the requested feature.

    @param sTargetFrameName
                points to the frame, which must be used (or may be created) for this dispatch.

    @param nSearchFlags
                in case the <var>sTargetFrameName</var> isn't unique, these flags regulate further searches.

    @param lArguments
                optional arguments for this request.

    @return An Any which capsulate a possible result of the internal wrapped dispatch.
 */
css::uno::Any SAL_CALL DispatchHelper::executeDispatch(
                                const css::uno::Reference< css::frame::XDispatchProvider >& xDispatchProvider ,
                                const ::rtl::OUString&                                      sURL              ,
                                const ::rtl::OUString&                                      sTargetFrameName  ,
                                      sal_Int32                                             nSearchFlags      ,
                                const css::uno::Sequence< css::beans::PropertyValue >&      lArguments        )
    throw(css::uno::RuntimeException)
{
    css::uno::Reference< css::uno::XInterface > xTHIS(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);

    // check for valid parameters
    if (
        (!xDispatchProvider.is()) ||
        (sURL.isEmpty()         )
       )
    {
        return css::uno::Any();
    }

    // parse given URL
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::util::XURLTransformer > xParser(css::util::URLTransformer::create(::comphelper::ComponentContext(m_xSMGR).getUNOContext()) );
    aReadLock.unlock();
    /* } SAFE */

    css::util::URL aURL;
    aURL.Complete = sURL;
    xParser->parseStrict(aURL);

    // search dispatcher
    css::uno::Reference< css::frame::XDispatch >          xDispatch       = xDispatchProvider->queryDispatch(aURL, sTargetFrameName, nSearchFlags);
    css::uno::Reference< css::frame::XNotifyingDispatch > xNotifyDispatch (xDispatch, css::uno::UNO_QUERY);

    // make sure that synchronous execution is used (if possible)
    css::uno::Sequence< css::beans::PropertyValue > aArguments( lArguments );
    sal_Int32 nLength = lArguments.getLength();
    aArguments.realloc( nLength + 1 );
    aArguments[ nLength ].Name = ::rtl::OUString("SynchronMode");
    aArguments[ nLength ].Value <<= (sal_Bool) sal_True;

    css::uno::Any aResult;
    if (xNotifyDispatch.is())
    {
        // dispatch it with guaranteed notification
        // Here we can hope for a result ... instead of the normal dispatch.
        css::uno::Reference< css::frame::XDispatchResultListener > xListener(xTHIS, css::uno::UNO_QUERY);
        /* SAFE { */
        WriteGuard aWriteLock(m_aLock);
        m_xBroadcaster = css::uno::Reference< css::uno::XInterface >(xNotifyDispatch, css::uno::UNO_QUERY);
        m_aResult      = css::uno::Any();
        m_aBlock.reset();
        aWriteLock.unlock();
        /* } SAFE */

        // dispatch it and wait for a notification
        // TODO/MBA: waiting in main thread?!
        xNotifyDispatch->dispatchWithNotification(aURL, aArguments, xListener);
        aResult = m_aResult;
    }
    else
    if (xDispatch.is())
    {
        // dispatch it without any chance to get a result
        xDispatch->dispatch( aURL, aArguments );
    }

    return aResult;
}

//_______________________________________________

/** callback for started dispatch with guaranteed notifications.

    We must save the result, so the method executeDispatch() can return it.
    Further we must release the broadcaster (otherwhise it can't die)
    and unblock the waiting executeDispatch() request.

    @param  aResult
                describes the result of the dispatch operation
 */
void SAL_CALL DispatchHelper::dispatchFinished( const css::frame::DispatchResultEvent& aResult )
    throw(css::uno::RuntimeException)
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);

    m_aResult <<= aResult;
    m_aBlock.set();
    m_xBroadcaster.clear();

    /* } SAFE */
}

//_______________________________________________

/** we has to realease our broadcaster reference.

    @param aEvent
                describe the source of this event and MUST be our save broadcaster!
 */
void SAL_CALL DispatchHelper::disposing( const css::lang::EventObject& )
    throw(css::uno::RuntimeException)
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);

    m_aResult.clear();
    m_aBlock.set();
    m_xBroadcaster.clear();

    /* } SAFE */
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
