/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "txtparai.hxx"

#include "xmlimp.hxx"

using namespace com::sun::star;

namespace writerperfect
{
namespace exp
{

/// Handler for <text:span>.
class XMLSpanContext : public XMLImportContext
{
public:
    XMLSpanContext(XMLImport &rImport);

    void SAL_CALL startElement(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;
    void SAL_CALL endElement(const OUString &rName) override;
    void SAL_CALL characters(const OUString &rChars) override;
};

XMLSpanContext::XMLSpanContext(XMLImport &rImport)
    : XMLImportContext(rImport)
{
}

void XMLSpanContext::startElement(const OUString &/*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    mrImport.GetGenerator().openSpan(librevenge::RVNGPropertyList());
}

void XMLSpanContext::endElement(const OUString &/*rName*/)
{
    mrImport.GetGenerator().closeSpan();
}

void XMLSpanContext::characters(const OUString &rChars)
{
    OString sCharU8 = OUStringToOString(rChars, RTL_TEXTENCODING_UTF8);
    mrImport.GetGenerator().insertText(librevenge::RVNGString(sCharU8.getStr()));
}

XMLParaContext::XMLParaContext(XMLImport &rImport)
    : XMLImportContext(rImport)
{
}

XMLImportContext *XMLParaContext::CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    if (rName == "text:span")
        return new XMLSpanContext(mrImport);
    return nullptr;
}

void XMLParaContext::startElement(const OUString &/*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs)
{
    librevenge::RVNGPropertyList aPropertyList;
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        const OUString &rAttributeName = xAttribs->getNameByIndex(i);
        if (rAttributeName != "text:style-name")
        {
            OString sName = OUStringToOString(rAttributeName, RTL_TEXTENCODING_UTF8);
            OString sValue = OUStringToOString(xAttribs->getValueByIndex(i), RTL_TEXTENCODING_UTF8);
            aPropertyList.insert(sName.getStr(), sValue.getStr());
        }
    }

    mrImport.GetGenerator().openParagraph(aPropertyList);
}

void XMLParaContext::endElement(const OUString &/*rName*/)
{
    mrImport.GetGenerator().closeParagraph();
}

void XMLParaContext::characters(const OUString &rChars)
{
    OString sCharU8 = OUStringToOString(rChars, RTL_TEXTENCODING_UTF8);
    mrImport.GetGenerator().insertText(librevenge::RVNGString(sCharU8.getStr()));
}

} // namespace exp
} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
