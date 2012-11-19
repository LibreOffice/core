/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __SC_ORCUSINTERFACE_HXX__
#define __SC_ORCUSINTERFACE_HXX__

#include "address.hxx"

#define __ORCUS_STATIC_LIB
#include <orcus/spreadsheet/import_interface.hpp>

#include <boost/ptr_container/ptr_vector.hpp>

class ScDocument;
class ScOrcusSheet;

class ScOrcusFactory : public orcus::spreadsheet::iface::import_factory
{
    ScDocument& mrDoc;
    boost::ptr_vector<ScOrcusSheet> maSheets;

public:
    ScOrcusFactory(ScDocument& rDoc);

    virtual orcus::spreadsheet::iface::import_sheet* append_sheet(const char *sheet_name, size_t sheet_name_length);
    virtual orcus::spreadsheet::iface::import_sheet* get_sheet(const char *sheet_name, size_t sheet_name_length);
    virtual orcus::spreadsheet::iface::import_shared_strings* get_shared_strings();
    virtual orcus::spreadsheet::iface::import_styles* get_styles();
};

class ScOrcusSheet : public orcus::spreadsheet::iface::import_sheet
{
    ScDocument& mrDoc;
    SCTAB mnTab;
public:
    ScOrcusSheet(ScDocument& rDoc, SCTAB nTab);

    // Orcus import interface
    virtual void set_auto(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, const char* p, size_t n);
    virtual void set_format(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, size_t xf_index);
    virtual void set_formula(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, orcus::spreadsheet::formula_grammar_t grammar, const char* p, size_t n);
    virtual void set_formula_result(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, const char* p, size_t n);
    virtual void set_shared_formula(
        orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, orcus::spreadsheet::formula_grammar_t grammar, size_t sindex,
        const char* p_formula, size_t n_formula);
    virtual void set_shared_formula(
        orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, orcus::spreadsheet::formula_grammar_t grammar, size_t sindex,
        const char* p_formula, size_t n_formula, const char* p_range, size_t n_range);
    virtual void set_shared_formula(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, size_t sindex);
    virtual void set_string(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, size_t sindex);
    virtual void set_value(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, double value);

    SCTAB getIndex() const { return mnTab; }
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
