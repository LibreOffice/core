/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "txtparai.hxx"

#include "XMLTextFrameContext.hxx"
#include "xmlimp.hxx"

using namespace com::sun::star;

namespace
{

/// Looks for rName in rAutomaticStyles (and failing that, in rNamedStyles) and
/// fills rPropertyList based on that.
void FillStyles(const OUString &rName,
                std::map<OUString, librevenge::RVNGPropertyList> &rAutomaticStyles,
                std::map<OUString, librevenge::RVNGPropertyList> &rNamedStyles,
                librevenge::RVNGPropertyList &rPropertyList);

/// Looks for rName in rStyles and fills rPropertyList based on that
/// (rAutomaticStyles and rNamedStyles are a list of possible parents).
void FillStyle(const OUString &rName,
               std::map<OUString, librevenge::RVNGPropertyList> &rStyles,
               std::map<OUString, librevenge::RVNGPropertyList> &rAutomaticStyles,
               std::map<OUString, librevenge::RVNGPropertyList> &rNamedStyles,
               librevenge::RVNGPropertyList &rPropertyList)
{
    auto itStyle = rStyles.find(rName);
    if (itStyle == rStyles.end())
        return;

    const librevenge::RVNGPropertyList &rStyle = itStyle->second;
    if (rStyle["style:parent-style-name"])
    {
        // Style has a parent.
        OUString aParent = OStringToOUString(rStyle["style:parent-style-name"]->getStr().cstr(), RTL_TEXTENCODING_UTF8);
        if (!aParent.isEmpty())
            FillStyles(aParent, rAutomaticStyles, rNamedStyles, rPropertyList);
    }

    // Apply properties from named style.
    librevenge::RVNGPropertyList::Iter itProp(rStyle);
    for (itProp.rewind(); itProp.next();)
    {
        if (OString("style:parent-style-name") != itProp.key())
            rPropertyList.insert(itProp.key(), itProp()->clone());
    }
}

void FillStyles(const OUString &rName,
                std::map<OUString, librevenge::RVNGPropertyList> &rAutomaticStyles,
                std::map<OUString, librevenge::RVNGPropertyList> &rNamedStyles,
                librevenge::RVNGPropertyList &rPropertyList)
{
    FillStyle(rName, rAutomaticStyles, rAutomaticStyles, rNamedStyles, rPropertyList);
    FillStyle(rName, rNamedStyles, rAutomaticStyles, rNamedStyles, rPropertyList);
}

}

namespace writerperfect
{
namespace exp
{

/// Handler for <text:span>.
class XMLSpanContext : public XMLImportContext
{
public:
    XMLSpanContext(XMLImport &rImport, librevenge::RVNGPropertyList *pPropertyList);

    XMLImportContext *CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;

    void SAL_CALL startElement(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;
    void SAL_CALL characters(const OUString &rChars) override;

private:
    librevenge::RVNGPropertyList m_aPropertyList;
};

XMLSpanContext::XMLSpanContext(XMLImport &rImport, librevenge::RVNGPropertyList *pPropertyList)
    : XMLImportContext(rImport)
{
    if (!pPropertyList)
        return;

    // Inherit properties from parent span.
    librevenge::RVNGPropertyList::Iter itProp(*pPropertyList);
    for (itProp.rewind(); itProp.next();)
        m_aPropertyList.insert(itProp.key(), itProp()->clone());
}

XMLImportContext *XMLSpanContext::CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    if (rName == "draw:frame")
        return new XMLTextFrameContext(mrImport);
    if (rName == "text:span")
        return new XMLSpanContext(mrImport, &m_aPropertyList);
    return nullptr;
}

void XMLSpanContext::startElement(const OUString &/*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs)
{
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        const OUString &rAttributeName = xAttribs->getNameByIndex(i);
        const OUString &rAttributeValue = xAttribs->getValueByIndex(i);
        if (rAttributeName == "text:style-name")
            FillStyles(rAttributeValue, mrImport.GetAutomaticTextStyles(), mrImport.GetTextStyles(), m_aPropertyList);
        else
        {
            OString sName = OUStringToOString(rAttributeName, RTL_TEXTENCODING_UTF8);
            OString sValue = OUStringToOString(rAttributeValue, RTL_TEXTENCODING_UTF8);
            m_aPropertyList.insert(sName.getStr(), sValue.getStr());
        }
    }
}

void XMLSpanContext::characters(const OUString &rChars)
{
    mrImport.GetGenerator().openSpan(m_aPropertyList);

    OString sCharU8 = OUStringToOString(rChars, RTL_TEXTENCODING_UTF8);
    mrImport.GetGenerator().insertText(librevenge::RVNGString(sCharU8.getStr()));

    mrImport.GetGenerator().closeSpan();
}

/// Handler for <text:a>.
class XMLHyperlinkContext : public XMLImportContext
{
public:
    XMLHyperlinkContext(XMLImport &rImport);

    void SAL_CALL startElement(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;
    void SAL_CALL endElement(const OUString &rName) override;
    void SAL_CALL characters(const OUString &rChars) override;
};

XMLHyperlinkContext::XMLHyperlinkContext(XMLImport &rImport)
    : XMLImportContext(rImport)
{
}

void XMLHyperlinkContext::startElement(const OUString &/*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs)
{
    librevenge::RVNGPropertyList aPropertyList;
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        const OUString &rAttributeName = xAttribs->getNameByIndex(i);
        if (rAttributeName == "xlink:href")
        {
            OString sName = OUStringToOString(rAttributeName, RTL_TEXTENCODING_UTF8);
            OString sValue = OUStringToOString(xAttribs->getValueByIndex(i), RTL_TEXTENCODING_UTF8);
            aPropertyList.insert(sName.getStr(), sValue.getStr());
        }
    }

    mrImport.GetGenerator().openLink(aPropertyList);
}

void XMLHyperlinkContext::endElement(const OUString &/*rName*/)
{
    mrImport.GetGenerator().closeLink();
}

void XMLHyperlinkContext::characters(const OUString &rChars)
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
        return new XMLSpanContext(mrImport, nullptr);
    if (rName == "text:a")
        return new XMLHyperlinkContext(mrImport);
    return nullptr;
}

void XMLParaContext::startElement(const OUString &/*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs)
{
    librevenge::RVNGPropertyList aPropertyList;
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        const OUString &rAttributeName = xAttribs->getNameByIndex(i);
        const OUString &rAttributeValue = xAttribs->getValueByIndex(i);
        if (rAttributeName == "text:style-name")
        {
            m_aStyleName = rAttributeValue;
            FillStyles(m_aStyleName, mrImport.GetAutomaticParagraphStyles(), mrImport.GetParagraphStyles(), aPropertyList);
        }
        else
        {
            OString sName = OUStringToOString(rAttributeName, RTL_TEXTENCODING_UTF8);
            OString sValue = OUStringToOString(rAttributeValue, RTL_TEXTENCODING_UTF8);
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
    librevenge::RVNGPropertyList aPropertyList;
    if (!m_aStyleName.isEmpty())
        FillStyles(m_aStyleName, mrImport.GetAutomaticTextStyles(), mrImport.GetTextStyles(), aPropertyList);
    mrImport.GetGenerator().openSpan(aPropertyList);

    OString sCharU8 = OUStringToOString(rChars, RTL_TEXTENCODING_UTF8);
    mrImport.GetGenerator().insertText(librevenge::RVNGString(sCharU8.getStr()));

    mrImport.GetGenerator().closeSpan();
}

} // namespace exp
} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
