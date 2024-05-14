/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#define OUSTR_TO_STDSTR(s) std::string( OUStringToOString( s, RTL_TEXTENCODING_UTF8 ) )
#define STD_TO_OUSTR( str ) OUString( str.c_str(), str.length( ), RTL_TEXTENCODING_UTF8 )

#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/PasswordContainer.hpp>
#include <com/sun/star/task/XPasswordContainer2.hpp>

#include <comphelper/processfactory.hxx>
#include <ucbhelper/simpleauthenticationrequest.hxx>
#include <ucbhelper/authenticationfallback.hxx>

#include "auth_provider.hxx"

using namespace com::sun::star;

namespace cmis
{
    bool AuthProvider::authenticationQuery( std::string& username, std::string& password )
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

    std::string AuthProvider::getRefreshToken(std::string& rUsername)
    {
        std::string refreshToken;
        const css::uno::Reference<css::ucb::XCommandEnvironment> xEnv = getXEnv();
        if (xEnv.is())
        {
            uno::Reference<task::XInteractionHandler> xIH = xEnv->getInteractionHandler();

            if (rUsername.empty())
            {
                rtl::Reference<ucbhelper::SimpleAuthenticationRequest> xRequest
                    = new ucbhelper::SimpleAuthenticationRequest(
                        m_sUrl, m_sBindingUrl,
                        ucbhelper::SimpleAuthenticationRequest::EntityType::ENTITY_NA, OUString(),
                        ucbhelper::SimpleAuthenticationRequest::EntityType::ENTITY_MODIFY,
                        STD_TO_OUSTR(rUsername),
                        ucbhelper::SimpleAuthenticationRequest::EntityType::ENTITY_NA, OUString());
                xIH->handle(xRequest);

                rtl::Reference<ucbhelper::InteractionContinuation> xSelection
                    = xRequest->getSelection();

                if (xSelection.is())
                {
                    // Handler handled the request.
                    uno::Reference<task::XInteractionAbort> xAbort(xSelection.get(),
                                                                   uno::UNO_QUERY);
                    if (!xAbort.is())
                    {
                        const rtl::Reference<ucbhelper::InteractionSupplyAuthentication>& xSupp
                            = xRequest->getAuthenticationSupplier();

                        rUsername = OUSTR_TO_STDSTR(xSupp->getUserName());
                    }
                }
            }

            uno::Reference<uno::XComponentContext> xContext
                = ::comphelper::getProcessComponentContext();
            uno::Reference<task::XPasswordContainer2> xMasterPasswd
                = task::PasswordContainer::create(xContext);
            if (xMasterPasswd->hasMasterPassword())
            {
                xMasterPasswd->authorizateWithMasterPassword(xIH);
            }
            if (xMasterPasswd->isPersistentStoringAllowed())
            {
                task::UrlRecord aRec
                    = xMasterPasswd->findForName(m_sBindingUrl, STD_TO_OUSTR(rUsername), xIH);
                if (aRec.UserList.hasElements() && aRec.UserList[0].Passwords.hasElements())
                    refreshToken = OUSTR_TO_STDSTR(aRec.UserList[0].Passwords[0]);
            }
        }
        return refreshToken;
    }

    bool AuthProvider::storeRefreshToken(const std::string& username, const std::string& password,
                                         const std::string& refreshToken)
    {
        if (refreshToken.empty())
            return false;
        if (password == refreshToken)
            return true;
        const css::uno::Reference<css::ucb::XCommandEnvironment> xEnv = getXEnv();
        if (xEnv.is())
        {
            uno::Reference<task::XInteractionHandler> xIH = xEnv->getInteractionHandler();
            uno::Reference<uno::XComponentContext> xContext
                = ::comphelper::getProcessComponentContext();
            uno::Reference<task::XPasswordContainer2> xMasterPasswd
                = task::PasswordContainer::create(xContext);
            uno::Sequence<OUString> aPasswd{ STD_TO_OUSTR(refreshToken) };
            if (xMasterPasswd->isPersistentStoringAllowed())
            {
                if (xMasterPasswd->hasMasterPassword())
                {
                    xMasterPasswd->authorizateWithMasterPassword(xIH);
                }
                xMasterPasswd->addPersistent(m_sBindingUrl, STD_TO_OUSTR(username), aPasswd, xIH);
                return true;
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

    char* AuthProvider::copyWebAuthCodeFallback( const char* url,
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
                            u"Open the following link in your browser and "
                            "paste the code from the URL you have been redirected to in the "
                            "box below. For example:\n"
                            "http://localhost/LibreOffice?code=YOUR_CODE"_ustr,
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
