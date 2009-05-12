/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: preventduplicateinteraction.cxx,v $
 * $Revision: 1.6 $
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

#include "interaction/preventduplicateinteraction.hxx"

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
