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

        if ( !aParams.isEmpty() )
        {
            aResult.append( sal_Unicode( '?' ) );
            aResult.append( aParams );
        }

        if ( !aFragment.isEmpty() )
        {
            aResult.append( sal_Unicode( '#' ) );
            aResult.append( aFragment );
        }

        return aResult.makeStringAndClear();
    }

} } // namespace

#endif /* !INCLUDED_URIHELPER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
