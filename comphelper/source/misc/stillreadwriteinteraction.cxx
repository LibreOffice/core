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

#include <com/sun/star/task/XInteractionApprove.hpp>

#include <com/sun/star/ucb/UnsupportedDataSinkException.hpp>

#include <com/sun/star/ucb/AuthenticationRequest.hpp>

#include <com/sun/star/ucb/CertificateValidationRequest.hpp>

namespace comphelper{

StillReadWriteInteraction::StillReadWriteInteraction(const css::uno::Reference< css::task::XInteractionHandler >& xHandler,
                                                     const css::uno::Reference< css::task::XInteractionHandler >& xAuxiliaryHandler)
             : m_bUsed                    (false)
             , m_bHandledByMySelf         (false)
             , m_xAuxiliaryHandler(xAuxiliaryHandler)
{
    std::vector< ::ucbhelper::InterceptedInteraction::InterceptedRequest > lInterceptions;
    ::ucbhelper::InterceptedInteraction::InterceptedRequest                  aInterceptedRequest;

    aInterceptedRequest.Handle = HANDLE_INTERACTIVEIOEXCEPTION;
    aInterceptedRequest.Request <<= css::ucb::InteractiveIOException();
    aInterceptedRequest.Continuation = cppu::UnoType<css::task::XInteractionAbort>::get();
    lInterceptions.push_back(aInterceptedRequest);

    aInterceptedRequest.Handle = HANDLE_UNSUPPORTEDDATASINKEXCEPTION;
    aInterceptedRequest.Request <<= css::ucb::UnsupportedDataSinkException();
    aInterceptedRequest.Continuation = cppu::UnoType<css::task::XInteractionAbort>::get();
    lInterceptions.push_back(aInterceptedRequest);

    aInterceptedRequest.Handle = HANDLE_AUTHENTICATIONREQUESTEXCEPTION;
    aInterceptedRequest.Request <<= css::ucb::AuthenticationRequest();
    aInterceptedRequest.Continuation = cppu::UnoType<css::task::XInteractionApprove>::get();
    lInterceptions.push_back(aInterceptedRequest);

    aInterceptedRequest.Handle = HANDLE_CERTIFICATEVALIDATIONREQUESTEXCEPTION;
    aInterceptedRequest.Request <<= css::ucb::CertificateValidationRequest();
    aInterceptedRequest.Continuation = cppu::UnoType<css::task::XInteractionApprove>::get();
    lInterceptions.push_back(aInterceptedRequest);

    setInterceptedHandler(xHandler);
    setInterceptions(lInterceptions);
}

void StillReadWriteInteraction::resetInterceptions()
{
    setInterceptions(std::vector< ::ucbhelper::InterceptedInteraction::InterceptedRequest >());
}

void StillReadWriteInteraction::resetErrorStates()
{
    m_bUsed                     = false;
    m_bHandledByMySelf          = false;
}


ucbhelper::InterceptedInteraction::EInterceptionState StillReadWriteInteraction::intercepted(const ::ucbhelper::InterceptedInteraction::InterceptedRequest&                         aRequest,
                                                                  const css::uno::Reference< css::task::XInteractionRequest >& xRequest)
{
    // we are used!
    m_bUsed = true;

    // check if it's a real interception - might some parameters are not the right ones ...
    bool bAbort = false;
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
                // At least on Linux, a request to open some fuse-mounted file O_RDWR may fail with
                // EOPNOTSUPP (mapped to osl_File_E_NOSYS to IOErrorCode_NOT_SUPPORTED) when opening
                // it O_RDONLY would succeed:
                || (exIO.Code == css::ucb::IOErrorCode_NOT_SUPPORTED )
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
            bAbort = true;
        }
        break;
    case HANDLE_CERTIFICATEVALIDATIONREQUESTEXCEPTION:
    case HANDLE_AUTHENTICATIONREQUESTEXCEPTION:
       {
//use internal auxiliary handler and return
           if (m_xAuxiliaryHandler.is())
           {
               m_xAuxiliaryHandler->handle(xRequest);
               return ::ucbhelper::InterceptedInteraction::E_INTERCEPTED;
           }
           else //simply abort
               bAbort = true;
       }
       break;
    }

    // handle interaction by ourself
    if (bAbort)
    {
        m_bHandledByMySelf = true;
        css::uno::Reference< css::task::XInteractionContinuation > xAbort = ::ucbhelper::InterceptedInteraction::extractContinuation(
            xRequest->getContinuations(),
            cppu::UnoType<css::task::XInteractionAbort>::get() );
        if (!xAbort.is())
            return ::ucbhelper::InterceptedInteraction::E_NO_CONTINUATION_FOUND;
        xAbort->select();
        return ::ucbhelper::InterceptedInteraction::E_INTERCEPTED;
    }

    // Otherwise use internal handler.
    if (m_xInterceptedHandler.is())
    {
        m_xInterceptedHandler->handle(xRequest);
    }
    return ::ucbhelper::InterceptedInteraction::E_INTERCEPTED;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
