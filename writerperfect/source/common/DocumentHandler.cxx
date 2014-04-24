/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * For further information visit http://libwpd.sourceforge.net
 */

#include <writerperfect/DocumentHandler.hxx>

#include <string.h>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>

#include <xmloff/attrlist.hxx>

namespace writerperfect
{

using com::sun::star::uno::Reference;
using com::sun::star::xml::sax::XAttributeList;
using com::sun::star::xml::sax::XDocumentHandler;

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
    SvXMLAttributeList *pAttrList = new SvXMLAttributeList();
    Reference < XAttributeList > xAttrList(pAttrList);
    WPXPropertyList::Iter i(xPropList);
    for (i.rewind(); i.next(); )
    {
        // filter out libwpd elements
        if (strncmp(i.key(), "libwpd", 6) != 0)
        {
            OUString sName(i.key(), strlen(i.key()),  RTL_TEXTENCODING_UTF8);
            OUString sValue(i()->getStr().cstr(), strlen(i()->getStr().cstr()), RTL_TEXTENCODING_UTF8);
            pAttrList->AddAttribute(sName, sValue);
        }
    }

    OUString sElementName(psName, strlen(psName), RTL_TEXTENCODING_UTF8);
    mxHandler->startElement(sElementName, xAttrList);
}

void DocumentHandler::endElement(const char *psName)
{
    OUString sElementName(psName, strlen(psName), RTL_TEXTENCODING_UTF8);
    mxHandler->endElement(sElementName);
}

void DocumentHandler::characters(const WPXString &sCharacters)
{
    OUString sCharU16(sCharacters.cstr(), strlen(sCharacters.cstr()), RTL_TEXTENCODING_UTF8);
    mxHandler->characters(sCharU16);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
