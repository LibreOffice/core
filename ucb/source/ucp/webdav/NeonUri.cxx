/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NeonUri.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:37:31 $
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

using namespace webdav_ucp;

# if defined __SUNPRO_CC
// FIXME: not sure whether initializing a ne_uri statically is supposed to work
// the string fields of ne_uri are char*, not const char*
# pragma disable_warn
# endif
ne_uri NeonUri::sUriDefaultsHTTP  = { "http",  NULL, DEFAULT_HTTP_PORT,  NULL, NULL };
ne_uri NeonUri::sUriDefaultsHTTPS = { "https", NULL, DEFAULT_HTTPS_PORT, NULL, NULL };
ne_uri NeonUri::sUriDefaultsFTP   = { "ftp",   NULL, DEFAULT_FTP_PORT,   NULL, NULL };
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

    rtl::OString theInputUri(
        inUri.getStr(), inUri.getLength(), RTL_TEXTENCODING_UTF8 );

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
    ne_uri * pUriDefs
        = matchIgnoreAsciiCase( rUri,
                                RTL_CONSTASCII_STRINGPARAM( "ftp:" ) ) ?
              &sUriDefaultsFTP :
          matchIgnoreAsciiCase( rUri,
                                RTL_CONSTASCII_STRINGPARAM( "https:" ) ) ?
              &sUriDefaultsHTTPS :
              &sUriDefaultsHTTP;

    mScheme   = rtl::OStringToOUString(
                    pUri->scheme ? pUri->scheme : pUriDefs->scheme,
                    RTL_TEXTENCODING_UTF8 );
    mUserInfo = rtl::OStringToOUString(
                    pUri->authinfo ? pUri->authinfo : pUriDefs->authinfo,
                    RTL_TEXTENCODING_UTF8 );
    mHostName = rtl::OStringToOUString(
                    pUri->host ? pUri->host : pUriDefs->host,
                    RTL_TEXTENCODING_UTF8 );
    mPort     = pUri->port > 0 ? pUri->port : pUriDefs->port;
    mPath     = rtl::OStringToOUString(
                    pUri->path ? pUri->path : pUriDefs->path,
                    RTL_TEXTENCODING_UTF8 );
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
    aBuf.appendAscii( ":" );
    aBuf.append( rtl::OUString::valueOf( mPort ) );
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
        return mPath.copy (nPos + 1, mPath.getLength () - nPos - 1 - nTrail);
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

void NeonUri::AppendPath (const rtl::OUString& path)
{
    if (mPath.lastIndexOf ('/') != mPath.getLength () - 1)
        mPath += rtl::OUString::createFromAscii ("/");

    mPath += path;
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

