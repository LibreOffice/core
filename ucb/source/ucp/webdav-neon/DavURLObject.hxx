/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_UCB_SOURCE_UCP_WEBDAV_NEON_DAVURLOBJECT_HXX
#define INCLUDED_UCB_SOURCE_UCP_WEBDAV_NEON_DAVURLOBJECT_HXX

#include <ne_uri.h>
#include <rtl/ustring.hxx>
#include <tools/urlobj.hxx>
#include "DAVException.hxx"

namespace webdav_ucp
{

    /// class to manage URL used in WebDAV
    // URL naming scheme (from URI RFC):
    //  foo://example.com:8042/over/there?name=ferret#nose
    //  \_/   \______________/\_________/ \_________/ \__/
    //   |           |            |            |        |
    // scheme     authority       path        query   fragment

    class DavURLObject : public INetURLObject
    {

    public:
        DavURLObject();
        explicit DavURLObject( const OUString& TheAbsRef );
        virtual ~DavURLObject();
        OUString GetMainURL( DecodeMechanism eMechanism = INetURLObject::NO_DECODE ) const {
            return INetURLObject::GetMainURL( eMechanism );
        }

        inline OUString GetPathQueryFragment( DecodeMechanism eMechanism = INetURLObject::NO_DECODE ) const;
        inline OUString GetPathBaseName() const;
        inline OUString GetPercDecodedPathBaseName() const;
        inline void verifyURL() const
            throw ( DAVException );
    };

    inline OUString DavURLObject::GetPathQueryFragment( DecodeMechanism eMechanism ) const
    {
        OUString PathQueryFragment( GetURLPath( eMechanism ) );
        if ( HasParam() )
        {
            PathQueryFragment += "?";
            PathQueryFragment += GetParam( eMechanism );
        }

        if ( HasMark() )
        {
            PathQueryFragment += "#";
            PathQueryFragment += GetMark( eMechanism );
        }
        return PathQueryFragment;
    };

    /// return the last segment of the path, escaped
    inline OUString DavURLObject::GetPathBaseName() const
    {
        OUString aRet( getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::NO_DECODE ) );
        if ( HasParam() )
        {
            aRet += "?";
            aRet += GetParam( INetURLObject::NO_DECODE );
        }

        if ( HasMark() )
        {
            aRet += "#";
            aRet += GetMark( INetURLObject::NO_DECODE );
        }
        return aRet;
    }

    /// return the last segment of the path, percent-decoded
    inline OUString DavURLObject::GetPercDecodedPathBaseName() const
    {
        OUString aRet( getName( INetURLObject::LAST_SEGMENT, true, DECODE_WITH_CHARSET ) );
        if ( HasParam() )
        {
            aRet += "?";
            aRet += GetParam( INetURLObject::NO_DECODE );
        }

        if ( HasMark() )
        {
            aRet += "#";
            aRet += GetMark( INetURLObject::NO_DECODE );
        }
        return aRet;
    }

    // Specific function member to be adapted to the support library
    // used for http transaction management.
    // This is for neon library.
    inline void DavURLObject::verifyURL() const
        throw ( DAVException )
    {
        if ( HasError() )
            throw DAVException( DAVException::DAV_INVALID_ARG );

        // the hext code verify the URL against neon libray needs
        OUString aEscapedUri( GetMainURL() );
        OString theInputUri(
            aEscapedUri.getStr(), aEscapedUri.getLength(), RTL_TEXTENCODING_UTF8 );

        ne_uri theUri;
        if ( ne_uri_parse( theInputUri.getStr(), &theUri ) != 0 )
        {
            ne_uri_free( &theUri );
            throw DAVException( DAVException::DAV_INVALID_ARG );
        }
        ne_uri_free( &theUri );
    }

}

#endif // INCLUDED_UCB_SOURCE_UCP_WEBDAV_NEON_DAVURI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
