/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <documentimport.hxx>
#include <formula/grammar.hxx>

#include <orcus/spreadsheet/import_interface.hpp>

formula::FormulaGrammar::Grammar
getCalcGrammarFromOrcus(orcus::spreadsheet::formula_grammar_t grammar);

class ScOrcusGlobalSettings : public orcus::spreadsheet::iface::import_global_settings
{
    ScDocumentImport& mrDoc;
    formula::FormulaGrammar::Grammar meCalcGrammar;
    orcus::spreadsheet::formula_grammar_t meOrcusGrammar;
    rtl_TextEncoding mnTextEncoding;

public:
    ScOrcusGlobalSettings(ScDocumentImport& rDoc);

    virtual void set_origin_date(int year, int month, int day) override;
    virtual void set_character_set(orcus::character_set_t cs) override;

    virtual void
    set_default_formula_grammar(orcus::spreadsheet::formula_grammar_t grammar) override;
    virtual orcus::spreadsheet::formula_grammar_t get_default_formula_grammar() const override;

    formula::FormulaGrammar::Grammar getCalcGrammar() const { return meCalcGrammar; }

    rtl_TextEncoding getTextEncoding() const { return mnTextEncoding; }

    ScDocumentImport& getDoc() const { return mrDoc; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
