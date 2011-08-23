/* 
 * Copyright (C) 2004 William Lachance (william.lachance@sympatico.ca)
 * Copyright (C) 2004 Net Integration Technologies (http://www.net-itech.com)
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
  *
 * For further information visit http://libwpd.sourceforge.net
 *
 */
#include "DocumentHandler.hxx"
#include "FilterInternal.hxx"

#include <string.h>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>

#ifndef _ATTRLIST_HPP_
#include <xmloff/attrlist.hxx>
#endif

using namespace ::rtl;
using rtl::OUString;

using com::sun::star::xml::sax::XAttributeList;

DocumentHandler::DocumentHandler(Reference < XDocumentHandler > &xHandler) :
        mxHandler(xHandler)
{
}

void DocumentHandler::startDocument() 
{
    WRITER_DEBUG_MSG(("DocumentHandler::startDocument\n"));
    mxHandler->startDocument();
}

void DocumentHandler::endDocument()
{
    WRITER_DEBUG_MSG(("DocumentHandler::endDocument\n"));
    mxHandler->endDocument();
}

void DocumentHandler::startElement(const char *psName, const WPXPropertyList &xPropList)
{
    WRITER_DEBUG_MSG(("DocumentHandler::startElement\n"));
        SvXMLAttributeList *pAttrList = new SvXMLAttributeList();
    Reference < XAttributeList > xAttrList(pAttrList);
    WPXPropertyList::Iter i(xPropList);
    for (i.rewind(); i.next(); )
    {
                // filter out libwpd elements
                if (strncmp(i.key(), "libwpd", 6) != 0)
                        pAttrList->AddAttribute(OUString::createFromAscii(i.key()),
                                                OUString::createFromAscii(i()->getStr().cstr()));
        }

        mxHandler->startElement(OUString::createFromAscii(psName), xAttrList);
}

void DocumentHandler::endElement(const char *psName)
{
    WRITER_DEBUG_MSG(("DocumentHandler::endElement\n"));
        mxHandler->endElement(OUString::createFromAscii(psName));
}

void DocumentHandler::characters(const WPXString &sCharacters)
{
    WRITER_DEBUG_MSG(("DocumentHandler::characters\n"));
        OUString sCharU16(sCharacters.cstr(), strlen(sCharacters.cstr()), RTL_TEXTENCODING_UTF8);
        mxHandler->characters(sCharU16);
}
