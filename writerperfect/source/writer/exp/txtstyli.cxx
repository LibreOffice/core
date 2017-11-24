/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "txtstyli.hxx"

#include "xmlfmt.hxx"
#include "xmlimp.hxx"

using namespace com::sun::star;

namespace writerperfect
{
namespace exp
{

/// Handler for <style:paragraph-properties>.
class XMLParagraphPropertiesContext : public XMLImportContext
{
public:
    XMLParagraphPropertiesContext(XMLImport &rImport, XMLStyleContext &rStyle);

    void SAL_CALL startElement(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;

private:
    XMLStyleContext &mrStyle;
};

XMLParagraphPropertiesContext::XMLParagraphPropertiesContext(XMLImport &rImport, XMLStyleContext &rStyle)
    : XMLImportContext(rImport)
    , mrStyle(rStyle)
{
}

void XMLParagraphPropertiesContext::startElement(const OUString &/*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs)
{
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        OString sName = OUStringToOString(xAttribs->getNameByIndex(i), RTL_TEXTENCODING_UTF8);
        OString sValue = OUStringToOString(xAttribs->getValueByIndex(i), RTL_TEXTENCODING_UTF8);
        mrStyle.GetParagraphPropertyList().insert(sName.getStr(), sValue.getStr());
    }
}

/// Handler for <style:text-properties>.
class XMLTextPropertiesContext : public XMLImportContext
{
public:
    XMLTextPropertiesContext(XMLImport &rImport, XMLStyleContext &rStyle);

    void SAL_CALL startElement(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;

private:
    XMLStyleContext &mrStyle;
};

XMLTextPropertiesContext::XMLTextPropertiesContext(XMLImport &rImport, XMLStyleContext &rStyle)
    : XMLImportContext(rImport)
    , mrStyle(rStyle)
{
}

void XMLTextPropertiesContext::startElement(const OUString &/*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs)
{
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        OString sName = OUStringToOString(xAttribs->getNameByIndex(i), RTL_TEXTENCODING_UTF8);
        OString sValue = OUStringToOString(xAttribs->getValueByIndex(i), RTL_TEXTENCODING_UTF8);
        mrStyle.GetTextPropertyList().insert(sName.getStr(), sValue.getStr());
    }
}

/// Handler for <style:table-column-properties>.
class XMLTableColumnPropertiesContext : public XMLImportContext
{
public:
    XMLTableColumnPropertiesContext(XMLImport &rImport, XMLStyleContext &rStyle);

    void SAL_CALL startElement(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;

private:
    XMLStyleContext &mrStyle;
};

XMLTableColumnPropertiesContext::XMLTableColumnPropertiesContext(XMLImport &rImport, XMLStyleContext &rStyle)
    : XMLImportContext(rImport)
    , mrStyle(rStyle)
{
}

void XMLTableColumnPropertiesContext::startElement(const OUString &/*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs)
{
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        OString sName = OUStringToOString(xAttribs->getNameByIndex(i), RTL_TEXTENCODING_UTF8);
        OString sValue = OUStringToOString(xAttribs->getValueByIndex(i), RTL_TEXTENCODING_UTF8);
        mrStyle.GetColumnPropertyList().insert(sName.getStr(), sValue.getStr());
    }
}

/// Handler for <style:table-cell-properties>.
class XMLTableCellPropertiesContext : public XMLImportContext
{
public:
    XMLTableCellPropertiesContext(XMLImport &rImport, XMLStyleContext &rStyle);

    void SAL_CALL startElement(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;

private:
    XMLStyleContext &mrStyle;
};

XMLTableCellPropertiesContext::XMLTableCellPropertiesContext(XMLImport &rImport, XMLStyleContext &rStyle)
    : XMLImportContext(rImport)
    , mrStyle(rStyle)
{
}

void XMLTableCellPropertiesContext::startElement(const OUString &/*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs)
{
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        OString sName = OUStringToOString(xAttribs->getNameByIndex(i), RTL_TEXTENCODING_UTF8);
        OString sValue = OUStringToOString(xAttribs->getValueByIndex(i), RTL_TEXTENCODING_UTF8);
        mrStyle.GetCellPropertyList().insert(sName.getStr(), sValue.getStr());
    }
}

XMLStyleContext::XMLStyleContext(XMLImport &rImport, XMLStylesContext &rStyles)
    : XMLImportContext(rImport),
      m_rStyles(rStyles)
{
}

rtl::Reference<XMLImportContext> XMLStyleContext::CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    if (rName == "style:paragraph-properties")
        return new XMLParagraphPropertiesContext(mrImport, *this);
    if (rName == "style:text-properties")
        return new XMLTextPropertiesContext(mrImport, *this);
    if (rName == "style:table-cell-properties")
        return new XMLTableCellPropertiesContext(mrImport, *this);
    if (rName == "style:table-column-properties")
        return new XMLTableColumnPropertiesContext(mrImport, *this);
    return nullptr;
}

void XMLStyleContext::startElement(const OUString &/*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs)
{
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        const OUString &rAttributeName = xAttribs->getNameByIndex(i);
        const OUString &rAttributeValue = xAttribs->getValueByIndex(i);
        if (rAttributeName == "style:name")
            m_aName = rAttributeValue;
        else if (rAttributeName == "style:family")
            m_aFamily = rAttributeValue;

        // Remember properties of the style itself, e.g. parent name.
        OString sName = OUStringToOString(rAttributeName, RTL_TEXTENCODING_UTF8);
        OString sValue = OUStringToOString(rAttributeValue, RTL_TEXTENCODING_UTF8);
        m_aTextPropertyList.insert(sName.getStr(), sValue.getStr());
        m_aParagraphPropertyList.insert(sName.getStr(), sValue.getStr());
    }
}

void XMLStyleContext::endElement(const OUString &/*rName*/)
{
    if (m_aName.isEmpty())
        return;

    if (m_aFamily == "text" || m_aFamily == "paragraph")
        m_rStyles.GetCurrentTextStyles()[m_aName] = m_aTextPropertyList;
    if (m_aFamily == "paragraph")
        m_rStyles.GetCurrentParagraphStyles()[m_aName] = m_aParagraphPropertyList;
    if (m_aFamily == "table-cell")
        m_rStyles.GetCurrentCellStyles()[m_aName] = m_aCellPropertyList;
    if (m_aFamily == "table-column")
        m_rStyles.GetCurrentColumnStyles()[m_aName] = m_aColumnPropertyList;
}

librevenge::RVNGPropertyList &XMLStyleContext::GetTextPropertyList()
{
    return m_aTextPropertyList;
}

librevenge::RVNGPropertyList &XMLStyleContext::GetParagraphPropertyList()
{
    return m_aParagraphPropertyList;
}

librevenge::RVNGPropertyList &XMLStyleContext::GetCellPropertyList()
{
    return m_aCellPropertyList;
}

librevenge::RVNGPropertyList &XMLStyleContext::GetColumnPropertyList()
{
    return m_aColumnPropertyList;
}

} // namespace exp
} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
