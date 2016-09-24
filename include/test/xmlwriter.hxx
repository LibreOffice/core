/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_XMLWRITER_HXX
#define INCLUDED_TEST_XMLWRITER_HXX

#include <test/testdllapi.hxx>

#include <libxml/xmlwriter.h>
#include <rtl/ustring.hxx>
#include <rtl/string.hxx>

class SvStream;

class OOO_DLLPUBLIC_TEST XmlWriter
{
private:
    SvStream* mpStream;
    xmlTextWriterPtr mpWriter;

public:
    XmlWriter(SvStream* pStream);
    virtual ~XmlWriter();

    void startDocument();
    void endDocument();

    void element(const OString& sTagName);

    void startElement(const OString& sTagName);
    void endElement();

    void attribute(const OString& sTagName, const OString& aValue);
    void attribute(const OString& sTagName, const OUString& aValue);
    void attribute(const OString& sTagName, sal_Int32 aNumber);

    void content(const OString& aValue);
    void content(const OUString& aValue);
};

#endif // INCLUDED_TEST_XMLWRITER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
