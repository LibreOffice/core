/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* DocumentElement: The items we are collecting to be put into the Writer
 * document: paragraph and spans of text, as well as section breaks.
 *
 * Copyright (C) 2002-2003 William Lachance (wrlach@gmail.com)
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

/* "This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */

#include "DocumentElement.hxx"
#include "OdfDocumentHandler.hxx"
#include "FilterInternal.hxx"
#include <string.h>

#define ASCII_SPACE 0x0020

void TagElement::print() const
{
    WRITER_DEBUG_MSG(("%s\n", msTagName.cstr()));
}

void TagOpenElement::write(OdfDocumentHandler *pHandler) const
{
    pHandler->startElement(getTagName().cstr(), maAttrList);
}

void TagOpenElement::print() const
{
    TagElement::print();
}

void TagOpenElement::addAttribute(const WPXString &szAttributeName, const WPXString &sAttributeValue)
{
        maAttrList.insert(szAttributeName.cstr(), sAttributeValue);
}

void TagCloseElement::write(OdfDocumentHandler *pHandler) const
{
    WRITER_DEBUG_MSG(("TagCloseElement: write (%s)\n", getTagName().cstr()));

    pHandler->endElement(getTagName().cstr());
}

void CharDataElement::write(OdfDocumentHandler *pHandler) const
{
    WRITER_DEBUG_MSG(("TextElement: write\n"));
    pHandler->characters(msData);
}

void TextElement::write(OdfDocumentHandler *pHandler) const
{
    if (msTextBuf.len() <= 0)
        return;
    pHandler->characters(msTextBuf);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
