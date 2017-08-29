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

namespace writerperfect
{
namespace exp
{

/// Handler for <text:span>.
class XMLSpanContext : public XMLImportContext
{
public:
    XMLSpanContext(XMLImport &rImport);

    XMLImportContext *CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;

    void SAL_CALL startElement(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;
    void SAL_CALL endElement(const OUString &rName) override;
    void SAL_CALL characters(const OUString &rChars) override;
};

XMLSpanContext::XMLSpanContext(XMLImport &rImport)
    : XMLImportContext(rImport)
{
}

XMLImportContext *XMLSpanContext::CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    if (rName == "draw:frame")
        return new XMLTextFrameContext(mrImport);
    return nullptr;
}

void XMLSpanContext::startElement(const OUString &/*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs)
{
    librevenge::RVNGPropertyList aPropertyList;
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        const OUString &rAttributeName = xAttribs->getNameByIndex(i);
        const OUString &rAttributeValue = xAttribs->getValueByIndex(i);
        if (rAttributeName == "text:style-name")
        {
            // Reference to an automatic text style, try to look it up.
            auto itStyle = mrImport.GetAutomaticTextStyles().find(rAttributeValue);
            if (itStyle == mrImport.GetAutomaticTextStyles().end())
                continue;

            // Apply properties directly, librevenge has no notion of automatic styles.
            librevenge::RVNGPropertyList::Iter itProp(itStyle->second);
            for (itProp.rewind(); itProp.next();)
                aPropertyList.insert(itProp.key(), itProp()->clone());
        }
        else
        {
            OString sName = OUStringToOString(rAttributeName, RTL_TEXTENCODING_UTF8);
            OString sValue = OUStringToOString(rAttributeValue, RTL_TEXTENCODING_UTF8);
            aPropertyList.insert(sName.getStr(), sValue.getStr());
        }
    }

    mrImport.GetGenerator().openSpan(aPropertyList);
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
        return new XMLSpanContext(mrImport);
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

            // Reference to an automatic style, try to look it up.
            auto itStyle = mrImport.GetAutomaticParagraphStyles().find(m_aStyleName);
            if (itStyle == mrImport.GetAutomaticParagraphStyles().end())
                continue;

            // Found an automatic paragraph style.
            librevenge::RVNGPropertyList::Iter itProp(itStyle->second);
            for (itProp.rewind(); itProp.next();)
                aPropertyList.insert(itProp.key(), itProp()->clone());
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
    {
        // Reference to an automatic style, try to look it up.
        auto itStyle = mrImport.GetAutomaticTextStyles().find(m_aStyleName);
        if (itStyle != mrImport.GetAutomaticTextStyles().end())
        {
            // Found an automatic text style.
            librevenge::RVNGPropertyList::Iter itProp(itStyle->second);
            for (itProp.rewind(); itProp.next();)
                aPropertyList.insert(itProp.key(), itProp()->clone());
        }
    }
    mrImport.GetGenerator().openSpan(aPropertyList);

    OString sCharU8 = OUStringToOString(rChars, RTL_TEXTENCODING_UTF8);
    mrImport.GetGenerator().insertText(librevenge::RVNGString(sCharU8.getStr()));

    mrImport.GetGenerator().closeSpan();
}

} // namespace exp
} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
