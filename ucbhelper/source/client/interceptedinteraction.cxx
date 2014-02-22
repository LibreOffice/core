/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <ucbhelper/interceptedinteraction.hxx>


namespace ucbhelper{

InterceptedInteraction::InterceptedInteraction()
{
}

void InterceptedInteraction::setInterceptedHandler(const css::uno::Reference< css::task::XInteractionHandler >& xInterceptedHandler)
{
    m_xInterceptedHandler = xInterceptedHandler;
}

void InterceptedInteraction::setInterceptions(const ::std::vector< InterceptedRequest >& lInterceptions)
{
    m_lInterceptions = lInterceptions;
}

InterceptedInteraction::EInterceptionState InterceptedInteraction::intercepted(
    const InterceptedRequest&,
    const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionRequest >&)
{
    
    return E_NOT_INTERCEPTED;
}

css::uno::Reference< css::task::XInteractionContinuation > InterceptedInteraction::extractContinuation(const css::uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation > >& lContinuations,
                                                                                                       const css::uno::Type&                                                                   aType         )
{
    const css::uno::Reference< css::task::XInteractionContinuation >* pContinuations = lContinuations.getConstArray();

    sal_Int32 c = lContinuations.getLength();
    sal_Int32 i = 0;

    for (i=0; i<c; ++i)
    {
        css::uno::Reference< css::uno::XInterface > xCheck(pContinuations[i], css::uno::UNO_QUERY);
        if (xCheck->queryInterface(aType).hasValue())
            return pContinuations[i];
    }

    return css::uno::Reference< css::task::XInteractionContinuation >();
}

void SAL_CALL InterceptedInteraction::handle(const css::uno::Reference< css::task::XInteractionRequest >& xRequest)
    throw(css::uno::RuntimeException)
{
    impl_handleDefault(xRequest);
}

void InterceptedInteraction::impl_handleDefault(const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionRequest >& xRequest)
{
    EInterceptionState eState = impl_interceptRequest(xRequest);

    switch(eState)
    {
        case E_NOT_INTERCEPTED:
        {
            
            
            if (m_xInterceptedHandler.is())
                m_xInterceptedHandler->handle(xRequest);
        }
        break;

        case E_NO_CONTINUATION_FOUND:
        {
            
            
            
            OSL_FAIL("InterceptedInteraction::handle()\nCould intercept this interaction request - but cant locate the right continuation!");
        }
        break;

        case E_INTERCEPTED:
        break;
    }
}

InterceptedInteraction::EInterceptionState InterceptedInteraction::impl_interceptRequest(const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionRequest >& xRequest)
{
    css::uno::Any                                                                    aRequest       = xRequest->getRequest();
    css::uno::Type                                                                   aRequestType   = aRequest.getValueType();
    css::uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation > > lContinuations = xRequest->getContinuations();

    
    sal_Int32 nHandle = 0;
    ::std::vector< InterceptedRequest >::const_iterator pIt;
    for (  pIt  = m_lInterceptions.begin();
           pIt != m_lInterceptions.end()  ;
         ++pIt                            )
    {
        const InterceptedRequest& rInterception = *pIt;
        css::uno::Type aInterceptedType = rInterception.Request.getValueType();

        
        bool bMatch = false;
        if (rInterception.MatchExact)
            bMatch = aInterceptedType.equals(aRequestType);
        else
            bMatch = aInterceptedType.isAssignableFrom(aRequestType); 

        
        
        
        
        if (bMatch)
        {
            EInterceptionState eState = intercepted(rInterception, xRequest);
            if (eState == E_NOT_INTERCEPTED)
                break;
            return eState;
        }

        ++nHandle;
    }

    if (pIt != m_lInterceptions.end()) 
    {
        
        const InterceptedRequest& rInterception = *pIt;
        css::uno::Reference< css::task::XInteractionContinuation > xContinuation = InterceptedInteraction::extractContinuation(lContinuations, rInterception.Continuation);
        if (xContinuation.is())
        {
            xContinuation->select();
            return E_INTERCEPTED;
        }

        
        
        return E_NO_CONTINUATION_FOUND;
    }

    return E_NOT_INTERCEPTED;
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
