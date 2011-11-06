/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
