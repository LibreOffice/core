/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: handleinteractionrequest.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2007-11-07 10:08:29 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucbhelper.hxx"

#ifndef INCLUDED_UCBHELPER_HANDLEINTERACTIONREQUEST_HXX
#include "ucbhelper/handleinteractionrequest.hxx"
#endif

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONABORT_HPP_
#include "com/sun/star/task/XInteractionAbort.hpp"
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include "com/sun/star/task/XInteractionHandler.hpp"
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONRETRY_HPP_
#include "com/sun/star/task/XInteractionRetry.hpp"
#endif
#ifndef _COM_SUN_STAR_UCB_COMMANDFAILEDEXCEPTION_HPP_
#include "com/sun/star/ucb/CommandFailedException.hpp"
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDENVIRONMENT_HPP_
#include "com/sun/star/ucb/XCommandEnvironment.hpp"
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include "com/sun/star/uno/Reference.hxx"
#endif
#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include "com/sun/star/uno/RuntimeException.hpp"
#endif
#ifndef _CPPUHELPER_EXC_HLP_HXX_
#include "cppuhelper/exc_hlp.hxx"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include "osl/diagnose.h"
#endif
#ifndef _RTL_USTRING_HXX_
#include "rtl/ustring.hxx"
#endif
#ifndef _UCBHELPER_INTERACTIONREQUEST_HXX
#include "ucbhelper/interactionrequest.hxx"
#endif
#ifndef _UCBHELPER_SIMPLEAUTHENTICATIONREQUEST_HXX
#include "ucbhelper/simpleauthenticationrequest.hxx"
#endif
#ifndef _UCBHELPER_SIMPLEINTERACTIONREQUEST_HXX
#include "ucbhelper/simpleinteractionrequest.hxx"
#endif
#ifndef _UCBHELPER_SIMPLECERTIFICATEVALIDATIONREQUEST_HXX
#include "ucbhelper/simplecertificatevalidationrequest.hxx"
#endif
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

