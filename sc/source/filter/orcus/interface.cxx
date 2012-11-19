/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcusinterface.hxx"

#include "document.hxx"

using orcus::spreadsheet::row_t;
using orcus::spreadsheet::col_t;
using orcus::spreadsheet::formula_grammar_t;

ScOrcusFactory::ScOrcusFactory(ScDocument& rDoc) : mrDoc(rDoc) {}

orcus::spreadsheet::iface::import_sheet* ScOrcusFactory::append_sheet(const char* sheet_name, size_t sheet_name_length)
{
    OUString aTabName(sheet_name, sheet_name_length, RTL_TEXTENCODING_UTF8);
    if (!mrDoc.InsertTab(SC_TAB_APPEND, aTabName))
        return NULL;

    SCTAB nTab = mrDoc.GetTableCount() - 1;
    maSheets.push_back(new ScOrcusSheet(mrDoc, nTab));
    return &maSheets.back();
}

class FindSheetByIndex : std::unary_function<ScOrcusSheet, bool>
{
    SCTAB mnTab;
public:
    FindSheetByIndex(SCTAB nTab) : mnTab(nTab) {}
    bool operator() (const ScOrcusSheet& rSheet) const
    {
        return rSheet.getIndex() == mnTab;
    }
};

orcus::spreadsheet::iface::import_sheet* ScOrcusFactory::get_sheet(const char* sheet_name, size_t sheet_name_length)
{
    OUString aTabName(sheet_name, sheet_name_length, RTL_TEXTENCODING_UTF8);
    SCTAB nTab = -1;
    if (!mrDoc.GetTable(aTabName, nTab))
        // Sheet by that name not found.
        return NULL;

    // See if we already have an orcus sheet instance by that index.
    boost::ptr_vector<ScOrcusSheet>::iterator it =
        std::find_if(maSheets.begin(), maSheets.end(), FindSheetByIndex(nTab));

    if (it != maSheets.end())
        // We already have one. Return it.
        return &(*it);

    // Create a new orcus sheet instance for this.
    maSheets.push_back(new ScOrcusSheet(mrDoc, nTab));
    return &maSheets.back();
}

orcus::spreadsheet::iface::import_shared_strings* ScOrcusFactory::get_shared_strings()
{
    // We don't support it yet.
    return NULL;
}

orcus::spreadsheet::iface::import_styles* ScOrcusFactory::get_styles()
{
    // We don't support it yet.
    return NULL;
}

ScOrcusSheet::ScOrcusSheet(ScDocument& rDoc, SCTAB nTab) :
    mrDoc(rDoc), mnTab(nTab) {}

void ScOrcusSheet::set_auto(row_t row, col_t col, const char* p, size_t n)
{
    OUString aVal(p, n, RTL_TEXTENCODING_UTF8);
    mrDoc.SetString(col, row, mnTab, aVal);
}

void ScOrcusSheet::set_format(row_t /*row*/, col_t /*col*/, size_t /*xf_index*/)
{
}

void ScOrcusSheet::set_formula(
    row_t /*row*/, col_t /*col*/, formula_grammar_t /*grammar*/, const char* /*p*/, size_t /*n*/)
{
}

void ScOrcusSheet::set_formula_result(row_t /*row*/, col_t /*col*/, const char* /*p*/, size_t /*n*/)
{
}

void ScOrcusSheet::set_shared_formula(
    row_t /*row*/, col_t /*col*/, formula_grammar_t /*grammar*/, size_t /*sindex*/,
    const char* /*p_formula*/, size_t /*n_formula*/)
{
}

void ScOrcusSheet::set_shared_formula(
    row_t /*row*/, col_t /*col*/, formula_grammar_t /*grammar*/, size_t /*sindex*/,
    const char* /*p_formula*/, size_t /*n_formula*/, const char* /*p_range*/, size_t /*n_range*/)
{
}

void ScOrcusSheet::set_shared_formula(row_t /*row*/, col_t /*col*/, size_t /*sindex*/)
{
}

void ScOrcusSheet::set_string(row_t /*row*/, col_t /*col*/, size_t /*sindex*/)
{
}

void ScOrcusSheet::set_value(row_t /*row*/, col_t /*col*/, double /*value*/)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
