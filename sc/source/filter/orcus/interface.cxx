/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <orcusinterface.hxx>

#include <document.hxx>
#include <formulacell.hxx>
#include <rangenam.hxx>
#include <tokenarray.hxx>
#include <globalnames.hxx>
#include <docoptio.hxx>
#include <globstr.hrc>
#include <compiler.hxx>
#include <dbdata.hxx>
#include <stlpool.hxx>
#include <scitems.hxx>
#include <patattr.hxx>
#include <docpool.hxx>
#include <attrib.hxx>

#include <editeng/postitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/borderline.hxx>
#include <editeng/charhiddenitem.hxx>
#include <editeng/protitem.hxx>
#include <editeng/prntitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/lineitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/justifyitem.hxx>

#include <formula/token.hxx>
#include <tools/datetime.hxx>
#include <svl/sharedstringpool.hxx>
#include <o3tl/make_unique.hxx>
#include <svl/zforlist.hxx>
#include <svl/intitem.hxx>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <comphelper/processfactory.hxx>
#include <i18nlangtag/lang.h>
#include <vcl/outdev.hxx>
#include <tools/colordata.hxx>
#include <tools/fontenum.hxx>
#include <iostream>

using namespace com::sun::star;

namespace os = orcus::spreadsheet;

namespace {

formula::FormulaGrammar::Grammar getCalcGrammarFromOrcus( os::formula_grammar_t grammar )
{
    formula::FormulaGrammar::Grammar eGrammar = formula::FormulaGrammar::GRAM_ODFF;
    switch(grammar)
    {
        case orcus::spreadsheet::formula_grammar_t::ods:
            eGrammar = formula::FormulaGrammar::GRAM_ODFF;
            break;
        case orcus::spreadsheet::formula_grammar_t::xlsx_2007:
        case orcus::spreadsheet::formula_grammar_t::xlsx_2010:
            eGrammar = formula::FormulaGrammar::GRAM_OOXML;
            break;
        case orcus::spreadsheet::formula_grammar_t::gnumeric:
            eGrammar = formula::FormulaGrammar::GRAM_ENGLISH_XL_A1;
            break;
        case orcus::spreadsheet::formula_grammar_t::xls_xml:
            eGrammar = formula::FormulaGrammar::GRAM_ENGLISH_XL_R1C1;
            break;
        case orcus::spreadsheet::formula_grammar_t::unknown:
            break;
    }

    return eGrammar;
}

}

ScOrcusGlobalSettings::ScOrcusGlobalSettings(ScDocumentImport& rDoc) :
    mrDoc(rDoc), meOrcusGrammar(os::formula_grammar_t::unknown) {}

void ScOrcusGlobalSettings::set_origin_date(int year, int month, int day)
{
    mrDoc.setOriginDate(year, month, day);
}

void ScOrcusGlobalSettings::set_default_formula_grammar(os::formula_grammar_t grammar)
{
    meCalcGrammar = getCalcGrammarFromOrcus(grammar);
    meOrcusGrammar = grammar;
}

orcus::spreadsheet::formula_grammar_t ScOrcusGlobalSettings::get_default_formula_grammar() const
{
    return meOrcusGrammar;
}

ScOrcusRefResolver::ScOrcusRefResolver( const ScDocumentImport& rDoc, const ScOrcusGlobalSettings& rGS ) :
    mrDoc(rDoc), mrGlobalSettings(rGS) {}

os::address_t ScOrcusRefResolver::resolve_address(const char* p, size_t n)
{
    OUString aStr(p, n, RTL_TEXTENCODING_UTF8);

    ScAddress aAddr;
    aAddr.Parse(aStr, nullptr,
        formula::FormulaGrammar::extractRefConvention(
            mrGlobalSettings.getCalcGrammar()));

    os::address_t ret;

    if (aAddr.IsValid())
    {
        ret.column = aAddr.Col();
        ret.row = aAddr.Row();
    }

    return ret;
}

os::range_t ScOrcusRefResolver::resolve_range(const char* p, size_t n)
{
    OUString aStr(p, n, RTL_TEXTENCODING_UTF8);

    ScRange aRange;
    aRange.Parse(aStr, nullptr,
        formula::FormulaGrammar::extractRefConvention(
            mrGlobalSettings.getCalcGrammar()));

    os::range_t ret;

    if (aRange.IsValid())
    {
        ret.first.column = aRange.aStart.Col();
        ret.first.row    = aRange.aStart.Row();
        ret.last.column  = aRange.aEnd.Col();
        ret.last.row     = aRange.aEnd.Row();
    }

    return ret;
}

ScOrcusNamedExpression::ScOrcusNamedExpression(
    ScDocumentImport& rDoc, const ScOrcusGlobalSettings& rGS ) :
    mrDoc(rDoc), mrGlobalSettings(rGS) {}

void ScOrcusNamedExpression::define_name(const char* p_name, size_t n_name, const char* p_exp, size_t n_exp)
{
    OUString aName(p_name, n_name, RTL_TEXTENCODING_UTF8);
    OUString aExpr(p_exp, n_exp, RTL_TEXTENCODING_UTF8);

    ScRangeName* pNames = mrDoc.getDoc().GetRangeName();
    if (!pNames)
        return;

    ScRangeData* pRange = new ScRangeData(
        &mrDoc.getDoc(), aName, aExpr, ScAddress(), ScRangeData::Type::Name,
        mrGlobalSettings.getCalcGrammar());

    pNames->insert(pRange, false);
}

ScOrcusFactory::CellStoreToken::CellStoreToken( const ScAddress& rPos, Type eType ) :
    maPos(rPos), meType(eType)
{
    rtl::math::setNan(&mfValue);
}

ScOrcusFactory::CellStoreToken::CellStoreToken( const ScAddress& rPos, double fValue ) :
    maPos(rPos), meType(Type::Numeric), mfValue(fValue) {}

ScOrcusFactory::CellStoreToken::CellStoreToken( const ScAddress& rPos, uint32_t nIndex ) :
    maPos(rPos), meType(Type::String), mnIndex1(nIndex)
{
    rtl::math::setNan(&mfValue);
}

ScOrcusFactory::CellStoreToken::CellStoreToken(
    const ScAddress& rPos, const OUString& rFormula, formula::FormulaGrammar::Grammar eGrammar ) :
    maPos(rPos), meType(Type::Formula), maStr1(rFormula), meGrammar(eGrammar)
{
    rtl::math::setNan(&mfValue);
}

ScOrcusFactory::ScOrcusFactory(ScDocument& rDoc) :
    maDoc(rDoc),
    maGlobalSettings(maDoc),
    maRefResolver(maDoc, maGlobalSettings),
    maSharedStrings(*this),
    maNamedExpressions(maDoc, maGlobalSettings),
    maStyles(rDoc),
    mnProgress(0) {}

orcus::spreadsheet::iface::import_sheet* ScOrcusFactory::append_sheet(
    orcus::spreadsheet::sheet_t sheet_index, const char* sheet_name, size_t sheet_name_length)
{
    OUString aTabName(sheet_name, sheet_name_length, RTL_TEXTENCODING_UTF8);

    if (sheet_index == 0)
    {
        // The calc document initializes with one sheet already present.
        assert(maDoc.getSheetCount() == 1);
        maDoc.setSheetName(0, aTabName);
        maSheets.push_back(o3tl::make_unique<ScOrcusSheet>(maDoc, 0, *this));
        return maSheets.back().get();
    }

    if (!maDoc.appendSheet(aTabName))
        return nullptr;

    SCTAB nTab = maDoc.getSheetCount() - 1;
    maSheets.push_back(o3tl::make_unique<ScOrcusSheet>(maDoc, nTab, *this));
    return maSheets.back().get();
}

class FindSheetByIndex
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

orcus::spreadsheet::iface::import_named_expression* ScOrcusFactory::get_named_expression()
{
    return &maNamedExpressions;
}

orcus::spreadsheet::iface::import_styles* ScOrcusFactory::get_styles()
{
    return &maStyles;
}

void ScOrcusFactory::finalize()
{
    auto toFormulaCell = [this]( const CellStoreToken& rToken ) -> std::unique_ptr<ScFormulaCell>
    {
        const ScOrcusSheet& rSheet = *maSheets.at(rToken.maPos.Tab());
        const sc::SharedFormulaGroups& rSFG = rSheet.getSharedFormulaGroups();
        const ScTokenArray* pArray = rSFG.get(rToken.mnIndex1);
        if (!pArray)
            return std::unique_ptr<ScFormulaCell>();

        return o3tl::make_unique<ScFormulaCell>(&maDoc.getDoc(), rToken.maPos, *pArray);
    };

    int nCellCount = 0;

    for (const CellStoreToken& rToken : maCellStoreTokens)
    {
        switch (rToken.meType)
        {
            case CellStoreToken::Type::Auto:
            {
                maDoc.setAutoInput(rToken.maPos, rToken.maStr1);
                ++nCellCount;
                break;
            }
            case CellStoreToken::Type::String:
            {
                if (rToken.mnIndex1 >= maStrings.size())
                    // String index out-of-bound!  Something is up.
                    break;

                maDoc.setStringCell(rToken.maPos, maStrings[rToken.mnIndex1]);
                ++nCellCount;
                break;
            }
            case CellStoreToken::Type::Numeric:
            {
                maDoc.setNumericCell(rToken.maPos, rToken.mfValue);
                ++nCellCount;
                break;
            }
            case CellStoreToken::Type::Formula:
            {
                maDoc.setFormulaCell(
                    rToken.maPos, rToken.maStr1, rToken.meGrammar);

                ++nCellCount;
                break;
            }
            case CellStoreToken::Type::FormulaWithResult:
            {
                if (rtl::math::isFinite(rToken.mfValue))
                    maDoc.setFormulaCell(rToken.maPos, rToken.maStr1, rToken.meGrammar, &rToken.mfValue);
                else
                    maDoc.setFormulaCell(rToken.maPos, rToken.maStr1, rToken.meGrammar, rToken.maStr2);

                ++nCellCount;
                break;
            }
            case CellStoreToken::Type::SharedFormula:
            {
                std::unique_ptr<ScFormulaCell> pCell = toFormulaCell(rToken);
                if (!pCell)
                    break;

                maDoc.setFormulaCell(rToken.maPos, pCell.release());

                ++nCellCount;
                break;
            }
            case CellStoreToken::Type::SharedFormulaWithResult:
            {
                std::unique_ptr<ScFormulaCell> pCell = toFormulaCell(rToken);
                if (!pCell)
                    break;

                if (rtl::math::isFinite(rToken.mfValue))
                    pCell->SetResultDouble(rToken.mfValue);
                else
                    pCell->SetHybridString(
                        maDoc.getDoc().GetSharedStringPool().intern(rToken.maStr2));

                maDoc.setFormulaCell(rToken.maPos, pCell.release());

                ++nCellCount;
                break;
            }
            case CellStoreToken::Type::Matrix:
            {
                if (!rToken.mnIndex1 || !rToken.mnIndex2)
                    break;

                ScRange aRange(rToken.maPos);
                aRange.aEnd.IncCol(rToken.mnIndex1-1);
                aRange.aEnd.IncRow(rToken.mnIndex2-1);

                ScCompiler aComp(&maDoc.getDoc(), aRange.aStart, rToken.meGrammar);
                std::unique_ptr<ScTokenArray> pArray(aComp.CompileString(rToken.maStr1));
                if (!pArray)
                    break;

                maDoc.setMatrixCells(aRange, *pArray, rToken.meGrammar);
                break;
            }
            default:
                ;
        }

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
    maStringHash.emplace(rStr, nPos);

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

void ScOrcusFactory::pushCellStoreAutoToken( const ScAddress& rPos, const OUString& rVal )
{
    maCellStoreTokens.emplace_back(rPos, CellStoreToken::Type::Auto);
    maCellStoreTokens.back().maStr1 = rVal;
}

void ScOrcusFactory::pushCellStoreToken( const ScAddress& rPos, uint32_t nStrIndex )
{
    maCellStoreTokens.emplace_back(rPos, nStrIndex);
}

void ScOrcusFactory::pushCellStoreToken( const ScAddress& rPos, double fValue )
{
    maCellStoreTokens.emplace_back(rPos, fValue);
}

void ScOrcusFactory::pushCellStoreToken(
    const ScAddress& rPos, const OUString& rFormula, formula::FormulaGrammar::Grammar eGrammar )
{
    maCellStoreTokens.emplace_back(rPos, rFormula, eGrammar);
}

void ScOrcusFactory::pushSharedFormulaToken( const ScAddress& rPos, uint32_t nIndex )
{
    maCellStoreTokens.emplace_back(rPos, CellStoreToken::Type::SharedFormula);
    maCellStoreTokens.back().mnIndex1 = nIndex;
}

void ScOrcusFactory::pushMatrixFormulaToken(
    const ScAddress& rPos, const OUString& rFormula, formula::FormulaGrammar::Grammar eGrammar,
    uint32_t nRowRange, uint32_t nColRange )
{
    maCellStoreTokens.emplace_back(rPos, CellStoreToken::Type::Matrix);
    CellStoreToken& rT = maCellStoreTokens.back();
    rT.maStr1 = rFormula;
    rT.meGrammar = eGrammar;
    rT.mnIndex1 = nColRange;
    rT.mnIndex2 = nRowRange;
}

void ScOrcusFactory::pushFormulaResult( const ScAddress& rPos, double fValue )
{
    // Formula result is expected to be pushed immediately following the
    // formula token it belongs.
    if (maCellStoreTokens.empty())
        return;

    CellStoreToken& rToken = maCellStoreTokens.back();
    if (rToken.maPos != rPos)
        return;

    switch (rToken.meType)
    {
        case CellStoreToken::Type::Formula:
            rToken.meType = CellStoreToken::Type::FormulaWithResult;
            break;
        case CellStoreToken::Type::SharedFormula:
            rToken.meType = CellStoreToken::Type::SharedFormulaWithResult;
            break;
        default:
            return;
    }

    rToken.mfValue = fValue;
}

void ScOrcusFactory::pushFormulaResult( const ScAddress& rPos, const OUString& rValue )
{
    // Formula result is expected to be pushed immediately following the
    // formula token it belongs.
    if (maCellStoreTokens.empty())
        return;

    CellStoreToken& rToken = maCellStoreTokens.back();
    if (rToken.maPos != rPos)
        return;

    switch (rToken.meType)
    {
        case CellStoreToken::Type::Formula:
            rToken.meType = CellStoreToken::Type::FormulaWithResult;
            break;
        case CellStoreToken::Type::SharedFormula:
            rToken.meType = CellStoreToken::Type::SharedFormulaWithResult;
            break;
        default:
            return;
    }

    rToken.maStr2 = rValue;
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
        case orcus::length_unit_t::inch:
            return nVal * 72.0 * 20.0;
            break;
        case orcus::length_unit_t::twip:
            return nVal;
            break;
        case orcus::length_unit_t::point:
            return nVal * 20.0;
            break;
        case orcus::length_unit_t::centimeter:
            return nVal * 20.0 * 72.0 / 2.54;
            break;
        case orcus::length_unit_t::unknown:
            if (nVal != 0)
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

void ScOrcusSheetProperties::set_merge_cell_range(const orcus::spreadsheet::range_t& range)
{
    mrDoc.setMergedCells(mnTab, range.first.column, range.first.row, range.last.column, range.last.row);
}

ScOrcusConditionalFormat::ScOrcusConditionalFormat(SCTAB nTab, ScDocument& rDoc):
    mnTab(nTab),
    mrDoc(rDoc),
    mpCurrentFormat(new ScConditionalFormat(0, &mrDoc)),
    meEntryType(ScFormatEntry::Type::Condition)
{
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
    assert(meEntryType == ScFormatEntry::Type::Condition);
    SAL_INFO("sc.orcus.condformat", "set_condition_type");
}

void ScOrcusConditionalFormat::set_formula(const char* /*p*/, size_t /*n*/)
{
    SAL_INFO("sc.orcus.condformat", "set_formula");
}

void ScOrcusConditionalFormat::set_date(os::condition_date_t /*date*/)
{
    assert(meEntryType == ScFormatEntry::Type::Date);
    SAL_INFO("sc.orcus.condformat", "set_date");
}

void ScOrcusConditionalFormat::commit_condition()
{
    SAL_INFO("sc.orcus.condformat", "commit_condition");
}

void ScOrcusConditionalFormat::set_icon_name(const char* /*p*/, size_t /*n*/)
{
    assert(meEntryType == ScFormatEntry::Type::Iconset);
    SAL_INFO("sc.orcus.condformat", "set_icon_name");
}

void ScOrcusConditionalFormat::set_databar_gradient(bool /*gradient*/)
{
    assert(meEntryType == ScFormatEntry::Type::Databar);
    SAL_INFO("sc.orcus.condformat", "set_databar_gradient");
}

void ScOrcusConditionalFormat::set_databar_axis(os::databar_axis_t /*axis*/)
{
    assert(meEntryType == ScFormatEntry::Type::Databar);
    SAL_INFO("sc.orcus.condformat", "set_databar_axis");
}

void ScOrcusConditionalFormat::set_databar_color_positive(os::color_elem_t /*alpha*/, os::color_elem_t /*red*/,
        os::color_elem_t /*green*/, os::color_elem_t /*blue*/)
{
    assert(meEntryType == ScFormatEntry::Type::Databar);
    SAL_INFO("sc.orcus.condformat", "set_databar_color_positive");
}

void ScOrcusConditionalFormat::set_databar_color_negative(os::color_elem_t /*alpha*/, os::color_elem_t /*red*/,
        os::color_elem_t /*green*/, os::color_elem_t /*blue*/)
{
    assert(meEntryType == ScFormatEntry::Type::Databar);
    SAL_INFO("sc.orcus.condformat", "set_databar_color_negative");
}

void ScOrcusConditionalFormat::set_min_databar_length(double /*length*/)
{
    assert(meEntryType == ScFormatEntry::Type::Databar);
    SAL_INFO("sc.orcus.condformat", "set_min_databar_length");
}

void ScOrcusConditionalFormat::set_max_databar_length(double /*length*/)
{
    assert(meEntryType == ScFormatEntry::Type::Databar);
    SAL_INFO("sc.orcus.condformat", "set_max_databar_length");
}

void ScOrcusConditionalFormat::set_show_value(bool /*show*/)
{
    SAL_INFO("sc.orcus.condformat", "set_show_value");
}

void ScOrcusConditionalFormat::set_iconset_reverse(bool /*reverse*/)
{
    assert(meEntryType == ScFormatEntry::Type::Iconset);
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
        case os::conditional_format_t::condition:
        case os::conditional_format_t::formula:
            meEntryType = ScFormatEntry::Type::Condition;
            // mpCurrentEntry.reset(new ScCondFormatEntry());
        break;
        case os::conditional_format_t::date:
        break;
        case os::conditional_format_t::colorscale:
        break;
        case os::conditional_format_t::databar:
        break;
        case os::conditional_format_t::iconset:
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
    maAutoFilter(),
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

os::iface::import_auto_filter* ScOrcusSheet::get_auto_filter()
{
    return &maAutoFilter;
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
    mrFactory.pushCellStoreAutoToken(ScAddress(col, row, mnTab), aVal);
    cellInserted();
}

void ScOrcusSheet::set_string(os::row_t row, os::col_t col, size_t sindex)
{
    mrFactory.pushCellStoreToken(ScAddress(col, row, mnTab), uint32_t(sindex));
    cellInserted();
}

void ScOrcusSheet::set_value(os::row_t row, os::col_t col, double value)
{
    mrFactory.pushCellStoreToken(ScAddress(col, row, mnTab), value);
    cellInserted();
}

void ScOrcusSheet::set_bool(os::row_t row, os::col_t col, bool value)
{
    mrFactory.pushCellStoreToken(ScAddress(col, row, mnTab), value ? 1.0 : 0.0);
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
    long nDateDiff = aDate - pFormatter->GetNullDate();

    double fTime =
        static_cast<double>(aTime.GetNanoSec()) / ::tools::Time::nanoSecPerSec +
        aTime.GetSec() +
        aTime.GetMin() * ::tools::Time::secondPerMinute +
        aTime.GetHour() * ::tools::Time::secondPerHour;

    fTime /= DATE_TIME_FACTOR;

    mrFactory.pushCellStoreToken(ScAddress(col, row, mnTab), nDateDiff + fTime);
    cellInserted();
}

void ScOrcusSheet::set_format(os::row_t row, os::col_t col, size_t xf_index)
{
    SAL_INFO("sc.orcus.style", "set format: " << xf_index);

    ScPatternAttr aPattern(mrDoc.getDoc().GetPool());
    mrStyles.applyXfToItemSet(aPattern.GetItemSet(), xf_index);
    mrDoc.getDoc().ApplyPattern(col, row, mnTab, aPattern);
}

void ScOrcusSheet::set_format(os::row_t row_start, os::col_t col_start,
        os::row_t row_end, os::col_t col_end, size_t xf_index)
{
    SAL_INFO("sc.orcus.style", "set format range: " << xf_index);
    ScPatternAttr aPattern(mrDoc.getDoc().GetPool());
    mrStyles.applyXfToItemSet(aPattern.GetItemSet(), xf_index);
    mrDoc.getDoc().ApplyPatternAreaTab(col_start, row_start, col_end, row_end, mnTab, aPattern);
}

void ScOrcusSheet::set_formula(
    os::row_t row, os::col_t col, os::formula_grammar_t grammar, const char* p, size_t n)
{
    OUString aFormula(p, n, RTL_TEXTENCODING_UTF8);
    mrFactory.pushCellStoreToken(
        ScAddress(col, row, mnTab), aFormula, getCalcGrammarFromOrcus(grammar));
    cellInserted();
}

void ScOrcusSheet::set_formula_result(os::row_t row, os::col_t col, const char* p, size_t n)
{
    OUString aResult( p, n, RTL_TEXTENCODING_UTF8);
    mrFactory.pushFormulaResult(ScAddress(col, row, mnTab), aResult);
}

void ScOrcusSheet::set_formula_result(os::row_t row, os::col_t col, double val)
{
    mrFactory.pushFormulaResult(ScAddress(col, row, mnTab), val);
}

void ScOrcusSheet::set_shared_formula(
    os::row_t row, os::col_t col, os::formula_grammar_t grammar, size_t sindex,
    const char* p_formula, size_t n_formula)
{
    ScAddress aPos(col, row, mnTab);
    OUString aFormula(p_formula, n_formula, RTL_TEXTENCODING_UTF8);
    formula::FormulaGrammar::Grammar eGram = getCalcGrammarFromOrcus(grammar);

    // Compile the formula expression into tokens.
    ScCompiler aComp(&mrDoc.getDoc(), aPos, eGram);
    ScTokenArray* pArray = aComp.CompileString(aFormula);
    if (!pArray)
        // Tokenization failed.
        return;

    maFormulaGroups.set(sindex, pArray);

    mrFactory.pushSharedFormulaToken(aPos, sindex);
    cellInserted();
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

    mrFactory.pushSharedFormulaToken(aPos, sindex);
    cellInserted();
}

void ScOrcusSheet::set_array_formula(
    os::row_t row, os::col_t col, os::formula_grammar_t grammar,
    const char* p, size_t n, os::row_t array_rows, os::col_t array_cols)
{
    OUString aFormula(p, n, RTL_TEXTENCODING_UTF8);
    formula::FormulaGrammar::Grammar eGrammar = getCalcGrammarFromOrcus(grammar);

    ScAddress aPos(col, row, mnTab);
    mrFactory.pushMatrixFormulaToken(aPos, aFormula, eGrammar, array_rows, array_cols);
}

void ScOrcusSheet::set_array_formula(
    os::row_t /*row*/, os::col_t /*col*/, os::formula_grammar_t /*grammar*/,
    const char* /*p*/, size_t /*n*/, const char* /*p_range*/, size_t /*n_range*/)
{
}

orcus::spreadsheet::range_size_t ScOrcusSheet::get_sheet_size() const
{
    orcus::spreadsheet::range_size_t ret;
    ret.rows = MAXROWCOUNT;
    ret.columns = MAXCOLCOUNT;

    return ret;
}

const sc::SharedFormulaGroups& ScOrcusSheet::getSharedFormulaGroups() const
{
    return maFormulaGroups;
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
    if (!mrDoc.GetStyleSheetPool()->HasStandardStyles())
        mrDoc.GetStyleSheetPool()->CreateStandardStyles();
}

ScOrcusStyles::font::font():
    mbBold(false),
    mbItalic(false),
    mnSize(10),
    maColor(COL_BLACK),
    mbHasFontAttr(false),
    mbHasUnderlineAttr(false),
    mbHasStrikeout(false),
    meUnderline(LINESTYLE_NONE),
    maUnderlineColor(COL_WHITE),
    meStrikeout(STRIKEOUT_NONE)
{
}

ScOrcusStyles::fill::fill():
    maPattern(""),
    maFgColor(COL_WHITE),
    maBgColor(COL_WHITE),
    mbHasFillAttr(false)
{
}

ScOrcusStyles::number_format::number_format():
    maCode(""),
    mbHasNumberFormatAttr(false)
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
    if (mbHasFontAttr)
    {
        FontItalic eItalic = mbItalic ? ITALIC_NORMAL : ITALIC_NONE;
        rSet.Put(SvxPostureItem(eItalic, ATTR_FONT_POSTURE));

        FontWeight eWeight = mbBold ? WEIGHT_BOLD : WEIGHT_NORMAL;
        rSet.Put(SvxWeightItem(eWeight, ATTR_FONT_WEIGHT));

        rSet.Put( SvxColorItem(maColor, ATTR_FONT_COLOR));

        if(!maName.isEmpty())
            rSet.Put( SvxFontItem( FAMILY_DONTKNOW, maName, maName, PITCH_DONTKNOW, RTL_TEXTENCODING_DONTKNOW, ATTR_FONT ));

        rSet.Put( SvxFontHeightItem (translateToInternal(mnSize, orcus::length_unit_t::point), 100, ATTR_FONT_HEIGHT));
    }

    if (mbHasUnderlineAttr)
    {
        SvxUnderlineItem aUnderline(meUnderline, ATTR_FONT_UNDERLINE);
        aUnderline.SetColor(maUnderlineColor);
        rSet.Put(aUnderline);
    }

    if (mbHasStrikeout)
        rSet.Put(SvxCrossedOutItem(meStrikeout, ATTR_FONT_CROSSEDOUT));
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
    mbLocked(true),
    mbPrintContent(false),
    mbFormulaHidden(false),
    mbHasProtectionAttr(false)
{
}

void ScOrcusStyles::protection::applyToItemSet(SfxItemSet& rSet) const
{
    rSet.Put(ScProtectionAttr(mbLocked, mbFormulaHidden, mbHidden, mbPrintContent));
}

ScOrcusStyles::border::border():
    mbHasBorderAttr(false)
{
    border_line();
}

ScOrcusStyles::border::border_line::border_line():
    meStyle(SvxBorderLineStyle::SOLID),
    maColor(COL_WHITE),
    mnWidth(0)
{
}

namespace {

SvxBoxItemLine getDirection(os::border_direction_t dir)
{
    switch (dir)
    {
        case os::border_direction_t::right:
            return SvxBoxItemLine::RIGHT;
        break;
        case os::border_direction_t::left:
            return SvxBoxItemLine::LEFT;
        break;
        case os::border_direction_t::top:
            return SvxBoxItemLine::TOP;
        break;
        case os::border_direction_t::bottom:
            return SvxBoxItemLine::BOTTOM;
        break;
        default:
        break;
    }
    return SvxBoxItemLine::RIGHT;
}

}

void ScOrcusStyles::border::applyToItemSet(SfxItemSet& rSet) const
{
    SvxBoxItem aBoxItem(ATTR_BORDER);
    SvxLineItem aDiagonal_TLBR(ATTR_BORDER_TLBR);
    SvxLineItem aDiagonal_BLTR(ATTR_BORDER_BLTR);

    for (auto& current_border_line : border_lines)
    {
        SvxBoxItemLine eDir = getDirection(current_border_line.first);

        if (current_border_line.first == orcus::spreadsheet::border_direction_t::diagonal_tl_br)
        {
            editeng::SvxBorderLine aLine(&current_border_line.second.maColor, current_border_line.second.mnWidth, current_border_line.second.meStyle);
            aDiagonal_TLBR.SetLine(&aLine);
        }
        else if (current_border_line.first == orcus::spreadsheet::border_direction_t::diagonal_bl_tr)
        {
            editeng::SvxBorderLine aLine(&current_border_line.second.maColor, current_border_line.second.mnWidth, current_border_line.second.meStyle);
            aDiagonal_BLTR.SetLine(&aLine);
        }
        else
        {
            editeng::SvxBorderLine aLine(&current_border_line.second.maColor, current_border_line.second.mnWidth, current_border_line.second.meStyle);
            aBoxItem.SetLine(&aLine, eDir);
        }
    }
    rSet.Put(aDiagonal_BLTR);
    rSet.Put(aDiagonal_TLBR);
    rSet.Put(aBoxItem);
}

void ScOrcusStyles::number_format::applyToItemSet(SfxItemSet& rSet, const ScDocument& rDoc) const
{
    sal_uInt32 nKey;
    sal_Int32 nCheckPos;
    SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
    OUString Code = maCode; /* <-- Done because the SvNumberFormatter::PutEntry demands a non const NumFormat Code*/
    sal_Int16 type = css::util::NumberFormat::ALL;

    if (pFormatter->PutEntry(Code, nCheckPos, type, nKey, LANGUAGE_ENGLISH_US))
    {
        if (nCheckPos == 0)
        {
            rSet.Put(SfxUInt32Item(ATTR_VALUE_FORMAT, nKey));
        }
    }
    else
        SAL_WARN("sc.orcus.style", "Cannot set Number Format");
}

ScOrcusStyles::xf::xf():
    mnFontId(0),
    mnFillId(0),
    mnBorderId(0),
    mnProtectionId(0),
    mnNumberFormatId(0),
    mnStyleXf(0),
    mbAlignment(false),
    meHor_alignment(SvxCellHorJustify::Right),
    meVer_alignment(SvxCellVerJustify::Bottom)
{
}

ScOrcusStyles::cell_style::cell_style():
    maParentName("Default"),
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
    if (rFill.mbHasFillAttr)
        rFill.applyToItemSet(rSet);

    size_t nBorderId = rXf.mnBorderId;
    if (nBorderId >= maBorders.size())
    {
        SAL_WARN("sc.orcus.style", "invalid border id");
        return;
    }
    const border& rBorder = maBorders[nBorderId];
    if (rBorder.mbHasBorderAttr)
        rBorder.applyToItemSet(rSet);

    size_t nProtectionId = rXf.mnProtectionId;
    if (nProtectionId >= maProtections.size())
    {
        SAL_WARN("sc.orcus.style", "invalid protection id");
        return;
    }
    const protection& rProtection = maProtections[nProtectionId];
    if (rProtection.mbHasProtectionAttr)
        rProtection.applyToItemSet(rSet);

    size_t nNumberFormatId = rXf.mnNumberFormatId;
    if (nNumberFormatId >= maNumberFormats.size())
    {
        SAL_WARN("sc.orcus.style", "invalid number format id");
        return;
    }
    const number_format& rFormat = maNumberFormats[nNumberFormatId];
    if (rFormat.mbHasNumberFormatAttr)
        rFormat.applyToItemSet(rSet, mrDoc);

    if(rXf.mbAlignment)
    {
        rSet.Put(SvxHorJustifyItem(rXf.meHor_alignment, ATTR_HOR_JUSTIFY));
        rSet.Put(SvxVerJustifyItem(rXf.meVer_alignment, ATTR_VER_JUSTIFY));
    }
}

void ScOrcusStyles::applyXfToItemSet(SfxItemSet& rSet, size_t xfId)
{
    SAL_INFO("sc.orcus.style", "applyXfToitemSet: " << xfId);
    if (maCellXfs.size() <= xfId)
    {
        SAL_WARN("sc.orcus.style", "invalid xf id");
        return;
    }

    const xf& rXf = maCellXfs[xfId];
    applyXfToItemSet(rSet, rXf);
}

void ScOrcusStyles::set_font_count(size_t /*n*/)
{
    // needed at all?
}

void ScOrcusStyles::set_font_bold(bool b)
{
    maCurrentFont.mbBold = b;
    maCurrentFont.mbHasFontAttr = true;
}

void ScOrcusStyles::set_font_italic(bool b)
{
    maCurrentFont.mbItalic = b;
    maCurrentFont.mbHasFontAttr = true;
}

void ScOrcusStyles::set_font_name(const char* s, size_t n)
{
    OUString aName(s, n, RTL_TEXTENCODING_UTF8);
    maCurrentFont.maName = aName;
    maCurrentFont.mbHasFontAttr = true;
}

void ScOrcusStyles::set_font_size(double point)
{
    maCurrentFont.mnSize = point;
    maCurrentFont.mbHasFontAttr = true;
}

void ScOrcusStyles::set_font_underline(orcus::spreadsheet::underline_t e)
{
    switch(e)
    {
        case orcus::spreadsheet::underline_t::single_line:
        case orcus::spreadsheet::underline_t::single_accounting:
            maCurrentFont.meUnderline = LINESTYLE_SINGLE;
            break;
        case orcus::spreadsheet::underline_t::double_line:
        case orcus::spreadsheet::underline_t::double_accounting:
            maCurrentFont.meUnderline = LINESTYLE_DOUBLE;
            break;
        case orcus::spreadsheet::underline_t::none:
            maCurrentFont.meUnderline = LINESTYLE_NONE;
            break;
        case orcus::spreadsheet::underline_t::dotted:
            maCurrentFont.meUnderline = LINESTYLE_DOTTED;
            break;
        case orcus::spreadsheet::underline_t::dash:
            maCurrentFont.meUnderline = LINESTYLE_DASH;
            break;
        case orcus::spreadsheet::underline_t::long_dash:
            maCurrentFont.meUnderline = LINESTYLE_LONGDASH;
            break;
        case orcus::spreadsheet::underline_t::dot_dash:
            maCurrentFont.meUnderline = LINESTYLE_DASHDOT;
            break;
        case orcus::spreadsheet::underline_t::dot_dot_dot_dash:
            maCurrentFont.meUnderline = LINESTYLE_DASHDOTDOT; // dot-dot-dot-dash is absent from underline types in libo
            break;
        case orcus::spreadsheet::underline_t::wave:
            maCurrentFont.meUnderline = LINESTYLE_WAVE;
            break;
        default:
            ;
    }
    maCurrentFont.mbHasUnderlineAttr = true;
}

void ScOrcusStyles::set_font_underline_width(orcus::spreadsheet::underline_width_t e )
{
    if (e == orcus::spreadsheet::underline_width_t::bold || e == orcus::spreadsheet::underline_width_t::thick)
    {
        switch(maCurrentFont.meUnderline)
        {
            case LINESTYLE_NONE:
            case LINESTYLE_SINGLE:
                maCurrentFont.meUnderline = LINESTYLE_BOLD;
                break;
            case LINESTYLE_DOTTED:
                maCurrentFont.meUnderline = LINESTYLE_BOLDDOTTED;
                break;
            case LINESTYLE_DASH:
                maCurrentFont.meUnderline = LINESTYLE_BOLDDASH;
                break;
            case LINESTYLE_LONGDASH:
                maCurrentFont.meUnderline = LINESTYLE_BOLDLONGDASH;
                break;
            case LINESTYLE_DASHDOT:
                maCurrentFont.meUnderline = LINESTYLE_BOLDDASHDOT;
                break;
            case LINESTYLE_DASHDOTDOT:
                maCurrentFont.meUnderline = LINESTYLE_BOLDDASHDOTDOT;
                break;
            case LINESTYLE_WAVE:
                maCurrentFont.meUnderline = LINESTYLE_BOLDWAVE;
                break;
            default:
                ;
        }
    }
    maCurrentFont.mbHasUnderlineAttr = true;
}

void ScOrcusStyles::set_font_underline_mode(orcus::spreadsheet::underline_mode_t /* e */)
{

}

void ScOrcusStyles::set_font_underline_type(orcus::spreadsheet::underline_type_t  e )
{
    if (e == orcus::spreadsheet::underline_type_t::double_type)
    {
        switch(maCurrentFont.meUnderline)
        {
            case LINESTYLE_NONE:
            case LINESTYLE_SINGLE:
                maCurrentFont.meUnderline = LINESTYLE_DOUBLE;
                break;
            case LINESTYLE_WAVE:
                maCurrentFont.meUnderline = LINESTYLE_DOUBLEWAVE;
                break;
            default:
                ;
        }
    }
    maCurrentFont.mbHasUnderlineAttr = true;
}

void ScOrcusStyles::set_font_underline_color(orcus::spreadsheet::color_elem_t alpha,
            orcus::spreadsheet::color_elem_t red,
            orcus::spreadsheet::color_elem_t green,
            orcus::spreadsheet::color_elem_t blue)
{
    maCurrentFont.maUnderlineColor = Color(alpha, red, green, blue);
}

void ScOrcusStyles::set_font_color(orcus::spreadsheet::color_elem_t alpha,
            orcus::spreadsheet::color_elem_t red,
            orcus::spreadsheet::color_elem_t green,
            orcus::spreadsheet::color_elem_t blue)
{
    maCurrentFont.maColor = Color(alpha, red, green, blue);
    maCurrentFont.mbHasFontAttr = true;
}

void ScOrcusStyles::set_strikethrough_style(orcus::spreadsheet::strikethrough_style_t /*s*/)
{
}

void ScOrcusStyles::set_strikethrough_type(orcus::spreadsheet::strikethrough_type_t s)
{
    if (maCurrentFont.meStrikeout != STRIKEOUT_BOLD &&
        maCurrentFont.meStrikeout != STRIKEOUT_SLASH &&
        maCurrentFont.meStrikeout != STRIKEOUT_X)
    {
        switch (s)
        {
            case os::strikethrough_type_t::unknown:
                maCurrentFont.meStrikeout = STRIKEOUT_DONTKNOW;
                break;
            case os::strikethrough_type_t::none:
                maCurrentFont.meStrikeout = STRIKEOUT_NONE;
                break;
            case os::strikethrough_type_t::single:
                maCurrentFont.meStrikeout = STRIKEOUT_SINGLE;
                break;
            case os::strikethrough_type_t::double_type:
                maCurrentFont.meStrikeout = STRIKEOUT_DOUBLE;
                break;
            default:
                ;
        }
    }
    maCurrentFont.mbHasStrikeout = true;
}

void ScOrcusStyles::set_strikethrough_width(orcus::spreadsheet::strikethrough_width_t s)
{
    switch (s)
    {
        case os::strikethrough_width_t::bold:
            maCurrentFont.meStrikeout = STRIKEOUT_BOLD;
            break;
        default:
            ;
    }
    maCurrentFont.mbHasStrikeout = true;
}

void ScOrcusStyles::set_strikethrough_text(orcus::spreadsheet::strikethrough_text_t s)
{
    switch (s)
    {
        case os::strikethrough_text_t::slash:
            maCurrentFont.meStrikeout = STRIKEOUT_SLASH;
            break;
        case os::strikethrough_text_t::cross:
            maCurrentFont.meStrikeout = STRIKEOUT_X;
            break;
        default:
            ;
    }
    maCurrentFont.mbHasStrikeout = true;
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
    maCurrentFill.mbHasFillAttr = true;
}

void ScOrcusStyles::set_fill_fg_color(orcus::spreadsheet::color_elem_t alpha, orcus::spreadsheet::color_elem_t red, orcus::spreadsheet::color_elem_t green, orcus::spreadsheet::color_elem_t blue)
{
    maCurrentFill.maFgColor = Color(alpha, red, green, blue);
    maCurrentFill.mbHasFillAttr = true;
}

void ScOrcusStyles::set_fill_bg_color(orcus::spreadsheet::color_elem_t alpha, orcus::spreadsheet::color_elem_t red, orcus::spreadsheet::color_elem_t green, orcus::spreadsheet::color_elem_t blue)
{
    maCurrentFill.maBgColor = Color(alpha, red, green, blue);
    maCurrentFill.mbHasFillAttr = true;
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

void ScOrcusStyles::set_border_style(
    orcus::spreadsheet::border_direction_t dir, orcus::spreadsheet::border_style_t style)
{
    border::border_line& current_line = maCurrentBorder.border_lines[dir];
    switch (style)
    {
        case orcus::spreadsheet::border_style_t::unknown:
        case orcus::spreadsheet::border_style_t::none:
        case orcus::spreadsheet::border_style_t::solid:
        case orcus::spreadsheet::border_style_t::hair:
        case orcus::spreadsheet::border_style_t::medium:
        case orcus::spreadsheet::border_style_t::thick:
        case orcus::spreadsheet::border_style_t::thin:
            current_line.meStyle = SvxBorderLineStyle::SOLID;
            break;
        case orcus::spreadsheet::border_style_t::dash_dot:
            current_line.meStyle = SvxBorderLineStyle::DASH_DOT;
            break;
        case orcus::spreadsheet::border_style_t::dash_dot_dot:
            current_line.meStyle = SvxBorderLineStyle::DASH_DOT_DOT;
            break;
        case orcus::spreadsheet::border_style_t::dashed:
            current_line.meStyle = SvxBorderLineStyle::DASHED;
            break;
        case orcus::spreadsheet::border_style_t::dotted:
            current_line.meStyle = SvxBorderLineStyle::DOTTED;
            break;
        case orcus::spreadsheet::border_style_t::double_border:
            current_line.meStyle = SvxBorderLineStyle::DOUBLE;
            break;
        case orcus::spreadsheet::border_style_t::medium_dash_dot:
        case orcus::spreadsheet::border_style_t::slant_dash_dot:
            current_line.meStyle = SvxBorderLineStyle::DASH_DOT;
            break;
        case orcus::spreadsheet::border_style_t::medium_dash_dot_dot:
            current_line.meStyle = SvxBorderLineStyle::DASH_DOT_DOT;
            break;
        case orcus::spreadsheet::border_style_t::medium_dashed:
            current_line.meStyle = SvxBorderLineStyle::DASHED;
            break;
        default:
            ;
    }
    maCurrentBorder.mbHasBorderAttr = true;
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

void ScOrcusStyles::set_border_width(orcus::spreadsheet::border_direction_t  dir, double val, orcus::length_unit_t  unit )
{
    border::border_line& current_line = maCurrentBorder.border_lines[dir];
    current_line.mnWidth = translateToInternal(val, unit);
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
    maCurrentProtection.mbHasProtectionAttr = true;
}

void ScOrcusStyles::set_cell_locked(bool b)
{
    maCurrentProtection.mbLocked = b;
    maCurrentProtection.mbHasProtectionAttr = true;
}

void ScOrcusStyles::set_cell_print_content(bool b )
{
    maCurrentProtection.mbPrintContent = b;
    maCurrentProtection.mbHasProtectionAttr = true;
}

void ScOrcusStyles::set_cell_formula_hidden(bool b )
{
    maCurrentProtection.mbFormulaHidden = b;
    maCurrentProtection.mbHasProtectionAttr = true;
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
    maCurrentNumberFormat.mbHasNumberFormatAttr = true;
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

void ScOrcusStyles::set_xf_horizontal_alignment(orcus::spreadsheet::hor_alignment_t align)
{
    switch (align)
    {
        case os::hor_alignment_t::left:
            maCurrentXF.meHor_alignment = SvxCellHorJustify::Left;
        break;
        case os::hor_alignment_t::right:
            maCurrentXF.meHor_alignment = SvxCellHorJustify::Right;
        break;
        case os::hor_alignment_t::center:
            maCurrentXF.meHor_alignment = SvxCellHorJustify::Center;
        break;
        case os::hor_alignment_t::justified:
            maCurrentXF.meHor_alignment = SvxCellHorJustify::Standard;
        break;
        default:
            ;
    }
    maCurrentXF.mbAlignment = true;
}

void ScOrcusStyles::set_xf_vertical_alignment(orcus::spreadsheet::ver_alignment_t align)
{
    switch (align)
    {
        case os::ver_alignment_t::top:
            maCurrentXF.meVer_alignment = SvxCellVerJustify::Top;
        break;
        case os::ver_alignment_t::bottom:
            maCurrentXF.meVer_alignment = SvxCellVerJustify::Bottom;
        break;
        case os::ver_alignment_t::middle:
            maCurrentXF.meVer_alignment = SvxCellVerJustify::Center;
        break;
        case os::ver_alignment_t::justified:
            maCurrentXF.meVer_alignment = SvxCellVerJustify::Standard;
        break;
        default:
            ;
    }
    maCurrentXF.mbAlignment = true;
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

void ScOrcusStyles::set_cell_style_parent_name(const char* s, size_t n)
{
    OUString aParentName(s, n, RTL_TEXTENCODING_UTF8);
    maCurrentCellStyle.maParentName = aParentName;
}

size_t ScOrcusStyles::commit_cell_style()
{
    SAL_INFO("sc.orcus.style", "commit cell style: " << maCurrentCellStyle.maName);
    if (maCurrentCellStyle.mnXFId >= maCellStyleXfs.size())
    {
        SAL_WARN("sc.orcus.style", "invalid xf id for commit cell style");
        return 0;
    }
    if (maCurrentCellStyle.mnXFId == 0)
    {
        return 0;
    }

    ScStyleSheetPool* pPool = mrDoc.GetStyleSheetPool();
    SfxStyleSheetBase& rBase = pPool->Make(maCurrentCellStyle.maName, SfxStyleFamily::Para);
    rBase.SetParent(maCurrentCellStyle.maParentName);
    SfxItemSet& rSet = rBase.GetItemSet();

    xf& rXf = maCellStyleXfs[maCurrentCellStyle.mnXFId];
    applyXfToItemSet(rSet, rXf);

    maCurrentXF = ScOrcusStyles::xf();
    maCurrentCellStyle = ScOrcusStyles::cell_style();

    return 0;
}

// auto filter import

ScOrcusAutoFilter::ScOrcusAutoFilter()
{
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
