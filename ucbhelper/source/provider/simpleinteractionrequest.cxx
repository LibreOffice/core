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
#include <ucbhelper/simpleinteractionrequest.hxx>

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
sal_Int32 SimpleInteractionRequest::getResponse() const
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

        OSL_FAIL( "SimpleInteractionRequest::getResponse - Unknown continuation!" );
    }
    return CONTINUATION_UNKNOWN;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
