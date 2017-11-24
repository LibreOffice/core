/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xmltbli.hxx"

#include "txtparai.hxx"
#include "xmlimp.hxx"
#include "xmltext.hxx"

using namespace com::sun::star;

namespace writerperfect
{
namespace exp
{

/// Handler for <table:table-row>.
class XMLTableRowContext : public XMLImportContext
{
public:
    XMLTableRowContext(XMLImport &rImport);

    rtl::Reference<XMLImportContext> CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;
    int GetColumn() const;
    void SetColumn(int nColumn);

    void SAL_CALL startElement(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;
    void SAL_CALL endElement(const OUString &rName) override;

private:
    int m_nColumn = 0;
};

/// Handler for <table:cell>.
class XMLTableCellContext : public XMLImportContext
{
public:
    XMLTableCellContext(XMLImport &rImport, XMLTableRowContext &rRow);

    rtl::Reference<XMLImportContext> CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;

    void SAL_CALL startElement(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;
    void SAL_CALL endElement(const OUString &rName) override;

private:
    XMLTableRowContext &m_rRow;
};

XMLTableCellContext::XMLTableCellContext(XMLImport &rImport, XMLTableRowContext &rRow)
    : XMLImportContext(rImport),
      m_rRow(rRow)
{
}

rtl::Reference<XMLImportContext> XMLTableCellContext::CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    return CreateTextChildContext(mrImport, rName);
}

void XMLTableCellContext::startElement(const OUString &/*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs)
{
    librevenge::RVNGPropertyList aPropertyList;
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        const OUString &rAttributeName = xAttribs->getNameByIndex(i);
        const OUString &rAttributeValue = xAttribs->getValueByIndex(i);

        if (rAttributeName == "table:style-name")
            FillStyles(rAttributeValue, mrImport.GetAutomaticCellStyles(), mrImport.GetCellStyles(), aPropertyList);
        else
        {
            OString sName = OUStringToOString(rAttributeName, RTL_TEXTENCODING_UTF8);
            OString sValue = OUStringToOString(rAttributeValue, RTL_TEXTENCODING_UTF8);
            aPropertyList.insert(sName.getStr(), sValue.getStr());
        }
    }
    aPropertyList.insert("librevenge:column", m_rRow.GetColumn());
    mrImport.GetGenerator().openTableCell(aPropertyList);
    m_rRow.SetColumn(m_rRow.GetColumn() + 1);
}

void XMLTableCellContext::endElement(const OUString &/*rName*/)
{
    mrImport.GetGenerator().closeTableCell();
}

/// Handler for <table:table-column>.
class XMLTableColumnContext : public XMLImportContext
{
public:
    XMLTableColumnContext(XMLImport &rImport, librevenge::RVNGPropertyListVector &rColumns);

    void SAL_CALL startElement(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;

private:
    librevenge::RVNGPropertyListVector &m_rColumns;
};

XMLTableColumnContext::XMLTableColumnContext(XMLImport &rImport, librevenge::RVNGPropertyListVector &rColumns)
    : XMLImportContext(rImport),
      m_rColumns(rColumns)
{
}

void XMLTableColumnContext::startElement(const OUString &/*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs)
{
    librevenge::RVNGPropertyList aPropertyList;
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        const OUString &rAttributeName = xAttribs->getNameByIndex(i);
        const OUString &rAttributeValue = xAttribs->getValueByIndex(i);

        if (rAttributeName == "table:style-name")
            FillStyles(rAttributeValue, mrImport.GetAutomaticColumnStyles(), mrImport.GetColumnStyles(), aPropertyList);
    }
    m_rColumns.append(aPropertyList);
}

XMLTableRowContext::XMLTableRowContext(XMLImport &rImport)
    : XMLImportContext(rImport)
{
}

rtl::Reference<XMLImportContext> XMLTableRowContext::CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    if (rName == "table:table-cell")
        return new XMLTableCellContext(mrImport, *this);
    if (rName == "table:covered-table-cell")
    {
        ++m_nColumn;
        mrImport.GetGenerator().insertCoveredTableCell(librevenge::RVNGPropertyList());
    }
    else
        SAL_WARN("writerperfect", "XMLTableRowContext::CreateChildContext: unhandled " << rName);
    return nullptr;
}

void XMLTableRowContext::startElement(const OUString &/*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs)
{
    librevenge::RVNGPropertyList aPropertyList;
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        const OUString &rAttributeName = xAttribs->getNameByIndex(i);
        const OUString &rAttributeValue = xAttribs->getValueByIndex(i);

        if (rAttributeName == "table:style-name")
            FillStyles(rAttributeValue, mrImport.GetAutomaticRowStyles(), mrImport.GetRowStyles(), aPropertyList);
    }
    mrImport.GetGenerator().openTableRow(aPropertyList);
}

void XMLTableRowContext::endElement(const OUString &/*rName*/)
{
    mrImport.GetGenerator().closeTableRow();
}

int XMLTableRowContext::GetColumn() const
{
    return m_nColumn;
}

void XMLTableRowContext::SetColumn(int nColumn)
{
    m_nColumn = nColumn;
}

XMLTableContext::XMLTableContext(XMLImport &rImport)
    : XMLImportContext(rImport)
{
}

rtl::Reference<XMLImportContext> XMLTableContext::CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    if (rName == "table:table-column")
        // Make sure columns are parsed before we open the table.
        return new XMLTableColumnContext(mrImport, m_aColumns);

    if (!m_bTableOpened)
    {
        if (!m_aColumns.empty())
            m_aPropertyList.insert("librevenge:table-columns", m_aColumns);
        mrImport.GetGenerator().openTable(m_aPropertyList);
        m_bTableOpened = true;
    }

    if (rName == "table:table-row")
        return new XMLTableRowContext(mrImport);

    SAL_WARN("writerperfect", "XMLTableContext::CreateChildContext: unhandled " << rName);

    return nullptr;
}

void XMLTableContext::startElement(const OUString &/*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs)
{
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        const OUString &rAttributeName = xAttribs->getNameByIndex(i);
        const OUString &rAttributeValue = xAttribs->getValueByIndex(i);

        if (rAttributeName == "table:style-name")
            FillStyles(rAttributeValue, mrImport.GetAutomaticTableStyles(), mrImport.GetTableStyles(), m_aPropertyList);
        else
        {
            OString sName = OUStringToOString(rAttributeName, RTL_TEXTENCODING_UTF8);
            OString sValue = OUStringToOString(rAttributeValue, RTL_TEXTENCODING_UTF8);
            m_aPropertyList.insert(sName.getStr(), sValue.getStr());
        }
    }
}

void XMLTableContext::endElement(const OUString &/*rName*/)
{
    mrImport.GetGenerator().closeTable();
}

} // namespace exp
} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
