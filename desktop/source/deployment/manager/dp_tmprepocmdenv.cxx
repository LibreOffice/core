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
#include "precompiled_desktop.hxx"

#include "com/sun/star/deployment/VersionException.hpp"
#include "com/sun/star/deployment/LicenseIndividualAgreementException.hpp"
#include "com/sun/star/deployment/LicenseException.hpp"
#include "com/sun/star/deployment/InstallException.hpp"
#include "com/sun/star/task/XInteractionApprove.hpp"
#include "com/sun/star/task/XInteractionAbort.hpp"
#include "com/sun/star/ucb/XCommandEnvironment.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "dp_tmprepocmdenv.hxx"

namespace deployment = com::sun::star::deployment;
namespace lang  = com::sun::star::lang;
namespace task = com::sun::star::task;
namespace ucb = com::sun::star::ucb;
namespace uno = com::sun::star::uno;
namespace css = com::sun::star;

#define OUSTR(s) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(s))

using ::com::sun::star::uno::Reference;
using ::rtl::OUString;

namespace dp_manager {

TmpRepositoryCommandEnv::TmpRepositoryCommandEnv()
{
}

TmpRepositoryCommandEnv::TmpRepositoryCommandEnv(
    Reference< css::task::XInteractionHandler> const & handler)
    : m_forwardHandler(handler)
{
}

TmpRepositoryCommandEnv::~TmpRepositoryCommandEnv()
{
}
// XCommandEnvironment
//______________________________________________________________________________
Reference<task::XInteractionHandler> TmpRepositoryCommandEnv::getInteractionHandler()
throw (uno::RuntimeException)
{
    return this;
}

//______________________________________________________________________________
Reference<ucb::XProgressHandler> TmpRepositoryCommandEnv::getProgressHandler()
throw (uno::RuntimeException)
{
    return this;
}

// XInteractionHandler
void TmpRepositoryCommandEnv::handle(
    Reference< task::XInteractionRequest> const & xRequest )
    throw (uno::RuntimeException)
{
    uno::Any request( xRequest->getRequest() );
    OSL_ASSERT( request.getValueTypeClass() == uno::TypeClass_EXCEPTION );

    deployment::VersionException verExc;
    deployment::LicenseException licExc;
    deployment::InstallException instExc;
    deployment::LicenseIndividualAgreementException licAgreementExc;


    bool approve = false;
    bool abort = false;

    if ((request >>= verExc)
        || (request >>= licExc)
        || (request >>= instExc)
        || (request >>= licAgreementExc))
    {
        approve = true;
    }

    if (approve == false && abort == false)
    {
        if (m_forwardHandler.is())
            m_forwardHandler->handle(xRequest);
        else
            approve = true;
    }
    else
    {
        // select:
        uno::Sequence< Reference< task::XInteractionContinuation > > conts(
            xRequest->getContinuations() );
        Reference< task::XInteractionContinuation > const * pConts =
            conts.getConstArray();
        sal_Int32 len = conts.getLength();
        for ( sal_Int32 pos = 0; pos < len; ++pos )
        {
            if (approve) {
                Reference< task::XInteractionApprove > xInteractionApprove(
                    pConts[ pos ], uno::UNO_QUERY );
                if (xInteractionApprove.is()) {
                    xInteractionApprove->select();
                    // don't query again for ongoing continuations:
                    approve = false;
                }
            }
            else if (abort) {
                Reference< task::XInteractionAbort > xInteractionAbort(
                    pConts[ pos ], uno::UNO_QUERY );
                if (xInteractionAbort.is()) {
                    xInteractionAbort->select();
                    // don't query again for ongoing continuations:
                    abort = false;
                }
            }
        }
    }
}

// XProgressHandler
void TmpRepositoryCommandEnv::push( uno::Any const & /*Status*/ )
throw (uno::RuntimeException)
{
}


void TmpRepositoryCommandEnv::update( uno::Any const & /*Status */)
throw (uno::RuntimeException)
{
}

void TmpRepositoryCommandEnv::pop() throw (uno::RuntimeException)
{
}


} // namespace dp_manager


