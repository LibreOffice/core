/*************************************************************************
 *
 *  $RCSfile: handleinteractionrequest.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sb $ $Date: 2001-07-13 12:45:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef INCLUDED_UCBHELPER_HANDLEINTERACTIONREQUEST_HXX
#define INCLUDED_UCBHELPER_HANDLEINTERACTIONREQUEST_HXX

#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include "com/sun/star/uno/Exception.hpp"
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include "com/sun/star/uno/Reference.hxx"
#endif
#ifndef _RTL_REF_HXX_
#include "rtl/ref.hxx"
#endif
#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

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

#endif // INCLUDED_UCBHELPER_HANDLEINTERACTIONREQUEST_HXX
