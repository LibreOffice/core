/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/simplefileaccessinteraction.hxx>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/ucb/AuthenticationRequest.hpp>
#include <com/sun/star/ucb/CertificateValidationRequest.hpp>
#include <com/sun/star/ucb/InteractiveIOException.hpp>
#include <com/sun/star/ucb/InteractiveNetworkException.hpp>
#include <com/sun/star/ucb/UnsupportedDataSinkException.hpp>

namespace comphelper
{
/// Will handle com::sun::star::ucb::InteractiveIOException and derived classes
const sal_Int32 HANDLE_INTERACTIVEIOEXCEPTION = 0;
/// Will handle com::sun::star::ucb::UnsupportedDataSinkException
const sal_Int32 HANDLE_UNSUPPORTEDDATASINKEXCEPTION = 1;
/// Will handle com::sun::star::ucb::InteractiveNetworkException
const sal_Int32 HANDLE_INTERACTIVENETWORKEXCEPTION = 2;
/// Will handle com::sun::star::ucb::CertificateValidationRequest
const sal_Int32 HANDLE_CERTIFICATEREQUEST = 3;
/// Will handle com::sun::star::ucb::AuthenticationRequest
const sal_Int32 HANDLE_AUTHENTICATIONREQUEST = 4;

SimpleFileAccessInteraction::SimpleFileAccessInteraction(
    const css::uno::Reference<css::task::XInteractionHandler>& xHandler)
{
    std::vector<::ucbhelper::InterceptedInteraction::InterceptedRequest> lInterceptions{
        { //intercept standard IO error exception (local file and WebDAV)
          css::uno::Any(css::ucb::InteractiveIOException()),
          cppu::UnoType<css::task::XInteractionAbort>::get(), HANDLE_INTERACTIVEIOEXCEPTION },
        { //intercept internal error
          css::uno::Any(css::ucb::UnsupportedDataSinkException()),
          cppu::UnoType<css::task::XInteractionAbort>::get(), HANDLE_UNSUPPORTEDDATASINKEXCEPTION },
        {
            //intercept network error exception (WebDAV ucp provider)
            css::uno::Any(css::ucb::InteractiveNetworkException()),
            cppu::UnoType<css::task::XInteractionAbort>::get(),
            HANDLE_INTERACTIVENETWORKEXCEPTION,
        },
        { //intercept certificate validation request (WebDAV ucp provider)
          css::uno::Any(css::ucb::CertificateValidationRequest()),
          cppu::UnoType<css::task::XInteractionAbort>::get(), HANDLE_CERTIFICATEREQUEST },
        { //intercept authentication request (WebDAV ucp provider)
          css::uno::Any(css::ucb::AuthenticationRequest()),
          cppu::UnoType<css::task::XInteractionApprove>::get(), HANDLE_AUTHENTICATIONREQUEST }
    };

    setInterceptedHandler(xHandler);
    setInterceptions(std::move(lInterceptions));
}

SimpleFileAccessInteraction::~SimpleFileAccessInteraction() {}

ucbhelper::InterceptedInteraction::EInterceptionState SimpleFileAccessInteraction::intercepted(
    const ::ucbhelper::InterceptedInteraction::InterceptedRequest& aRequest,
    const css::uno::Reference<css::task::XInteractionRequest>& xRequest)
{
    bool bAbort = false;
    switch (aRequest.Handle)
    {
        case HANDLE_UNSUPPORTEDDATASINKEXCEPTION:
        case HANDLE_INTERACTIVENETWORKEXCEPTION:
        case HANDLE_INTERACTIVEIOEXCEPTION:
        {
            bAbort = true;
        }
        break;

        case HANDLE_CERTIFICATEREQUEST:
        {
            // use default internal handler.
            if (m_xInterceptedHandler.is())
            {
                m_xInterceptedHandler->handle(xRequest);
                return ::ucbhelper::InterceptedInteraction::E_INTERCEPTED;
            }
            else
                bAbort = true;
            break;
        }

        case HANDLE_AUTHENTICATIONREQUEST:
        {
            // use default internal handler.
            if (m_xInterceptedHandler.is())
            {
                m_xInterceptedHandler->handle(xRequest);
                return ::ucbhelper::InterceptedInteraction::E_INTERCEPTED;
            }
            else //simply abort
                bAbort = true;
        }
        break;
    }

    // handle interaction by ourself, by not doing
    // any selection...
    if (bAbort)
    {
        css::uno::Reference<css::task::XInteractionContinuation> xAbort
            = ::ucbhelper::InterceptedInteraction::extractContinuation(
                xRequest->getContinuations(), cppu::UnoType<css::task::XInteractionAbort>::get());
        if (!xAbort.is())
            return ::ucbhelper::InterceptedInteraction::E_NO_CONTINUATION_FOUND;
        return ::ucbhelper::InterceptedInteraction::E_INTERCEPTED;
    }

    return ::ucbhelper::InterceptedInteraction::E_INTERCEPTED;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
