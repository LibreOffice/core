/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <libcmis/libcmis.hxx>

#include <rtl/uri.hxx>

#include "cmis_url.hxx"
#include "cmis_oauth2_providers.hxx"

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
        // Google Drive doesn't support getObjectByPath
        if ( m_sBindingUrl == GDRIVE_BASE_URL )
            m_sPath = OUString( );
        else
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
    }

    void URL::setObjectId( OUString sId )
    {
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
