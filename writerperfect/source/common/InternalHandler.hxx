/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
    InternalHandler(const InternalHandler &);
    InternalHandler &operator=(const InternalHandler &);
    std::vector<DocumentElement *> *mpElements;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
