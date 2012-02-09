/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * [ Copyright (C) 2012 Red Hat, Inc., Stephan Bergmann <sbergman@redhat.com>
 *   (initial developer) ]
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef INCLUDED_L10NTOOLS_SOURCE_HELPER_HXX
#define INCLUDED_L10NTOOLS_SOURCE_HELPER_HXX

#include "sal/config.h"

#include <cassert>

#include "rtl/string.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"

namespace helper {

inline bool isAsciiWhitespace(char c) {
    return (c >= 0x09 && c <= 0x0D) || c == ' '; // HT, LF, VT, FF, CR
}

// cf. comphelper::string::isdigitAsciiString:
inline bool isAllAsciiDigits(rtl::OString const & text) {
    for (sal_Int32 i = 0; i != text.getLength(); ++i) {
        if (text[i] < '0' || text[i] > '9') {
            return false;
        }
    }
    return true;
}

// cf. comphelper::string::isupperAsciiString:
inline bool isAllAsciiUpperCase(rtl::OString const & text) {
    for (sal_Int32 i = 0; i != text.getLength(); ++i) {
        if (text[i] < 'A' || text[i] > 'Z') {
            return false;
        }
    }
    return true;
}

// cf. comphelper::string::islowerAsciiString:
inline bool isAllAsciiLowerCase(rtl::OString const & text) {
    for (sal_Int32 i = 0; i != text.getLength(); ++i) {
        if (text[i] < 'a' || text[i] > 'z') {
            return false;
        }
    }
    return true;
}

inline bool endsWith(rtl::OString const & text, rtl::OString const & search) {
    return text.getLength() >= search.getLength()
        && text.match(search, text.getLength() - search.getLength());
}

inline rtl::OString trimAscii(rtl::OString const & text) {
    sal_Int32 i1 = 0;
    while (i1 != text.getLength() && isAsciiWhitespace(text[i1])) {
        ++i1;
    }
    sal_Int32 i2 = text.getLength();
    while (i2 != i1 && isAsciiWhitespace(text[i2 - 1])) {
        --i2;
    }
    return text.copy(i1, i2 - i1);
}

inline sal_Int32 searchAndReplace(
    rtl::OString * text, rtl::OString const & search,
    rtl::OString const & replace)
{
    assert(text != 0);
    sal_Int32 i = text->indexOf(search);
    if (i != -1) {
        *text = text->replaceAt(i, search.getLength(), replace);
    }
    return i;
}

inline void searchAndReplaceAll(
    rtl::OString * text, rtl::OString const & search,
    rtl::OString const & replace)
{
    assert(text != 0);
    for (sal_Int32 i = 0;;) {
        i = text->indexOf(search, i);
        if (i == -1) {
            break;
        }
        *text = text->replaceAt(i, search.getLength(), replace);
        i += replace.getLength();
    }
}

inline void searchAndReplaceAll(
    rtl::OUString * text, rtl::OUString const & search,
    rtl::OUString const & replace)
{
    assert(text != 0);
    for (sal_Int32 i = 0;;) {
        i = text->indexOf(search, i);
        if (i == -1) {
            break;
        }
        *text = text->replaceAt(i, search.getLength(), replace);
        i += replace.getLength();
    }
}

inline rtl::OString getToken(
    rtl::OString const & text, sal_Int32 token, char separator)
{
    sal_Int32 i = 0;
    return text.getToken(token, separator, i);
}

inline sal_Int32 indexOfAnyAsciiL(
    rtl::OUString const & text, char const * chars, sal_Int32 charsLen,
    sal_Int32 index = 0)
{
    for (; index != text.getLength(); ++index) {
        sal_Unicode c = text[index];
        if (c <= 0x7F
            && (rtl_str_indexOfChar_WithLength(
                    chars, charsLen, static_cast< char >(c))
                != -1))
        {
            return index;
        }
    }
    return -1;
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
