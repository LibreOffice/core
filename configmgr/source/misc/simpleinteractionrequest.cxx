/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: simpleinteractionrequest.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 04:12:56 $
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

#include "simpleinteractionrequest.hxx"

namespace configmgr { namespace apihelper {

namespace uno = com::sun::star::uno;
namespace task = com::sun::star::task;
//=========================================================================
SimpleInteractionRequest::SimpleInteractionRequest(
                                    const uno::Any & rRequest,
                                    const Continuation nContinuations )
: InteractionRequest( rRequest )
{
    // Set continuations.
    OSL_ENSURE( nContinuations != CONTINUATION_UNKNOWN,
                "SimpleInteractionRequest - No continuation!" );

    sal_Int32 nLength = 0;

    const sal_uInt32 k_NumContinuationTypes = 4;
    uno::Reference< task::XInteractionContinuation > xContinuations[ k_NumContinuationTypes ];

    if ( nContinuations & CONTINUATION_ABORT )
        xContinuations[nLength++] = new InteractionAbort( this );

    if ( nContinuations & CONTINUATION_RETRY )
        xContinuations[nLength++] = new InteractionRetry( this );

    if ( nContinuations & CONTINUATION_APPROVE )
        xContinuations[nLength++] = new InteractionApprove( this );

    if (  nContinuations & CONTINUATION_DISAPPROVE )
        xContinuations[nLength++] = new InteractionDisapprove( this );

    OSL_ENSURE( nLength > 0,
                "SimpleInteractionRequest - No continuation!" );

    uno::Sequence< uno::Reference< task::XInteractionContinuation > >
        aContinuations( xContinuations, nLength );

    this->setContinuations( aContinuations );
}

//=========================================================================
const SimpleInteractionRequest::Continuation SimpleInteractionRequest::getResponse() const
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
