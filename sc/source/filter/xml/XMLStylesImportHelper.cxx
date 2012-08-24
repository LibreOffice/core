/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "XMLStylesImportHelper.hxx"
#include "xmlimprt.hxx"
#include <com/sun/star/util/NumberFormat.hpp>

using namespace com::sun::star;
using ::std::list;

void ScMyStyleNumberFormats::AddStyleNumberFormat(const rtl::OUString& rStyleName, const sal_Int32 nNumberFormat)
{
    aSet.insert(ScMyStyleNumberFormat(rStyleName, nNumberFormat));
}

sal_Int32 ScMyStyleNumberFormats::GetStyleNumberFormat(const rtl::OUString& rStyleName)
{
    ScMyStyleNumberFormat aStyleNumberFormat(rStyleName);
    ScMyStyleNumberFormatSet::iterator aItr(aSet.find(aStyleNumberFormat));
    if (aItr == aSet.end())
        return -1;
    else
        return aItr->nNumberFormat;
}

ScMyStyleRanges::ScMyStyleRanges() :
    pCurrencyList(NULL)
{
}

ScMyStyleRanges::~ScMyStyleRanges()
{
    delete pCurrencyList;
}

void ScMyStyleRanges::AddRange(const ScRange& rRange,
    const rtl::OUString* /*pStyleName*/, const sal_Int16 nType,
    ScXMLImport& /*rImport*/, const sal_uInt32 /*nMaxRanges*/)
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

void ScMyStyleRanges::AddCurrencyRange(const ScRange& rRange,
    const rtl::OUString* /*pStyleName*/, const rtl::OUString* pCurrency,
    ScXMLImport& /*rImport*/, const sal_uInt32 /*nMaxRanges*/)
{
    if (!pCurrencyList)
        pCurrencyList = new ScMyCurrencyStylesSet();
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

void ScMyStyleRanges::InsertCol(const sal_Int32 nCol, const sal_Int32 nTab, ScDocument* /*pDoc*/)
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
        ScMyCurrencyStylesSet::iterator aItr(pCurrencyList->begin());
        ScMyCurrencyStylesSet::iterator aEndItr(pCurrencyList->end());
        while (aItr != aEndItr)
        {
            aItr->mpRanges->insertCol(static_cast<SCCOL>(nCol), static_cast<SCTAB>(nTab));
            ++aItr;
        }
    }
}

void ScMyStyleRanges::SetStylesToRanges(const list<ScRange>& rRanges,
    const rtl::OUString* pStyleName, const sal_Int16 nCellType,
    const rtl::OUString* pCurrency, ScXMLImport& rImport)
{
    list<ScRange>::const_iterator itr = rRanges.begin(), itrEnd = rRanges.end();
    for (; itr != itrEnd; ++itr)
        rImport.SetStyleToRange(*itr, pStyleName, nCellType, pCurrency);
}

void ScMyStyleRanges::SetStylesToRanges(const rtl::OUString* pStyleName, ScXMLImport& rImport)
{
    if (mpNumberList)
    {
        list<ScRange> aList;
        mpNumberList->getRangeList(aList);
        SetStylesToRanges(aList, pStyleName, util::NumberFormat::NUMBER, NULL, rImport);
        mpNumberList->clear();
    }
    if (mpTextList)
    {
        list<ScRange> aList;
        mpTextList->getRangeList(aList);
        SetStylesToRanges(aList, pStyleName, util::NumberFormat::TEXT, NULL, rImport);
        mpTextList->clear();
    }
    if (mpTimeList)
    {
        list<ScRange> aList;
        mpTimeList->getRangeList(aList);
        SetStylesToRanges(aList, pStyleName, util::NumberFormat::TIME, NULL, rImport);
        mpTimeList->clear();
    }
    if (mpDateTimeList)
    {
        list<ScRange> aList;
        mpDateTimeList->getRangeList(aList);
        SetStylesToRanges(aList, pStyleName, util::NumberFormat::DATETIME, NULL, rImport);
        mpDateTimeList->clear();
    }
    if (mpPercentList)
    {
        list<ScRange> aList;
        mpPercentList->getRangeList(aList);
        SetStylesToRanges(aList, pStyleName, util::NumberFormat::PERCENT, NULL, rImport);
        mpPercentList->clear();
    }
    if (mpLogicalList)
    {
        list<ScRange> aList;
        mpLogicalList->getRangeList(aList);
        SetStylesToRanges(aList, pStyleName, util::NumberFormat::LOGICAL, NULL, rImport);
        mpLogicalList->clear();
    }
    if (mpUndefinedList)
    {
        list<ScRange> aList;
        mpUndefinedList->getRangeList(aList);
        SetStylesToRanges(aList, pStyleName, util::NumberFormat::UNDEFINED, NULL, rImport);
        mpUndefinedList->clear();
    }
    if (pCurrencyList)
    {
        ScMyCurrencyStylesSet::iterator aItr(pCurrencyList->begin());
        ScMyCurrencyStylesSet::iterator aEndItr(pCurrencyList->end());
        while (aItr != aEndItr)
        {
            list<ScRange> aList;
            aItr->mpRanges->getRangeList(aList);
            SetStylesToRanges(aList, pStyleName, util::NumberFormat::CURRENCY, &aItr->sCurrency, rImport);
            ++aItr;
        }
    }
}

//----------------------------------------------------------------------------

ScMyStylesImportHelper::ScMyStylesImportHelper(ScXMLImport& rTempImport)
    :
    aRowDefaultStyle(aCellStyles.end()),
    rImport(rTempImport),
    pStyleName(NULL),
    pPrevStyleName(NULL),
    pCurrency(NULL),
    pPrevCurrency(NULL),
    nMaxRanges(0),
    bPrevRangeAdded(true)
{
}

ScMyStylesImportHelper::~ScMyStylesImportHelper()
{
    delete pPrevStyleName;
    delete pPrevCurrency;
    delete pStyleName;
    delete pCurrency;
}

void ScMyStylesImportHelper::ResetAttributes()
{
    delete pPrevStyleName;
    delete pPrevCurrency;
    pPrevStyleName = pStyleName;
    pPrevCurrency = pCurrency;
    nPrevCellType = nCellType;
    pStyleName = NULL;
    pCurrency = NULL;
    nCellType = 0;
}

ScMyStylesSet::iterator ScMyStylesImportHelper::GetIterator(const rtl::OUString* pStyleNameP)
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
                    delete pPrevStyleName;
                    pPrevStyleName = new rtl::OUString(aPrevItr->sStyleName);
                    AddSingleRange(aRange);
                    nStartCol = i;
                    aPrevItr = aColDefaultStyles[i];
                }
            }
            if (aPrevItr != aCellStyles.end())
            {
                ScRange aRange(rRange);
                aRange.aStart.SetCol(nStartCol);
                delete pPrevStyleName;
                pPrevStyleName = new rtl::OUString(aPrevItr->sStyleName);
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
        delete pPrevStyleName;
        pPrevStyleName = new rtl::OUString(aRowDefaultStyle->sStyleName);
        AddSingleRange(rRange);
    }
}

void ScMyStylesImportHelper::AddSingleRange(const ScRange& rRange)
{
    if (nMaxRanges == 0)
        nMaxRanges = aColDefaultStyles.size();
    ScMyStylesSet::iterator aItr(GetIterator(pPrevStyleName));
    if (aItr != aCellStyles.end())
    {
        if (nPrevCellType != util::NumberFormat::CURRENCY)
            aItr->xRanges->AddRange(rRange, pPrevStyleName, nPrevCellType,
                rImport, nMaxRanges);
        else
            aItr->xRanges->AddCurrencyRange(rRange, pPrevStyleName, pPrevCurrency,
                rImport, nMaxRanges);
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

void ScMyStylesImportHelper::AddColumnStyle(const rtl::OUString& sStyleName, const sal_Int32 nColumn, const sal_Int32 nRepeat)
{
    (void)nColumn;  // avoid warning in product version
    OSL_ENSURE(static_cast<sal_uInt32>(nColumn) == aColDefaultStyles.size(), "some columns are absent");
    ScMyStylesSet::iterator aItr(GetIterator(&sStyleName));
    OSL_ENSURE(aItr != aCellStyles.end(), "no column default style");
    aColDefaultStyles.reserve(aColDefaultStyles.size() + nRepeat);
    for (sal_Int32 i = 0; i < nRepeat; ++i)
        aColDefaultStyles.push_back(aItr);
}

void ScMyStylesImportHelper::SetRowStyle(const rtl::OUString& sStyleName)
{
    aRowDefaultStyle = GetIterator(&sStyleName);
}

void ScMyStylesImportHelper::SetAttributes(rtl::OUString* pStyleNameP,
    rtl::OUString* pCurrencyP, const sal_Int16 nCellTypeP)
{
    delete this->pStyleName;
    delete this->pCurrency;
    this->pStyleName = pStyleNameP;
    this->pCurrency = pCurrencyP;
    this->nCellType = nCellTypeP;
}

void ScMyStylesImportHelper::AddRange(const ScRange& rRange)
{
    if (!bPrevRangeAdded)
    {
        bool bAddRange(false);
        if (nCellType == nPrevCellType &&
            IsEqual(pStyleName, pPrevStyleName) &&
            IsEqual(pCurrency, pPrevCurrency))
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

void ScMyStylesImportHelper::InsertCol(const sal_Int32 nCol, const sal_Int32 nTab, ScDocument* pDoc)
{
    ScXMLImport::MutexGuard aGuard(rImport);
    ScMyStylesSet::iterator aItr(aCellStyles.begin());
    ScMyStylesSet::iterator aEndItr(aCellStyles.end());
    while (aItr != aEndItr)
    {
        aItr->xRanges->InsertCol(nCol, nTab, pDoc);
        ++aItr;
    }
}

void ScMyStylesImportHelper::EndTable()
{
    if (!bPrevRangeAdded)
    {
        AddRange();
        bPrevRangeAdded = true;
    }
    nMaxRanges = 0;
}

void ScMyStylesImportHelper::SetStylesToRanges()
{
    ScMyStylesSet::iterator aItr(aCellStyles.begin());
    ScMyStylesSet::iterator aEndItr(aCellStyles.end());
    while (aItr != aEndItr)
    {
        aItr->xRanges->SetStylesToRanges(&aItr->sStyleName, rImport);
        ++aItr;
    }
    aColDefaultStyles.clear();
    aCellStyles.clear();
    nMaxRanges = 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
