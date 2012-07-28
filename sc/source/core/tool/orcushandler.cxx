/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 *   Copyright (C) 2012 Kohei Yoshida <kohei.yoshida@suse.com>
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "orcushandler.hxx"
#include "document.hxx"

#include "tools/urlobj.hxx"

#include <orcus/model/interface.hpp>
#include <orcus/orcus_csv.hpp>

#include <boost/ptr_container/ptr_vector.hpp>

using orcus::model::row_t;
using orcus::model::col_t;
using orcus::model::formula_grammar_t;

namespace {

class ScOrcusSheet;

class ScOrcusFactory : public orcus::model::iface::factory
{
    ScDocument& mrDoc;
    boost::ptr_vector<ScOrcusSheet> maSheets;

public:
    ScOrcusFactory(ScDocument& rDoc);

    virtual orcus::model::iface::sheet* append_sheet(const char *sheet_name, size_t sheet_name_length);
    virtual orcus::model::iface::shared_strings* get_shared_strings();
    virtual orcus::model::iface::styles* get_styles();
};

class ScOrcusSheet : public orcus::model::iface::sheet
{
    ScDocument& mrDoc;
    SCTAB mnTab;
public:
    ScOrcusSheet(ScDocument& rDoc, SCTAB nTab);

    virtual void set_auto(row_t row, col_t col, const char* p, size_t n);
    virtual void set_format(row_t row, col_t col, size_t xf_index);
    virtual void set_formula(row_t row, col_t col, formula_grammar_t grammar, const char* p, size_t n);
    virtual void set_formula_result(row_t row, col_t col, const char* p, size_t n);
    virtual void set_shared_formula(
        row_t row, col_t col, formula_grammar_t grammar, size_t sindex,
        const char* p_formula, size_t n_formula, const char* p_range, size_t n_range);
    virtual void set_shared_formula(row_t row, col_t col, size_t sindex);
    virtual void set_string(row_t row, col_t col, size_t sindex);
    virtual void set_value(row_t row, col_t col, double value);
};

ScOrcusFactory::ScOrcusFactory(ScDocument& rDoc) : mrDoc(rDoc) {}

orcus::model::iface::sheet* ScOrcusFactory::append_sheet(const char* sheet_name, size_t sheet_name_length)
{
    OUString aTabName(sheet_name, sheet_name_length, RTL_TEXTENCODING_UTF8);
    if (!mrDoc.InsertTab(SC_TAB_APPEND, aTabName))
        return NULL;

    SCTAB nTab = mrDoc.GetTableCount() - 1;
    maSheets.push_back(new ScOrcusSheet(mrDoc, nTab));
    return &maSheets.back();
}

orcus::model::iface::shared_strings* ScOrcusFactory::get_shared_strings()
{
    // We don't support it yet.
    return NULL;
}

orcus::model::iface::styles* ScOrcusFactory::get_styles()
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

} // anonymous namespace

bool ScOrcusFilters::importCSV(ScDocument& rDoc, const OUString& rPath)
{
    ScOrcusFactory aFactory(rDoc);
    INetURLObject aURL(rPath);
    const char* path = rtl::OUStringToOString(aURL.getFSysPath(INetURLObject::FSYS_UNX), RTL_TEXTENCODING_UTF8).getStr();

    try
    {
        orcus::orcus_csv filter(&aFactory);
        filter.read_file(path);
    }
    catch (const std::exception&)
    {
        rDoc.InsertTab(SC_TAB_APPEND, OUString("Foo"));
        rDoc.SetString(0, 0, 0, "Failed to load!!!");
        return false;
    }
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
