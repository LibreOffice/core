/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef AUTH_PROVIDER_HXX
#define AUTH_PROVIDER_HXX

#include <libcmis/libcmis.hxx>

#include <com/sun/star/ucb/XCommandEnvironment.hpp>

namespace cmis
{
    class AuthProvider : public libcmis::AuthProvider
    {
        const com::sun::star::uno::Reference< com::sun::star::ucb::XCommandEnvironment>& m_xEnv;
        OUString m_sUrl;
        OUString m_sBindingUrl;

        public:
            AuthProvider ( const com::sun::star::uno::Reference<
                                   com::sun::star::ucb::XCommandEnvironment>& xEnv,
                           OUString sUrl,
                           OUString sBindingUrl ):
                m_xEnv( xEnv ), m_sUrl( sUrl ), m_sBindingUrl( sBindingUrl ) { }

            bool authenticationQuery( std::string& username, std::string& password );
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
