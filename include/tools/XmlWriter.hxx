/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TOOLS_XMLWRITER_HXX
#define INCLUDED_TOOLS_XMLWRITER_HXX

#include <tools/toolsdllapi.h>
#include <rtl/ustring.hxx>
#include <memory>
#include <vector>

class SvStream;

namespace tools
{
struct XmlWriterImpl;

/**
 * XmlWriter writes a XML to a SvStream. It uses libxml2 for writing but hides
 * all the internal libxml2 workings and uses types that are native for LO
 * development.
 *
 * The codepage used for XML is always "utf-8" and the output is indented by
 * default so it is easier to read.
 *
 */
class TOOLS_DLLPUBLIC XmlWriter final
{
private:
    std::unique_ptr<XmlWriterImpl> mpImpl;

public:
    XmlWriter(SvStream* pStream);

    ~XmlWriter();

    bool startDocument(sal_Int32 nIndent = 2);
    void endDocument();

    void startElement(const OString& sName);
    void startElement(const OString& sPrefix, const OString& sName, const OString& sNamespaceUri);
    void endElement();

    void attribute(const OString& sTagName, const OString& aValue);
    void attribute(const OString& sTagName, const OUString& aValue);
    void attribute(const OString& sTagName, sal_Int32 aNumber);
    void attributeDouble(const OString& sTagName, double aNumber);
    void attributeBase64(const OString& sTagName, std::vector<sal_uInt8> const& rValueInBytes);
    void attributeBase64(const OString& sTagName, std::vector<char> const& rValueInBytes);

    void content(const OString& sValue);
    void content(const OUString& sValue);

    void element(const OString& sName);
};

} // end tools namespace

#endif // INCLUDED_TOOLS_XMLWRITER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
