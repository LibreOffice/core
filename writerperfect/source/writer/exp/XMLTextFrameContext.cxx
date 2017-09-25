/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "XMLTextFrameContext.hxx"

#include "XMLBase64ImportContext.hxx"
#include "xmlimp.hxx"

using namespace com::sun::star;

namespace writerperfect
{
namespace exp
{

/// Handler for <draw:image>.
class XMLTextImageContext : public XMLImportContext
{
public:
    XMLTextImageContext(XMLImport &rImport);

    rtl::Reference<XMLImportContext> CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;

    void SAL_CALL startElement(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;
    void SAL_CALL endElement(const OUString &rName) override;

private:
    OString m_aMimeType;
    rtl::Reference<XMLBase64ImportContext> m_xBinaryData;
};

XMLTextImageContext::XMLTextImageContext(XMLImport &rImport)
    : XMLImportContext(rImport)
{
}

rtl::Reference<XMLImportContext> XMLTextImageContext::CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    if (rName == "office:binary-data")
    {
        m_xBinaryData = new XMLBase64ImportContext(mrImport);
        return m_xBinaryData.get();
    }
    return nullptr;
}

void XMLTextImageContext::startElement(const OUString &/*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs)
{
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        const OUString &rAttributeName = xAttribs->getNameByIndex(i);
        if (rAttributeName == "loext:mime-type")
            m_aMimeType = OUStringToOString(xAttribs->getValueByIndex(i), RTL_TEXTENCODING_UTF8);
    }
}

void XMLTextImageContext::endElement(const OUString &/*rName*/)
{
    librevenge::RVNGPropertyList aPropertyList;

    aPropertyList.insert("librevenge:mime-type", m_aMimeType.getStr());
    if (m_xBinaryData.is())
        aPropertyList.insert("office:binary-data", m_xBinaryData->getBinaryData());

    mrImport.GetGenerator().insertBinaryObject(aPropertyList);
}

XMLTextFrameContext::XMLTextFrameContext(XMLImport &rImport)
    : XMLImportContext(rImport)
{
}

rtl::Reference<XMLImportContext> XMLTextFrameContext::CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    if (rName == "draw:image")
        return new XMLTextImageContext(mrImport);
    return nullptr;
}

void XMLTextFrameContext::startElement(const OUString &/*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    mrImport.GetGenerator().openFrame(librevenge::RVNGPropertyList());
}

void XMLTextFrameContext::endElement(const OUString &/*rName*/)
{
    mrImport.GetGenerator().closeFrame();
}

} // namespace exp
} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
