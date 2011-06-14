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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucbhelper.hxx"
#include "ucbhelper/handleinteractionrequest.hxx"
#include "com/sun/star/task/XInteractionAbort.hpp"
#include "com/sun/star/task/XInteractionHandler.hpp"
#include "com/sun/star/task/XInteractionRetry.hpp"
#include "com/sun/star/ucb/CommandFailedException.hpp"
#include "com/sun/star/ucb/XCommandEnvironment.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "cppuhelper/exc_hlp.hxx"
#include "osl/diagnose.h"
#include "rtl/ustring.hxx"
#include "ucbhelper/interactionrequest.hxx"
#include "ucbhelper/simpleauthenticationrequest.hxx"
#include "ucbhelper/simpleinteractionrequest.hxx"
#include "ucbhelper/simplecertificatevalidationrequest.hxx"
#ifndef INCLUDED_UTILITY
#include <utility>
#define INCLUDED_UTILITY
#endif

using namespace com::sun::star;

namespace {

void
handle(uno::Reference< task::XInteractionRequest > const & rRequest,
       uno::Reference< ucb::XCommandEnvironment > const & rEnvironment)
    SAL_THROW((uno::Exception))
{
    OSL_ENSURE(rRequest.is(), "specification violation");
    uno::Reference< task::XInteractionHandler > xHandler;
    if (rEnvironment.is())
        xHandler = rEnvironment->getInteractionHandler();
    if (!xHandler.is())
        cppu::throwException(rRequest->getRequest());
    xHandler->handle(rRequest.get());
}

}

namespace ucbhelper {

sal_Int32
handleInteractionRequest(
    rtl::Reference< ucbhelper::SimpleInteractionRequest > const & rRequest,
    uno::Reference< ucb::XCommandEnvironment > const & rEnvironment,
    bool bThrowOnAbort)
    SAL_THROW((uno::Exception))
{
    handle(rRequest.get(), rEnvironment);
    sal_Int32 nResponse = rRequest->getResponse();
    switch (nResponse)
    {
    case ucbhelper::CONTINUATION_UNKNOWN:
        cppu::throwException(rRequest->getRequest());
        break;

    case ucbhelper::CONTINUATION_ABORT:
        if (bThrowOnAbort)
            throw ucb::CommandFailedException(
                      rtl::OUString(), 0, rRequest->getRequest());
        break;
    }
    return nResponse;
}

std::pair< sal_Int32,
           rtl::Reference< ucbhelper::InteractionSupplyAuthentication > >
handleInteractionRequest(
    rtl::Reference< ucbhelper::SimpleAuthenticationRequest > const & rRequest,
    uno::Reference< ucb::XCommandEnvironment > const & rEnvironment,
    bool bThrowOnAbort)
    SAL_THROW((uno::Exception))
{
    handle(rRequest.get(), rEnvironment);
    rtl::Reference< ucbhelper::InteractionContinuation >
        xContinuation(rRequest->getSelection());
    if (uno::Reference< task::XInteractionAbort >(
                xContinuation.get(), uno::UNO_QUERY).
            is())
        if (bThrowOnAbort)
            throw ucb::CommandFailedException(
                      rtl::OUString(), 0, rRequest->getRequest());
        else
            return std::make_pair(
                       ucbhelper::CONTINUATION_ABORT,
                       rtl::Reference<
                           ucbhelper::InteractionSupplyAuthentication >());
    else if (uno::Reference< task::XInteractionRetry >(
                     xContinuation.get(), uno::UNO_QUERY).
                 is())
        return std::make_pair(
                   ucbhelper::CONTINUATION_ABORT,
                   rtl::Reference<
                       ucbhelper::InteractionSupplyAuthentication >());
    else
        return std::make_pair(
                   ucbhelper::CONTINUATION_UNKNOWN,
                   rtl::Reference<
                       ucbhelper::InteractionSupplyAuthentication >(
                           rRequest->getAuthenticationSupplier()));
}

}

namespace ucbhelper {

sal_Int32
handleInteractionRequest(
    rtl::Reference< ucbhelper::SimpleCertificateValidationRequest > const & rRequest,
    uno::Reference< ucb::XCommandEnvironment > const & rEnvironment,
    bool bThrowOnAbort)
    SAL_THROW((uno::Exception))
{
    handle(rRequest.get(), rEnvironment);
    sal_Int32 nResponse = rRequest->getResponse();
    switch (nResponse)
    {
    case ucbhelper::CONTINUATION_UNKNOWN:
        cppu::throwException(rRequest->getRequest());
        break;

    case ucbhelper::CONTINUATION_ABORT:
        if (bThrowOnAbort)
            throw ucb::CommandFailedException(
                      rtl::OUString(), 0, rRequest->getRequest());
        break;
    }
    return nResponse;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
