/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <rtl/uri.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include "SerfUri.hxx"
#include "DAVException.hxx"
#include "AprEnv.hxx"

#include <urihelper.hxx>

using namespace http_dav_ucp;


SerfUri::SerfUri( const apr_uri_t * inUri )
    : mAprUri( *inUri )
    , mURI()
    , mScheme()
    , mUserInfo()
    , mHostName()
    , mPort()
    , mPath()
{
    if ( inUri == nullptr )
        throw DAVException( DAVException::DAV_INVALID_ARG );

    char * uri = apr_uri_unparse( apr_environment::AprEnv::getAprEnv()->getAprPool(), &mAprUri, 0 );

    if ( uri == nullptr )
        throw DAVException( DAVException::DAV_INVALID_ARG );

    init( &mAprUri );

    calculateURI();
}

SerfUri::SerfUri( const OUString & inUri )
    : mAprUri()
    , mURI()
    , mScheme()
    , mUserInfo()
    , mHostName()
    , mPort()
    , mPath()
{
    if ( inUri.getLength() <= 0 )
        throw DAVException( DAVException::DAV_INVALID_ARG );

    // #i77023#
    OUString aEscapedUri( ucb_impl::urihelper::encodeURI( inUri ) );

    OString theInputUri(
        aEscapedUri.getStr(), aEscapedUri.getLength(), RTL_TEXTENCODING_UTF8 );

    if ( apr_uri_parse( apr_environment::AprEnv::getAprEnv()->getAprPool(),
                        theInputUri.getStr(), &mAprUri ) != APR_SUCCESS )
    {
        throw DAVException( DAVException::DAV_INVALID_ARG );
    }
    if ( !mAprUri.port )
    {
        mAprUri.port = apr_uri_port_of_scheme( mAprUri.scheme );
    }
    if ( !mAprUri.path )
    {
        mAprUri.path = const_cast<char *>("/");
    }

    init( &mAprUri );

    calculateURI();
}

void SerfUri::init( const apr_uri_t * pUri )
{
    mScheme   = pUri->scheme ? OStringToOUString( pUri->scheme, RTL_TEXTENCODING_UTF8 ) : "";
    mUserInfo = pUri->user ? OStringToOUString( pUri->user, RTL_TEXTENCODING_UTF8 ) : "";
    mHostName = pUri->hostname ? OStringToOUString( pUri->hostname, RTL_TEXTENCODING_UTF8 ) : "";
    mPort     = pUri->port;
    mPath     = OStringToOUString( pUri->path, RTL_TEXTENCODING_UTF8 );

    if ( pUri->query )
    {
        mPath += "?";
        mPath += OStringToOUString( pUri->query,  RTL_TEXTENCODING_UTF8 );
    }

    if ( pUri->fragment )
    {
        mPath += "#";
        mPath += OStringToOUString( pUri->fragment,  RTL_TEXTENCODING_UTF8 );
    }
}

void SerfUri::calculateURI ()
{
    OUStringBuffer aBuf( mScheme );
    aBuf.append( "://" );
    if ( mUserInfo.getLength() > 0 )
    {
        aBuf.append( mUserInfo );
        aBuf.append( "@" );
    }
    // Is host a numeric IPv6 address?
    if ( ( mHostName.indexOf( ':' ) != -1 ) &&
         ( mHostName[ 0 ] != '[' ) )
    {
        aBuf.append( "[" );
        aBuf.append( mHostName );
        aBuf.append( "]" );
    }
    else
    {
        aBuf.append( mHostName );
    }

    // append port, but only, if not default port.
    bool bAppendPort = true;
    switch ( mPort )
    {
    case DEFAULT_HTTP_PORT:
        bAppendPort = (mScheme != "http");
        break;

    case DEFAULT_HTTPS_PORT:
        bAppendPort = (mScheme != "https");
        break;
    }
    if ( bAppendPort )
    {
        aBuf.append( ":" );
        aBuf.append( mPort );
    }
    aBuf.append( mPath );

    mURI = aBuf.makeStringAndClear();
}

OUString SerfUri::GetPathBaseName () const
{
    sal_Int32 nPos = mPath.lastIndexOf ('/');
    sal_Int32 nTrail = 0;
    if (nPos == mPath.getLength () - 1)
    {
        // Trailing slash found. Skip.
        nTrail = 1;
        nPos = mPath.lastIndexOf ('/', nPos);
    }
    if (nPos != -1)
    {
        OUString aTemp(
            mPath.copy (nPos + 1, mPath.getLength () - nPos - 1 - nTrail) );

        // query, fragment present?
        nPos = aTemp.indexOf( '?' );
        if ( nPos == -1 )
            nPos = aTemp.indexOf( '#' );

        if ( nPos != -1 )
            aTemp = aTemp.copy( 0, nPos );

        return aTemp;
    }
    else
        return "/";
}

bool SerfUri::operator== ( const SerfUri & rOther ) const
{
    return ( mURI == rOther.mURI );
}

OUString SerfUri::GetPathBaseNameUnescaped () const
{
    return unescape( GetPathBaseName() );
}

void SerfUri::AppendPath (const OUString& rPath)
{
    if (mPath.lastIndexOf ('/') != mPath.getLength () - 1)
        mPath += "/";

    mPath += rPath;
    calculateURI ();
};

// static
OUString SerfUri::escapeSegment( const OUString& segment )
{
    return rtl::Uri::encode( segment,
                             rtl_UriCharClassPchar,
                             rtl_UriEncodeIgnoreEscapes,
                             RTL_TEXTENCODING_UTF8 );
}

// static
OUString SerfUri::unescape( const OUString& segment )
{
    return rtl::Uri::decode( segment,
                             rtl_UriDecodeWithCharset,
                             RTL_TEXTENCODING_UTF8 );
}

// static
OUString SerfUri::makeConnectionEndPointString(
                                const OUString & rHostName, int nPort )
{
    OUStringBuffer aBuf;

    // Is host a numeric IPv6 address?
    if ( ( rHostName.indexOf( ':' ) != -1 ) &&
         ( rHostName[ 0 ] != '[' ) )
    {
        aBuf.append( "[" );
        aBuf.append( rHostName );
        aBuf.append( "]" );
    }
    else
    {
        aBuf.append( rHostName );
    }

    if ( ( nPort != DEFAULT_HTTP_PORT ) && ( nPort != DEFAULT_HTTPS_PORT ) )
    {
        aBuf.append( ":" );
        aBuf.append( sal_Int32( nPort ) );
    }
    return aBuf.makeStringAndClear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
