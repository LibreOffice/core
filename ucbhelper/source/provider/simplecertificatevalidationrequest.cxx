/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: simplecertificatevalidationrequest.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-11-07 10:08:55 $
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

#ifndef _COM_SUN_STAR_UCB_CERTIFICATEVALIDATIONREQUEST_HPP_
#include <com/sun/star/ucb/CertificateValidationRequest.hpp>
#endif

#ifndef _UCBHELPER_SIMPLECERTIFICATEVALIDATIONREQUEST_HXX
#include <ucbhelper/simplecertificatevalidationrequest.hxx>
#endif

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
const sal_Int32 SimpleCertificateValidationRequest::getResponse() const
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
