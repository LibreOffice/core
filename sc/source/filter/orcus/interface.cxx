/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <limits>
#include <memory>
#include <orcusinterface.hxx>

#include <document.hxx>
#include <formulacell.hxx>
#include <rangenam.hxx>
#include <tokenarray.hxx>
#include <globalnames.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <compiler.hxx>
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
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/lineitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/justifyitem.hxx>

#include <svl/sharedstringpool.hxx>
#include <svl/numformat.hxx>
#include <svl/zforlist.hxx>
#include <svl/intitem.hxx>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <i18nlangtag/lang.h>
#include <tools/fontenum.hxx>
#include <sal/log.hxx>

#include <stylesbuffer.hxx>
#include <orcus/exception.hpp>
#include <stylehelper.hxx>

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
        case orcus::spreadsheet::formula_grammar_t::xlsx:
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

ScOrcusGlobalSettings::ScOrcusGlobalSettings(ScDocumentImport& rDoc)
    : mrDoc(rDoc)
    , meCalcGrammar(formula::FormulaGrammar::GRAM_UNSPECIFIED)
    , meOrcusGrammar(os::formula_grammar_t::unknown)
    , mnTextEncoding(RTL_TEXTENCODING_UTF8)
{
}

void ScOrcusGlobalSettings::set_origin_date(int year, int month, int day)
{
    mrDoc.setOriginDate(year, month, day);
}

void ScOrcusGlobalSettings::set_character_set(orcus::character_set_t cs)
{
    switch (cs)
    {
        case orcus::character_set_t::iso_2022_jp:
        case orcus::character_set_t::iso_2022_jp_2:
            mnTextEncoding = RTL_TEXTENCODING_ISO_2022_JP;
            break;
        case orcus::character_set_t::jis_x0201:
            mnTextEncoding = RTL_TEXTENCODING_JIS_X_0201;
            break;
        case orcus::character_set_t::shift_jis:
            mnTextEncoding = RTL_TEXTENCODING_SHIFT_JIS;
            break;
        case orcus::character_set_t::us_ascii:
            mnTextEncoding = RTL_TEXTENCODING_ASCII_US;
            break;
        case orcus::character_set_t::utf_7:
            mnTextEncoding = RTL_TEXTENCODING_UTF7;
            break;
        case orcus::character_set_t::windows_1250:
            mnTextEncoding = RTL_TEXTENCODING_MS_1250;
            break;
        case orcus::character_set_t::windows_1251:
            mnTextEncoding = RTL_TEXTENCODING_MS_1251;
            break;
        case orcus::character_set_t::windows_1252:
            mnTextEncoding = RTL_TEXTENCODING_MS_1252;
            break;
        case orcus::character_set_t::windows_1253:
            mnTextEncoding = RTL_TEXTENCODING_MS_1253;
            break;
        case orcus::character_set_t::windows_1254:
            mnTextEncoding = RTL_TEXTENCODING_MS_1254;
            break;
        case orcus::character_set_t::windows_1255:
            mnTextEncoding = RTL_TEXTENCODING_MS_1255;
            break;
        case orcus::character_set_t::windows_1256:
            mnTextEncoding = RTL_TEXTENCODING_MS_1256;
            break;
        case orcus::character_set_t::windows_1257:
            mnTextEncoding = RTL_TEXTENCODING_MS_1257;
            break;
        case orcus::character_set_t::windows_1258:
            mnTextEncoding = RTL_TEXTENCODING_MS_1258;
            break;
        default:
            ; // Add more as needed.
    }
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

ScOrcusRefResolver::ScOrcusRefResolver( const ScOrcusGlobalSettings& rGS ) :
    mrGlobalSettings(rGS) {}

os::src_address_t ScOrcusRefResolver::resolve_address(std::string_view address)
{
    OUString aStr(address.data(), address.size(), mrGlobalSettings.getTextEncoding());

    ScAddress aAddr;
    aAddr.Parse(aStr, mrGlobalSettings.getDoc().getDoc(),
        formula::FormulaGrammar::extractRefConvention(
            mrGlobalSettings.getCalcGrammar()));

    if (!aAddr.IsValid())
    {
        std::ostringstream os;
        os << "'" << address << "' is not a valid address expression.";
        throw orcus::invalid_arg_error(os.str());
    }

    os::src_address_t ret;
    ret.sheet = aAddr.Tab();
    ret.column = aAddr.Col();
    ret.row = aAddr.Row();

    return ret;
}

os::src_range_t ScOrcusRefResolver::resolve_range(std::string_view range)
{
    OUString aStr(range.data(), range.size(), mrGlobalSettings.getTextEncoding());

    ScRange aRange;
    aRange.Parse(aStr, mrGlobalSettings.getDoc().getDoc(),
        formula::FormulaGrammar::extractRefConvention(
            mrGlobalSettings.getCalcGrammar()));

    if (!aRange.IsValid())
    {
        std::ostringstream os;
        os << "'" << range << "' is not a valid range expression.";
        throw orcus::invalid_arg_error(os.str());
    }

    os::src_range_t ret;
    ret.first.sheet  = aRange.aStart.Tab();
    ret.first.column = aRange.aStart.Col();
    ret.first.row    = aRange.aStart.Row();
    ret.last.sheet   = aRange.aEnd.Tab();
    ret.last.column  = aRange.aEnd.Col();
    ret.last.row     = aRange.aEnd.Row();

    return ret;
}

ScOrcusNamedExpression::ScOrcusNamedExpression(
    ScDocumentImport& rDoc, const ScOrcusGlobalSettings& rGS, SCTAB nTab ) :
    mrDoc(rDoc), mrGlobalSettings(rGS), mnTab(nTab) {}

void ScOrcusNamedExpression::reset()
{
    maBasePos.SetTab(0);
    maBasePos.SetCol(0);
    maBasePos.SetRow(0);
    maName.clear();
    maExpr.clear();
}

void ScOrcusNamedExpression::set_base_position(const orcus::spreadsheet::src_address_t& pos)
{
    maBasePos.SetTab(pos.sheet);
    maBasePos.SetCol(pos.column);
    maBasePos.SetRow(pos.row);
}

void ScOrcusNamedExpression::set_named_expression(std::string_view name, std::string_view expression)
{
    maName = OUString(name.data(), name.size(), mrGlobalSettings.getTextEncoding());
    maExpr = OUString(expression.data(), expression.size(), mrGlobalSettings.getTextEncoding());
}

void ScOrcusNamedExpression::set_named_range(std::string_view /*name*/, std::string_view /*range*/)
{
    throw std::runtime_error("ScOrcusNamedExpression::set_named_range not implemented yet.");
}

void ScOrcusNamedExpression::commit()
{
    ScRangeName* pNames = mnTab >= 0 ? mrDoc.getDoc().GetRangeName(mnTab) : mrDoc.getDoc().GetRangeName();
    if (!pNames)
        return;

    ScRangeData* pRange = new ScRangeData(
        mrDoc.getDoc(), maName, maExpr, maBasePos, ScRangeData::Type::Name,
        mrGlobalSettings.getCalcGrammar());

    pNames->insert(pRange, false);

    reset(); // make sure to reset the state for the next run.
}

ScOrcusFactory::CellStoreToken::CellStoreToken(const ScAddress& rPos, Type eType)
    : mfValue(std::numeric_limits<double>::quiet_NaN())
    , maPos(rPos)
    , meType(eType)
    , mnIndex1(0)
    , mnIndex2(0)
    , meGrammar(formula::FormulaGrammar::GRAM_UNSPECIFIED)
{
}

ScOrcusFactory::CellStoreToken::CellStoreToken(const ScAddress& rPos, double fValue)
    : mfValue(fValue)
    , maPos(rPos)
    , meType(Type::Numeric)
    , mnIndex1(0)
    , mnIndex2(0)
    , meGrammar(formula::FormulaGrammar::GRAM_UNSPECIFIED)
{
}

ScOrcusFactory::CellStoreToken::CellStoreToken(const ScAddress& rPos, uint32_t nIndex)
    : mfValue(std::numeric_limits<double>::quiet_NaN())
    , maPos(rPos)
    , meType(Type::String)
    , mnIndex1(nIndex)
    , mnIndex2(0)
    , meGrammar(formula::FormulaGrammar::GRAM_UNSPECIFIED)
{
}

ScOrcusFactory::CellStoreToken::CellStoreToken(const ScAddress& rPos, const OUString& rFormula,
        formula::FormulaGrammar::Grammar eGrammar)
    : maStr1(rFormula)
    , mfValue(std::numeric_limits<double>::quiet_NaN())
    , maPos(rPos)
    , meType(Type::Formula)
    , mnIndex1(0)
    , mnIndex2(0)
    , meGrammar(eGrammar)
{
}

ScOrcusFactory::ScOrcusFactory(ScDocument& rDoc, bool bSkipDefaultStyles) :
    maDoc(rDoc),
    maGlobalSettings(maDoc),
    maSharedStrings(*this),
    maNamedExpressions(maDoc, maGlobalSettings),
    maStyles(*this, bSkipDefaultStyles),
    mnProgress(0) {}

orcus::spreadsheet::iface::import_sheet* ScOrcusFactory::append_sheet(
    orcus::spreadsheet::sheet_t sheet_index, std::string_view sheet_name)
{
    OUString aTabName(sheet_name.data(), sheet_name.size(), maGlobalSettings.getTextEncoding());

    if (sheet_index == 0)
    {
        // The calc document initializes with one sheet already present.
        assert(maDoc.getSheetCount() == 1);
        maDoc.setSheetName(0, aTabName);
        maSheets.push_back(std::make_unique<ScOrcusSheet>(maDoc, 0, *this));
        return maSheets.back().get();
    }

    if (!maDoc.appendSheet(aTabName))
        return nullptr;

    SCTAB nTab = maDoc.getSheetCount() - 1;
    maSheets.push_back(std::make_unique<ScOrcusSheet>(maDoc, nTab, *this));
    return maSheets.back().get();
}

namespace {

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

}

orcus::spreadsheet::iface::import_sheet* ScOrcusFactory::get_sheet(std::string_view sheet_name)
{
    OUString aTabName(sheet_name.data(), sheet_name.size(), maGlobalSettings.getTextEncoding());
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
    maSheets.push_back(std::make_unique<ScOrcusSheet>(maDoc, nTab, *this));
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
    maSheets.push_back(std::make_unique<ScOrcusSheet>(maDoc, nTab, *this));
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

        return std::make_unique<ScFormulaCell>(maDoc.getDoc(), rToken.maPos, *pArray);
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
                if (std::isfinite(rToken.mfValue))
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

                if (std::isfinite(rToken.mfValue))
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

                ScCompiler aComp(maDoc.getDoc(), aRange.aStart, rToken.meGrammar);
                std::unique_ptr<ScTokenArray> pArray(aComp.CompileString(rToken.maStr1));
                if (!pArray)
                    break;

                maDoc.setMatrixCells(aRange, *pArray, rToken.meGrammar);
                break;
            }
            case CellStoreToken::Type::FillDownCells:
            {
                if (!rToken.mnIndex1)
                    break;

                maDoc.fillDownCells(rToken.maPos, rToken.mnIndex1);
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

ScDocumentImport& ScOrcusFactory::getDoc()
{
    return maDoc;
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

const OUString* ScOrcusFactory::getString(size_t nIndex) const
{
    return nIndex < maStrings.size() ? &maStrings[nIndex] : nullptr;
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

void ScOrcusFactory::pushFillDownCellsToken( const ScAddress& rPos, uint32_t nFillSize )
{
    maCellStoreTokens.emplace_back(rPos, CellStoreToken::Type::FillDownCells);
    maCellStoreTokens.back().mnIndex1 = nFillSize;
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
        mxStatusIndicator->start(ScResId(STR_LOAD_DOC), 100);

    if (mnProgress == 99)
        return;

    ++mnProgress;
    mxStatusIndicator->setValue(mnProgress);
}

void ScOrcusFactory::setStatusIndicator(const uno::Reference<task::XStatusIndicator>& rIndicator)
{
    mxStatusIndicator = rIndicator;
}

const ScOrcusGlobalSettings& ScOrcusFactory::getGlobalSettings() const
{
    return maGlobalSettings;
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
        case orcus::length_unit_t::twip:
            return nVal;
        case orcus::length_unit_t::point:
            return nVal * 20.0;
        case orcus::length_unit_t::centimeter:
            return nVal * 20.0 * 72.0 / 2.54;
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

void ScOrcusConditionalFormat::set_formula(std::string_view /*formula*/)
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

void ScOrcusConditionalFormat::set_icon_name(std::string_view /*name*/)
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

void ScOrcusConditionalFormat::set_range(std::string_view /*range*/)
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
    maAutoFilter(rFactory.getGlobalSettings()),
    maProperties(mnTab, mrDoc),
    maConditionalFormat(mnTab, rDoc.getDoc()),
    maNamedExpressions(rDoc, rFactory.getGlobalSettings(), nTab),
    maFormula(*this),
    maArrayFormula(*this),
    mnCellCount(0)
{
}

void ScOrcusFormula::reset()
{
    mnCol = -1;
    mnRow = -1;
    maFormula.clear();
    meGrammar = formula::FormulaGrammar::GRAM_UNSPECIFIED;
    mnSharedFormulaIndex = 0;
    mbShared = false;
    meResType = ResultType::NotSet;
    maResult.clear();
    mfResult = 0.0;
}

ScOrcusFormula::ScOrcusFormula( ScOrcusSheet& rSheet ) :
    mrSheet(rSheet),
    mnCol(-1),
    mnRow(-1),
    meGrammar(formula::FormulaGrammar::GRAM_UNSPECIFIED),
    mnSharedFormulaIndex(0),
    mbShared(false),
    meResType(ResultType::NotSet),
    mfResult(0.0) {}

ScOrcusFormula::~ScOrcusFormula() {}

void ScOrcusFormula::set_position(os::row_t row, os::col_t col)
{
    mnCol = col;
    mnRow = row;
}

void ScOrcusFormula::set_formula(os::formula_grammar_t grammar, std::string_view formula)
{
    maFormula = OUString(formula.data(), formula.size(), mrSheet.getFactory().getGlobalSettings().getTextEncoding());
    meGrammar = getCalcGrammarFromOrcus(grammar);
}

void ScOrcusFormula::set_shared_formula_index(size_t index)
{
    mnSharedFormulaIndex = index;
    mbShared = true;
}

void ScOrcusFormula::set_result_value(double value)
{
    meResType = ResultType::Value;
    mfResult = value;
}

void ScOrcusFormula::set_result_string(std::string_view value)
{
    meResType = ResultType::String;
    maResult = OUString(value.data(), value.size(), mrSheet.getFactory().getGlobalSettings().getTextEncoding());
}

void ScOrcusFormula::set_result_empty()
{
    meResType = ResultType::Empty;
}

void ScOrcusFormula::set_result_bool(bool value)
{
    meResType = ResultType::Value;
    mfResult = value ? 1.0 : 0.0;
}

void ScOrcusFormula::commit()
{
    ScOrcusFactory& rFactory = mrSheet.getFactory();
    sc::SharedFormulaGroups& rGroups = mrSheet.getSharedFormulaGroups();
    ScAddress aPos(mnCol, mnRow, mrSheet.getIndex());

    if (mbShared)
    {
        if (maFormula.isEmpty())
        {
            // shared formula that references existing formula token.
            const ScTokenArray* pArray = rGroups.get(mnSharedFormulaIndex);
            if (!pArray)
                return;
        }
        else
        {
            // topmost shared formula with new formula token.

            // Compile the formula expression into tokens.
            ScCompiler aComp(mrSheet.getDoc().getDoc(), aPos, meGrammar);
            std::unique_ptr<ScTokenArray> pArray = aComp.CompileString(maFormula);
            if (!pArray)
                // Tokenization failed.
                return;

            rGroups.set(mnSharedFormulaIndex, std::move(pArray));
        }
        rFactory.pushSharedFormulaToken(aPos, mnSharedFormulaIndex);
    }
    else
    {
        // non-shared formula
        rFactory.pushCellStoreToken(aPos, maFormula, meGrammar);
    }

    switch (meResType)
    {
        case ResultType::String:
        {
            rFactory.pushFormulaResult(aPos, maResult);
            break;
        }
        case ResultType::Value:
            rFactory.pushFormulaResult(aPos, mfResult);
            break;
        default:
            ;
    }

    mrSheet.cellInserted();
}

void ScOrcusArrayFormula::reset()
{
    mnCol = -1;
    mnRow = -1;
    mnColRange = 0;
    mnRowRange = 0;

    maFormula.clear();
    meGrammar = formula::FormulaGrammar::GRAM_UNSPECIFIED;
}

ScOrcusArrayFormula::ScOrcusArrayFormula( ScOrcusSheet& rSheet ) :
    mrSheet(rSheet),
    mnCol(-1),
    mnRow(-1),
    mnColRange(0),
    mnRowRange(0),
    meGrammar(formula::FormulaGrammar::GRAM_UNSPECIFIED) {}

ScOrcusArrayFormula::~ScOrcusArrayFormula() {}

void ScOrcusArrayFormula::set_range(const os::range_t& range)
{
    mnCol = range.first.column;
    mnRow = range.first.row;

    mnColRange = range.last.column - range.first.column + 1;
    mnRowRange = range.last.row - range.first.column + 1;
}

void ScOrcusArrayFormula::set_formula(os::formula_grammar_t grammar, std::string_view formula)
{
    meGrammar = getCalcGrammarFromOrcus(grammar);
    maFormula = OUString(formula.data(), formula.size(), mrSheet.getFactory().getGlobalSettings().getTextEncoding());
}

void ScOrcusArrayFormula::set_result_value(os::row_t /*row*/, os::col_t /*col*/, double /*value*/)
{
    // TODO : implement result cache for matrix
}

void ScOrcusArrayFormula::set_result_string(os::row_t /*row*/, os::col_t /*col*/, std::string_view /*value*/)
{
    // TODO : implement result cache for matrix
}

void ScOrcusArrayFormula::set_result_empty(os::row_t /*row*/, os::col_t /*col*/)
{
    // TODO : implement result cache for matrix
}

void ScOrcusArrayFormula::set_result_bool(os::row_t /*row*/, os::col_t /*col*/, bool /*value*/)
{
    // TODO : implement result cache for matrix
}

void ScOrcusArrayFormula::commit()
{
    ScAddress aPos(mnCol, mnRow, mrSheet.getIndex());
    mrSheet.getFactory().pushMatrixFormulaToken(aPos, maFormula, meGrammar, mnRowRange, mnColRange);
    mrSheet.cellInserted();
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

ScDocumentImport& ScOrcusSheet::getDoc()
{
    return mrDoc;
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

os::iface::import_named_expression* ScOrcusSheet::get_named_expression()
{
    return &maNamedExpressions;
}

os::iface::import_formula* ScOrcusSheet::get_formula()
{
    maFormula.reset();
    return &maFormula;
}

os::iface::import_array_formula* ScOrcusSheet::get_array_formula()
{
    maArrayFormula.reset();
    return &maArrayFormula;
}

void ScOrcusSheet::set_auto(os::row_t row, os::col_t col, std::string_view value)
{
    OUString aVal(value.data(), value.size(), mrFactory.getGlobalSettings().getTextEncoding());
    mrFactory.pushCellStoreAutoToken(ScAddress(col, row, mnTab), aVal);
    cellInserted();
}

void ScOrcusSheet::set_string(os::row_t row, os::col_t col, os::string_id_t sindex)
{
    mrFactory.pushCellStoreToken(ScAddress(col, row, mnTab), sindex);
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
    tools::Long nDateDiff = aDate - pFormatter->GetNullDate();

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

orcus::spreadsheet::range_size_t ScOrcusSheet::get_sheet_size() const
{
    orcus::spreadsheet::range_size_t ret;
    ret.rows = MAXROWCOUNT;
    ret.columns = MAXCOLCOUNT;

    return ret;
}

void ScOrcusSheet::fill_down_cells(os::row_t row, os::col_t col, os::row_t range_size)
{
    mrFactory.pushFillDownCellsToken(ScAddress(col, row, mnTab), range_size);
    cellInserted();
}

const sc::SharedFormulaGroups& ScOrcusSheet::getSharedFormulaGroups() const
{
    return maFormulaGroups;
}

sc::SharedFormulaGroups& ScOrcusSheet::getSharedFormulaGroups()
{
    return maFormulaGroups;
}

ScOrcusFactory& ScOrcusSheet::getFactory()
{
    return mrFactory;
}

ScOrcusSharedStrings::ScOrcusSharedStrings(ScOrcusFactory& rFactory) :
    mrFactory(rFactory) {}

size_t ScOrcusSharedStrings::append(std::string_view s)
{
    OUString aNewString(s.data(), s.size(), mrFactory.getGlobalSettings().getTextEncoding());
    return mrFactory.appendString(aNewString);
}

size_t ScOrcusSharedStrings::add(std::string_view s)
{
    OUString aNewString(s.data(), s.size(), mrFactory.getGlobalSettings().getTextEncoding());
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

void ScOrcusSharedStrings::set_segment_font_name(std::string_view /*s*/)
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

void ScOrcusSharedStrings::append_segment(std::string_view s)
{
    maCurSegment.append(s.data(), s.size());
}

size_t ScOrcusSharedStrings::commit_segments()
{
    OString aStr = maCurSegment.makeStringAndClear();
    return mrFactory.addString(
        OStringToOUString(aStr, mrFactory.getGlobalSettings().getTextEncoding()));
}

ScOrcusStyles::ScOrcusStyles( ScOrcusFactory& rFactory, bool bSkipDefaultStyles ) :
    mrFactory(rFactory)
{
    ScDocument& rDoc = rFactory.getDoc().getDoc();
    if (!bSkipDefaultStyles && !rDoc.GetStyleSheetPool()->HasStandardStyles())
        rDoc.GetStyleSheetPool()->CreateStandardStyles();
}

ScOrcusStyles::fill::fill():
    mePattern(orcus::spreadsheet::fill_pattern_t::none),
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
    if (mbItalic)
    {
        FontItalic eItalic = *mbItalic ? ITALIC_NORMAL : ITALIC_NONE;
        rSet.Put(SvxPostureItem(eItalic, ATTR_FONT_POSTURE));
        rSet.Put(SvxPostureItem(eItalic, ATTR_CJK_FONT_POSTURE));
        rSet.Put(SvxPostureItem(eItalic, ATTR_CTL_FONT_POSTURE));
    }

    if (mbBold)
    {
        FontWeight eWeight = *mbBold ? WEIGHT_BOLD : WEIGHT_NORMAL;
        rSet.Put(SvxWeightItem(eWeight, ATTR_FONT_WEIGHT));
        rSet.Put(SvxWeightItem(eWeight, ATTR_CJK_FONT_WEIGHT));
        rSet.Put(SvxWeightItem(eWeight, ATTR_CTL_FONT_WEIGHT));
    }

    if (maColor)
        rSet.Put( SvxColorItem(*maColor, ATTR_FONT_COLOR));

    if (maName && !maName->isEmpty())
        rSet.Put( SvxFontItem( FAMILY_DONTKNOW, *maName, *maName, PITCH_DONTKNOW, RTL_TEXTENCODING_DONTKNOW, ATTR_FONT ));

    if (mnSize)
    {
        double fSize = translateToInternal(*mnSize, orcus::length_unit_t::point);
        rSet.Put(SvxFontHeightItem(fSize, 100, ATTR_FONT_HEIGHT));
        rSet.Put(SvxFontHeightItem(fSize, 100, ATTR_CJK_FONT_HEIGHT));
        rSet.Put(SvxFontHeightItem(fSize, 100, ATTR_CTL_FONT_HEIGHT));
    }

    if (meUnderline)
    {
        SvxUnderlineItem aUnderline(*meUnderline, ATTR_FONT_UNDERLINE);
        if (maUnderlineColor)
            aUnderline.SetColor(*maUnderlineColor);
        rSet.Put(aUnderline);
    }

    if (meStrikeout)
        rSet.Put(SvxCrossedOutItem(*meStrikeout, ATTR_FONT_CROSSEDOUT));
}

void ScOrcusStyles::fill::applyToItemSet(SfxItemSet& rSet) const
{
    if (mePattern == orcus::spreadsheet::fill_pattern_t::none)
    {
        SAL_INFO("sc.orcus.style", "no fill style");
        return;
    }

    if (mePattern == orcus::spreadsheet::fill_pattern_t::solid)
        rSet.Put(SvxBrushItem(maFgColor, ATTR_BACKGROUND));
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
    maColor(COL_BLACK),
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
        case os::border_direction_t::left:
            return SvxBoxItemLine::LEFT;
        case os::border_direction_t::top:
            return SvxBoxItemLine::TOP;
        case os::border_direction_t::bottom:
            return SvxBoxItemLine::BOTTOM;
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
    SvNumFormatType type = SvNumFormatType::ALL;

    pFormatter->PutEntry(Code, nCheckPos, type, nKey, LANGUAGE_ENGLISH_US);
    if (!nCheckPos)
        rSet.Put(SfxUInt32Item(ATTR_VALUE_FORMAT, nKey));
}

ScOrcusStyles::xf::xf():
    mnFontId(0),
    mnFillId(0),
    mnBorderId(0),
    mnProtectionId(0),
    mnNumberFormatId(0),
    mnStyleXf(0),
    mbAlignment(false),
    meHorAlignment(SvxCellHorJustify::Standard),
    meVerAlignment(SvxCellVerJustify::Standard),
    meHorAlignMethod(SvxCellJustifyMethod::Auto),
    meVerAlignMethod(SvxCellJustifyMethod::Auto)
{
}

ScOrcusStyles::cell_style::cell_style():
    maParentName(OUString(SC_STYLE_PROG_STANDARD)),
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
        rFormat.applyToItemSet(rSet, mrFactory.getDoc().getDoc());

    if(rXf.mbAlignment)
    {
        rSet.Put(SvxHorJustifyItem(rXf.meHorAlignment, ATTR_HOR_JUSTIFY));
        rSet.Put(SvxVerJustifyItem(rXf.meVerAlignment, ATTR_VER_JUSTIFY));
        rSet.Put(SvxJustifyMethodItem(rXf.meHorAlignMethod, ATTR_HOR_JUSTIFY_METHOD));
        rSet.Put(SvxJustifyMethodItem(rXf.meVerAlignMethod, ATTR_VER_JUSTIFY_METHOD));
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
}

void ScOrcusStyles::set_font_italic(bool b)
{
    maCurrentFont.mbItalic = b;
}

void ScOrcusStyles::set_font_name(std::string_view name)
{
    OUString aName(name.data(), name.size(), mrFactory.getGlobalSettings().getTextEncoding());
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
}

void ScOrcusStyles::set_font_underline_width(orcus::spreadsheet::underline_width_t e )
{
    if (e == orcus::spreadsheet::underline_width_t::bold || e == orcus::spreadsheet::underline_width_t::thick)
    {
        if (maCurrentFont.meUnderline)
        {
            switch (*maCurrentFont.meUnderline)
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
        else
            maCurrentFont.meUnderline = LINESTYLE_BOLD;
    }
}

void ScOrcusStyles::set_font_underline_mode(orcus::spreadsheet::underline_mode_t /*e*/)
{
}

void ScOrcusStyles::set_font_underline_type(orcus::spreadsheet::underline_type_t  e )
{
    if (e == orcus::spreadsheet::underline_type_t::double_type)
    {
        if (maCurrentFont.meUnderline)
        {
            switch (*maCurrentFont.meUnderline)
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
        else
            maCurrentFont.meUnderline = LINESTYLE_DOUBLE;
    }
}

void ScOrcusStyles::set_font_underline_color(orcus::spreadsheet::color_elem_t alpha,
            orcus::spreadsheet::color_elem_t red,
            orcus::spreadsheet::color_elem_t green,
            orcus::spreadsheet::color_elem_t blue)
{
    maCurrentFont.maUnderlineColor = Color(ColorAlpha, alpha, red, green, blue);
}

void ScOrcusStyles::set_font_color(orcus::spreadsheet::color_elem_t alpha,
            orcus::spreadsheet::color_elem_t red,
            orcus::spreadsheet::color_elem_t green,
            orcus::spreadsheet::color_elem_t blue)
{
    maCurrentFont.maColor = Color(ColorAlpha, alpha, red, green, blue);
}

void ScOrcusStyles::set_strikethrough_style(orcus::spreadsheet::strikethrough_style_t /*s*/)
{
}

void ScOrcusStyles::set_strikethrough_type(orcus::spreadsheet::strikethrough_type_t s)
{
    if (maCurrentFont.meStrikeout)
    {
        if (*maCurrentFont.meStrikeout == STRIKEOUT_BOLD ||
            *maCurrentFont.meStrikeout == STRIKEOUT_SLASH ||
            *maCurrentFont.meStrikeout == STRIKEOUT_X)
            return;
    }

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

void ScOrcusStyles::set_fill_pattern_type(orcus::spreadsheet::fill_pattern_t fp)
{
    maCurrentFill.mePattern = fp;
}

void ScOrcusStyles::set_fill_fg_color(
    orcus::spreadsheet::color_elem_t /*alpha*/, orcus::spreadsheet::color_elem_t red, orcus::spreadsheet::color_elem_t green, orcus::spreadsheet::color_elem_t blue)
{
    // Ignore the alpha element for now.
    maCurrentFill.maFgColor = Color(red, green, blue);
    maCurrentFill.mbHasFillAttr = true;
}

void ScOrcusStyles::set_fill_bg_color(
    orcus::spreadsheet::color_elem_t /*alpha*/, orcus::spreadsheet::color_elem_t red, orcus::spreadsheet::color_elem_t green, orcus::spreadsheet::color_elem_t blue)
{
    // Ignore the alpha element for now.
    maCurrentFill.maBgColor = Color(red, green, blue);
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
        case orcus::spreadsheet::border_style_t::solid:
            current_line.meStyle = SvxBorderLineStyle::SOLID;
            current_line.mnWidth = oox::xls::API_LINE_THIN;
            break;
        case orcus::spreadsheet::border_style_t::hair:
            current_line.meStyle = SvxBorderLineStyle::SOLID;
            current_line.mnWidth = oox::xls::API_LINE_HAIR;
            break;
        case orcus::spreadsheet::border_style_t::medium:
            current_line.meStyle = SvxBorderLineStyle::SOLID;
            current_line.mnWidth = oox::xls::API_LINE_MEDIUM;
            break;
        case orcus::spreadsheet::border_style_t::thick:
            current_line.meStyle = SvxBorderLineStyle::SOLID;
            current_line.mnWidth = oox::xls::API_LINE_THICK;
            break;
        case orcus::spreadsheet::border_style_t::thin:
            current_line.meStyle = SvxBorderLineStyle::SOLID;
            current_line.mnWidth = oox::xls::API_LINE_THIN;
            break;
        case orcus::spreadsheet::border_style_t::dash_dot:
            current_line.meStyle = SvxBorderLineStyle::DASH_DOT;
            current_line.mnWidth = oox::xls::API_LINE_THIN;
            break;
        case orcus::spreadsheet::border_style_t::dash_dot_dot:
            current_line.meStyle = SvxBorderLineStyle::DASH_DOT_DOT;
            current_line.mnWidth = oox::xls::API_LINE_THIN;
            break;
        case orcus::spreadsheet::border_style_t::dashed:
            current_line.meStyle = SvxBorderLineStyle::DASHED;
            current_line.mnWidth = oox::xls::API_LINE_THIN;
            break;
        case orcus::spreadsheet::border_style_t::dotted:
            current_line.meStyle = SvxBorderLineStyle::DOTTED;
            current_line.mnWidth = oox::xls::API_LINE_THIN;
            break;
        case orcus::spreadsheet::border_style_t::double_border:
            current_line.meStyle = SvxBorderLineStyle::DOUBLE;
            current_line.mnWidth = oox::xls::API_LINE_THICK;
            break;
        case orcus::spreadsheet::border_style_t::medium_dash_dot:
        case orcus::spreadsheet::border_style_t::slant_dash_dot:
            current_line.meStyle = SvxBorderLineStyle::DASH_DOT;
            current_line.mnWidth = oox::xls::API_LINE_MEDIUM;
            break;
        case orcus::spreadsheet::border_style_t::medium_dash_dot_dot:
            current_line.meStyle = SvxBorderLineStyle::DASH_DOT_DOT;
            current_line.mnWidth = oox::xls::API_LINE_MEDIUM;
            break;
        case orcus::spreadsheet::border_style_t::medium_dashed:
            current_line.meStyle = SvxBorderLineStyle::DASHED;
            current_line.mnWidth = oox::xls::API_LINE_MEDIUM;
            break;
        case orcus::spreadsheet::border_style_t::unknown:
        case orcus::spreadsheet::border_style_t::none:
            current_line.mnWidth = oox::xls::API_LINE_NONE;
            break;
        default:
            ;
    }
    maCurrentBorder.mbHasBorderAttr = true;
}

void ScOrcusStyles::set_border_color(orcus::spreadsheet::border_direction_t dir,
            orcus::spreadsheet::color_elem_t /*alpha*/,
            orcus::spreadsheet::color_elem_t red,
            orcus::spreadsheet::color_elem_t green,
            orcus::spreadsheet::color_elem_t blue)
{
    border::border_line& current_line = maCurrentBorder.border_lines[dir];
    current_line.maColor = Color(red, green, blue);
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

void ScOrcusStyles::set_number_format_code(std::string_view s)
{
    OUString aCode(s.data(), s.size(), mrFactory.getGlobalSettings().getTextEncoding());
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
            maCurrentXF.meHorAlignment = SvxCellHorJustify::Left;
            break;
        case os::hor_alignment_t::right:
            maCurrentXF.meHorAlignment = SvxCellHorJustify::Right;
            break;
        case os::hor_alignment_t::center:
            maCurrentXF.meHorAlignment = SvxCellHorJustify::Center;
            break;
        case os::hor_alignment_t::justified:
            maCurrentXF.meHorAlignment = SvxCellHorJustify::Block;
            break;
        case os::hor_alignment_t::distributed:
            maCurrentXF.meHorAlignment = SvxCellHorJustify::Block;
            maCurrentXF.meHorAlignMethod = SvxCellJustifyMethod::Distribute;
            break;
        case os::hor_alignment_t::unknown:
            maCurrentXF.meHorAlignment = SvxCellHorJustify::Standard;
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
            maCurrentXF.meVerAlignment = SvxCellVerJustify::Top;
            break;
        case os::ver_alignment_t::bottom:
            maCurrentXF.meVerAlignment = SvxCellVerJustify::Bottom;
            break;
        case os::ver_alignment_t::middle:
            maCurrentXF.meVerAlignment = SvxCellVerJustify::Center;
            break;
        case os::ver_alignment_t::justified:
            maCurrentXF.meVerAlignment = SvxCellVerJustify::Block;
            break;
        case os::ver_alignment_t::distributed:
            maCurrentXF.meVerAlignment = SvxCellVerJustify::Block;
            maCurrentXF.meVerAlignMethod = SvxCellJustifyMethod::Distribute;
            break;
        case os::ver_alignment_t::unknown:
            maCurrentXF.meVerAlignment = SvxCellVerJustify::Standard;
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

void ScOrcusStyles::set_cell_style_name(std::string_view name)
{
    OUString aName(name.data(), name.size(), mrFactory.getGlobalSettings().getTextEncoding());
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

void ScOrcusStyles::set_cell_style_parent_name(std::string_view name)
{
    const OUString aParentName(name.data(), name.size(), mrFactory.getGlobalSettings().getTextEncoding());
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

    ScStyleSheetPool* pPool = mrFactory.getDoc().getDoc().GetStyleSheetPool();
    SfxStyleSheetBase& rBase = pPool->Make(maCurrentCellStyle.maName, SfxStyleFamily::Para);
    // Need to convert the parent name to localized UI name, see tdf#139205.
    rBase.SetParent(ScStyleNameConversion::ProgrammaticToDisplayName(maCurrentCellStyle.maParentName,
                                                                     SfxStyleFamily::Para));
    SfxItemSet& rSet = rBase.GetItemSet();

    xf& rXf = maCellStyleXfs[maCurrentCellStyle.mnXFId];
    applyXfToItemSet(rSet, rXf);

    maCurrentXF = ScOrcusStyles::xf();
    maCurrentCellStyle = ScOrcusStyles::cell_style();

    return 0;
}

// auto filter import

ScOrcusAutoFilter::ScOrcusAutoFilter( const ScOrcusGlobalSettings& rGS ) :
    mrGlobalSettings(rGS)
{
}

ScOrcusAutoFilter::~ScOrcusAutoFilter()
{
}

void ScOrcusAutoFilter::set_range(const orcus::spreadsheet::range_t& range)
{
    maRange.aStart.SetRow(range.first.row);
    maRange.aStart.SetCol(range.first.column);
    maRange.aEnd.SetRow(range.last.row);
    maRange.aEnd.SetCol(range.last.column);
}

void ScOrcusAutoFilter::set_column(orcus::spreadsheet::col_t col)
{
    SAL_INFO("sc.orcus.autofilter", "set_column: " << col);
}

void ScOrcusAutoFilter::append_column_match_value(std::string_view value)
{
    OUString aString(value.data(), value.size(), mrGlobalSettings.getTextEncoding());
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
