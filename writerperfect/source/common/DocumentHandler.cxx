/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information visit http://libwpd.sourceforge.net
 */

#include <DocumentHandler.hxx>

#include <string.h>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>

#include <xmloff/attrlist.hxx>
#include <xmloff/xmlimp.hxx>

namespace writerperfect
{
const unsigned char librvng_utf8_skip_data[256]
    = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
        3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 1, 1 };

static const char* librvng_utf8_next_char(const char* p)
{
    return p + librvng_utf8_skip_data[*reinterpret_cast<unsigned char const*>(p)];
}

static void unescapeXML(const char* s, const unsigned long sz, librevenge::RVNGString& res)
{
    const char* p = s;
    const char* const end = p + sz;
    while (p != end)
    {
        const char* const next = librvng_utf8_next_char(p);
        if (next > end)
        {
            // oops, the string is invalid
            break;
        }
        if (p + 4 <= end && p + 1 == next && *p == '&')
        {
            // look for &amp; , &lt; , &gt; , &apos; , &quot;
            bool escapedChar = false;
            switch (*(p + 1))
            {
                case 'a':
                    if (p + 5 <= end && strncmp(p, "&amp;", 5) == 0)
                    {
                        res.append('&');
                        p += 5;
                        escapedChar = true;
                    }
                    else if (p + 6 <= end && strncmp(p, "&apos;", 6) == 0)
                    {
                        res.append('\'');
                        p += 6;
                        escapedChar = true;
                    }
                    break;
                case 'g':
                    if (strncmp(p, "&gt;", 4) == 0)
                    {
                        res.append('>');
                        p += 4;
                        escapedChar = true;
                    }
                    break;
                case 'l':
                    if (strncmp(p, "&lt;", 4) == 0)
                    {
                        res.append('<');
                        p += 4;
                        escapedChar = true;
                    }
                    break;
                case 'q':
                    if (p + 6 <= end && strncmp(p, "&quot;", 6) == 0)
                    {
                        res.append('"');
                        p += 6;
                        escapedChar = true;
                    }
                    break;
                default:
                    break;
            }
            if (escapedChar)
                continue;
        }

        while (p != next)
        {
            res.append(*p);
            ++p;
        }
        p = next;
    }
}

using com::sun::star::uno::Reference;
using com::sun::star::xml::sax::XAttributeList;
using com::sun::star::xml::sax::XDocumentHandler;

DocumentHandler::DocumentHandler(Reference<XDocumentHandler> const& xHandler)
    : mxHandler(xHandler)
{
    if (SvXMLImport* pFastHandler = dynamic_cast<SvXMLImport*>(mxHandler.get()))
        mxHandler.set(new SvXMLLegacyToFastDocHandler(pFastHandler));
}

void DocumentHandler::startDocument() { mxHandler->startDocument(); }

void DocumentHandler::endDocument() { mxHandler->endDocument(); }

void DocumentHandler::startElement(const char* psName,
                                   const librevenge::RVNGPropertyList& xPropList)
{
    rtl::Reference<SvXMLAttributeList> pAttrList = new SvXMLAttributeList();
    librevenge::RVNGPropertyList::Iter i(xPropList);
    for (i.rewind(); i.next();)
    {
        // filter out librevenge elements
        if (strncmp(i.key(), "librevenge", 10) != 0)
        {
            size_t keyLength = strlen(i.key());
            OUString sName(i.key(), keyLength, RTL_TEXTENCODING_UTF8);
            OUString sValue(i()->getStr().cstr(), i()->getStr().len(), RTL_TEXTENCODING_UTF8);

            // libodfgen xml-encodes some attribute's value, so check if the value is encoded or not
            for (int j = 0; j < 9; ++j)
            {
                // list of the encoded attributes followed by their lengths
                static char const* listEncoded[9]
                    = { "draw:name",        "svg:font-family",  "style:condition",
                        "style:num-prefix", "style:num-suffix", "table:formula",
                        "text:bullet-char", "text:label",       "xlink:href" };
                static size_t const listEncodedLength[9] = { 9, 15, 15, 16, 16, 13, 16, 10, 10 };
                if (keyLength == listEncodedLength[j]
                    && strncmp(i.key(), listEncoded[j], keyLength) == 0)
                {
                    librevenge::RVNGString decodedValue("");
                    unescapeXML(i()->getStr().cstr(),
                                static_cast<unsigned long>(i()->getStr().len()), decodedValue);
                    sValue
                        = OUString(decodedValue.cstr(), decodedValue.len(), RTL_TEXTENCODING_UTF8);
                    break;
                }
            }
            pAttrList->AddAttribute(sName, sValue);
        }
    }

    OUString sElementName(psName, strlen(psName), RTL_TEXTENCODING_UTF8);
    mxHandler->startElement(sElementName, pAttrList);
}

void DocumentHandler::endElement(const char* psName)
{
    OUString sElementName(psName, strlen(psName), RTL_TEXTENCODING_UTF8);
    mxHandler->endElement(sElementName);
}

void DocumentHandler::characters(const librevenge::RVNGString& sCharacters)
{
    OUString sCharU16(sCharacters.cstr(), strlen(sCharacters.cstr()), RTL_TEXTENCODING_UTF8);
    mxHandler->characters(sCharU16);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
