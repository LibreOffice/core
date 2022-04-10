/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <libxml/parser.h>

#include <o3tl/safeint.hxx>
#include <o3tl/string_view.hxx>
#include <rtl/strbuf.hxx>

#include <helper.hxx>

namespace helper {

OString escapeAll(
    std::string_view rText, std::string_view rUnEscaped, std::string_view rEscaped )
{
    assert( rEscaped.size() == 2*rUnEscaped.size() );
    OStringBuffer sReturn;
    for ( size_t nIndex = 0; nIndex < rText.size(); ++nIndex )
    {
        size_t nUnEscapedOne = rUnEscaped.find(rText[nIndex]);
        if( nUnEscapedOne != std::string_view::npos )
        {
            sReturn.append(rEscaped.substr(nUnEscapedOne*2,2));
        }
        else
            sReturn.append(rText[nIndex]);
    }
    return sReturn.makeStringAndClear();
}


OString unEscapeAll(
    std::string_view rText, std::string_view rEscaped, std::string_view rUnEscaped)
{
    assert( rEscaped.size() == 2*rUnEscaped.length() );
    OStringBuffer sReturn;
    const size_t nLength = rText.size();
    for ( size_t nIndex = 0; nIndex < nLength; ++nIndex )
    {
        if( rText[nIndex] == '\\' && nIndex+1 < nLength )
        {
            size_t nEscapedOne = rEscaped.find(rText.substr(nIndex,2));
            if( nEscapedOne != std::string_view::npos )
            {
                sReturn.append(rUnEscaped[nEscapedOne/2]);
                ++nIndex;
            }
            else
            {
                sReturn.append(rText[nIndex]);
            }
        }
        else
            sReturn.append(rText[nIndex]);
    }
    return sReturn.makeStringAndClear();
}


OString QuotHTML(std::string_view rString)
{
    OStringBuffer sReturn;
    for (size_t i = 0; i < rString.size(); ++i)
    {
        switch (rString[i])
        {
        case '<':
            sReturn.append("&lt;");
            break;
        case '>':
            sReturn.append("&gt;");
            break;
        case '"':
            sReturn.append("&quot;");
            break;
        case '\'':
            sReturn.append("&apos;");
            break;
        case '&':
            if (o3tl::starts_with(rString.substr(i), "&amp;"))
                sReturn.append('&');
            else
                sReturn.append("&amp;");
            break;
        default:
            sReturn.append(rString[i]);
            break;
        }
    }
    return sReturn.makeStringAndClear();
}

OString UnQuotHTML( std::string_view rString )
{
    OStringBuffer sReturn;
    for (size_t i = 0; i != rString.size();) {
        auto tmp = rString.substr(i);
        if (o3tl::starts_with(tmp, "&amp;")) {
            sReturn.append('&');
            i += RTL_CONSTASCII_LENGTH("&amp;");
        } else if (o3tl::starts_with(tmp, "&lt;")) {
            sReturn.append('<');
            i += RTL_CONSTASCII_LENGTH("&lt;");
        } else if (o3tl::starts_with(tmp, "&gt;")) {
            sReturn.append('>');
            i += RTL_CONSTASCII_LENGTH("&gt;");
        } else if (o3tl::starts_with(tmp, "&quot;")) {
            sReturn.append('"');
            i += RTL_CONSTASCII_LENGTH("&quot;");
        } else if (o3tl::starts_with(tmp, "&apos;")) {
            sReturn.append('\'');
            i += RTL_CONSTASCII_LENGTH("&apos;");
        } else {
            sReturn.append(rString[i]);
            ++i;
        }
    }
    return sReturn.makeStringAndClear();
}

bool isWellFormedXML( std::string_view text )
{
    xmlDocPtr doc;
    bool result = true;

    OString content = OString::Concat("<root>") + text + "</root>";
    doc = xmlParseMemory(content.getStr(),static_cast<int>(content.getLength()));
    if (doc == nullptr) {
        result = false;
    }
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return result;
}

//Convert xmlChar* to OString
OString xmlStrToOString( const xmlChar* pString )
{
    xmlChar* pTemp = xmlStrdup( pString );
    OString sResult = reinterpret_cast<char*>( pTemp );
    xmlFree( pTemp );
    return sResult;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
