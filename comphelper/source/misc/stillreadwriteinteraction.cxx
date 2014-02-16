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

#include <comphelper/stillreadwriteinteraction.hxx>

#include <com/sun/star/ucb/InteractiveIOException.hpp>

#include <com/sun/star/task/XInteractionAbort.hpp>

#include <com/sun/star/ucb/UnsupportedDataSinkException.hpp>

namespace comphelper{

StillReadWriteInteraction::StillReadWriteInteraction(const css::uno::Reference< css::task::XInteractionHandler >& xHandler)
             : m_bUsed                    (sal_False)
             , m_bHandledByMySelf         (sal_False)
             , m_bHandledByInternalHandler(sal_False)
{
    ::std::vector< ::ucbhelper::InterceptedInteraction::InterceptedRequest > lInterceptions;
    ::ucbhelper::InterceptedInteraction::InterceptedRequest                  aInterceptedRequest;

    aInterceptedRequest.Handle = HANDLE_INTERACTIVEIOEXCEPTION;
    aInterceptedRequest.Request <<= css::ucb::InteractiveIOException();
    aInterceptedRequest.Continuation = ::getCppuType(static_cast< css::uno::Reference< css::task::XInteractionAbort >* >(0));
    aInterceptedRequest.MatchExact = false;
    lInterceptions.push_back(aInterceptedRequest);

    aInterceptedRequest.Handle = HANDLE_UNSUPPORTEDDATASINKEXCEPTION;
    aInterceptedRequest.Request <<= css::ucb::UnsupportedDataSinkException();
    aInterceptedRequest.Continuation = ::getCppuType(static_cast< css::uno::Reference< css::task::XInteractionAbort >* >(0));
    aInterceptedRequest.MatchExact = false;
    lInterceptions.push_back(aInterceptedRequest);

    setInterceptedHandler(xHandler);
    setInterceptions(lInterceptions);
}

void StillReadWriteInteraction::resetInterceptions()
{
    setInterceptions(::std::vector< ::ucbhelper::InterceptedInteraction::InterceptedRequest >());
}

void StillReadWriteInteraction::resetErrorStates()
{
    m_bUsed                     = sal_False;
    m_bHandledByMySelf          = sal_False;
    m_bHandledByInternalHandler = sal_False;
}

sal_Bool StillReadWriteInteraction::wasWriteError()
{
    return (m_bUsed && m_bHandledByMySelf);
}

ucbhelper::InterceptedInteraction::EInterceptionState StillReadWriteInteraction::intercepted(const ::ucbhelper::InterceptedInteraction::InterceptedRequest&                         aRequest,
                                                                  const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionRequest >& xRequest)
{
    // we are used!
    m_bUsed = sal_True;

    // check if it's a real interception - might some parameters are not the right ones ...
    sal_Bool bAbort = sal_False;
    switch(aRequest.Handle)
    {
    case HANDLE_INTERACTIVEIOEXCEPTION:
        {
            css::ucb::InteractiveIOException exIO;
            xRequest->getRequest() >>= exIO;
            bAbort = (
                (exIO.Code == css::ucb::IOErrorCode_ACCESS_DENIED     )
                || (exIO.Code == css::ucb::IOErrorCode_LOCKING_VIOLATION )
                || (exIO.Code == css::ucb::IOErrorCode_NOT_EXISTING )
#ifdef MACOSX
                // this is a workaround for MAC, on this platform if the file is locked
                // the returned error code looks to be wrong
                || (exIO.Code == css::ucb::IOErrorCode_GENERAL )
#endif
                );
        }
        break;

    case HANDLE_UNSUPPORTEDDATASINKEXCEPTION:
        {
            bAbort = sal_True;
        }
        break;
    }

    // handle interaction by ourself
    if (bAbort)
    {
        m_bHandledByMySelf = sal_True;
        css::uno::Reference< css::task::XInteractionContinuation > xAbort = ::ucbhelper::InterceptedInteraction::extractContinuation(
            xRequest->getContinuations(),
            ::getCppuType(static_cast< css::uno::Reference< css::task::XInteractionAbort >* >(0)));
        if (!xAbort.is())
            return ::ucbhelper::InterceptedInteraction::E_NO_CONTINUATION_FOUND;
        xAbort->select();
        return ::ucbhelper::InterceptedInteraction::E_INTERCEPTED;
    }

    // Otherwhise use internal handler.
    if (m_xInterceptedHandler.is())
    {
        m_bHandledByInternalHandler = sal_True;
        m_xInterceptedHandler->handle(xRequest);
    }
    return ::ucbhelper::InterceptedInteraction::E_INTERCEPTED;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
