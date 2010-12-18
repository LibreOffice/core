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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
