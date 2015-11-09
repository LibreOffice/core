/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_UCB_SOURCE_UCP_CMIS_AUTH_PROVIDER_HXX
#define INCLUDED_UCB_SOURCE_UCP_CMIS_AUTH_PROVIDER_HXX

#include <libcmis/libcmis.hxx>

#include <com/sun/star/ucb/XCommandEnvironment.hpp>

namespace cmis
{
    class AuthProvider : public libcmis::AuthProvider
    {
        const css::uno::Reference< css::ucb::XCommandEnvironment>& m_xEnv;
        static css::uno::Reference< css::ucb::XCommandEnvironment> sm_xEnv;
        OUString m_sUrl;
        OUString m_sBindingUrl;

        public:
            AuthProvider ( const css::uno::Reference< css::ucb::XCommandEnvironment>& xEnv,
                           const OUString& sUrl,
                           const OUString& sBindingUrl ):
                m_xEnv( xEnv ), m_sUrl( sUrl ), m_sBindingUrl( sBindingUrl ) { }

            bool authenticationQuery( std::string& username, std::string& password ) override;

            static char* onedriveAuthCodeFallback( const char* url,
                    const char* /*username*/,
                    const char* /*password*/ );

            static void setXEnv( const css::uno::Reference< css::ucb::XCommandEnvironment>& xEnv ) { sm_xEnv = xEnv; }

            static css::uno::Reference< css::ucb::XCommandEnvironment> getXEnv( ) { return sm_xEnv; }
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
