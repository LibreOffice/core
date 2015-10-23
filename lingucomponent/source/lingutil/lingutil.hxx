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

#ifndef INCLUDED_LINGUCOMPONENT_SOURCE_LINGUTIL_LINGUTIL_HXX
#define INCLUDED_LINGUCOMPONENT_SOURCE_LINGUTIL_LINGUTIL_HXX

#include <com/sun/star/lang/Locale.hpp>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>

#include <vector>
#include <list>

#define OU2ENC(rtlOUString, rtlEncoding) \
    OString((rtlOUString).getStr(), (rtlOUString).getLength(), \
    rtlEncoding, RTL_UNICODETOTEXT_FLAGS_UNDEFINED_QUESTIONMARK).getStr()

struct SvtLinguConfigDictionaryEntry;

struct lt_rtl_OUString
{
    bool operator() (const OUString &r1, const OUString &r2) const
    {
        return r1 < r2;
    }
};

inline bool operator == ( const css::lang::Locale &rL1, const css::lang::Locale &rL2 )
{
    return  rL1.Language ==  rL2.Language   &&
            rL1.Country  ==  rL2.Country    &&
            rL1.Variant  ==  rL2.Variant;
}

#if defined(WNT)

// to be use to get a path name with long path prefix
// under Windows for Hunspell, Hyphen and MyThes libraries
OString Win_AddLongPathPrefix( const OString &rPathName );
#endif


// temporary function, to be removed when new style dictionaries
// using configuration entries are fully implemented and provided
std::vector< SvtLinguConfigDictionaryEntry > GetOldStyleDics( const char * pDicType );
void MergeNewStyleDicsAndOldStyleDics( std::list< SvtLinguConfigDictionaryEntry > &rNewStyleDics, const std::vector< SvtLinguConfigDictionaryEntry > &rOldStyleDics );

//Find an encoding from a charset string, using
//rtl_getTextEncodingFromMimeCharset and falling back to
//rtl_getTextEncodingFromUnixCharset with the addition of
//ISCII-DEVANAGARI. On failure will return final fallback of
//RTL_TEXTENCODING_ISO_8859_1
rtl_TextEncoding getTextEncodingFromCharset(const sal_Char* pCharset);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
