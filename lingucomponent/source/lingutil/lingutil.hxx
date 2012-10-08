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
