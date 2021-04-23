/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <rtl/uri.hxx>
#include <tools/urlobj.hxx>

#include "cmis_url.hxx"

using namespace std;

namespace cmis
{
    URL::URL( OUString const & urlStr )
    {
        INetURLObject aUrl( urlStr );

        // Decode the authority to get the binding URL and repository id
        OUString sDecodedHost = aUrl.GetHost( INetURLObject::DecodeMechanism::WithCharset );
        INetURLObject aHostUrl( sDecodedHost );
        m_sBindingUrl = aHostUrl.GetURLNoMark( );
        m_sRepositoryId = aHostUrl.GetMark( );

        m_sUser = aUrl.GetUser( INetURLObject::DecodeMechanism::WithCharset );
        m_sPass = aUrl.GetPass( INetURLObject::DecodeMechanism::WithCharset );

        // Store the path to the object
        m_sPath = aUrl.GetURLPath( INetURLObject::DecodeMechanism::WithCharset );
        m_sId = aUrl.GetMark( INetURLObject::DecodeMechanism::WithCharset );

        if ( m_sPath == "/" && m_sBindingUrl.indexOf( "google" ) != -1 )
            m_sId = "root";
    }


    void URL::setObjectPath( const OUString& sPath )
    {
        m_sPath = sPath;
    }

    void URL::setObjectId( const OUString& sId )
    {
        m_sId = sId;
    }

    void URL::setUsername( const OUString& sUser )
    {
        m_sUser = sUser;
    }

    OUString URL::asString( )
    {
        OUString sUrl;
        // Related tdf#96174, can no longer save on Google Drive
        // the user field may contain characters that need to be escaped according to
        // RFC3896 userinfo URI field
        // see <https://tools.ietf.org/html/rfc3986#section-3.2.1>
        OUString sEncodedUser = ( m_sUser.isEmpty() ?
                                   OUString() :
                                   rtl::Uri::encode( m_sUser, rtl_UriCharClassUserinfo,
                                                     rtl_UriEncodeIgnoreEscapes, RTL_TEXTENCODING_UTF8) );
        OUString sEncodedBinding = rtl::Uri::encode(
                m_sBindingUrl + "#" + m_sRepositoryId,
                rtl_UriCharClassRelSegment,
                rtl_UriEncodeKeepEscapes,
                RTL_TEXTENCODING_UTF8 );
        sUrl = "vnd.libreoffice.cmis://" +
                ( sEncodedUser.isEmpty() ? OUString( ) : (sEncodedUser + "@") ) +
                sEncodedBinding;

        if ( !m_sPath.isEmpty( ) )
        {
            sal_Int32 nPos = -1;
            OUStringBuffer sEncodedPath;
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
                    sEncodedPath.append("/" + rtl::Uri::encode( sSegment,
                            rtl_UriCharClassRelSegment,
                            rtl_UriEncodeKeepEscapes,
                            RTL_TEXTENCODING_UTF8 ));
                }
            }
            while ( nPos != -1 );
            sUrl += sEncodedPath.makeStringAndClear();
        } else if ( !m_sId.isEmpty( ) )
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
