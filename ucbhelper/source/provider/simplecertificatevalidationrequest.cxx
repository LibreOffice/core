/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucbhelper.hxx"
#include <com/sun/star/ucb/CertificateValidationRequest.hpp>
#include <ucbhelper/simplecertificatevalidationrequest.hxx>

using namespace com::sun::star;
using namespace ucbhelper;

//=========================================================================
SimpleCertificateValidationRequest::SimpleCertificateValidationRequest( const sal_Int32 & lCertificateValidity,
                                                                        const com::sun::star::uno::Reference<com::sun::star::security::XCertificate> pCertificate,
                                                                        const rtl::OUString & hostname)
{
    // Fill request...
    ucb::CertificateValidationRequest aRequest;
    aRequest.CertificateValidity = lCertificateValidity;
    aRequest.Certificate = pCertificate;
    aRequest.HostName = hostname;

    setRequest( uno::makeAny( aRequest ) );

    uno::Sequence< uno::Reference< task::XInteractionContinuation > > aContinuations( 2 );
    aContinuations[ 0 ] = new InteractionAbort( this );
    aContinuations[ 1 ] = new InteractionApprove( this );

    setContinuations( aContinuations );
    pCertificate.get();
}

//=========================================================================
sal_Int32 SimpleCertificateValidationRequest::getResponse() const
{
    rtl::Reference< InteractionContinuation > xSelection = getSelection();
    if ( xSelection.is() )
    {
        InteractionContinuation * pSelection = xSelection.get();

        uno::Reference< task::XInteractionAbort > xAbort(
                                        pSelection, uno::UNO_QUERY );
        if ( xAbort.is() )
            return 1;

        uno::Reference< task::XInteractionRetry > xRetry(
                                        pSelection, uno::UNO_QUERY );
        if ( xRetry.is() )
            return 2;

        uno::Reference< task::XInteractionApprove > xApprove(
                                        pSelection, uno::UNO_QUERY );
        if ( xApprove.is() )
            return 4;

        uno::Reference< task::XInteractionDisapprove > xDisapprove(
                                        pSelection, uno::UNO_QUERY );
        if ( xDisapprove.is() )
            return 8;

        OSL_ENSURE( sal_False, "CertificateValidationRequest - Unknown continuation!" );
    }
    return 0;
}
