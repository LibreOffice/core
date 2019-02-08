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

#include <tools/color.hxx>

#include <tools/debug.hxx>
#include <i18nlangtag/mslangid.hxx>

#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <svl/currencytable.hxx>

#include <svx/numfmtsh.hxx>
#include <svx/flagsdef.hxx>
#include <svx/tbcontrl.hxx>

#include <limits>

namespace
{
double GetDefaultValNum(const SvNumFormatType nType)
{
    switch (nType)
    {
        case SvNumFormatType::NUMBER:
            return fSvxNumValConst[SvxNumValCategory::Standard];
        case SvNumFormatType::CURRENCY:
            return fSvxNumValConst[SvxNumValCategory::Currency];
        case SvNumFormatType::PERCENT:
            return fSvxNumValConst[SvxNumValCategory::Percent];
        case SvNumFormatType::DATE:
        case SvNumFormatType::DATETIME:
            return fSvxNumValConst[SvxNumValCategory::Date];
        case SvNumFormatType::TIME:
            return fSvxNumValConst[SvxNumValCategory::Time];
        case SvNumFormatType::SCIENTIFIC:
            return fSvxNumValConst[SvxNumValCategory::Scientific];
        case SvNumFormatType::FRACTION:
            return fSvxNumValConst[SvxNumValCategory::Fraction];
        case SvNumFormatType::LOGICAL:
            return fSvxNumValConst[SvxNumValCategory::Boolean];
        default:
        break;
    }
    return fSvxNumValConst[SvxNumValCategory::NoValue];
}
}

SvxNumberFormatShell* SvxNumberFormatShell::Create(SvNumberFormatter* pNumFormatter,
                                                   sal_uInt32 nFormatKey,
                                                   SvxNumberValueType eNumValType,
                                                   const OUString& rNumStr)
{
    return new SvxNumberFormatShell(pNumFormatter, nFormatKey, eNumValType, rNumStr);
}

SvxNumberFormatShell* SvxNumberFormatShell::Create(SvNumberFormatter* pNumFormatter,
                                                   sal_uInt32 nFormatKey,
                                                   SvxNumberValueType eNumValType, double nNumVal,
                                                   const OUString* pNumStr)
{
    return new SvxNumberFormatShell(pNumFormatter, nFormatKey, eNumValType, nNumVal, pNumStr);
}

SvxNumberFormatShell::SvxNumberFormatShell(SvNumberFormatter* pNumFormatter, sal_uInt32 nFormatKey,
                                           SvxNumberValueType eNumValType, const OUString& rNumStr)
    : pFormatter(pNumFormatter)
    , pCurFmtTable(nullptr)
    , eValType(eNumValType)
    , bUndoAddList(true)
    , nCurFormatKey(nFormatKey)
    , nCurCategory(SvNumFormatType::ALL)
    , eCurLanguage(LANGUAGE_NONE)
    , pCurCurrencyEntry(nullptr)
    , bBankingSymbol(false)
    , nCurCurrencyEntryPos(sal_uInt16(SELPOS_NONE))
    , bUseStarFormat(false)
    , bIsDefaultValNum(true)
{
    nValNum = 0;

    switch (eValType)
    {
        case SvxNumberValueType::String:
            aValStr = rNumStr;
        break;
        case SvxNumberValueType::Number:
            if (pFormatter)
            {
                nValNum = GetDefaultValNum(pFormatter->GetType(nCurFormatKey));
            }
            [[fallthrough]];
        case SvxNumberValueType::Undefined:
        default:
            aValStr.clear();
    }
}

SvxNumberFormatShell::SvxNumberFormatShell(SvNumberFormatter* pNumFormatter, sal_uInt32 nFormatKey,
                                           SvxNumberValueType eNumValType, double nNumVal,
                                           const OUString* pNumStr)
    : pFormatter(pNumFormatter)
    , pCurFmtTable(nullptr)
    , eValType(eNumValType)
    , bUndoAddList(true)
    , nCurFormatKey(nFormatKey)
    , nCurCategory(SvNumFormatType::ALL)
    , eCurLanguage(LANGUAGE_NONE)
    , pCurCurrencyEntry(nullptr)
    , bBankingSymbol(false)
    , nCurCurrencyEntryPos(sal_uInt16(SELPOS_NONE))
    , bUseStarFormat(false)
    , bIsDefaultValNum(false)
{
    //  #50441# When used in Writer, the SvxNumberInfoItem contains the
    //  original string in addition to the value

    if (pNumStr)
        aValStr = *pNumStr;

    switch (eValType)
    {
        case SvxNumberValueType::Number:
            nValNum = nNumVal;
        break;
        case SvxNumberValueType::String:
        case SvxNumberValueType::Undefined:
        default:
            nValNum = 0;
            bIsDefaultValNum = true;
    }
}

SvxNumberFormatShell::~SvxNumberFormatShell()
{
    /*
     * At this point, depending on whether the added user-defined were
     * validated (ValidateNewEntries()), the add list is removed from
     * the number formatter again.
     *
     * Deleting formats from the formatter happens for Undo reasons
     * only in the calling instance.
     */

    if (bUndoAddList)
    {
        // Added formats are invalid => remove them

        for (const auto& rItem : aAddList)
            pFormatter->DeleteEntry(rItem);
    }
}

std::vector<sal_uInt32> const& SvxNumberFormatShell::GetUpdateData() const { return aDelList; }

void SvxNumberFormatShell::CategoryChanged(sal_uInt16 nCatLbPos, short& rFmtSelPos,
                                           std::vector<OUString>& rFmtEntries)
{
    SvNumFormatType nOldCategory = nCurCategory;
    PosToCategory_Impl(nCatLbPos, nCurCategory);
    pCurFmtTable = &(pFormatter->GetEntryTable(nCurCategory, nCurFormatKey, eCurLanguage));
    // reinitialize currency if category newly entered
    if (nCurCategory == SvNumFormatType::CURRENCY && nOldCategory != nCurCategory)
        pCurCurrencyEntry = nullptr;
    rFmtSelPos = FillEntryList_Impl(rFmtEntries);
}

void SvxNumberFormatShell::LanguageChanged(LanguageType eLangType, short& rFmtSelPos,
                                           std::vector<OUString>& rFmtEntries)
{
    eCurLanguage = eLangType;
    pCurFmtTable = &(pFormatter->ChangeCL(nCurCategory, nCurFormatKey, eCurLanguage));
    rFmtSelPos = FillEntryList_Impl(rFmtEntries);
}

void SvxNumberFormatShell::FormatChanged(sal_uInt16 nFmtLbPos, OUString& rPreviewStr,
                                         Color*& rpFontColor)
{
    if (static_cast<size_t>(nFmtLbPos) < aCurEntryList.size())
    {
        nCurFormatKey = aCurEntryList[nFmtLbPos];

        if (nCurFormatKey != NUMBERFORMAT_ENTRY_NOT_FOUND)
        {
            GetPreviewString_Impl(rPreviewStr, rpFontColor);
        }
        else if (nCurCategory == SvNumFormatType::CURRENCY)
        {
            if (static_cast<size_t>(nFmtLbPos) < aCurrencyFormatList.size())
            {
                MakePrevStringFromVal(aCurrencyFormatList[nFmtLbPos], rPreviewStr, rpFontColor, nValNum);
            }
        }
    }
}

bool SvxNumberFormatShell::AddFormat(OUString& rFormat, sal_Int32& rErrPos,
                                     sal_uInt16& rCatLbSelPos, short& rFmtSelPos,
                                     std::vector<OUString>& rFmtEntries)
{
    bool bInserted = false;
    sal_uInt32 nAddKey = pFormatter->GetEntryKey(rFormat, eCurLanguage);

    if (nAddKey != NUMBERFORMAT_ENTRY_NOT_FOUND) // exists already?
    {
        ::std::vector<sal_uInt32>::iterator nAt = GetRemoved_Impl(nAddKey);
        if (nAt != aDelList.end())
        {
            aDelList.erase(nAt);
            bInserted = true;
        }
        else
        {
            OSL_FAIL("duplicate format!");
        }
    }
    else // new format
    {
        sal_Int32 nPos;
        bInserted = pFormatter->PutEntry(rFormat, nPos, nCurCategory, nAddKey, eCurLanguage);
        rErrPos = (nPos >= 0) ? nPos : -1;

        if (bInserted)
        {
            // May be sorted under a different locale if LCID was parsed.
            const SvNumberformat* pEntry = pFormatter->GetEntry(nAddKey);
            if (pEntry)
            {
                LanguageType nLang = pEntry->GetLanguage();
                if (eCurLanguage != nLang)
                {
                    // Current language's list would not show entry, adapt.
                    eCurLanguage = nLang;
                }
            }
        }
    }

    if (bInserted)
    {
        nCurFormatKey = nAddKey;
        DBG_ASSERT(GetAdded_Impl(nCurFormatKey) == aAddList.end(), "duplicate format!");
        aAddList.push_back(nCurFormatKey);

        // get current table
        pCurFmtTable = &(pFormatter->GetEntryTable(nCurCategory, nCurFormatKey, eCurLanguage));
        nCurCategory = pFormatter->GetType(nAddKey);
        CategoryToPos_Impl(nCurCategory, rCatLbSelPos);
        rFmtSelPos = FillEntryList_Impl(rFmtEntries);
    }
    else if (rErrPos != 0) // syntax error
    {
        ;
    }
    else // insert twice not possible
    {
        OSL_FAIL("duplicate format!");
    }

    return bInserted;
}

void SvxNumberFormatShell::RemoveFormat(const OUString& rFormat, sal_uInt16& rCatLbSelPos,
                                        short& rFmtSelPos, std::vector<OUString>& rFmtEntries)
{
    sal_uInt32 nDelKey = pFormatter->GetEntryKey(rFormat, eCurLanguage);

    DBG_ASSERT(nDelKey != NUMBERFORMAT_ENTRY_NOT_FOUND, "entry not found!");
    DBG_ASSERT(!IsRemoved_Impl(nDelKey), "entry already removed!");

    if ((nDelKey != NUMBERFORMAT_ENTRY_NOT_FOUND) && !IsRemoved_Impl(nDelKey))
    {
        aDelList.push_back(nDelKey);

        ::std::vector<sal_uInt32>::iterator nAt = GetAdded_Impl(nDelKey);
        if (nAt != aAddList.end())
        {
            aAddList.erase(nAt);
        }

        nCurCategory = pFormatter->GetType(nDelKey);
        pCurFmtTable = &(pFormatter->GetEntryTable(nCurCategory, nCurFormatKey, eCurLanguage));

        nCurFormatKey = pFormatter->GetStandardFormat(nCurCategory, eCurLanguage);

        CategoryToPos_Impl(nCurCategory, rCatLbSelPos);
        rFmtSelPos = FillEntryList_Impl(rFmtEntries);
    }
}

void SvxNumberFormatShell::MakeFormat(OUString& rFormat, bool bThousand, bool bNegRed,
                                      sal_uInt16 nPrecision, sal_uInt16 nLeadingZeroes,
                                      sal_uInt16 nCurrencyPos)
{
    if (aCurrencyFormatList.size() > static_cast<size_t>(nCurrencyPos))
    {
        sal_Int32 rErrPos = 0;
        std::vector<OUString> aFmtEList;

        sal_uInt32 nFound = pFormatter->TestNewString(aCurrencyFormatList[nCurrencyPos], eCurLanguage);

        if (nFound == NUMBERFORMAT_ENTRY_NOT_FOUND)
        {
            sal_uInt16 rCatLbSelPos = 0;
            short rFmtSelPos = 0;
            AddFormat(aCurrencyFormatList[nCurrencyPos], rErrPos, rCatLbSelPos, rFmtSelPos, aFmtEList);
        }

        if (rErrPos == 0)
        {
            rFormat = pFormatter->GenerateFormat(nCurFormatKey, eCurLanguage, bThousand, bNegRed,
                                                 nPrecision, nLeadingZeroes);
        }
    }
    else
    {
        rFormat = pFormatter->GenerateFormat(nCurFormatKey, eCurLanguage, bThousand, bNegRed,
                                             nPrecision, nLeadingZeroes);
    }
}

sal_uInt16 SvxNumberFormatShell::GetFormatIntegerDigits(const OUString& rFormat) const
{
    sal_uInt32 nFmtKey = pFormatter->GetEntryKey(rFormat, eCurLanguage);

    return pFormatter->GetFormatIntegerDigits(nFmtKey);
}

void SvxNumberFormatShell::GetOptions(const OUString& rFormat, bool& rThousand, bool& rNegRed,
                                      sal_uInt16& rPrecision, sal_uInt16& rLeadingZeroes,
                                      sal_uInt16& rCatLbPos)
{
    sal_uInt32 nFmtKey = pFormatter->GetEntryKey(rFormat, eCurLanguage);

    if (nFmtKey != NUMBERFORMAT_ENTRY_NOT_FOUND)
    {
        pFormatter->GetFormatSpecialInfo(nFmtKey, rThousand, rNegRed, rPrecision, rLeadingZeroes);

        CategoryToPos_Impl(pFormatter->GetType(nFmtKey), rCatLbPos);
    }
    else
    {
        bool bTestBanking = false;
        sal_uInt16 nPos = FindCurrencyTableEntry(rFormat, bTestBanking);

        if (IsInTable(nPos, bTestBanking, rFormat)
                && pFormatter->GetFormatSpecialInfo(rFormat, rThousand, rNegRed, rPrecision,
                    rLeadingZeroes, eCurLanguage) == 0)
        {
            rCatLbPos = CAT_CURRENCY;
        }
        else
            rCatLbPos = CAT_USERDEFINED;
    }
}

void SvxNumberFormatShell::MakePreviewString(const OUString& rFormatStr, OUString& rPreviewStr,
                                             Color*& rpFontColor)
{
    rpFontColor = nullptr;

    sal_uIntPtr nExistingFormat = pFormatter->GetEntryKey(rFormatStr, eCurLanguage);
    if (nExistingFormat == NUMBERFORMAT_ENTRY_NOT_FOUND)
    {
        //  real preview - not implemented in NumberFormatter for text formats
        pFormatter->GetPreviewString(rFormatStr, nValNum, rPreviewStr, &rpFontColor, eCurLanguage, bUseStarFormat);
    }
    else
    {
        //  format exists

        //  #50441# if a string was set in addition to the value, use it for text formats
        bool bUseText = (eValType == SvxNumberValueType::String
                || (!aValStr.isEmpty() && (pFormatter->GetType(nExistingFormat) & SvNumFormatType::TEXT)));

        if (bUseText)
        {
            pFormatter->GetOutputString(aValStr, nExistingFormat, rPreviewStr, &rpFontColor);
        }
        else
        {
            if (bIsDefaultValNum)
                nValNum = GetDefaultValNum(pFormatter->GetType(nExistingFormat));
            pFormatter->GetOutputString(nValNum, nExistingFormat, rPreviewStr, &rpFontColor, bUseStarFormat);
        }
    }
}

bool SvxNumberFormatShell::IsUserDefined(const OUString& rFmtString)
{
    sal_uInt32 nFound = pFormatter->GetEntryKey(rFmtString, eCurLanguage);

    bool bFlag = false;
    if (nFound != NUMBERFORMAT_ENTRY_NOT_FOUND)
    {
        bFlag = pFormatter->IsUserDefined(rFmtString, eCurLanguage);

        if (bFlag)
        {
            const SvNumberformat* pNumEntry = pFormatter->GetEntry(nFound);

            if (pNumEntry != nullptr && pNumEntry->HasNewCurrency())
            {
                bool bTestBanking;
                sal_uInt16 nPos = FindCurrencyTableEntry(rFmtString, bTestBanking);
                bFlag = !IsInTable(nPos, bTestBanking, rFmtString);
            }
        }
    }
    return bFlag;
}

bool SvxNumberFormatShell::FindEntry(const OUString& rFmtString, sal_uInt32* pAt /* = NULL */)
{
    bool bRes = false;

    sal_uInt32 nFound = NUMBERFORMAT_ENTRY_NOT_FOUND;
    // There may be multiple builtin entries with the same format code, first
    // try if the current key matches.
    const SvNumberformat* pEntry = pFormatter->GetEntry(nCurFormatKey);
    if (pEntry && pEntry->GetLanguage() == eCurLanguage && pEntry->GetFormatstring() == rFmtString)
        nFound = nCurFormatKey;

    if (nFound == NUMBERFORMAT_ENTRY_NOT_FOUND)
        // Find the first matching format code.
        nFound = pFormatter->TestNewString(rFmtString, eCurLanguage);

    if (nFound == NUMBERFORMAT_ENTRY_NOT_FOUND)
    {
        bool bTestBanking = false;
        sal_uInt16 nPos = FindCurrencyTableEntry(rFmtString, bTestBanking);

        if (IsInTable(nPos, bTestBanking, rFmtString))
        {
            nFound = NUMBERFORMAT_ENTRY_NEW_CURRENCY;
            bRes = true;
        }
    }
    else
    {
        bRes = !IsRemoved_Impl(nFound);
    }

    if (pAt)
        *pAt = nFound;

    return bRes;
}

void SvxNumberFormatShell::GetInitSettings(sal_uInt16& nCatLbPos, LanguageType& rLangType,
                                           sal_uInt16& nFmtLbSelPos,
                                           std::vector<OUString>& rFmtEntries,
                                           OUString& rPrevString, Color*& rpPrevColor)
{
    // precondition: number formater found
    DBG_ASSERT(pFormatter != nullptr, "Number formatter not found!");

    short nSelPos = SELPOS_NONE;

    // special treatment for undefined number format:
    if ((eValType == SvxNumberValueType::Undefined) && (nCurFormatKey == 0))
        PosToCategory_Impl(CAT_ALL, nCurCategory); // category = all
    else
        nCurCategory = SvNumFormatType::UNDEFINED; // category = undefined

    pCurFmtTable = &(pFormatter->GetFirstEntryTable(nCurCategory, nCurFormatKey, eCurLanguage));

    CategoryToPos_Impl(nCurCategory, nCatLbPos);
    rLangType = eCurLanguage;

    nSelPos = FillEntryList_Impl(rFmtEntries);

    DBG_ASSERT(nSelPos != SELPOS_NONE, "Leere Formatliste!");

    nFmtLbSelPos = (nSelPos != SELPOS_NONE) ? static_cast<sal_uInt16>(nSelPos) : 0;
    GetPreviewString_Impl(rPrevString, rpPrevColor);
}

short SvxNumberFormatShell::FillEntryList_Impl(std::vector<OUString>& rList)
{
    /* Create a current list of format entries. The return value is
     * the list position of the current format. If the list is empty
     * or if there is no current format, SELPOS_NONE is delivered.
     */
    short nSelPos = SELPOS_NONE;

    aCurEntryList.clear();

    if (nCurCategory == SvNumFormatType::ALL)
    {
        FillEListWithStd_Impl(rList, SvNumFormatType::NUMBER, nSelPos);
        nSelPos = FillEListWithUsD_Impl(rList, SvNumFormatType::NUMBER, nSelPos);

        FillEListWithStd_Impl(rList, SvNumFormatType::PERCENT, nSelPos);
        nSelPos = FillEListWithUsD_Impl(rList, SvNumFormatType::PERCENT, nSelPos);

        FillEListWithStd_Impl(rList, SvNumFormatType::CURRENCY, nSelPos);
        // No FillEListWithUsD_Impl() here, user defined currency formats
        // were already added.

        FillEListWithStd_Impl(rList, SvNumFormatType::DATE, nSelPos);
        nSelPos = FillEListWithUsD_Impl(rList, SvNumFormatType::DATE, nSelPos);

        FillEListWithStd_Impl(rList, SvNumFormatType::TIME, nSelPos);
        nSelPos = FillEListWithUsD_Impl(rList, SvNumFormatType::TIME, nSelPos);

        nSelPos = FillEListWithDateTime_Impl(rList, nSelPos, false);
        nSelPos = FillEListWithUsD_Impl(rList, SvNumFormatType::DATETIME, nSelPos);

        FillEListWithStd_Impl(rList, SvNumFormatType::SCIENTIFIC, nSelPos);
        nSelPos = FillEListWithUsD_Impl(rList, SvNumFormatType::SCIENTIFIC, nSelPos);

        FillEListWithStd_Impl(rList, SvNumFormatType::FRACTION, nSelPos);
        nSelPos = FillEListWithUsD_Impl(rList, SvNumFormatType::FRACTION, nSelPos);

        FillEListWithStd_Impl(rList, SvNumFormatType::LOGICAL, nSelPos);
        nSelPos = FillEListWithUsD_Impl(rList, SvNumFormatType::LOGICAL, nSelPos);

        FillEListWithStd_Impl(rList, SvNumFormatType::TEXT, nSelPos);
        nSelPos = FillEListWithUsD_Impl(rList, SvNumFormatType::TEXT, nSelPos);
    }
    else
    {
        FillEListWithStd_Impl(rList, nCurCategory, nSelPos, true);
        if (nCurCategory != SvNumFormatType::CURRENCY)
            nSelPos = FillEListWithUsD_Impl(rList, nCurCategory, nSelPos);
        if (nCurCategory == SvNumFormatType::DATE || nCurCategory == SvNumFormatType::TIME)
            nSelPos = FillEListWithDateTime_Impl(rList, nSelPos, true);
    }

    return nSelPos;
}

void SvxNumberFormatShell::FillEListWithStd_Impl(std::vector<OUString>& rList,
                                                 SvNumFormatType eCategory, short& nSelPos,
                                                 bool bSuppressDuplicates)
{
    /* Create a current list of format entries. The return value is
     * the list position of the current format. If the list is empty
     * or if there is no current format, SELPOS_NONE is delivered.
     */

    assert(pCurFmtTable != nullptr);

    aCurrencyFormatList.clear();

    NfIndexTableOffset eOffsetStart;
    NfIndexTableOffset eOffsetEnd;

    switch (eCategory)
    {
        case SvNumFormatType::NUMBER:
            eOffsetStart = NF_NUMBER_START;
            eOffsetEnd = NF_NUMBER_END;
        break;
        case SvNumFormatType::PERCENT:
            eOffsetStart = NF_PERCENT_START;
            eOffsetEnd = NF_PERCENT_END;
        break;
        case SvNumFormatType::CURRENCY:
            // Currency entries are generated and assembled, ignore
            // bSuppressDuplicates.
            nSelPos = FillEListWithCurrency_Impl(rList, nSelPos);
            return;
        case SvNumFormatType::DATE:
            eOffsetStart = NF_DATE_START;
            eOffsetEnd = NF_DATE_END;
        break;
        case SvNumFormatType::TIME:
            eOffsetStart = NF_TIME_START;
            eOffsetEnd = NF_TIME_END;
        break;
        case SvNumFormatType::SCIENTIFIC:
            eOffsetStart = NF_SCIENTIFIC_START;
            eOffsetEnd = NF_SCIENTIFIC_END;
        break;
        case SvNumFormatType::FRACTION:
            eOffsetStart = NF_FRACTION_START;
            eOffsetEnd = NF_FRACTION_END;
            // Fraction formats are internally generated by the number
            // formatter and are not supposed to contain duplicates anyway.
            nSelPos = FillEListWithFormats_Impl(rList, nSelPos, eOffsetStart, eOffsetEnd, false);
            nSelPos = FillEListWithFormats_Impl(rList, nSelPos, NF_FRACTION_3D, NF_FRACTION_100, false);
            return;
        case SvNumFormatType::LOGICAL:
            eOffsetStart = NF_BOOLEAN;
            eOffsetEnd = NF_BOOLEAN;
        break;
        case SvNumFormatType::TEXT:
            eOffsetStart = NF_TEXT;
            eOffsetEnd = NF_TEXT;
        break;
        default:
            return;
    }

    nSelPos = FillEListWithFormats_Impl(rList, nSelPos, eOffsetStart, eOffsetEnd, bSuppressDuplicates);
}

short SvxNumberFormatShell::FillEListWithFormats_Impl(std::vector<OUString>& rList, short nSelPos,
                                                      NfIndexTableOffset eOffsetStart,
                                                      NfIndexTableOffset eOffsetEnd,
                                                      bool bSuppressDuplicates)
{
    /* Create a current list of format entries. The return value is
     * the list position of the current format. If the list is empty
     * or if there is no current format, SELPOS_NONE is delivered.
     */
    sal_uInt16 nMyType;

    sal_uInt32 nNFEntry;
    OUString aNewFormNInfo;

    long nIndex;

    for (nIndex = eOffsetStart; nIndex <= eOffsetEnd; nIndex++)
    {
        nNFEntry = pFormatter->GetFormatIndex(static_cast<NfIndexTableOffset>(nIndex), eCurLanguage);

        const SvNumberformat* pNumEntry = pFormatter->GetEntry(nNFEntry);

        if (pNumEntry == nullptr)
            continue;

        SvNumFormatType nMyCat = pNumEntry->GetMaskedType();
        CategoryToPos_Impl(nMyCat, nMyType);
        aNewFormNInfo = pNumEntry->GetFormatstring();

        if (nNFEntry == nCurFormatKey)
        {
            nSelPos = (!IsRemoved_Impl(nNFEntry)) ? aCurEntryList.size() : SELPOS_NONE;
        }

        if (!bSuppressDuplicates || IsEssentialFormat_Impl(nMyCat, nNFEntry)
                || std::find(rList.begin(), rList.end(), aNewFormNInfo) == rList.end())
        {
            rList.push_back(aNewFormNInfo);
            aCurEntryList.push_back(nNFEntry);
        }
    }

    return nSelPos;
}

short SvxNumberFormatShell::FillEListWithDateTime_Impl(std::vector<OUString>& rList, short nSelPos,
                                                       bool bSuppressDuplicates)
{
    sal_uInt16 nMyType;

    sal_uInt32 nNFEntry;
    OUString aNewFormNInfo;

    for (long nIndex = NF_DATETIME_START; nIndex <= NF_DATETIME_END; ++nIndex)
    {
        nNFEntry = pFormatter->GetFormatIndex(static_cast<NfIndexTableOffset>(nIndex), eCurLanguage);

        const SvNumberformat* pNumEntry = pFormatter->GetEntry(nNFEntry);
        if (pNumEntry != nullptr)
        {
            SvNumFormatType nMyCat = pNumEntry->GetMaskedType();
            CategoryToPos_Impl(nMyCat, nMyType);
            aNewFormNInfo = pNumEntry->GetFormatstring();

            if (nNFEntry == nCurFormatKey)
            {
                nSelPos = (!IsRemoved_Impl(nNFEntry)) ? aCurEntryList.size() : SELPOS_NONE;
            }

            if (!bSuppressDuplicates || IsEssentialFormat_Impl(nMyCat, nNFEntry)
                    || std::find(rList.begin(), rList.end(), aNewFormNInfo) == rList.end())
            {
                // Ugly hack to suppress an ISO date+time format that is the
                // default date+time format of the locale and identical to the
                // internally generated one to be added below.
                if (!bSuppressDuplicates || aNewFormNInfo != "YYYY-MM-DD HH:MM:SS")
                {
                    rList.push_back(aNewFormNInfo);
                    aCurEntryList.push_back(nNFEntry);
                }
            }
        }
    }

    // Always add the internally generated ISO format.
    nSelPos = FillEListWithFormats_Impl(rList, nSelPos, NF_DATETIME_ISO_YYYYMMDD_HHMMSS,
                                        NF_DATETIME_ISO_YYYYMMDD_HHMMSS, false);

    return nSelPos;
}

bool SvxNumberFormatShell::IsEssentialFormat_Impl(SvNumFormatType eType, sal_uInt32 nKey)
{
    if (nKey == nCurFormatKey)
        return true;

    const NfIndexTableOffset nIndex = pFormatter->GetIndexTableOffset(nKey);
    switch (nIndex)
    {
        // These are preferred or edit formats.
        case NF_DATE_SYS_DDMMYYYY:
        case NF_DATE_ISO_YYYYMMDD:
        case NF_TIME_HH_MMSS:
        case NF_TIME_MMSS00:
        case NF_TIME_HH_MMSS00:
        case NF_DATETIME_SYS_DDMMYYYY_HHMMSS:
        case NF_DATETIME_ISO_YYYYMMDD_HHMMSS:
            return true;
        default:
        break;
    }

    return nKey == pFormatter->GetStandardFormat(eType, eCurLanguage);
}

short SvxNumberFormatShell::FillEListWithCurrency_Impl(std::vector<OUString>& rList, short nSelPos)
{
    /* Create a current list of format entries. The return value is
     * the list position of the current format. If the list is empty
     * or if there is no current format, SELPOS_NONE is delivered.
     */
    DBG_ASSERT(pCurFmtTable != nullptr, "unknown NumberFormat");

    const NfCurrencyEntry* pTmpCurrencyEntry;
    bool bTmpBanking;
    OUString rSymbol;

    bool bFlag = pFormatter->GetNewCurrencySymbolString(nCurFormatKey, rSymbol, &pTmpCurrencyEntry, &bTmpBanking);

    if ((!bFlag && pCurCurrencyEntry == nullptr)
            || (bFlag && pTmpCurrencyEntry == nullptr && rSymbol.isEmpty())
            || (nCurCategory == SvNumFormatType::ALL))
    {
        if (nCurCategory == SvNumFormatType::ALL)
            FillEListWithUserCurrencys(rList, nSelPos);
        nSelPos = FillEListWithSysCurrencys(rList, nSelPos);
    }
    else
    {
        nSelPos = FillEListWithUserCurrencys(rList, nSelPos);
    }

    return nSelPos;
}

short SvxNumberFormatShell::FillEListWithSysCurrencys(std::vector<OUString>& rList, short nSelPos)
{
    /* Create a current list of format entries. The return value is
     * the list position of the current format. If the list is empty
     * or if there is no current format, SELPOS_NONE is delivered.
     */
    sal_uInt16 nMyType;

    DBG_ASSERT(pCurFmtTable != nullptr, "unknown NumberFormat");

    sal_uInt32 nNFEntry;
    OUString aNewFormNInfo;

    nCurCurrencyEntryPos = 0;

    for (long nIndex = NF_CURRENCY_START; nIndex <= NF_CURRENCY_END; nIndex++)
    {
        nNFEntry = pFormatter->GetFormatIndex(static_cast<NfIndexTableOffset>(nIndex), eCurLanguage);

        if (nCurCategory == SvNumFormatType::ALL && nNFEntry != nCurFormatKey)
            // Deprecated old currency entries, for ALL add only if used as
            // current format key.
            continue;

        const SvNumberformat* pNumEntry = pFormatter->GetEntry(nNFEntry);

        if (pNumEntry == nullptr)
            continue;

        SvNumFormatType nMyCat = pNumEntry->GetMaskedType();
        CategoryToPos_Impl(nMyCat, nMyType);
        aNewFormNInfo = pNumEntry->GetFormatstring();

        if (nNFEntry == nCurFormatKey)
        {
            nSelPos = (!IsRemoved_Impl(nNFEntry)) ? aCurEntryList.size() : SELPOS_NONE;
        }

        rList.push_back(aNewFormNInfo);
        aCurEntryList.push_back(nNFEntry);
    }

    if (nCurCategory != SvNumFormatType::ALL)
    {
        for (const auto& rEntry : *pCurFmtTable)
        {
            sal_uInt32 nKey = rEntry.first;
            const SvNumberformat* pNumEntry = rEntry.second;

            if (!IsRemoved_Impl(nKey))
            {
                bool bUserNewCurrency = false;
                if (pNumEntry->HasNewCurrency())
                {
                    const NfCurrencyEntry* pTmpCurrencyEntry;
                    bool bTmpBanking;
                    OUString rSymbol;

                    pFormatter->GetNewCurrencySymbolString(nKey, rSymbol, &pTmpCurrencyEntry, &bTmpBanking);

                    bUserNewCurrency = (pTmpCurrencyEntry != nullptr);
                }

                if (!bUserNewCurrency && (pNumEntry->GetType() & SvNumFormatType::DEFINED))
                {
                    SvNumFormatType nMyCat = pNumEntry->GetMaskedType();
                    CategoryToPos_Impl(nMyCat, nMyType);
                    aNewFormNInfo = pNumEntry->GetFormatstring();

                    if (nKey == nCurFormatKey)
                        nSelPos = aCurEntryList.size();
                    rList.push_back(aNewFormNInfo);
                    aCurEntryList.push_back(nKey);
                }
            }
        }
    }
    return nSelPos;
}

short SvxNumberFormatShell::FillEListWithUserCurrencys(std::vector<OUString>& rList, short nSelPos)
{
    /* Create a current list of format entries. The return value is
     * the list position of the current format. If the list is empty
     * or if there is no current format, SELPOS_NONE is delivered.
     */
    sal_uInt16 nMyType;

    DBG_ASSERT(pCurFmtTable != nullptr, "unknown NumberFormat");

    OUString aNewFormNInfo;

    const NfCurrencyEntry* pTmpCurrencyEntry;
    bool bTmpBanking, bAdaptSelPos;
    OUString rSymbol;
    OUString rBankSymbol;

    std::vector<OUString> aList;
    std::vector<sal_uInt32> aKeyList;

    pFormatter->GetNewCurrencySymbolString(nCurFormatKey, rSymbol, &pTmpCurrencyEntry, &bTmpBanking);

    OUString rShortSymbol;

    if (pCurCurrencyEntry == nullptr)
    {
        // #110398# If no currency format was previously selected (we're not
        // about to add another currency), try to select the initial currency
        // format (nCurFormatKey) that was set in FormatChanged() after
        // matching the format string entered in the dialog.
        bAdaptSelPos = true;
        pCurCurrencyEntry = const_cast<NfCurrencyEntry*>(pTmpCurrencyEntry);
        bBankingSymbol = bTmpBanking;
        nCurCurrencyEntryPos = FindCurrencyFormat(pTmpCurrencyEntry, bTmpBanking);
    }
    else
    {
        if (pTmpCurrencyEntry == pCurCurrencyEntry)
            bAdaptSelPos = true;
        else
        {
            bAdaptSelPos = false;
            pTmpCurrencyEntry = pCurCurrencyEntry;
        }
        bTmpBanking = bBankingSymbol;
    }

    if (pTmpCurrencyEntry != nullptr)
    {
        rSymbol = pTmpCurrencyEntry->BuildSymbolString(false);
        rBankSymbol = pTmpCurrencyEntry->BuildSymbolString(true);
        rShortSymbol = pTmpCurrencyEntry->BuildSymbolString(bTmpBanking, true);
    }

    for (const auto& rEntry : *pCurFmtTable)
    {
        sal_uInt32 nKey = rEntry.first;
        const SvNumberformat* pNumEntry = rEntry.second;

        if (!IsRemoved_Impl(nKey))
        {
            if (pNumEntry->GetType() & SvNumFormatType::DEFINED || pNumEntry->IsAdditionalBuiltin())
            {
                SvNumFormatType nMyCat = pNumEntry->GetMaskedType();
                CategoryToPos_Impl(nMyCat, nMyType);
                aNewFormNInfo = pNumEntry->GetFormatstring();

                bool bInsFlag = false;
                if (pNumEntry->HasNewCurrency())
                {
                    bInsFlag = true; // merge locale formats into currency selection
                }
                else if ((!bTmpBanking && aNewFormNInfo.indexOf(rSymbol) >= 0)
                        || (bTmpBanking && aNewFormNInfo.indexOf(rBankSymbol) >= 0))
                {
                    bInsFlag = true;
                }
                else if (aNewFormNInfo.indexOf(rShortSymbol) >= 0)
                {
                    OUString rTstSymbol;
                    const NfCurrencyEntry* pTstCurrencyEntry;
                    bool bTstBanking;

                    pFormatter->GetNewCurrencySymbolString(nKey, rTstSymbol, &pTstCurrencyEntry, &bTstBanking);

                    if (pTmpCurrencyEntry == pTstCurrencyEntry && bTstBanking == bTmpBanking)
                    {
                        bInsFlag = true;
                    }
                }

                if (bInsFlag)
                {
                    aList.push_back(aNewFormNInfo);
                    aKeyList.push_back(nKey);
                }
            }
        }
    }

    NfWSStringsDtor aWSStringsDtor;
    sal_uInt16 nDefault;
    if (pTmpCurrencyEntry && nCurCategory != SvNumFormatType::ALL)
    {
        nDefault = pFormatter->GetCurrencyFormatStrings(aWSStringsDtor, *pTmpCurrencyEntry, bTmpBanking);
        if (!bTmpBanking)
            pFormatter->GetCurrencyFormatStrings(aWSStringsDtor, *pTmpCurrencyEntry, true);
    }
    else
        nDefault = 0;
    if (!bTmpBanking && nCurCategory != SvNumFormatType::ALL)
    {
        // append formats for all currencies defined in the current I18N locale
        const NfCurrencyTable& rCurrencyTable = SvNumberFormatter::GetTheCurrencyTable();
        sal_uInt16 nCurrCount = rCurrencyTable.size();
        LanguageType eLang = MsLangId::getRealLanguage(eCurLanguage);
        for (sal_uInt16 i = 0; i < nCurrCount; ++i)
        {
            const NfCurrencyEntry* pCurr = &rCurrencyTable[i];
            if (pCurr->GetLanguage() == eLang && pTmpCurrencyEntry != pCurr)
            {
                pFormatter->GetCurrencyFormatStrings(aWSStringsDtor, *pCurr, false);
                pFormatter->GetCurrencyFormatStrings(aWSStringsDtor, *pCurr, true);
            }
        }
    }

    size_t nOldListCount = rList.size();
    for (size_t i = 0, nPos = nOldListCount; i < aWSStringsDtor.size(); ++i)
    {
        bool bFlag = true;
        OUString aInsStr(aWSStringsDtor[i]);
        size_t j;
        for (j = 0; j < aList.size(); ++j)
        {
            if (aList[j] == aInsStr)
            {
                bFlag = false;
                break;
            }
        }
        if (bFlag)
        {
            rList.push_back(aInsStr);
            aCurEntryList.insert(aCurEntryList.begin() + (nPos++), NUMBERFORMAT_ENTRY_NOT_FOUND);
        }
        else
        {
            rList.push_back(aList[j]);
            aList.erase(aList.begin() + j);
            aCurEntryList.insert(aCurEntryList.begin() + (nPos++), aKeyList[j]);
            aKeyList.erase(aKeyList.begin() + j);
        }
    }

    for (size_t i = 0; i < aKeyList.size(); ++i)
    {
        if (aKeyList[i] != NUMBERFORMAT_ENTRY_NOT_FOUND)
        {
            rList.push_back(aList[i]);
            aCurEntryList.push_back(aKeyList[i]);
        }
    }

    for (size_t i = nOldListCount; i < rList.size(); ++i)
    {
        aCurrencyFormatList.push_back(rList[i]);

        if (nSelPos == SELPOS_NONE && bAdaptSelPos && aCurEntryList[i] == nCurFormatKey)
            nSelPos = i;
    }

    if (nSelPos == SELPOS_NONE && nCurCategory != SvNumFormatType::ALL)
        nSelPos = nDefault;

    return nSelPos;
}

short SvxNumberFormatShell::FillEListWithUsD_Impl(std::vector<OUString>& rList,
                                                  SvNumFormatType eCategory, short nSelPos)
{
    /* Create a current list of format entries. The return value is
     * the list position of the current format. If the list is empty
     * or if there is no current format, SELPOS_NONE is delivered.
     */

    assert(pCurFmtTable != nullptr);

    OUString aNewFormNInfo;

    const bool bCatDefined = (eCategory == SvNumFormatType::DEFINED);
    const bool bCategoryMatch = (eCategory != SvNumFormatType::ALL && !bCatDefined);

    for (const auto& rEntry : *pCurFmtTable)
    {
        const SvNumberformat* pNumEntry = rEntry.second;

        if (bCategoryMatch && (pNumEntry->GetMaskedType() & eCategory) != eCategory)
            continue; // for; type does not match category if not ALL

        const bool bUserDefined = bool(pNumEntry->GetType() & SvNumFormatType::DEFINED);
        if (!bUserDefined && bCatDefined)
            continue; // for; not user defined in DEFINED category

        if (!(bUserDefined || (!bCatDefined && pNumEntry->IsAdditionalBuiltin())))
            continue; // for; does not match criteria at all

        const sal_uInt32 nKey = rEntry.first;
        if (!IsRemoved_Impl(nKey))
        {
            aNewFormNInfo = pNumEntry->GetFormatstring();

            bool bAdd = true;
            if (pNumEntry->HasNewCurrency())
            {
                bool bTestBanking;
                sal_uInt16 nPos = FindCurrencyTableEntry(aNewFormNInfo, bTestBanking);
                bAdd = !IsInTable(nPos, bTestBanking, aNewFormNInfo);
            }
            if (bAdd)
            {
                if (nKey == nCurFormatKey)
                    nSelPos = aCurEntryList.size();
                rList.push_back(aNewFormNInfo);
                aCurEntryList.push_back(nKey);
            }
        }
    }
    return nSelPos;
}

void SvxNumberFormatShell::GetPreviewString_Impl(OUString& rString, Color*& rpColor)
{
    rpColor = nullptr;

    //  #50441# if a string was set in addition to the value, use it for text formats
    bool bUseText = (eValType == SvxNumberValueType::String
            || (!aValStr.isEmpty() && (pFormatter->GetType(nCurFormatKey) & SvNumFormatType::TEXT)));

    if (bUseText)
    {
        pFormatter->GetOutputString(aValStr, nCurFormatKey, rString, &rpColor);
    }
    else
    {
        pFormatter->GetOutputString(nValNum, nCurFormatKey, rString, &rpColor, bUseStarFormat);
    }
}

::std::vector<sal_uInt32>::iterator SvxNumberFormatShell::GetRemoved_Impl(size_t nKey)
{
    return ::std::find(aDelList.begin(), aDelList.end(), nKey);
}

bool SvxNumberFormatShell::IsRemoved_Impl(size_t nKey)
{
    return GetRemoved_Impl(nKey) != aDelList.end();
}

::std::vector<sal_uInt32>::iterator SvxNumberFormatShell::GetAdded_Impl(size_t nKey)
{
    return ::std::find(aAddList.begin(), aAddList.end(), nKey);
}

// Conversion routines:
void SvxNumberFormatShell::PosToCategory_Impl(sal_uInt16 nPos, SvNumFormatType& rCategory)
{
    // map category css::form positions (->resource)
    switch (nPos)
    {
        case CAT_USERDEFINED:
            rCategory = SvNumFormatType::DEFINED;
        break;
        case CAT_NUMBER:
            rCategory = SvNumFormatType::NUMBER;
        break;
        case CAT_PERCENT:
            rCategory = SvNumFormatType::PERCENT;
        break;
        case CAT_CURRENCY:
            rCategory = SvNumFormatType::CURRENCY;
        break;
        case CAT_DATE:
            rCategory = SvNumFormatType::DATE;
        break;
        case CAT_TIME:
            rCategory = SvNumFormatType::TIME;
        break;
        case CAT_SCIENTIFIC:
            rCategory = SvNumFormatType::SCIENTIFIC;
        break;
        case CAT_FRACTION:
            rCategory = SvNumFormatType::FRACTION;
        break;
        case CAT_BOOLEAN:
            rCategory = SvNumFormatType::LOGICAL;
        break;
        case CAT_TEXT:
            rCategory = SvNumFormatType::TEXT;
        break;
        case CAT_ALL:
        default:
            rCategory = SvNumFormatType::ALL;
        break;
    }
}

void SvxNumberFormatShell::CategoryToPos_Impl(SvNumFormatType nCategory, sal_uInt16& rPos)
{
    // map category to css::form positions (->resource)
    switch (nCategory)
    {
        case SvNumFormatType::DEFINED:
            rPos = CAT_USERDEFINED;
        break;
        case SvNumFormatType::NUMBER:
            rPos = CAT_NUMBER;
        break;
        case SvNumFormatType::PERCENT:
            rPos = CAT_PERCENT;
        break;
        case SvNumFormatType::CURRENCY:
            rPos = CAT_CURRENCY;
        break;
        case SvNumFormatType::DATETIME:
        case SvNumFormatType::DATE:
            rPos = CAT_DATE;
        break;
        case SvNumFormatType::TIME:
            rPos = CAT_TIME;
        break;
        case SvNumFormatType::SCIENTIFIC:
            rPos = CAT_SCIENTIFIC;
        break;
        case SvNumFormatType::FRACTION:
            rPos = CAT_FRACTION;
        break;
        case SvNumFormatType::LOGICAL:
            rPos = CAT_BOOLEAN;
        break;
        case SvNumFormatType::TEXT:
            rPos = CAT_TEXT;
        break;
        case SvNumFormatType::ALL:
        default:
            rPos = CAT_ALL;
    }
}

/*
 * Function:   Formats the number nValue dependent on rFormatStr
 *             and stores the result in rPreviewStr.
 * Input:      FormatString, color, number to format
 * Output:     Output string rPreviewStr
 */
void SvxNumberFormatShell::MakePrevStringFromVal(const OUString& rFormatStr, OUString& rPreviewStr,
                                                 Color*& rpFontColor, double nValue)
{
    rpFontColor = nullptr;
    pFormatter->GetPreviewString(rFormatStr, nValue, rPreviewStr, &rpFontColor, eCurLanguage);
}

/*
 * Function:   Returns the comment for a given entry.
 * Input:      Number of the entry
 * Output:     Comment string
 */
void SvxNumberFormatShell::SetComment4Entry(short nEntry, const OUString& aEntStr)
{
    SvNumberformat* pNumEntry;
    if (nEntry < 0)
        return;
    sal_uInt32 nMyNfEntry = aCurEntryList[nEntry];
    pNumEntry = const_cast<SvNumberformat*>(pFormatter->GetEntry(nMyNfEntry));
    if (pNumEntry != nullptr)
        pNumEntry->SetComment(aEntStr);
}

/*
 * Function:   Returns the comment for a given entry.
 * Input:      Number of the entry
 * Output:     Comment string
 */
OUString SvxNumberFormatShell::GetComment4Entry(short nEntry)
{
    if (nEntry < 0)
        return OUString();

    if (static_cast<size_t>(nEntry) < aCurEntryList.size())
    {
        sal_uInt32 nMyNfEntry = aCurEntryList[nEntry];
        const SvNumberformat* pNumEntry = pFormatter->GetEntry(nMyNfEntry);
        if (pNumEntry != nullptr)
            return pNumEntry->GetComment();
    }

    return OUString();
}

/*
 * Function:   Returns the category number for a given entry.
 * Input:      Number of the entry
 * Output:     Category number
 */
short SvxNumberFormatShell::GetCategory4Entry(short nEntry) const
{
    if (nEntry < 0)
        return 0;
    if (static_cast<size_t>(nEntry) < aCurEntryList.size())
    {
        sal_uInt32 nMyNfEntry = aCurEntryList[nEntry];

        if (nMyNfEntry != NUMBERFORMAT_ENTRY_NOT_FOUND)
        {
            const SvNumberformat* pNumEntry = pFormatter->GetEntry(nMyNfEntry);
            sal_uInt16 nMyType;
            if (pNumEntry != nullptr)
            {
                SvNumFormatType nMyCat = pNumEntry->GetMaskedType();
                CategoryToPos_Impl(nMyCat, nMyType);

                return static_cast<short>(nMyType);
            }
            return 0;
        }
        else if (!aCurrencyFormatList.empty())
        {
            return CAT_CURRENCY;
        }
    }
    return 0;
}

/*
 * Function:   Returns the information about whether an entry is user-specific.
 * Input:      Number of the entry
 * Output:     User-specific?
 */
bool SvxNumberFormatShell::GetUserDefined4Entry(short nEntry)
{
    if (nEntry < 0)
        return false;
    if (static_cast<size_t>(nEntry) < aCurEntryList.size())
    {
        sal_uInt32 nMyNfEntry = aCurEntryList[nEntry];
        const SvNumberformat* pNumEntry = pFormatter->GetEntry(nMyNfEntry);

        if (pNumEntry != nullptr)
        {
            if (pNumEntry->GetType() & SvNumFormatType::DEFINED)
            {
                return true;
            }
        }
    }
    return false;
}

/*
 * Function:   Returns the format string for a given entry.
 * Input:      Number of the entry
 * Output:     Format string
 */
OUString SvxNumberFormatShell::GetFormat4Entry(short nEntry)
{
    if (nEntry < 0)
        return OUString();

    if (!aCurrencyFormatList.empty())
    {
        if (aCurrencyFormatList.size() > static_cast<size_t>(nEntry))
            return aCurrencyFormatList[nEntry];
    }
    else
    {
        sal_uInt32 nMyNfEntry = aCurEntryList[nEntry];
        const SvNumberformat* pNumEntry = pFormatter->GetEntry(nMyNfEntry);

        if (pNumEntry != nullptr)
            return pNumEntry->GetFormatstring();
    }
    return OUString();
}

/*
 * Function:   Returns the list number for a given format index.
 * Input:      Number of the entry
 * Output:     Category number
 */
short SvxNumberFormatShell::GetListPos4Entry( sal_uInt32 nIdx, const OUString& rFmtString )
{
    short nSelP = SELPOS_NONE;
    if (nIdx != NUMBERFORMAT_ENTRY_NEW_CURRENCY)
    {
        // Check list size against return type limit.
        if (aCurEntryList.size() <= static_cast<size_t>(::std::numeric_limits<short>::max()))
        {
            for (size_t i = 0; i < aCurEntryList.size(); ++i)
            {
                if (aCurEntryList[i] == nIdx)
                {
                    nSelP = i;
                    break;
                }
            }
        }
        else
        {
            OSL_FAIL("svx::SvxNumberFormatShell::GetListPos4Entry(), list got too large!");
        }
    }
    else
    {
        // A second list holds the generated currency formats.
        for (size_t i = 0; i < aCurrencyFormatList.size(); ++i)
        {
            if (rFmtString == aCurrencyFormatList[i])
            {
                nSelP = static_cast<short>(i);
                break;
            }
        }
    }
    return nSelP;
}

OUString SvxNumberFormatShell::GetStandardName() const
{
    return pFormatter->GetStandardName(eCurLanguage);
}

void SvxNumberFormatShell::GetCurrencySymbols(std::vector<OUString>& rList, sal_uInt16* pPos)
{
    const NfCurrencyEntry* pTmpCurrencyEntry = SvNumberFormatter::MatchSystemCurrency();

    bool bFlag = (pTmpCurrencyEntry == nullptr);

    SvxCurrencyToolBoxControl::GetCurrencySymbols(rList, bFlag, aCurCurrencyList);

    if (pPos != nullptr)
    {
        const NfCurrencyTable& rCurrencyTable = SvNumberFormatter::GetTheCurrencyTable();
        sal_uInt16 nTableCount = rCurrencyTable.size();

        *pPos = 0;
        size_t nCount = aCurCurrencyList.size();

        if (bFlag)
        {
            *pPos = 1;
            nCurCurrencyEntryPos = 1;
        }
        else
        {
            for (size_t i = 1; i < nCount; i++)
            {
                const sal_uInt16 j = aCurCurrencyList[i];
                if (j != sal_uInt16(-1) && j < nTableCount && pTmpCurrencyEntry == &rCurrencyTable[j])
                {
                    *pPos = static_cast<sal_uInt16>(i);
                    nCurCurrencyEntryPos = static_cast<sal_uInt16>(i);
                    break;
                }
            }
        }
    }
}

void SvxNumberFormatShell::SetCurrencySymbol(sal_uInt32 nPos)
{
    const NfCurrencyTable& rCurrencyTable = SvNumberFormatter::GetTheCurrencyTable();
    sal_uInt16 nCount = rCurrencyTable.size();

    bBankingSymbol = (nPos >= nCount);

    if (nPos < aCurCurrencyList.size())
    {
        sal_uInt16 nCurrencyPos = aCurCurrencyList[nPos];
        if (nCurrencyPos != sal_uInt16(-1))
        {
            pCurCurrencyEntry = const_cast<NfCurrencyEntry*>(&rCurrencyTable[nCurrencyPos]);
            nCurCurrencyEntryPos = nPos;
        }
        else
        {
            pCurCurrencyEntry = nullptr;
            nCurCurrencyEntryPos = 0;
            nCurFormatKey = pFormatter->GetFormatIndex(NF_CURRENCY_1000DEC2_RED, eCurLanguage);
        }
    }
}

void SvxNumberFormatShell::SetCurCurrencyEntry(NfCurrencyEntry* pCEntry)
{
    pCurCurrencyEntry = pCEntry;
}

bool SvxNumberFormatShell::IsTmpCurrencyFormat(const OUString& rFmtString)
{
    sal_uInt32 nFound;
    FindEntry(rFmtString, &nFound);
    return nFound == NUMBERFORMAT_ENTRY_NEW_CURRENCY;
}

sal_uInt16 SvxNumberFormatShell::FindCurrencyFormat(const OUString& rFmtString)
{
    const NfCurrencyTable& rCurrencyTable = SvNumberFormatter::GetTheCurrencyTable();
    sal_uInt16 nCount = rCurrencyTable.size();

    bool bTestBanking = false;

    sal_uInt16 nPos = FindCurrencyTableEntry(rFmtString, bTestBanking);

    if (nPos != sal_uInt16(-1))
    {
        sal_uInt16 nStart = 0;
        if (bTestBanking && aCurCurrencyList.size() > nPos)
        {
            nStart = nCount;
        }
        for (size_t j = nStart; j < aCurCurrencyList.size(); j++)
        {
            if (aCurCurrencyList[j] == nPos)
                return j;
        }
    }
    return sal_uInt16(-1);
}

sal_uInt16 SvxNumberFormatShell::FindCurrencyTableEntry(const OUString& rFmtString,
                                                        bool& bTestBanking)
{
    sal_uInt16 nPos = sal_uInt16(-1);

    const NfCurrencyTable& rCurrencyTable = SvNumberFormatter::GetTheCurrencyTable();
    sal_uInt16 nCount = rCurrencyTable.size();

    const SvNumberformat* pFormat;
    OUString aSymbol, aExtension;
    sal_uInt32 nFound = pFormatter->TestNewString(rFmtString, eCurLanguage);
    if (nFound != NUMBERFORMAT_ENTRY_NOT_FOUND
            && ((pFormat = pFormatter->GetEntry(nFound)) != nullptr)
            && pFormat->GetNewCurrencySymbol(aSymbol, aExtension))
    {
        // eventually match with format locale
        const NfCurrencyEntry* pTmpCurrencyEntry = SvNumberFormatter::GetCurrencyEntry(
            bTestBanking, aSymbol, aExtension, pFormat->GetLanguage());
        if (pTmpCurrencyEntry)
        {
            for (sal_uInt16 i = 0; i < nCount; i++)
            {
                if (pTmpCurrencyEntry == &rCurrencyTable[i])
                {
                    nPos = i;
                    break;
                }
            }
        }
    }
    else
    {
        // search symbol string only
        for (sal_uInt16 i = 0; i < nCount; i++)
        {
            const NfCurrencyEntry* pTmpCurrencyEntry = &rCurrencyTable[i];
            OUString _aSymbol = pTmpCurrencyEntry->BuildSymbolString(false);
            OUString aBankSymbol = pTmpCurrencyEntry->BuildSymbolString(true);

            if (rFmtString.indexOf(_aSymbol) != -1)
            {
                bTestBanking = false;
                nPos = i;
                break;
            }
            else if (rFmtString.indexOf(aBankSymbol) != -1)
            {
                bTestBanking = true;
                nPos = i;
                break;
            }
        }
    }

    return nPos;
}

sal_uInt16 SvxNumberFormatShell::FindCurrencyFormat(const NfCurrencyEntry* pTmpCurrencyEntry,
                                                    bool bTmpBanking)
{
    const NfCurrencyTable& rCurrencyTable = SvNumberFormatter::GetTheCurrencyTable();
    sal_uInt16 nCount = rCurrencyTable.size();

    sal_uInt16 nPos = 0;
    for (sal_uInt16 i = 0; i < nCount; i++)
    {
        if (pTmpCurrencyEntry == &rCurrencyTable[i])
        {
            nPos = i;
            break;
        }
    }

    sal_uInt16 nStart = 0;
    if (bTmpBanking && aCurCurrencyList.size() > nPos)
    {
        nStart = nCount;
    }
    for (size_t j = nStart; j < aCurCurrencyList.size(); j++)
    {
        if (aCurCurrencyList[j] == nPos)
            return j;
    }
    return sal_uInt16(-1);
}

bool SvxNumberFormatShell::IsInTable(sal_uInt16 const nPos, bool const bTmpBanking,
                                     OUString const& rFmtString)
{
    bool bFlag = false;

    if (nPos != sal_uInt16(-1))
    {
        const NfCurrencyTable& rCurrencyTable = SvNumberFormatter::GetTheCurrencyTable();

        if (nPos < rCurrencyTable.size())
        {
            NfWSStringsDtor aWSStringsDtor;
            pFormatter->GetCurrencyFormatStrings(aWSStringsDtor, rCurrencyTable[nPos], bTmpBanking);

            for (const OUString& s : aWSStringsDtor)
            {
                if (s == rFmtString)
                {
                    bFlag = true;
                    break;
                }
            }
        }
    }

    return bFlag;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
