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
#include "iconsets.hrc"
#include "scresid.hxx"
#if DUMP_FORMAT_INFO
#include <iostream>
#endif

#include <algorithm>

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

double ScColorScaleFormat::GetMinValue() const
{
    const_iterator itr = maColorScales.begin();

    if(itr->GetType() == COLORSCALE_VALUE || itr->GetType() == COLORSCALE_FORMULA)
        return itr->GetValue();
    else
    {
        return getMinValue();
    }
}

double ScColorScaleFormat::GetMaxValue() const
{
    ColorScaleEntries::const_reverse_iterator itr = maColorScales.rbegin();

    if(itr->GetType() == COLORSCALE_VALUE || itr->GetType() == COLORSCALE_FORMULA)
        return itr->GetValue();
    else
    {
        return getMaxValue();
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

std::vector<double>& ScColorFormat::getValues() const
{
    if(!mpCache)
    {
        mpCache.reset(new ScColorFormatCache);
        std::vector<double>& rValues = mpCache->maValues;

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

        std::sort(rValues.begin(), rValues.end());
    }

    return mpCache->maValues;
}

double ScColorFormat::getMinValue() const
{
    std::vector<double>& rValues = getValues();
    if(rValues.empty())
        return 0;
    return rValues[0];
}

double ScColorFormat::getMaxValue() const
{
    std::vector<double>& rValues = getValues();
    if(rValues.empty())
        return 0;
    return rValues[rValues.size()-1];
}

void ScColorFormat::startRendering()
{
    mpCache.reset();
}

void ScColorFormat::endRendering()
{
    mpCache.reset();
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

/**
 * @param rVector sorted vector of the array
 * @param fPercentile percentile
 */
double GetPercentile( const std::vector<double>& rArray, double fPercentile )
{
    size_t nSize = rArray.size();
    size_t nIndex = (size_t)::rtl::math::approxFloor( fPercentile * (nSize-1));
    double fDiff = fPercentile * (nSize-1) - ::rtl::math::approxFloor( fPercentile * (nSize-1));
    std::vector<double>::const_iterator iter = rArray.begin() + nIndex;
    if (fDiff == 0.0)
        return *iter;
    else
    {
        double fVal = *iter;
        iter = rArray.begin() + nIndex+1;
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
        case COLORSCALE_MAX:
            return nMax;
        case COLORSCALE_PERCENTILE:
        {
            std::vector<double>& rValues = getValues();
            if(rValues.size() == 1)
                return rValues[0];
            else
            {
                double fPercentile = itr->GetValue()/100.0;
                return GetPercentile(rValues, fPercentile);
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
        mpDoc->RepaintRange(GetRange());
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
        case COLORSCALE_AUTO:
            return true;
        default:
            return false;
    }
}

}

void ScDataBarFormat::DataChanged(const ScRange& rRange)
{
    bool bNeedUpdate = NeedUpdate(mpFormatData->mpUpperLimit.get());
    bNeedUpdate |= NeedUpdate(mpFormatData->mpLowerLimit.get());

    bNeedUpdate &= GetRange().Intersects(rRange);

    if(bNeedUpdate)
    {
        mpDoc->RepaintRange(GetRange());
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

        case COLORSCALE_AUTO:
            return std::min<double>(0, nMin);

        case COLORSCALE_PERCENT:
            return nMin + (nMax-nMin)/100*mpFormatData->mpLowerLimit->GetValue();

        case COLORSCALE_PERCENTILE:
        {
            double fPercentile = mpFormatData->mpLowerLimit->GetValue()/100.0;
            std::vector<double>& rValues = getValues();
            return GetPercentile(rValues, fPercentile);
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
        case COLORSCALE_AUTO:
            return std::max<double>(0, nMax);
        case COLORSCALE_PERCENT:
            return nMin + (nMax-nMin)/100*mpFormatData->mpUpperLimit->GetValue();
        case COLORSCALE_PERCENTILE:
        {
            double fPercentile = mpFormatData->mpUpperLimit->GetValue()/100.0;
            std::vector<double>& rValues = getValues();
            return GetPercentile(rValues, fPercentile);
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
    double nValMin = getMinValue();
    double nValMax = getMaxValue();
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

ScIconSetFormat::ScIconSetFormat(ScDocument* pDoc):
    ScColorFormat(pDoc),
    mpFormatData(new ScIconSetFormatData)
{
}

ScIconSetFormat::ScIconSetFormat(ScDocument* pDoc, const ScIconSetFormat& rFormat):
    ScColorFormat(pDoc),
    mpFormatData(new ScIconSetFormatData(*rFormat.mpFormatData))
{
}

ScColorFormat* ScIconSetFormat::Clone( ScDocument* pDoc ) const
{
    return new ScIconSetFormat(pDoc, *this);
}

void ScIconSetFormat::SetIconSetData( ScIconSetFormatData* pFormatData )
{
    mpFormatData.reset( pFormatData );
}

const ScIconSetFormatData* ScIconSetFormat::GetIconSetData() const
{
    return mpFormatData.get();
}

ScIconSetInfo* ScIconSetFormat::GetIconSetInfo(const ScAddress& rAddr) const
{
    CellType eCellType = mpDoc->GetCellType(rAddr);
    if(eCellType != CELLTYPE_VALUE && eCellType != CELLTYPE_FORMULA)
        return NULL;

    if (eCellType == CELLTYPE_FORMULA)
    {
        if(!static_cast<ScFormulaCell*>(mpDoc->GetCell(rAddr))->IsValue())
            return NULL;
    }

    ScIconSetInfo* pInfo = new ScIconSetInfo;

    // now we have for sure a value
    double nVal = mpDoc->GetValue(rAddr);

    if (mpFormatData->maEntries.size() < 2)
        return NULL;

    double nMin = GetMinValue();
    double nMax = GetMaxValue();

    // this check is for safety
    if(nMin >= nMax)
        return NULL;

    sal_Int32 nIndex = 0;
    const_iterator itr = begin();
    ++itr;
    double nValMax = CalcValue(nMin, nMax, itr);

    ++itr;
    while(itr != end() && nVal > nValMax)
    {
        ++nIndex;
        nValMax = CalcValue(nMin, nMax, itr);
        ++itr;
    }
    if(nVal > nValMax)
        ++nIndex;

    pInfo->nIconIndex = nIndex;
    pInfo->eIconSetType = mpFormatData->eIconSetType;
    return pInfo;
}

condformat::ScFormatEntryType ScIconSetFormat::GetType() const
{
    return condformat::ICONSET;
}

void ScIconSetFormat::DataChanged( const ScRange& )
{
}

void ScIconSetFormat::UpdateMoveTab( SCTAB nOldTab, SCTAB nNewTab )
{
    for(iterator itr = begin(); itr != end(); ++itr)
    {
        itr->UpdateMoveTab(nOldTab, nNewTab, 0);
    }
}

void ScIconSetFormat::UpdateReference( UpdateRefMode eUpdateRefMode,
        const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz )
{
    for(iterator itr = begin(); itr != end(); ++itr)
    {
        itr->UpdateReference( eUpdateRefMode, rRange, nDx, nDy, nDz );
    }
}

void ScIconSetFormat::dumpInfo( rtl::OUStringBuffer& rBuffer ) const
{
    rBuffer.append("IconSet: \n");
    for(const_iterator itr = begin(); itr != end(); ++itr)
    {
        itr->dumpInfo(rBuffer);
    }
}

ScIconSetFormat::iterator ScIconSetFormat::begin()
{
    return mpFormatData->maEntries.begin();
}

ScIconSetFormat::const_iterator ScIconSetFormat::begin() const
{
    return mpFormatData->maEntries.begin();
}

ScIconSetFormat::iterator ScIconSetFormat::end()
{
    return mpFormatData->maEntries.end();
}

ScIconSetFormat::const_iterator ScIconSetFormat::end() const
{
    return mpFormatData->maEntries.end();
}

double ScIconSetFormat::GetMinValue() const
{
    const_iterator itr = begin();

    if(itr->GetType() == COLORSCALE_VALUE || itr->GetType() == COLORSCALE_FORMULA)
        return itr->GetValue();
    else
    {
        return getMinValue();
    }
}

double ScIconSetFormat::GetMaxValue() const
{
    boost::ptr_vector<ScColorScaleEntry>::const_reverse_iterator itr = mpFormatData->maEntries.rbegin();

    if(itr->GetType() == COLORSCALE_VALUE || itr->GetType() == COLORSCALE_FORMULA)
        return itr->GetValue();
    else
    {
        return getMaxValue();
    }
}

double ScIconSetFormat::CalcValue(double nMin, double nMax, ScIconSetFormat::const_iterator& itr) const
{
    switch(itr->GetType())
    {
        case COLORSCALE_PERCENT:
            return nMin + (nMax-nMin)*(itr->GetValue()/100);
        case COLORSCALE_MIN:
            return nMin;
        case COLORSCALE_MAX:
            return nMax;
        case COLORSCALE_PERCENTILE:
        {
            std::vector<double>& rValues = getValues();
            if(rValues.size() == 1)
                return rValues[0];
            else
            {
                double fPercentile = itr->GetValue()/100.0;
                return GetPercentile(rValues, fPercentile);
            }
        }

        default:
        break;
    }

    return itr->GetValue();
}

ScIconSetMap* ScIconSetFormat::getIconSetMap()
{

    static ScIconSetMap aIconSetMap[] = {
        { "3Arrows", IconSet_3Arrows, 3 },
        { "3ArrowsGray", IconSet_3ArrowsGray, 3 },
        { "3Flags", IconSet_3Flags, 3 },
        { "3TrafficLights1", IconSet_3TrafficLights1, 3 },
        { "3TrafficLights2", IconSet_3TrafficLights2, 3 },
        { "3Signs", IconSet_3Signs, 3 },
        { "3Symbols", IconSet_3Symbols, 3 },
        { "3Symbols2", IconSet_3Symbols2, 3 },
        { "4Arrows", IconSet_4Arrows, 4 },
        { "4ArrowsGray", IconSet_4ArrowsGray, 4 },
        { "4RedToBlack", IconSet_4RedToBlack, 4 },
        { "4Rating", IconSet_4Rating, 4 },
        { "4TrafficLights", IconSet_4TrafficLights, 4 },
        { "5Arrows", IconSet_5Arrows, 5 },
        { "5ArrowsGray", IconSet_5ArrowsGray, 5 },
        { "5Rating", IconSet_5Ratings, 5 },
        { "5Quarters", IconSet_5Quarters, 5 },
        { NULL, IconSet_3Arrows, 0 }
    };

    return aIconSetMap;
}

namespace {

sal_Int32 a3TrafficLights1[] = {
    BMP_ICON_SET_CIRCLES1_RED, BMP_ICON_SET_CIRCLES1_YELLOW, BMP_ICON_SET_CIRCLES1_GREEN
};

sal_Int32 a3TrafficLights2[] = {
    BMP_ICON_SET_TRAFFICLIGHTS_RED, BMP_ICON_SET_TRAFFICLIGHTS_YELLOW, BMP_ICON_SET_TRAFFICLIGHTS_GREEN
};

sal_Int32 a3Arrows[] = {
    BMP_ICON_SET_COLORARROWS_DOWN, BMP_ICON_SET_COLORARROWS_SAME, BMP_ICON_SET_COLORARROWS_UP
};

sal_Int32 a3ArrowsGray[] = {
    BMP_ICON_SET_GRAYARROWS_DOWN, BMP_ICON_SET_GRAYARROWS_SAME, BMP_ICON_SET_GRAYARROWS_UP
};

sal_Int32 a3Flags[] = {
    BMP_ICON_SET_FLAGS_RED, BMP_ICON_SET_FLAGS_YELLOW, BMP_ICON_SET_FLAGS_GREEN
};

sal_Int32 a4Arrows[] = {
    BMP_ICON_SET_COLORARROWS_DOWN, BMP_ICON_SET_COLORARROWS_SLIGHTLY_DOWN, BMP_ICON_SET_COLORARROWS_SLIGHTLY_UP, BMP_ICON_SET_COLORARROWS_UP
};

sal_Int32 a4ArrowsGray[] = {
    BMP_ICON_SET_GRAYARROWS_DOWN, BMP_ICON_SET_GRAYARROWS_SLIGHTLY_DOWN, BMP_ICON_SET_GRAYARROWS_SLIGHTLY_UP, BMP_ICON_SET_GRAYARROWS_UP
};

sal_Int32 a5Arrows[] = {
    BMP_ICON_SET_COLORARROWS_DOWN, BMP_ICON_SET_COLORARROWS_SLIGHTLY_DOWN,
    BMP_ICON_SET_COLORARROWS_SAME, BMP_ICON_SET_COLORARROWS_SLIGHTLY_UP, BMP_ICON_SET_COLORARROWS_UP
};

sal_Int32 a5ArrowsGray[] = {
    BMP_ICON_SET_GRAYARROWS_DOWN, BMP_ICON_SET_GRAYARROWS_SLIGHTLY_DOWN,
    BMP_ICON_SET_GRAYARROWS_SAME, BMP_ICON_SET_GRAYARROWS_SLIGHTLY_UP, BMP_ICON_SET_GRAYARROWS_UP
};

sal_Int32 a4TrafficLights[] = {
    BMP_ICON_SET_CIRCLES1_GRAY, BMP_ICON_SET_CIRCLES1_RED,
    BMP_ICON_SET_CIRCLES1_YELLOW, BMP_ICON_SET_CIRCLES1_GREEN
};

sal_Int32 a5Quarters[] = {
    BMP_ICON_SET_PIES_EMPTY, BMP_ICON_SET_PIES_ONE_QUARTER, BMP_ICON_SET_PIES_HALF,
    BMP_ICON_SET_PIES_THREE_QUARTER, BMP_ICON_SET_PIES_FULL,
};

sal_Int32 a3Symbols1[] = {
    BMP_ICON_SET_SYMBOLS1_CROSS, BMP_ICON_SET_SYMBOLS1_EXCLAMATION_MARK, BMP_ICON_SET_SYMBOLS1_CHECK
};

sal_Int32 a3Signs[] = {
    BMP_ICON_SET_SHAPES_DIAMOND, BMP_ICON_SET_SHAPES_TRIANGLE, BMP_ICON_SET_SHAPES_CIRCLE
};

sal_Int32 a4RedToBlack[] = {
    BMP_ICON_SET_CIRCLES2_DARK_GRAY, BMP_ICON_SET_CIRCLES2_LIGHT_GRAY,
    BMP_ICON_SET_CIRCLES2_LIGHT_RED, BMP_ICON_SET_CIRCLES2_DARK_RED
};

sal_Int32 a4Ratings[] = {
    BMP_ICON_SET_BARS_ONE_QUARTER, BMP_ICON_SET_BARS_HALF,
    BMP_ICON_SET_BARS_THREE_QUARTER, BMP_ICON_SET_BARS_FULL
};

sal_Int32 a5Ratings[] = {
    BMP_ICON_SET_BARS_EMPTY, BMP_ICON_SET_BARS_ONE_QUARTER, BMP_ICON_SET_BARS_HALF,
    BMP_ICON_SET_BARS_THREE_QUARTER, BMP_ICON_SET_BARS_FULL
};

struct ScIconSetBitmapMap {
    ScIconSetType eType;
    sal_Int32* nBitmaps;
};

static ScIconSetBitmapMap aBitmapMap[] = {
    { IconSet_3Arrows, a3Arrows },
    { IconSet_3ArrowsGray, a3ArrowsGray },
    { IconSet_3Flags, a3Flags },
    { IconSet_3Signs, a3Signs },
    { IconSet_3Symbols, a3Symbols1 },
    { IconSet_3Symbols2, a3Symbols1 },
    { IconSet_3TrafficLights1, a3TrafficLights1 },
    { IconSet_3TrafficLights2, a3TrafficLights2 },
    { IconSet_4Arrows, a4Arrows },
    { IconSet_4ArrowsGray, a4ArrowsGray },
    { IconSet_4Rating, a4Ratings },
    { IconSet_4RedToBlack, a4RedToBlack },
    { IconSet_4TrafficLights, a4TrafficLights },
    { IconSet_5Arrows, a5Arrows },
    { IconSet_5ArrowsGray, a5ArrowsGray },
    { IconSet_5Quarters, a5Quarters },
    { IconSet_5Ratings, a5Ratings }
};

}

BitmapEx& ScIconSetFormat::getBitmap( ScIconSetType eType, sal_Int32 nIndex )
{
    static std::map< sal_Int32, BitmapEx > aIconSetBitmaps;

    sal_Int32 nBitmap = -1;

    for(size_t i = 0; i < SAL_N_ELEMENTS(aBitmapMap); ++i)
    {
        if(aBitmapMap[i].eType == eType)
        {
            nBitmap = *(aBitmapMap[i].nBitmaps + nIndex);
        }
    }
    assert( nBitmap != -1 );

    std::map<sal_Int32, BitmapEx>::iterator itr = aIconSetBitmaps.find( nBitmap );
    if(itr != aIconSetBitmaps.end())
        return itr->second;

    BitmapEx aBitmap = BitmapEx(ScResId(nBitmap));
    std::pair<sal_Int32, BitmapEx> aPair( nBitmap, aBitmap );
    std::pair<std::map<sal_Int32, BitmapEx>::iterator, bool> itrNew = aIconSetBitmaps.insert(aPair);
    assert(itrNew.second);

    return itrNew.first->second;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
