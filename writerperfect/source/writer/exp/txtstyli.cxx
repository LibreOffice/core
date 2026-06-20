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

using namespace com::sun::star;

namespace writerperfect::exp
{
namespace
{
/// Handler for <style:paragraph-properties>.
class XMLParagraphPropertiesContext : public XMLImportContext
{
public:
    XMLParagraphPropertiesContext(XMLImport& rImport, XMLStyleContext& rStyle);

    void SAL_CALL
    startElement(const OUString& rName,
                 const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs) override;

private:
    XMLStyleContext& mrStyle;
};
}

XMLParagraphPropertiesContext::XMLParagraphPropertiesContext(XMLImport& rImport,
                                                             XMLStyleContext& rStyle)
    : XMLImportContext(rImport)
    , mrStyle(rStyle)
{
}

void XMLParagraphPropertiesContext::startElement(
    const OUString& /*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs)
{
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        OString sName = OUStringToOString(xAttribs->getNameByIndex(i), RTL_TEXTENCODING_UTF8);
        OString sValue = OUStringToOString(xAttribs->getValueByIndex(i), RTL_TEXTENCODING_UTF8);
        mrStyle.GetParagraphPropertyList().insert(sName.getStr(), sValue.getStr());
    }
}

namespace
{
/// Handler for <style:text-properties>.
class XMLTextPropertiesContext : public XMLImportContext
{
public:
    XMLTextPropertiesContext(XMLImport& rImport, XMLStyleContext& rStyle);

    void SAL_CALL
    startElement(const OUString& rName,
                 const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs) override;

private:
    XMLStyleContext& mrStyle;
};
}

XMLTextPropertiesContext::XMLTextPropertiesContext(XMLImport& rImport, XMLStyleContext& rStyle)
    : XMLImportContext(rImport)
    , mrStyle(rStyle)
{
}

void XMLTextPropertiesContext::startElement(
    const OUString& /*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs)
{
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        OString sName = OUStringToOString(xAttribs->getNameByIndex(i), RTL_TEXTENCODING_UTF8);
        OString sValue = OUStringToOString(xAttribs->getValueByIndex(i), RTL_TEXTENCODING_UTF8);
        mrStyle.GetTextPropertyList().insert(sName.getStr(), sValue.getStr());
    }
}

namespace
{
/// Handler for <style:graphic-properties>.
class XMLGraphicPropertiesContext : public XMLImportContext
{
public:
    XMLGraphicPropertiesContext(XMLImport& rImport, XMLStyleContext& rStyle);

    void SAL_CALL
    startElement(const OUString& rName,
                 const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs) override;

private:
    XMLStyleContext& mrStyle;
};
}

XMLGraphicPropertiesContext::XMLGraphicPropertiesContext(XMLImport& rImport,
                                                         XMLStyleContext& rStyle)
    : XMLImportContext(rImport)
    , mrStyle(rStyle)
{
}

void XMLGraphicPropertiesContext::startElement(
    const OUString& /*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs)
{
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        OString sName = OUStringToOString(xAttribs->getNameByIndex(i), RTL_TEXTENCODING_UTF8);
        OString sValue = OUStringToOString(xAttribs->getValueByIndex(i), RTL_TEXTENCODING_UTF8);
        mrStyle.GetGraphicPropertyList().insert(sName.getStr(), sValue.getStr());
    }
}

namespace
{
/// Handler for <text:list-level-style-number> and <text:text:list-level-style-bullet>.
class XMLListPropertiesContext : public XMLImportContext
{
public:
    XMLListPropertiesContext(XMLImport& rImport, XMLStyleContext& rStyle,
                             const OUString& rStyleName);

    void SAL_CALL
    startElement(const OUString& rName,
                 const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs) override;

private:
    XMLStyleContext& mrStyle;
    const OUString& mrStyleName;
};
}

XMLListPropertiesContext::XMLListPropertiesContext(XMLImport& rImport, XMLStyleContext& rStyle,
                                                   const OUString& rStyleName)
    : XMLImportContext(rImport)
    , mrStyle(rStyle)
    , mrStyleName(rStyleName)
{
}

void XMLListPropertiesContext::startElement(
    const OUString& /*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs)
{
    OString aStyleName = OUStringToOString(mrStyleName, RTL_TEXTENCODING_UTF8);

    // a list style is defined with:
    // 1 "text:list-style" tag which only contains the attribute "style:name"
    // several (most of times 10) "text:list-level-style-number" or "text:list-level-style-bullet" subtags
    // remark: these subtags contains other elements but we haven't dealt with them
    // anyway, that's why we need a RVNGPropertyListVector, this one will contains subtags
    // all attributes of the a subtag will be in a RVNGPropertyList

    // declare the vector for the current style
    librevenge::RVNGPropertyListVector aStyleCurrentVector;

    // check if there's an existing one and if it's the case, use it
    const librevenge::RVNGProperty* pStyleProperty
        = mrStyle.GetListPropertyList().child(aStyleName.getStr());
    if (pStyleProperty)
    {
        const librevenge::RVNGPropertyListVector* pStyleVector
            = static_cast<const librevenge::RVNGPropertyListVector*>(pStyleProperty);
        aStyleCurrentVector = *pStyleVector;
    }

    // create a RVNGPropertyList for the subtag of a style
    librevenge::RVNGPropertyList aCurrentPropertyList;
    // save all attributes of the subtag
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        OString sName = OUStringToOString(xAttribs->getNameByIndex(i), RTL_TEXTENCODING_UTF8);
        OString sValue = OUStringToOString(xAttribs->getValueByIndex(i), RTL_TEXTENCODING_UTF8);
        aCurrentPropertyList.insert(sName.getStr(), sValue.getStr());
    }
    // now we can add the subtag in the vector
    aStyleCurrentVector.append(aCurrentPropertyList);

    // and insert/reinsert it in mrStyle.GetListPropertyList
    mrStyle.GetListPropertyList().insert(aStyleName.getStr(), aStyleCurrentVector);
}

namespace
{
/// Handler for <style:page-layout-properties>.
class XMLPageLayoutPropertiesContext : public XMLImportContext
{
public:
    XMLPageLayoutPropertiesContext(XMLImport& rImport, XMLStyleContext& rStyle);

    void SAL_CALL
    startElement(const OUString& rName,
                 const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs) override;

private:
    XMLStyleContext& mrStyle;
};
}

XMLPageLayoutPropertiesContext::XMLPageLayoutPropertiesContext(XMLImport& rImport,
                                                               XMLStyleContext& rStyle)
    : XMLImportContext(rImport)
    , mrStyle(rStyle)
{
}

void XMLPageLayoutPropertiesContext::startElement(
    const OUString& /*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs)
{
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        OString sName = OUStringToOString(xAttribs->getNameByIndex(i), RTL_TEXTENCODING_UTF8);
        OString sValue = OUStringToOString(xAttribs->getValueByIndex(i), RTL_TEXTENCODING_UTF8);
        // We only care about writing-mode for now.
        if (sName != "style:writing-mode")
            continue;

        mrStyle.GetPageLayoutPropertyList().insert(sName.getStr(), sValue.getStr());
    }
}

namespace
{
/// Handler for <style:table-properties>.
class XMLTablePropertiesContext : public XMLImportContext
{
public:
    XMLTablePropertiesContext(XMLImport& rImport, XMLStyleContext& rStyle);

    void SAL_CALL
    startElement(const OUString& rName,
                 const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs) override;

private:
    XMLStyleContext& mrStyle;
};
}

XMLTablePropertiesContext::XMLTablePropertiesContext(XMLImport& rImport, XMLStyleContext& rStyle)
    : XMLImportContext(rImport)
    , mrStyle(rStyle)
{
}

void XMLTablePropertiesContext::startElement(
    const OUString& /*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs)
{
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        OString sName = OUStringToOString(xAttribs->getNameByIndex(i), RTL_TEXTENCODING_UTF8);
        OString sValue = OUStringToOString(xAttribs->getValueByIndex(i), RTL_TEXTENCODING_UTF8);
        if (sName == "style:rel-width")
            // Make sure this is passed through as a string, and not parsed as a double.
            mrStyle.GetTablePropertyList().insert(
                sName.getStr(), librevenge::RVNGPropertyFactory::newStringProp(sValue.getStr()));
        else
            mrStyle.GetTablePropertyList().insert(sName.getStr(), sValue.getStr());
    }
}

namespace
{
/// Handler for <style:table-row-properties>.
class XMLTableRowPropertiesContext : public XMLImportContext
{
public:
    XMLTableRowPropertiesContext(XMLImport& rImport, XMLStyleContext& rStyle);

    void SAL_CALL
    startElement(const OUString& rName,
                 const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs) override;

private:
    XMLStyleContext& mrStyle;
};
}

XMLTableRowPropertiesContext::XMLTableRowPropertiesContext(XMLImport& rImport,
                                                           XMLStyleContext& rStyle)
    : XMLImportContext(rImport)
    , mrStyle(rStyle)
{
}

void XMLTableRowPropertiesContext::startElement(
    const OUString& /*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs)
{
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        OString sName = OUStringToOString(xAttribs->getNameByIndex(i), RTL_TEXTENCODING_UTF8);
        OString sValue = OUStringToOString(xAttribs->getValueByIndex(i), RTL_TEXTENCODING_UTF8);
        mrStyle.GetRowPropertyList().insert(sName.getStr(), sValue.getStr());
    }
}

namespace
{
/// Handler for <style:table-column-properties>.
class XMLTableColumnPropertiesContext : public XMLImportContext
{
public:
    XMLTableColumnPropertiesContext(XMLImport& rImport, XMLStyleContext& rStyle);

    void SAL_CALL
    startElement(const OUString& rName,
                 const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs) override;

private:
    XMLStyleContext& mrStyle;
};
}

XMLTableColumnPropertiesContext::XMLTableColumnPropertiesContext(XMLImport& rImport,
                                                                 XMLStyleContext& rStyle)
    : XMLImportContext(rImport)
    , mrStyle(rStyle)
{
}

void XMLTableColumnPropertiesContext::startElement(
    const OUString& /*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs)
{
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        OString sName = OUStringToOString(xAttribs->getNameByIndex(i), RTL_TEXTENCODING_UTF8);
        OString sValue = OUStringToOString(xAttribs->getValueByIndex(i), RTL_TEXTENCODING_UTF8);
        mrStyle.GetColumnPropertyList().insert(sName.getStr(), sValue.getStr());
    }
}

namespace
{
/// Handler for <style:table-cell-properties>.
class XMLTableCellPropertiesContext : public XMLImportContext
{
public:
    XMLTableCellPropertiesContext(XMLImport& rImport, XMLStyleContext& rStyle);

    void SAL_CALL
    startElement(const OUString& rName,
                 const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs) override;

private:
    XMLStyleContext& mrStyle;
};
}

XMLTableCellPropertiesContext::XMLTableCellPropertiesContext(XMLImport& rImport,
                                                             XMLStyleContext& rStyle)
    : XMLImportContext(rImport)
    , mrStyle(rStyle)
{
}

void XMLTableCellPropertiesContext::startElement(
    const OUString& /*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs)
{
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        OString sName = OUStringToOString(xAttribs->getNameByIndex(i), RTL_TEXTENCODING_UTF8);
        OString sValue = OUStringToOString(xAttribs->getValueByIndex(i), RTL_TEXTENCODING_UTF8);
        mrStyle.GetCellPropertyList().insert(sName.getStr(), sValue.getStr());
    }
}

XMLStyleContext::XMLStyleContext(XMLImport& rImport, XMLStylesContext& rStyles)
    : XMLImportContext(rImport)
    , m_rStyles(rStyles)
{
}

rtl::Reference<XMLImportContext> XMLStyleContext::CreateChildContext(
    const OUString& rName, const css::uno::Reference<css::xml::sax::XAttributeList>& /*xAttribs*/)
{
    if (rName == "style:paragraph-properties")
        return new XMLParagraphPropertiesContext(GetImport(), *this);
    if (rName == "style:text-properties")
        return new XMLTextPropertiesContext(GetImport(), *this);
    if (rName == "style:table-cell-properties")
        return new XMLTableCellPropertiesContext(GetImport(), *this);
    if (rName == "style:table-column-properties")
        return new XMLTableColumnPropertiesContext(GetImport(), *this);
    if (rName == "style:table-row-properties")
        return new XMLTableRowPropertiesContext(GetImport(), *this);
    if (rName == "style:table-properties")
        return new XMLTablePropertiesContext(GetImport(), *this);
    if (rName == "style:graphic-properties")
        return new XMLGraphicPropertiesContext(GetImport(), *this);
    if (rName == "text:list-level-style-number" || rName == "text:list-level-style-bullet")
        return new XMLListPropertiesContext(GetImport(), *this, m_aName);
    if (rName == "style:page-layout-properties")
        return new XMLPageLayoutPropertiesContext(GetImport(), *this);
    return nullptr;
}

void XMLStyleContext::startElement(
    const OUString& /*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs)
{
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        const OUString aAttributeName = xAttribs->getNameByIndex(i);
        const OUString aAttributeValue = xAttribs->getValueByIndex(i);
        if (aAttributeName == "style:name")
            m_aName = aAttributeValue;
        else if (aAttributeName == "style:family")
            m_aFamily = aAttributeValue;

        // Remember properties of the style itself, e.g. parent name.
        OString sName = OUStringToOString(aAttributeName, RTL_TEXTENCODING_UTF8);
        OString sValue = OUStringToOString(aAttributeValue, RTL_TEXTENCODING_UTF8);
        m_aTextPropertyList.insert(sName.getStr(), sValue.getStr());
        m_aParagraphPropertyList.insert(sName.getStr(), sValue.getStr());
        m_aGraphicPropertyList.insert(sName.getStr(), sValue.getStr());
        m_aListPropertyList.insert(sName.getStr(), sValue.getStr());
        m_aPageLayoutPropertyList.insert(sName.getStr(), sValue.getStr());
        m_aMasterPagePropertyList.insert(sName.getStr(), sValue.getStr());
        m_aTablePropertyList.insert(sName.getStr(), sValue.getStr());
    }
}

void XMLStyleContext::endElement(const OUString& rName)
{
    if (m_aName.isEmpty())
        return;

    if (m_aFamily == "text" || m_aFamily == "paragraph")
        m_rStyles.GetCurrentTextStyles()[m_aName] = m_aTextPropertyList;
    if (m_aFamily == "paragraph")
        m_rStyles.GetCurrentParagraphStyles()[m_aName] = m_aParagraphPropertyList;
    else if (m_aFamily == "table-cell")
        m_rStyles.GetCurrentCellStyles()[m_aName] = m_aCellPropertyList;
    else if (m_aFamily == "table-column")
        m_rStyles.GetCurrentColumnStyles()[m_aName] = m_aColumnPropertyList;
    else if (m_aFamily == "table-row")
        m_rStyles.GetCurrentRowStyles()[m_aName] = m_aRowPropertyList;
    else if (m_aFamily == "table")
        m_rStyles.GetCurrentTableStyles()[m_aName] = m_aTablePropertyList;
    else if (m_aFamily == "graphic")
        m_rStyles.GetCurrentGraphicStyles()[m_aName] = m_aGraphicPropertyList;
    else if (rName == "text:list-style")
        m_rStyles.GetCurrentListStyles()[m_aName] = m_aListPropertyList;
    else if (rName == "style:page-layout")
        m_rStyles.GetCurrentPageLayouts()[m_aName] = m_aPageLayoutPropertyList;
    else if (rName == "style:master-page")
        m_rStyles.GetCurrentMasterStyles()[m_aName] = m_aMasterPagePropertyList;
}

librevenge::RVNGPropertyList& XMLStyleContext::GetTextPropertyList() { return m_aTextPropertyList; }

librevenge::RVNGPropertyList& XMLStyleContext::GetParagraphPropertyList()
{
    return m_aParagraphPropertyList;
}

librevenge::RVNGPropertyList& XMLStyleContext::GetCellPropertyList() { return m_aCellPropertyList; }

librevenge::RVNGPropertyList& XMLStyleContext::GetColumnPropertyList()
{
    return m_aColumnPropertyList;
}

librevenge::RVNGPropertyList& XMLStyleContext::GetRowPropertyList() { return m_aRowPropertyList; }

librevenge::RVNGPropertyList& XMLStyleContext::GetTablePropertyList()
{
    return m_aTablePropertyList;
}

librevenge::RVNGPropertyList& XMLStyleContext::GetGraphicPropertyList()
{
    return m_aGraphicPropertyList;
}

librevenge::RVNGPropertyList& XMLStyleContext::GetListPropertyList() { return m_aListPropertyList; }

librevenge::RVNGPropertyList& XMLStyleContext::GetPageLayoutPropertyList()
{
    return m_aPageLayoutPropertyList;
}

} // namespace writerperfect::exp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
