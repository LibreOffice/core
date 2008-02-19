/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NeonUri.cxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-19 12:35:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

#include <string.h>

#ifndef _RTL_URI_HXX_
#include <rtl/uri.hxx>
#endif
#ifndef _RTL_OUSTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _NEONURI_HXX_
#include "NeonUri.hxx"
#endif
#ifndef _DAVEXCEPTION_HXX_
#include "DAVException.hxx"
#endif

#include "../inc/urihelper.hxx"

using namespace webdav_ucp;

    char *scheme;
    char *host, *userinfo;
    unsigned int port;
    char *path, *query, *fragment;

# if defined __SUNPRO_CC
// FIXME: not sure whether initializing a ne_uri statically is supposed to work
// the string fields of ne_uri are char*, not const char*
# pragma disable_warn
# endif

namespace {

const ne_uri g_sUriDefaultsHTTP  = { "http",
#if NEON_VERSION >= 0x0260
                                     NULL,
#endif
                                     NULL,
                                     DEFAULT_HTTP_PORT,
#if NEON_VERSION >= 0x0260
                                     NULL,
#endif
                                     NULL,
                                     NULL };
const ne_uri g_sUriDefaultsHTTPS = { "https",
#if NEON_VERSION >= 0x0260
                                     NULL,
#endif
                                     NULL,
                                     DEFAULT_HTTPS_PORT,
#if NEON_VERSION >= 0x0260
                                     NULL,
#endif
                                     NULL,
                                     NULL };
const ne_uri g_sUriDefaultsFTP   = { "ftp",
#if NEON_VERSION >= 0x0260
                                     NULL,
#endif
                                     NULL,
                                     DEFAULT_FTP_PORT,
#if NEON_VERSION >= 0x0260
                                     NULL,
#endif
                                     NULL,
                                     NULL };
} // namespace

# if defined __SUNPRO_CC
# pragma enable_warn
#endif

// -------------------------------------------------------------------
// Constructor
// -------------------------------------------------------------------

namespace {

//TODO! rtl::OString::matchIgnoreAsciiCaseAsciiL() missing
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

NeonUri::NeonUri( const ne_uri * inUri )
    throw ( DAVException )
{
    if ( inUri == 0 )
        throw DAVException( DAVException::DAV_INVALID_ARG );

    char * uri = ne_uri_unparse( inUri );

    if ( uri == 0 )
        throw DAVException( DAVException::DAV_INVALID_ARG );

    init( rtl::OString( uri ), inUri );
    free( uri );

    calculateURI();
}

NeonUri::NeonUri( const rtl::OUString & inUri )
    throw ( DAVException )
{
    if ( inUri.getLength() <= 0 )
        throw DAVException( DAVException::DAV_INVALID_ARG );

    // #i77023#
    rtl::OUString aEscapedUri( ucb_impl::urihelper::encodeURI( inUri ) );

    rtl::OString theInputUri(
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

void NeonUri::init( const rtl::OString & rUri, const ne_uri * pUri )
{
    // Complete URI.
    const ne_uri * pUriDefs
        = matchIgnoreAsciiCase( rUri,
                                RTL_CONSTASCII_STRINGPARAM( "ftp:" ) ) ?
              &g_sUriDefaultsFTP :
          matchIgnoreAsciiCase( rUri,
                                RTL_CONSTASCII_STRINGPARAM( "https:" ) ) ?
              &g_sUriDefaultsHTTPS :
              &g_sUriDefaultsHTTP;

    mScheme   = rtl::OStringToOUString(
                    pUri->scheme ? pUri->scheme : pUriDefs->scheme,
                    RTL_TEXTENCODING_UTF8 );
    mUserInfo = rtl::OStringToOUString(
#if NEON_VERSION >= 0x0260
                    pUri->userinfo ? pUri->userinfo : pUriDefs->userinfo,
#else
                    pUri->authinfo ? pUri->authinfo : pUriDefs->authinfo,
#endif
                    RTL_TEXTENCODING_UTF8 );
    mHostName = rtl::OStringToOUString(
                    pUri->host ? pUri->host : pUriDefs->host,
                    RTL_TEXTENCODING_UTF8 );
    mPort     = pUri->port > 0 ? pUri->port : pUriDefs->port;
    mPath     = rtl::OStringToOUString(
                    pUri->path ? pUri->path : pUriDefs->path,
                    RTL_TEXTENCODING_UTF8 );

#if NEON_VERSION >= 0x0260
    if ( pUri->query )
    {
        mPath += rtl::OUString::createFromAscii( "?" );
        mPath += rtl::OStringToOUString(
            pUri->query,  RTL_TEXTENCODING_UTF8 );
    }

    if ( pUri->fragment )
    {
        mPath += rtl::OUString::createFromAscii( "#" );
        mPath += rtl::OStringToOUString(
            pUri->fragment,  RTL_TEXTENCODING_UTF8 );
    }
#endif
}

// -------------------------------------------------------------------
// Destructor
// -------------------------------------------------------------------
NeonUri::~NeonUri( )
{
}

void NeonUri::calculateURI ()
{
    rtl::OUStringBuffer aBuf( mScheme );
    aBuf.appendAscii( "://" );
    if ( mUserInfo.getLength() > 0 )
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
        bAppendPort
            = !mScheme.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "http" ) );
        break;

    case DEFAULT_HTTPS_PORT:
        bAppendPort
            = !mScheme.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "https" ) );
        break;

    case DEFAULT_FTP_PORT:
        bAppendPort
            = !mScheme.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ftp" ) );
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

::rtl::OUString NeonUri::GetPathBaseName () const
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

bool NeonUri::operator== ( const NeonUri & rOther ) const
{
    return ( mURI == rOther.mURI );
}

::rtl::OUString NeonUri::GetPathBaseNameUnescaped () const
{
    return unescape( GetPathBaseName() );
}

::rtl::OUString NeonUri::GetPathDirName () const
{
    sal_Int32 nPos = mPath.lastIndexOf ('/');
    if (nPos == mPath.getLength () - 1)
    {
        // Trailing slash found. Skip.
        nPos = mPath.lastIndexOf ('/', nPos);
    }
    if (nPos != -1)
        return mPath.copy (0, nPos + 1);
    else
        return rtl::OUString::createFromAscii ("/");
}

void NeonUri::AppendPath (const rtl::OUString& rPath)
{
    if (mPath.lastIndexOf ('/') != mPath.getLength () - 1)
        mPath += rtl::OUString::createFromAscii ("/");

    mPath += rPath;
    calculateURI ();
};

// static
rtl::OUString NeonUri::escapeSegment( const rtl::OUString& segment )
{
    return rtl::Uri::encode( segment,
                             rtl_UriCharClassPchar,
                             rtl_UriEncodeIgnoreEscapes,
                             RTL_TEXTENCODING_UTF8 );
}

// static
rtl::OUString NeonUri::unescape( const rtl::OUString& segment )
{
    return rtl::Uri::decode( segment,
                             rtl_UriDecodeWithCharset,
                             RTL_TEXTENCODING_UTF8 );
}

// static
rtl::OUString NeonUri::makeConnectionEndPointString(
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

