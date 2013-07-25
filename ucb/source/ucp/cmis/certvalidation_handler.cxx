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
 */

#include <com/sun/star/security/XCertificate.hpp>
#include <com/sun/star/security/CertificateValidity.hpp>
#include <com/sun/star/xml/crypto/SEInitializer.hpp>
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>

#include <comphelper/sequence.hxx>
#include <ucbhelper/simplecertificatevalidationrequest.hxx>

#include "certvalidation_handler.hxx"

#define STD_TO_OUSTR( str ) OUString( str.c_str(), str.length( ), RTL_TEXTENCODING_UTF8 )

using namespace std;
using namespace com::sun::star;

namespace cmis
{
    bool CertValidationHandler::validateCertificate( vector< string > aCertificates )
    {
        bool bValidate = false;
        if ( !aCertificates.empty() && m_xEnv.is() )
        {
            uno::Reference< xml::crypto::XSEInitializer > xSEInitializer;
            try
            {
                xSEInitializer = xml::crypto::SEInitializer::create( m_xContext );
            }
            catch ( uno::Exception const & )
            {
            }

            if ( xSEInitializer.is() )
            {
                uno::Reference< xml::crypto::XXMLSecurityContext > xSecurityContext(
                    xSEInitializer->createSecurityContext( OUString() ) );

                uno::Reference< xml::crypto::XSecurityEnvironment > xSecurityEnv(
                        xSecurityContext->getSecurityEnvironment() );

                vector< string >::iterator pIt = aCertificates.begin();
                string sCert = *pIt;
                // We need to get rid of the PEM header/footer lines
                OUString sCleanCert = STD_TO_OUSTR( sCert );
                sCleanCert = sCleanCert.replaceAll( "-----BEGIN CERTIFICATE-----", "" );
                sCleanCert = sCleanCert.replaceAll( "-----END CERTIFICATE-----", "" );
                uno::Reference< security::XCertificate > xCert(
                        xSecurityEnv->createCertificateFromAscii(
                            sCleanCert ) );

                std::vector< uno::Reference< security::XCertificate > > vecCerts;

                for ( ++pIt; pIt != aCertificates.end(); ++pIt )
                {
                    sCert = *pIt;
                    uno::Reference< security::XCertificate> xImCert(
                        xSecurityEnv->createCertificateFromAscii(
                            STD_TO_OUSTR( sCert ) ) );
                    if ( xImCert.is() )
                        vecCerts.push_back( xImCert );
                }

                sal_Int64 certValidity = xSecurityEnv->verifyCertificate( xCert,
                    ::comphelper::containerToSequence( vecCerts ) );

                uno::Reference< task::XInteractionHandler > xIH(
                    m_xEnv->getInteractionHandler() );
                if ( xIH.is() )
                {
                    rtl::Reference< ucbhelper::SimpleCertificateValidationRequest >
                        xRequest( new ucbhelper::SimpleCertificateValidationRequest(
                                   sal_Int32( certValidity ), xCert, m_sHostname ) );
                    xIH->handle( xRequest.get() );
                    rtl::Reference< ucbhelper::InteractionContinuation > xSelection
                        = xRequest->getSelection();

                    if ( xSelection.is() )
                    {
                        uno::Reference< task::XInteractionApprove > xApprove(
                            xSelection.get(), uno::UNO_QUERY );
                        bValidate = xApprove.is();
                    }
                }
            }
        }
        return bValidate;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
