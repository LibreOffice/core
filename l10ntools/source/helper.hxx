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

#include <algorithm>
#include <cassert>

#include <libxml/parser.h>

#include "rtl/string.hxx"
#include "rtl/ustring.hxx"
#include "rtl/strbuf.hxx"
#include "sal/types.h"

namespace helper {

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

inline sal_Int32 countOccurrences(rtl::OString const & text, char c) {
    sal_Int32 n = 0;
    for (sal_Int32 i = 0;; ++i) {
        i = text.indexOf(c, i);
        if (i == -1) {
            break;
        }
        ++n;
    }
    return n;
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

rtl::OString QuotHTML(const rtl::OString &rString)
{
    rtl::OStringBuffer sReturn;
    for (sal_Int32 i = 0; i < rString.getLength(); ++i) {
        switch (rString[i]) {
        case '\\':
            if (i < rString.getLength()) {
                switch (rString[i + 1]) {
                case '"':
                case '<':
                case '>':
                case '\\':
                    ++i;
                    break;
                }
            }
            // fall through
        default:
            sReturn.append(rString[i]);
            break;

        case '<':
            sReturn.append("&lt;");
            break;

        case '>':
            sReturn.append("&gt;");
            break;

        case '"':
            sReturn.append("&quot;");
            break;

        case '&':
            if (rString.matchL(RTL_CONSTASCII_STRINGPARAM("&amp;"), i))
                sReturn.append('&');
            else
                sReturn.append(RTL_CONSTASCII_STRINGPARAM("&amp;"));
            break;
        }
    }
    return sReturn.makeStringAndClear();
}

inline bool isWellFormedXML( OString const & text )
{
    xmlDocPtr doc;
    OString content;
    bool result = true;

    content = "<root>";
    content += text;
    content += "</root>";
    doc = xmlParseMemory(content.getStr(),(int)content.getLength());
    if (doc == NULL) {
        result = false;
    }
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return result;
}


template< typename T > inline T abbreviate(
    T const & text, sal_Int32 start, sal_Int32 length)
{
    start = std::max(sal_Int32(0), start);
    assert(start <= text.getLength());
    return text.copy(start, std::min(text.getLength() - start, length));
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
