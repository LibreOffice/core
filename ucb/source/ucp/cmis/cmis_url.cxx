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

#include <rtl/uri.hxx>

#include "cmis_url.hxx"

using namespace std;

namespace cmis
{
    URL::URL( OUString const & urlStr )
    {
        INetURLObject aUrl( urlStr );

        // Decode the authority to get the binding URL and repository id
        OUString sDecodedHost = aUrl.GetHost( INetURLObject::DECODE_WITH_CHARSET );
        INetURLObject aHostUrl( sDecodedHost );
        m_sBindingUrl = aHostUrl.GetURLNoMark( );
        m_sRepositoryId = aHostUrl.GetMark( );

        m_sUser = aUrl.GetUser( INetURLObject::DECODE_WITH_CHARSET );
        m_sPass = aUrl.GetPass( INetURLObject::DECODE_WITH_CHARSET );

        // Store the path to the object
        m_sPath = aUrl.GetURLPath( INetURLObject::DECODE_WITH_CHARSET );
        m_sId = aUrl.GetMark( INetURLObject::DECODE_WITH_CHARSET );

        if ( !m_sId.isEmpty( ) )
            m_sPath = OUString( );
    }

    OUString& URL::getObjectPath( )
    {
        return m_sPath;
    }

    OUString& URL::getObjectId( )
    {
        return m_sId;
    }

    OUString& URL::getBindingUrl( )
    {
        return m_sBindingUrl;
    }

    OUString& URL::getRepositoryId( )
    {
        return m_sRepositoryId;
    }

    void URL::setObjectPath( OUString sPath )
    {
        m_sPath = sPath;
        m_sId = OUString( );
    }

    void URL::setObjectId( OUString sId )
    {
        m_sPath = OUString( );
        m_sId = sId;
    }

    OUString URL::asString( )
    {
        OUString sUrl;
        OUString sEncodedBinding = rtl::Uri::encode(
                m_sBindingUrl + "#" + m_sRepositoryId,
                rtl_UriCharClassRelSegment,
                rtl_UriEncodeKeepEscapes,
                RTL_TEXTENCODING_UTF8 );
        sUrl = "vnd.libreoffice.cmis://" + sEncodedBinding;

        if ( !m_sPath.isEmpty( ) )
        {
            sal_Int32 nPos = -1;
            OUString sEncodedPath;
            do
            {
                sal_Int32 nStartPos = nPos + 1;
                nPos = m_sPath.indexOf( '/', nStartPos );
                sal_Int32 nLen = nPos - nStartPos;
                if ( nPos == -1 )
                    nLen = m_sPath.getLength( ) - nStartPos;
                OUString sSegment = m_sPath.copy( nStartPos, nLen );

                if ( !sSegment.isEmpty( ) )
                {
                    sEncodedPath += "/" + rtl::Uri::encode( sSegment,
                            rtl_UriCharClassRelSegment,
                            rtl_UriEncodeKeepEscapes,
                            RTL_TEXTENCODING_UTF8 );
                }
            }
            while ( nPos != -1 );
            sUrl += sEncodedPath;
        }
        else if ( !m_sId.isEmpty( ) )
        {
            sUrl += "#" + rtl::Uri::encode( m_sId,
                rtl_UriCharClassRelSegment,
                rtl_UriEncodeKeepEscapes,
                RTL_TEXTENCODING_UTF8 );
        }

        return sUrl;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
