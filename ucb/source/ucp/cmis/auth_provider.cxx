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
                        OUString(), true, false );
                xIH->handle( xRequest.get() );

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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
