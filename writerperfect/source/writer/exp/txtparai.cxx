/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "txtparai.hxx"

#include "XMLFootnoteImportContext.hxx"
#include "XMLTextFrameContext.hxx"
#include "xmlimp.hxx"

using namespace com::sun::star;

namespace
{

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
            writerperfect::exp::FillStyles(aParent, rAutomaticStyles, rNamedStyles, rPropertyList);
    }

    // Apply properties from named style.
    librevenge::RVNGPropertyList::Iter itProp(rStyle);
    for (itProp.rewind(); itProp.next();)
    {
        if (OString("style:parent-style-name") != itProp.key())
            rPropertyList.insert(itProp.key(), itProp()->clone());
    }
}

}

namespace writerperfect
{
namespace exp
{

/// Handler for <text:sequence>.
class XMLTextSequenceContext : public XMLImportContext
{
public:
    XMLTextSequenceContext(XMLImport &rImport, const librevenge::RVNGPropertyList &rPropertyList);

    void SAL_CALL characters(const OUString &rChars) override;

private:
    librevenge::RVNGPropertyList m_aPropertyList;
};

XMLTextSequenceContext::XMLTextSequenceContext(XMLImport &rImport, const librevenge::RVNGPropertyList &rPropertyList)
    : XMLImportContext(rImport)
{
    // Inherit properties from parent.
    librevenge::RVNGPropertyList::Iter itProp(rPropertyList);
    for (itProp.rewind(); itProp.next();)
        m_aPropertyList.insert(itProp.key(), itProp()->clone());
}

void XMLTextSequenceContext::characters(const OUString &rChars)
{
    mrImport.GetGenerator().openSpan(m_aPropertyList);

    OString sCharU8 = OUStringToOString(rChars, RTL_TEXTENCODING_UTF8);
    mrImport.GetGenerator().insertText(librevenge::RVNGString(sCharU8.getStr()));

    mrImport.GetGenerator().closeSpan();
}

/// Handler for <text:span>.
class XMLSpanContext : public XMLImportContext
{
public:
    XMLSpanContext(XMLImport &rImport, const librevenge::RVNGPropertyList &rPropertyList);

    rtl::Reference<XMLImportContext> CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;

    void SAL_CALL startElement(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;
    void SAL_CALL characters(const OUString &rChars) override;

private:
    librevenge::RVNGPropertyList m_aPropertyList;
};

XMLSpanContext::XMLSpanContext(XMLImport &rImport, const librevenge::RVNGPropertyList &rPropertyList)
    : XMLImportContext(rImport)
{
    // Inherit properties from parent.
    librevenge::RVNGPropertyList::Iter itProp(rPropertyList);
    for (itProp.rewind(); itProp.next();)
        m_aPropertyList.insert(itProp.key(), itProp()->clone());
}

rtl::Reference<XMLImportContext> XMLSpanContext::CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    return CreateParagraphOrSpanChildContext(mrImport, rName, m_aPropertyList);
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

/// Base class for contexts that represent a single character only.
class XMLCharContext : public XMLImportContext
{
public:
    XMLCharContext(XMLImport &rImport, const librevenge::RVNGPropertyList &rPropertyList);

protected:
    librevenge::RVNGPropertyList m_aPropertyList;
};

XMLCharContext::XMLCharContext(XMLImport &rImport, const librevenge::RVNGPropertyList &rPropertyList)
    : XMLImportContext(rImport)
{
    // Inherit properties from parent.
    librevenge::RVNGPropertyList::Iter itProp(rPropertyList);
    for (itProp.rewind(); itProp.next();)
        m_aPropertyList.insert(itProp.key(), itProp()->clone());
}

/// Handler for <text:line-break>.
class XMLLineBreakContext : public XMLCharContext
{
public:
    XMLLineBreakContext(XMLImport &rImport, const librevenge::RVNGPropertyList &rPropertyList);

    void SAL_CALL startElement(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;
};

XMLLineBreakContext::XMLLineBreakContext(XMLImport &rImport, const librevenge::RVNGPropertyList &rPropertyList)
    : XMLCharContext(rImport, rPropertyList)
{
}

void XMLLineBreakContext::startElement(const OUString &/*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    mrImport.GetGenerator().openSpan(m_aPropertyList);
    mrImport.GetGenerator().insertLineBreak();
    mrImport.GetGenerator().closeSpan();
}

/// Handler for <text:s>.
class XMLSpaceContext : public XMLCharContext
{
public:
    XMLSpaceContext(XMLImport &rImport, const librevenge::RVNGPropertyList &rPropertyList);

    void SAL_CALL startElement(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;
};

XMLSpaceContext::XMLSpaceContext(XMLImport &rImport, const librevenge::RVNGPropertyList &rPropertyList)
    : XMLCharContext(rImport, rPropertyList)
{
}

void XMLSpaceContext::startElement(const OUString &/*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    mrImport.GetGenerator().openSpan(m_aPropertyList);
    mrImport.GetGenerator().insertSpace();
    mrImport.GetGenerator().closeSpan();
}

/// Handler for <text:tab>.
class XMLTabContext : public XMLCharContext
{
public:
    XMLTabContext(XMLImport &rImport, const librevenge::RVNGPropertyList &rPropertyList);

    void SAL_CALL startElement(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;
};

XMLTabContext::XMLTabContext(XMLImport &rImport, const librevenge::RVNGPropertyList &rPropertyList)
    : XMLCharContext(rImport, rPropertyList)
{
}

void XMLTabContext::startElement(const OUString &/*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    mrImport.GetGenerator().openSpan(m_aPropertyList);
    mrImport.GetGenerator().insertTab();
    mrImport.GetGenerator().closeSpan();
}

/// Handler for <draw:a>.
class XMLTextFrameHyperlinkContext : public XMLImportContext
{
public:
    XMLTextFrameHyperlinkContext(XMLImport &rImport, const librevenge::RVNGPropertyList &rPropertyList);
    rtl::Reference<XMLImportContext> CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;

    void SAL_CALL startElement(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;
    void SAL_CALL endElement(const OUString &rName) override;
    void SAL_CALL characters(const OUString &rChars) override;

private:
    librevenge::RVNGPropertyList m_aPropertyList;
};

XMLTextFrameHyperlinkContext::XMLTextFrameHyperlinkContext(XMLImport &rImport, const librevenge::RVNGPropertyList &rPropertyList)
    : XMLImportContext(rImport)
{
    // Inherit properties from parent.
    librevenge::RVNGPropertyList::Iter itProp(rPropertyList);
    for (itProp.rewind(); itProp.next();)
        m_aPropertyList.insert(itProp.key(), itProp()->clone());
}

rtl::Reference<XMLImportContext> XMLTextFrameHyperlinkContext::CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    return CreateParagraphOrSpanChildContext(mrImport, rName, m_aPropertyList);
}

void XMLTextFrameHyperlinkContext::startElement(const OUString &/*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs)
{
    librevenge::RVNGPropertyList aPropertyList;
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        const OUString &rAttributeName = xAttribs->getNameByIndex(i);
        const OUString &rAttributeValue = xAttribs->getValueByIndex(i);
        if (rAttributeName == "text:style-name")
            // This affects the nested span's properties.
            FillStyles(rAttributeValue, mrImport.GetAutomaticTextStyles(), mrImport.GetTextStyles(), m_aPropertyList);
        else
        {
            // This affects the link's properties.
            OString sName = OUStringToOString(rAttributeName, RTL_TEXTENCODING_UTF8);
            OString sValue = OUStringToOString(rAttributeValue, RTL_TEXTENCODING_UTF8);
            aPropertyList.insert(sName.getStr(), sValue.getStr());
        }
    }

    mrImport.GetGenerator().openLink(aPropertyList);
}

void XMLTextFrameHyperlinkContext::endElement(const OUString &/*rName*/)
{
    mrImport.GetGenerator().closeLink();
}

void XMLTextFrameHyperlinkContext::characters(const OUString &rChars)
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
    XMLHyperlinkContext(XMLImport &rImport, const librevenge::RVNGPropertyList &rPropertyList);
    rtl::Reference<XMLImportContext> CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;

    void SAL_CALL startElement(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;
    void SAL_CALL endElement(const OUString &rName) override;
    void SAL_CALL characters(const OUString &rChars) override;

private:
    librevenge::RVNGPropertyList m_aPropertyList;
};

XMLHyperlinkContext::XMLHyperlinkContext(XMLImport &rImport, const librevenge::RVNGPropertyList &rPropertyList)
    : XMLImportContext(rImport)
{
    // Inherit properties from parent.
    librevenge::RVNGPropertyList::Iter itProp(rPropertyList);
    for (itProp.rewind(); itProp.next();)
        m_aPropertyList.insert(itProp.key(), itProp()->clone());
}

rtl::Reference<XMLImportContext> XMLHyperlinkContext::CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    return CreateParagraphOrSpanChildContext(mrImport, rName, m_aPropertyList);
}

void XMLHyperlinkContext::startElement(const OUString &/*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs)
{
    librevenge::RVNGPropertyList aPropertyList;
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        const OUString &rAttributeName = xAttribs->getNameByIndex(i);
        const OUString &rAttributeValue = xAttribs->getValueByIndex(i);
        if (rAttributeName == "text:style-name")
            // This affects the nested span's properties.
            FillStyles(rAttributeValue, mrImport.GetAutomaticTextStyles(), mrImport.GetTextStyles(), m_aPropertyList);
        else
        {
            // This affects the link's properties.
            OString sName = OUStringToOString(rAttributeName, RTL_TEXTENCODING_UTF8);
            OString sValue = OUStringToOString(rAttributeValue, RTL_TEXTENCODING_UTF8);
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
    mrImport.GetGenerator().openSpan(m_aPropertyList);

    OString sCharU8 = OUStringToOString(rChars, RTL_TEXTENCODING_UTF8);
    mrImport.GetGenerator().insertText(librevenge::RVNGString(sCharU8.getStr()));

    mrImport.GetGenerator().closeSpan();
}

XMLParaContext::XMLParaContext(XMLImport &rImport)
    : XMLImportContext(rImport)
{
}

rtl::Reference<XMLImportContext> XMLParaContext::CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    if (rName == "text:a")
        return new XMLHyperlinkContext(mrImport, m_aTextPropertyList);
    if (rName == "draw:a")
        return new XMLTextFrameHyperlinkContext(mrImport, m_aTextPropertyList);
    return CreateParagraphOrSpanChildContext(mrImport, rName, m_aTextPropertyList);
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
            FillStyles(m_aStyleName, mrImport.GetAutomaticTextStyles(), mrImport.GetTextStyles(), m_aTextPropertyList);
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

rtl::Reference<XMLImportContext> CreateParagraphOrSpanChildContext(XMLImport &rImport, const OUString &rName, const librevenge::RVNGPropertyList &rTextPropertyList)
{
    if (rName == "text:span")
        return new XMLSpanContext(rImport, rTextPropertyList);
    if (rName == "text:line-break")
        return new XMLLineBreakContext(rImport, rTextPropertyList);
    if (rName == "text:s")
        return new XMLSpaceContext(rImport, rTextPropertyList);
    if (rName == "text:tab")
        return new XMLTabContext(rImport, rTextPropertyList);
    if (rName == "draw:frame")
        return new XMLTextFrameContext(rImport);
    if (rName == "text:sequence")
        return new XMLTextSequenceContext(rImport, rTextPropertyList);
    if (rName == "text:note")
        return new XMLFootnoteImportContext(rImport);
    SAL_WARN("writerperfect", "CreateParagraphOrSpanChildContext: unhandled " << rName);
    return nullptr;
}

void FillStyles(const OUString &rName,
                std::map<OUString, librevenge::RVNGPropertyList> &rAutomaticStyles,
                std::map<OUString, librevenge::RVNGPropertyList> &rNamedStyles,
                librevenge::RVNGPropertyList &rPropertyList)
{
    FillStyle(rName, rAutomaticStyles, rAutomaticStyles, rNamedStyles, rPropertyList);
    FillStyle(rName, rNamedStyles, rAutomaticStyles, rNamedStyles, rPropertyList);
}

} // namespace exp
} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
