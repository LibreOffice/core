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
#include "dbdata.hxx"
#include "stlpool.hxx"
#include "scitems.hxx"
#include "patattr.hxx"
#include "docpool.hxx"

#include <editeng/postitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/borderline.hxx>

#include <formula/token.hxx>
#include <tools/datetime.hxx>
#include <svl/sharedstringpool.hxx>
#include <o3tl/make_unique.hxx>

#include <com/sun/star/task/XStatusIndicator.hpp>

using namespace com::sun::star;

namespace os = orcus::spreadsheet;

ScOrcusGlobalSettings::ScOrcusGlobalSettings(ScDocumentImport& rDoc) : mrDoc(rDoc) {}

void ScOrcusGlobalSettings::set_origin_date(int year, int month, int day)
{
    mrDoc.setOriginDate(year, month, day);
}

void ScOrcusGlobalSettings::set_default_formula_grammar(orcus::spreadsheet::formula_grammar_t /*grammar*/)
{
}

orcus::spreadsheet::formula_grammar_t ScOrcusGlobalSettings::get_default_formula_grammar() const
{
    return orcus::spreadsheet::formula_grammar_unknown;
}

ScOrcusFactory::StringCellCache::StringCellCache(const ScAddress& rPos, size_t nIndex) :
    maPos(rPos), mnIndex(nIndex) {}

ScOrcusFactory::ScOrcusFactory(ScDocument& rDoc) :
    maDoc(rDoc),
    maGlobalSettings(maDoc),
    maSharedStrings(*this),
    maStyles(rDoc),
    mnProgress(0) {}

orcus::spreadsheet::iface::import_sheet* ScOrcusFactory::append_sheet(const char* sheet_name, size_t sheet_name_length)
{
    OUString aTabName(sheet_name, sheet_name_length, RTL_TEXTENCODING_UTF8);
    if (!maDoc.appendSheet(aTabName))
        return nullptr;

    SCTAB nTab = maDoc.getSheetCount() - 1;
    maSheets.push_back(o3tl::make_unique<ScOrcusSheet>(maDoc, nTab, *this));
    return maSheets.back().get();
}

class FindSheetByIndex : std::unary_function< std::unique_ptr<ScOrcusSheet>, bool>
{
    SCTAB mnTab;
public:
    explicit FindSheetByIndex(SCTAB nTab) : mnTab(nTab) {}
    bool operator() (const std::unique_ptr<ScOrcusSheet>& rSheet) const
    {
        return rSheet->getIndex() == mnTab;
    }
};

orcus::spreadsheet::iface::import_sheet* ScOrcusFactory::get_sheet(const char* sheet_name, size_t sheet_name_length)
{
    OUString aTabName(sheet_name, sheet_name_length, RTL_TEXTENCODING_UTF8);
    SCTAB nTab = maDoc.getSheetIndex(aTabName);
    if (nTab < 0)
        // Sheet by that name not found.
        return nullptr;

    // See if we already have an orcus sheet instance by that index.
    std::vector< std::unique_ptr<ScOrcusSheet> >::iterator it =
        std::find_if(maSheets.begin(), maSheets.end(), FindSheetByIndex(nTab));

    if (it != maSheets.end())
        // We already have one. Return it.
        return it->get();

    // Create a new orcus sheet instance for this.
    maSheets.push_back(o3tl::make_unique<ScOrcusSheet>(maDoc, nTab, *this));
    return maSheets.back().get();
}

orcus::spreadsheet::iface::import_sheet* ScOrcusFactory::get_sheet(orcus::spreadsheet::sheet_t sheet_index)
{
    SCTAB nTab = static_cast<SCTAB>(sheet_index);
    // See if we already have an orcus sheet instance by that index.
    std::vector< std::unique_ptr<ScOrcusSheet> >::iterator it =
        std::find_if(maSheets.begin(), maSheets.end(), FindSheetByIndex(nTab));

    if (it != maSheets.end())
        // We already have one. Return it.
        return it->get();

    // Create a new orcus sheet instance for this.
    maSheets.push_back(o3tl::make_unique<ScOrcusSheet>(maDoc, nTab, *this));
    return maSheets.back().get();
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

ScOrcusSheetProperties::ScOrcusSheetProperties(SCTAB nTab, ScDocumentImport& rDoc):
    mrDoc(rDoc),
    mnTab(nTab)
{
}

ScOrcusSheetProperties::~ScOrcusSheetProperties()
{
}

namespace {

double translateToInternal(double nVal, orcus::length_unit_t unit)
{
    switch(unit)
    {
        case orcus::length_unit_inch:
            return nVal * 72.0 * 20.0;
            break;
        case orcus::length_unit_twip:
            return nVal;
            break;
        case orcus::length_unit_point:
            return nVal * 20.0;
            break;
        case orcus::length_unit_centimeter:
            return nVal * 20.0 * 72.0 / 2.54;
            break;
        case orcus::length_unit_unknown:
            SAL_WARN("sc.orcus", "unknown unit");
            break;
        default:
            break;
    }
    return nVal;
}


}

void ScOrcusSheetProperties::set_column_width(os::col_t col, double width, orcus::length_unit_t unit)
{
    double nNewWidth = translateToInternal(width, unit);
    mrDoc.getDoc().SetColWidthOnly(col, mnTab, nNewWidth);
}

void ScOrcusSheetProperties::set_column_hidden(os::col_t col, bool hidden)
{
    if (hidden)
        mrDoc.getDoc().SetColHidden(col, col, mnTab, hidden);
}

void ScOrcusSheetProperties::set_row_height(os::row_t row, double height, orcus::length_unit_t unit)
{
    double nNewHeight = translateToInternal(height, unit);
    mrDoc.getDoc().SetRowHeightOnly(row, row,mnTab, nNewHeight);
}

void ScOrcusSheetProperties::set_row_hidden(os::row_t row, bool hidden)
{
    if (hidden)
        mrDoc.getDoc().SetRowHidden(row, row, mnTab, hidden);
}

void ScOrcusSheetProperties::set_merge_cell_range(const char* /*p_range*/, size_t /*n_range*/)
{
}

ScOrcusConditionalFormat::ScOrcusConditionalFormat(SCTAB nTab, ScDocument& rDoc):
    mnTab(nTab),
    mrDoc(rDoc),
    mpCurrentFormat(new ScConditionalFormat(0, &mrDoc)),
    meEntryType(condformat::CONDITION)
{
    (void)mnTab;
}

ScOrcusConditionalFormat::~ScOrcusConditionalFormat()
{
}

void ScOrcusConditionalFormat::set_color(os::color_elem_t /*alpha*/, os::color_elem_t /*red*/,
        os::color_elem_t /*green*/, os::color_elem_t /*blue*/)
{
    SAL_INFO("sc.orcus.condformat", "set_color");
}

void ScOrcusConditionalFormat::set_condition_type(os::condition_type_t /*type*/)
{
    assert(meEntryType == condformat::CONDITION);
    SAL_INFO("sc.orcus.condformat", "set_condition_type");
}

void ScOrcusConditionalFormat::set_formula(const char* /*p*/, size_t /*n*/)
{
    SAL_INFO("sc.orcus.condformat", "set_formula");
}

void ScOrcusConditionalFormat::set_date(os::condition_date_t /*date*/)
{
    assert(meEntryType == condformat::DATE);
    SAL_INFO("sc.orcus.condformat", "set_date");
}

void ScOrcusConditionalFormat::commit_condition()
{
    SAL_INFO("sc.orcus.condformat", "commit_condition");
}

void ScOrcusConditionalFormat::set_icon_name(const char* /*p*/, size_t /*n*/)
{
    assert(meEntryType == condformat::ICONSET);
    SAL_INFO("sc.orcus.condformat", "set_icon_name");
}

void ScOrcusConditionalFormat::set_databar_gradient(bool /*gradient*/)
{
    assert(meEntryType == condformat::DATABAR);
    SAL_INFO("sc.orcus.condformat", "set_databar_gradient");
}

void ScOrcusConditionalFormat::set_databar_axis(os::databar_axis_t /*axis*/)
{
    assert(meEntryType == condformat::DATABAR);
    SAL_INFO("sc.orcus.condformat", "set_databar_axis");
}

void ScOrcusConditionalFormat::set_databar_color_positive(os::color_elem_t /*alpha*/, os::color_elem_t /*red*/,
        os::color_elem_t /*green*/, os::color_elem_t /*blue*/)
{
    assert(meEntryType == condformat::DATABAR);
    SAL_INFO("sc.orcus.condformat", "set_databar_color_positive");
}

void ScOrcusConditionalFormat::set_databar_color_negative(os::color_elem_t /*alpha*/, os::color_elem_t /*red*/,
        os::color_elem_t /*green*/, os::color_elem_t /*blue*/)
{
    assert(meEntryType == condformat::DATABAR);
    SAL_INFO("sc.orcus.condformat", "set_databar_color_negative");
}

void ScOrcusConditionalFormat::set_min_databar_length(double /*length*/)
{
    assert(meEntryType == condformat::DATABAR);
    SAL_INFO("sc.orcus.condformat", "set_min_databar_length");
}

void ScOrcusConditionalFormat::set_max_databar_length(double /*length*/)
{
    assert(meEntryType == condformat::DATABAR);
    SAL_INFO("sc.orcus.condformat", "set_max_databar_length");
}

void ScOrcusConditionalFormat::set_show_value(bool /*show*/)
{
    SAL_INFO("sc.orcus.condformat", "set_show_value");
}

void ScOrcusConditionalFormat::set_iconset_reverse(bool /*reverse*/)
{
    assert(meEntryType == condformat::ICONSET);
    SAL_INFO("sc.orcus.condformat", "set_iconset_reverse");
}

void ScOrcusConditionalFormat::set_xf_id(size_t /*xf*/)
{
    SAL_INFO("sc.orcus.condformat", "set_xf_id");
}

void ScOrcusConditionalFormat::set_operator(os::condition_operator_t /*condition_type*/)
{
    SAL_INFO("sc.orcus.condformat", "set_operator");
}

void ScOrcusConditionalFormat::set_type(os::conditional_format_t type)
{
    switch (type)
    {
        case os::conditional_format_condition:
        case os::conditional_format_formula:
            meEntryType = condformat::CONDITION;
            // mpCurrentEntry.reset(new ScCondFormatEntry());
        break;
        case os::conditional_format_date:
        break;
        case os::conditional_format_colorscale:
        break;
        case os::conditional_format_databar:
        break;
        case os::conditional_format_iconset:
        break;
        default:
            SAL_INFO("sc.orcus.condformat", "unknown conditional_format_t value");
        break;
    }
    SAL_INFO("sc.orcus.condformat", "set_type");
}

void ScOrcusConditionalFormat::commit_entry()
{
    SAL_INFO("sc.orcus.condformat", "commit_entry");
}

void ScOrcusConditionalFormat::set_range(const char* /*p*/, size_t /*n*/)
{
    SAL_INFO("sc.orcus.condformat", "set_range");
}

void ScOrcusConditionalFormat::set_range(os::row_t row_start, os::col_t col_start,
        os::row_t row_end, os::col_t col_end)
{
    SAL_INFO("sc.orcus.condformat", "set_range");
    ScRange aRange(col_start, row_start, mnTab, col_end, row_end, mnTab);
    mpCurrentFormat->SetRange(aRange);
}

void ScOrcusConditionalFormat::commit_format()
{
    SAL_INFO("sc.orcus.condformat", "commit_format");
    mpCurrentFormat.reset(new ScConditionalFormat(0, &mrDoc));
}

ScOrcusSheet::ScOrcusSheet(ScDocumentImport& rDoc, SCTAB nTab, ScOrcusFactory& rFactory) :
    mrDoc(rDoc),
    mnTab(nTab),
    mrFactory(rFactory),
    mrStyles(static_cast<ScOrcusStyles&>(*mrFactory.get_styles())),
    maAutoFilter(rDoc.getDoc()),
    maProperties(mnTab, mrDoc),
    maConditionalFormat(mnTab, rDoc.getDoc()),
    mnCellCount(0)
{
}

void ScOrcusSheet::cellInserted()
{
    ++mnCellCount;
    if (mnCellCount == 100000)
    {
        mrFactory.incrementProgress();
        mnCellCount = 0;
    }
}

os::iface::import_table* ScOrcusSheet::get_table()
{
    return nullptr;
}

os::iface::import_sheet_properties* ScOrcusSheet::get_sheet_properties()
{
    return &maProperties;
}

os::iface::import_conditional_format* ScOrcusSheet::get_conditional_format()
{
    return &maConditionalFormat;
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
    sal_uInt32 nNanoSec = (second - nSec) * ::tools::Time::nanoSecPerSec;
    tools::Time aTime(hour, minute, nSec, nNanoSec);
    Date aNullDate(*pFormatter->GetNullDate());
    long nDateDiff = aDate - aNullDate;

    double fTime =
        static_cast<double>(aTime.GetNanoSec()) / ::tools::Time::nanoSecPerSec +
        aTime.GetSec() +
        aTime.GetMin() * ::tools::Time::secondPerMinute +
        aTime.GetHour() * ::tools::Time::secondPerHour;

    fTime /= DATE_TIME_FACTOR;

    mrDoc.setNumericCell(ScAddress(col, row, mnTab), nDateDiff + fTime);
    cellInserted();
}

void ScOrcusSheet::set_format(os::row_t /*row*/, os::col_t /*col*/, size_t xf_index)
{
    SAL_INFO("sc.orcus.style", "set format: " << xf_index);
}

void ScOrcusSheet::set_format(os::row_t row_start, os::col_t col_start,
        os::row_t row_end, os::col_t col_end, size_t xf_index)
{
    SAL_INFO("sc.orcus.style", "set format range: " << xf_index);
    ScPatternAttr aPattern(mrDoc.getDoc().GetPool());
    mrStyles.applyXfToItemSet(aPattern.GetItemSet(), xf_index);
    mrDoc.getDoc().ApplyPatternAreaTab(col_start, row_start, col_end, row_end, mnTab, aPattern);
}

namespace {

formula::FormulaGrammar::Grammar getCalcGrammarFromOrcus( os::formula_grammar_t grammar )
{
    formula::FormulaGrammar::Grammar eGrammar = formula::FormulaGrammar::GRAM_ODFF;
    switch(grammar)
    {
        case orcus::spreadsheet::formula_grammar_ods:
            eGrammar = formula::FormulaGrammar::GRAM_ODFF;
            break;
        case orcus::spreadsheet::formula_grammar_xlsx_2007:
        case orcus::spreadsheet::formula_grammar_xlsx_2010:
            eGrammar = formula::FormulaGrammar::GRAM_OOXML;
            break;
        case orcus::spreadsheet::formula_grammar_gnumeric:
            eGrammar = formula::FormulaGrammar::GRAM_ENGLISH_XL_A1;
            break;
        case orcus::spreadsheet::formula_grammar_unknown:
            break;
    }

    return eGrammar;
}

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
    pCell->SetHybridString(mrDoc.getDoc().GetSharedStringPool().intern(aResult));
}

void ScOrcusSheet::set_formula_result(os::row_t row, os::col_t col, double /*val*/)
{
    ScFormulaCell* pCell = mrDoc.getDoc().GetFormulaCell(ScAddress(col, row, mnTab));
    if (!pCell)
    {
        SAL_WARN("sc", "trying to set formula result for non formula \
                cell! Col: " << col << ";Row: " << row << ";Tab: " << mnTab);
        return;
    }

    // TODO: FIXME
}

void ScOrcusSheet::set_shared_formula(
    os::row_t row, os::col_t col, os::formula_grammar_t grammar, size_t sindex,
    const char* p_formula, size_t n_formula)
{
    ScAddress aPos(col, row, mnTab);
    OUString aFormula(p_formula, n_formula, RTL_TEXTENCODING_UTF8);
    formula::FormulaGrammar::Grammar eGram = getCalcGrammarFromOrcus(grammar);

    // Compile the formula expression into tokens.
    ScCompiler aComp(&mrDoc.getDoc(), aPos);
    aComp.SetGrammar(eGram);
    ScTokenArray* pArray = aComp.CompileString(aFormula);
    if (!pArray)
        // Tokenization failed.
        return;

    maFormulaGroups.set(sindex, pArray);

    ScFormulaCell* pCell = new ScFormulaCell(&mrDoc.getDoc(), aPos, *pArray);
    mrDoc.setFormulaCell(aPos, pCell);
    cellInserted();

    // For now, orcus doesn't support setting cached result. Mark it for re-calculation.
    pCell->SetDirty();
}

void ScOrcusSheet::set_shared_formula(
    os::row_t row, os::col_t col, os::formula_grammar_t grammar, size_t sindex,
    const char* p_formula, size_t n_formula, const char* /*p_range*/, size_t /*n_range*/)
{
    set_shared_formula(row, col, grammar, sindex, p_formula, n_formula);
}

void ScOrcusSheet::set_shared_formula(os::row_t row, os::col_t col, size_t sindex)
{
    ScAddress aPos(col, row, mnTab);

    const ScTokenArray* pArray = maFormulaGroups.get(sindex);
    if (!pArray)
        return;

    ScFormulaCell* pCell = new ScFormulaCell(&mrDoc.getDoc(), aPos, *pArray);
    mrDoc.setFormulaCell(aPos, pCell);
    cellInserted();

    // For now, orcus doesn't support setting cached result. Mark it for re-calculation.
    pCell->SetDirty();
}

void ScOrcusSheet::set_array_formula(
    os::row_t row, os::col_t col, os::formula_grammar_t grammar,
    const char* p, size_t n, os::row_t array_rows, os::col_t array_cols)
{
    formula::FormulaGrammar::Grammar eGrammar = getCalcGrammarFromOrcus( grammar );
    OUString aFormula(p, n, RTL_TEXTENCODING_UTF8);

    ScRange aRange(col, row, mnTab, col+array_cols, row + array_rows, mnTab);

    ScCompiler aComp(&mrDoc.getDoc(), aRange.aStart);
    aComp.SetGrammar(eGrammar);
    std::unique_ptr<ScTokenArray> pArray(aComp.CompileString(aFormula));
    if (!pArray)
        return;

    mrDoc.setMatrixCells(aRange, *pArray, eGrammar);
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

void ScOrcusSharedStrings::set_segment_font(size_t /*font_index*/)
{
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

void ScOrcusSharedStrings::set_segment_font_color(orcus::spreadsheet::color_elem_t,
            orcus::spreadsheet::color_elem_t,
            orcus::spreadsheet::color_elem_t,
            orcus::spreadsheet::color_elem_t)
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

ScOrcusStyles::ScOrcusStyles(ScDocument& rDoc):
    mrDoc(rDoc)
{
    mrDoc.GetStyleSheetPool()->CreateStandardStyles();
}

ScOrcusStyles::font::font():
    mbBold(false),
    mbItalic(false),
    mnSize(10),
    meUnderline(UNDERLINE_NONE)
{
}

/*
namespace {

std::ostream& operator<<(std::ostream& rStrm, const Color& rColor)
{
    rStrm << "Red: " << (int)rColor.GetRed() << ", Green: " << (int)rColor.GetGreen() << ", Blue: " << (int)rColor.GetBlue();
    return rStrm;
}

}
*/

void ScOrcusStyles::font::applyToItemSet(SfxItemSet& rSet) const
{
    FontItalic eItalic = mbItalic ? ITALIC_NORMAL : ITALIC_NONE;
    rSet.Put(SvxPostureItem(eItalic, ATTR_FONT_POSTURE));

    FontWeight eWeight = mbBold ? WEIGHT_BOLD : WEIGHT_NORMAL;
    rSet.Put(SvxWeightItem(eWeight, ATTR_FONT_WEIGHT));

    rSet.Put(SvxColorItem(maColor, ATTR_FONT_COLOR));
    rSet.Put(SvxUnderlineItem(meUnderline, ATTR_FONT_UNDERLINE));
}

void ScOrcusStyles::fill::applyToItemSet(SfxItemSet& rSet) const
{
    if (maPattern.equalsIgnoreAsciiCase("none"))
    {
        SAL_INFO("sc.orcus.style", "no fill style");
        return;
    }

    rSet.Put(SvxBrushItem(maBgColor, ATTR_BACKGROUND));
}

ScOrcusStyles::protection::protection():
    mbHidden(false),
    mbLocked(false)
{
}

void ScOrcusStyles::protection::applyToItemSet(SfxItemSet& /*rSet*/) const
{
    (void)this; // loplugin:staticmethods
}

ScOrcusStyles::border::border()
{
}

namespace {

SvxBoxItemLine getDirection(os::border_direction_t dir)
{
    switch (dir)
    {
        case os::border_right:
            return SvxBoxItemLine::RIGHT;
        break;
        case os::border_left:
            return SvxBoxItemLine::RIGHT;
        break;
        case os::border_top:
            return SvxBoxItemLine::RIGHT;
        break;
        case os::border_bottom:
            return SvxBoxItemLine::RIGHT;
        break;
        default:
        break;
    }
    return SvxBoxItemLine::RIGHT;
}

}

void ScOrcusStyles::border::applyToItemSet(SfxItemSet& rSet) const
{
    SvxBoxItem aItem(ATTR_BORDER);

    for (auto& current_border_line : border_lines)
    {
        SvxBoxItemLine eDir = getDirection(current_border_line.first);
        editeng::SvxBorderLine aLine(&current_border_line.second.maColor, 1);
        aItem.SetLine(&aLine, eDir);
    }

    rSet.Put(aItem);
}

void ScOrcusStyles::number_format::applyToItemSet(SfxItemSet& /*rSet*/) const
{
    (void)this; // loplugin:staticmethods
}

ScOrcusStyles::xf::xf():
    mnFontId(0),
    mnFillId(0),
    mnBorderId(0),
    mnProtectionId(0),
    mnNumberFormatId(0),
    mnStyleXf(0)
{
}

ScOrcusStyles::cell_style::cell_style():
    mnXFId(0),
    mnBuiltInId(0)
{
}

void ScOrcusStyles::applyXfToItemSet(SfxItemSet& rSet, const xf& rXf)
{
    size_t nFontId = rXf.mnFontId;
    if (nFontId >= maFonts.size())
    {
        SAL_WARN("sc.orcus.style", "invalid font id");
        return;
    }

    const font& rFont = maFonts[nFontId];
    rFont.applyToItemSet(rSet);

    size_t nFillId = rXf.mnFillId;
    if (nFillId >= maFills.size())
    {
        SAL_WARN("sc.orcus.style", "invalid fill id");
        return;
    }

    const fill& rFill = maFills[nFillId];
    rFill.applyToItemSet(rSet);

    size_t nBorderId = rXf.mnBorderId;
    if (nBorderId >= maBorders.size())
    {
        SAL_WARN("sc.orcus.style", "invalid border id");
        return;
    }
    const border& rBorder = maBorders[nBorderId];
    rBorder.applyToItemSet(rSet);

    size_t nProtectionId = rXf.mnProtectionId;
    if (nProtectionId >= maProtections.size())
    {
        SAL_WARN("sc.orcus.style", "invalid protection id");
        return;
    }
    const protection& rProtection = maProtections[nProtectionId];
    rProtection.applyToItemSet(rSet);

    size_t nNumberFormatId = rXf.mnNumberFormatId;
    if (nNumberFormatId >= maNumberFormats.size())
    {
        SAL_WARN("sc.orcus.style", "invalid number format id");
        return;
    }
    const number_format& rFormat = maNumberFormats[nNumberFormatId];
    rFormat.applyToItemSet(rSet);
}

bool ScOrcusStyles::applyXfToItemSet(SfxItemSet& rSet, size_t xfId)
{
    SAL_INFO("sc.orcus.style", "applyXfToitemSet: " << xfId);
    if (maCellXfs.size() <= xfId)
    {
        SAL_WARN("sc.orcus.style", "invalid xf id");
        return false;
    }

    const xf& rXf = maCellXfs[xfId];
    applyXfToItemSet(rSet, rXf);
    return true;
}

void ScOrcusStyles::set_font_count(size_t /*n*/)
{
    // needed at all?
}

void ScOrcusStyles::set_font_bold(bool b)
{
    maCurrentFont.mbBold = b;
}

void ScOrcusStyles::set_font_italic(bool b)
{
    maCurrentFont.mbItalic = b;
}

void ScOrcusStyles::set_font_name(const char* s, size_t n)
{
    OUString aName(s, n, RTL_TEXTENCODING_UTF8);
    maCurrentFont.maName = aName;
}

void ScOrcusStyles::set_font_size(double point)
{
    maCurrentFont.mnSize = point;
}

void ScOrcusStyles::set_font_underline(orcus::spreadsheet::underline_t e)
{
    switch(e)
    {
        case orcus::spreadsheet::underline_single:
        case orcus::spreadsheet::underline_single_accounting:
            maCurrentFont.meUnderline = UNDERLINE_SINGLE;
            break;
        case orcus::spreadsheet::underline_double:
        case orcus::spreadsheet::underline_double_accounting:
            maCurrentFont.meUnderline = UNDERLINE_DOUBLE;
            break;
        case orcus::spreadsheet::underline_none:
            maCurrentFont.meUnderline = UNDERLINE_NONE;
            break;
    }
}

void ScOrcusStyles::set_font_color(orcus::spreadsheet::color_elem_t alpha,
            orcus::spreadsheet::color_elem_t red,
            orcus::spreadsheet::color_elem_t green,
            orcus::spreadsheet::color_elem_t blue)
{
    maCurrentFont.maColor = Color(alpha, red, green, blue);
}

size_t ScOrcusStyles::commit_font()
{
    SAL_INFO("sc.orcus.style", "commit font");
    maFonts.push_back(maCurrentFont);
    maCurrentFont = ScOrcusStyles::font();
    return maFonts.size() - 1;
}

// fill

void ScOrcusStyles::set_fill_count(size_t /*n*/)
{
    // needed at all?
}

void ScOrcusStyles::set_fill_pattern_type(const char* s, size_t n)
{
    maCurrentFill.maPattern = OUString(s, n, RTL_TEXTENCODING_UTF8);
}

void ScOrcusStyles::set_fill_fg_color(orcus::spreadsheet::color_elem_t alpha, orcus::spreadsheet::color_elem_t red, orcus::spreadsheet::color_elem_t green, orcus::spreadsheet::color_elem_t blue)
{
    maCurrentFill.maFgColor = Color(alpha, red, green, blue);
}

void ScOrcusStyles::set_fill_bg_color(orcus::spreadsheet::color_elem_t alpha, orcus::spreadsheet::color_elem_t red, orcus::spreadsheet::color_elem_t green, orcus::spreadsheet::color_elem_t blue)
{
    maCurrentFill.maBgColor = Color(alpha, red, green, blue);
}

size_t ScOrcusStyles::commit_fill()
{
    SAL_INFO("sc.orcus.style", "commit fill");
    maFills.push_back(maCurrentFill);
    maCurrentFill = ScOrcusStyles::fill();
    return maFills.size() - 1;
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

void ScOrcusStyles::set_border_color(orcus::spreadsheet::border_direction_t dir,
            orcus::spreadsheet::color_elem_t alpha,
            orcus::spreadsheet::color_elem_t red,
            orcus::spreadsheet::color_elem_t green,
            orcus::spreadsheet::color_elem_t blue)
{
    border::border_line& current_line = maCurrentBorder.border_lines[dir];
    current_line.maColor = Color(alpha, red, green, blue);
}

size_t ScOrcusStyles::commit_border()
{
    SAL_INFO("sc.orcus.style", "commit border");
    maBorders.push_back(maCurrentBorder);
    maCurrentBorder = ScOrcusStyles::border();
    return maBorders.size() - 1;
}

// cell protection
void ScOrcusStyles::set_cell_hidden(bool b)
{
    maCurrentProtection.mbHidden = b;
}

void ScOrcusStyles::set_cell_locked(bool b)
{
    maCurrentProtection.mbLocked = b;
}

size_t ScOrcusStyles::commit_cell_protection()
{
    SAL_INFO("sc.orcus.style", "commit cell protection");
    maProtections.push_back(maCurrentProtection);
    maCurrentProtection = ScOrcusStyles::protection();
    return maProtections.size() - 1;
}

void ScOrcusStyles::set_number_format_count(size_t)
{
}

void ScOrcusStyles::set_number_format_identifier(size_t)
{
}

void ScOrcusStyles::set_number_format_code(const char* s, size_t n)
{
    OUString aCode(s, n, RTL_TEXTENCODING_UTF8);
    maCurrentNumberFormat.maCode = aCode;
}

size_t ScOrcusStyles::commit_number_format()
{
    SAL_INFO("sc.orcus.style", "commit number format");
    maNumberFormats.push_back(maCurrentNumberFormat);
    maCurrentNumberFormat = ScOrcusStyles::number_format();
    return maNumberFormats.size() - 1;
}

// cell style xf

void ScOrcusStyles::set_cell_style_xf_count(size_t /*n*/)
{
    // needed at all?
}

size_t ScOrcusStyles::commit_cell_style_xf()
{
    SAL_INFO("sc.orcus.style", "commit cell style xf");
    maCellStyleXfs.push_back(maCurrentXF);
    return maCellStyleXfs.size() - 1;
}

// cell xf

void ScOrcusStyles::set_cell_xf_count(size_t /*n*/)
{
    // needed at all?
}

size_t ScOrcusStyles::commit_cell_xf()
{
    SAL_INFO("sc.orcus.style", "commit cell xf");
    maCellXfs.push_back(maCurrentXF);
    return maCellXfs.size() - 1;
}

// dxf

void ScOrcusStyles::set_dxf_count(size_t /*n*/)
{
}

size_t ScOrcusStyles::commit_dxf()
{
    return 0;
}

// xf (cell format) - used both by cell xf and cell style xf.

void ScOrcusStyles::set_xf_number_format(size_t index)
{
    maCurrentXF.mnNumberFormatId = index;
}

void ScOrcusStyles::set_xf_font(size_t index)
{
    maCurrentXF.mnFontId = index;
}

void ScOrcusStyles::set_xf_fill(size_t index)
{
    maCurrentXF.mnFillId = index;
}

void ScOrcusStyles::set_xf_border(size_t index)
{
    maCurrentXF.mnBorderId = index;
}

void ScOrcusStyles::set_xf_protection(size_t index)
{
    maCurrentXF.mnProtectionId = index;
}

void ScOrcusStyles::set_xf_style_xf(size_t index)
{
    maCurrentXF.mnStyleXf = index;
}

void ScOrcusStyles::set_xf_apply_alignment(bool /*b*/)
{
}

void ScOrcusStyles::set_xf_horizontal_alignment(orcus::spreadsheet::hor_alignment_t /*align*/)
{
}

void ScOrcusStyles::set_xf_vertical_alignment(orcus::spreadsheet::ver_alignment_t /*align*/)
{
}

// cell style entry
// not needed for now for gnumeric

void ScOrcusStyles::set_cell_style_count(size_t /*n*/)
{
    // needed at all?
}

void ScOrcusStyles::set_cell_style_name(const char* s, size_t n)
{
    OUString aName(s, n, RTL_TEXTENCODING_UTF8);
    maCurrentCellStyle.maName = aName;
}

void ScOrcusStyles::set_cell_style_xf(size_t index)
{
    maCurrentCellStyle.mnXFId = index;
}

void ScOrcusStyles::set_cell_style_builtin(size_t index)
{
    // not needed for gnumeric
    maCurrentCellStyle.mnBuiltInId = index;
}

size_t ScOrcusStyles::commit_cell_style()
{
    SAL_INFO("sc.orcus.style", "commit cell style: " << maCurrentCellStyle.maName);
    if (maCurrentCellStyle.mnXFId >= maCellStyleXfs.size())
    {
        SAL_WARN("sc.orcus.style", "invalid xf id for commit cell style");
        return 0;
    }

    ScStyleSheetPool* pPool = mrDoc.GetStyleSheetPool();
    SfxStyleSheetBase& rBase = pPool->Make(maCurrentCellStyle.maName, SFX_STYLE_FAMILY_PARA);
    SfxItemSet& rSet = rBase.GetItemSet();

    xf& rXf = maCellStyleXfs[maCurrentCellStyle.mnXFId];
    applyXfToItemSet(rSet, rXf);

    return 0;
}

// auto filter import

ScOrcusAutoFilter::ScOrcusAutoFilter(ScDocument& rDoc):
    mrDoc(rDoc)
{
    (void)mrDoc;
}

ScOrcusAutoFilter::~ScOrcusAutoFilter()
{
}

void ScOrcusAutoFilter::set_range(const char* p_ref, size_t n_ref)
{
    OUString aRange(p_ref, n_ref, RTL_TEXTENCODING_UTF8);
    SAL_INFO("sc.orcus.autofilter", "set_range: " << aRange);

    maRange.Parse(aRange);
}

void ScOrcusAutoFilter::set_column(orcus::spreadsheet::col_t col)
{
    SAL_INFO("sc.orcus.autofilter", "set_column: " << col);
}

void ScOrcusAutoFilter::append_column_match_value(const char* p, size_t n)
{
    OUString aString(p, n, RTL_TEXTENCODING_UTF8);
    SAL_INFO("sc.orcus.autofilter", "append_column_match_value: " << aString);
}

void ScOrcusAutoFilter::commit_column()
{
    SAL_INFO("sc.orcus.autofilter", "commit column");
}

void ScOrcusAutoFilter::commit()
{
    SAL_INFO("sc.orcus.autofilter", "commit");
}

ScOrcusTable::ScOrcusTable(SCTAB nTab, ScDocument& rDoc):
    mnTab(nTab),
    mrDoc(rDoc),
    maAutoFilter(rDoc)
{
}

ScOrcusTable::~ScOrcusTable()
{
}

os::iface::import_auto_filter* ScOrcusTable::get_auto_filter()
{
    return &maAutoFilter;
}

void ScOrcusTable::set_identifier(size_t id)
{
    SAL_INFO("sc.orcus.table", "set_identifier :" << id);
}

namespace {

std::ostream& operator<<(std::ostream& rStrm, const ScAddress& rAddr)
{
    rStrm << "Col: " << rAddr.Col() << ", Row: " << rAddr.Row() << ", Tab: " << rAddr.Tab();
    return rStrm;
}

std::ostream& operator<<(std::ostream& rStrm, const ScRange& rRange)
{
    rStrm << "aStart: " << rRange.aStart << std::endl;
    rStrm << "aEnd: " << rRange.aEnd;
    return rStrm;
}

}

void ScOrcusTable::set_range(const char* p_ref, size_t n_ref)
{
    OUString aRange(p_ref, n_ref, RTL_TEXTENCODING_UTF8);
    SAL_INFO("sc.orcus.table", "set_range: " << aRange);

    maRange.Parse(aRange);
    SAL_INFO("sc.orcus.table", "set_range translated range: " << maRange);
}

void ScOrcusTable::set_totals_row_count(size_t row_count)
{
    SAL_INFO("sc.orcus.table", "set_totals_row_count: " << row_count);
}

void ScOrcusTable::set_name(const char* p, size_t n)
{
    maName = OUString(p, n, RTL_TEXTENCODING_UTF8);
    SAL_INFO("sc.orcus.table", "set_name: " << maName);
}

void ScOrcusTable::set_display_name(const char* p, size_t n)
{
    OUString aName(p, n, RTL_TEXTENCODING_UTF8);
    SAL_INFO("sc.orcus.table", "set_display_name: " << aName);
}

void ScOrcusTable::set_column_count(size_t n)
{
    SAL_INFO("sc.orcus.table", "set_column_count: " << n);
}

void ScOrcusTable::set_column_identifier(size_t id)
{
    SAL_INFO("sc.orcus.table", "set_column_identifier: " << id);
}

void ScOrcusTable::set_column_name(const char* p, size_t n)
{
    OUString aName(p, n, RTL_TEXTENCODING_UTF8);
    SAL_INFO("sc.orcus.table", "set_column_name: " << aName);
}

void ScOrcusTable::set_column_totals_row_label(const char* p, size_t n)
{
    OUString aLabel(p, n, RTL_TEXTENCODING_UTF8);
    SAL_INFO("sc.orcus.table", "set_column_totals_row_label: " << aLabel);
}

void ScOrcusTable::set_column_totals_row_function(os::totals_row_function_t )
{
    SAL_INFO("sc.orcus.table", "set_column_totals_row_function");
}

void ScOrcusTable::commit_column()
{
    SAL_INFO("sc.orcus.table", "commit_column");
}

void ScOrcusTable::set_style_name(const char* p, size_t n)
{
    OUString aName(p, n, RTL_TEXTENCODING_UTF8);
    SAL_INFO("sc.orcus.table", "set_style_name: " << aName);
}

void ScOrcusTable::set_style_show_first_column(bool b)
{
    SAL_INFO("sc.orcus.table", "set_style_show_first_column: " << b);
}

void ScOrcusTable::set_style_show_last_column(bool b)
{
    SAL_INFO("sc.orcus.table", "set_style_show_last_column: " << b);
}

void ScOrcusTable::set_style_show_row_stripes(bool b)
{
    SAL_INFO("sc.orcus.table", "set_style_show_row_stripes: " << b);
}

void ScOrcusTable::set_style_show_column_stripes(bool b)
{
    SAL_INFO("sc.orcus.table", "set_style_show_column_stripes: " << b);
}

void ScOrcusTable::commit()
{
    SAL_INFO("sc.orcus.table", "commit");

    ScDBData* pDBData = new ScDBData(maName, mnTab,
                        maRange.aStart.Col(), maRange.aStart.Row(),
                        maRange.aEnd.Col(), maRange.aEnd.Row());

    if(!mrDoc.GetDBCollection()->getNamedDBs().insert(pDBData))
        delete pDBData;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
