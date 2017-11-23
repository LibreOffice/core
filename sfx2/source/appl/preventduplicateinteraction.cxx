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

#include <preventduplicateinteraction.hxx>

#include <comphelper/processfactory.hxx>
#include <osl/diagnose.h>

#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionRetry.hpp>

namespace sfx2 {

PreventDuplicateInteraction::PreventDuplicateInteraction(const css::uno::Reference< css::uno::XComponentContext >& rxContext)
    : ThreadHelpBase2()
    , m_xContext(rxContext)
{
}

PreventDuplicateInteraction::~PreventDuplicateInteraction()
{
}

void PreventDuplicateInteraction::setHandler(const css::uno::Reference< css::task::XInteractionHandler >& xHandler)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    m_xWarningDialogsParent.reset();
    m_xHandler = xHandler;
    aLock.clear();
    // <- SAFE
}

void PreventDuplicateInteraction::useDefaultUUIHandler()
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    aLock.clear();
    // <- SAFE

    //if we use the default handler, set the parent to a window belonging to this object so that the dialogs
    //don't block unrelated windows.
    m_xWarningDialogsParent.reset(new WarningDialogsParentScope(m_xContext));
    css::uno::Reference<css::task::XInteractionHandler> xHandler(css::task::InteractionHandler::createWithParent(
        m_xContext, m_xWarningDialogsParent->GetDialogParent()), css::uno::UNO_QUERY_THROW);

    // SAFE ->
    aLock.reset();
    m_xHandler = xHandler;
    aLock.clear();
    // <- SAFE
}

css::uno::Any SAL_CALL PreventDuplicateInteraction::queryInterface( const css::uno::Type& aType )
{
    if ( aType.equals( cppu::UnoType<XInteractionHandler2>::get() ) )
    {
        ::osl::ResettableMutexGuard aLock(m_aLock);
        css::uno::Reference< css::task::XInteractionHandler2 > xHandler( m_xHandler, css::uno::UNO_QUERY );
        if ( !xHandler.is() )
            return css::uno::Any();
    }
    return ::cppu::WeakImplHelper<css::lang::XInitialization, css::task::XInteractionHandler2>::queryInterface(aType);
}

void SAL_CALL PreventDuplicateInteraction::handle(const css::uno::Reference< css::task::XInteractionRequest >& xRequest)
{
    css::uno::Any aRequest  = xRequest->getRequest();
    bool          bHandleIt = true;

    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    InteractionList::iterator pIt;
    for (  pIt  = m_lInteractionRules.begin();
           pIt != m_lInteractionRules.end();
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

    if ( bHandleIt && xHandler.is() )
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

sal_Bool SAL_CALL PreventDuplicateInteraction::handleInteractionRequest( const css::uno::Reference< css::task::XInteractionRequest >& xRequest )
{
    css::uno::Any aRequest  = xRequest->getRequest();
    bool      bHandleIt = true;

    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    InteractionList::iterator pIt;
    for (  pIt  = m_lInteractionRules.begin();
           pIt != m_lInteractionRules.end();
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

    if ( bHandleIt && xHandler.is() )
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

void PreventDuplicateInteraction::addInteractionRule(const PreventDuplicateInteraction::InteractionInfo& aInteractionInfo)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    InteractionList::iterator pIt;
    for (  pIt  = m_lInteractionRules.begin();
           pIt != m_lInteractionRules.end();
         ++pIt                               )
    {
        InteractionInfo& rInfo = *pIt;
        if (rInfo.m_aInteraction == aInteractionInfo.m_aInteraction)
        {
            rInfo.m_nMaxCount  = aInteractionInfo.m_nMaxCount;
            rInfo.m_nCallCount = aInteractionInfo.m_nCallCount;
            return;
        }
    }

    m_lInteractionRules.push_back(aInteractionInfo);

    aLock.clear();
    // <- SAFE
}

bool PreventDuplicateInteraction::getInteractionInfo(const css::uno::Type&                               aInteraction,
                                                           PreventDuplicateInteraction::InteractionInfo* pReturn     ) const
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    PreventDuplicateInteraction::InteractionList::const_iterator pIt;
    for (  pIt  = m_lInteractionRules.begin();
           pIt != m_lInteractionRules.end();
         ++pIt                               )
    {
        const PreventDuplicateInteraction::InteractionInfo& rInfo = *pIt;
        if (rInfo.m_aInteraction == aInteraction)
        {
            *pReturn = rInfo;
            return true;
        }
    }

    aLock.clear();
    // <- SAFE

    return false;
}

void SAL_CALL PreventDuplicateInteraction::initialize(const css::uno::Sequence<css::uno::Any>& rArguments)
{
    // If we're re-initialized to set a specific new window as a parent then drop our temporary
    // dialog parent
    css::uno::Reference<css::lang::XInitialization> xHandler(m_xHandler, css::uno::UNO_QUERY);
    if (xHandler.is())
    {
        m_xWarningDialogsParent.reset();
        xHandler->initialize(rArguments);
    }
}

IMPL_STATIC_LINK_NOARG(WarningDialogsParent, TerminateDesktop, void*, void)
{
    css::frame::Desktop::create(comphelper::getProcessComponentContext())->terminate();
}

} // namespace sfx2

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
