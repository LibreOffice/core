/*************************************************************************
 *
 *  $RCSfile: restricteduiinteraction.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: as $ $Date: 2002-08-12 11:45:06 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "interaction/restricteduiinteraction.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
#include <threadhelp/readguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
#include <threadhelp/writeguard.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_H_
#include <services.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONABORT_HPP_
#include <com/sun/star/task/XInteractionAbort.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONRETRY_HPP_
#include <com/sun/star/task/XInteractionRetry.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_INTERACTIVEIOEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveIOException.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_INTERACTIVENETWORKEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveNetworkException.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_INTERACTIVECHAOSEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveCHAOSException.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_INTERACTIVEWRONGMEDIUMEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveWrongMediumException.hpp>
#endif

#ifndef _COM_SUN_STAR_JAVA_WRONGJAVAVERSIONEXCEPTION_HPP_
#include <com/sun/star/java/WrongJavaVersionException.hpp>
#endif

#ifndef _COM_SUN_STAR_SYNC2_BADPARTNERSHIPEXCEPTION_HPP_
#include <com/sun/star/sync2/BadPartnershipException.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

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
