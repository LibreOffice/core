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

using namespace com::sun::star;

namespace writerperfect
{
namespace exp
{

/// Handler for <table:cell>.
class XMLTableCellContext : public XMLImportContext
{
public:
    XMLTableCellContext(XMLImport &rImport);

    XMLImportContext *CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;

    void SAL_CALL startElement(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;
    void SAL_CALL endElement(const OUString &rName) override;
};

XMLTableCellContext::XMLTableCellContext(XMLImport &rImport)
    : XMLImportContext(rImport)
{
}

XMLImportContext *XMLTableCellContext::CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    if (rName == "text:p" || rName == "text:h")
        return new XMLParaContext(mrImport);
    if (rName == "table:table")
        return new XMLTableContext(mrImport);
    return nullptr;
}

void XMLTableCellContext::startElement(const OUString &/*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    mrImport.GetGenerator().openTableCell(librevenge::RVNGPropertyList());
}

void XMLTableCellContext::endElement(const OUString &/*rName*/)
{
    mrImport.GetGenerator().closeTableCell();
}

/// Handler for <table:row>.
class XMLTableRowContext : public XMLImportContext
{
public:
    XMLTableRowContext(XMLImport &rImport);

    XMLImportContext *CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;

    void SAL_CALL startElement(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;
    void SAL_CALL endElement(const OUString &rName) override;
};

XMLTableRowContext::XMLTableRowContext(XMLImport &rImport)
    : XMLImportContext(rImport)
{
}

XMLImportContext *XMLTableRowContext::CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    if (rName == "table:table-cell")
        return new XMLTableCellContext(mrImport);
    return nullptr;
}

void XMLTableRowContext::startElement(const OUString &/*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    mrImport.GetGenerator().openTableRow(librevenge::RVNGPropertyList());
}

void XMLTableRowContext::endElement(const OUString &/*rName*/)
{
    mrImport.GetGenerator().closeTableRow();
}

XMLTableContext::XMLTableContext(XMLImport &rImport)
    : XMLImportContext(rImport)
{
}

XMLImportContext *XMLTableContext::CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    if (rName == "table:table-row")
        return new XMLTableRowContext(mrImport);
    return nullptr;
}

void XMLTableContext::startElement(const OUString &/*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    mrImport.GetGenerator().openTable(librevenge::RVNGPropertyList());
}

void XMLTableContext::endElement(const OUString &/*rName*/)
{
    mrImport.GetGenerator().closeTable();
}

} // namespace exp
} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
