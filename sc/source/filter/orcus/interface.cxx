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
#include "globalnames.hxx"
#include "docoptio.hxx"
#include "globstr.hrc"
#include "compiler.hxx"

#include "formula/token.hxx"
#include "tools/datetime.hxx"

#include <com/sun/star/task/XStatusIndicator.hpp>

using namespace com::sun::star;

namespace os = orcus::spreadsheet;

ScOrcusGlobalSettings::ScOrcusGlobalSettings(ScDocumentImport& rDoc) : mrDoc(rDoc) {}

void ScOrcusGlobalSettings::set_origin_date(int year, int month, int day)
{
    mrDoc.setOriginDate(year, month, day);
}

ScOrcusFactory::StringCellCache::StringCellCache(const ScAddress& rPos, size_t nIndex) :
    maPos(rPos), mnIndex(nIndex) {}

ScOrcusFactory::ScOrcusFactory(ScDocument& rDoc) :
    maDoc(rDoc),
    maGlobalSettings(maDoc),
    maSharedStrings(*this),
    mnProgress(0) {}

orcus::spreadsheet::iface::import_sheet* ScOrcusFactory::append_sheet(const char* sheet_name, size_t sheet_name_length)
{
    OUString aTabName(sheet_name, sheet_name_length, RTL_TEXTENCODING_UTF8);
    if (!maDoc.appendSheet(aTabName))
        return NULL;

    SCTAB nTab = maDoc.getSheetCount() - 1;
    maSheets.push_back(new ScOrcusSheet(maDoc, nTab, *this));
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
    SCTAB nTab = maDoc.getSheetIndex(aTabName);
    if (nTab < 0)
        // Sheet by that name not found.
        return NULL;

    // See if we already have an orcus sheet instance by that index.
    boost::ptr_vector<ScOrcusSheet>::iterator it =
        std::find_if(maSheets.begin(), maSheets.end(), FindSheetByIndex(nTab));

    if (it != maSheets.end())
        // We already have one. Return it.
        return &(*it);

    // Create a new orcus sheet instance for this.
    maSheets.push_back(new ScOrcusSheet(maDoc, nTab, *this));
    return &maSheets.back();
}

orcus::spreadsheet::iface::import_global_settings* ScOrcusFactory::get_global_settings()
{
    return &maGlobalSettings;
}

orcus::spreadsheet::iface::import_shared_strings* ScOrcusFactory::get_shared_strings()
{
    return &maSharedStrings;
}

orcus::spreadsheet::iface::import_styles* ScOrcusFactory::get_styles()
{
    return &maStyles;
}

void ScOrcusFactory::finalize()
{
    int nCellCount = 0;
    StringCellCaches::const_iterator it = maStringCells.begin(), itEnd = maStringCells.end();
    for (; it != itEnd; ++it)
    {
        if (it->mnIndex >= maStrings.size())
            // String index out-of-bound!  Something is up.
            continue;

        maDoc.setStringCell(it->maPos, maStrings[it->mnIndex]);
        ++nCellCount;
        if (nCellCount == 100000)
        {
            incrementProgress();
            nCellCount = 0;
        }
    }

    if (mxStatusIndicator.is())
        mxStatusIndicator->end();

    maDoc.finalize();
}

size_t ScOrcusFactory::appendString(const OUString& rStr)
{
    size_t nPos = maStrings.size();
    maStrings.push_back(rStr);
    maStringHash.insert(StringHashType::value_type(rStr, nPos));

    return nPos;
}

size_t ScOrcusFactory::addString(const OUString& rStr)
{
    // Add only if the string is not yet present in the string pool.
    StringHashType::iterator it = maStringHash.find(rStr);
    if (it != maStringHash.end())
        return it->second;

    return appendString(rStr);
}

void ScOrcusFactory::pushStringCell(const ScAddress& rPos, size_t nStrIndex)
{
    maStringCells.push_back(StringCellCache(rPos, nStrIndex));
}

void ScOrcusFactory::incrementProgress()
{
    if (!mxStatusIndicator.is())
        // Status indicator object not set.
        return;

    // For now, we'll hard-code the progress range to be 100, and stops at 99
    // in all cases.

    if (!mnProgress)
        mxStatusIndicator->start(ScGlobal::GetRscString(STR_LOAD_DOC), 100);

    if (mnProgress == 99)
        return;

    ++mnProgress;
    mxStatusIndicator->setValue(mnProgress);
}

void ScOrcusFactory::setStatusIndicator(const uno::Reference<task::XStatusIndicator>& rIndicator)
{
    mxStatusIndicator = rIndicator;
}

ScOrcusSheet::ScOrcusSheet(ScDocumentImport& rDoc, SCTAB nTab, ScOrcusFactory& rFactory) :
    mrDoc(rDoc), mnTab(nTab), mrFactory(rFactory), mnCellCount(0) {}

void ScOrcusSheet::cellInserted()
{
    ++mnCellCount;
    if (mnCellCount == 100000)
    {
        mrFactory.incrementProgress();
        mnCellCount = 0;
    }
}

void ScOrcusSheet::set_auto(os::row_t row, os::col_t col, const char* p, size_t n)
{
    OUString aVal(p, n, RTL_TEXTENCODING_UTF8);
    mrDoc.setAutoInput(ScAddress(col, row, mnTab), aVal);
    cellInserted();
}

void ScOrcusSheet::set_string(os::row_t row, os::col_t col, size_t sindex)
{
    // We need to defer string cells since the shared string pool is not yet
    // populated at the time this method is called.  Orcus imports string
    // table after the cells get imported.  We won't need to do this once we
    // implement true shared strings in Calc core.

    mrFactory.pushStringCell(ScAddress(col, row, mnTab), sindex);
    cellInserted();
}

void ScOrcusSheet::set_value(os::row_t row, os::col_t col, double value)
{
    mrDoc.setNumericCell(ScAddress(col, row, mnTab), value);
    cellInserted();
}

void ScOrcusSheet::set_bool(os::row_t row, os::col_t col, bool value)
{
    mrDoc.setNumericCell(ScAddress(col, row, mnTab), value ? 1.0 : 0.0);
    cellInserted();
}

void ScOrcusSheet::set_date_time(
    os::row_t row, os::col_t col, int year, int month, int day, int hour, int minute, double second)
{
    SvNumberFormatter* pFormatter = mrDoc.getDoc().GetFormatTable();

    Date aDate(day, month, year);
    sal_uInt32 nSec = floor(second);
    sal_uInt32 nNanoSec = (second - nSec) * ::Time::nanoSecPerSec;
    Time aTime(hour, minute, nSec, nNanoSec);
    Date aNullDate(*pFormatter->GetNullDate());
    long nDateDiff = aDate - aNullDate;

    double fTime =
        static_cast<double>(aTime.GetNanoSec()) / ::Time::nanoSecPerSec +
        aTime.GetSec() +
        aTime.GetMin() * ::Time::secondPerMinute +
        aTime.GetHour() * ::Time::secondPerHour;

    fTime /= DATE_TIME_FACTOR;

    mrDoc.setNumericCell(ScAddress(col, row, mnTab), nDateDiff + fTime);
    cellInserted();
}

void ScOrcusSheet::set_format(os::row_t /*row*/, os::col_t /*col*/, size_t /*xf_index*/)
{
}

namespace {

formula::FormulaGrammar::Grammar getCalcGrammarFromOrcus( os::formula_grammar_t grammar )
{
    formula::FormulaGrammar::Grammar eGrammar = formula::FormulaGrammar::GRAM_ODFF;
    switch(grammar)
    {
        case orcus::spreadsheet::ods:
            eGrammar = formula::FormulaGrammar::GRAM_ODFF;
            break;
        case orcus::spreadsheet::xlsx_2007:
        case orcus::spreadsheet::xlsx_2010:
            eGrammar = formula::FormulaGrammar::GRAM_ENGLISH_XL_OOX;
            break;
        case orcus::spreadsheet::gnumeric:
            eGrammar = formula::FormulaGrammar::GRAM_ENGLISH_XL_A1;
            break;
    }

    return eGrammar;
}

class SharedFormulaGroups
{
    struct Key
    {
        sal_Int32 mnId;
        sal_Int32 mnCol;

        Key(sal_Int32 nId, sal_Int32 nCol) : mnId(nId), mnCol(nCol) {}

        bool operator== ( const Key& rOther ) const
        {
            return mnId == rOther.mnId && mnCol == rOther.mnCol;
        }

        bool operator!= ( const Key& rOther ) const
        {
            return !operator==(rOther);
        }
    };

    struct KeyHash
    {
        size_t operator() ( const Key& rKey ) const
        {
            double nVal = rKey.mnId;
            nVal *= 256.0;
            nVal += rKey.mnCol;
            return static_cast<size_t>(nVal);
        }
    };

    typedef boost::unordered_map<Key, ScFormulaCellGroupRef, KeyHash> StoreType;
    StoreType maStore;
public:

    void set( sal_Int32 nSharedId, sal_Int32 nCol, const ScFormulaCellGroupRef& xGroup )
    {
        Key aKey(nSharedId, nCol);
        maStore.insert(StoreType::value_type(aKey, xGroup));
    }

    ScFormulaCellGroupRef get( sal_Int32 nSharedId, sal_Int32 nCol ) const
    {
        Key aKey(nSharedId, nCol);
        StoreType::const_iterator it = maStore.find(aKey);
        return it == maStore.end() ? ScFormulaCellGroupRef() : it->second;
    }
};

}

void ScOrcusSheet::set_formula(
    os::row_t row, os::col_t col, os::formula_grammar_t grammar, const char* p, size_t n)
{
    OUString aFormula(p, n, RTL_TEXTENCODING_UTF8);
    formula::FormulaGrammar::Grammar eGrammar = getCalcGrammarFromOrcus( grammar );
    mrDoc.setFormulaCell(ScAddress(col,row,mnTab), aFormula, eGrammar);
    cellInserted();
}

void ScOrcusSheet::set_formula_result(os::row_t row, os::col_t col, const char* p, size_t n)
{
    ScFormulaCell* pCell = mrDoc.getDoc().GetFormulaCell(ScAddress(col, row, mnTab));
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
    os::row_t /*row*/, os::col_t /*col*/, os::formula_grammar_t /*grammar*/, size_t /*sindex*/,
    const char* /*p_formula*/, size_t /*n_formula*/)
{
    // TODO: We need to revise this interface in orcus.
}

void ScOrcusSheet::set_shared_formula(
    os::row_t row, os::col_t col, os::formula_grammar_t grammar, size_t sindex,
    const char* p_formula, size_t n_formula, const char* p_range, size_t n_range)
{
    ScAddress aPos(col, row, mnTab);
    OUString aFormula(p_formula, n_formula, RTL_TEXTENCODING_UTF8);
    OUString aRangeStr(p_range, n_range, RTL_TEXTENCODING_UTF8);
    formula::FormulaGrammar::Grammar eGram = getCalcGrammarFromOrcus(grammar);
    formula::FormulaGrammar::AddressConvention eConv = formula::FormulaGrammar::extractRefConvention(eGram);

    // Convert the shared formula range.
    ScRange aRange;
    sal_uInt16 nRes = aRange.Parse(aRangeStr, &mrDoc.getDoc(), eConv);
    if (!(nRes & SCA_VALID))
        // Conversion failed.
        return;

    // Compile the formula expression into tokens.
    ScCompiler aComp(&mrDoc.getDoc(), aPos);
    aComp.SetGrammar(eGram);
    ScTokenArray* pArray = aComp.CompileString(aFormula);
    if (!pArray)
        // Tokenization failed.
        return;

    for (sal_Int32 nCol = aRange.aStart.Col(); nCol <= aRange.aEnd.Col(); ++nCol)
    {
        // Create one group per column, since Calc doesn't support shared
        // formulas across multiple columns.
        ScFormulaCellGroupRef xNewGroup(new ScFormulaCellGroup);
        xNewGroup->mnStart = aRange.aStart.Row();
        xNewGroup->mnLength = aRange.aEnd.Row() - aRange.aStart.Row() + 1;
        xNewGroup->setCode(*pArray);
        maFormulaGroups.set(sindex, nCol, xNewGroup);
    }

    ScFormulaCellGroupRef xGroup = maFormulaGroups.get(sindex, aPos.Col());
    if (!xGroup)
        return;

    // Generate code for the top cell only.
    xGroup->compileCode(mrDoc.getDoc(), aPos, formula::FormulaGrammar::GRAM_DEFAULT);
    ScFormulaCell* pCell = new ScFormulaCell(&mrDoc.getDoc(), aPos, xGroup);
    mrDoc.setFormulaCell(aPos, pCell);
    cellInserted();

    // For now, orcus doesn't support setting cached result. Mark it for re-calculation.
    pCell->SetDirty(true);
    pCell->StartListeningTo(&mrDoc.getDoc());
}

void ScOrcusSheet::set_shared_formula(os::row_t row, os::col_t col, size_t sindex)
{
    ScAddress aPos(col, row, mnTab);

    ScFormulaCellGroupRef xGroup = maFormulaGroups.get(sindex, aPos.Col());
    if (!xGroup)
        return;

    ScFormulaCell* pCell = new ScFormulaCell(&mrDoc.getDoc(), aPos, xGroup);
    mrDoc.setFormulaCell(aPos, pCell);
    cellInserted();

    // For now, orcus doesn't support setting cached result. Mark it for re-calculation.
    pCell->SetDirty(true);
    pCell->StartListeningTo(&mrDoc.getDoc());
}

void ScOrcusSheet::set_array_formula(
    os::row_t /*row*/, os::col_t /*col*/, os::formula_grammar_t /*grammar*/,
    const char* /*p*/, size_t /*n*/, os::row_t /*array_rows*/, os::col_t /*array_cols*/)
{
}

void ScOrcusSheet::set_array_formula(
    os::row_t /*row*/, os::col_t /*col*/, os::formula_grammar_t /*grammar*/,
    const char* /*p*/, size_t /*n*/, const char* /*p_range*/, size_t /*n_range*/)
{
}

ScOrcusSharedStrings::ScOrcusSharedStrings(ScOrcusFactory& rFactory) :
    mrFactory(rFactory) {}

size_t ScOrcusSharedStrings::append(const char* s, size_t n)
{
    OUString aNewString(s, n, RTL_TEXTENCODING_UTF8);
    return mrFactory.appendString(aNewString);
}

size_t ScOrcusSharedStrings::add(const char* s, size_t n)
{
    OUString aNewString(s, n, RTL_TEXTENCODING_UTF8);
    return mrFactory.addString(aNewString);
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

void ScOrcusSharedStrings::append_segment(const char* s, size_t n)
{
    maCurSegment.append(s, n);
}

size_t ScOrcusSharedStrings::commit_segments()
{
    OString aStr = maCurSegment.makeStringAndClear();
    return mrFactory.addString(OStringToOUString(aStr, RTL_TEXTENCODING_UTF8));
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
