/*************************************************************************
 *
 *  $RCSfile: handleinteractionrequest.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sb $ $Date: 2001-07-13 12:48:52 $
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

#ifndef INCLUDED_UTILITY
#include <utility>
#define INCLUDED_UTILITY
#endif

using namespace com::sun;

namespace {

void
handle(star::uno::Reference< star::task::XInteractionRequest > const &
           rRequest,
       star::uno::Reference< star::ucb::XCommandEnvironment > const &
           rEnvironment)
    SAL_THROW((star::uno::Exception))
{
    OSL_ENSURE(rRequest.is(), "specification violation");
    star::uno::Reference< star::task::XInteractionHandler > xHandler;
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
    rtl::Reference< ucbhelper::SimpleInteractionRequest > const &
        rRequest,
    star::uno::Reference< star::ucb::XCommandEnvironment > const &
        rEnvironment,
    bool bThrowOnAbort)
    SAL_THROW((star::uno::Exception))
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
            throw star::ucb::CommandFailedException(
                      rtl::OUString(), 0, rRequest->getRequest());
        break;
    }
    return nResponse;
}

std::pair< sal_Int32,
           rtl::Reference< ucbhelper::InteractionSupplyAuthentication > >
handleInteractionRequest(
    rtl::Reference< ucbhelper::SimpleAuthenticationRequest > const & rRequest,
    com::sun::star::uno::Reference<
            com::sun::star::ucb::XCommandEnvironment > const &
        rEnvironment,
    bool bThrowOnAbort)
    SAL_THROW((com::sun::star::uno::Exception))
{
    handle(rRequest.get(), rEnvironment);
    rtl::Reference< ucbhelper::InteractionContinuation >
        xContinuation(rRequest->getSelection());
    if (star::uno::Reference< star::task::XInteractionAbort >(
                xContinuation.get(), star::uno::UNO_QUERY).
            is())
        if (bThrowOnAbort)
            throw star::ucb::CommandFailedException(
                      rtl::OUString(), 0, rRequest->getRequest());
        else
            return std::make_pair(
                       ucbhelper::CONTINUATION_ABORT,
                       rtl::Reference<
                           ucbhelper::InteractionSupplyAuthentication >());
    else if (star::uno::Reference< star::task::XInteractionRetry >(
                     xContinuation.get(), star::uno::UNO_QUERY).
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
