/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "colorscale.hxx"
#include "document.hxx"
#include "cell.hxx"

ScColorScaleEntry::ScColorScaleEntry(double nVal, const Color& rCol):
    mnVal(nVal),
    maColor(rCol),
    mpCell(NULL),
    mbMin(false),
    mbMax(false),
    mbPercent(false)
{
}

ScColorScaleEntry::ScColorScaleEntry(const ScColorScaleEntry& rEntry):
    mnVal(rEntry.mnVal),
    maColor(rEntry.maColor),
    mpCell(NULL),
    mbMin(rEntry.mbMin),
    mbMax(rEntry.mbMax),
    mbPercent(rEntry.mbPercent)
{
}

ScColorScaleEntry::ScColorScaleEntry(ScDocument* pDoc, const ScColorScaleEntry& rEntry):
    mnVal(rEntry.mnVal),
    maColor(rEntry.maColor),
    mpCell(static_cast<ScFormulaCell*>(rEntry.mpCell->Clone(*pDoc))),
    mbMin(rEntry.mbMin),
    mbMax(rEntry.mbMax),
    mbPercent(rEntry.mbPercent)
{
}

ScColorScaleEntry::~ScColorScaleEntry()
{
}

void ScColorScaleEntry::SetFormula( const rtl::OUString& rFormula, ScDocument* pDoc, const ScAddress& rAddr, formula::FormulaGrammar::Grammar eGrammar )
{
    mpCell.reset(new ScFormulaCell( pDoc, rAddr, rFormula, eGrammar ));
    mpCell->StartListeningTo( pDoc );
}

const ScTokenArray* ScColorScaleEntry::GetFormula() const
{
    if(mpCell)
    {
        return mpCell->GetCode();
    }

    return NULL;
}

double ScColorScaleEntry::GetValue() const
{
    if(mpCell)
    {
        mpCell->Interpret();
        if(mpCell->IsValue())
            return mpCell->GetValue();

        return std::numeric_limits<double>::max();
    }

    return mnVal;
}

void ScColorScaleEntry::UpdateMoveTab( SCTAB nOldTab, SCTAB nNewTab, SCTAB nTabNo )
{
    if(mpCell)
    {
        mpCell->UpdateMoveTab( nOldTab, nNewTab, nTabNo );
    }
}

void ScColorScaleEntry::UpdateReference( UpdateRefMode eUpdateRefMode,
            const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
{
    if(mpCell)
    {
        mpCell->UpdateReference( eUpdateRefMode, rRange, nDx, nDy, nDz );
    }
}

const Color& ScColorScaleEntry::GetColor() const
{
    return maColor;
}

ScColorScaleFormat::ScColorScaleFormat(ScDocument* pDoc):
    mpDoc(pDoc)
{
}

ScColorScaleFormat::ScColorScaleFormat(ScDocument* pDoc, const ScColorScaleFormat& rFormat):
    maRanges(rFormat.maRanges),
    mpDoc(pDoc)
{
    for(const_iterator itr = rFormat.begin(); itr != rFormat.end(); ++itr)
    {
        maColorScales.push_back(new ScColorScaleEntry(pDoc, *itr));
    }
}


void ScColorScaleFormat::AddEntry( ScColorScaleEntry* pEntry )
{
    maColorScales.push_back( pEntry );
}

bool ScColorScaleEntry::GetMin() const
{
    return mbMin;
}

bool ScColorScaleEntry::GetMax() const
{
    return mbMax;
}

bool ScColorScaleEntry::GetPercent() const
{
    return mbPercent;
}

bool ScColorScaleEntry::HasFormula() const
{
    return mpCell;
}

void ScColorScaleEntry::SetMin(bool bMin)
{
    mbMin = bMin;
}

void ScColorScaleEntry::SetMax(bool bMax)
{
    mbMax = bMax;
}

void ScColorScaleEntry::SetPercent(bool bPercent)
{
    mbPercent = bPercent;
}

namespace {

double getMinValue(const ScRange& rRange, ScDocument* pDoc)
{
    double aMinValue = std::numeric_limits<double>::max();
    //iterate through columns
    SCTAB nTab = rRange.aStart.Tab();
    for(SCCOL nCol = rRange.aStart.Col(); nCol <= rRange.aEnd.Col(); ++nCol)
    {
        for(SCROW nRow = rRange.aStart.Row(); nRow <= rRange.aEnd.Row(); ++nRow)
        {
            ScAddress aAddr(nCol, nRow, rRange.aStart.Tab());
            CellType eType = pDoc->GetCellType(aAddr);
            if(eType == CELLTYPE_VALUE)
            {
                double aVal = pDoc->GetValue(nCol, nRow, nTab);
                if( aVal < aMinValue )
                    aMinValue = aVal;
            }
            else if(eType == CELLTYPE_FORMULA)
            {
                if(static_cast<ScFormulaCell*>(pDoc->GetCell(aAddr))->IsValue())
                {
                    double aVal = pDoc->GetValue(nCol, nRow, nTab);
                    if( aVal < aMinValue )
                        aMinValue = aVal;
                }
            }
        }
    }
    return aMinValue;
}

double getMaxValue(const ScRange& rRange, ScDocument* pDoc)
{
    double aMaxValue = std::numeric_limits<double>::min();
    //iterate through columns
    SCTAB nTab = rRange.aStart.Tab();
    for(SCCOL nCol = rRange.aStart.Col(); nCol <= rRange.aEnd.Col(); ++nCol)
    {
        for(SCROW nRow = rRange.aStart.Row(); nRow <= rRange.aEnd.Row(); ++nRow)
        {
            ScAddress aAddr(nCol, nRow, rRange.aStart.Tab());
            CellType eType = pDoc->GetCellType(aAddr);
            if(eType == CELLTYPE_VALUE)
            {
                double aVal = pDoc->GetValue(nCol, nRow, nTab);
                if( aVal > aMaxValue )
                    aMaxValue = aVal;
            }
            else if(eType == CELLTYPE_FORMULA)
            {
                if(static_cast<ScFormulaCell*>(pDoc->GetCell(aAddr))->IsValue())
                {
                    double aVal = pDoc->GetValue(nCol, nRow, nTab);
                    if( aVal > aMaxValue )
                        aMaxValue = aVal;
                }
            }
        }
    }
    return aMaxValue;
}

}

double ScColorScaleFormat::GetMinValue() const
{
    const_iterator itr = maColorScales.begin();

    double aMinValue = std::numeric_limits<double>::max();
    if(!itr->GetMin())
        return itr->GetValue();
    else
    {
        size_t n = maRanges.size();
        for(size_t i = 0; i < n; ++i)
        {
            const ScRange* pRange = maRanges[i];
            double aVal = getMinValue(*pRange, mpDoc);
            if( aVal < aMinValue )
                aMinValue = aVal;
        }
    }

    return aMinValue;
}

double ScColorScaleFormat::GetMaxValue() const
{
    ColorScaleEntries::const_reverse_iterator itr = maColorScales.rbegin();

    double aMaxVal = std::numeric_limits<double>::min();
    if(!itr->GetMax())
        return itr->GetValue();
    else
    {
        size_t n = maRanges.size();
        for(size_t i = 0; i < n; ++i)
        {
            const ScRange* pRange = maRanges[i];
            double aVal = getMaxValue(*pRange, mpDoc);
            if( aVal > aMaxVal )
                aMaxVal = aVal;
        }
    }

    return aMaxVal;;
}

void ScColorScaleFormat::calcMinMax(double& rMin, double& rMax) const
{
    rMin = GetMinValue();
    rMax = GetMaxValue();
}

void ScColorScaleFormat::SetRange(const ScRangeList& rList)
{
    maRanges = rList;
}

const ScRangeList& ScColorScaleFormat::GetRange() const
{
    return maRanges;
}

namespace {

sal_uInt8 GetColorValue( double nVal, double nVal1, sal_uInt8 nColVal1, double nVal2, sal_uInt8 nColVal2 )
{
    if (nVal <= nVal1)
        return nColVal1;

    if (nVal >= nVal2)
        return nColVal2;

    sal_uInt8 nColVal = static_cast<sal_uInt8>((nVal - nVal1)/(nVal2-nVal1)*(nColVal2-nColVal1))+nColVal1;
    return nColVal;
}

Color CalcColor( double nVal, double nVal1, const Color& rCol1, double nVal2, const Color& rCol2)
{
    sal_uInt8 nColRed = GetColorValue(nVal, nVal1, rCol1.GetRed(), nVal2, rCol2.GetRed());
    sal_uInt8 nColBlue = GetColorValue(nVal, nVal1, rCol1.GetBlue(), nVal2, rCol2.GetBlue());
    sal_uInt8 nColGreen = GetColorValue(nVal, nVal1, rCol1.GetGreen(), nVal2, rCol2.GetGreen());

    return Color(nColRed, nColGreen, nColBlue);
}

double CalcValue(double nMin, double nMax, ScColorScaleFormat::const_iterator& itr)
{
    if(itr->GetPercent())
    {
        return nMin + (nMax-nMin)*(itr->GetValue()/100);
    }
    else if(itr->GetMin())
    {
        return nMin;
    }
    else if(itr->GetMax())
    {
        return nMax;
    }

    return itr->GetValue();
}

}

Color* ScColorScaleFormat::GetColor( const ScAddress& rAddr ) const
{
    CellType eCellType = mpDoc->GetCellType(rAddr);
    if(eCellType != CELLTYPE_VALUE && eCellType != CELLTYPE_FORMULA)
        return NULL;

    if (eCellType == CELLTYPE_FORMULA)
    {
        if(!static_cast<ScFormulaCell*>(mpDoc->GetCell(rAddr))->IsValue())
            return NULL;
    }

    // now we have for sure a value
    double nVal = mpDoc->GetValue(rAddr);

    if (maColorScales.size() < 2)
        return NULL;

    double nMin = std::numeric_limits<double>::max();
    double nMax = std::numeric_limits<double>::min();
    calcMinMax(nMin, nMax);

    // this check is for safety
    if(nMin >= nMax)
        return NULL;

    const_iterator itr = begin();
    double nValMin = CalcValue(nMin, nMax, itr);
    Color rColMin = itr->GetColor();
    ++itr;
    double nValMax = CalcValue(nMin, nMax, itr);
    Color rColMax = itr->GetColor();

    ++itr;
    while(itr != end() && nVal > nValMax)
    {
        rColMin = rColMax;
        nValMin = nValMax;
        rColMax = itr->GetColor();
        nValMax = CalcValue(nMin, nMax, itr);
        ++itr;
    }

    Color aColor = CalcColor(nVal, nValMin, rColMin, nValMax, rColMax);

    return new Color(aColor);
}

void ScColorScaleFormat::UpdateMoveTab(SCTAB nOldTab, SCTAB nNewTab)
{
    size_t n = maRanges.size();
    SCTAB nMinTab = std::min<SCTAB>(nOldTab, nNewTab);
    SCTAB nMaxTab = std::max<SCTAB>(nOldTab, nNewTab);
    SCTAB nThisTab = -1;
    for(size_t i = 0; i < n; ++i)
    {
        ScRange* pRange = maRanges[i];
        SCTAB nTab = pRange->aStart.Tab();
        if(nTab < nMinTab || nTab > nMaxTab)
        {
            nThisTab = nTab;
            continue;
        }

        if(nTab == nOldTab)
        {
            pRange->aStart.SetTab(nNewTab);
            pRange->aEnd.SetTab(nNewTab);
            nThisTab = nNewTab;
            continue;
        }

        if(nNewTab < nOldTab)
        {
            pRange->aStart.IncTab();
            pRange->aEnd.IncTab();
            nThisTab = nTab + 1;
        }
        else
        {
            pRange->aStart.IncTab(-1);
            pRange->aEnd.IncTab(-1);
            nThisTab = nTab - 1;
        }
    }

    if(nThisTab == -1)
        nThisTab = 0;
    for(iterator itr = begin(); itr != end(); ++itr)
    {
        itr->UpdateMoveTab(nOldTab, nNewTab, nThisTab);
    }
}

void ScColorScaleFormat::UpdateReference( UpdateRefMode eUpdateRefMode,
            const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
{
    maRanges.UpdateReference( eUpdateRefMode, mpDoc, rRange, nDx, nDy, nDz );

    for(iterator itr = begin(); itr != end(); ++itr)
    {
        itr->UpdateReference(eUpdateRefMode, rRange, nDx, nDy, nDz);
    }
}

bool ScColorScaleFormat::CheckEntriesForRel(const ScRange& rRange) const
{
    bool bNeedUpdate = false;
    for(const_iterator itr = begin(); itr != end(); ++itr)
    {
        if(itr->GetMin() || itr->GetMax())
        {
            bNeedUpdate = true;
            break;
        }

        if(itr->HasFormula())
            return true;
    }

    // TODO: check also if the changed value is the new min/max
    // or has been the old min/max value
    bNeedUpdate = bNeedUpdate && maRanges.Intersects(rRange);
    return bNeedUpdate;
}

void ScColorScaleFormat::DataChanged(const ScRange& rRange)
{
    bool bNeedUpdate = CheckEntriesForRel(rRange);
    if(bNeedUpdate)
    {
        size_t n = maRanges.size();
        for(size_t i = 0; i < n; ++i)
        {
            ScRange* pRange = maRanges[i];
            mpDoc->RepaintRange(*pRange);
        }
    }
}

ScColorScaleFormat::iterator ScColorScaleFormat::begin()
{
    return maColorScales.begin();
}

ScColorScaleFormat::const_iterator ScColorScaleFormat::begin() const
{
    return maColorScales.begin();
}

ScColorScaleFormat::iterator ScColorScaleFormat::end()
{
    return maColorScales.end();
}

ScColorScaleFormat::const_iterator ScColorScaleFormat::end() const
{
    return maColorScales.end();
}

ScColorScaleFormatList::ScColorScaleFormatList(ScDocument* pDoc, const ScColorScaleFormatList& rList)
{
    for(const_iterator itr = rList.begin(); itr != rList.end(); ++itr)
    {
        maColorScaleFormats.push_back(new ScColorScaleFormat(pDoc, *itr));
    }
}

void ScColorScaleFormatList::AddFormat( ScColorScaleFormat* pFormat )
{
    maColorScaleFormats.push_back( pFormat );
}

// attention nFormat is 1 based, 0 is reserved for no format
ScColorScaleFormat* ScColorScaleFormatList::GetFormat(sal_uInt32 nFormat)
{
    if( nFormat > size() || !nFormat )
        return NULL;

    return &maColorScaleFormats[nFormat-1];
}

ScColorScaleFormatList::iterator ScColorScaleFormatList::begin()
{
    return maColorScaleFormats.begin();
}

ScColorScaleFormatList::const_iterator ScColorScaleFormatList::begin() const
{
    return maColorScaleFormats.begin();
}

ScColorScaleFormatList::iterator ScColorScaleFormatList::end()
{
    return maColorScaleFormats.end();
}

ScColorScaleFormatList::const_iterator ScColorScaleFormatList::end() const
{
    return maColorScaleFormats.end();
}

size_t ScColorScaleFormatList::size() const
{
    return maColorScaleFormats.size();
}

void ScColorScaleFormatList::DataChanged(const ScRange& rRange)
{
    for(iterator itr = begin(); itr != end(); ++itr)
    {
        itr->DataChanged(rRange);
    }
}

void ScColorScaleFormatList::UpdateMoveTab(SCTAB nOldTab, SCTAB nNewTab)
{
    for(iterator itr = begin(); itr != end(); ++itr)
    {
        itr->UpdateMoveTab(nOldTab, nNewTab);
    }
}

void ScColorScaleFormatList::UpdateReference( UpdateRefMode eUpdateRefMode,
            const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
{
    for(iterator itr = begin(); itr != end(); ++itr)
    {
        itr->UpdateReference( eUpdateRefMode, rRange, nDx, nDy, nDz );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
