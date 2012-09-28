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
#include "fillinfo.hxx"
#if DUMP_FORMAT_INFO
#include <iostream>
#endif

ScColorScaleEntry::ScColorScaleEntry():
    mnVal(0),
    mpCell(NULL),
    meType(COLORSCALE_VALUE)
{
}

ScColorScaleEntry::ScColorScaleEntry(double nVal, const Color& rCol):
    mnVal(nVal),
    maColor(rCol),
    mpCell(NULL),
    meType(COLORSCALE_VALUE)
{
}

ScColorScaleEntry::ScColorScaleEntry(const ScColorScaleEntry& rEntry):
    mnVal(rEntry.mnVal),
    maColor(rEntry.maColor),
    mpCell(),
    meType(rEntry.meType)
{
}

ScColorScaleEntry::ScColorScaleEntry(ScDocument* pDoc, const ScColorScaleEntry& rEntry):
    mnVal(rEntry.mnVal),
    maColor(rEntry.maColor),
    mpCell(),
    meType(rEntry.meType)
{
    if(rEntry.mpCell)
    {
        mpCell.reset(static_cast<ScFormulaCell*>(rEntry.mpCell->Clone(*pDoc, SC_CLONECELL_NOMAKEABS_EXTERNAL)));
        mpCell->StartListeningTo( pDoc );
    }
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

rtl::OUString ScColorScaleEntry::GetFormula( formula::FormulaGrammar::Grammar eGrammar ) const
{
    rtl::OUString aFormula;
    if(mpCell)
    {
        mpCell->GetFormula(aFormula, eGrammar);
    }

    return aFormula;
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

void ScColorScaleEntry::SetValue(double nValue)
{
    mnVal = nValue;
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

void ScColorScaleEntry::SetColor(const Color& rColor)
{
    maColor = rColor;
}

#if DUMP_FORMAT_INFO

void ScColorScaleEntry::dumpInfo(rtl::OUStringBuffer& rBuf) const
{
    rBuf.append("Color Scale Entry\n");
    rBuf.append("Type: ");
    switch(meType)
    {
        case COLORSCALE_VALUE:
            rBuf.append( "Value\n" );
            break;
        case COLORSCALE_MIN:
            rBuf.append( "Min\n" );
            break;
        case COLORSCALE_MAX:
            rBuf.append( "Max\n" );
            break;
        case COLORSCALE_PERCENT:
            rBuf.append( "Percent\n" );
            break;
        case COLORSCALE_PERCENTILE:
            rBuf.append( "Percentile\n" );
            break;
        case COLORSCALE_FORMULA:
            rBuf.append( "Formual\n" );
            break;
        default:
            rBuf.append( "Unsupported Type\n" );
    }
    rBuf.append( "Color: " ).append( (sal_Int32)maColor.GetRed() ).append( "," ).append( (sal_Int32)maColor.GetGreen() ).append( "," ).append( (sal_Int32)maColor.GetBlue() ).append( "\n" );
    if(meType == COLORSCALE_FORMULA)
        rBuf.append( "Formula: " ).append( GetFormula( formula::FormulaGrammar::GRAM_DEFAULT ) ).append("\n");
    else if( meType != COLORSCALE_MIN && meType != COLORSCALE_MAX )
        rBuf.append( "Value: " ).append( mnVal ).append( "\n" );
}

#endif

ScColorFormat::ScColorFormat(ScDocument* pDoc):
    ScFormatEntry(pDoc)
{
}

ScColorFormat::~ScColorFormat()
{
}

void ScColorFormat::SetParent( ScConditionalFormat* pParent )
{
    mpParent = pParent;
}

ScColorScaleFormat::ScColorScaleFormat(ScDocument* pDoc):
    ScColorFormat(pDoc)
{
}

ScColorScaleFormat::ScColorScaleFormat(ScDocument* pDoc, const ScColorScaleFormat& rFormat):
    ScColorFormat(pDoc)
{
    for(const_iterator itr = rFormat.begin(); itr != rFormat.end(); ++itr)
    {
        maColorScales.push_back(new ScColorScaleEntry(pDoc, *itr));
    }
}

ScColorFormat* ScColorScaleFormat::Clone(ScDocument* pDoc) const
{
    return new ScColorScaleFormat(pDoc, *this);
}

ScColorScaleFormat::~ScColorScaleFormat()
{
}

void ScColorScaleFormat::AddEntry( ScColorScaleEntry* pEntry )
{
    maColorScales.push_back( pEntry );
}

void ScColorScaleEntry::SetType( ScColorScaleEntryType eType )
{
    meType = eType;
    if(eType != COLORSCALE_FORMULA)
        mpCell.reset();
}

ScColorScaleEntryType ScColorScaleEntry::GetType() const
{
    return meType;
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

double getMinValue(const ScRangeList& rList, ScDocument* pDoc)
{
    double aMinValue = std::numeric_limits<double>::max();

    size_t n = rList.size();
    for(size_t i = 0; i < n; ++i)
    {
        const ScRange* pRange = rList[i];
        double aVal = getMinValue(*pRange, pDoc);
        if( aVal < aMinValue )
            aMinValue = aVal;
    }
    return aMinValue;
}

double getMaxValue(const ScRangeList& rList, ScDocument* pDoc)
{
    double aMaxVal = std::numeric_limits<double>::min();

    size_t n = rList.size();
    for(size_t i = 0; i < n; ++i)
    {
        const ScRange* pRange = rList[i];
        double aVal = getMaxValue(*pRange, pDoc);
        if( aVal > aMaxVal )
            aMaxVal = aVal;
    }

    return aMaxVal;
}

}

double ScColorScaleFormat::GetMinValue() const
{
    const_iterator itr = maColorScales.begin();

    if(itr->GetType() == COLORSCALE_VALUE || itr->GetType() == COLORSCALE_FORMULA)
        return itr->GetValue();
    else
    {
        return getMinValue(GetRange(), mpDoc);
    }
}

double ScColorScaleFormat::GetMaxValue() const
{
    ColorScaleEntries::const_reverse_iterator itr = maColorScales.rbegin();

    if(itr->GetType() == COLORSCALE_VALUE || itr->GetType() == COLORSCALE_FORMULA)
        return itr->GetValue();
    else
    {
        return getMaxValue(GetRange(), mpDoc);
    }
}

void ScColorScaleFormat::calcMinMax(double& rMin, double& rMax) const
{
    rMin = GetMinValue();
    rMax = GetMaxValue();
}

const ScRangeList& ScColorFormat::GetRange() const
{
    return mpParent->GetRange();
}

void ScColorFormat::getValues(std::vector<double>& rValues) const
{
    size_t n = GetRange().size();
    const ScRangeList& aRanges = GetRange();
    for(size_t i = 0; i < n; ++i)
    {
        const ScRange* pRange = aRanges[i];
        SCTAB nTab = pRange->aStart.Tab();
        for(SCCOL nCol = pRange->aStart.Col(); nCol <= pRange->aEnd.Col(); ++nCol)
        {
            for(SCCOL nRow = pRange->aStart.Row(); nRow <= pRange->aEnd.Row(); ++nRow)
            {
                ScAddress aAddr(nCol, nRow, nTab);
                CellType eType = mpDoc->GetCellType(aAddr);
                if(eType == CELLTYPE_VALUE)
                {
                    double aVal = mpDoc->GetValue(nCol, nRow, nTab);
                    rValues.push_back(aVal);
                }
                else if(eType == CELLTYPE_FORMULA)
                {
                    if(static_cast<ScFormulaCell*>(mpDoc->GetCell(aAddr))->IsValue())
                    {
                        double aVal = mpDoc->GetValue(nCol, nRow, nTab);
                        rValues.push_back(aVal);
                    }
                }
            }
        }
    }
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

double GetPercentile( std::vector<double>& rArray, double fPercentile )
{
    size_t nSize = rArray.size();
    size_t nIndex = (size_t)::rtl::math::approxFloor( fPercentile * (nSize-1));
    double fDiff = fPercentile * (nSize-1) - ::rtl::math::approxFloor( fPercentile * (nSize-1));
    std::vector<double>::iterator iter = rArray.begin() + nIndex;
    ::std::nth_element( rArray.begin(), iter, rArray.end());
    if (fDiff == 0.0)
        return *iter;
    else
    {
        double fVal = *iter;
        iter = rArray.begin() + nIndex+1;
        ::std::nth_element( rArray.begin(), iter, rArray.end());
        return fVal + fDiff * (*iter - fVal);
    }
}

}

double ScColorScaleFormat::CalcValue(double nMin, double nMax, ScColorScaleFormat::const_iterator& itr) const
{
    switch(itr->GetType())
    {
        case COLORSCALE_PERCENT:
            return nMin + (nMax-nMin)*(itr->GetValue()/100);
        case COLORSCALE_MIN:
            return nMin;
        case COLORSCALE_AUTOMIN:
            return std::min<double>(0, nMin);
        case COLORSCALE_MAX:
            return nMax;
        case COLORSCALE_AUTOMAX:
            return std::max<double>(0, nMax);
        case COLORSCALE_PERCENTILE:
        {
            std::vector<double> aValues;
            getValues(aValues);
            if(aValues.size() == 1)
                return aValues[0];
            else
            {
                double fPercentile = itr->GetValue()/100.0;
                return GetPercentile(aValues, fPercentile);
            }
        }

        default:
        break;
    }

    return itr->GetValue();
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

#if DUMP_FORMAT_INFO
void ScColorScaleFormat::dumpInfo(rtl::OUStringBuffer& rBuf) const
{
    rBuf.append("Color Scale with ").append(static_cast<sal_Int32>(size())).append(" entries\n");
    for(const_iterator itr = begin(); itr != end(); ++itr)
    {
        itr->dumpInfo(rBuf);
    }

    const ScRangeList& rRange = GetRange();
    size_t n = rRange.size();
    for(size_t i = 0; i < n; ++i)
    {
        const ScRange* pRange = rRange[i];
        SCTAB nTab = pRange->aStart.Tab();
        for( SCCOL nCol = pRange->aStart.Col(), nEndCol = pRange->aEnd.Col(); nCol <= nEndCol; ++nCol)
        {
            for( SCROW nRow = pRange->aStart.Row(), nEndRow = pRange->aEnd.Row(); nRow <= nEndRow; ++nRow)
            {
                boost::scoped_ptr<Color> pColor( GetColor(ScAddress(nCol, nRow, nTab)) );
                rBuf.append((sal_Int32)nCol).append(",").append(nRow).append(",").append((sal_Int32)nTab).append(",");
                rBuf.append(((sal_Int32)pColor->GetRed())).append(",").append(((sal_Int32)pColor->GetGreen())).append(",").append(((sal_Int32)pColor->GetBlue())).append("\n");
            }
        }
    }
}
#endif

void ScColorScaleFormat::UpdateMoveTab(SCTAB nOldTab, SCTAB nNewTab)
{
    SCTAB nThisTab = GetRange().front()->aStart.Tab();
    for(iterator itr = begin(); itr != end(); ++itr)
    {
        itr->UpdateMoveTab(nOldTab, nNewTab, nThisTab);
    }
}

void ScColorScaleFormat::UpdateReference( UpdateRefMode eUpdateRefMode,
            const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
{
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
        ScColorScaleEntryType eType = itr->GetType();
        switch(eType)
        {
            case COLORSCALE_MIN:
            case COLORSCALE_MAX:
            case COLORSCALE_AUTOMIN:
            case COLORSCALE_AUTOMAX:
                bNeedUpdate = true;
                break;
            case COLORSCALE_FORMULA:
                return true;
            default:
                break;
        }
    }

    // TODO: check also if the changed value is the new min/max
    // or has been the old min/max value
    bNeedUpdate = bNeedUpdate && GetRange().Intersects(rRange);
    return bNeedUpdate;
}

void ScColorScaleFormat::DataChanged(const ScRange& rRange)
{
    bool bNeedUpdate = CheckEntriesForRel(rRange);
    if(bNeedUpdate)
    {
        size_t n = GetRange().size();
        for(size_t i = 0; i < n; ++i)
        {
            const ScRange* pRange = GetRange()[i];
            mpDoc->RepaintRange(*pRange);
        }
    }
}

condformat::ScFormatEntryType ScColorScaleFormat::GetType() const
{
    return condformat::COLORSCALE;
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

size_t ScColorScaleFormat::size() const
{
    return maColorScales.size();
}

ScDataBarFormat::ScDataBarFormat(ScDocument* pDoc):
    ScColorFormat(pDoc)
{
}

ScDataBarFormat::ScDataBarFormat(ScDocument* pDoc, const ScDataBarFormat& rFormat):
    ScColorFormat(pDoc),
    mpFormatData(new ScDataBarFormatData(*rFormat.mpFormatData))
{
}

void ScDataBarFormat::SetDataBarData( ScDataBarFormatData* pData )
{
    mpFormatData.reset(pData);
}

const ScDataBarFormatData* ScDataBarFormat::GetDataBarData() const
{
    return mpFormatData.get();
}

ScColorFormat* ScDataBarFormat::Clone(ScDocument* pDoc) const
{
    return new ScDataBarFormat(pDoc, *this);
}

condformat::ScFormatEntryType ScDataBarFormat::GetType() const
{
    return condformat::DATABAR;
}

void ScDataBarFormat::UpdateReference( UpdateRefMode eRefMode,
            const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
{
    mpFormatData->mpUpperLimit->UpdateReference( eRefMode, rRange, nDx, nDy, nDz );
    mpFormatData->mpLowerLimit->UpdateReference( eRefMode, rRange, nDx, nDy, nDz );
}

namespace {

bool NeedUpdate(ScColorScaleEntry* pEntry)
{
    switch(pEntry->GetType())
    {
        case COLORSCALE_MIN:
        case COLORSCALE_MAX:
        case COLORSCALE_FORMULA:
        case COLORSCALE_AUTOMIN:
        case COLORSCALE_AUTOMAX:
            return true;
        default:
            return false;
    }
}

}

void ScDataBarFormat::DataChanged(const ScRange& rRange)
{
    bool bNeedUpdate = false;

    bNeedUpdate = NeedUpdate(mpFormatData->mpUpperLimit.get());
    bNeedUpdate &= NeedUpdate(mpFormatData->mpLowerLimit.get());

    bNeedUpdate &= GetRange().Intersects(rRange);

    if(bNeedUpdate)
    {
        size_t n = GetRange().size();
        for(size_t i = 0; i < n; ++i)
        {
            const ScRange* pRange = GetRange()[i];
            mpDoc->RepaintRange(*pRange);
        }
    }
}

void ScDataBarFormat::UpdateMoveTab(SCTAB nOldTab, SCTAB nNewTab)
{
    SCTAB nThisTab = GetRange().front()->aStart.Tab();
    mpFormatData->mpUpperLimit->UpdateMoveTab(nOldTab, nNewTab, nThisTab);
    mpFormatData->mpLowerLimit->UpdateMoveTab(nOldTab, nNewTab, nThisTab);
}

double ScDataBarFormat::getMin(double nMin, double nMax) const
{
    switch(mpFormatData->mpLowerLimit->GetType())
    {
        case COLORSCALE_MIN:
            return nMin;

        case COLORSCALE_AUTOMIN:
            return std::min<double>(0, nMin);

        case COLORSCALE_PERCENT:
            return nMin + (nMax-nMin)/100*mpFormatData->mpLowerLimit->GetValue();

        case COLORSCALE_PERCENTILE:
        {
            double fPercentile = mpFormatData->mpLowerLimit->GetValue()/100.0;
            std::vector<double> aValues;
            getValues(aValues);
            return GetPercentile(aValues, fPercentile);
        }

        default:
        break;
    }

    return mpFormatData->mpLowerLimit->GetValue();
}

double ScDataBarFormat::getMax(double nMin, double nMax) const
{
    switch(mpFormatData->mpUpperLimit->GetType())
    {
        case COLORSCALE_MAX:
            return nMax;
        case COLORSCALE_AUTOMAX:
            return std::max<double>(0, nMax);
        case COLORSCALE_PERCENT:
            return nMin + (nMax-nMin)/100*mpFormatData->mpUpperLimit->GetValue();
        case COLORSCALE_PERCENTILE:
        {
            double fPercentile = mpFormatData->mpUpperLimit->GetValue()/100.0;
            std::vector<double> aValues;
            getValues(aValues);
            return GetPercentile(aValues, fPercentile);
        }

        default:
            break;
    }

    return mpFormatData->mpUpperLimit->GetValue();
}

ScDataBarInfo* ScDataBarFormat::GetDataBarInfo(const ScAddress& rAddr) const
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
    //
    double nValMin = getMinValue(GetRange(), mpDoc);
    double nValMax = getMaxValue(GetRange(), mpDoc);
    double nMin = getMin(nValMin, nValMax);
    double nMax = getMax(nValMin, nValMax);


    double nValue = mpDoc->GetValue(rAddr);

    ScDataBarInfo* pInfo = new ScDataBarInfo();
    if(mpFormatData->meAxisPosition == databar::NONE)
    {
        if(nValue <= nMin)
        {
            pInfo->mnLength = 0;
        }
        else if(nValue >= nMax)
        {
            pInfo->mnLength = 100;
        }
        else
        {
            double nDiff = nMax - nMin;
            pInfo->mnLength = (nValue - nMin)/nDiff*100.0;
        }
        pInfo->mnZero = 0;
    }
    else
    {
        double nMinPositive = 0;
        double nMaxNegative = 0;
        //calculate the zero position first
        if(mpFormatData->meAxisPosition == databar::AUTOMATIC)
        {
            if(nMin < 0)
            {
                if(nMax < 0)
                    pInfo->mnZero = 100;
                else
                {
                    pInfo->mnZero = -100*nMin/(nMax-nMin);
                }
            }
            else
                pInfo->mnZero = 0;

            // if max or min is used we may need to adjust it
            // for the length calculation
            if (mpFormatData->mpLowerLimit->GetType() == COLORSCALE_MIN && nMin > 0)
                nMinPositive = nMin;
            if (mpFormatData->mpUpperLimit->GetType() == COLORSCALE_MAX && nMax < 0)
                nMaxNegative = nMax;
        }
        else if( mpFormatData->meAxisPosition == databar::MIDDLE)
            pInfo->mnZero = 50;

        //calculate the length
        if(nValue < 0)
        {
            if (nValue < nMin)
                pInfo->mnLength = -100;
            else
                pInfo->mnLength = -100 * (nValue-nMaxNegative)/(nMin-nMaxNegative);
        }
        else
        {
            if ( nValue > nMax )
                pInfo->mnLength = 100;
            else
                pInfo->mnLength = (nValue-nMinPositive)/(nMax-nMinPositive)*100;
        }
    }


    // set color
    if(mpFormatData->mbNeg && nValue < 0)
    {
        if(mpFormatData->mpNegativeColor)
        {
            pInfo->maColor = *mpFormatData->mpNegativeColor.get();
        }
        else
        {
            // default negative color is red
            pInfo->maColor = COL_LIGHTRED;
        }

    }
    else
        pInfo->maColor = mpFormatData->maPositiveColor;

    pInfo->mbGradient = mpFormatData->mbGradient;
    pInfo->mbShowValue = !mpFormatData->mbOnlyBar;
    pInfo->maAxisColor = mpFormatData->maAxisColor;

    return pInfo;
}

#if DUMP_FORMAT_INFO
void ScDataBarFormat::dumpInfo(rtl::OUStringBuffer& rBuf) const
{
    const ScRangeList& rRange = GetRange();
    size_t n = rRange.size();
    for(size_t i = 0; i < n; ++i)
    {
        const ScRange* pRange = rRange[i];
        SCTAB nTab = pRange->aStart.Tab();
        for( SCCOL nCol = pRange->aStart.Col(), nEndCol = pRange->aEnd.Col(); nCol <= nEndCol; ++nCol)
        {
            for( SCROW nRow = pRange->aStart.Row(), nEndRow = pRange->aEnd.Row(); nRow <= nEndRow; ++nRow)
            {
                boost::scoped_ptr<ScDataBarInfo> pInfo( GetDataBarInfo(ScAddress(nCol, nRow, nTab)) );
                rBuf.append((sal_Int32) nCol).append(",").append(nRow).append(",").append((sal_Int32) nTab).append(",").append(pInfo->mnZero).append(",");
                rBuf.append(pInfo->mnLength).append(",").append((sal_Bool)pInfo->mbGradient).append(",").append((sal_Bool)pInfo->mbShowValue).append("\n");
            }
        }
    }
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
