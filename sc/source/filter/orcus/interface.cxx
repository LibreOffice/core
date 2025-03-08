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
#include <editeng/eeitem.hxx>
#include <editeng/escapementitem.hxx>

#include <svl/sharedstringpool.hxx>
#include <svl/numformat.hxx>
#include <svl/zforlist.hxx>
#include <svl/intitem.hxx>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <i18nlangtag/lang.h>
#include <o3tl/unit_conversion.hxx>
#include <tools/fontenum.hxx>
#include <sal/log.hxx>

#include <stylesbuffer.hxx>
#include <orcus/exception.hpp>
#include <stylehelper.hxx>
#include <utility>
#include <unordered_map>
#include <frozen/bits/defines.h>
#include <frozen/bits/elsa_std.h>

using namespace com::sun::star;

namespace os = orcus::spreadsheet;

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

ScOrcusFactory::CellStoreToken::CellStoreToken(const ScAddress& rPos, OUString aFormula,
        formula::FormulaGrammar::Grammar eGrammar)
    : maStr1(std::move(aFormula))
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
    maRefResolver(maGlobalSettings),
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
        maSheets.push_back(std::make_unique<ScOrcusSheet>(maDoc, maGlobalSettings, 0, *this));
        return maSheets.back().get();
    }

    if (!maDoc.appendSheet(aTabName))
        return nullptr;

    SCTAB nTab = maDoc.getSheetCount() - 1;
    maSheets.push_back(std::make_unique<ScOrcusSheet>(maDoc, maGlobalSettings, nTab, *this));
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
    maSheets.push_back(std::make_unique<ScOrcusSheet>(maDoc, maGlobalSettings, nTab, *this));
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
    maSheets.push_back(std::make_unique<ScOrcusSheet>(maDoc, maGlobalSettings, nTab, *this));
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

os::iface::import_reference_resolver* ScOrcusFactory::get_reference_resolver(os::formula_ref_context_t cxt)
{
    switch (cxt)
    {
        case os::formula_ref_context_t::global:
            return &maRefResolver;
        case os::formula_ref_context_t::named_expression_base:
        case os::formula_ref_context_t::named_range:
        case os::formula_ref_context_t::table_range:
            return nullptr;
    }

    return nullptr;
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

                const auto& s = maStrings[rToken.mnIndex1];
                switch (s.index())
                {
                    case 0: // OUString
                        maDoc.setStringCell(rToken.maPos, std::get<0>(s));
                        break;
                    case 1: // std::unique_ptr<EditTextObject>
                        maDoc.setEditCell(rToken.maPos, std::get<1>(s)->Clone());
                        break;
                }
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

std::size_t ScOrcusFactory::appendFormattedString(std::unique_ptr<EditTextObject> pEditText)
{
    std::size_t nPos = maStrings.size();
    maStrings.push_back(std::move(pEditText));
    return nPos;
}

const OUString* ScOrcusFactory::getString(size_t nIndex) const
{
    if (nIndex >= maStrings.size())
        return nullptr;

    const StringValueType& rStr = maStrings[nIndex];
    if (rStr.index() != 0)
        return nullptr;

    return &std::get<OUString>(rStr);
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
            return o3tl::convert(nVal, o3tl::Length::in, o3tl::Length::twip);
        case orcus::length_unit_t::twip:
            return nVal;
        case orcus::length_unit_t::point:
            return o3tl::convert(nVal, o3tl::Length::pt, o3tl::Length::twip);
        case orcus::length_unit_t::centimeter:
            return o3tl::convert(nVal, o3tl::Length::cm, o3tl::Length::twip);
        case orcus::length_unit_t::millimeter:
            return o3tl::convert(nVal, o3tl::Length::mm, o3tl::Length::twip);
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

void ScOrcusSheetProperties::set_column_width(os::col_t col, os::col_t col_span, double width, orcus::length_unit_t unit)
{
    double nNewWidth = translateToInternal(width, unit);

    for (os::col_t offset = 0; offset < col_span; ++offset)
        mrDoc.getDoc().SetColWidthOnly(col + offset, mnTab, nNewWidth);
}

void ScOrcusSheetProperties::set_column_hidden(os::col_t col, os::col_t col_span, bool hidden)
{
    if (hidden)
        mrDoc.getDoc().SetColHidden(col, col + col_span - 1, mnTab, hidden);
}

void ScOrcusSheetProperties::set_row_height(
    os::row_t row, os::row_t row_span, double height, orcus::length_unit_t unit)
{
    double nNewHeight = translateToInternal(height, unit);
    SCROW nStartRow = row;
    SCROW nEndRow = row + row_span - 1;
    mrDoc.getDoc().SetRowHeightOnly(nStartRow, nEndRow, mnTab, nNewHeight);
}

void ScOrcusSheetProperties::set_row_hidden(os::row_t row, os::row_t row_span, bool hidden)
{
    SCROW nStartRow = row;
    SCROW nEndRow = row + row_span - 1;
    mrDoc.getDoc().SetRowHidden(nStartRow, nEndRow, mnTab, hidden);
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

ScOrcusSheet::ScOrcusSheet(ScDocumentImport& rDoc, const ScOrcusGlobalSettings& rGS, SCTAB nTab, ScOrcusFactory& rFactory) :
    mrDoc(rDoc),
    mnTab(nTab),
    mrFactory(rFactory),
    mrStyles(static_cast<ScOrcusStyles&>(*mrFactory.get_styles())),
    maProperties(mnTab, mrDoc),
    maConditionalFormat(mnTab, rDoc.getDoc()),
    maNamedExpressions(rDoc, rFactory.getGlobalSettings(), nTab),
    maFormula(*this),
    maArrayFormula(*this),
    maAutoFilter(rDoc, rGS, nTab),
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

    ScPatternAttr aPattern(mrDoc.getDoc().getCellAttributeHelper());
    mrStyles.applyXfToItemSet(aPattern.GetItemSet(), xf_index);
    mrDoc.getDoc().ApplyPattern(col, row, mnTab, aPattern);
}

void ScOrcusSheet::set_format(os::row_t row_start, os::col_t col_start,
        os::row_t row_end, os::col_t col_end, size_t xf_index)
{
    SAL_INFO("sc.orcus.style", "set format range: " << xf_index);
    ScPatternAttr aPattern(mrDoc.getDoc().getCellAttributeHelper());
    mrStyles.applyXfToItemSet(aPattern.GetItemSet(), xf_index);
    mrDoc.getDoc().ApplyPatternAreaTab(col_start, row_start, col_end, row_end, mnTab, aPattern);
}

void ScOrcusSheet::set_column_format(
    os::col_t col, os::col_t col_span, std::size_t xf_index)
{
    ScPatternAttr aPattern(mrDoc.getDoc().getCellAttributeHelper());
    mrStyles.applyXfToItemSet(aPattern.GetItemSet(), xf_index);

    mrDoc.getDoc().ApplyPatternAreaTab(
        col, 0, col + col_span - 1, mrDoc.getDoc().MaxRow(), mnTab, aPattern);
}

void ScOrcusSheet::set_row_format(os::row_t row, std::size_t xf_index)
{
    ScPatternAttr aPattern(mrDoc.getDoc().getCellAttributeHelper());
    mrStyles.applyXfToItemSet(aPattern.GetItemSet(), xf_index);

    mrDoc.getDoc().ApplyPatternAreaTab(
        0, row, mrDoc.getDoc().MaxCol(), row, mnTab, aPattern);
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

os::iface::import_auto_filter* ScOrcusSheet::start_auto_filter(const os::range_t& range)
{
    maAutoFilter.reset(range);
    return &maAutoFilter;
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

void ScOrcusStrikethrough::reset()
{
    meStyle.reset();
    meType.reset();
    meWidth.reset();
    meText.reset();
}

std::optional<FontStrikeout> ScOrcusStrikethrough::toFontStrikeout() const
{
    std::optional<FontStrikeout> eStrikeout;

    if (meType)
    {
        switch (*meType)
        {
            case os::strikethrough_type_t::unknown:
                eStrikeout = STRIKEOUT_DONTKNOW;
                break;
            case os::strikethrough_type_t::none:
                eStrikeout = STRIKEOUT_NONE;
                break;
            case os::strikethrough_type_t::single_type:
                eStrikeout = STRIKEOUT_SINGLE;
                break;
            case os::strikethrough_type_t::double_type:
                eStrikeout = STRIKEOUT_DOUBLE;
                break;
        }
    }

    if (meWidth)
    {
        switch (*meWidth)
        {
            case os::strikethrough_width_t::bold:
                eStrikeout = STRIKEOUT_BOLD;
                break;
            default:;
        }
    }

    if (meText)
    {
        switch (*meText)
        {
            case os::strikethrough_text_t::slash:
                eStrikeout = STRIKEOUT_SLASH;
                break;
            case os::strikethrough_text_t::cross:
                eStrikeout = STRIKEOUT_X;
                break;
            case os::strikethrough_text_t::unknown:
                break;
        }
    }

    return eStrikeout;
}


void ScOrcusUnderline::reset()
{
    meStyle.reset();
    meThickness.reset();
    meSpacing.reset();
    meCount.reset();
}

std::optional<FontLineStyle> ScOrcusUnderline::toFontLineStyle() const
{
    std::optional<FontLineStyle> eUnderline;

    if (meStyle)
    {
        switch (*meStyle)
        {
            case os::underline_style_t::none:
                eUnderline = LINESTYLE_NONE;
                break;
            case os::underline_style_t::solid:
            {
                if (meCount)
                {
                    switch (*meCount)
                    {
                        case os::underline_count_t::single_count:
                            eUnderline = LINESTYLE_SINGLE;
                            break;
                        case os::underline_count_t::double_count:
                            eUnderline = LINESTYLE_DOUBLE;
                            break;
                        case os::underline_count_t::none:
                            break;
                    }
                }
                else
                    eUnderline = LINESTYLE_SINGLE;
                break;
            }
            case os::underline_style_t::dotted:
                eUnderline = LINESTYLE_DOTTED;
                break;
            case os::underline_style_t::dash:
                eUnderline = LINESTYLE_DASH;
                break;
            case os::underline_style_t::long_dash:
                eUnderline = LINESTYLE_LONGDASH;
                break;
            case os::underline_style_t::dot_dash:
                eUnderline = LINESTYLE_DASHDOT;
                break;
            case os::underline_style_t::dot_dot_dash:
                eUnderline = LINESTYLE_DASHDOTDOT;
                break;
            case os::underline_style_t::wave:
            {
                if (meCount)
                {
                    switch (*meCount)
                    {
                        case os::underline_count_t::single_count:
                            eUnderline = LINESTYLE_WAVE;
                            break;
                        case os::underline_count_t::double_count:
                            eUnderline = LINESTYLE_DOUBLEWAVE;
                            break;
                        case os::underline_count_t::none:
                            break;
                    }
                }
                else
                    eUnderline = LINESTYLE_WAVE;
                break;
            }
        }
    }

    bool bApplyBold = false;

    if (meThickness)
    {
        switch (*meThickness)
        {
            case os::underline_thickness_t::bold:
            case os::underline_thickness_t::thick:
                bApplyBold = true;
                break;
            default:;
        }
    }

    if (bApplyBold)
    {
        if (eUnderline)
        {
            switch (*eUnderline)
            {
                case LINESTYLE_NONE:
                case LINESTYLE_SINGLE:
                    eUnderline = LINESTYLE_BOLD;
                    break;
                case LINESTYLE_DOTTED:
                    eUnderline = LINESTYLE_BOLDDOTTED;
                    break;
                case LINESTYLE_DASH:
                    eUnderline = LINESTYLE_BOLDDASH;
                    break;
                case LINESTYLE_LONGDASH:
                    eUnderline = LINESTYLE_BOLDLONGDASH;
                    break;
                case LINESTYLE_DASHDOT:
                    eUnderline = LINESTYLE_BOLDDASHDOT;
                    break;
                case LINESTYLE_DASHDOTDOT:
                    eUnderline = LINESTYLE_BOLDDASHDOTDOT;
                    break;
                case LINESTYLE_WAVE:
                    eUnderline = LINESTYLE_BOLDWAVE;
                    break;
                default:
                    ;
            }
        }
    }

    return eUnderline;
}

void ScOrcusSegmentStrikethrough::reset(SfxItemSet* pDestFormat)
{
    mpDestFormat = pDestFormat;
    maAttrs.reset();
}

void ScOrcusSegmentStrikethrough::set_style(os::strikethrough_style_t s)
{
    maAttrs.meStyle = s;
}

void ScOrcusSegmentStrikethrough::set_type(os::strikethrough_type_t s)
{
    maAttrs.meType = s;
}

void ScOrcusSegmentStrikethrough::set_width(os::strikethrough_width_t s)
{
    maAttrs.meWidth = s;
}

void ScOrcusSegmentStrikethrough::set_text(os::strikethrough_text_t s)
{
    maAttrs.meText = s;
}

void ScOrcusSegmentStrikethrough::commit()
{
    auto eStrikeout = maAttrs.toFontStrikeout();
    if (!eStrikeout)
        return;

    mpDestFormat->Put(SvxCrossedOutItem(*eStrikeout, EE_CHAR_STRIKEOUT));
}

void ScOrcusSegmentUnderline::reset(SfxItemSet* pDestFormat)
{
    mpDestFormat = pDestFormat;
    maAttrs.reset();
    maColor.reset();
}

void ScOrcusSegmentUnderline::set_style(os::underline_style_t e)
{
    maAttrs.meStyle = e;
}

void ScOrcusSegmentUnderline::set_thickness(os::underline_thickness_t e)
{
    maAttrs.meThickness = e;
}

void ScOrcusSegmentUnderline::set_spacing(os::underline_spacing_t e)
{
    maAttrs.meSpacing = e;
}

void ScOrcusSegmentUnderline::set_count(os::underline_count_t e)
{
    maAttrs.meCount = e;
}

void ScOrcusSegmentUnderline::set_color(
    os::color_elem_t alpha, os::color_elem_t red, os::color_elem_t green, os::color_elem_t blue)
{
    maColor = Color(ColorAlpha, alpha, red, green, blue);
}

void ScOrcusSegmentUnderline::commit()
{
    auto eUnderline = maAttrs.toFontLineStyle();
    if (!eUnderline)
        return;

    SvxUnderlineItem aItem(*eUnderline, EE_CHAR_UNDERLINE);

    if (maColor)
        aItem.SetColor(*maColor);

    mpDestFormat->Put(aItem);
}

OUString ScOrcusSharedStrings::toOUString(std::string_view s)
{
    return {s.data(), sal_Int32(s.size()), mrFactory.getGlobalSettings().getTextEncoding()};
}

ScOrcusSharedStrings::ScOrcusSharedStrings(ScOrcusFactory& rFactory) :
    mrFactory(rFactory),
    mrEditEngine(rFactory.getDoc().getDoc().GetEditEngine()),
    maCurFormat(mrEditEngine.GetEmptyItemSet())
{
    mrEditEngine.Clear();
}

size_t ScOrcusSharedStrings::append(std::string_view s)
{
    return mrFactory.appendString(toOUString(s));
}

size_t ScOrcusSharedStrings::add(std::string_view s)
{
    return mrFactory.addString(toOUString(s));
}

void ScOrcusSharedStrings::set_segment_font(size_t /*font_index*/)
{
}

void ScOrcusSharedStrings::set_segment_bold(bool b)
{
    FontWeight eWeight = b ? WEIGHT_BOLD : WEIGHT_NORMAL;
    maCurFormat.Put(SvxWeightItem(eWeight, EE_CHAR_WEIGHT));
}

void ScOrcusSharedStrings::set_segment_italic(bool b)
{
    FontItalic eItalic = b ? ITALIC_NORMAL : ITALIC_NONE;
    maCurFormat.Put(SvxPostureItem(eItalic, EE_CHAR_ITALIC));
}

void ScOrcusSharedStrings::set_segment_superscript(bool b)
{
    if (b)
        maCurFormat.Put(SvxEscapementItem(SvxEscapement::Superscript, EE_CHAR_ESCAPEMENT));
}

void ScOrcusSharedStrings::set_segment_subscript(bool b)
{
    if (b)
        maCurFormat.Put(SvxEscapementItem(SvxEscapement::Subscript, EE_CHAR_ESCAPEMENT));
}

void ScOrcusSharedStrings::set_segment_font_name(std::string_view s)
{
    OUString aName = toOUString(s);
    maCurFormat.Put(
        SvxFontItem(
            FAMILY_DONTKNOW, aName, aName, PITCH_DONTKNOW,
            mrFactory.getGlobalSettings().getTextEncoding(),
            EE_CHAR_FONTINFO
        )
    );
}

void ScOrcusSharedStrings::set_segment_font_size(double point)
{
    // points to 100th of millimeters
    tools::Long nMM = o3tl::convert(point, o3tl::Length::pt, o3tl::Length::mm100);
    maCurFormat.Put(SvxFontHeightItem(nMM, 100, EE_CHAR_FONTHEIGHT));
}

void ScOrcusSharedStrings::set_segment_font_color(
    os::color_elem_t alpha, os::color_elem_t red, os::color_elem_t green, os::color_elem_t blue)
{
    Color aColor(ColorAlpha, alpha, red, green, blue);
    maCurFormat.Put(SvxColorItem(aColor, EE_CHAR_COLOR));
}

os::iface::import_underline* ScOrcusSharedStrings::start_underline()
{
    maImportUnderline.reset(&maCurFormat);
    return &maImportUnderline;
}

os::iface::import_strikethrough* ScOrcusSharedStrings::start_strikethrough()
{
    maImportStrikethrough.reset(&maCurFormat);
    return &maImportStrikethrough;
}

void ScOrcusSharedStrings::append_segment(std::string_view s)
{
    ESelection aSel{ mrEditEngine.GetEnd() }; // end of current text

    OUString aStr = toOUString(s);
    mrEditEngine.QuickInsertText(aStr, aSel);

    aSel.end = mrEditEngine.GetEnd(); // expand the selection over the current segment
    maFormatSegments.emplace_back(aSel, maCurFormat);
    maCurFormat.ClearItem();
}

size_t ScOrcusSharedStrings::commit_segments()
{
    for (const auto& [rSel, rFormat] : maFormatSegments)
        mrEditEngine.QuickSetAttribs(rFormat, rSel);

    auto nPos = mrFactory.appendFormattedString(mrEditEngine.CreateTextObject());
    mrEditEngine.Clear();
    maFormatSegments.clear();
    return nPos;
}

void ScOrcusFont::applyToItemSet( SfxItemSet& rSet ) const
{
    if (mbBold)
    {
        FontWeight eWeight = *mbBold ? WEIGHT_BOLD : WEIGHT_NORMAL;
        rSet.Put(SvxWeightItem(eWeight, ATTR_FONT_WEIGHT));
    }

    if (mbBoldAsian)
    {
        FontWeight eWeight = *mbBoldAsian ? WEIGHT_BOLD : WEIGHT_NORMAL;
        rSet.Put(SvxWeightItem(eWeight, ATTR_CJK_FONT_WEIGHT));
    }

    if (mbBoldComplex)
    {
        FontWeight eWeight = *mbBoldComplex ? WEIGHT_BOLD : WEIGHT_NORMAL;
        rSet.Put(SvxWeightItem(eWeight, ATTR_CTL_FONT_WEIGHT));
    }

    if (mbItalic)
    {
        FontItalic eItalic = *mbItalic ? ITALIC_NORMAL : ITALIC_NONE;
        rSet.Put(SvxPostureItem(eItalic, ATTR_FONT_POSTURE));
    }

    if (mbItalicAsian)
    {
        FontItalic eItalic = *mbItalicAsian ? ITALIC_NORMAL : ITALIC_NONE;
        rSet.Put(SvxPostureItem(eItalic, ATTR_CJK_FONT_POSTURE));
    }

    if (mbItalicComplex)
    {
        FontItalic eItalic = *mbItalicComplex ? ITALIC_NORMAL : ITALIC_NONE;
        rSet.Put(SvxPostureItem(eItalic, ATTR_CTL_FONT_POSTURE));
    }

    if (maColor)
        rSet.Put( SvxColorItem(*maColor, ATTR_FONT_COLOR));

    if (maName && !maName->isEmpty())
        rSet.Put( SvxFontItem( FAMILY_DONTKNOW, *maName, *maName, PITCH_DONTKNOW, RTL_TEXTENCODING_DONTKNOW, ATTR_FONT ));

    if (maNameAsian && !maNameAsian->isEmpty())
        rSet.Put( SvxFontItem( FAMILY_DONTKNOW, *maNameAsian, *maNameAsian, PITCH_DONTKNOW, RTL_TEXTENCODING_DONTKNOW, ATTR_CJK_FONT ));

    if (maNameComplex && !maNameComplex->isEmpty())
        rSet.Put( SvxFontItem( FAMILY_DONTKNOW, *maNameComplex, *maNameComplex, PITCH_DONTKNOW, RTL_TEXTENCODING_DONTKNOW, ATTR_CTL_FONT ));

    if (mnSize)
    {
        double fSize = translateToInternal(*mnSize, orcus::length_unit_t::point);
        rSet.Put(SvxFontHeightItem(fSize, 100, ATTR_FONT_HEIGHT));
    }

    if (mnSizeAsian)
    {
        double fSize = translateToInternal(*mnSizeAsian, orcus::length_unit_t::point);
        rSet.Put(SvxFontHeightItem(fSize, 100, ATTR_CJK_FONT_HEIGHT));
    }

    if (mnSizeComplex)
    {
        double fSize = translateToInternal(*mnSizeComplex, orcus::length_unit_t::point);
        rSet.Put(SvxFontHeightItem(fSize, 100, ATTR_CTL_FONT_HEIGHT));
    }

    if (meUnderline)
    {
        SvxUnderlineItem aUnderline(*meUnderline, ATTR_FONT_UNDERLINE);
        if (maUnderlineColor)
            // Separate color specified for the underline
            aUnderline.SetColor(*maUnderlineColor);
        else if (maColor)
            // Use font color
            aUnderline.SetColor(*maColor);
        rSet.Put(aUnderline);
    }

    if (meStrikeout)
        rSet.Put(SvxCrossedOutItem(*meStrikeout, ATTR_FONT_CROSSEDOUT));
}

void ScOrcusFill::applyToItemSet( SfxItemSet& rSet ) const
{
    if (!mePattern || !maFgColor)
        return;

    if (*mePattern == os::fill_pattern_t::solid)
        rSet.Put(SvxBrushItem(*maFgColor, ATTR_BACKGROUND));
}

void ScOrcusBorder::applyToItemSet( SfxItemSet& rSet ) const
{
    auto getDirection = [](os::border_direction_t dir) -> SvxBoxItemLine
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
                ;
        }
        return SvxBoxItemLine::RIGHT;
    };

    if (maBorders.empty())
        return;

    SvxBoxItem aBoxItem(ATTR_BORDER);
    SvxLineItem aDiagonal_TLBR(ATTR_BORDER_TLBR);
    SvxLineItem aDiagonal_BLTR(ATTR_BORDER_BLTR);

    for (const auto& [dir, attrs] : maBorders)
    {
        SvxBoxItemLine eDir = getDirection(dir);

        SvxBorderLineStyle eStyle = attrs.meStyle.value_or(SvxBorderLineStyle::SOLID);
        Color aColor = attrs.maColor.value_or(COL_BLACK);
        double nWidth = attrs.mnWidth.value_or(0.0);

        switch (dir)
        {
            case os::border_direction_t::diagonal_tl_br:
            {
                editeng::SvxBorderLine aLine(&aColor, nWidth, eStyle);
                aDiagonal_TLBR.SetLine(&aLine);
                break;
            }
            case os::border_direction_t::diagonal_bl_tr:
            {
                editeng::SvxBorderLine aLine(&aColor, nWidth, eStyle);
                aDiagonal_BLTR.SetLine(&aLine);
                break;
            }
            default:
            {
                editeng::SvxBorderLine aLine(&aColor, nWidth, eStyle);
                aBoxItem.SetLine(&aLine, eDir);
            }
        }
    }

    rSet.Put(aDiagonal_BLTR);
    rSet.Put(aDiagonal_TLBR);
    rSet.Put(aBoxItem);
}

void ScOrcusProtection::applyToItemSet( SfxItemSet& rSet ) const
{
    if (!mbLocked.has_value() && !mbHidden.has_value() && !mbPrintContent.has_value() && !mbFormulaHidden.has_value())
        return;

    bool bLocked = mbLocked.value_or(true); // defaults to true.
    bool bHidden = mbHidden.value_or(false);
    bool bFormulaHidden = mbFormulaHidden.value_or(false);
    bool bPrintContent = mbPrintContent.value_or(false);
    rSet.Put(ScProtectionAttr(bLocked, bFormulaHidden, bHidden, bPrintContent));
}

void ScOrcusNumberFormat::applyToItemSet( SfxItemSet& rSet, const ScDocument& rDoc ) const
{
    if (!maCode)
        return;

    sal_uInt32 nKey;
    sal_Int32 nCheckPos;
    SvNumberFormatter* pFormatter = rDoc.GetFormatTable();
    OUString Code = *maCode; /* <-- Done because the SvNumberFormatter::PutEntry demands a non const NumFormat Code*/
    SvNumFormatType type = SvNumFormatType::ALL;

    pFormatter->PutEntry(Code, nCheckPos, type, nKey, LANGUAGE_ENGLISH_US);
    if (!nCheckPos)
        rSet.Put(SfxUInt32Item(ATTR_VALUE_FORMAT, nKey));
}

ScOrcusXf::ScOrcusXf() :
    mnFontId(0),
    mnFillId(0),
    mnBorderId(0),
    mnProtectionId(0),
    mnNumberFormatId(0),
    mnStyleXf(0),
    mbApplyAlignment(false),
    mbWrapText(false),
    mbShrinkToFit(false),
    meHorAlignment(SvxCellHorJustify::Standard),
    meVerAlignment(SvxCellVerJustify::Standard),
    meHorAlignMethod(SvxCellJustifyMethod::Auto),
    meVerAlignMethod(SvxCellJustifyMethod::Auto)
{
}

ScOrcusCellStyle::ScOrcusCellStyle() :
    maParentName(SC_STYLE_PROG_STANDARD),
    mnXFId(0),
    mnBuiltInId(0)
{
}

void ScOrcusImportFontUnderlineStyle::reset(ScOrcusFont* pDest)
{
    mpDestFont = pDest;

    maAttrs.reset();
    maColor.reset();
}

void ScOrcusImportFontUnderlineStyle::set_style(os::underline_style_t e)
{
    maAttrs.meStyle = e;
}

void ScOrcusImportFontUnderlineStyle::set_thickness(os::underline_thickness_t e)
{
    maAttrs.meThickness = e;
}

void ScOrcusImportFontUnderlineStyle::set_spacing(os::underline_spacing_t e)
{
    maAttrs.meSpacing = e;
}

void ScOrcusImportFontUnderlineStyle::set_count(os::underline_count_t e)
{
    maAttrs.meCount = e;
}

void ScOrcusImportFontUnderlineStyle::set_color(
    os::color_elem_t alpha, os::color_elem_t red, os::color_elem_t green, os::color_elem_t blue)
{
    maColor = Color(ColorAlpha, alpha, red, green, blue);
}

void ScOrcusImportFontUnderlineStyle::commit()
{
    mpDestFont->meUnderline = maAttrs.toFontLineStyle();
    mpDestFont->maUnderlineColor = maColor;
}

void ScOrcusImportFontStrikethroughStyle::reset(ScOrcusFont* pDest)
{
    mpDestFont = pDest;
    maAttrs.reset();
}

void ScOrcusImportFontStrikethroughStyle::set_style(orcus::spreadsheet::strikethrough_style_t s)
{
    maAttrs.meStyle = s;
}

void ScOrcusImportFontStrikethroughStyle::set_type(orcus::spreadsheet::strikethrough_type_t s)
{
    maAttrs.meType = s;
}

void ScOrcusImportFontStrikethroughStyle::set_width(orcus::spreadsheet::strikethrough_width_t s)
{
    maAttrs.meWidth = s;
}

void ScOrcusImportFontStrikethroughStyle::set_text(orcus::spreadsheet::strikethrough_text_t s)
{
    maAttrs.meText = s;
}

void ScOrcusImportFontStrikethroughStyle::commit()
{
    mpDestFont->meStrikeout = maAttrs.toFontStrikeout();
}

ScOrcusImportFontStyle::ScOrcusImportFontStyle( ScOrcusFactory& rFactory, std::vector<ScOrcusFont>& rFonts ) :
    mrFactory(rFactory),
    mrFonts(rFonts)
{
}

void ScOrcusImportFontStyle::reset()
{
    maCurrentFont = ScOrcusFont();
}

void ScOrcusImportFontStyle::set_bold(bool b)
{
    maCurrentFont.mbBold = b;
}

void ScOrcusImportFontStyle::set_bold_asian(bool b)
{
    maCurrentFont.mbBoldAsian = b;
}

void ScOrcusImportFontStyle::set_bold_complex(bool b)
{
    maCurrentFont.mbBoldComplex = b;
}

void ScOrcusImportFontStyle::set_italic(bool b)
{
    maCurrentFont.mbItalic = b;
}

void ScOrcusImportFontStyle::set_italic_asian(bool b)
{
    maCurrentFont.mbItalicAsian = b;
}

void ScOrcusImportFontStyle::set_italic_complex(bool b)
{
    maCurrentFont.mbItalicComplex = b;
}

void ScOrcusImportFontStyle::set_name(std::string_view name)
{
    OUString aName(name.data(), name.size(), mrFactory.getGlobalSettings().getTextEncoding());
    maCurrentFont.maName = aName;
}

void ScOrcusImportFontStyle::set_name_asian(std::string_view name)
{
    OUString aName(name.data(), name.size(), mrFactory.getGlobalSettings().getTextEncoding());
    maCurrentFont.maNameAsian = aName;
}

void ScOrcusImportFontStyle::set_name_complex(std::string_view name)
{
    OUString aName(name.data(), name.size(), mrFactory.getGlobalSettings().getTextEncoding());
    maCurrentFont.maNameComplex = aName;
}

void ScOrcusImportFontStyle::set_size(double point)
{
    maCurrentFont.mnSize = point;
}

void ScOrcusImportFontStyle::set_size_asian(double point)
{
    maCurrentFont.mnSizeAsian = point;
}

void ScOrcusImportFontStyle::set_size_complex(double point)
{
    maCurrentFont.mnSizeComplex = point;
}

void ScOrcusImportFontStyle::set_color(
    os::color_elem_t alpha, os::color_elem_t red, os::color_elem_t green, os::color_elem_t blue)
{
    maCurrentFont.maColor = Color(ColorAlpha, alpha, red, green, blue);
}

orcus::spreadsheet::iface::import_underline* ScOrcusImportFontStyle::start_underline()
{
    maUnderlineImport.reset(&maCurrentFont);
    return &maUnderlineImport;
}

orcus::spreadsheet::iface::import_strikethrough* ScOrcusImportFontStyle::start_strikethrough()
{
    maStrikeoutImport.reset(&maCurrentFont);
    return &maStrikeoutImport;
}

std::size_t ScOrcusImportFontStyle::commit()
{
    SAL_INFO("sc.orcus.style", "commit font");
    mrFonts.push_back(maCurrentFont);
    maCurrentFont = ScOrcusFont();
    return mrFonts.size() - 1;
}

ScOrcusImportFillStyle::ScOrcusImportFillStyle( std::vector<ScOrcusFill>& rFills ) :
    mrFills(rFills)
{
}

void ScOrcusImportFillStyle::reset()
{
    maCurrentFill = ScOrcusFill();
}

void ScOrcusImportFillStyle::set_pattern_type(os::fill_pattern_t fp)
{
    maCurrentFill.mePattern = fp;
}

void ScOrcusImportFillStyle::set_fg_color(
    os::color_elem_t alpha, os::color_elem_t red, os::color_elem_t green, os::color_elem_t blue)
{
    maCurrentFill.maFgColor = Color(ColorAlpha, alpha, red, green, blue);
}

void ScOrcusImportFillStyle::set_bg_color(
    os::color_elem_t alpha, os::color_elem_t red, os::color_elem_t green, os::color_elem_t blue)
{
    maCurrentFill.maBgColor = Color(ColorAlpha, alpha, red, green, blue);
}

std::size_t ScOrcusImportFillStyle::commit()
{
    SAL_INFO("sc.orcus.style", "commit fill");
    mrFills.push_back(maCurrentFill);
    maCurrentFill = ScOrcusFill();
    return mrFills.size() - 1;
}

ScOrcusImportBorderStyle::ScOrcusImportBorderStyle( std::vector<ScOrcusBorder>& rBorders ) :
    mrBorders(rBorders)
{
}

void ScOrcusImportBorderStyle::set_style(
    os::border_direction_t dir, os::border_style_t style)
{
    ScOrcusBorder::BorderLine& rBorderLine = maCurrentBorder.maBorders[dir];

    switch (style)
    {
        case os::border_style_t::solid:
            rBorderLine.meStyle = SvxBorderLineStyle::SOLID;
            rBorderLine.mnWidth = oox::xls::API_LINE_THIN;
            break;
        case os::border_style_t::hair:
            rBorderLine.meStyle = SvxBorderLineStyle::SOLID;
            rBorderLine.mnWidth = oox::xls::API_LINE_HAIR;
            break;
        case os::border_style_t::medium:
            rBorderLine.meStyle = SvxBorderLineStyle::SOLID;
            rBorderLine.mnWidth = oox::xls::API_LINE_MEDIUM;
            break;
        case os::border_style_t::thick:
            rBorderLine.meStyle = SvxBorderLineStyle::SOLID;
            rBorderLine.mnWidth = oox::xls::API_LINE_THICK;
            break;
        case os::border_style_t::thin:
            rBorderLine.meStyle = SvxBorderLineStyle::SOLID;
            rBorderLine.mnWidth = oox::xls::API_LINE_THIN;
            break;
        case os::border_style_t::dash_dot:
            rBorderLine.meStyle = SvxBorderLineStyle::DASH_DOT;
            rBorderLine.mnWidth = oox::xls::API_LINE_THIN;
            break;
        case os::border_style_t::dash_dot_dot:
            rBorderLine.meStyle = SvxBorderLineStyle::DASH_DOT_DOT;
            rBorderLine.mnWidth = oox::xls::API_LINE_THIN;
            break;
        case os::border_style_t::dashed:
            rBorderLine.meStyle = SvxBorderLineStyle::DASHED;
            rBorderLine.mnWidth = oox::xls::API_LINE_THIN;
            break;
        case os::border_style_t::dotted:
            rBorderLine.meStyle = SvxBorderLineStyle::DOTTED;
            rBorderLine.mnWidth = oox::xls::API_LINE_THIN;
            break;
        case os::border_style_t::double_border:
            rBorderLine.meStyle = SvxBorderLineStyle::DOUBLE;
            rBorderLine.mnWidth = oox::xls::API_LINE_THICK;
            break;
        case os::border_style_t::medium_dash_dot:
        case os::border_style_t::slant_dash_dot:
            rBorderLine.meStyle = SvxBorderLineStyle::DASH_DOT;
            rBorderLine.mnWidth = oox::xls::API_LINE_MEDIUM;
            break;
        case os::border_style_t::medium_dash_dot_dot:
            rBorderLine.meStyle = SvxBorderLineStyle::DASH_DOT_DOT;
            rBorderLine.mnWidth = oox::xls::API_LINE_MEDIUM;
            break;
        case os::border_style_t::medium_dashed:
            rBorderLine.meStyle = SvxBorderLineStyle::DASHED;
            rBorderLine.mnWidth = oox::xls::API_LINE_MEDIUM;
            break;
        case os::border_style_t::unknown:
        case os::border_style_t::none:
            rBorderLine.mnWidth = oox::xls::API_LINE_NONE;
            break;
        default:
            ;
    }
}

void ScOrcusImportBorderStyle::set_color(
    os::border_direction_t dir, os::color_elem_t alpha, os::color_elem_t red,
    os::color_elem_t green, os::color_elem_t blue)
{
    ScOrcusBorder::BorderLine& rBorderLine = maCurrentBorder.maBorders[dir];
    rBorderLine.maColor = Color(ColorAlpha, alpha, red, green, blue);
}

void ScOrcusImportBorderStyle::reset()
{
    maCurrentBorder = ScOrcusBorder();
}

void ScOrcusImportBorderStyle::set_width(os::border_direction_t  dir, double val, orcus::length_unit_t unit)
{
    ScOrcusBorder::BorderLine& rBorderLine = maCurrentBorder.maBorders[dir];
    rBorderLine.mnWidth = translateToInternal(val, unit);
}

std::size_t ScOrcusImportBorderStyle::commit()
{
    SAL_INFO("sc.orcus.style", "commit border");
    mrBorders.push_back(maCurrentBorder);
    maCurrentBorder = ScOrcusBorder();
    return mrBorders.size() - 1;
}

ScOrcusImportCellProtection::ScOrcusImportCellProtection( std::vector<ScOrcusProtection>& rProtections ) :
    mrProtections(rProtections)
{
}

void ScOrcusImportCellProtection::reset()
{
    maCurrentProtection = ScOrcusProtection();
}

void ScOrcusImportCellProtection::set_hidden(bool b)
{
    maCurrentProtection.mbHidden = b;
}

void ScOrcusImportCellProtection::set_locked(bool b)
{
    maCurrentProtection.mbLocked = b;
}

void ScOrcusImportCellProtection::set_print_content(bool b )
{
    maCurrentProtection.mbPrintContent = b;
}

void ScOrcusImportCellProtection::set_formula_hidden(bool b )
{
    maCurrentProtection.mbFormulaHidden = b;
}

std::size_t ScOrcusImportCellProtection::commit()
{
    SAL_INFO("sc.orcus.style", "commit cell protection");
    mrProtections.push_back(maCurrentProtection);
    maCurrentProtection = ScOrcusProtection();
    return mrProtections.size() - 1;
}

ScOrcusImportNumberFormat::ScOrcusImportNumberFormat( ScOrcusFactory& rFactory, std::vector<ScOrcusNumberFormat>& rFormats ) :
    mrFactory(rFactory), mrNumberFormats(rFormats)
{
}

void ScOrcusImportNumberFormat::reset()
{
    maCurrentFormat = ScOrcusNumberFormat();
}

void ScOrcusImportNumberFormat::set_identifier(std::size_t /*id*/)
{
}

void ScOrcusImportNumberFormat::set_code(std::string_view s)
{
    OUString aCode(s.data(), s.size(), mrFactory.getGlobalSettings().getTextEncoding());
    maCurrentFormat.maCode = aCode;
}

std::size_t ScOrcusImportNumberFormat::commit()
{
    SAL_INFO("sc.orcus.style", "commit number format");
    mrNumberFormats.push_back(maCurrentFormat);
    maCurrentFormat = ScOrcusNumberFormat();
    return mrNumberFormats.size() - 1;
}

ScOrucsImportCellStyle::ScOrucsImportCellStyle(
    ScOrcusFactory& rFactory, ScOrcusStyles& rStyles, const std::vector<ScOrcusXf>& rXfs ) :
    mrFactory(rFactory),
    mrStyles(rStyles),
    mrXfs(rXfs)
{
}

void ScOrucsImportCellStyle::reset()
{
    maCurrentStyle = ScOrcusCellStyle();
}

void ScOrucsImportCellStyle::set_name(std::string_view name)
{
    maCurrentStyle.maName = OUString(name.data(), name.size(), mrFactory.getGlobalSettings().getTextEncoding());
}

void ScOrucsImportCellStyle::set_display_name(std::string_view name)
{
    maCurrentStyle.maDisplayName = OUString(name.data(), name.size(), mrFactory.getGlobalSettings().getTextEncoding());
}

void ScOrucsImportCellStyle::set_xf(size_t index)
{
    maCurrentStyle.mnXFId = index;
}

void ScOrucsImportCellStyle::set_builtin(size_t index)
{
    maCurrentStyle.mnBuiltInId = index;
}

void ScOrucsImportCellStyle::set_parent_name(std::string_view name)
{
    maCurrentStyle.maParentName = OUString(name.data(), name.size(), mrFactory.getGlobalSettings().getTextEncoding());
}

void ScOrucsImportCellStyle::commit()
{
    SAL_INFO("sc.orcus.style", "commit cell style: " << maCurrentStyle.maName);
    if (maCurrentStyle.mnXFId >= mrXfs.size())
    {
        SAL_WARN("sc.orcus.style", "invalid xf id for commit cell style");
        return;
    }

    if (maCurrentStyle.mnXFId == 0)
        return;

    ScStyleSheetPool* pPool = mrFactory.getDoc().getDoc().GetStyleSheetPool();
    SfxStyleSheetBase& rBase = pPool->Make(maCurrentStyle.maName, SfxStyleFamily::Para);
    // Need to convert the parent name to localized UI name, see tdf#139205.
    rBase.SetParent(
        ScStyleNameConversion::ProgrammaticToDisplayName(
            maCurrentStyle.maParentName, SfxStyleFamily::Para));

    SfxItemSet& rSet = rBase.GetItemSet();
    const ScOrcusXf& rXf = mrXfs[maCurrentStyle.mnXFId];
    mrStyles.applyXfToItemSet(rSet, rXf);

    maCurrentStyle = ScOrcusCellStyle();
}

void ScOrcusImportXf::reset( std::vector<ScOrcusXf>& rXfs )
{
    mpXfs = &rXfs;
    maCurrentXf = ScOrcusXf();
}

void ScOrcusImportXf::set_font(std::size_t index)
{
    maCurrentXf.mnFontId = index;
}

void ScOrcusImportXf::set_fill(std::size_t index)
{
    maCurrentXf.mnFillId = index;
}

void ScOrcusImportXf::set_border(std::size_t index)
{
    maCurrentXf.mnBorderId = index;
}

void ScOrcusImportXf::set_protection(std::size_t index)
{
    maCurrentXf.mnProtectionId = index;
}

void ScOrcusImportXf::set_number_format(std::size_t index)
{
    maCurrentXf.mnNumberFormatId = index;
}

void ScOrcusImportXf::set_style_xf(std::size_t index)
{
    maCurrentXf.mnStyleXf = index;
}

void ScOrcusImportXf::set_apply_alignment(bool b)
{
    maCurrentXf.mbApplyAlignment = b;
}

void ScOrcusImportXf::set_horizontal_alignment(os::hor_alignment_t align)
{
    switch (align)
    {
        case os::hor_alignment_t::left:
            maCurrentXf.meHorAlignment = SvxCellHorJustify::Left;
            break;
        case os::hor_alignment_t::right:
            maCurrentXf.meHorAlignment = SvxCellHorJustify::Right;
            break;
        case os::hor_alignment_t::center:
            maCurrentXf.meHorAlignment = SvxCellHorJustify::Center;
            break;
        case os::hor_alignment_t::justified:
            maCurrentXf.meHorAlignment = SvxCellHorJustify::Block;
            break;
        case os::hor_alignment_t::distributed:
            maCurrentXf.meHorAlignment = SvxCellHorJustify::Block;
            maCurrentXf.meHorAlignMethod = SvxCellJustifyMethod::Distribute;
            break;
        case os::hor_alignment_t::unknown:
            maCurrentXf.meHorAlignment = SvxCellHorJustify::Standard;
            break;
        default:
            ;
    }
    maCurrentXf.mbApplyAlignment = true;
}

void ScOrcusImportXf::set_vertical_alignment(os::ver_alignment_t align)
{
    switch (align)
    {
        case os::ver_alignment_t::top:
            maCurrentXf.meVerAlignment = SvxCellVerJustify::Top;
            break;
        case os::ver_alignment_t::bottom:
            maCurrentXf.meVerAlignment = SvxCellVerJustify::Bottom;
            break;
        case os::ver_alignment_t::middle:
            maCurrentXf.meVerAlignment = SvxCellVerJustify::Center;
            break;
        case os::ver_alignment_t::justified:
            maCurrentXf.meVerAlignment = SvxCellVerJustify::Block;
            break;
        case os::ver_alignment_t::distributed:
            maCurrentXf.meVerAlignment = SvxCellVerJustify::Block;
            maCurrentXf.meVerAlignMethod = SvxCellJustifyMethod::Distribute;
            break;
        case os::ver_alignment_t::unknown:
            maCurrentXf.meVerAlignment = SvxCellVerJustify::Standard;
            break;
        default:
            ;
    }
    maCurrentXf.mbApplyAlignment = true;
}

void ScOrcusImportXf::set_wrap_text(bool b)
{
    maCurrentXf.mbWrapText = b;
}

void ScOrcusImportXf::set_shrink_to_fit(bool b)
{
    maCurrentXf.mbShrinkToFit = b;
}

std::size_t ScOrcusImportXf::commit()
{
    mpXfs->push_back(maCurrentXf);
    return mpXfs->size() - 1;
}


ScOrcusStyles::ScOrcusStyles( ScOrcusFactory& rFactory, bool bSkipDefaultStyles ) :
    mrFactory(rFactory),
    maFontStyle(rFactory, maFonts),
    maFillStyle(maFills),
    maBorderStyle(maBorders),
    maCellProtection(maProtections),
    maNumberFormat(rFactory, maNumberFormats),
    maCellStyle(rFactory, *this, maCellStyleXfs)
{
    ScDocument& rDoc = rFactory.getDoc().getDoc();
    if (!bSkipDefaultStyles && !rDoc.GetStyleSheetPool()->HasStandardStyles())
        rDoc.GetStyleSheetPool()->CreateStandardStyles();
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

void ScOrcusStyles::applyXfToItemSet( SfxItemSet& rSet, const ScOrcusXf& rXf )
{
    size_t nFontId = rXf.mnFontId;
    if (nFontId >= maFonts.size())
    {
        SAL_WARN("sc.orcus.style", "invalid font id");
        return;
    }

    maFonts[nFontId].applyToItemSet(rSet);

    size_t nFillId = rXf.mnFillId;
    if (nFillId >= maFills.size())
    {
        SAL_WARN("sc.orcus.style", "invalid fill id");
        return;
    }

    maFills[nFillId].applyToItemSet(rSet);

    size_t nBorderId = rXf.mnBorderId;
    if (nBorderId >= maBorders.size())
    {
        SAL_WARN("sc.orcus.style", "invalid border id");
        return;
    }
    maBorders[nBorderId].applyToItemSet(rSet);

    size_t nProtectionId = rXf.mnProtectionId;
    if (nProtectionId >= maProtections.size())
    {
        SAL_WARN("sc.orcus.style", "invalid protection id");
        return;
    }

    maProtections[nProtectionId].applyToItemSet(rSet);

    size_t nNumberFormatId = rXf.mnNumberFormatId;
    if (nNumberFormatId >= maNumberFormats.size())
    {
        SAL_WARN("sc.orcus.style", "invalid number format id");
        return;
    }
    const ScOrcusNumberFormat& rFormat = maNumberFormats[nNumberFormatId];
    rFormat.applyToItemSet(rSet, mrFactory.getDoc().getDoc());

    if (rXf.mbApplyAlignment)
    {
        rSet.Put(SvxHorJustifyItem(rXf.meHorAlignment, ATTR_HOR_JUSTIFY));
        rSet.Put(SvxVerJustifyItem(rXf.meVerAlignment, ATTR_VER_JUSTIFY));
        rSet.Put(SvxJustifyMethodItem(rXf.meHorAlignMethod, ATTR_HOR_JUSTIFY_METHOD));
        rSet.Put(SvxJustifyMethodItem(rXf.meVerAlignMethod, ATTR_VER_JUSTIFY_METHOD));
    }

    if (rXf.mbWrapText)
        rSet.Put(ScLineBreakCell(*rXf.mbWrapText));

    if (rXf.mbShrinkToFit)
        rSet.Put(ScShrinkToFitCell(*rXf.mbShrinkToFit));
}

void ScOrcusStyles::applyXfToItemSet( SfxItemSet& rSet, std::size_t xfId )
{
    SAL_INFO("sc.orcus.style", "applyXfToitemSet: " << xfId);
    if (maCellXfs.size() <= xfId)
    {
        SAL_WARN("sc.orcus.style", "invalid xf id");
        return;
    }

    applyXfToItemSet(rSet, maCellXfs[xfId]);
}

os::iface::import_font_style* ScOrcusStyles::start_font_style()
{
    maFontStyle.reset();
    return &maFontStyle;
}

os::iface::import_fill_style* ScOrcusStyles::start_fill_style()
{
    maFillStyle.reset();
    return &maFillStyle;
}

os::iface::import_border_style* ScOrcusStyles::start_border_style()
{
    maBorderStyle.reset();
    return &maBorderStyle;
}

os::iface::import_cell_protection* ScOrcusStyles::start_cell_protection()
{
    maCellProtection.reset();
    return &maCellProtection;
}

os::iface::import_number_format* ScOrcusStyles::start_number_format()
{
    maNumberFormat.reset();
    return &maNumberFormat;
}

os::iface::import_xf* ScOrcusStyles::start_xf(os::xf_category_t cat)
{
    switch (cat)
    {
        case os::xf_category_t::cell:
            maXf.reset(maCellXfs);
            break;
        case os::xf_category_t::cell_style:
            maXf.reset(maCellStyleXfs);
            break;
        case os::xf_category_t::differential:
            maXf.reset(maCellDiffXfs);
            break;
        case os::xf_category_t::unknown:
            SAL_WARN("sc.orcus.style", "unknown xf category");
            return nullptr;
    }

    return &maXf;
}

os::iface::import_cell_style* ScOrcusStyles::start_cell_style()
{
    maCellStyle.reset();
    return &maCellStyle;
}

void ScOrcusStyles::set_font_count(size_t /*n*/)
{
}

void ScOrcusStyles::set_fill_count(size_t /*n*/)
{
}

void ScOrcusStyles::set_border_count(size_t /*n*/)
{
}

void ScOrcusStyles::set_number_format_count(size_t /*n*/)
{
}

void ScOrcusStyles::set_xf_count(os::xf_category_t /*cat*/, size_t /*n*/)
{
}

void ScOrcusStyles::set_cell_style_count(size_t /*n*/)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
