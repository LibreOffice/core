/*
 * Copyright (C) 2007 Fridrich Strba (fridrich.strba@bluewin.ch)
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

#ifndef _INTERNALHANDLER_H
#define _INTERNALHANDLER_H
#include <libwpd/libwpd.h>
#include <libwpd/WPXProperty.h>
#include <libwpd/WPXString.h>
#include "DocumentElement.hxx"
#include "OdfDocumentHandler.hxx"

class InternalHandler : public OdfDocumentHandler
{
public:
    InternalHandler(std::vector<DocumentElement *> *elements);
    ~InternalHandler() {};

    void startDocument() {};
    void endDocument() {};
    void startElement(const char *psName, const WPXPropertyList &xPropList);
    void endElement(const char *psName);
    void characters(const WPXString &sCharacters);
private:
    std::vector<DocumentElement *> *mpElements;
};
#endif
