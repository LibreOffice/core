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

#ifndef _LINGUTIL_HXX_
#define _LINGUTIL_HXX_


#include <com/sun/star/lang/Locale.hpp>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <tools/string.hxx>

#include <vector>
#include <list>


#ifndef A2OU
#  define A2OU(x) ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( x ))
#endif

#define OU2A(rtlOUString) \
    ::rtl::OString((rtlOUString).getStr(), (rtlOUString).getLength(), \
    RTL_TEXTENCODING_ASCII_US).getStr()

#define OU2UTF8(rtlOUString) \
    ::rtl::OString((rtlOUString).getStr(), (rtlOUString).getLength(), \
    RTL_TEXTENCODING_UTF8).getStr()

#define OU2ISO_1(rtlOUString) \
    ::rtl::OString((rtlOUString).getStr(), (rtlOUString).getLength(), \
    RTL_TEXTENCODING_ISO_8859_1).getStr()

#define OU2ENC(rtlOUString, rtlEncoding) \
    ::rtl::OString((rtlOUString).getStr(), (rtlOUString).getLength(), \
    rtlEncoding, RTL_UNICODETOTEXT_FLAGS_UNDEFINED_QUESTIONMARK).getStr()


struct SvtLinguConfigDictionaryEntry;

///////////////////////////////////////////////////////////////////////////

struct lt_rtl_OUString
{
    bool operator() (const rtl::OUString &r1, const rtl::OUString &r2) const
    {
        return r1 < r2;
    }
};

inline sal_Bool operator == ( const ::com::sun::star::lang::Locale &rL1, const ::com::sun::star::lang::Locale &rL2 )
{
    return  rL1.Language ==  rL2.Language   &&
            rL1.Country  ==  rL2.Country    &&
            rL1.Variant  ==  rL2.Variant;
}

#if 0
///////////////////////////////////////////////////////////////////////////

String GetDirectoryPathFromFileURL( const String &rFileURL );
#endif

#if defined(WNT)
///////////////////////////////////////////////////////////////////////////
// to be use to get a short path name under Windows that still can be used with
// the 'fopen' call. This is necessary since under Windows there seems to be
// a restriction of only about 110-130 characters length to a path name in order
// for it to work with 'fopen'. And that length is usually easily exceeded
// when using extensions...
rtl::OString Win_GetShortPathName( const rtl::OUString &rLongPathName );
#endif

///////////////////////////////////////////////////////////////////////////

// temporary function, to be removed when new style dictionaries
// using configuration entries are fully implemented and provided
std::vector< SvtLinguConfigDictionaryEntry > GetOldStyleDics( const char * pDicType );
void MergeNewStyleDicsAndOldStyleDics( std::list< SvtLinguConfigDictionaryEntry > &rNewStyleDics, const std::vector< SvtLinguConfigDictionaryEntry > &rOldStyleDics );

///////////////////////////////////////////////////////////////////////////


//Find an encoding from a charset string, using
//rtl_getTextEncodingFromMimeCharset and falling back to
//rtl_getTextEncodingFromUnixCharset with the addition of
//ISCII-DEVANAGARI. On failure will return final fallback of
//RTL_TEXTENCODING_ISO_8859_1
rtl_TextEncoding getTextEncodingFromCharset(const sal_Char* pCharset);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
