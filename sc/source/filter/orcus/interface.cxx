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
#include "formulacell.hxx"
#include "rangenam.hxx"
#include "tokenarray.hxx"
#include "stringutil.hxx"

#include <formula/token.hxx>

using orcus::spreadsheet::row_t;
using orcus::spreadsheet::col_t;
using orcus::spreadsheet::formula_grammar_t;

ScOrcusFactory::StringCellCache::StringCellCache(const ScAddress& rPos, size_t nIndex) :
    maPos(rPos), mnIndex(nIndex) {}

ScOrcusFactory::ScOrcusFactory(ScDocument& rDoc) : mrDoc(rDoc), maSharedStrings(maStrings) {}

orcus::spreadsheet::iface::import_sheet* ScOrcusFactory::append_sheet(const char* sheet_name, size_t sheet_name_length)
{
    OUString aTabName(sheet_name, sheet_name_length, RTL_TEXTENCODING_UTF8);
    if (!mrDoc.InsertTab(SC_TAB_APPEND, aTabName))
        return NULL;

    SCTAB nTab = mrDoc.GetTableCount() - 1;
    maSheets.push_back(new ScOrcusSheet(mrDoc, nTab, *this));
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
    maSheets.push_back(new ScOrcusSheet(mrDoc, nTab, *this));
    return &maSheets.back();
}

orcus::spreadsheet::iface::import_shared_strings* ScOrcusFactory::get_shared_strings()
{
    return &maSharedStrings;
}

orcus::spreadsheet::iface::import_styles* ScOrcusFactory::get_styles()
{
    // We don't support it yet.
    return &maStyles;
}

void ScOrcusFactory::finalize()
{
    ScSetStringParam aParam;
    aParam.setTextInput();
    StringCellCaches::const_iterator it = maStringCells.begin(), itEnd = maStringCells.end();
    for (; it != itEnd; ++it)
    {
        if (it->mnIndex >= maStrings.size())
            // String index out-of-bound!  Something is up.
            continue;

        mrDoc.SetString(it->maPos, maStrings[it->mnIndex], &aParam);
    }
}

void ScOrcusFactory::pushStringCell(const ScAddress& rPos, size_t nStrIndex)
{
    maStringCells.push_back(StringCellCache(rPos, nStrIndex));
}

ScOrcusSheet::ScOrcusSheet(ScDocument& rDoc, SCTAB nTab, ScOrcusFactory& rFactory) :
    mrDoc(rDoc), mnTab(nTab), mrFactory(rFactory) {}

void ScOrcusSheet::set_auto(row_t row, col_t col, const char* p, size_t n)
{
    OUString aVal(p, n, RTL_TEXTENCODING_UTF8);
    mrDoc.SetString(col, row, mnTab, aVal);
}

void ScOrcusSheet::set_format(row_t /*row*/, col_t /*col*/, size_t /*xf_index*/)
{
}

namespace {

formula::FormulaGrammar::Grammar getCalcGrammarFromOrcus( formula_grammar_t grammar )
{
    formula::FormulaGrammar::Grammar eGrammar = formula::FormulaGrammar::GRAM_ODFF;
    switch(grammar)
    {
        case orcus::spreadsheet::ods:
            eGrammar = formula::FormulaGrammar::GRAM_ODFF;
            break;
        case orcus::spreadsheet::xlsx_2007:
        case orcus::spreadsheet::xlsx_2010:
            eGrammar = formula::FormulaGrammar::GRAM_ENGLISH_XL_A1;
            break;
        case orcus::spreadsheet::gnumeric:
            eGrammar = formula::FormulaGrammar::GRAM_ENGLISH_XL_A1;
            break;
    }

    return eGrammar;
}

}

void ScOrcusSheet::set_formula(
    row_t row, col_t col, formula_grammar_t grammar, const char* p, size_t n)
{
    OUString aFormula(p, n, RTL_TEXTENCODING_UTF8);
    formula::FormulaGrammar::Grammar eGrammar = getCalcGrammarFromOrcus( grammar );
    mrDoc.SetFormula(ScAddress(col,row,mnTab), aFormula, eGrammar);
}

void ScOrcusSheet::set_formula_result(row_t row, col_t col, const char* p, size_t n)
{
    ScFormulaCell* pCell = mrDoc.GetFormulaCell(ScAddress(col, row, mnTab));
    if (!pCell)
    {
        SAL_WARN("sc", "trying to set formula result for non formula \
                cell! Col: " << col << ";Row: " << row << ";Tab: " << mnTab);
        return;
    }
    OUString aResult( p, n, RTL_TEXTENCODING_UTF8);
    pCell->SetHybridString(aResult);
}

void ScOrcusSheet::set_shared_formula(
    row_t row, col_t col, formula_grammar_t grammar, size_t sindex,
    const char* p_formula, size_t n_formula)
{
    OUString aFormula( p_formula, n_formula, RTL_TEXTENCODING_UTF8 );
    formula::FormulaGrammar::Grammar eGrammar =  getCalcGrammarFromOrcus( grammar );
    ScRangeName* pRangeName = mrDoc.GetRangeName();

    OUString aName("shared_");
    aName += OUString::valueOf(sal_Int32(pRangeName->size()));
    ScRangeData* pSharedFormula = new ScRangeData( &mrDoc, aName, aFormula, ScAddress(col, row, mnTab), RT_SHARED, eGrammar);
    if(pRangeName->insert(pSharedFormula))
    {
        maSharedFormulas.insert( std::pair<size_t, ScRangeData*>(sindex, pSharedFormula) );
        ScTokenArray aArr;
        aArr.AddToken( formula::FormulaIndexToken( ocName, pSharedFormula->GetIndex() ) );
        mrDoc.SetFormula(ScAddress(col,row,mnTab), aArr);
    }
}

void ScOrcusSheet::set_shared_formula(
    row_t row, col_t col, formula_grammar_t grammar, size_t sindex,
    const char* p_formula, size_t n_formula, const char* /*p_range*/, size_t /*n_range*/)
{
    OUString aFormula( p_formula, n_formula, RTL_TEXTENCODING_UTF8 );
    formula::FormulaGrammar::Grammar eGrammar = getCalcGrammarFromOrcus( grammar );
    ScRangeName* pRangeName = mrDoc.GetRangeName();

    OUString aName("shared_");
    aName += OUString::valueOf(sal_Int32(pRangeName->size()));
    ScRangeData* pSharedFormula = new ScRangeData( &mrDoc, aName, aFormula, ScAddress(col, row, mnTab), RT_SHARED, eGrammar);
    if(pRangeName->insert(pSharedFormula))
    {
        maSharedFormulas.insert( std::pair<size_t, ScRangeData*>(sindex, pSharedFormula) );
        ScTokenArray aArr;
        aArr.AddToken( formula::FormulaIndexToken( ocName, pSharedFormula->GetIndex() ) );
        mrDoc.SetFormula(ScAddress(col,row,mnTab), aArr);
    }
}

void ScOrcusSheet::set_shared_formula(row_t row, col_t col, size_t sindex)
{
    if(maSharedFormulas.find(sindex) == maSharedFormulas.end())
        return;

    ScRangeData* pSharedFormula = maSharedFormulas.find(sindex)->second;
    ScTokenArray aArr;
    aArr.AddToken( formula::FormulaIndexToken( ocName, pSharedFormula->GetIndex() ) );
    mrDoc.SetFormula(ScAddress(col,row,mnTab), aArr);
}

void ScOrcusSheet::set_array_formula(
    row_t /*row*/, col_t /*col*/, formula_grammar_t /*grammar*/,
    const char* /*p*/, size_t /*n*/, row_t /*array_rows*/, col_t /*array_cols*/)
{
}

void ScOrcusSheet::set_array_formula(
    row_t /*row*/, col_t /*col*/, formula_grammar_t /*grammar*/,
    const char* /*p*/, size_t /*n*/, const char* /*p_range*/, size_t /*n_range*/)
{
}

void ScOrcusSheet::set_string(row_t row, col_t col, size_t sindex)
{
    // We need to defer string cells since the shared string pool is not yet
    // populated at the time this method is called.  Orcus imports string
    // table after the cells get imported.  We won't need to do this once we
    // implement true shared strings in Calc core.

    mrFactory.pushStringCell(ScAddress(col, row, mnTab), sindex);
}

void ScOrcusSheet::set_value(row_t row, col_t col, double value)
{
    mrDoc.SetValue( col, row, mnTab, value );
}

void ScOrcusSheet::set_bool(row_t row, col_t col, bool value)
{
    mrDoc.SetValue(col, row, mnTab, value ? 1.0 : 0.0);
}

ScOrcusSharedStrings::ScOrcusSharedStrings(std::vector<OUString>& rStrings) :
    mrStrings(rStrings) {}

size_t ScOrcusSharedStrings::append(const char* s, size_t n)
{
    OUString aNewString(s, n, RTL_TEXTENCODING_UTF8);
    mrStrings.push_back(aNewString);

    return mrStrings.size() - 1;
}

size_t ScOrcusSharedStrings::add(const char* s, size_t n)
{
    OUString aNewString(s, n, RTL_TEXTENCODING_UTF8);
    mrStrings.push_back(aNewString);

    return mrStrings.size() - 1;
}

void ScOrcusSharedStrings::set_segment_bold(bool /*b*/)
{
}
void ScOrcusSharedStrings::set_segment_italic(bool /*b*/)
{
}
void ScOrcusSharedStrings::set_segment_font_name(const char* /*s*/, size_t /*n*/)
{
}
void ScOrcusSharedStrings::set_segment_font_size(double /*point*/)
{
}
void ScOrcusSharedStrings::append_segment(const char* /*s*/, size_t /*n*/)
{
}

size_t ScOrcusSharedStrings::commit_segments()
{
    return 0;
}

void ScOrcusStyles::set_font_count(size_t /*n*/)
{
    // needed at all?
}

void ScOrcusStyles::set_font_bold(bool /*b*/)
{
}

void ScOrcusStyles::set_font_italic(bool /*b*/)
{
}

void ScOrcusStyles::set_font_name(const char* /*s*/, size_t /*n*/)
{
}

void ScOrcusStyles::set_font_size(double /*point*/)
{
}

void ScOrcusStyles::set_font_underline(orcus::spreadsheet::underline_t /*e*/)
{
}

size_t ScOrcusStyles::commit_font()
{
    return 0;
}


// fill

void ScOrcusStyles::set_fill_count(size_t /*n*/)
{
    // needed at all?
}

void ScOrcusStyles::set_fill_pattern_type(const char* /*s*/, size_t /*n*/)
{
}

void ScOrcusStyles::set_fill_fg_color(orcus::spreadsheet::color_elem_t /*alpha*/, orcus::spreadsheet::color_elem_t /*red*/, orcus::spreadsheet::color_elem_t /*green*/, orcus::spreadsheet::color_elem_t /*blue*/)
{
}

void ScOrcusStyles::set_fill_bg_color(orcus::spreadsheet::color_elem_t /*alpha*/, orcus::spreadsheet::color_elem_t /*red*/, orcus::spreadsheet::color_elem_t /*green*/, orcus::spreadsheet::color_elem_t /*blue*/)
{
}

size_t ScOrcusStyles::commit_fill()
{
    return 0;
}


// border

void ScOrcusStyles::set_border_count(size_t /*n*/)
{
    // needed at all?
}

void ScOrcusStyles::set_border_style(orcus::spreadsheet::border_direction_t /*dir*/, const char* /*s*/, size_t /*n*/)
{
    // implement later
}

size_t ScOrcusStyles::commit_border()
{
    return 0;
}


// cell protection
void ScOrcusStyles::set_cell_hidden(bool /*b*/)
{
}

void ScOrcusStyles::set_cell_locked(bool /*b*/)
{
}

size_t ScOrcusStyles::commit_cell_protection()
{
    return 0;
}

void ScOrcusStyles::set_number_format(const char* /*s*/, size_t /*n*/)
{
}

size_t ScOrcusStyles::commit_number_format()
{
    return 0;
}

// cell style xf

void ScOrcusStyles::set_cell_style_xf_count(size_t /*n*/)
{
    // needed at all?
}

size_t ScOrcusStyles::commit_cell_style_xf()
{
    return 0;
}


// cell xf

void ScOrcusStyles::set_cell_xf_count(size_t /*n*/)
{
    // needed at all?
}

size_t ScOrcusStyles::commit_cell_xf()
{
    return 0;
}


// xf (cell format) - used both by cell xf and cell style xf.

void ScOrcusStyles::set_xf_number_format(size_t /*index*/)
{
    // no number format interfaces implemented yet
}

void ScOrcusStyles::set_xf_font(size_t /*index*/)
{
}

void ScOrcusStyles::set_xf_fill(size_t /*index*/)
{
}

void ScOrcusStyles::set_xf_border(size_t /*index*/)
{
}

void ScOrcusStyles::set_xf_protection(size_t /*index*/)
{
}

void ScOrcusStyles::set_xf_style_xf(size_t /*index*/)
{
}


// cell style entry
// not needed for now for gnumeric

void ScOrcusStyles::set_cell_style_count(size_t /*n*/)
{
    // needed at all?
}

void ScOrcusStyles::set_cell_style_name(const char* /*s*/, size_t /*n*/)
{
}

void ScOrcusStyles::set_cell_style_xf(size_t /*index*/)
{
}

void ScOrcusStyles::set_cell_style_builtin(size_t /*index*/)
{
    // not needed for gnumeric
}

size_t ScOrcusStyles::commit_cell_style()
{
    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
