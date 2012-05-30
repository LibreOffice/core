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

#include "cmis_url.hxx"

using namespace std;

#define OUSTR_TO_STDSTR(s) string( rtl::OUStringToOString( s, RTL_TEXTENCODING_UTF8 ).getStr() )


namespace cmis
{
    URL::URL( rtl::OUString const & urlStr ) :
        m_aUrl( urlStr )
    {

        rtl::OUString bindingUrl( "http://" );
        bindingUrl += m_aUrl.GetHostPort( );
        bindingUrl += m_aUrl.GetURLPath( );
        m_sBindingUrl = bindingUrl;

        // Split the query into bits and locate the repo-id key
        rtl::OUString query = m_aUrl.GetParam( );
        while ( !query.isEmpty() )
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
            rtl::OUString key = segment.copy( 0, nEqPos );
            rtl::OUString value = segment.copy( nEqPos +1 );

            if ( key == "repo-id" )
                m_sRepositoryId = value;
            else
                m_aQuery[key] = value;
        }

    }

    map< int, string > URL::getSessionParams( )
    {
        map< int, string > params;
        params[ATOMPUB_URL] = OUSTR_TO_STDSTR( m_sBindingUrl );
        params[REPOSITORY_ID] = OUSTR_TO_STDSTR( m_sRepositoryId );
        params[USERNAME] = OUSTR_TO_STDSTR( m_aUrl.GetUser() );
        params[PASSWORD] = OUSTR_TO_STDSTR( m_aUrl.GetPass() );

        return params;
    }

    rtl::OUString URL::getObjectId( )
    {
        rtl::OUString sResult;
        map< rtl::OUString, rtl::OUString >::iterator it = m_aQuery.find( "id" );
        if ( it != m_aQuery.end( ) )
            sResult = it->second;
        return sResult;
    }

    rtl::OUString URL::getBindingUrl( )
    {
        return m_sBindingUrl;
    }

    void URL::setObjectId( rtl::OUString sId )
    {
        m_aQuery["id"] = sId;
        updateUrlQuery( );
    }

    rtl::OUString URL::asString( )
    {
        return m_aUrl.GetMainURL( INetURLObject::NO_DECODE );
    }

    void URL::updateUrlQuery( )
    {
        rtl::OUString sParam =  "repo-id=" + m_sRepositoryId + "&id=" + m_aQuery["id"];
        m_aUrl.SetParam( sParam );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
