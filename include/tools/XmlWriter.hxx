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

#include <sal/config.h>

#include <basegfx/numeric/ftools.hxx>
#include <tools/toolsdllapi.h>
#include <rtl/string.hxx>
#include <memory>
#include <string_view>
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

    bool startDocument(sal_Int32 nIndent = 2, bool bWriteXmlHeader = true);
    void endDocument();

    void startElement(const char* sName);
    void startElement(const OString& sName) { startElement(sName.getStr()); }
    void startElement(const OString& sPrefix, const OString& sName, const OString& sNamespaceUri);
    void endElement();

    void attribute(const char* sTagName, const OString& aValue);
    void attribute(const OString& sTagName, const OString& aValue)
    {
        attribute(sTagName.getStr(), aValue);
    }
    void attribute(const char* sTagName, std::u16string_view aValue);
    void attribute(const char* sTagName, sal_Int64 aNumber);
    template <typename T>
    requires std::is_arithmetic_v<T> void attribute(const char* sTagName, T aNumber)
    {
        if constexpr (std::is_floating_point_v<T>)
            return attribute(sTagName, basegfx::fround64(aNumber));
        else
            return attribute(sTagName, static_cast<sal_Int64>(aNumber));
    }
    void attributeDouble(const char* sTagName, double aNumber);
    void attributeBase64(const char* sTagName, std::vector<sal_uInt8> const& rValueInBytes);
    void attributeBase64(const char* sTagName, std::vector<char> const& rValueInBytes);

    void content(const OString& sValue);
    void content(std::u16string_view sValue);

    void element(const char* sName);
};

} // end tools namespace

#endif // INCLUDED_TOOLS_XMLWRITER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
