/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: simpleinteractionrequest.cxx,v $
 * $Revision: 1.5.18.1 $
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
#include "precompiled_configmgr.hxx"

#include "simpleinteractionrequest.hxx"

namespace configmgr { namespace apihelper {

namespace uno = com::sun::star::uno;
namespace task = com::sun::star::task;
//=========================================================================
SimpleInteractionRequest::SimpleInteractionRequest(
                                    const uno::Any & rRequest,
                                    const sal_uInt32 nContinuations )
: InteractionRequest( rRequest )
{
    // Set continuations.
    OSL_ENSURE( nContinuations != CONTINUATION_UNKNOWN,
                "SimpleInteractionRequest - No continuation!" );

    sal_Int32 nLength = 0;

    const sal_uInt32 k_NumContinuationTypes = 4;
    uno::Reference< task::XInteractionContinuation > xContinuations[ k_NumContinuationTypes ];

    if ( nContinuations & CONTINUATION_ABORT )
        xContinuations[nLength++] = new InteractionContinuation< task::XInteractionAbort >( this );

    if ( nContinuations & CONTINUATION_RETRY )
        xContinuations[nLength++] = new InteractionContinuation< task::XInteractionRetry >( this );

    if ( nContinuations & CONTINUATION_APPROVE )
        xContinuations[nLength++] = new InteractionContinuation< task::XInteractionApprove >( this );

    if (  nContinuations & CONTINUATION_DISAPPROVE )
        xContinuations[nLength++] = new InteractionContinuation< task::XInteractionDisapprove >( this );

    OSL_ENSURE( nLength > 0,
                "SimpleInteractionRequest - No continuation!" );

    uno::Sequence< uno::Reference< task::XInteractionContinuation > >
        aContinuations( xContinuations, nLength );

    this->setContinuations( aContinuations );
}

//=========================================================================
sal_uInt32 SimpleInteractionRequest::getResponse() const
{
    uno::Reference< task::XInteractionContinuation > xSelection = this->getSelection();
    if ( xSelection.is() )
    {
        if ( uno::Reference< task::XInteractionApprove >::query(xSelection).is() )
            return CONTINUATION_APPROVE;

        if ( uno::Reference< task::XInteractionDisapprove >::query(xSelection).is() )
            return CONTINUATION_DISAPPROVE;

        if ( uno::Reference< task::XInteractionRetry >::query(xSelection).is() )
            return CONTINUATION_RETRY;

        if ( uno::Reference< task::XInteractionAbort >::query(xSelection).is() )
            return CONTINUATION_ABORT;

        OSL_ENSURE( sal_False,
            "SimpleInteractionRequest::getResponse - Unknown continuation!" );
    }
    return CONTINUATION_UNKNOWN;
}

} }
