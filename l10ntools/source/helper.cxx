/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "helper.hxx"

namespace helper {

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
            if (rString.match("&amp;", i))
                sReturn.append('&');
            else
                sReturn.append("&amp;");
            break;
        }
    }
    return sReturn.makeStringAndClear();
}

bool isWellFormedXML( OString const & text )
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

//Convert xmlChar* to OString
OString xmlStrToOString( const xmlChar* pString )
{
    xmlChar* pTemp = xmlStrdup( pString );
    OString sResult =
        static_cast<OString>(reinterpret_cast<sal_Char*>( pTemp ));
    xmlFree( pTemp );
    return sResult;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
