/*************************************************************************
 *
 *  $RCSfile: NeonUri.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: sb $ $Date: 2001-08-08 10:04:35 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _RTL_URI_HXX_
#include <rtl/uri.hxx>
#endif
#ifndef _RTL_OUSTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _NEONURI_HXX_
#include "NeonUri.hxx"
#endif
#ifndef _DAVEXCEPTION_HXX_
#include "DAVException.hxx"
#endif

using namespace webdav_ucp;

uri NeonUri::sUriDefaultsHTTP  = { "http",  NULL, DEFAULT_HTTP_PORT,  NULL };
uri NeonUri::sUriDefaultsHTTPS = { "https", NULL, DEFAULT_HTTPS_PORT, NULL };
uri NeonUri::sUriDefaultsFTP   = { "ftp",   NULL, DEFAULT_FTP_PORT,   NULL };

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

NeonUri::NeonUri( const rtl::OUString & inUri )
{
    if ( inUri.getLength() <= 0 )
        throw DAVException( DAVException::DAV_INVALID_ARG );

    rtl::OString theInputUri(
        inUri.getStr(), inUri.getLength(), RTL_TEXTENCODING_UTF8 );

    rtl::OString aProtocol
        = theInputUri.copy( 0, RTL_CONSTASCII_LENGTH( "https:" ) );

    uri theUri;
    uri* pUriDefs
        = matchIgnoreAsciiCase(theInputUri,
                               RTL_CONSTASCII_STRINGPARAM("http:")) ?
              &sUriDefaultsHTTP :
          matchIgnoreAsciiCase(theInputUri,
                               RTL_CONSTASCII_STRINGPARAM("https:")) ?
              &sUriDefaultsHTTPS :
              &sUriDefaultsFTP;

    if ( uri_parse( theInputUri.getStr(), &theUri, pUriDefs ) != 0 )
    {
        uri_free( &theUri );
        throw DAVException( DAVException::DAV_INVALID_ARG );
    }

    mScheme   = rtl::OStringToOUString( theUri.scheme, RTL_TEXTENCODING_UTF8 );
    mUserInfo = rtl::OStringToOUString( theUri.authinfo, RTL_TEXTENCODING_UTF8 );
    mHostName = rtl::OStringToOUString( theUri.host, RTL_TEXTENCODING_UTF8 );
    mPort     = theUri.port;
    mPath     = rtl::OStringToOUString( theUri.path, RTL_TEXTENCODING_UTF8 );

    uri_free( &theUri );

    calculateURI ();
}

// -------------------------------------------------------------------
// Destructor
// -------------------------------------------------------------------
NeonUri::~NeonUri( )
{
}

void NeonUri::calculateURI ()
{
    mURI = mScheme;
    mURI += rtl::OUString::createFromAscii ("://");
    if (mUserInfo.getLength() > 0)
    {
        //TODO! differentiate between empty and missing userinfo
        mURI += mUserInfo;
        mURI += rtl::OUString::createFromAscii ("@");
    }
    mURI += mHostName;
    mURI += rtl::OUString::createFromAscii (":");
    mURI += rtl::OUString::valueOf (mPort);
    mURI += mPath;
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
//    rtl::OUString aServer(
//            rtl::OStringToOUString( rHostName, RTL_TEXTENCODING_UTF8 ) );
    rtl::OUString aServer( rHostName );
    if ( ( nPort != DEFAULT_HTTP_PORT ) && ( nPort != DEFAULT_HTTPS_PORT ) )
    {
        aServer += rtl::OUString::createFromAscii( ":" );
        aServer += rtl::OUString::valueOf( sal_Int32( nPort ) );
    }
    return aServer;
}

