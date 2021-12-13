/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <queryevaluator.hxx>

#include <cellform.hxx>
#include <cellvalue.hxx>
#include <colorscale.hxx>
#include <document.hxx>
#include <docoptio.hxx>
#include <queryparam.hxx>
#include <scitems.hxx>
#include <table.hxx>

#include <editeng/brushitem.hxx>
#include <editeng/colritem.hxx>
#include <svl/numformat.hxx>
#include <svl/sharedstringpool.hxx>
#include <svl/zformat.hxx>
#include <unotools/collatorwrapper.hxx>

bool ScQueryEvaluator::isPartialTextMatchOp(const ScQueryEntry& rEntry)
{
    switch (rEntry.eOp)
    {
        // these operators can only be used with textural comparisons.
        case SC_CONTAINS:
        case SC_DOES_NOT_CONTAIN:
        case SC_BEGINS_WITH:
        case SC_ENDS_WITH:
        case SC_DOES_NOT_BEGIN_WITH:
        case SC_DOES_NOT_END_WITH:
            return true;
        default:;
    }
    return false;
}

bool ScQueryEvaluator::isTextMatchOp(const ScQueryEntry& rEntry)
{
    if (isPartialTextMatchOp(rEntry))
        return true;

    switch (rEntry.eOp)
    {
        // these operators can be used for either textural or value comparison.
        case SC_EQUAL:
        case SC_NOT_EQUAL:
            return true;
        default:;
    }
    return false;
}

void ScQueryEvaluator::setupTransliteratorIfNeeded()
{
    if (!mpTransliteration)
        mpTransliteration = &ScGlobal::GetTransliteration(mrParam.bCaseSens);
}

void ScQueryEvaluator::setupCollatorIfNeeded()
{
    if (!mpCollator)
        mpCollator = &ScGlobal::GetCollator(mrParam.bCaseSens);
}

ScQueryEvaluator::ScQueryEvaluator(ScDocument& rDoc, const ScTable& rTab,
                                   const ScQueryParam& rParam, const ScInterpreterContext* pContext,
                                   bool* pTestEqualCondition)
    : mrDoc(rDoc)
    , mrStrPool(rDoc.GetSharedStringPool())
    , mrTab(rTab)
    , mrParam(rParam)
    , mpTestEqualCondition(pTestEqualCondition)
    , mpTransliteration(nullptr)
    , mpCollator(nullptr)
    , mbMatchWholeCell(rDoc.GetDocOptions().IsMatchWholeCell())
    , mbCaseSensitive(rParam.bCaseSens)
    , mpContext(pContext)
    , mnEntryCount(mrParam.GetEntryCount())
{
    if (mnEntryCount <= nFixedBools)
    {
        mpPasst = &maBool[0];
        mpTest = &maTest[0];
    }
    else
    {
        mpBoolDynamic.reset(new bool[mnEntryCount]);
        mpTestDynamic.reset(new bool[mnEntryCount]);
        mpPasst = mpBoolDynamic.get();
        mpTest = mpTestDynamic.get();
    }
}

bool ScQueryEvaluator::isRealWildOrRegExp(const ScQueryEntry& rEntry) const
{
    if (mrParam.eSearchType == utl::SearchParam::SearchType::Normal)
        return false;

    return isTextMatchOp(rEntry);
}

bool ScQueryEvaluator::isTestWildOrRegExp(const ScQueryEntry& rEntry) const
{
    if (!mpTestEqualCondition)
        return false;

    if (mrParam.eSearchType == utl::SearchParam::SearchType::Normal)
        return false;

    return (rEntry.eOp == SC_LESS_EQUAL || rEntry.eOp == SC_GREATER_EQUAL);
}

bool ScQueryEvaluator::isQueryByValue(const ScQueryEntry& rEntry, const ScQueryEntry::Item& rItem,
                                      const ScRefCellValue& rCell)
{
    if (rItem.meType == ScQueryEntry::ByString || isPartialTextMatchOp(rEntry))
        return false;

    return isQueryByValueForCell(rCell);
}

bool ScQueryEvaluator::isQueryByValueForCell(const ScRefCellValue& rCell)
{
    if (rCell.meType == CELLTYPE_FORMULA && rCell.mpFormula->GetErrCode() != FormulaError::NONE)
        // Error values are compared as string.
        return false;

    return rCell.hasNumeric();
}

bool ScQueryEvaluator::isQueryByString(const ScQueryEntry& rEntry, const ScQueryEntry::Item& rItem,
                                       const ScRefCellValue& rCell)
{
    if (isTextMatchOp(rEntry))
        return true;

    if (rItem.meType != ScQueryEntry::ByString)
        return false;

    return rCell.hasString();
}

sal_uInt32 ScQueryEvaluator::getNumFmt(SCCOL nCol, SCROW nRow)
{
    sal_uInt32 nNumFmt
        = (mpContext ? mrTab.GetNumberFormat(*mpContext, ScAddress(nCol, nRow, mrTab.GetTab()))
                     : mrTab.GetNumberFormat(nCol, nRow));
    if (nNumFmt && (nNumFmt % SV_COUNTRY_LANGUAGE_OFFSET) == 0)
        // Any General of any locale is irrelevant for rounding.
        nNumFmt = 0;
    return nNumFmt;
}

std::pair<bool, bool> ScQueryEvaluator::compareByValue(const ScRefCellValue& rCell, SCCOL nCol,
                                                       SCROW nRow, const ScQueryEntry& rEntry,
                                                       const ScQueryEntry::Item& rItem)
{
    bool bOk = false;
    bool bTestEqual = false;
    double nCellVal;
    double fQueryVal = rItem.mfVal;
    // Defer all number format detection to as late as possible as it's a
    // bottle neck, even if that complicates the code. Also do not
    // unnecessarily call ScDocument::RoundValueAsShown() for the same
    // reason.
    sal_uInt32 nNumFmt = NUMBERFORMAT_ENTRY_NOT_FOUND;

    switch (rCell.meType)
    {
        case CELLTYPE_VALUE:
            nCellVal = rCell.mfValue;
            break;
        case CELLTYPE_FORMULA:
            nCellVal = rCell.mpFormula->GetValue();
            break;
        default:
            nCellVal = 0.0;
    }
    if (rItem.mbRoundForFilter && nCellVal != 0.0)
    {
        nNumFmt = getNumFmt(nCol, nRow);
        if (nNumFmt)
        {
            switch (rCell.meType)
            {
                case CELLTYPE_VALUE:
                case CELLTYPE_FORMULA:
                    nCellVal = mrDoc.RoundValueAsShown(nCellVal, nNumFmt, mpContext);
                    break;
                default:
                    assert(!"can't be");
            }
        }
    }

    /* NOTE: lcl_PrepareQuery() prepares a filter query such that if a
     * date+time format was queried rEntry.bQueryByDate is not set. In
     * case other queries wanted to use this mechanism they should do
     * the same, in other words only if rEntry.nVal is an integer value
     * rEntry.bQueryByDate should be true and the time fraction be
     * stripped here. */

    if (rItem.meType == ScQueryEntry::ByDate)
    {
        if (nNumFmt == NUMBERFORMAT_ENTRY_NOT_FOUND)
            nNumFmt = getNumFmt(nCol, nRow);
        if (nNumFmt)
        {
            SvNumberFormatter* pFormatter
                = mpContext ? mpContext->GetFormatTable() : mrDoc.GetFormatTable();
            const SvNumberformat* pEntry = pFormatter->GetEntry(nNumFmt);
            if (pEntry)
            {
                SvNumFormatType nNumFmtType = pEntry->GetType();
                /* NOTE: Omitting the check for absence of
                 * css::util::NumberFormat::TIME would include also date+time formatted
                 * values of the same day. That may be desired in some
                 * cases, querying all time values of a day, but confusing
                 * in other cases. A user can always setup a standard
                 * filter query for x >= date AND x < date+1 */
                if ((nNumFmtType & SvNumFormatType::DATE) && !(nNumFmtType & SvNumFormatType::TIME))
                {
                    // The format is of date type.  Strip off the time
                    // element.
                    nCellVal = ::rtl::math::approxFloor(nCellVal);
                }
            }
        }
    }

    switch (rEntry.eOp)
    {
        case SC_EQUAL:
            bOk = ::rtl::math::approxEqual(nCellVal, fQueryVal);
            break;
        case SC_LESS:
            bOk = (nCellVal < fQueryVal) && !::rtl::math::approxEqual(nCellVal, fQueryVal);
            break;
        case SC_GREATER:
            bOk = (nCellVal > fQueryVal) && !::rtl::math::approxEqual(nCellVal, fQueryVal);
            break;
        case SC_LESS_EQUAL:
            bOk = (nCellVal < fQueryVal) || ::rtl::math::approxEqual(nCellVal, fQueryVal);
            if (bOk && mpTestEqualCondition)
                bTestEqual = ::rtl::math::approxEqual(nCellVal, fQueryVal);
            break;
        case SC_GREATER_EQUAL:
            bOk = (nCellVal > fQueryVal) || ::rtl::math::approxEqual(nCellVal, fQueryVal);
            if (bOk && mpTestEqualCondition)
                bTestEqual = ::rtl::math::approxEqual(nCellVal, fQueryVal);
            break;
        case SC_NOT_EQUAL:
            bOk = !::rtl::math::approxEqual(nCellVal, fQueryVal);
            break;
        default:
            assert(false);
            break;
    }

    return std::pair<bool, bool>(bOk, bTestEqual);
}

OUString ScQueryEvaluator::getCellString(const ScRefCellValue& rCell, SCROW nRow,
                                         const ScQueryEntry& rEntry,
                                         const svl::SharedString** sharedString)
{
    if (rCell.meType == CELLTYPE_FORMULA && rCell.mpFormula->GetErrCode() != FormulaError::NONE)
    {
        // Error cell is evaluated as string (for now).
        const FormulaError error = rCell.mpFormula->GetErrCode();
        auto it = mCachedSharedErrorStrings.find(error);
        if (it == mCachedSharedErrorStrings.end())
        {
            svl::SharedString str = mrStrPool.intern(ScGlobal::GetErrorString(error));
            auto pos = mCachedSharedErrorStrings.insert({ error, str });
            assert(pos.second); // inserted
            it = pos.first;
        }
        *sharedString = &it->second;
        return OUString();
    }
    else if (rCell.meType == CELLTYPE_STRING)
    {
        *sharedString = rCell.mpString;
        return OUString();
    }
    else
    {
        sal_uInt32 nFormat
            = mpContext
                  ? mrTab.GetNumberFormat(*mpContext, ScAddress(static_cast<SCCOL>(rEntry.nField),
                                                                nRow, mrTab.GetTab()))
                  : mrTab.GetNumberFormat(static_cast<SCCOL>(rEntry.nField), nRow);
        SvNumberFormatter* pFormatter
            = mpContext ? mpContext->GetFormatTable() : mrDoc.GetFormatTable();
        return ScCellFormat::GetInputString(rCell, nFormat, *pFormatter, mrDoc, sharedString,
                                            rEntry.bDoQuery);
    }
}

bool ScQueryEvaluator::isFastCompareByString(const ScQueryEntry& rEntry) const
{
    // If this is true, then there's a fast path in compareByString() which
    // can be selected using the template argument to get fast code
    // that will not check the same conditions every time. This makes a difference
    // in fast lookups that search for an exact value (case sensitive or not).
    bool bMatchWholeCell = mbMatchWholeCell;
    if (isPartialTextMatchOp(rEntry))
        // may have to do partial textural comparison.
        bMatchWholeCell = false;

    const bool bRealWildOrRegExp = isRealWildOrRegExp(rEntry);
    const bool bTestWildOrRegExp = isTestWildOrRegExp(rEntry);

    // SC_EQUAL is part of isTextMatchOp(rEntry)
    return rEntry.eOp == SC_EQUAL && !bRealWildOrRegExp && !bTestWildOrRegExp && bMatchWholeCell;
}

// The value is placed inside one parameter: [pValueSource1] or [pValueSource2] but never in both.
// For the template argument see isFastCompareByString().
template <bool bFast>
std::pair<bool, bool> ScQueryEvaluator::compareByString(const ScQueryEntry& rEntry,
                                                        const ScQueryEntry::Item& rItem,
                                                        const svl::SharedString* pValueSource1,
                                                        const OUString* pValueSource2)
{
    bool bOk = false;
    bool bTestEqual = false;
    bool bMatchWholeCell;
    if (bFast)
        bMatchWholeCell = true;
    else
    {
        bMatchWholeCell = mbMatchWholeCell;
        if (isPartialTextMatchOp(rEntry))
            // may have to do partial textural comparison.
            bMatchWholeCell = false;
    }

    const bool bRealWildOrRegExp = !bFast && isRealWildOrRegExp(rEntry);
    const bool bTestWildOrRegExp = !bFast && isTestWildOrRegExp(rEntry);

    assert(!bFast || pValueSource1 != nullptr); // shared string for fast path
    // [pValueSource1] or [pValueSource2] but never both of them or none of them
    assert((pValueSource1 != nullptr) != (pValueSource2 != nullptr));

    if (!bFast && (bRealWildOrRegExp || bTestWildOrRegExp))
    {
        const OUString& rValue = pValueSource1 ? pValueSource1->getString() : *pValueSource2;

        sal_Int32 nStart = 0;
        sal_Int32 nEnd = rValue.getLength();

        // from 614 on, nEnd is behind the found text
        bool bMatch = false;
        if (rEntry.eOp == SC_ENDS_WITH || rEntry.eOp == SC_DOES_NOT_END_WITH)
        {
            nEnd = 0;
            nStart = rValue.getLength();
            bMatch
                = rEntry.GetSearchTextPtr(mrParam.eSearchType, mrParam.bCaseSens, bMatchWholeCell)
                      ->SearchBackward(rValue, &nStart, &nEnd);
        }
        else
        {
            bMatch
                = rEntry.GetSearchTextPtr(mrParam.eSearchType, mrParam.bCaseSens, bMatchWholeCell)
                      ->SearchForward(rValue, &nStart, &nEnd);
        }
        if (bMatch && bMatchWholeCell && (nStart != 0 || nEnd != rValue.getLength()))
            bMatch = false; // RegExp must match entire cell string
        if (bRealWildOrRegExp)
        {
            switch (rEntry.eOp)
            {
                case SC_EQUAL:
                case SC_CONTAINS:
                    bOk = bMatch;
                    break;
                case SC_NOT_EQUAL:
                case SC_DOES_NOT_CONTAIN:
                    bOk = !bMatch;
                    break;
                case SC_BEGINS_WITH:
                    bOk = (bMatch && (nStart == 0));
                    break;
                case SC_DOES_NOT_BEGIN_WITH:
                    bOk = !(bMatch && (nStart == 0));
                    break;
                case SC_ENDS_WITH:
                    bOk = (bMatch && (nEnd == rValue.getLength()));
                    break;
                case SC_DOES_NOT_END_WITH:
                    bOk = !(bMatch && (nEnd == rValue.getLength()));
                    break;
                default:
                    assert(false);
                    break;
            }
        }
        else
            bTestEqual = bMatch;
    }
    if (bFast || !bRealWildOrRegExp)
    {
        // Simple string matching i.e. no regexp match.
        if (bFast || isTextMatchOp(rEntry))
        {
            // Check this even with bFast.
            if (rItem.meType != ScQueryEntry::ByString && rItem.maString.isEmpty())
            {
                // #i18374# When used from functions (match, countif, sumif, vlookup, hlookup, lookup),
                // the query value is assigned directly, and the string is empty. In that case,
                // don't find any string (isEqual would find empty string results in formula cells).
                bOk = false;
                if (rEntry.eOp == SC_NOT_EQUAL)
                    bOk = !bOk;
            }
            else if (bFast || bMatchWholeCell)
            {
                if (bFast || pValueSource1)
                {
                    // Fast string equality check by comparing string identifiers.
                    // This is the bFast path, all conditions should lead here on bFast == true.
                    if (mrParam.bCaseSens)
                    {
                        bOk = pValueSource1->getData() == rItem.maString.getData();
                    }
                    else
                    {
                        bOk = pValueSource1->getDataIgnoreCase()
                              == rItem.maString.getDataIgnoreCase();
                    }
                }
                else // if (pValueSource2)
                {
                    if (mrParam.bCaseSens)
                    {
                        bOk = (*pValueSource2 == rItem.maString.getString());
                    }
                    else
                    {
                        // fallback
                        const svl::SharedString rSource2(mrStrPool.intern(*pValueSource2));
                        // Fast string equality check by comparing string identifiers.
                        bOk = rSource2.getDataIgnoreCase() == rItem.maString.getDataIgnoreCase();
                    }
                }

                if (!bFast && rEntry.eOp == SC_NOT_EQUAL)
                    bOk = !bOk;
            }
            else
            {
                // Where do we find a match (if at all)
                sal_Int32 nStrPos;

                if (!mbCaseSensitive)
                { // Common case for vlookup etc.
                    const svl::SharedString rSource(
                        pValueSource1 ? *pValueSource1 : mrStrPool.intern(*pValueSource2));

                    const rtl_uString* pQuer = rItem.maString.getDataIgnoreCase();
                    const rtl_uString* pCellStr = rSource.getDataIgnoreCase();

                    assert(pQuer != nullptr);
                    assert(pCellStr != nullptr);

                    const sal_Int32 nIndex
                        = (rEntry.eOp == SC_ENDS_WITH || rEntry.eOp == SC_DOES_NOT_END_WITH)
                              ? (pCellStr->length - pQuer->length)
                              : 0;

                    if (nIndex < 0)
                        nStrPos = -1;
                    else
                    { // OUString::indexOf
                        nStrPos = rtl_ustr_indexOfStr_WithLength(pCellStr->buffer + nIndex,
                                                                 pCellStr->length - nIndex,
                                                                 pQuer->buffer, pQuer->length);

                        if (nStrPos >= 0)
                            nStrPos += nIndex;
                    }
                }
                else
                {
                    const OUString& rValue
                        = pValueSource1 ? pValueSource1->getString() : *pValueSource2;
                    const OUString aQueryStr = rItem.maString.getString();
                    const LanguageType nLang
                        = ScGlobal::oSysLocale->GetLanguageTag().getLanguageType();
                    setupTransliteratorIfNeeded();
                    const OUString aCell(mpTransliteration->transliterate(
                        rValue, nLang, 0, rValue.getLength(), nullptr));

                    const OUString aQuer(mpTransliteration->transliterate(
                        aQueryStr, nLang, 0, aQueryStr.getLength(), nullptr));

                    const sal_Int32 nIndex
                        = (rEntry.eOp == SC_ENDS_WITH || rEntry.eOp == SC_DOES_NOT_END_WITH)
                              ? (aCell.getLength() - aQuer.getLength())
                              : 0;
                    nStrPos = ((nIndex < 0) ? -1 : aCell.indexOf(aQuer, nIndex));
                }
                switch (rEntry.eOp)
                {
                    case SC_EQUAL:
                    case SC_CONTAINS:
                        bOk = (nStrPos != -1);
                        break;
                    case SC_NOT_EQUAL:
                    case SC_DOES_NOT_CONTAIN:
                        bOk = (nStrPos == -1);
                        break;
                    case SC_BEGINS_WITH:
                        bOk = (nStrPos == 0);
                        break;
                    case SC_DOES_NOT_BEGIN_WITH:
                        bOk = (nStrPos != 0);
                        break;
                    case SC_ENDS_WITH:
                        bOk = (nStrPos >= 0);
                        break;
                    case SC_DOES_NOT_END_WITH:
                        bOk = (nStrPos < 0);
                        break;
                    default:
                        assert(false);
                        break;
                }
            }
        }
        else
        { // use collator here because data was probably sorted
            const OUString& rValue = pValueSource1 ? pValueSource1->getString() : *pValueSource2;
            setupCollatorIfNeeded();
            sal_Int32 nCompare = mpCollator->compareString(rValue, rItem.maString.getString());
            switch (rEntry.eOp)
            {
                case SC_LESS:
                    bOk = (nCompare < 0);
                    break;
                case SC_GREATER:
                    bOk = (nCompare > 0);
                    break;
                case SC_LESS_EQUAL:
                    bOk = (nCompare <= 0);
                    if (bOk && mpTestEqualCondition && !bTestEqual)
                        bTestEqual = (nCompare == 0);
                    break;
                case SC_GREATER_EQUAL:
                    bOk = (nCompare >= 0);
                    if (bOk && mpTestEqualCondition && !bTestEqual)
                        bTestEqual = (nCompare == 0);
                    break;
                default:
                    assert(false);
                    break;
            }
        }
    }

    return std::pair<bool, bool>(bOk, bTestEqual);
}

std::pair<bool, bool> ScQueryEvaluator::compareByTextColor(SCCOL nCol, SCROW nRow,
                                                           const ScQueryEntry::Item& rItem)
{
    ScAddress aPos(nCol, nRow, mrTab.GetTab());
    Color color;
    bool bHasConditionalColor = false;
    // Text color can be set via conditional formatting - check that first
    const ScPatternAttr* pPattern = mrDoc.GetPattern(nCol, nRow, mrTab.GetTab());
    if (pPattern)
    {
        if (!pPattern->GetItem(ATTR_CONDITIONAL).GetCondFormatData().empty())
        {
            const SfxItemSet* pCondSet = mrDoc.GetCondResult(nCol, nRow, mrTab.GetTab());
            const SvxColorItem* pColor = &pPattern->GetItem(ATTR_FONT_COLOR, pCondSet);
            color = pColor->GetValue();
            bHasConditionalColor = true;
        }
    }

    if (!bHasConditionalColor)
    {
        const SvxColorItem* pColor = mrDoc.GetAttr(aPos, ATTR_FONT_COLOR);
        color = pColor->GetValue();
    }

    bool bMatch = rItem.maColor == color;
    return std::pair<bool, bool>(bMatch, false);
}

std::pair<bool, bool> ScQueryEvaluator::compareByBackgroundColor(SCCOL nCol, SCROW nRow,
                                                                 const ScQueryEntry::Item& rItem)
{
    ScAddress aPos(nCol, nRow, mrTab.GetTab());
    Color color;

    // Background color can be set via conditional formatting - check that first
    bool bHasConditionalColor = false;
    const ScPatternAttr* pPattern = mrDoc.GetPattern(nCol, nRow, mrTab.GetTab());
    if (pPattern)
    {
        if (!pPattern->GetItem(ATTR_CONDITIONAL).GetCondFormatData().empty())
        {
            const SfxItemSet* pCondSet = mrDoc.GetCondResult(nCol, nRow, mrTab.GetTab());
            const SvxBrushItem* pBackgroundColor = &pPattern->GetItem(ATTR_BACKGROUND, pCondSet);
            color = pBackgroundColor->GetColor();
            bHasConditionalColor = true;
        }
    }

    ScConditionalFormat* pCondFormat = mrDoc.GetCondFormat(nCol, nRow, mrTab.GetTab());
    if (pCondFormat)
    {
        for (size_t i = 0; i < pCondFormat->size(); i++)
        {
            auto aEntry = pCondFormat->GetEntry(i);
            if (aEntry->GetType() == ScFormatEntry::Type::Colorscale)
            {
                const ScColorScaleFormat* pColFormat
                    = static_cast<const ScColorScaleFormat*>(aEntry);
                color = *(pColFormat->GetColor(aPos));
                bHasConditionalColor = true;
            }
        }
    }

    if (!bHasConditionalColor)
    {
        const SvxBrushItem* pBrush = mrDoc.GetAttr(aPos, ATTR_BACKGROUND);
        color = pBrush->GetColor();
    }

    bool bMatch = rItem.maColor == color;
    return std::pair<bool, bool>(bMatch, false);
}

// To be called only if both isQueryByValue() and isQueryByString()
// returned false and range lookup is wanted! In range lookup comparison
// numbers are less than strings. Nothing else is compared.
std::pair<bool, bool> ScQueryEvaluator::compareByRangeLookup(const ScRefCellValue& rCell,
                                                             const ScQueryEntry& rEntry,
                                                             const ScQueryEntry::Item& rItem)
{
    bool bTestEqual = false;

    if (rItem.meType == ScQueryEntry::ByString && rEntry.eOp != SC_LESS
        && rEntry.eOp != SC_LESS_EQUAL)
        return std::pair<bool, bool>(false, bTestEqual);

    if (rItem.meType != ScQueryEntry::ByString && rEntry.eOp != SC_GREATER
        && rEntry.eOp != SC_GREATER_EQUAL)
        return std::pair<bool, bool>(false, bTestEqual);

    if (rItem.meType == ScQueryEntry::ByString)
    {
        if (rCell.meType == CELLTYPE_FORMULA && rCell.mpFormula->GetErrCode() != FormulaError::NONE)
            // Error values are compared as string.
            return std::pair<bool, bool>(false, bTestEqual);

        return std::pair<bool, bool>(rCell.hasNumeric(), bTestEqual);
    }

    return std::pair<bool, bool>(!rCell.hasNumeric(), bTestEqual);
}

std::pair<bool, bool> ScQueryEvaluator::processEntry(SCROW nRow, SCCOL nCol, ScRefCellValue& aCell,
                                                     const ScQueryEntry& rEntry, size_t nEntryIndex)
{
    std::pair<bool, bool> aRes(false, false);
    const ScQueryEntry::QueryItemsType& rItems = rEntry.GetQueryItems();
    if (rItems.size() == 1 && rItems.front().meType == ScQueryEntry::ByEmpty)
    {
        if (rEntry.IsQueryByEmpty())
            aRes.first = aCell.isEmpty();
        else
        {
            assert(rEntry.IsQueryByNonEmpty());
            aRes.first = !aCell.isEmpty();
        }
        return aRes;
    }
    if (rEntry.eOp == SC_EQUAL && rItems.size() >= 10)
    {
        // If there are many items to query for (autofilter does this), then try to search
        // efficiently in those items. So first search all the items of the relevant type,
        // If that does not find anything, fall back to the generic code.
        double value = 0;
        bool valid = true;
        // For ScQueryEntry::ByValue check that the cell either is a value or is a formula
        // that has a value and is not an error (those are compared as strings). This
        // is basically simplified isQueryByValue().
        if (aCell.meType == CELLTYPE_VALUE)
            value = aCell.mfValue;
        else if (aCell.meType == CELLTYPE_FORMULA
                 && aCell.mpFormula->GetErrCode() != FormulaError::NONE
                 && aCell.mpFormula->IsValue())
        {
            value = aCell.mpFormula->GetValue();
        }
        else
            valid = false;
        if (valid)
        {
            if (rItems.size() >= 100)
            {
                // Sort, cache and binary search for the value in items.
                // Don't bother comparing approximately.
                if (mCachedSortedItemValues.size() <= nEntryIndex)
                {
                    mCachedSortedItemValues.resize(nEntryIndex + 1);
                    auto& values = mCachedSortedItemValues[nEntryIndex];
                    values.reserve(rItems.size());
                    for (const auto& rItem : rItems)
                        if (rItem.meType == ScQueryEntry::ByValue)
                            values.push_back(rItem.mfVal);
                    std::sort(values.begin(), values.end());
                }
                auto& values = mCachedSortedItemValues[nEntryIndex];
                auto it = std::lower_bound(values.begin(), values.end(), value);
                if (it != values.end() && *it == value)
                    return std::make_pair(true, true);
            }
            else
            {
                for (const auto& rItem : rItems)
                {
                    // For speed don't bother comparing approximately here, usually there either
                    // will be an exact match or it wouldn't match anyway.
                    if (rItem.meType == ScQueryEntry::ByValue && value == rItem.mfVal)
                    {
                        return std::make_pair(true, true);
                    }
                }
            }
        }
    }
    const svl::SharedString* cellSharedString = nullptr;
    OUString cellString;
    bool cellStringSet = false;
    const bool bFastCompareByString = isFastCompareByString(rEntry);
    if (rEntry.eOp == SC_EQUAL && rItems.size() >= 10 && bFastCompareByString)
    {
        // The same as above but for strings. Try to optimize the case when
        // it's a svl::SharedString comparison. That happens when SC_EQUAL is used
        // and simple matching is used, see compareByString()
        if (!cellStringSet)
        {
            cellString = getCellString(aCell, nRow, rEntry, &cellSharedString);
            cellStringSet = true;
        }
        // Allow also checking ScQueryEntry::ByValue if the cell is not numeric,
        // as in that case isQueryByNumeric() would be false and isQueryByString() would
        // be true because of SC_EQUAL making isTextMatchOp() true.
        bool compareByValue = !isQueryByValueForCell(aCell);
        // For ScQueryEntry::ByString check that the cell is represented by a shared string,
        // which means it's either a string cell or a formula error. This is not as
        // generous as isQueryByString() but it should be enough and better be safe.
        if (cellSharedString != nullptr)
        {
            if (rItems.size() >= 100)
            {
                // Sort, cache and binary search for the string in items.
                // Since each SharedString is identified by pointer value,
                // sorting by pointer value is enough.
                if (mCachedSortedItemStrings.size() <= nEntryIndex)
                {
                    mCachedSortedItemStrings.resize(nEntryIndex + 1);
                    auto& values = mCachedSortedItemStrings[nEntryIndex];
                    values.reserve(rItems.size());
                    for (const auto& rItem : rItems)
                    {
                        if (rItem.meType == ScQueryEntry::ByString
                            || (compareByValue && rItem.meType == ScQueryEntry::ByValue))
                        {
                            values.push_back(mrParam.bCaseSens
                                                 ? rItem.maString.getData()
                                                 : rItem.maString.getDataIgnoreCase());
                        }
                    }
                    std::sort(values.begin(), values.end());
                }
                auto& values = mCachedSortedItemStrings[nEntryIndex];
                const rtl_uString* string = mrParam.bCaseSens
                                                ? cellSharedString->getData()
                                                : cellSharedString->getDataIgnoreCase();
                auto it = std::lower_bound(values.begin(), values.end(), string);
                if (it != values.end() && *it == string)
                    return std::make_pair(true, true);
            }
            else
            {
                for (const auto& rItem : rItems)
                {
                    if ((rItem.meType == ScQueryEntry::ByString
                         || (compareByValue && rItem.meType == ScQueryEntry::ByValue))
                        && (mrParam.bCaseSens
                                ? cellSharedString->getData() == rItem.maString.getData()
                                : cellSharedString->getDataIgnoreCase()
                                      == rItem.maString.getDataIgnoreCase()))
                    {
                        return std::make_pair(true, true);
                    }
                }
            }
        }
    }
    // Generic handling.
    for (const auto& rItem : rItems)
    {
        if (rItem.meType == ScQueryEntry::ByTextColor)
        {
            std::pair<bool, bool> aThisRes = compareByTextColor(nCol, nRow, rItem);
            aRes.first |= aThisRes.first;
            aRes.second |= aThisRes.second;
        }
        else if (rItem.meType == ScQueryEntry::ByBackgroundColor)
        {
            std::pair<bool, bool> aThisRes = compareByBackgroundColor(nCol, nRow, rItem);
            aRes.first |= aThisRes.first;
            aRes.second |= aThisRes.second;
        }
        else if (isQueryByValue(rEntry, rItem, aCell))
        {
            std::pair<bool, bool> aThisRes = compareByValue(aCell, nCol, nRow, rEntry, rItem);
            aRes.first |= aThisRes.first;
            aRes.second |= aThisRes.second;
        }
        else if (isQueryByString(rEntry, rItem, aCell))
        {
            if (!cellStringSet)
            {
                cellString = getCellString(aCell, nRow, rEntry, &cellSharedString);
                cellStringSet = true;
            }
            std::pair<bool, bool> aThisRes;
            if (cellSharedString && bFastCompareByString) // fast
                aThisRes = compareByString<true>(rEntry, rItem, cellSharedString, nullptr);
            else if (cellSharedString)
                aThisRes = compareByString(rEntry, rItem, cellSharedString, nullptr);
            else
                aThisRes = compareByString(rEntry, rItem, nullptr, &cellString);
            aRes.first |= aThisRes.first;
            aRes.second |= aThisRes.second;
        }
        else if (mrParam.mbRangeLookup)
        {
            std::pair<bool, bool> aThisRes = compareByRangeLookup(aCell, rEntry, rItem);
            aRes.first |= aThisRes.first;
            aRes.second |= aThisRes.second;
        }

        if (aRes.first && (aRes.second || mpTestEqualCondition == nullptr))
            break;
    }
    return aRes;
}

bool ScQueryEvaluator::ValidQuery(SCROW nRow, const ScRefCellValue* pCell,
                                  sc::TableColumnBlockPositionSet* pBlockPos)
{
    if (!mrParam.GetEntry(0).bDoQuery)
        return true;

    tools::Long nPos = -1;
    ScQueryParam::const_iterator it, itBeg = mrParam.begin(), itEnd = mrParam.end();
    for (it = itBeg; it != itEnd && (*it)->bDoQuery; ++it)
    {
        const ScQueryEntry& rEntry = **it;

        // Short-circuit the test at the end of the loop - if this is SC_AND
        // and the previous value is false, this value will not be needed.
        // Disable this if pbTestEqualCondition is present as that one may get set
        // even if the result is false (that also means pTest doesn't need to be
        // handled here).
        if (rEntry.eConnect == SC_AND && mpTestEqualCondition == nullptr && nPos != -1
            && !mpPasst[nPos])
        {
            continue;
        }

        SCCOL nCol = static_cast<SCCOL>(rEntry.nField);

        // We can only handle one single direct query passed as a known pCell,
        // subsequent queries have to obtain the cell.
        ScRefCellValue aCell;
        if (pCell && it == itBeg)
            aCell = *pCell;
        else if (pBlockPos)
        { // hinted mdds access
            aCell = const_cast<ScTable&>(mrTab).GetCellValue(
                nCol, *pBlockPos->getBlockPosition(nCol), nRow);
        }
        else
            aCell = mrTab.GetCellValue(nCol, nRow);

        std::pair<bool, bool> aRes = processEntry(nRow, nCol, aCell, rEntry, it - itBeg);

        if (nPos == -1)
        {
            nPos++;
            mpPasst[nPos] = aRes.first;
            mpTest[nPos] = aRes.second;
        }
        else
        {
            if (rEntry.eConnect == SC_AND)
            {
                mpPasst[nPos] = mpPasst[nPos] && aRes.first;
                mpTest[nPos] = mpTest[nPos] && aRes.second;
            }
            else
            {
                nPos++;
                mpPasst[nPos] = aRes.first;
                mpTest[nPos] = aRes.second;
            }
        }
    }

    for (tools::Long j = 1; j <= nPos; j++)
    {
        mpPasst[0] = mpPasst[0] || mpPasst[j];
        mpTest[0] = mpTest[0] || mpTest[j];
    }

    bool bRet = mpPasst[0];
    if (mpTestEqualCondition)
        *mpTestEqualCondition = mpTest[0];

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
