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

#ifndef INCLUDED_UCBHELPER_HANDLEINTERACTIONREQUEST_HXX
#define INCLUDED_UCBHELPER_HANDLEINTERACTIONREQUEST_HXX

#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "rtl/ref.hxx"
#include "sal/types.h"

#ifndef INCLUDED_UTILITY
#include <utility>
#define INCLUDED_UTILITY
#endif

namespace com { namespace sun { namespace star { namespace ucb {
    class XCommandEnvironment;
} } } }
namespace ucbhelper {
    class InteractionSupplyAuthentication;
    class SimpleAuthenticationRequest;
    class SimpleInteractionRequest;
    class SimpleCertificateValidationRequest;
}

/** Pass a <type scope="ucbhelper">SimpleInteractionRequest</type> to an
    <type scope="com::sun::star::task">XInteractionHandler</type>, and handle
    (by throwing the request as an exception) those cases where an interaction
    handler is either not available or does not handle the request.

    @param rRequest
    a <type scope="ucbhelper">SimpleInteractionRequest</type>.  Must not be
    <NULL/>.

    @param rEnvironment
    At the moment, only the
    <type scope="com::sun::star::task">XInteractionHandler</type> part is
    used.  May be <NULL/>.

    @param bThrowOnAbort
    determines what is done if the interaction handler selects a
    <const scope="ucbhelper">CONTINUATION_ABORT</const> continuation:  If
    <TRUE/>, an appropriate
    <type scope="com::sun::star::ucb">CommandFailedException</type> is thrown.
    If <FALSE/>, <const scope="ucbhelper">CONTINUATION_ABORT</const> is passed
    to the caller of this function.

    @returns
    the constant (defined in ucbhelper/simpelinteractionrequest.hxx) that
    corresponds to the continuation selected by the interaction handler.
    The constant <const scope="ucbhelper">CONTINUATION_UNKNOWN</const> will
    never be returned.

    @throws
    <ul>
    <li>the exception specified by the request, if an interaction handler is
    either not available or does not handle the request;</li>
    <li>a <type scope="com::sun::star::ucb">CommandFailedException</type> if
    the interaction handler selects a
    <const scope="ucbhelper">CONTINUATION_ABORT</const> continuation and
    <code>bThrowOnAbort</code> is <TRUE/>;</li>
    <li>a <type scope="com::sun::star::uno">RuntimeException</type> if such an
    exception is thrown by code called from within this function.</li>
    </ul>
 */
namespace ucbhelper {

sal_Int32
handleInteractionRequest(
    rtl::Reference< ucbhelper::SimpleInteractionRequest > const & rRequest,
    com::sun::star::uno::Reference<
            com::sun::star::ucb::XCommandEnvironment > const &
        rEnvironment,
    bool bThrowOnAbort = true)
    SAL_THROW((com::sun::star::uno::Exception));

}

/** Pass a <type scope="ucbhelper">SimpleAuthenticationRequest</type> to an
    <type scope="com::sun::star::task">XInteractionHandler</type>, and handle
    (by throwing the request as an exception) those cases where an interaction
    handler is either not available or does not handle the request.

    @param rRequest
    a <type scope="ucbhelper">SimpleAuthenticationRequest</type>.  Must not be
    <NULL/>.

    @param rEnvironment
    At the moment, only the
    <type scope="com::sun::star::task">XInteractionHandler</type> part is
    used.  May be <NULL/>.

    @param bThrowOnAbort
    determines what is done if the interaction handler selects a
    <const scope="ucbhelper">CONTINUATION_ABORT</const> continuation:  If
    <TRUE/>, an appropriate
    <type scope="com::sun::star::ucb">CommandFailedException</type> is thrown.
    If <FALSE/>, <const scope="ucbhelper">CONTINUATION_ABORT</const> is passed
    to the caller of this function.

    @returns
    either a pair consisting of one of the constants
    <const scope="ucbhelper">CONTINUATION_ABORT</const> or
    <const scope="ucbhelper">CONTINUATION_RETRY</const> (defined in
    ucbhelper/simpelinteractionrequest.hxx) and an empty reference, or a pair
    consisting of the constant
    <const scope="ucbhelper">CONTINUATION_UNKNOWN</const> and a reference to
    an <type scope="ucbhelper">InteractionSupplyAuthentication</type> that
    contains the supplied data.

    @throws
    <ul>
    <li>the exception specified by the request, if an interaction handler is
    either not available or does not handle the request;</li>
    <li>a <type scope="com::sun::star::ucb">CommandFailedException</type> if
    the interaction handler selects a
    <const scope="ucbhelper">CONTINUATION_ABORT</const> continuation and
    <code>bThrowOnAbort</code> is <TRUE/>;</li>
    <li>a <type scope="com::sun::star::uno">RuntimeException</type> if such an
    exception is thrown by code called from within this function.</li>
    </ul>
 */
namespace ucbhelper {

std::pair< sal_Int32,
           rtl::Reference< ucbhelper::InteractionSupplyAuthentication > >
handleInteractionRequest(
    rtl::Reference< ucbhelper::SimpleAuthenticationRequest > const & rRequest,
    com::sun::star::uno::Reference<
            com::sun::star::ucb::XCommandEnvironment > const &
        rEnvironment,
    bool bThrowOnAbort = true)
    SAL_THROW((com::sun::star::uno::Exception));

}

/** Pass a <type scope="ucbhelper">SimpleCertificateValidationRequest</type> to an
    <type scope="com::sun::star::task">XInteractionHandler</type>, and handle
    (by throwing the request as an exception) those cases where an interaction
    handler is either not available or does not handle the request.

    @param rRequest
    a <type scope="ucbhelper">SimpleCertificateValidationRequest</type>.  Must not be
    <NULL/>.

    @param rEnvironment
    At the moment, only the
    <type scope="com::sun::star::task">XInteractionHandler</type> part is
    used.  May be <NULL/>.

    @param bThrowOnAbort
    determines what is done if the interaction handler selects a
    <const scope="ucbhelper">CONTINUATION_ABORT</const> continuation:  If
    <TRUE/>, an appropriate
    <type scope="com::sun::star::ucb">CommandFailedException</type> is thrown.
    If <FALSE/>, <const scope="ucbhelper">CONTINUATION_ABORT</const> is passed
    to the caller of this function.

   @returns
    the constant (defined in ucbhelper/simpelinteractionrequest.hxx) that
    corresponds to the continuation selected by the interaction handler.
    The constant <const scope="ucbhelper">CONTINUATION_UNKNOWN</const> will
    never be returned.

    @throws
    <ul>
    <li>the exception specified by the request, if an interaction handler is
    either not available or does not handle the request;</li>
    <li>a <type scope="com::sun::star::ucb">CommandFailedException</type> if
    the interaction handler selects a
    <const scope="ucbhelper">CONTINUATION_ABORT</const> continuation and
    <code>bThrowOnAbort</code> is <TRUE/>;</li>
    <li>a <type scope="com::sun::star::uno">RuntimeException</type> if such an
    exception is thrown by code called from within this function.</li>
    </ul>
 */
namespace ucbhelper {

sal_Int32
handleInteractionRequest(
    rtl::Reference< ucbhelper::SimpleCertificateValidationRequest > const & rRequest,
    com::sun::star::uno::Reference<
            com::sun::star::ucb::XCommandEnvironment > const &
        rEnvironment,
    bool bThrowOnAbort = true)
    SAL_THROW((com::sun::star::uno::Exception));

}
#endif // INCLUDED_UCBHELPER_HANDLEINTERACTIONREQUEST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
