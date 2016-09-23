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
#include <comphelper/sequence.hxx>

#include <osl/diagnose.h>

using namespace com::sun::star;
using namespace ucbhelper;


SimpleInteractionRequest::SimpleInteractionRequest(
                                    const uno::Any & rRequest,
                                    const ContinuationFlags nContinuations )
: InteractionRequest( rRequest )
{
    // Set continuations.
    OSL_ENSURE( nContinuations != ContinuationFlags::NONE,
                "SimpleInteractionRequest - No continuation!" );

    std::vector< uno::Reference< task::XInteractionContinuation > > aContinuations;
    if ( nContinuations & ContinuationFlags::Abort )
    {
        aContinuations.push_back( new InteractionAbort( this ) );
    }
    if ( nContinuations & ContinuationFlags::Retry )
    {
        aContinuations.push_back( new InteractionRetry( this ) );
    }
    if ( nContinuations & ContinuationFlags::Approve )
    {
        aContinuations.push_back( new InteractionApprove( this ) );
    }
    if (  nContinuations & ContinuationFlags::Disapprove )
    {
        aContinuations.push_back( new InteractionDisapprove( this ) );
    }
    setContinuations( comphelper::containerToSequence(aContinuations) );
}


ContinuationFlags SimpleInteractionRequest::getResponse() const
{
    rtl::Reference< InteractionContinuation > xSelection = getSelection();
    if ( xSelection.is() )
    {
        InteractionContinuation * pSelection = xSelection.get();

        uno::Reference< task::XInteractionAbort > xAbort(
                                        pSelection, uno::UNO_QUERY );
        if ( xAbort.is() )
            return ContinuationFlags::Abort;

        uno::Reference< task::XInteractionRetry > xRetry(
                                        pSelection, uno::UNO_QUERY );
        if ( xRetry.is() )
            return ContinuationFlags::Retry;

        uno::Reference< task::XInteractionApprove > xApprove(
                                        pSelection, uno::UNO_QUERY );
        if ( xApprove.is() )
            return ContinuationFlags::Approve;

        uno::Reference< task::XInteractionDisapprove > xDisapprove(
                                        pSelection, uno::UNO_QUERY );
        if ( xDisapprove.is() )
            return ContinuationFlags::Disapprove;

        OSL_FAIL( "SimpleInteractionRequest::getResponse - Unknown continuation!" );
    }
    return ContinuationFlags::NONE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
