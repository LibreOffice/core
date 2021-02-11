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
#include "txtparai.hxx"
#include "xmlimp.hxx"
#include "xmltext.hxx"

#include <sal/log.hxx>

using namespace com::sun::star;

namespace writerperfect::exp
{
namespace
{
/// Handler for <draw:text-box>.
class XMLTextBoxContext : public XMLImportContext
{
public:
    XMLTextBoxContext(XMLImport& rImport);

    rtl::Reference<XMLImportContext>
    CreateChildContext(const OUString& rName,
                       const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs) override;

    void SAL_CALL
    startElement(const OUString& rName,
                 const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs) override;
    void SAL_CALL endElement(const OUString& rName) override;
};
}

XMLTextBoxContext::XMLTextBoxContext(XMLImport& rImport)
    : XMLImportContext(rImport)
{
}

rtl::Reference<XMLImportContext> XMLTextBoxContext::CreateChildContext(
    const OUString& rName, const css::uno::Reference<css::xml::sax::XAttributeList>& /*xAttribs*/)
{
    return CreateTextChildContext(GetImport(), rName);
}

void XMLTextBoxContext::startElement(
    const OUString& /*rName*/,
    const css::uno::Reference<css::xml::sax::XAttributeList>& /*xAttribs*/)
{
    GetImport().GetGenerator().openTextBox(librevenge::RVNGPropertyList());
}

void XMLTextBoxContext::endElement(const OUString& /*rName*/)
{
    GetImport().GetGenerator().closeTextBox();
}

namespace
{
/// Handler for <draw:image>.
class XMLTextImageContext : public XMLImportContext
{
public:
    XMLTextImageContext(XMLImport& rImport);

    rtl::Reference<XMLImportContext>
    CreateChildContext(const OUString& rName,
                       const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs) override;

    void SAL_CALL
    startElement(const OUString& rName,
                 const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs) override;
    void SAL_CALL endElement(const OUString& rName) override;

private:
    OString m_aMimeType;
    rtl::Reference<XMLBase64ImportContext> m_xBinaryData;
};
}

XMLTextImageContext::XMLTextImageContext(XMLImport& rImport)
    : XMLImportContext(rImport)
{
}

rtl::Reference<XMLImportContext> XMLTextImageContext::CreateChildContext(
    const OUString& rName, const css::uno::Reference<css::xml::sax::XAttributeList>& /*xAttribs*/)
{
    if (rName == "office:binary-data")
    {
        m_xBinaryData = new XMLBase64ImportContext(GetImport());
        return m_xBinaryData;
    }
    return nullptr;
}

void XMLTextImageContext::startElement(
    const OUString& /*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs)
{
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        const OUString& rAttributeName = xAttribs->getNameByIndex(i);
        if (rAttributeName == "loext:mime-type" || rAttributeName == "draw:mime-type")
            m_aMimeType = OUStringToOString(xAttribs->getValueByIndex(i), RTL_TEXTENCODING_UTF8);
    }
}

void XMLTextImageContext::endElement(const OUString& /*rName*/)
{
    librevenge::RVNGPropertyList aPropertyList;

    aPropertyList.insert("librevenge:mime-type", m_aMimeType.getStr());
    if (m_xBinaryData.is())
        aPropertyList.insert("office:binary-data", m_xBinaryData->getBinaryData());

    GetImport().GetGenerator().insertBinaryObject(aPropertyList);
}

XMLTextFrameContext::XMLTextFrameContext(XMLImport& rImport)
    : XMLImportContext(rImport)
{
}

rtl::Reference<XMLImportContext> XMLTextFrameContext::CreateChildContext(
    const OUString& rName, const css::uno::Reference<css::xml::sax::XAttributeList>& /*xAttribs*/)
{
    if (rName == "draw:image")
        return new XMLTextImageContext(GetImport());
    if (rName == "draw:text-box")
        return new XMLTextBoxContext(GetImport());
    SAL_WARN("writerperfect", "XMLTextFrameContext::CreateChildContext: unhandled " << rName);
    return nullptr;
}

void XMLTextFrameContext::startElement(
    const OUString& /*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs)
{
    librevenge::RVNGPropertyList aPropertyList;
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        const OUString& rAttributeName = xAttribs->getNameByIndex(i);
        const OUString& rAttributeValue = xAttribs->getValueByIndex(i);

        if (rAttributeName == "draw:style-name")
            FillStyles(rAttributeValue, GetImport().GetAutomaticGraphicStyles(),
                       GetImport().GetGraphicStyles(), aPropertyList);
        else
        {
            OString sName = OUStringToOString(rAttributeName, RTL_TEXTENCODING_UTF8);
            OString sValue = OUStringToOString(rAttributeValue, RTL_TEXTENCODING_UTF8);
            aPropertyList.insert(sName.getStr(), sValue.getStr());
        }
    }
    GetImport().GetGenerator().openFrame(aPropertyList);
}

void XMLTextFrameContext::endElement(const OUString& /*rName*/)
{
    GetImport().GetGenerator().closeFrame();
}

} // namespace writerperfect::exp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
