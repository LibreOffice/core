/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _DOCUMENTHANDLER_H
#define _DOCUMENTHANDLER_H
#include <libwpd/libwpd.h>

enum OdfStreamType { ODF_FLAT_XML, ODF_CONTENT_XML, ODF_STYLES_XML, ODF_SETTINGS_XML, ODF_META_XML };

class OdfDocumentHandler
{
public:
    OdfDocumentHandler() {};
    virtual ~OdfDocumentHandler() {};

    virtual void startDocument() = 0;
    virtual void endDocument() = 0;
    virtual void startElement(const char *psName, const WPXPropertyList &xPropList) = 0;
    virtual void endElement(const char *psName) = 0;
    virtual void characters(const WPXString &sCharacters) = 0;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
