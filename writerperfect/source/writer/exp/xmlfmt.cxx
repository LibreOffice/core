/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xmlfmt.hxx"

#include "XMLBase64ImportContext.hxx"
#include "txtstyli.hxx"
#include "xmlimp.hxx"

using namespace com::sun::star;

namespace writerperfect
{
namespace exp
{

XMLStylesContext::XMLStylesContext(XMLImport &rImport, StyleType eType)
    : XMLImportContext(rImport),
      m_rParagraphStyles(eType == StyleType_AUTOMATIC ? mrImport.GetAutomaticParagraphStyles() : mrImport.GetParagraphStyles()),
      m_rTextStyles(eType == StyleType_AUTOMATIC ? mrImport.GetAutomaticTextStyles() : mrImport.GetTextStyles()),
      m_rCellStyles(eType == StyleType_AUTOMATIC ? mrImport.GetAutomaticCellStyles() : mrImport.GetCellStyles()),
      m_rColumnStyles(eType == StyleType_AUTOMATIC ? mrImport.GetAutomaticColumnStyles() : mrImport.GetColumnStyles()),
      m_rRowStyles(eType == StyleType_AUTOMATIC ? mrImport.GetAutomaticRowStyles() : mrImport.GetRowStyles()),
      m_rTableStyles(eType == StyleType_AUTOMATIC ? mrImport.GetAutomaticTableStyles() : mrImport.GetTableStyles()),
      m_rGraphicStyles(eType == StyleType_AUTOMATIC ? mrImport.GetAutomaticGraphicStyles() : mrImport.GetGraphicStyles())
{
}

rtl::Reference<XMLImportContext> XMLStylesContext::CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    if (rName == "style:style")
        return new XMLStyleContext(mrImport, *this);
    return nullptr;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLStylesContext::GetCurrentParagraphStyles()
{
    return m_rParagraphStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLStylesContext::GetCurrentTextStyles()
{
    return m_rTextStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLStylesContext::GetCurrentCellStyles()
{
    return m_rCellStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLStylesContext::GetCurrentColumnStyles()
{
    return m_rColumnStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLStylesContext::GetCurrentRowStyles()
{
    return m_rRowStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLStylesContext::GetCurrentTableStyles()
{
    return m_rTableStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLStylesContext::GetCurrentGraphicStyles()
{
    return m_rGraphicStyles;
}

/// Handler for <style:font-face>.
class XMLFontFaceContext : public XMLImportContext
{
public:
    XMLFontFaceContext(XMLImport &rImport);
    void SAL_CALL startElement(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;

    rtl::Reference<XMLImportContext> CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;

    OUString maName;
};

/// Handler for <svg:font-face-src>.
class XMLFontFaceSrcContext : public XMLImportContext
{
public:
    XMLFontFaceSrcContext(XMLImport &rImport, XMLFontFaceContext &rFontFace);

    rtl::Reference<XMLImportContext> CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;

private:
    XMLFontFaceContext &mrFontFace;
};

/// Handler for <svg:font-face-uri>.
class XMLFontFaceUriContext : public XMLImportContext
{
public:
    XMLFontFaceUriContext(XMLImport &rImport, XMLFontFaceContext const &rFontFace);
    void SAL_CALL startElement(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;
    void SAL_CALL endElement(const OUString &rName) override;

    rtl::Reference<XMLImportContext> CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;

    librevenge::RVNGPropertyList maPropertyList;

private:
    rtl::Reference<XMLBase64ImportContext> mxBinaryData;
};

/// Handler for <svg:font-face-format>.
class XMLFontFaceFormatContext : public XMLImportContext
{
public:
    XMLFontFaceFormatContext(XMLImport &rImport, XMLFontFaceUriContext &rFontFaceUri);
    void SAL_CALL startElement(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;

private:
    XMLFontFaceUriContext &mrFontFaceUri;
};

XMLFontFaceFormatContext::XMLFontFaceFormatContext(XMLImport &rImport, XMLFontFaceUriContext &rFontFaceUri)
    : XMLImportContext(rImport)
    , mrFontFaceUri(rFontFaceUri)
{
}

void XMLFontFaceFormatContext::startElement(const OUString &/*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs)
{
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        const OUString &rAttributeName = xAttribs->getNameByIndex(i);
        const OUString &rAttributeValue = xAttribs->getValueByIndex(i);
        if (rAttributeName == "svg:string")
        {
            OString aAttributeValueU8 = OUStringToOString(rAttributeValue, RTL_TEXTENCODING_UTF8);
            mrFontFaceUri.maPropertyList.insert("librevenge:mime-type", aAttributeValueU8.getStr());
        }
    }
}

XMLFontFaceUriContext::XMLFontFaceUriContext(XMLImport &rImport, XMLFontFaceContext const &rFontFace)
    : XMLImportContext(rImport)
{
    OString aNameU8 = OUStringToOString(rFontFace.maName, RTL_TEXTENCODING_UTF8);
    maPropertyList.insert("librevenge:name", aNameU8.getStr());
}

void XMLFontFaceUriContext::startElement(const OUString &/*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs)
{
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        const OUString &rAttributeName = xAttribs->getNameByIndex(i);
        const OUString &rAttributeValue = xAttribs->getValueByIndex(i);
        if (rAttributeName == "loext:font-style")
        {
            OString aAttributeValueU8 = OUStringToOString(rAttributeValue, RTL_TEXTENCODING_UTF8);
            maPropertyList.insert("librevenge:font-style", aAttributeValueU8.getStr());
        }
        else if (rAttributeName == "loext:font-weight")
        {
            OString aAttributeValueU8 = OUStringToOString(rAttributeValue, RTL_TEXTENCODING_UTF8);
            maPropertyList.insert("librevenge:font-weight", aAttributeValueU8.getStr());
        }
    }
}

void XMLFontFaceUriContext::endElement(const OUString &/*rName*/)
{
    if (mxBinaryData.is())
        maPropertyList.insert("office:binary-data", mxBinaryData->getBinaryData());
    mrImport.GetGenerator().defineEmbeddedFont(maPropertyList);
}

rtl::Reference<XMLImportContext> XMLFontFaceUriContext::CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    if (rName == "office:binary-data")
    {
        mxBinaryData = new XMLBase64ImportContext(mrImport);
        return mxBinaryData.get();
    }
    if (rName == "svg:font-face-format")
        return new XMLFontFaceFormatContext(mrImport, *this);

    SAL_WARN("writerperfect", "XMLFontFaceUriContext::CreateChildContext: unhandled " << rName);
    return nullptr;
}

XMLFontFaceSrcContext::XMLFontFaceSrcContext(XMLImport &rImport, XMLFontFaceContext &rFontFace)
    : XMLImportContext(rImport)
    , mrFontFace(rFontFace)
{
}

rtl::Reference<XMLImportContext> XMLFontFaceSrcContext::CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    if (rName == "svg:font-face-uri")
        return new XMLFontFaceUriContext(mrImport, mrFontFace);
    SAL_WARN("writerperfect", "XMLFontFaceSrcContext::CreateChildContext: unhandled " << rName);
    return nullptr;
}

XMLFontFaceContext::XMLFontFaceContext(XMLImport &rImport)
    : XMLImportContext(rImport)
{
}

void XMLFontFaceContext::startElement(const OUString &/*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs)
{
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        const OUString &rAttributeName = xAttribs->getNameByIndex(i);
        const OUString &rAttributeValue = xAttribs->getValueByIndex(i);
        if (rAttributeName == "style:name")
            maName = rAttributeValue;
    }
}

rtl::Reference<XMLImportContext> XMLFontFaceContext::CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    if (rName == "svg:font-face-src")
        return new XMLFontFaceSrcContext(mrImport, *this);
    SAL_WARN("writerperfect", "XMLFontFaceContext::CreateChildContext: unhandled " << rName);
    return nullptr;
}

XMLFontFaceDeclsContext::XMLFontFaceDeclsContext(XMLImport &rImport)
    : XMLImportContext(rImport)
{
}

rtl::Reference<XMLImportContext> XMLFontFaceDeclsContext::CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    if (rName == "style:font-face")
        return new XMLFontFaceContext(mrImport);
    return nullptr;
}

} // namespace exp
} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
