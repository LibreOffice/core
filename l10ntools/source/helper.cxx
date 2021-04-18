/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <o3tl/safeint.hxx>

#include <helper.hxx>

namespace helper {

OString escapeAll(
    const OString& rText, const OString& rUnEscaped, const OString& rEscaped )
{
    assert( rEscaped.getLength() == 2*rUnEscaped.getLength() );
    OStringBuffer sReturn;
    for ( sal_Int32 nIndex = 0; nIndex < rText.getLength(); ++nIndex )
    {
        sal_Int32 nUnEscapedOne = rUnEscaped.indexOf(rText[nIndex]);
        if( nUnEscapedOne != -1 )
        {
            sReturn.append(rEscaped.subView(nUnEscapedOne*2,2));
        }
        else
            sReturn.append(rText[nIndex]);
    }
    return sReturn.makeStringAndClear();
}


OString unEscapeAll(
    const OString& rText, const OString& rEscaped, std::string_view rUnEscaped)
{
    assert( o3tl::make_unsigned(rEscaped.getLength()) == 2*rUnEscaped.length() );
    OStringBuffer sReturn;
    const sal_Int32 nLength = rText.getLength();
    for ( sal_Int32 nIndex = 0; nIndex < nLength; ++nIndex )
    {
        if( rText[nIndex] == '\\' && nIndex+1 < nLength )
        {
            sal_Int32 nEscapedOne = rEscaped.indexOf(rText.subView(nIndex,2));
            if( nEscapedOne != -1 )
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


OString QuotHTML(const OString &rString)
{
    OStringBuffer sReturn;
    for (sal_Int32 i = 0; i < rString.getLength(); ++i)
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
            if (rString.match("&amp;", i))
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

OString UnQuotHTML( const OString& rString )
{
    OStringBuffer sReturn;
    for (sal_Int32 i = 0; i != rString.getLength();) {
        if (rString.match("&amp;", i)) {
            sReturn.append('&');
            i += RTL_CONSTASCII_LENGTH("&amp;");
        } else if (rString.match("&lt;", i)) {
            sReturn.append('<');
            i += RTL_CONSTASCII_LENGTH("&lt;");
        } else if (rString.match("&gt;", i)) {
            sReturn.append('>');
            i += RTL_CONSTASCII_LENGTH("&gt;");
        } else if (rString.match("&quot;", i)) {
            sReturn.append('"');
            i += RTL_CONSTASCII_LENGTH("&quot;");
        } else if (rString.match("&apos;", i)) {
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
