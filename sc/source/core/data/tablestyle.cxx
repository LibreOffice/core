/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tablestyle.hxx>

ScTableStyle::ScTableStyle(const OUString& rName, const std::optional<OUString>& rUIName)
    : mnFirstRowStripeSize(1)
    , mnSecondRowStripeSize(1)
    , mnFirstColStripeSize(1)
    , mnSecondColStripeSize(1)
    , maStyleName(rName)
    , maUIName(rUIName)
    , mbIsOOXMLDefault(false)
{
}

const SfxItemSet* ScTableStyle::GetTableCellItemSet(const ScDBData& rDBData, SCCOL nCol, SCROW nRow,
                                                    SCROW nRowIndex) const
{
    const ScTableStyleParam* pParam = rDBData.GetTableStyleInfo();
    ScRange aRange;
    rDBData.GetArea(aRange);

    bool bHasHeader = rDBData.HasHeader();
    bool bHasTotal = rDBData.HasTotals();
    if (bHasHeader && mpLastHeaderCellPattern && nRow == aRange.aStart.Row()
        && nCol == aRange.aEnd.Col())
    {
        return &mpLastHeaderCellPattern->GetItemSet();
    }

    if (bHasHeader && mpFirstHeaderCellPattern && nRow == aRange.aStart.Row()
        && nCol == aRange.aStart.Col())
    {
        return &mpFirstHeaderCellPattern->GetItemSet();
    }

    if (bHasTotal && mpTotalRowPattern && nRow == aRange.aEnd.Row())
    {
        return &mpTotalRowPattern->GetItemSet();
    }

    if (bHasHeader && mpHeaderRowPattern && nRow == aRange.aStart.Row())
    {
        return &mpHeaderRowPattern->GetItemSet();
    }

    if (pParam->mbFirstColumn && mpFirstColumnPattern && nCol == aRange.aStart.Col())
    {
        return &mpFirstColumnPattern->GetItemSet();
    }

    if (pParam->mbLastColumn && mpLastColumnPattern && nCol == aRange.aEnd.Col())
    {
        return &mpLastColumnPattern->GetItemSet();
    }

    if (!bHasTotal || aRange.aEnd.Row() != nRow)
    {
        if (pParam->mbRowStripes && nRowIndex >= 0)
        {
            sal_Int32 nTotalRowStripPattern = mnFirstRowStripeSize + mnSecondRowStripeSize;
            bool bFirstRowStripe = (nRowIndex % nTotalRowStripPattern) < mnFirstRowStripeSize;
            if (mpSecondRowStripePattern && !bFirstRowStripe)
            {
                return &mpSecondRowStripePattern->GetItemSet();
            }

            if (mpFirstRowStripePattern && bFirstRowStripe)
            {
                return &mpFirstRowStripePattern->GetItemSet();
            }
        }

        if (pParam->mbColumnStripes)
        {
            SCCOL nRelativeCol = nCol - aRange.aStart.Col();
            sal_Int32 nTotalColStripePattern = mnFirstColStripeSize + mnSecondColStripeSize;
            bool bFirstColStripe = (nRelativeCol % nTotalColStripePattern) < mnFirstColStripeSize;
            if (mpSecondColumnStripePattern && !bFirstColStripe)
            {
                return &mpSecondColumnStripePattern->GetItemSet();
            }

            if (mpFirstColumnStripePattern && bFirstColStripe)
            {
                return &mpFirstColumnStripePattern->GetItemSet();
            }
        }
    }

    if (mpTablePattern)
    {
        return &mpTablePattern->GetItemSet();
    }

    return nullptr;
}

void ScTableStyle::SetRowStripeSize(sal_Int32 nFirstRowStripeSize, sal_Int32 nSecondRowStripeSize)
{
    if (nFirstRowStripeSize >= 1)
        mnFirstRowStripeSize = nFirstRowStripeSize;

    if (nSecondRowStripeSize >= 1)
        mnSecondRowStripeSize = nSecondRowStripeSize;
}

void ScTableStyle::SetColStripeSize(sal_Int32 nFirstColStripeSize, sal_Int32 nSecondColStripeSize)
{
    if (nFirstColStripeSize >= 1)
        mnFirstColStripeSize = nFirstColStripeSize;

    if (nSecondColStripeSize >= 1)
        mnSecondColStripeSize = nSecondColStripeSize;
}

void ScTableStyle::SetOOXMLDefault(bool bDefault) { mbIsOOXMLDefault = bDefault; }

bool ScTableStyle::IsOOXMLDefault() const { return mbIsOOXMLDefault; }

const OUString& ScTableStyle::GetName() const { return maStyleName; }

const OUString& ScTableStyle::GetUIName() const
{
    if (maUIName)
        return *maUIName;

    return maStyleName;
}

void ScTableStyle::SetPattern(ScTableStyleElement eTableStyleElement,
                              std::unique_ptr<ScPatternAttr> pPattern)
{
    switch (eTableStyleElement)
    {
        case ScTableStyleElement::WholeTable:
            mpTablePattern = std::move(pPattern);
            break;
        case ScTableStyleElement::FirstColumnStripe:
            mpFirstColumnStripePattern = std::move(pPattern);
            break;
        case ScTableStyleElement::SecondColumnStripe:
            mpSecondColumnStripePattern = std::move(pPattern);
            break;
        case ScTableStyleElement::FirstRowStripe:
            mpFirstRowStripePattern = std::move(pPattern);
            break;
        case ScTableStyleElement::SecondRowStripe:
            mpSecondRowStripePattern = std::move(pPattern);
            break;
        case ScTableStyleElement::LastColumn:
            mpLastColumnPattern = std::move(pPattern);
            break;
        case ScTableStyleElement::FirstColumn:
            mpFirstColumnPattern = std::move(pPattern);
            break;
        case ScTableStyleElement::HeaderRow:
            mpHeaderRowPattern = std::move(pPattern);
            break;
        case ScTableStyleElement::TotalRow:
            mpTotalRowPattern = std::move(pPattern);
            break;
        case ScTableStyleElement::FirstHeaderCell:
            mpFirstHeaderCellPattern = std::move(pPattern);
            break;
        case ScTableStyleElement::LastHeaderCell:
            mpLastHeaderCellPattern = std::move(pPattern);
            break;
    }
}

void ScTableStyle::SetTablePattern(std::unique_ptr<ScPatternAttr> pPattern)
{
    mpTablePattern = std::move(pPattern);
}

void ScTableStyle::SetFirstColumnStripePattern(std::unique_ptr<ScPatternAttr> pPattern)
{
    mpFirstColumnStripePattern = std::move(pPattern);
}

void ScTableStyle::SetSecondColumnStripePattern(std::unique_ptr<ScPatternAttr> pPattern)
{
    mpSecondColumnStripePattern = std::move(pPattern);
}

void ScTableStyle::SetFirstRowStripePattern(std::unique_ptr<ScPatternAttr> pPattern)
{
    mpFirstRowStripePattern = std::move(pPattern);
}

void ScTableStyle::SetSecondRowStripePattern(std::unique_ptr<ScPatternAttr> pPattern)
{
    mpSecondRowStripePattern = std::move(pPattern);
}

void ScTableStyle::SetLastColumnPattern(std::unique_ptr<ScPatternAttr> pPattern)
{
    mpLastColumnPattern = std::move(pPattern);
}

void ScTableStyle::SetFirstColumnPattern(std::unique_ptr<ScPatternAttr> pPattern)
{
    mpFirstColumnPattern = std::move(pPattern);
}

void ScTableStyle::SetHeaderRowPattern(std::unique_ptr<ScPatternAttr> pPattern)
{
    mpHeaderRowPattern = std::move(pPattern);
}

void ScTableStyle::SetTotalRowPattern(std::unique_ptr<ScPatternAttr> pPattern)
{
    mpTotalRowPattern = std::move(pPattern);
}

void ScTableStyle::SetFirstHeaderCellPattern(std::unique_ptr<ScPatternAttr> pPattern)
{
    mpFirstHeaderCellPattern = std::move(pPattern);
}

void ScTableStyle::SetLastHeaderCellPattern(std::unique_ptr<ScPatternAttr> pPattern)
{
    mpLastHeaderCellPattern = std::move(pPattern);
}

std::map<ScTableStyleElement, const ScPatternAttr*> ScTableStyle::GetSetPatterns() const
{
    std::map<ScTableStyleElement, const ScPatternAttr*> aPatterns;
    if (mpTablePattern)
    {
        aPatterns.emplace(ScTableStyleElement::WholeTable, mpTablePattern.get());
    }

    if (mpFirstColumnStripePattern)
    {
        aPatterns.emplace(ScTableStyleElement::FirstColumnStripe, mpFirstColumnStripePattern.get());
    }

    if (mpSecondColumnStripePattern)
    {
        aPatterns.emplace(ScTableStyleElement::SecondColumnStripe,
                          mpSecondColumnStripePattern.get());
    }

    if (mpFirstRowStripePattern)
    {
        aPatterns.emplace(ScTableStyleElement::FirstRowStripe, mpFirstRowStripePattern.get());
    }

    if (mpSecondRowStripePattern)
    {
        aPatterns.emplace(ScTableStyleElement::SecondRowStripe, mpSecondRowStripePattern.get());
    }

    if (mpLastColumnPattern)
    {
        aPatterns.emplace(ScTableStyleElement::LastColumn, mpLastColumnPattern.get());
    }

    if (mpFirstColumnPattern)
    {
        aPatterns.emplace(ScTableStyleElement::FirstColumn, mpFirstColumnPattern.get());
    }

    if (mpHeaderRowPattern)
    {
        aPatterns.emplace(ScTableStyleElement::HeaderRow, mpHeaderRowPattern.get());
    }

    if (mpTotalRowPattern)
    {
        aPatterns.emplace(ScTableStyleElement::TotalRow, mpTotalRowPattern.get());
    }

    if (mpFirstHeaderCellPattern)
    {
        aPatterns.emplace(ScTableStyleElement::FirstHeaderCell, mpFirstHeaderCellPattern.get());
    }

    if (mpLastHeaderCellPattern)
    {
        aPatterns.emplace(ScTableStyleElement::LastHeaderCell, mpLastHeaderCellPattern.get());
    }

    return aPatterns;
}

sal_Int32 ScTableStyle::GetFirstRowStripeSize() const { return mnFirstRowStripeSize; }

sal_Int32 ScTableStyle::GetSecondRowStripeSize() const { return mnSecondRowStripeSize; }

sal_Int32 ScTableStyle::GetFirstColumnStripeSize() const { return mnFirstColStripeSize; }

sal_Int32 ScTableStyle::GetSecondColumnStripeSize() const { return mnSecondColStripeSize; }

ScTableStyles::ScTableStyles() {}

void ScTableStyles::AddTableStyle(std::unique_ptr<ScTableStyle> pTableStyle)
{
    maTableStyles.insert({ pTableStyle->GetName(), std::move(pTableStyle) });
}

void ScTableStyles::DeleteTableStyle(const OUString& rName) { maTableStyles.erase(rName); }

const ScTableStyle* ScTableStyles::GetTableStyle(const OUString& rName) const
{
    if (maTableStyles.find(rName) == maTableStyles.end())
        return nullptr;

    return maTableStyles.find(rName)->second.get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
