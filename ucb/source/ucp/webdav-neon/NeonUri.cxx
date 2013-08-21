/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "sal/config.h"

#include <string.h>
#include <rtl/uri.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include "ne_alloc.h"
#include "NeonUri.hxx"
#include "DAVException.hxx"

#include "../inc/urihelper.hxx"

using namespace webdav_ucp;

# if defined __SUNPRO_CC
// FIXME: not sure whether initializing a ne_uri statically is supposed to work
// the string fields of ne_uri are char*, not const char*
# pragma disable_warn
# endif

#if HAVE_GCC_PRAGMA_DIAGNOSTIC_MODIFY
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

namespace {

const ne_uri g_sUriDefaultsHTTP  = { (char *) "http",
                                     NULL,
                                     NULL,
                                     DEFAULT_HTTP_PORT,
                                     NULL,
                                     NULL,
                                     NULL };
const ne_uri g_sUriDefaultsHTTPS = { (char *) "https",
                                     NULL,
                                     NULL,
                                     DEFAULT_HTTPS_PORT,
                                     NULL,
                                     NULL,
                                     NULL };
const ne_uri g_sUriDefaultsFTP   = { (char *) "ftp",
                                     NULL,
                                     NULL,
                                     DEFAULT_FTP_PORT,
                                     NULL,
                                     NULL,
                                     NULL };
} // namespace

# if defined __SUNPRO_CC
# pragma enable_warn
#endif

NeonUri::NeonUri( const ne_uri * inUri )
    throw ( DAVException )
{
    if ( inUri == 0 )
        throw DAVException( DAVException::DAV_INVALID_ARG );

    char * uri = ne_uri_unparse( inUri );

    if ( uri == 0 )
        throw DAVException( DAVException::DAV_INVALID_ARG );

    init( OString( uri ), inUri );
    ne_free( uri );

    calculateURI();
}

NeonUri::NeonUri( const OUString & inUri )
    throw ( DAVException )
{
    if ( inUri.isEmpty() )
        throw DAVException( DAVException::DAV_INVALID_ARG );

    // #i77023#
    OUString aEscapedUri( ucb_impl::urihelper::encodeURI( inUri ) );

    OString theInputUri(
        aEscapedUri.getStr(), aEscapedUri.getLength(), RTL_TEXTENCODING_UTF8 );

    ne_uri theUri;
    if ( ne_uri_parse( theInputUri.getStr(), &theUri ) != 0 )
    {
        ne_uri_free( &theUri );
        throw DAVException( DAVException::DAV_INVALID_ARG );
    }

    init( theInputUri, &theUri );
    ne_uri_free( &theUri );

    calculateURI();
}

void NeonUri::init( const OString & rUri, const ne_uri * pUri )
{
    // Complete URI.
    const ne_uri * pUriDefs
        = rUri.matchIgnoreAsciiCase( "ftp:" ) ?
              &g_sUriDefaultsFTP :
          rUri.matchIgnoreAsciiCase( "https:" ) ?
              &g_sUriDefaultsHTTPS :
              &g_sUriDefaultsHTTP;

    mScheme   = OStringToOUString(
                    pUri->scheme ? pUri->scheme : pUriDefs->scheme,
                    RTL_TEXTENCODING_UTF8 );
    mUserInfo = OStringToOUString(
                    pUri->userinfo ? pUri->userinfo : pUriDefs->userinfo,
                    RTL_TEXTENCODING_UTF8 );
    mHostName = OStringToOUString(
                    pUri->host ? pUri->host : pUriDefs->host,
                    RTL_TEXTENCODING_UTF8 );
    mPort     = pUri->port > 0 ? pUri->port : pUriDefs->port;
    mPath     = OStringToOUString(
                    pUri->path ? pUri->path : pUriDefs->path,
                    RTL_TEXTENCODING_UTF8 );

    if ( pUri->query )
    {
        mPath += OUString("?");
        mPath += OStringToOUString(
            pUri->query,  RTL_TEXTENCODING_UTF8 );
    }

    if ( pUri->fragment )
    {
        mPath += OUString("#");
        mPath += OStringToOUString(
            pUri->fragment,  RTL_TEXTENCODING_UTF8 );
    }
}

NeonUri::~NeonUri( )
{
}

void NeonUri::calculateURI ()
{
    OUStringBuffer aBuf( mScheme );
    aBuf.appendAscii( "://" );
    if ( !mUserInfo.isEmpty() )
    {
        //TODO! differentiate between empty and missing userinfo
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
        bAppendPort = mScheme != "http";
        break;

    case DEFAULT_HTTPS_PORT:
        bAppendPort = mScheme != "https";
        break;

    case DEFAULT_FTP_PORT:
        bAppendPort = mScheme != "ftp";
        break;
    }
    if ( bAppendPort )
    {
        aBuf.appendAscii( ":" );
        aBuf.append( OUString::number( mPort ) );
    }
    aBuf.append( mPath );

    mURI = aBuf.makeStringAndClear();
}

OUString NeonUri::GetPathBaseName () const
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
        return OUString("/");
}

bool NeonUri::operator== ( const NeonUri & rOther ) const
{
    return ( mURI == rOther.mURI );
}

OUString NeonUri::GetPathBaseNameUnescaped () const
{
    return unescape( GetPathBaseName() );
}

void NeonUri::AppendPath (const OUString& rPath)
{
    if (mPath.lastIndexOf ('/') != mPath.getLength () - 1)
        mPath += OUString("/");

    mPath += rPath;
    calculateURI ();
};

// static
OUString NeonUri::escapeSegment( const OUString& segment )
{
    return rtl::Uri::encode( segment,
                             rtl_UriCharClassPchar,
                             rtl_UriEncodeIgnoreEscapes,
                             RTL_TEXTENCODING_UTF8 );
}

// static
OUString NeonUri::unescape( const OUString& segment )
{
    return rtl::Uri::decode( segment,
                             rtl_UriDecodeWithCharset,
                             RTL_TEXTENCODING_UTF8 );
}

// static
OUString NeonUri::makeConnectionEndPointString(
                                const OUString & rHostName, int nPort )
{
    OUStringBuffer aBuf;

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
        aBuf.append( OUString::number( nPort ) );
    }
    return aBuf.makeStringAndClear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
