/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information visit http://libwpd.sourceforge.net
 */
#include "DocumentHandler.hxx"
#include "FilterInternal.hxx"

#include <string.h>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>

#include <xmloff/attrlist.hxx>

// #define DEBUG_XML 1

using namespace ::rtl;
using rtl::OUString;

using com::sun::star::xml::sax::XAttributeList;

DocumentHandler::DocumentHandler(Reference < XDocumentHandler > &xHandler) :
    mxHandler(xHandler)
{
}

void DocumentHandler::startDocument()
{
    mxHandler->startDocument();
}

void DocumentHandler::endDocument()
{
    mxHandler->endDocument();
}

void DocumentHandler::startElement(const char *psName, const WPXPropertyList &xPropList)
{
#ifdef DEBUG_XML
    printf("<%s", psName);
#endif
    SvXMLAttributeList *pAttrList = new SvXMLAttributeList();
    Reference < XAttributeList > xAttrList(pAttrList);
    WPXPropertyList::Iter i(xPropList);
    for (i.rewind(); i.next(); )
    {
        // filter out libwpd elements
        if (strncmp(i.key(), "libwpd", 6) != 0)
        {
            pAttrList->AddAttribute(OUString::createFromAscii(i.key()),
                                    OUString::createFromAscii(i()->getStr().cstr()));
#ifdef DEBUG_XML
            printf(" %s=\"%s\"", i.key(), i()->getStr().cstr());
#endif
        }
    }
#ifdef DEBUG_XML
    printf(">");
#endif

    mxHandler->startElement(OUString::createFromAscii(psName), xAttrList);
}

void DocumentHandler::endElement(const char *psName)
{
#ifdef DEBUG_XML
    printf("</%s>", psName);
#endif
    mxHandler->endElement(OUString::createFromAscii(psName));
}

void DocumentHandler::characters(const WPXString &sCharacters)
{
    OUString sCharU16(sCharacters.cstr(), strlen(sCharacters.cstr()), RTL_TEXTENCODING_UTF8);
#ifdef DEBUG_XML
    WPXString sEscapedCharacters(sCharacters, true);
    printf("%s", sEscapedCharacters.cstr());
#endif
    mxHandler->characters(sCharU16);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
