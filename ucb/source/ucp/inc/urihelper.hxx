/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: urihelper.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-15 14:20:26 $
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

#ifndef INCLUDED_URIHELPER_HXX
#define INCLUDED_URIHELPER_HXX

#include "rtl/ustring.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/uri.hxx"

//=========================================================================

namespace ucb_impl { namespace urihelper {

    inline ::rtl::OUString encodeSegment( const ::rtl::OUString & rSegment )
    {
        return rtl::Uri::encode( rSegment,
                                 rtl_UriCharClassPchar,
                                 rtl_UriEncodeIgnoreEscapes,
                                 RTL_TEXTENCODING_UTF8 );
    }

    inline ::rtl::OUString decodeSegment( const rtl::OUString& rSegment )
    {
        return rtl::Uri::decode( rSegment,
                                 rtl_UriDecodeWithCharset,
                                 RTL_TEXTENCODING_UTF8 );
    }

    inline ::rtl::OUString encodeURI( const ::rtl::OUString & rURI )
    {
        rtl::OUString aFragment;
        rtl::OUString aParams;
        rtl::OUString aURI;

        sal_Int32 nFragment = rURI.lastIndexOf( sal_Unicode( '#' ) );
        if ( nFragment != -1 )
            aFragment = rURI.copy( nFragment + 1 );

        sal_Int32 nParams = ( nFragment == -1 )
            ? rURI.lastIndexOf( sal_Unicode( '?' ) )
            : rURI.lastIndexOf( sal_Unicode( '?' ), nFragment );
        if ( nParams != -1 )
            aParams = ( nFragment == -1 )
                ? rURI.copy( nParams + 1 )
                : rURI.copy( nParams + 1, nFragment - nParams - 1 );

        aURI = ( nParams != -1 )
            ? rURI.copy( 0, nParams )
            : ( nFragment != -1 )
                  ? rURI.copy( 0, nFragment )
                  : rURI;

        if ( aFragment.getLength() > 1 )
            aFragment =
                rtl::Uri::encode( aFragment,
                                  rtl_UriCharClassUric,
                                  rtl_UriEncodeKeepEscapes, /* #i81690# */
                                  RTL_TEXTENCODING_UTF8 );

        if ( aParams.getLength() > 1 )
            aParams =
                rtl::Uri::encode( aParams,
                                  rtl_UriCharClassUric,
                                  rtl_UriEncodeKeepEscapes, /* #i81690# */
                                  RTL_TEXTENCODING_UTF8 );

        rtl::OUStringBuffer aResult;
        sal_Int32 nIndex = 0;
        do
        {
            aResult.append(
                rtl::Uri::encode( aURI.getToken( 0, '/', nIndex ),
                                  rtl_UriCharClassPchar,
                                  rtl_UriEncodeKeepEscapes, /* #i81690# */
                                  RTL_TEXTENCODING_UTF8 ) );
            if ( nIndex >= 0 )
                aResult.append( sal_Unicode( '/' ) );
        }
        while ( nIndex >= 0 );

        if ( aParams.getLength() > 0 )
        {
            aResult.append( sal_Unicode( '?' ) );
            aResult.append( aParams );
        }

        if ( aFragment.getLength() > 0 )
        {
            aResult.append( sal_Unicode( '#' ) );
            aResult.append( aFragment );
        }

        return aResult.makeStringAndClear();
    }

} } // namespace

#endif /* !INCLUDED_URIHELPER_HXX */
