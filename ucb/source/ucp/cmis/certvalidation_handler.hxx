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
#ifndef INCLUDED_UCB_SOURCE_UCP_CMIS_CERTVALIDATION_HANDLER_HXX
#define INCLUDED_UCB_SOURCE_UCP_CMIS_CERTVALIDATION_HANDLER_HXX

#include <libcmis/libcmis.hxx>

#include <com/sun/star/ucb/XCommandEnvironment.hpp>

namespace cmis
{
    class CertValidationHandler : public libcmis::CertValidationHandler
    {
        const css::uno::Reference< css::ucb::XCommandEnvironment>& m_xEnv;
        const css::uno::Reference< css::uno::XComponentContext >& m_xContext;
        OUString m_sHostname;

        public:
            CertValidationHandler (
                           const css::uno::Reference< css::ucb::XCommandEnvironment>& xEnv,
                           const css::uno::Reference< css::uno::XComponentContext>& xContext,
                           const OUString& sHostname ):
                m_xEnv( xEnv ), m_xContext( xContext ), m_sHostname( sHostname ) { }

            bool validateCertificate( std::vector< std::string > certificates ) override;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
