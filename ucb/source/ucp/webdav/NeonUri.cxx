/*************************************************************************
 *
 *  $RCSfile: NeonUri.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kso $ $Date: 2001-02-15 11:09:27 $
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

#include "NeonUri.hxx"
#include "DAVException.hxx"

using namespace rtl;
using namespace webdav_ucp;


uri NeonUri::sUriDefaults = { "http", NULL, DEFAULT_HTTP_PORT, NULL };

// -------------------------------------------------------------------
// Constructor
// -------------------------------------------------------------------
NeonUri::NeonUri( const OUString & inUri )
{
    if ( inUri.getLength() <= 0 )
        throw DAVException( DAVException::DAV_INVALID_ARG );

    OString theInputUri (
        inUri.getStr(), inUri.getLength(), RTL_TEXTENCODING_UTF8);

    uri theUri;
    if ( uri_parse( theInputUri.getStr(), &theUri, &sUriDefaults ) != 0 )
    {
        uri_free( &theUri );
        throw DAVException( DAVException::DAV_INVALID_ARG );
    }

    mScheme     = OStringToOUString( theUri.scheme, RTL_TEXTENCODING_UTF8 );
    mHostName   = OStringToOUString( theUri.host, RTL_TEXTENCODING_UTF8 );
    mPort       = theUri.port;
    mPath       = OStringToOUString( theUri.path, RTL_TEXTENCODING_UTF8 );

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
    mURI += OUString::createFromAscii ("://");
    mURI += mHostName;
    mURI += OUString::createFromAscii (":");
    mURI += OUString::valueOf (mPort);
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
        return OUString::createFromAscii ("/");
}

::rtl::OUString NeonUri::GetPathBaseNameUnescaped () const
{
    OString aName
        = OUStringToOString( GetPathBaseName(), RTL_TEXTENCODING_UTF8 );

    char* pU = uri_unescape( aName.getStr() );
    OUString aNameU = OUString::createFromAscii( pU );
    free( pU );
    return aNameU;
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
        return OUString::createFromAscii ("/");
}

void NeonUri::AppendPath (const OUString& path)
{
    if (mPath.lastIndexOf ('/') != mPath.getLength () - 1)
        mPath += OUString::createFromAscii ("/");

    mPath += path;
    calculateURI ();
};

// static
OUString NeonUri::escapeSegment( const OUString& segment )
{
    OString aSegment = OUStringToOString( segment, RTL_TEXTENCODING_UTF8 );
    char * pE = uri_abspath_escape( aSegment.getStr() );
    OUString aSegmentE = OUString::createFromAscii( pE );
    free( pE );
    return aSegmentE;
}

