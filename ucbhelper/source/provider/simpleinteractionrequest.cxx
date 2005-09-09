/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: simpleinteractionrequest.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 16:41:54 $
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

#ifndef _UCBHELPER_SIMPLEINTERACTIONREQUEST_HXX
#include <ucbhelper/simpleinteractionrequest.hxx>
#endif

using namespace com::sun::star;
using namespace ucbhelper;

//=========================================================================
SimpleInteractionRequest::SimpleInteractionRequest(
                                    const uno::Any & rRequest,
                                    const sal_Int32 nContinuations )
: InteractionRequest( rRequest )
{
    // Set continuations.
    OSL_ENSURE( nContinuations != CONTINUATION_UNKNOWN,
                "SimpleInteractionRequest - No continuation!" );

    sal_Int32 nLength = 0;

    uno::Reference< task::XInteractionContinuation > xAbort;
    uno::Reference< task::XInteractionContinuation > xRetry;
    uno::Reference< task::XInteractionContinuation > xApprove;
    uno::Reference< task::XInteractionContinuation > xDisapprove;

    if ( nContinuations & CONTINUATION_ABORT )
    {
        ++nLength;
        xAbort = new InteractionAbort( this );
    }

    if ( nContinuations & CONTINUATION_RETRY )
    {
        ++nLength;
        xRetry = new InteractionRetry( this );
    }

    if ( nContinuations & CONTINUATION_APPROVE )
    {
        ++nLength;
        xApprove = new InteractionApprove( this );
    }

    if (  nContinuations & CONTINUATION_DISAPPROVE )
    {
        ++nLength;
        xDisapprove = new InteractionDisapprove( this );
    }

    OSL_ENSURE( nLength > 0,
                "SimpleInteractionRequest - No continuation!" );

    uno::Sequence< uno::Reference< task::XInteractionContinuation > >
        aContinuations( nLength );

    nLength = 0;

    if ( xAbort.is() )
        aContinuations[ nLength++ ] = xAbort;

    if ( xRetry.is() )
        aContinuations[ nLength++ ] = xRetry;

    if ( xApprove.is() )
        aContinuations[ nLength++ ] = xApprove;

    if ( xDisapprove.is() )
        aContinuations[ nLength++ ] = xDisapprove;

    setContinuations( aContinuations );
}

//=========================================================================
const sal_Int32 SimpleInteractionRequest::getResponse() const
{
    rtl::Reference< InteractionContinuation > xSelection = getSelection();
    if ( xSelection.is() )
    {
        InteractionContinuation * pSelection = xSelection.get();

        uno::Reference< task::XInteractionAbort > xAbort(
                                        pSelection, uno::UNO_QUERY );
        if ( xAbort.is() )
            return CONTINUATION_ABORT;

        uno::Reference< task::XInteractionRetry > xRetry(
                                        pSelection, uno::UNO_QUERY );
        if ( xRetry.is() )
            return CONTINUATION_RETRY;

        uno::Reference< task::XInteractionApprove > xApprove(
                                        pSelection, uno::UNO_QUERY );
        if ( xApprove.is() )
            return CONTINUATION_APPROVE;

        uno::Reference< task::XInteractionDisapprove > xDisapprove(
                                        pSelection, uno::UNO_QUERY );
        if ( xDisapprove.is() )
            return CONTINUATION_DISAPPROVE;

        OSL_ENSURE( sal_False,
            "SimpleInteractionRequest::getResponse - Unknown continuation!" );
    }
    return CONTINUATION_UNKNOWN;
}

