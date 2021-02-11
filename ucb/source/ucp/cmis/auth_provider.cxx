/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#define OUSTR_TO_STDSTR(s) string( OUStringToOString( s, RTL_TEXTENCODING_UTF8 ).getStr() )
#define STD_TO_OUSTR( str ) OUString( str.c_str(), str.length( ), RTL_TEXTENCODING_UTF8 )

#include <com/sun/star/task/XInteractionHandler.hpp>

#include <ucbhelper/simpleauthenticationrequest.hxx>
#include <ucbhelper/authenticationfallback.hxx>

#include "auth_provider.hxx"

using namespace com::sun::star;
using namespace std;

namespace cmis
{
    bool AuthProvider::authenticationQuery( string& username, string& password )
    {
        if ( m_xEnv.is() )
        {
            uno::Reference< task::XInteractionHandler > xIH
                = m_xEnv->getInteractionHandler();

            if ( xIH.is() )
            {
                rtl::Reference< ucbhelper::SimpleAuthenticationRequest > xRequest
                    = new ucbhelper::SimpleAuthenticationRequest(
                        m_sUrl, m_sBindingUrl, OUString(),
                        STD_TO_OUSTR( username ),
                        STD_TO_OUSTR( password ),
                        false, false );
                xIH->handle( xRequest );

                rtl::Reference< ucbhelper::InteractionContinuation > xSelection
                    = xRequest->getSelection();

                if ( xSelection.is() )
                {
                    // Handler handled the request.
                    uno::Reference< task::XInteractionAbort > xAbort(
                        xSelection.get(), uno::UNO_QUERY );
                    if ( !xAbort.is() )
                    {
                        const rtl::Reference<
                            ucbhelper::InteractionSupplyAuthentication > & xSupp
                            = xRequest->getAuthenticationSupplier();

                        username = OUSTR_TO_STDSTR( xSupp->getUserName() );
                        password = OUSTR_TO_STDSTR( xSupp->getPassword() );

                        return true;
                    }
                }
            }
        }
        return false;
    }

    css::uno::WeakReference< css::ucb::XCommandEnvironment> AuthProvider::sm_xEnv;

    void AuthProvider::setXEnv(const css::uno::Reference< css::ucb::XCommandEnvironment>& xEnv )
    {
        sm_xEnv = xEnv;
    }

    css::uno::Reference< css::ucb::XCommandEnvironment> AuthProvider::getXEnv()
    {
        return sm_xEnv;
    }

    char* AuthProvider::onedriveAuthCodeFallback( const char* url,
            const char* /*username*/,
            const char* /*password*/ )
    {
        OUString url_oustr( url, strlen( url ), RTL_TEXTENCODING_UTF8 );
        const css::uno::Reference<
            css::ucb::XCommandEnvironment> xEnv = getXEnv( );

        if ( xEnv.is() )
        {
            uno::Reference< task::XInteractionHandler > xIH
                = xEnv->getInteractionHandler();

            if ( xIH.is() )
            {
                rtl::Reference< ucbhelper::AuthenticationFallbackRequest > xRequest
                    = new ucbhelper::AuthenticationFallbackRequest (
                            "Open the following link in your browser and "
                            "paste the code from the URL you have been redirected to in the "
                            "box below. For example:\n"
                            "http://localhost/LibreOffice?code=YOUR_CODE",
                            url_oustr );

                xIH->handle( xRequest );

                rtl::Reference< ucbhelper::InteractionContinuation > xSelection
                    = xRequest->getSelection();

                if ( xSelection.is() )
                {
                    // Handler handled the request.
                    const rtl::Reference< ucbhelper::InteractionAuthFallback >&
                        xAuthFallback = xRequest->getAuthFallbackInter( );
                    if ( xAuthFallback.is() )
                    {
                        OUString code = xAuthFallback->getCode( );
                        return strdup( OUSTR_TO_STDSTR( code ).c_str( ) );
                    }
                }
            }
        }

        return strdup( "" );
    }

    char* AuthProvider::gdriveAuthCodeFallback( const char* /*url*/,
            const char* /*username*/,
            const char* /*password*/ )
    {
        const css::uno::Reference<
            css::ucb::XCommandEnvironment> xEnv = getXEnv( );

        if ( xEnv.is() )
        {
            uno::Reference< task::XInteractionHandler > xIH
                = xEnv->getInteractionHandler();

            if ( xIH.is() )
            {
                rtl::Reference< ucbhelper::AuthenticationFallbackRequest > xRequest
                    = new ucbhelper::AuthenticationFallbackRequest (
                            "PIN:", "" );

                xIH->handle( xRequest );

                rtl::Reference< ucbhelper::InteractionContinuation > xSelection
                    = xRequest->getSelection();

                if ( xSelection.is() )
                {
                    // Handler handled the request.
                    const rtl::Reference< ucbhelper::InteractionAuthFallback >&
                        xAuthFallback = xRequest->getAuthFallbackInter( );
                    if ( xAuthFallback.is() )
                    {
                        OUString code = xAuthFallback->getCode( );
                        return strdup( OUSTR_TO_STDSTR( code ).c_str( ) );
                    }
                }
            }
        }

        return strdup( "" );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
