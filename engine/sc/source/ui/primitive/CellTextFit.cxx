/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <CellTextFit.hxx>

#include <attrib.hxx>
#include <cellvalue.hxx>
#include <document.hxx>
#include <formulacell.hxx>
#include <global.hxx>
#include <patattr.hxx>
#include <scitems.hxx>

#include <comphelper/string.hxx>
#include <formula/errorcodes.hxx>
#include <rtl/ustrbuf.hxx>
#include <svl/numformat.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <tools/long.hxx>
#include <unotools/localedatawrapper.hxx>

#include <algorithm>
#include <cmath>

namespace sc
{
namespace
{
/// The number of 10% steps a text shrinks by after the first step, so it ends at a bit less
/// than half the size of the first step.
constexpr sal_uInt16 constShrinkAgainMax = 7;

/// One copy of a short text measures imprecisely, so the width of a copy is taken from a sample
/// of this many characters.
constexpr sal_Int32 constSampleSize = 20;
}

bool isCellContentHidden(const ScDocument& rDocument, const ScAddress& rAddress, bool bTabProtected,
                         bool bShowFormulas, bool bPrint)
{
    if (!bPrint && !bTabProtected)
        return false;

    const ScProtectionAttr* pProtection
        = rDocument.GetEffItem(rAddress.Col(), rAddress.Row(), rAddress.Tab(), ATTR_PROTECTION);
    if (bPrint && pProtection->GetHidePrint())
        return true;
    if (!bTabProtected)
        return false;
    if (pProtection->GetHideCell())
        return true;
    return bShowFormulas && pProtection->GetHideFormula()
           && rDocument.GetCellType(rAddress) == CELLTYPE_FORMULA;
}

bool isAvailableForOverflow(ScDocument& rDocument, const ScAddress& rAddress, bool bTabProtected,
                            bool bShowFormulas, bool bPrint)
{
    ScRefCellValue aCell(rDocument, rAddress);
    if (!aCell.isEmpty()
        && !isCellContentHidden(rDocument, rAddress, bTabProtected, bShowFormulas, bPrint))
        return false;

    const ScPatternAttr* pPattern = rDocument.GetPattern(rAddress);
    return !(pPattern->GetItem(ATTR_MERGE).IsMerged()
             || pPattern->GetItem(ATTR_MERGE_FLAG).IsOverlapped());
}

bool isAmbiguousScript(SvtScriptType nScript)
{
    return nScript != SvtScriptType::LATIN && nScript != SvtScriptType::ASIAN
           && nScript != SvtScriptType::COMPLEX;
}

std::optional<OUString> fillWithRepeatCharacter(const OUString& rText, sal_Int32 nRepeatPosition,
                                                sal_Unicode cRepeat, double fSpace,
                                                double fMinWidth, const TextMeasure& rMeasure)
{
    if (nRepeatPosition < 0 || nRepeatPosition > rText.getLength())
        return std::nullopt;

    OUStringBuffer aFill(constSampleSize);
    comphelper::string::padToLength(aFill, constSampleSize, cRepeat);
    const double fAverageWidth = rMeasure(aFill.makeStringAndClear()) / constSampleSize;

    // The width of one character counts in whole units, rounded toward zero.
    const double fCharacterWidth = std::trunc(fAverageWidth);
    if (fCharacterWidth < std::max(1.0, fMinWidth) || fSpace <= fCharacterWidth)
        return std::nullopt;

    const sal_Int32 nInsert = static_cast<sal_Int32>(fSpace / fAverageWidth);
    aFill.ensureCapacity(nInsert);
    comphelper::string::padToLength(aFill, nInsert, cRepeat);
    return rText.replaceAt(nRepeatPosition, 0, aFill);
}

std::optional<OUString> repeatToWidth(std::u16string_view rText, double fTextWidth,
                                      double fAvailable, double fExtraWidth,
                                      const TextMeasure& rMeasure)
{
    double fRepeatWidth = fTextWidth;
    const sal_Int32 nLength = static_cast<sal_Int32>(rText.size());
    if (nLength && nLength < constSampleSize)
    {
        OUStringBuffer aSample(rText);
        aSample.ensureCapacity(constSampleSize);
        while (aSample.getLength() < constSampleSize)
            aSample.append(rText);
        fRepeatWidth = rMeasure(aSample.copy(0, constSampleSize).makeStringAndClear()) * nLength
                       / constSampleSize;
    }
    fRepeatWidth += fExtraWidth;
    if (fRepeatWidth <= 0)
        return std::nullopt;

    const tools::Long nRepeatCount = static_cast<tools::Long>(fAvailable / fRepeatWidth);
    if (nRepeatCount <= 1)
        return std::nullopt;

    OUStringBuffer aRepeated(rText);
    for (tools::Long nRepeat = 1; nRepeat < nRepeatCount; ++nRepeat)
        aRepeated.append(rText);
    return aRepeated.makeStringAndClear();
}

bool shrinkToFit(double fAvailable, const std::function<double()>& rFirstStep,
                 const std::function<double()>& rNextStep)
{
    double fSize = rFirstStep();
    for (sal_uInt16 nStep = 0; fSize > fAvailable && nStep < constShrinkAgainMax; ++nStep)
        fSize = rNextStep();
    return fSize <= fAvailable;
}

NumberFit fitNumberToWidth(ScRefCellValue aCell, sal_uInt32 nFormat, bool bShowFormulas,
                           ScDocument& rDocument, double fWidth, const NumberWidths& rWidths,
                           const TextMeasure& rMeasure, OUString& rText)
{
    const CellType eType = aCell.getType();
    if (eType != CELLTYPE_VALUE && eType != CELLTYPE_FORMULA)
        return NumberFit::Unchanged;

    if (eType == CELLTYPE_FORMULA)
    {
        ScFormulaCell* pFormulaCell = aCell.getFormula();
        // An error that does not fit always shows "###", and so does a formula shown as text
        // (#i116691#).
        if (pFormulaCell->GetErrCode() != FormulaError::NONE || bShowFormulas)
            return NumberFit::Hash;
        if (!pFormulaCell->IsValue())
            return NumberFit::Unchanged;
    }

    // Only the General format can write a number with fewer digits.
    if ((nFormat % SV_COUNTRY_LANGUAGE_OFFSET) != 0)
        return NumberFit::Hash;

    const double fValue = aCell.getValue();
    SvNumberFormatter* pFormatter = rDocument.GetFormatTable();
    const SvNumberformat* pNumberFormat = pFormatter->GetEntry(nFormat);
    if (!pNumberFormat)
        return NumberFit::Unchanged;

    const double fMaxDigit = rWidths.maMaxDigit();
    if (fMaxDigit <= 0)
        return NumberFit::Unchanged;

    auto digitsFor = [fMaxDigit](double fRoom)
    { return static_cast<sal_uInt16>(static_cast<tools::Long>(fRoom / fMaxDigit)); };
    if (!pNumberFormat->GetOutputString(fValue, digitsFor(fWidth), rText, pFormatter->GetNatNum()))
        return NumberFit::Failed;

    sal_Int32 nSignCount = 0;
    sal_Int32 nDecimalCount = 0;
    sal_Int32 nExponentCount = 0;
    const sal_Unicode cDecimalSeparator
        = ScGlobal::getLocaleData().getLocaleItem().decimalSeparator[0];
    for (sal_Int32 nIndex = 0; nIndex < rText.getLength(); ++nIndex)
    {
        const sal_Unicode cCharacter = rText[nIndex];
        if (cCharacter == '-')
            ++nSignCount;
        else if (cCharacter == cDecimalSeparator)
            ++nDecimalCount;
        else if (cCharacter == 'E')
            ++nExponentCount;
    }

    // #i112250# A small value can be written as "0" when only the digits are counted, but fit
    // with the narrower decimal separator.
    if (rText == "0" && fValue != 0.0)
        nDecimalCount = 1;

    // The separator, the sign and the exponent are narrower than a digit, which leaves room for
    // more digits.
    if (nDecimalCount)
        fWidth += (fMaxDigit - rWidths.maDecimalSeparator()) * nDecimalCount;
    if (nSignCount)
        fWidth += (fMaxDigit - rWidths.maSign()) * nSignCount;
    if (nExponentCount)
        fWidth += (fMaxDigit - rWidths.maExponent()) * nExponentCount;

    if ((nDecimalCount || nSignCount || nExponentCount)
        && !pNumberFormat->GetOutputString(fValue, digitsFor(fWidth), rText,
                                           pFormatter->GetNatNum()))
        return NumberFit::Failed;

    return rMeasure(rText) > fWidth ? NumberFit::Hash : NumberFit::Fits;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
