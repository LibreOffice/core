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

#include "XMLStylesImportHelper.hxx"
#include "xmlimprt.hxx"
#include <com/sun/star/util/NumberFormat.hpp>
#include <osl/diagnose.h>

using namespace com::sun::star;
using ::std::list;

void ScMyStyleNumberFormats::AddStyleNumberFormat(const OUString& rStyleName, const sal_Int32 nNumberFormat)
{
    aSet.insert(ScMyStyleNumberFormat(rStyleName, nNumberFormat));
}

sal_Int32 ScMyStyleNumberFormats::GetStyleNumberFormat(const OUString& rStyleName)
{
    ScMyStyleNumberFormat aStyleNumberFormat(rStyleName);
    ScMyStyleNumberFormatSet::iterator aItr(aSet.find(aStyleNumberFormat));
    if (aItr == aSet.end())
        return -1;
    else
        return aItr->nNumberFormat;
}

ScMyStyleRanges::ScMyStyleRanges()
{
}

ScMyStyleRanges::~ScMyStyleRanges()
{
}

void ScMyStyleRanges::AddRange(const ScRange& rRange, const sal_Int16 nType)
{
    switch (nType)
    {
        case util::NumberFormat::NUMBER:
        {
            if (!mpNumberList)
                mpNumberList.reset(new ScSimpleRangeList);
            mpNumberList->addRange(rRange);
        }
        break;
        case util::NumberFormat::TEXT:
        {
            if (!mpTextList)
                mpTextList.reset(new ScSimpleRangeList);
            mpTextList->addRange(rRange);
        }
        break;
        case util::NumberFormat::TIME:
        {
            if (!mpTimeList)
                mpTimeList.reset(new ScSimpleRangeList);
            mpTimeList->addRange(rRange);
        }
        break;
        case util::NumberFormat::DATETIME:
        {
            if (!mpDateTimeList)
                mpDateTimeList.reset(new ScSimpleRangeList);
            mpDateTimeList->addRange(rRange);
        }
        break;
        case util::NumberFormat::PERCENT:
        {
            if (!mpPercentList)
                mpPercentList.reset(new ScSimpleRangeList);
            mpPercentList->addRange(rRange);
        }
        break;
        case util::NumberFormat::LOGICAL:
        {
            if (!mpLogicalList)
                mpLogicalList.reset(new ScSimpleRangeList);
            mpLogicalList->addRange(rRange);
        }
        break;
        case util::NumberFormat::UNDEFINED:
        {
            if (!mpUndefinedList)
                mpUndefinedList.reset(new ScSimpleRangeList);
            mpUndefinedList->addRange(rRange);
        }
        break;
        default:
        {
            OSL_FAIL("wrong type");
        }
        break;
    }
}

void ScMyStyleRanges::AddCurrencyRange(const ScRange& rRange, const boost::optional<OUString> & pCurrency)
{
    if (!pCurrencyList)
        pCurrencyList.reset( new ScMyCurrencyStylesSet );
    ScMyCurrencyStyle aStyle;
    if (pCurrency)
        aStyle.sCurrency = *pCurrency;
    ScMyCurrencyStylesSet::iterator aItr(pCurrencyList->find(aStyle));
    if (aItr == pCurrencyList->end())
    {
        std::pair<ScMyCurrencyStylesSet::iterator, bool> aPair(pCurrencyList->insert(aStyle));
        if (aPair.second)
        {
            aItr = aPair.first;
            aItr->mpRanges->addRange(rRange);
        }
    }
    else
        aItr->mpRanges->addRange(rRange);
}

void ScMyStyleRanges::InsertCol(const sal_Int32 nCol, const sal_Int32 nTab)
{
    if (mpTextList)
        mpTextList->insertCol(static_cast<SCCOL>(nCol), static_cast<SCTAB>(nTab));
    if (mpNumberList)
        mpNumberList->insertCol(static_cast<SCCOL>(nCol), static_cast<SCTAB>(nTab));
    if (mpTimeList)
        mpTimeList->insertCol(static_cast<SCCOL>(nCol), static_cast<SCTAB>(nTab));
    if (mpDateTimeList)
        mpDateTimeList->insertCol(static_cast<SCCOL>(nCol), static_cast<SCTAB>(nTab));
    if (mpPercentList)
        mpPercentList->insertCol(static_cast<SCCOL>(nCol), static_cast<SCTAB>(nTab));
    if (mpLogicalList)
        mpLogicalList->insertCol(static_cast<SCCOL>(nCol), static_cast<SCTAB>(nTab));
    if (mpUndefinedList)
        mpUndefinedList->insertCol(static_cast<SCCOL>(nCol), static_cast<SCTAB>(nTab));

    if (pCurrencyList)
    {
        for (auto& rCurrency : *pCurrencyList)
        {
            rCurrency.mpRanges->insertCol(static_cast<SCCOL>(nCol), static_cast<SCTAB>(nTab));
        }
    }
}

void ScMyStyleRanges::SetStylesToRanges(const list<ScRange>& rRanges,
    const OUString* pStyleName, const sal_Int16 nCellType,
    const OUString* pCurrency, ScXMLImport& rImport)
{
    for (const auto& rRange : rRanges)
        rImport.SetStyleToRange(rRange, pStyleName, nCellType, pCurrency);
}

void ScMyStyleRanges::SetStylesToRanges(const OUString* pStyleName, ScXMLImport& rImport)
{
    if (mpNumberList)
    {
        list<ScRange> aList;
        mpNumberList->getRangeList(aList);
        SetStylesToRanges(aList, pStyleName, util::NumberFormat::NUMBER, nullptr, rImport);
        mpNumberList->clear();
    }
    if (mpTextList)
    {
        list<ScRange> aList;
        mpTextList->getRangeList(aList);
        SetStylesToRanges(aList, pStyleName, util::NumberFormat::TEXT, nullptr, rImport);
        mpTextList->clear();
    }
    if (mpTimeList)
    {
        list<ScRange> aList;
        mpTimeList->getRangeList(aList);
        SetStylesToRanges(aList, pStyleName, util::NumberFormat::TIME, nullptr, rImport);
        mpTimeList->clear();
    }
    if (mpDateTimeList)
    {
        list<ScRange> aList;
        mpDateTimeList->getRangeList(aList);
        SetStylesToRanges(aList, pStyleName, util::NumberFormat::DATETIME, nullptr, rImport);
        mpDateTimeList->clear();
    }
    if (mpPercentList)
    {
        list<ScRange> aList;
        mpPercentList->getRangeList(aList);
        SetStylesToRanges(aList, pStyleName, util::NumberFormat::PERCENT, nullptr, rImport);
        mpPercentList->clear();
    }
    if (mpLogicalList)
    {
        list<ScRange> aList;
        mpLogicalList->getRangeList(aList);
        SetStylesToRanges(aList, pStyleName, util::NumberFormat::LOGICAL, nullptr, rImport);
        mpLogicalList->clear();
    }
    if (mpUndefinedList)
    {
        list<ScRange> aList;
        mpUndefinedList->getRangeList(aList);
        SetStylesToRanges(aList, pStyleName, util::NumberFormat::UNDEFINED, nullptr, rImport);
        mpUndefinedList->clear();
    }
    if (pCurrencyList)
    {
        for (const auto& rCurrency : *pCurrencyList)
        {
            list<ScRange> aList;
            rCurrency.mpRanges->getRangeList(aList);
            SetStylesToRanges(aList, pStyleName, util::NumberFormat::CURRENCY, &rCurrency.sCurrency, rImport);
        }
    }
}

ScMyStylesImportHelper::ScMyStylesImportHelper(ScXMLImport& rTempImport)
    :
    aRowDefaultStyle(aCellStyles.end()),
    rImport(rTempImport),
    nCellType(0),
    nPrevCellType(0),
    bPrevRangeAdded(true)
{
}

ScMyStylesImportHelper::~ScMyStylesImportHelper()
{
}

void ScMyStylesImportHelper::ResetAttributes()
{
    pPrevStyleName = std::move(pStyleName);
    pPrevCurrency = std::move(pCurrency);
    nPrevCellType = nCellType;
    nCellType = 0;
}

ScMyStylesSet::iterator ScMyStylesImportHelper::GetIterator(const boost::optional<OUString> & pStyleNameP)
{
    ScMyStyle aStyle;
    if (pStyleNameP)
        aStyle.sStyleName = *pStyleNameP;
    else
    {
        OSL_FAIL("here is no stylename given");
    }
    ScMyStylesSet::iterator aItr(aCellStyles.find(aStyle));
    if (aItr == aCellStyles.end())
    {
        std::pair<ScMyStylesSet::iterator, bool> aPair(aCellStyles.insert(aStyle));
        if (aPair.second)
            aItr = aPair.first;
        else
        {
            OSL_FAIL("not possible to insert style");
            return aCellStyles.end();
        }
    }
    return aItr;
}

void ScMyStylesImportHelper::AddDefaultRange(const ScRange& rRange)
{
    OSL_ENSURE(aRowDefaultStyle != aCellStyles.end(), "no row default style");
    if (aRowDefaultStyle->sStyleName.isEmpty())
    {
        SCCOL nStartCol(rRange.aStart.Col());
        SCCOL nEndCol(rRange.aEnd.Col());
        if (aColDefaultStyles.size() > sal::static_int_cast<sal_uInt32>(nStartCol))
        {
            ScMyStylesSet::iterator aPrevItr(aColDefaultStyles[nStartCol]);
            OSL_ENSURE(aColDefaultStyles.size() > sal::static_int_cast<sal_uInt32>(nEndCol), "to much columns");
            for (SCCOL i = nStartCol + 1; (i <= nEndCol) && (i < sal::static_int_cast<SCCOL>(aColDefaultStyles.size())); ++i)
            {
                if (aPrevItr != aColDefaultStyles[i])
                {
                    OSL_ENSURE(aPrevItr != aCellStyles.end(), "no column default style");
                    ScRange aRange(rRange);
                    aRange.aStart.SetCol(nStartCol);
                    aRange.aEnd.SetCol(i - 1);
                    pPrevStyleName = aPrevItr->sStyleName;
                    AddSingleRange(aRange);
                    nStartCol = i;
                    aPrevItr = aColDefaultStyles[i];
                }
            }
            if (aPrevItr != aCellStyles.end())
            {
                ScRange aRange(rRange);
                aRange.aStart.SetCol(nStartCol);
                pPrevStyleName = aPrevItr->sStyleName;
                AddSingleRange(aRange);
            }
            else
            {
                OSL_FAIL("no column default style");
            }
        }
        else
        {
            OSL_FAIL("too much columns");
        }
    }
    else
    {
        pPrevStyleName = aRowDefaultStyle->sStyleName;
        AddSingleRange(rRange);
    }
}

void ScMyStylesImportHelper::AddSingleRange(const ScRange& rRange)
{
    ScMyStylesSet::iterator aItr(GetIterator(pPrevStyleName));
    if (aItr != aCellStyles.end())
    {
        if (nPrevCellType != util::NumberFormat::CURRENCY)
            aItr->xRanges->AddRange(rRange, nPrevCellType);
        else
            aItr->xRanges->AddCurrencyRange(rRange, pPrevCurrency);
    }
}

void ScMyStylesImportHelper::AddRange()
{
    if (pPrevStyleName && !pPrevStyleName->isEmpty())
        AddSingleRange(aPrevRange);
    else
        AddDefaultRange(aPrevRange);
    ResetAttributes();
}

void ScMyStylesImportHelper::AddColumnStyle(const OUString& sStyleName, const sal_Int32 nColumn, const sal_Int32 nRepeat)
{
    OSL_ENSURE(static_cast<sal_uInt32>(nColumn) == aColDefaultStyles.size(), "some columns are absent");
    ScMyStylesSet::iterator aItr(GetIterator(sStyleName));
    OSL_ENSURE(aItr != aCellStyles.end(), "no column default style");
    aColDefaultStyles.reserve(aColDefaultStyles.size() + nRepeat);
    for (sal_Int32 i = 0; i < nRepeat; ++i)
        aColDefaultStyles.push_back(aItr);
}

void ScMyStylesImportHelper::SetRowStyle(const OUString& sStyleName)
{
    aRowDefaultStyle = GetIterator(sStyleName);
}

void ScMyStylesImportHelper::SetAttributes(boost::optional<OUString> pStyleNameP,
    boost::optional<OUString> pCurrencyP, const sal_Int16 nCellTypeP)
{
    pStyleName = std::move(pStyleNameP);
    pCurrency = std::move(pCurrencyP);
    nCellType = nCellTypeP;
}

void ScMyStylesImportHelper::AddRange(const ScRange& rRange)
{
    if (!bPrevRangeAdded)
    {
        bool bAddRange(false);
        if (nCellType == nPrevCellType &&
            pStyleName == pPrevStyleName &&
            pCurrency == pPrevCurrency)
        {
            if (rRange.aStart.Row() == aPrevRange.aStart.Row())
            {
                if (rRange.aEnd.Row() == aPrevRange.aEnd.Row())
                {
                    OSL_ENSURE(aPrevRange.aEnd.Col() + 1 == rRange.aStart.Col(), "something wents wrong");
                    aPrevRange.aEnd.SetCol(rRange.aEnd.Col());
                }
                else
                    bAddRange = true;
            }
            else
            {
                if (rRange.aStart.Col() == aPrevRange.aStart.Col() &&
                    rRange.aEnd.Col() == aPrevRange.aEnd.Col())
                {
                    OSL_ENSURE(aPrevRange.aEnd.Row() + 1 == rRange.aStart.Row(), "something wents wrong");
                    aPrevRange.aEnd.SetRow(rRange.aEnd.Row());
                }
                else
                    bAddRange = true;
            }
        }
        else
            bAddRange = true;
        if (bAddRange)
        {
            AddRange();
            aPrevRange = rRange;
        }
    }
    else
    {
        aPrevRange = rRange;
        ResetAttributes();
        bPrevRangeAdded = false;
    }
}

void ScMyStylesImportHelper::AddCell(const ScAddress& rAddress)
{
    ScRange aScRange( rAddress, rAddress );
    AddRange(aScRange);
}

void ScMyStylesImportHelper::InsertCol(const sal_Int32 nCol, const sal_Int32 nTab)
{
    ScXMLImport::MutexGuard aGuard(rImport);
    for (auto& rCellStyle : aCellStyles)
    {
        rCellStyle.xRanges->InsertCol(nCol, nTab);
    }
}

void ScMyStylesImportHelper::EndTable()
{
    if (!bPrevRangeAdded)
    {
        AddRange();
        bPrevRangeAdded = true;
    }
}

void ScMyStylesImportHelper::SetStylesToRanges()
{
    for (auto& rCellStyle : aCellStyles)
    {
        rCellStyle.xRanges->SetStylesToRanges(&rCellStyle.sStyleName, rImport);
    }
    aColDefaultStyles.clear();
    aCellStyles.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
