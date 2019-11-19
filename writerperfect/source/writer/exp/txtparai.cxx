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

#include <sal/log.hxx>

using namespace com::sun::star;

namespace
{
/// Looks for rName in rStyles and fills rPropertyList based on that
/// (rAutomaticStyles and rNamedStyles are a list of possible parents).
void FillStyle(const OUString& rName, std::map<OUString, librevenge::RVNGPropertyList>& rStyles,
               std::map<OUString, librevenge::RVNGPropertyList>& rAutomaticStyles,
               std::map<OUString, librevenge::RVNGPropertyList>& rNamedStyles,
               librevenge::RVNGPropertyList& rPropertyList)
{
    auto itStyle = rStyles.find(rName);
    if (itStyle == rStyles.end())
        return;

    const librevenge::RVNGPropertyList& rStyle = itStyle->second;
    if (rStyle["style:parent-style-name"])
    {
        // The style has a parent.
        OUString aParent = OStringToOUString(rStyle["style:parent-style-name"]->getStr().cstr(),
                                             RTL_TEXTENCODING_UTF8);
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
namespace
{
/// Handler for <text:sequence>.
class XMLTextSequenceContext : public XMLImportContext
{
public:
    XMLTextSequenceContext(XMLImport& rImport, const librevenge::RVNGPropertyList& rPropertyList);

    void SAL_CALL characters(const OUString& rChars) override;

private:
    librevenge::RVNGPropertyList m_aPropertyList;
};
}

XMLTextSequenceContext::XMLTextSequenceContext(XMLImport& rImport,
                                               const librevenge::RVNGPropertyList& rPropertyList)
    : XMLImportContext(rImport)
{
    // Inherit properties from parent.
    librevenge::RVNGPropertyList::Iter itProp(rPropertyList);
    for (itProp.rewind(); itProp.next();)
        m_aPropertyList.insert(itProp.key(), itProp()->clone());
}

void XMLTextSequenceContext::characters(const OUString& rChars)
{
    GetImport().GetGenerator().openSpan(m_aPropertyList);

    OString sCharU8 = OUStringToOString(rChars, RTL_TEXTENCODING_UTF8);
    GetImport().GetGenerator().insertText(librevenge::RVNGString(sCharU8.getStr()));

    GetImport().GetGenerator().closeSpan();
}

namespace
{
/// Handler for <text:span>.
class XMLSpanContext : public XMLImportContext
{
public:
    XMLSpanContext(XMLImport& rImport, const librevenge::RVNGPropertyList& rPropertyList);

    rtl::Reference<XMLImportContext>
    CreateChildContext(const OUString& rName,
                       const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs) override;

    void SAL_CALL
    startElement(const OUString& rName,
                 const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs) override;
    void SAL_CALL characters(const OUString& rChars) override;

private:
    librevenge::RVNGPropertyList m_aPropertyList;
};
}

XMLSpanContext::XMLSpanContext(XMLImport& rImport,
                               const librevenge::RVNGPropertyList& rPropertyList)
    : XMLImportContext(rImport)
{
    // Inherit properties from parent.
    librevenge::RVNGPropertyList::Iter itProp(rPropertyList);
    for (itProp.rewind(); itProp.next();)
        m_aPropertyList.insert(itProp.key(), itProp()->clone());
}

rtl::Reference<XMLImportContext> XMLSpanContext::CreateChildContext(
    const OUString& rName, const css::uno::Reference<css::xml::sax::XAttributeList>& /*xAttribs*/)
{
    return CreateParagraphOrSpanChildContext(GetImport(), rName, m_aPropertyList);
}

void XMLSpanContext::startElement(
    const OUString& /*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs)
{
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        const OUString& rAttributeName = xAttribs->getNameByIndex(i);
        const OUString& rAttributeValue = xAttribs->getValueByIndex(i);
        if (rAttributeName == "text:style-name")
            FillStyles(rAttributeValue, GetImport().GetAutomaticTextStyles(),
                       GetImport().GetTextStyles(), m_aPropertyList);
        else
        {
            OString sName = OUStringToOString(rAttributeName, RTL_TEXTENCODING_UTF8);
            OString sValue = OUStringToOString(rAttributeValue, RTL_TEXTENCODING_UTF8);
            m_aPropertyList.insert(sName.getStr(), sValue.getStr());
        }
    }
}

void XMLSpanContext::characters(const OUString& rChars)
{
    GetImport().GetGenerator().openSpan(m_aPropertyList);

    OString sCharU8 = OUStringToOString(rChars, RTL_TEXTENCODING_UTF8);
    GetImport().GetGenerator().insertText(librevenge::RVNGString(sCharU8.getStr()));

    GetImport().GetGenerator().closeSpan();
}

namespace
{
/// Handler for <text:ruby>.
class XMLRubyContext : public XMLImportContext
{
public:
    XMLRubyContext(XMLImport& rImport, const librevenge::RVNGPropertyList& rPropertyList);

    rtl::Reference<XMLImportContext>
    CreateChildContext(const OUString& rName,
                       const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs) override;

    void SAL_CALL endElement(const OUString& rName) override;

    void SetRubyText(const OUString& rRubyText) { m_sRubyText = rRubyText; }

    OUString& GetRubyBase() { return m_sRubyBase; }

private:
    OUString m_sRubyText;
    OUString m_sRubyBase;
    librevenge::RVNGPropertyList m_aPropertyList;
};

/// Handler for <text:ruby-text>.
class XMLRubyTextContext : public XMLImportContext
{
public:
    XMLRubyTextContext(XMLImport& rImport, XMLRubyContext& rParent)
        : XMLImportContext(rImport)
        , m_rParent(rParent)
    {
    }

    void SAL_CALL characters(const OUString& rChars) override { m_rParent.SetRubyText(rChars); }

private:
    XMLRubyContext& m_rParent;
};

/// Handler for <text:ruby-base>.
class XMLRubyBaseContext : public XMLImportContext
{
public:
    XMLRubyBaseContext(XMLImport& rImport, XMLRubyContext& rParent)
        : XMLImportContext(rImport)
        , m_rParent(rParent)
    {
    }

    void SAL_CALL characters(const OUString& rChars) override { m_rParent.GetRubyBase() += rChars; }

private:
    XMLRubyContext& m_rParent;
};
}

XMLRubyContext::XMLRubyContext(XMLImport& rImport,
                               const librevenge::RVNGPropertyList& rPropertyList)
    : XMLImportContext(rImport)
{
    // Inherit properties from parent.
    librevenge::RVNGPropertyList::Iter itProp(rPropertyList);
    for (itProp.rewind(); itProp.next();)
        m_aPropertyList.insert(itProp.key(), itProp()->clone());
}

rtl::Reference<XMLImportContext> XMLRubyContext::CreateChildContext(
    const OUString& rName, const css::uno::Reference<css::xml::sax::XAttributeList>& /*xAttribs*/)
{
    if (rName == "text:ruby-base")
        return new XMLRubyBaseContext(GetImport(), *this);
    if (rName == "text:ruby-text")
        return new XMLRubyTextContext(GetImport(), *this);
    return nullptr;
}

void XMLRubyContext::endElement(const OUString& /*rName*/)
{
    OString sRubyText = OUStringToOString(m_sRubyText, RTL_TEXTENCODING_UTF8);
    OString sRubyBase = OUStringToOString(m_sRubyBase, RTL_TEXTENCODING_UTF8);
    if (sRubyText.getLength())
        m_aPropertyList.insert("text:ruby-text", sRubyText.getStr());
    GetImport().GetGenerator().openSpan(m_aPropertyList);
    GetImport().GetGenerator().insertText(sRubyBase.getStr());
    GetImport().GetGenerator().closeSpan();
}

namespace
{
/// Base class for contexts that represent a single character only.
class XMLCharContext : public XMLImportContext
{
public:
    XMLCharContext(XMLImport& rImport, const librevenge::RVNGPropertyList& rPropertyList);

    const librevenge::RVNGPropertyList& GetPropertyList() const { return m_aPropertyList; }

private:
    librevenge::RVNGPropertyList m_aPropertyList;
};
}

XMLCharContext::XMLCharContext(XMLImport& rImport,
                               const librevenge::RVNGPropertyList& rPropertyList)
    : XMLImportContext(rImport)
{
    // Inherit properties from parent.
    librevenge::RVNGPropertyList::Iter itProp(rPropertyList);
    for (itProp.rewind(); itProp.next();)
        m_aPropertyList.insert(itProp.key(), itProp()->clone());
}

namespace
{
/// Handler for <text:line-break>.
class XMLLineBreakContext : public XMLCharContext
{
public:
    XMLLineBreakContext(XMLImport& rImport, const librevenge::RVNGPropertyList& rPropertyList);

    void SAL_CALL
    startElement(const OUString& rName,
                 const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs) override;
};
}

XMLLineBreakContext::XMLLineBreakContext(XMLImport& rImport,
                                         const librevenge::RVNGPropertyList& rPropertyList)
    : XMLCharContext(rImport, rPropertyList)
{
}

void XMLLineBreakContext::startElement(
    const OUString& /*rName*/,
    const css::uno::Reference<css::xml::sax::XAttributeList>& /*xAttribs*/)
{
    GetImport().GetGenerator().openSpan(GetPropertyList());
    GetImport().GetGenerator().insertLineBreak();
    GetImport().GetGenerator().closeSpan();
}

namespace
{
/// Handler for <text:s>.
class XMLSpaceContext : public XMLCharContext
{
public:
    XMLSpaceContext(XMLImport& rImport, const librevenge::RVNGPropertyList& rPropertyList);

    void SAL_CALL
    startElement(const OUString& rName,
                 const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs) override;
};
}

XMLSpaceContext::XMLSpaceContext(XMLImport& rImport,
                                 const librevenge::RVNGPropertyList& rPropertyList)
    : XMLCharContext(rImport, rPropertyList)
{
}

void XMLSpaceContext::startElement(
    const OUString& /*rName*/,
    const css::uno::Reference<css::xml::sax::XAttributeList>& /*xAttribs*/)
{
    GetImport().GetGenerator().openSpan(GetPropertyList());
    GetImport().GetGenerator().insertSpace();
    GetImport().GetGenerator().closeSpan();
}

namespace
{
/// Handler for <text:tab>.
class XMLTabContext : public XMLCharContext
{
public:
    XMLTabContext(XMLImport& rImport, const librevenge::RVNGPropertyList& rPropertyList);

    void SAL_CALL
    startElement(const OUString& rName,
                 const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs) override;
};
}

XMLTabContext::XMLTabContext(XMLImport& rImport, const librevenge::RVNGPropertyList& rPropertyList)
    : XMLCharContext(rImport, rPropertyList)
{
}

void XMLTabContext::startElement(
    const OUString& /*rName*/,
    const css::uno::Reference<css::xml::sax::XAttributeList>& /*xAttribs*/)
{
    GetImport().GetGenerator().openSpan(GetPropertyList());
    GetImport().GetGenerator().insertTab();
    GetImport().GetGenerator().closeSpan();
}

namespace
{
/// Handler for <draw:a>.
class XMLTextFrameHyperlinkContext : public XMLImportContext
{
public:
    XMLTextFrameHyperlinkContext(XMLImport& rImport,
                                 const librevenge::RVNGPropertyList& rPropertyList);
    rtl::Reference<XMLImportContext>
    CreateChildContext(const OUString& rName,
                       const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs) override;

    void SAL_CALL
    startElement(const OUString& rName,
                 const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs) override;
    void SAL_CALL endElement(const OUString& rName) override;
    void SAL_CALL characters(const OUString& rChars) override;

private:
    librevenge::RVNGPropertyList m_aPropertyList;
    PopupState m_ePopupState = PopupState::NONE;
};
}

XMLTextFrameHyperlinkContext::XMLTextFrameHyperlinkContext(
    XMLImport& rImport, const librevenge::RVNGPropertyList& rPropertyList)
    : XMLImportContext(rImport)
{
    // Inherit properties from parent.
    librevenge::RVNGPropertyList::Iter itProp(rPropertyList);
    for (itProp.rewind(); itProp.next();)
        m_aPropertyList.insert(itProp.key(), itProp()->clone());
}

rtl::Reference<XMLImportContext> XMLTextFrameHyperlinkContext::CreateChildContext(
    const OUString& rName, const css::uno::Reference<css::xml::sax::XAttributeList>& /*xAttribs*/)
{
    return CreateParagraphOrSpanChildContext(GetImport(), rName, m_aPropertyList);
}

void XMLTextFrameHyperlinkContext::startElement(
    const OUString& /*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs)
{
    librevenge::RVNGPropertyList aPropertyList;
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        const OUString& rAttributeName = xAttribs->getNameByIndex(i);
        const OUString& rAttributeValue = xAttribs->getValueByIndex(i);
        if (rAttributeName == "text:style-name")
            // This affects the nested span's properties.
            FillStyles(rAttributeValue, GetImport().GetAutomaticTextStyles(),
                       GetImport().GetTextStyles(), m_aPropertyList);
        else
        {
            if (rAttributeName == "xlink:href")
            {
                m_ePopupState = GetImport().FillPopupData(rAttributeValue, aPropertyList);
                if (m_ePopupState != PopupState::NotConsumed)
                    continue;
            }

            // This affects the link's properties.
            OString sName = OUStringToOString(rAttributeName, RTL_TEXTENCODING_UTF8);
            OString sValue = OUStringToOString(rAttributeValue, RTL_TEXTENCODING_UTF8);
            aPropertyList.insert(sName.getStr(), sValue.getStr());
        }
    }

    if (m_ePopupState != PopupState::Ignore)
        GetImport().GetGenerator().openLink(aPropertyList);
}

void XMLTextFrameHyperlinkContext::endElement(const OUString& /*rName*/)
{
    if (m_ePopupState != PopupState::Ignore)
        GetImport().GetGenerator().closeLink();
}

void XMLTextFrameHyperlinkContext::characters(const OUString& rChars)
{
    GetImport().GetGenerator().openSpan(m_aPropertyList);

    OString sCharU8 = OUStringToOString(rChars, RTL_TEXTENCODING_UTF8);
    GetImport().GetGenerator().insertText(librevenge::RVNGString(sCharU8.getStr()));

    GetImport().GetGenerator().closeSpan();
}

namespace
{
/// Handler for <text:a>.
class XMLHyperlinkContext : public XMLImportContext
{
public:
    XMLHyperlinkContext(XMLImport& rImport, const librevenge::RVNGPropertyList& rPropertyList);
    rtl::Reference<XMLImportContext>
    CreateChildContext(const OUString& rName,
                       const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs) override;

    void SAL_CALL
    startElement(const OUString& rName,
                 const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs) override;
    void SAL_CALL endElement(const OUString& rName) override;
    void SAL_CALL characters(const OUString& rChars) override;

private:
    librevenge::RVNGPropertyList m_aPropertyList;
    PopupState m_ePopupState = PopupState::NONE;
};
}

XMLHyperlinkContext::XMLHyperlinkContext(XMLImport& rImport,
                                         const librevenge::RVNGPropertyList& rPropertyList)
    : XMLImportContext(rImport)
{
    // Inherit properties from parent.
    librevenge::RVNGPropertyList::Iter itProp(rPropertyList);
    for (itProp.rewind(); itProp.next();)
        m_aPropertyList.insert(itProp.key(), itProp()->clone());
}

rtl::Reference<XMLImportContext> XMLHyperlinkContext::CreateChildContext(
    const OUString& rName, const css::uno::Reference<css::xml::sax::XAttributeList>& /*xAttribs*/)
{
    return CreateParagraphOrSpanChildContext(GetImport(), rName, m_aPropertyList);
}

void XMLHyperlinkContext::startElement(
    const OUString& /*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs)
{
    librevenge::RVNGPropertyList aPropertyList;
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        const OUString& rAttributeName = xAttribs->getNameByIndex(i);
        const OUString& rAttributeValue = xAttribs->getValueByIndex(i);
        if (rAttributeName == "text:style-name")
            // This affects the nested span's properties.
            FillStyles(rAttributeValue, GetImport().GetAutomaticTextStyles(),
                       GetImport().GetTextStyles(), m_aPropertyList);
        else
        {
            if (rAttributeName == "xlink:href")
            {
                m_ePopupState = GetImport().FillPopupData(rAttributeValue, aPropertyList);
                if (m_ePopupState != PopupState::NotConsumed)
                    continue;
            }

            // This affects the link's properties.
            OString sName = OUStringToOString(rAttributeName, RTL_TEXTENCODING_UTF8);
            OString sValue = OUStringToOString(rAttributeValue, RTL_TEXTENCODING_UTF8);
            aPropertyList.insert(sName.getStr(), sValue.getStr());
        }
    }

    if (m_ePopupState != PopupState::Ignore)
        GetImport().GetGenerator().openLink(aPropertyList);
}

void XMLHyperlinkContext::endElement(const OUString& /*rName*/)
{
    if (m_ePopupState != PopupState::Ignore)
        GetImport().GetGenerator().closeLink();
}

void XMLHyperlinkContext::characters(const OUString& rChars)
{
    GetImport().GetGenerator().openSpan(m_aPropertyList);

    OString sCharU8 = OUStringToOString(rChars, RTL_TEXTENCODING_UTF8);
    GetImport().GetGenerator().insertText(librevenge::RVNGString(sCharU8.getStr()));

    GetImport().GetGenerator().closeSpan();
}

XMLParaContext::XMLParaContext(XMLImport& rImport, bool bTopLevel)
    : XMLImportContext(rImport)
    , m_bTopLevel(bTopLevel)
{
}

rtl::Reference<XMLImportContext> XMLParaContext::CreateChildContext(
    const OUString& rName, const css::uno::Reference<css::xml::sax::XAttributeList>& /*xAttribs*/)
{
    if (rName == "text:a")
        return new XMLHyperlinkContext(GetImport(), m_aTextPropertyList);
    if (rName == "draw:a")
        return new XMLTextFrameHyperlinkContext(GetImport(), m_aTextPropertyList);
    if (rName == "text:ruby")
        return new XMLRubyContext(GetImport(), m_aTextPropertyList);
    return CreateParagraphOrSpanChildContext(GetImport(), rName, m_aTextPropertyList);
}

void XMLParaContext::startElement(
    const OUString& /*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs)
{
    librevenge::RVNGPropertyList aPropertyList;
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        const OUString& rAttributeName = xAttribs->getNameByIndex(i);
        const OUString& rAttributeValue = xAttribs->getValueByIndex(i);
        if (rAttributeName == "text:style-name")
        {
            m_aStyleName = rAttributeValue;
            FillStyles(m_aStyleName, GetImport().GetAutomaticParagraphStyles(),
                       GetImport().GetParagraphStyles(), aPropertyList);
            FillStyles(m_aStyleName, GetImport().GetAutomaticTextStyles(),
                       GetImport().GetTextStyles(), m_aTextPropertyList);
            if (m_bTopLevel)
                GetImport().HandlePageSpan(aPropertyList);
        }
        else
        {
            OString sName = OUStringToOString(rAttributeName, RTL_TEXTENCODING_UTF8);
            OString sValue = OUStringToOString(rAttributeValue, RTL_TEXTENCODING_UTF8);
            aPropertyList.insert(sName.getStr(), sValue.getStr());
        }
    }

    GetImport().GetGenerator().openParagraph(aPropertyList);
}

void XMLParaContext::endElement(const OUString& /*rName*/)
{
    GetImport().GetGenerator().closeParagraph();
}

void XMLParaContext::characters(const OUString& rChars)
{
    librevenge::RVNGPropertyList aPropertyList;
    if (!m_aStyleName.isEmpty())
        FillStyles(m_aStyleName, GetImport().GetAutomaticTextStyles(), GetImport().GetTextStyles(),
                   aPropertyList);
    GetImport().GetGenerator().openSpan(aPropertyList);

    OString sCharU8 = OUStringToOString(rChars, RTL_TEXTENCODING_UTF8);
    GetImport().GetGenerator().insertText(librevenge::RVNGString(sCharU8.getStr()));

    GetImport().GetGenerator().closeSpan();
}

rtl::Reference<XMLImportContext>
CreateParagraphOrSpanChildContext(XMLImport& rImport, const OUString& rName,
                                  const librevenge::RVNGPropertyList& rTextPropertyList)
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

void FillStyles(const OUString& rName,
                std::map<OUString, librevenge::RVNGPropertyList>& rAutomaticStyles,
                std::map<OUString, librevenge::RVNGPropertyList>& rNamedStyles,
                librevenge::RVNGPropertyList& rPropertyList)
{
    FillStyle(rName, rAutomaticStyles, rAutomaticStyles, rNamedStyles, rPropertyList);
    FillStyle(rName, rNamedStyles, rAutomaticStyles, rNamedStyles, rPropertyList);
}

} // namespace exp
} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
