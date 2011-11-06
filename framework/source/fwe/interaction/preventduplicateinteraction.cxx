/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

#include "framework/preventduplicateinteraction.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionRetry.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

namespace css = ::com::sun::star;

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

#define IMPLEMENTATIONNAME_UIINTERACTIONHANDLER                 ::rtl::OUString::createFromAscii("com.sun.star.comp.uui.UUIInteractionHandler")

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________

PreventDuplicateInteraction::PreventDuplicateInteraction(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
    : ThreadHelpBase2()
    , m_xSMGR(xSMGR)
{
}

//_________________________________________________________________________________________________________________

PreventDuplicateInteraction::~PreventDuplicateInteraction()
{
}

//_________________________________________________________________________________________________________________

void PreventDuplicateInteraction::setHandler(const css::uno::Reference< css::task::XInteractionHandler >& xHandler)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    m_xHandler = xHandler;
    aLock.clear();
    // <- SAFE
}

//_________________________________________________________________________________________________________________

void PreventDuplicateInteraction::useDefaultUUIHandler()
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    aLock.clear();
    // <- SAFE

    css::uno::Reference< css::task::XInteractionHandler > xHandler(
                            xSMGR->createInstance(IMPLEMENTATIONNAME_UIINTERACTIONHANDLER),
                            css::uno::UNO_QUERY_THROW);

    // SAFE ->
    aLock.reset();
    m_xHandler = xHandler;
    aLock.clear();
    // <- SAFE
}

//_________________________________________________________________________________________________________________
css::uno::Any SAL_CALL PreventDuplicateInteraction::queryInterface( const css::uno::Type& aType )
    throw (css::uno::RuntimeException)
{
    if ( aType.equals( XInteractionHandler2::static_type() ) )
    {
        ::osl::ResettableMutexGuard aLock(m_aLock);
        css::uno::Reference< css::task::XInteractionHandler2 > xHandler( m_xHandler, css::uno::UNO_QUERY );
        if ( !xHandler.is() )
            return css::uno::Any();
    }
    return ::cppu::WeakImplHelper1< css::task::XInteractionHandler2 >::queryInterface( aType );
}

//_________________________________________________________________________________________________________________

void SAL_CALL PreventDuplicateInteraction::handle(const css::uno::Reference< css::task::XInteractionRequest >& xRequest)
    throw(css::uno::RuntimeException)
{
    css::uno::Any aRequest  = xRequest->getRequest();
    sal_Bool      bHandleIt = sal_True;

    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    InteractionList::iterator pIt;
    for (  pIt  = m_lInteractionRules.begin();
           pIt != m_lInteractionRules.end()  ;
         ++pIt                               )
    {
        InteractionInfo& rInfo = *pIt;

        if (aRequest.isExtractableTo(rInfo.m_aInteraction))
        {
            ++rInfo.m_nCallCount;
            rInfo.m_xRequest = xRequest;
            bHandleIt = (rInfo.m_nCallCount <= rInfo.m_nMaxCount);
            break;
        }
    }

    css::uno::Reference< css::task::XInteractionHandler > xHandler = m_xHandler;

    aLock.clear();
    // <- SAFE

    if (
        (bHandleIt    ) &&
        (xHandler.is())
       )
    {
        xHandler->handle(xRequest);
    }
    else
    {
        const css::uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation > > lContinuations = xRequest->getContinuations();
        sal_Int32 c = lContinuations.getLength();
        sal_Int32 i = 0;
        for (i=0; i<c; ++i)
        {
            css::uno::Reference< css::task::XInteractionAbort > xAbort(lContinuations[i], css::uno::UNO_QUERY);
            if (xAbort.is())
            {
                xAbort->select();
                break;
            }
        }
    }
}

//_________________________________________________________________________________________________________________

::sal_Bool SAL_CALL PreventDuplicateInteraction::handleInteractionRequest( const css::uno::Reference< css::task::XInteractionRequest >& xRequest )
            throw (css::uno::RuntimeException)
{
    css::uno::Any aRequest  = xRequest->getRequest();
    sal_Bool      bHandleIt = sal_True;

    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    InteractionList::iterator pIt;
    for (  pIt  = m_lInteractionRules.begin();
           pIt != m_lInteractionRules.end()  ;
         ++pIt                               )
    {
        InteractionInfo& rInfo = *pIt;

        if (aRequest.isExtractableTo(rInfo.m_aInteraction))
        {
            ++rInfo.m_nCallCount;
            rInfo.m_xRequest = xRequest;
            bHandleIt = (rInfo.m_nCallCount <= rInfo.m_nMaxCount);
            break;
        }
    }

    css::uno::Reference< css::task::XInteractionHandler2 > xHandler( m_xHandler, css::uno::UNO_QUERY );
    OSL_ENSURE( xHandler.is() || !m_xHandler.is(),
        "PreventDuplicateInteraction::handleInteractionRequest: inconsistency!" );

    aLock.clear();
    // <- SAFE

    if (
        (bHandleIt    ) &&
        (xHandler.is())
       )
    {
        return xHandler->handleInteractionRequest(xRequest);
    }
    else
    {
        const css::uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation > > lContinuations = xRequest->getContinuations();
        sal_Int32 c = lContinuations.getLength();
        sal_Int32 i = 0;
        for (i=0; i<c; ++i)
        {
            css::uno::Reference< css::task::XInteractionAbort > xAbort(lContinuations[i], css::uno::UNO_QUERY);
            if (xAbort.is())
            {
                xAbort->select();
                break;
            }
        }
    }
    return false;
}

//_________________________________________________________________________________________________________________

void PreventDuplicateInteraction::addInteractionRule(const PreventDuplicateInteraction::InteractionInfo& aInteractionInfo)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    InteractionList::iterator pIt;
    for (  pIt  = m_lInteractionRules.begin();
           pIt != m_lInteractionRules.end()  ;
         ++pIt                               )
    {
        InteractionInfo& rInfo = *pIt;
        if (rInfo.m_aInteraction == aInteractionInfo.m_aInteraction)
        {
            rInfo.m_nMaxCount  = aInteractionInfo.m_nMaxCount ;
            rInfo.m_nCallCount = aInteractionInfo.m_nCallCount;
            return;
        }
    }

    m_lInteractionRules.push_back(aInteractionInfo);

    aLock.clear();
    // <- SAFE
}

//_________________________________________________________________________________________________________________

sal_Bool PreventDuplicateInteraction::getInteractionInfo(const css::uno::Type&                               aInteraction,
                                                               PreventDuplicateInteraction::InteractionInfo* pReturn     ) const
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    PreventDuplicateInteraction::InteractionList::const_iterator pIt;
    for (  pIt  = m_lInteractionRules.begin();
           pIt != m_lInteractionRules.end()  ;
         ++pIt                               )
    {
        const PreventDuplicateInteraction::InteractionInfo& rInfo = *pIt;
        if (rInfo.m_aInteraction == aInteraction)
        {
            *pReturn = rInfo;
            return sal_True;
        }
    }

    aLock.clear();
    // <- SAFE

    return sal_False;
}

} // namespace framework
