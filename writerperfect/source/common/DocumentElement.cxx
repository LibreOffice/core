/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* DocumentElement: The items we are collecting to be put into the Writer
 * document: paragraph and spans of text, as well as section breaks.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "DocumentElement.hxx"
#include "OdfDocumentHandler.hxx"
#include "FilterInternal.hxx"
#include <string.h>

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
