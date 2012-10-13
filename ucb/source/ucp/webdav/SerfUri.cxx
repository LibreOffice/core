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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

#include <string.h>
#include <rtl/uri.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include "SerfUri.hxx"
#include "DAVException.hxx"
#include <AprEnv.hxx>

#include "../inc/urihelper.hxx"

using namespace http_dav_ucp;

# if defined __SUNPRO_CC
# pragma enable_warn
#endif

// -------------------------------------------------------------------
// Constructor
// -------------------------------------------------------------------

namespace {

inline bool matchIgnoreAsciiCase(rtl::OString const & rStr1,
                                 sal_Char const * pStr2,
                                 sal_Int32 nStr2Len) SAL_THROW(())
{
    return
        rtl_str_shortenedCompareIgnoreAsciiCase_WithLength(
                rStr1.getStr(), rStr1.getLength(), pStr2, nStr2Len, nStr2Len)
            == 0;
}

}

SerfUri::SerfUri( const apr_uri_t * inUri )
    throw ( DAVException )
    : mAprUri( *inUri )
    , mURI()
    , mScheme()
    , mUserInfo()
    , mHostName()
    , mPort()
    , mPath()
{
    if ( inUri == 0 )
        throw DAVException( DAVException::DAV_INVALID_ARG );

    char * uri = apr_uri_unparse( apr_environment::AprEnv::getAprEnv()->getAprPool(), &mAprUri, 0 );

    if ( uri == 0 )
        throw DAVException( DAVException::DAV_INVALID_ARG );

    init( &mAprUri );

    calculateURI();
}

SerfUri::SerfUri( const rtl::OUString & inUri )
    throw ( DAVException )
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
    rtl::OUString aEscapedUri( ucb_impl::urihelper::encodeURI( inUri ) );

    rtl::OString theInputUri(
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
        mAprUri.path = "/";
    }

    init( &mAprUri );

    calculateURI();
}

void SerfUri::init( const apr_uri_t * pUri )
{
    mScheme   = rtl::OStringToOUString( pUri->scheme, RTL_TEXTENCODING_UTF8 );
    mUserInfo = rtl::OStringToOUString( pUri->user, RTL_TEXTENCODING_UTF8 );
    mHostName = rtl::OStringToOUString( pUri->hostname, RTL_TEXTENCODING_UTF8 );
    mPort     = pUri->port;
    mPath     = rtl::OStringToOUString( pUri->path, RTL_TEXTENCODING_UTF8 );

    if ( pUri->query )
    {
        mPath += rtl::OUString::createFromAscii( "?" );
        mPath += rtl::OStringToOUString( pUri->query,  RTL_TEXTENCODING_UTF8 );
    }

    if ( pUri->fragment )
    {
        mPath += rtl::OUString::createFromAscii( "#" );
        mPath += rtl::OStringToOUString( pUri->fragment,  RTL_TEXTENCODING_UTF8 );
    }
}

SerfUri::~SerfUri( )
{
}

void SerfUri::calculateURI ()
{
    rtl::OUStringBuffer aBuf( mScheme );
    aBuf.appendAscii( "://" );
    if ( mUserInfo.getLength() > 0 )
    {
        aBuf.append( mUserInfo );
        aBuf.appendAscii( "@" );
    }
    // Is host a numeric IPv6 address?
    if ( ( mHostName.indexOf( ':' ) != -1 ) &&
         ( mHostName[ 0 ] != sal_Unicode( '[' ) ) )
    {
        aBuf.appendAscii( "[" );
        aBuf.append( mHostName );
        aBuf.appendAscii( "]" );
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
        bAppendPort = !mScheme.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "http" ) );
        break;

    case DEFAULT_HTTPS_PORT:
        bAppendPort = !mScheme.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "https" ) );
        break;
    }
    if ( bAppendPort )
    {
        aBuf.appendAscii( ":" );
        aBuf.append( rtl::OUString::valueOf( mPort ) );
    }
    aBuf.append( mPath );

    mURI = aBuf.makeStringAndClear();
}

::rtl::OUString SerfUri::GetPathBaseName () const
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
        rtl::OUString aTemp(
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
        return rtl::OUString::createFromAscii ("/");
}

bool SerfUri::operator== ( const SerfUri & rOther ) const
{
    return ( mURI == rOther.mURI );
}

::rtl::OUString SerfUri::GetPathBaseNameUnescaped () const
{
    return unescape( GetPathBaseName() );
}

void SerfUri::AppendPath (const rtl::OUString& rPath)
{
    if (mPath.lastIndexOf ('/') != mPath.getLength () - 1)
        mPath += rtl::OUString::createFromAscii ("/");

    mPath += rPath;
    calculateURI ();
};

// static
rtl::OUString SerfUri::escapeSegment( const rtl::OUString& segment )
{
    return rtl::Uri::encode( segment,
                             rtl_UriCharClassPchar,
                             rtl_UriEncodeIgnoreEscapes,
                             RTL_TEXTENCODING_UTF8 );
}

// static
rtl::OUString SerfUri::unescape( const rtl::OUString& segment )
{
    return rtl::Uri::decode( segment,
                             rtl_UriDecodeWithCharset,
                             RTL_TEXTENCODING_UTF8 );
}

// static
rtl::OUString SerfUri::makeConnectionEndPointString(
                                const rtl::OUString & rHostName, int nPort )
{
    rtl::OUStringBuffer aBuf;

    // Is host a numeric IPv6 address?
    if ( ( rHostName.indexOf( ':' ) != -1 ) &&
         ( rHostName[ 0 ] != sal_Unicode( '[' ) ) )
    {
        aBuf.appendAscii( "[" );
        aBuf.append( rHostName );
        aBuf.appendAscii( "]" );
    }
    else
    {
        aBuf.append( rHostName );
    }

    if ( ( nPort != DEFAULT_HTTP_PORT ) && ( nPort != DEFAULT_HTTPS_PORT ) )
    {
        aBuf.appendAscii( ":" );
        aBuf.append( rtl::OUString::valueOf( sal_Int32( nPort ) ) );
    }
    return aBuf.makeStringAndClear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
