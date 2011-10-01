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

#include <libcmis/session-factory.hxx>
#include <tools/urlobj.hxx>

#include "cmis_url.hxx"

using namespace std;

#define OUSTR_TO_STDSTR(s) string( rtl::OUStringToOString( s, RTL_TEXTENCODING_UTF8 ).getStr() )


namespace cmis
{
    URL::URL( rtl::OUString const & urlStr )
    {
        INetURLObject url( urlStr );

        string bindingUrl( "http://" );
        bindingUrl += OUSTR_TO_STDSTR( url.GetHostPort( ) );
        bindingUrl += OUSTR_TO_STDSTR( url.GetURLPath( ) );
        m_sBindingUrl = bindingUrl;

        // Split the query into bits and locate the repo-id key
        rtl::OUString query = url.GetParam( );
        while ( query.getLength() > 0 )
        {
            sal_Int32 nPos = query.indexOfAsciiL( "&", 1 );
            rtl::OUString segment;
            if ( nPos > 0 )
            {
                segment = query.copy( 0, nPos );
                query = query.copy( nPos + 1 );
            }
            else
            {
                segment = query;
                query = rtl::OUString();
            }

            sal_Int32 nEqPos = segment.indexOfAsciiL( "=", 1 );
            string key = OUSTR_TO_STDSTR( segment.copy( 0, nEqPos ) );
            string value = OUSTR_TO_STDSTR( segment.copy( nEqPos +1 ) );

            if ( key == "repo-id" )
                m_sRepositoryId = value;
            else
                m_aQuery[key] = value;
        }

    }

    map< int, string > URL::getSessionParams( )
    {
        map< int, string > params;
        params[ATOMPUB_URL] = m_sBindingUrl;
        params[REPOSITORY_ID] = m_sRepositoryId;

        return params;
    }

    string URL::getObjectId( )
    {
        return m_aQuery["id"];
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
