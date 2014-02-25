/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <ucbhelper/simpleinteractionrequest.hxx>

using namespace com::sun::star;
using namespace ucbhelper;


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
