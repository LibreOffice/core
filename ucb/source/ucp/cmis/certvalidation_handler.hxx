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
#ifndef CERTVALIDATION_HANDLER_HXX
#define CERTVALIDATION_HANDLER_HXX

#include <libcmis/libcmis.hxx>

#include <com/sun/star/ucb/XCommandEnvironment.hpp>

namespace cmis
{
    class CertValidationHandler : public libcmis::CertValidationHandler
    {
        const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment>& m_xEnv;
        const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& m_xContext;
        OUString m_sHostname;

        public:
            CertValidationHandler (
                           const com::sun::star::uno::Reference<
                                   com::sun::star::ucb::XCommandEnvironment>& xEnv,
                           const com::sun::star::uno::Reference<
                                   com::sun::star::uno::XComponentContext>& xContext,
                           OUString sHostname ):
                m_xEnv( xEnv ), m_xContext( xContext ), m_sHostname( sHostname ) { }

            bool validateCertificate( std::vector< std::string > certificates );
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
