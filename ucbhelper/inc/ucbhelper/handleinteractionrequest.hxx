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
