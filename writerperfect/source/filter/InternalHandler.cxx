/*
 * Copyright (C) 2007 Fridrich Strba .strba@bluewin.ch)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
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
    TagOpenElement    *element = new TagOpenElement(psName);
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
