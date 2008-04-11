/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: restricteduiinteraction.cxx,v $
 * $Revision: 1.4 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

#include "interaction/restricteduiinteraction.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>
#include <macros/generic.hxx>
#include <macros/debug.hxx>
#include <services.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionRetry.hpp>
#include <com/sun/star/ucb/InteractiveIOException.hpp>
#include <com/sun/star/ucb/InteractiveNetworkException.hpp>
#include <com/sun/star/ucb/InteractiveCHAOSException.hpp>
#include <com/sun/star/ucb/InteractiveWrongMediumException.hpp>
#include <com/sun/star/java/WrongJavaVersionException.hpp>
#include <com/sun/star/sync2/BadPartnershipException.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <vcl/svapp.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

DEFINE_XINTERFACE_2( RestrictedUIInteraction                          ,
                     OWeakObject                                      ,
                     DIRECT_INTERFACE(css::lang::XTypeProvider      ) ,
                     DIRECT_INTERFACE(css::task::XInteractionHandler) )

DEFINE_XTYPEPROVIDER_2( RestrictedUIInteraction        ,
                        css::lang::XTypeProvider       ,
                        css::task::XInteractionHandler )

//_________________________________________________________________________________________________________________

RestrictedUIInteraction::RestrictedUIInteraction( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR     ,
                                                        sal_Int32                                               nMaxRetry )
    : ThreadHelpBase          ( &Application::GetSolarMutex() )
    , ::cppu::OWeakObject     (                               )
    , m_aRequest              (                               )
    , m_nIORetry              ( 0                             )
    , m_nNetworkRetry         ( 0                             )
    , m_nChaosRetry           ( 0                             )
    , m_nWrongMediumRetry     ( 0                             )
    , m_nWrongJavaVersionRetry( 0                             )
    , m_nBadPartnershipRetry  ( 0                             )
    , m_nMaxRetry             ( nMaxRetry                     )
{
    m_xGenericUIHandler = css::uno::Reference< css::task::XInteractionHandler >(
                            xSMGR->createInstance(IMPLEMENTATIONNAME_UIINTERACTIONHANDLER),
                            css::uno::UNO_QUERY);
}

//_________________________________________________________________________________________________________________

void SAL_CALL RestrictedUIInteraction::handle( const css::uno::Reference< css::task::XInteractionRequest >& xRequest ) throw( css::uno::RuntimeException )
{
    // safe the request for outside analyzing everytime!
    css::uno::Any aRequest = xRequest->getRequest();
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);
    m_aRequest = aRequest;
    aWriteLock.unlock();
    /* } SAFE */

    // analyze the request
    // We need XAbort as possible continuation as minimum!
    // But we can use retry too, if it exist ...
    css::uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation > > lContinuations = xRequest->getContinuations();
    css::uno::Reference< css::task::XInteractionAbort >                              xAbort;
    css::uno::Reference< css::task::XInteractionRetry >                              xRetry;

    sal_Int32 nCount=lContinuations.getLength();
    for (sal_Int32 i=0; i<nCount; ++i)
    {
        if ( ! xAbort.is() )
            xAbort = css::uno::Reference< css::task::XInteractionAbort >( lContinuations[i], css::uno::UNO_QUERY );

        if ( ! xRetry.is() )
            xRetry = css::uno::Reference< css::task::XInteractionRetry >( lContinuations[i], css::uno::UNO_QUERY );
    }

    // differ between interactions for abort (io error)
    // and other ones (ambigous filter) which can be forwarded
    // to the generic UI handler

    // These interactions seams to inform the user only.
    // They can't solve any conflict realy.
    // But may some of them supports a retry. Then we use it.
    // Otherwhise we abort it - so the load request will fail.
    css::ucb::InteractiveIOException           aIoException                ;
    css::ucb::InteractiveNetworkException      aNetworkException           ;
    css::ucb::InteractiveCHAOSException        aChaosException             ;
    css::ucb::InteractiveWrongMediumException  aWrongMediumException       ;
    css::java::WrongJavaVersionException       aWrongJavaVersionException  ;
    css::sync2::BadPartnershipException        aBadPartnershipException    ;

    sal_Int32 nTriesBefore = 0       ;
    sal_Bool  bForward     = sal_True;

    /* SAFE { */
    aWriteLock.lock();

    if (aRequest >>= aIoException)
    {
        ++m_nIORetry;
        nTriesBefore = m_nIORetry;
        bForward     = sal_False;
    }
    else
    if (aRequest >>= aNetworkException)
    {
        ++m_nNetworkRetry;
        nTriesBefore = m_nNetworkRetry;
        bForward     = sal_False;
    }
    else
    if (aRequest >>= aChaosException)
    {
        ++m_nChaosRetry;
        nTriesBefore = m_nChaosRetry;
        bForward     = sal_False;
    }
    else
    if (aRequest >>= aWrongMediumException)
    {
        ++m_nWrongMediumRetry;
        nTriesBefore = m_nWrongMediumRetry;
        bForward     = sal_False;
    }
    else
    if (aRequest >>= aWrongJavaVersionException)
    {
        ++m_nWrongJavaVersionRetry;
        nTriesBefore = m_nWrongJavaVersionRetry;
        bForward     = sal_False;
    }
    else
    if (aRequest >>= aBadPartnershipException)
    {
        ++m_nBadPartnershipRetry;
        nTriesBefore = m_nBadPartnershipRetry;
        bForward     = sal_False;
    }

    // By the way - use the lock to get threadsafe member copies.
    sal_Int32                                               nMaxRetry = m_nMaxRetry;
    css::uno::Reference< css::task::XInteractionHandler >   xHandler  = m_xGenericUIHandler;

    aWriteLock.unlock();
    /* } SAFE */

    LOG_ASSERT(xHandler.is(), "RestrictedUIInteraction::handle()\nMiss generic UI handler to delegate request! Will do nothing ...")

    // It's a interaction which shouldn't be shown at the UI.
    // Look for possible retries and use it. Otherwhise abort it.
    if ( !bForward )
    {
        LOG_ASSERT(xAbort.is(), "RestrictedUIInteraction::handle()\nMiss \"Abort\" continuation as minimum. Will do nothing ... ")

        // It's a interaction which shouldn't be shown at the UI.
        // Look for possible retries and use it. Otherwhise abort it.
        if (nTriesBefore <= nMaxRetry && xRetry.is())
            xRetry->select();
        else
        if (xAbort.is())
            xAbort->select();
    }
    else
    // Otherwhise the request seams to show real dialogs to solve the conflict.
    // Delegate it to the generic UI handler.
    if (xHandler.is())
        xHandler->handle(xRequest);
}

//_________________________________________________________________________________________________________________

css::uno::Any RestrictedUIInteraction::getRequest() const
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    return m_aRequest;
    /* } SAFE */
}

//_________________________________________________________________________________________________________________

sal_Bool RestrictedUIInteraction::wasUsed() const
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    return m_aRequest.hasValue();
    /* } SAFE */
}

} // namespace framework
