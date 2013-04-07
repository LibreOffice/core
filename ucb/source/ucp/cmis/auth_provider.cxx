/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * [ Copyright (C) 2011 SUSE <cbosdonnat@suse.com> (initial developer) ]
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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
