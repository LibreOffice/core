/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/* "This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */

#include "InternalHandler.hxx"

#include <string.h>

InternalHandler::InternalHandler(std::vector<DocumentElement *> *elements):
    mpElements(elements)
{
}

void InternalHandler::startElement(const char *psName, const WPXPropertyList &xPropList)
{
    TagOpenElement *element = new TagOpenElement(psName);
    WPXPropertyList::Iter i(xPropList);
    for (i.rewind(); i.next(); )
    {
        // filter out libwpd elements
        if (strncmp(i.key(), "libwpd", 6) != 0)
            element->addAttribute(i.key(), i()->getStr());
    }
    mpElements->push_back(element);
}

void InternalHandler::endElement(const char *psName)
{
    mpElements->push_back(new TagCloseElement(psName));
}

void InternalHandler::characters(const WPXString &sCharacters)
{
    mpElements->push_back(new CharDataElement(sCharacters.cstr()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
