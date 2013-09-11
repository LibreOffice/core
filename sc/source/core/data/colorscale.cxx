/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "colorscale.hxx"
#include "document.hxx"
#include "formulacell.hxx"
#include "fillinfo.hxx"
#include "iconsets.hrc"
#include "scresid.hxx"
#include "tokenarray.hxx"
#include "refupdatecontext.hxx"

#include "formula/token.hxx"

#include <algorithm>

class ScFormulaListener : public SvtListener
{
private:
    std::vector<ScRange> maCells;
    mutable bool mbDirty;
    ScDocument* mpDoc;

    void startListening(ScTokenArray* pTokens, const ScAddress& rPos);

public:
    ScFormulaListener(ScFormulaCell* pCell);
    virtual ~ScFormulaListener();

    void Notify( SvtBroadcaster& rBC, const SfxHint& rHint );

    bool NeedsRepaint() const;
};

ScFormulaListener::ScFormulaListener(ScFormulaCell* pCell):
    mbDirty(false),
    mpDoc(pCell->GetDocument())
{
    startListening( pCell->GetCode(), pCell->aPos );
}

void ScFormulaListener::startListening(ScTokenArray* pArr, const ScAddress& rPos)
{
    pArr->Reset();
    ScToken* t;
    while ( ( t = static_cast<ScToken*>(pArr->GetNextReferenceRPN()) ) != NULL )
    {
        switch (t->GetType())
        {
            case formula::svSingleRef:
            {
                ScAddress aCell =  t->GetSingleRef().toAbs(rPos);
                if (aCell.IsValid())
                    mpDoc->StartListeningCell(aCell, this);

                maCells.push_back(aCell);
            }
            break;
            case formula::svDoubleRef:
            {
                const ScSingleRefData& rRef1 = t->GetSingleRef();
                const ScSingleRefData& rRef2 = t->GetSingleRef2();
                ScAddress aCell1 = rRef1.toAbs(rPos);
                ScAddress aCell2 = rRef2.toAbs(rPos);
                if (aCell1.IsValid() && aCell2.IsValid())
                {
                    if (t->GetOpCode() == ocColRowNameAuto)
                    {   // automagically
                        if ( rRef1.IsColRel() )
                        {   // ColName
                            aCell2.SetRow(MAXROW);
                        }
                        else
                        {   // RowName
                            aCell2.SetCol(MAXCOL);
                        }
                    }
                    mpDoc->StartListeningArea(ScRange(aCell1, aCell2), this);
                    maCells.push_back(ScRange(aCell1, aCell2));
                }
            }
            break;
            default:
                ;   // nothing
        }
    }

}

namespace {

struct StopListeningCell
{
    StopListeningCell(ScDocument* pDoc, SvtListener* pListener):
        mpDoc(pDoc), mpListener(pListener) {}

    void operator()(const ScRange& rRange)
    {
        for(SCTAB nTab = rRange.aStart.Tab(),
                nTabEnd = rRange.aEnd.Tab(); nTab <= nTabEnd; ++nTab)
        {
            for(SCCOL nCol = rRange.aStart.Col(),
                    nColEnd = rRange.aEnd.Col(); nCol <= nColEnd; ++nCol)
            {
                for(SCROW nRow = rRange.aStart.Row(),
                        nRowEnd = rRange.aEnd.Row(); nRow <= nRowEnd; ++nRow)
                {
                    mpDoc->EndListeningCell(ScAddress(nCol, nRow, nTab), mpListener);
                }
            }
        }
    }

private:
    ScDocument* mpDoc;
    SvtListener* mpListener;
};

}

ScFormulaListener::~ScFormulaListener()
{
    std::for_each(maCells.begin(), maCells.end(), StopListeningCell(mpDoc, this));
}

void ScFormulaListener::Notify(SvtBroadcaster&, const SfxHint&)
{
    mbDirty = true;
}

bool ScFormulaListener::NeedsRepaint() const
{
    bool bRet = mbDirty;
    mbDirty = false;
    return bRet;
}

ScColorScaleEntry::ScColorScaleEntry():
    mnVal(0),
    meType(COLORSCALE_VALUE)
{
}

ScColorScaleEntry::ScColorScaleEntry(double nVal, const Color& rCol):
    mnVal(nVal),
    maColor(rCol),
    meType(COLORSCALE_VALUE)
{
}

ScColorScaleEntry::ScColorScaleEntry(const ScColorScaleEntry& rEntry):
    mnVal(rEntry.mnVal),
    maColor(rEntry.maColor),
    meType(rEntry.meType)
{
    if(rEntry.mpCell)
    {
        mpCell.reset(new ScFormulaCell(*rEntry.mpCell, *rEntry.mpCell->GetDocument(), rEntry.mpCell->aPos, SC_CLONECELL_NOMAKEABS_EXTERNAL));
        mpCell->StartListeningTo( mpCell->GetDocument() );
        mpListener.reset(new ScFormulaListener(mpCell.get()));
    }
}

ScColorScaleEntry::ScColorScaleEntry(ScDocument* pDoc, const ScColorScaleEntry& rEntry):
    mnVal(rEntry.mnVal),
    maColor(rEntry.maColor),
    mpCell(),
    meType(rEntry.meType)
{
    if(rEntry.mpCell)
    {
        mpCell.reset(new ScFormulaCell(*rEntry.mpCell, *rEntry.mpCell->GetDocument(), rEntry.mpCell->aPos, SC_CLONECELL_NOMAKEABS_EXTERNAL));
        mpCell->StartListeningTo( pDoc );
        mpListener.reset(new ScFormulaListener(mpCell.get()));
    }
}

ScColorScaleEntry::~ScColorScaleEntry()
{
    if(mpCell)
        mpCell->EndListeningTo(mpCell->GetDocument());
}

void ScColorScaleEntry::SetFormula( const OUString& rFormula, ScDocument* pDoc, const ScAddress& rAddr, formula::FormulaGrammar::Grammar eGrammar )
{
    mpCell.reset(new ScFormulaCell( pDoc, rAddr, rFormula, eGrammar ));
    mpCell->StartListeningTo( pDoc );
    mpListener.reset(new ScFormulaListener(mpCell.get()));
}

const ScTokenArray* ScColorScaleEntry::GetFormula() const
{
    if(mpCell)
    {
        return mpCell->GetCode();
    }

    return NULL;
}

OUString ScColorScaleEntry::GetFormula( formula::FormulaGrammar::Grammar eGrammar ) const
{
    OUString aFormula;
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
    mpCell.reset();
}

void ScColorScaleEntry::UpdateMoveTab( SCTAB nOldTab, SCTAB nNewTab, SCTAB nTabNo )
{
    if(mpCell)
    {
        sc::RefUpdateMoveTabContext aCxt(nOldTab, nNewTab);
        mpCell->UpdateMoveTab(aCxt, nTabNo);
        mpListener.reset(new ScFormulaListener(mpCell.get()));
    }
}

void ScColorScaleEntry::UpdateReference( sc::RefUpdateContext& rCxt )
{
    if (!mpCell)
        return;

    mpCell->UpdateReference(rCxt);
    mpListener.reset(new ScFormulaListener(mpCell.get()));
}

void ScColorScaleEntry::UpdateInsertTab( sc::RefUpdateInsertTabContext& rCxt )
{
    if (!mpCell)
        return;

    mpCell->UpdateInsertTab(rCxt);
    mpListener.reset(new ScFormulaListener(mpCell.get()));
}

void ScColorScaleEntry::UpdateDeleteTab( sc::RefUpdateDeleteTabContext& rCxt )
{
    if (!mpCell)
        return;

    mpCell->UpdateDeleteTab(rCxt);
    mpListener.reset(new ScFormulaListener(mpCell.get()));
}

void ScColorScaleEntry::UpdateMoveTab( sc::RefUpdateMoveTabContext& rCxt )
{
    if (!mpCell)
        return;

    SCTAB nTabNo = rCxt.getNewTab(mpCell->aPos.Tab());
    mpCell->UpdateMoveTab(rCxt, nTabNo);
    mpListener.reset(new ScFormulaListener(mpCell.get()));
}

bool ScColorScaleEntry::NeedsRepaint() const
{
    if(mpListener)
        return mpListener->NeedsRepaint();

    return false;
}

const Color& ScColorScaleEntry::GetColor() const
{
    return maColor;
}

void ScColorScaleEntry::SetColor(const Color& rColor)
{
    maColor = rColor;
}

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
    {
        mpCell.reset();
        mpListener.reset();
    }
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

            SCCOL nColStart = pRange->aStart.Col();
            SCROW nRowStart = pRange->aStart.Row();
            SCCOL nColEnd = pRange->aEnd.Col();
            SCROW nRowEnd = pRange->aEnd.Row();

            if(nRowEnd == MAXROW)
            {
                bool bShrunk = false;
                mpDoc->ShrinkToUsedDataArea(bShrunk, nTab, nColStart, nRowStart,
                        nColEnd, nRowEnd, false);
            }
            for(SCCOL nCol = nColStart; nCol <= nColEnd; ++nCol)
            {
                for(SCROW nRow = nRowStart; nRow <= nRowEnd; ++nRow)
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
                        if (mpDoc->GetFormulaCell(aAddr)->IsValue())
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
        if (!mpDoc->GetFormulaCell(rAddr)->IsValue())
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

void ScColorScaleFormat::UpdateReference( sc::RefUpdateContext& rCxt )
{
    for(iterator itr = begin(); itr != end(); ++itr)
        itr->UpdateReference(rCxt);
}

void ScColorScaleFormat::UpdateInsertTab( sc::RefUpdateInsertTabContext& rCxt )
{
    for (iterator it = begin(); it != end(); ++it)
        it->UpdateInsertTab(rCxt);
}

void ScColorScaleFormat::UpdateDeleteTab( sc::RefUpdateDeleteTabContext& rCxt )
{
    for (iterator it = begin(); it != end(); ++it)
        it->UpdateDeleteTab(rCxt);
}

void ScColorScaleFormat::UpdateMoveTab( sc::RefUpdateMoveTabContext& rCxt )
{
    for (iterator it = begin(); it != end(); ++it)
        it->UpdateMoveTab(rCxt);
}

bool ScColorScaleFormat::NeedsRepaint() const
{
    for(const_iterator itr = begin(), itrEnd = end();
            itr != itrEnd; ++itr)
    {
        if(itr->NeedsRepaint())
            return true;
    }
    return false;
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

void ScDataBarFormat::UpdateReference( sc::RefUpdateContext& rCxt )
{
    mpFormatData->mpUpperLimit->UpdateReference(rCxt);
    mpFormatData->mpLowerLimit->UpdateReference(rCxt);
}

void ScDataBarFormat::UpdateInsertTab( sc::RefUpdateInsertTabContext& rCxt )
{
    mpFormatData->mpUpperLimit->UpdateInsertTab(rCxt);
    mpFormatData->mpLowerLimit->UpdateInsertTab(rCxt);
}

void ScDataBarFormat::UpdateDeleteTab( sc::RefUpdateDeleteTabContext& rCxt )
{
    mpFormatData->mpUpperLimit->UpdateDeleteTab(rCxt);
    mpFormatData->mpLowerLimit->UpdateDeleteTab(rCxt);
}

void ScDataBarFormat::UpdateMoveTab( sc::RefUpdateMoveTabContext& rCxt )
{
    mpFormatData->mpUpperLimit->UpdateMoveTab(rCxt);
    mpFormatData->mpLowerLimit->UpdateMoveTab(rCxt);
}

bool ScDataBarFormat::NeedsRepaint() const
{
    return mpFormatData->mpUpperLimit->NeedsRepaint() ||
        mpFormatData->mpLowerLimit->NeedsRepaint();
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
        if (!mpDoc->GetFormulaCell(rAddr)->IsValue())
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
        if (!mpDoc->GetFormulaCell(rAddr)->IsValue())
            return NULL;
    }

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
    while(itr != end() && nVal >= nValMax)
    {
        ++nIndex;
        nValMax = CalcValue(nMin, nMax, itr);
        ++itr;
    }
    if(nVal >= nValMax)
        ++nIndex;

    ScIconSetInfo* pInfo = new ScIconSetInfo;

    if(mpFormatData->mbReverse)
    {
        sal_Int32 nMaxIndex = mpFormatData->maEntries.size() - 1;
        pInfo->nIconIndex = nMaxIndex - nIndex;
    }
    else
        pInfo->nIconIndex = nIndex;
    pInfo->eIconSetType = mpFormatData->eIconSetType;
    pInfo->mbShowValue = mpFormatData->mbShowValue;
    return pInfo;
}

condformat::ScFormatEntryType ScIconSetFormat::GetType() const
{
    return condformat::ICONSET;
}

void ScIconSetFormat::DataChanged( const ScRange& )
{
}

void ScIconSetFormat::UpdateReference( sc::RefUpdateContext& rCxt )
{
    for(iterator itr = begin(); itr != end(); ++itr)
    {
        itr->UpdateReference(rCxt);
    }
}

void ScIconSetFormat::UpdateInsertTab( sc::RefUpdateInsertTabContext& rCxt )
{
    for(iterator itr = begin(); itr != end(); ++itr)
    {
        itr->UpdateInsertTab(rCxt);
    }
}

void ScIconSetFormat::UpdateDeleteTab( sc::RefUpdateDeleteTabContext& rCxt )
{
    for(iterator itr = begin(); itr != end(); ++itr)
    {
        itr->UpdateDeleteTab(rCxt);
    }
}

void ScIconSetFormat::UpdateMoveTab( sc::RefUpdateMoveTabContext& rCxt )
{
    for(iterator itr = begin(); itr != end(); ++itr)
    {
        itr->UpdateMoveTab(rCxt);
    }
}

bool ScIconSetFormat::NeedsRepaint() const
{
    for(const_iterator itr = begin(); itr != end(); ++itr)
    {
        if(itr->NeedsRepaint())
            return true;
    }

    return false;
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
        { "ColorSmilies", IconSet_ColorSmilies, 3 },
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

const sal_Int32 a3TrafficLights1[] = {
    BMP_ICON_SET_CIRCLES1_RED, BMP_ICON_SET_CIRCLES1_YELLOW, BMP_ICON_SET_CIRCLES1_GREEN
};

const sal_Int32 a3TrafficLights2[] = {
    BMP_ICON_SET_TRAFFICLIGHTS_RED, BMP_ICON_SET_TRAFFICLIGHTS_YELLOW, BMP_ICON_SET_TRAFFICLIGHTS_GREEN
};

const sal_Int32 a3Arrows[] = {
    BMP_ICON_SET_COLORARROWS_DOWN, BMP_ICON_SET_COLORARROWS_SAME, BMP_ICON_SET_COLORARROWS_UP
};

const sal_Int32 a3ArrowsGray[] = {
    BMP_ICON_SET_GRAYARROWS_DOWN, BMP_ICON_SET_GRAYARROWS_SAME, BMP_ICON_SET_GRAYARROWS_UP
};

const sal_Int32 a3Flags[] = {
    BMP_ICON_SET_FLAGS_RED, BMP_ICON_SET_FLAGS_YELLOW, BMP_ICON_SET_FLAGS_GREEN
};

const sal_Int32 a3ColorSmilies[] {
    BMP_ICON_SET_POSITIVE_GREEN_SMILIE, BMP_ICON_SET_NEUTRAL_YELLOW_SMILIE, BMP_ICON_SET_NEGATIVE_RED_SMILIE
};

const sal_Int32 a4Arrows[] = {
    BMP_ICON_SET_COLORARROWS_DOWN, BMP_ICON_SET_COLORARROWS_SLIGHTLY_DOWN, BMP_ICON_SET_COLORARROWS_SLIGHTLY_UP, BMP_ICON_SET_COLORARROWS_UP
};

const sal_Int32 a4ArrowsGray[] = {
    BMP_ICON_SET_GRAYARROWS_DOWN, BMP_ICON_SET_GRAYARROWS_SLIGHTLY_DOWN, BMP_ICON_SET_GRAYARROWS_SLIGHTLY_UP, BMP_ICON_SET_GRAYARROWS_UP
};

const sal_Int32 a5Arrows[] = {
    BMP_ICON_SET_COLORARROWS_DOWN, BMP_ICON_SET_COLORARROWS_SLIGHTLY_DOWN,
    BMP_ICON_SET_COLORARROWS_SAME, BMP_ICON_SET_COLORARROWS_SLIGHTLY_UP, BMP_ICON_SET_COLORARROWS_UP
};

const sal_Int32 a5ArrowsGray[] = {
    BMP_ICON_SET_GRAYARROWS_DOWN, BMP_ICON_SET_GRAYARROWS_SLIGHTLY_DOWN,
    BMP_ICON_SET_GRAYARROWS_SAME, BMP_ICON_SET_GRAYARROWS_SLIGHTLY_UP, BMP_ICON_SET_GRAYARROWS_UP
};

const sal_Int32 a4TrafficLights[] = {
    BMP_ICON_SET_CIRCLES1_GRAY, BMP_ICON_SET_CIRCLES1_RED,
    BMP_ICON_SET_CIRCLES1_YELLOW, BMP_ICON_SET_CIRCLES1_GREEN
};

const sal_Int32 a5Quarters[] = {
    BMP_ICON_SET_PIES_EMPTY, BMP_ICON_SET_PIES_ONE_QUARTER, BMP_ICON_SET_PIES_HALF,
    BMP_ICON_SET_PIES_THREE_QUARTER, BMP_ICON_SET_PIES_FULL,
};

const sal_Int32 a3Symbols1[] = {
    BMP_ICON_SET_SYMBOLS1_CROSS, BMP_ICON_SET_SYMBOLS1_EXCLAMATION_MARK, BMP_ICON_SET_SYMBOLS1_CHECK
};

const sal_Int32 a3Signs[] = {
    BMP_ICON_SET_SHAPES_DIAMOND, BMP_ICON_SET_SHAPES_TRIANGLE, BMP_ICON_SET_SHAPES_CIRCLE
};

const sal_Int32 a4RedToBlack[] = {
    BMP_ICON_SET_CIRCLES2_DARK_GRAY, BMP_ICON_SET_CIRCLES2_LIGHT_GRAY,
    BMP_ICON_SET_CIRCLES2_LIGHT_RED, BMP_ICON_SET_CIRCLES2_DARK_RED
};

const sal_Int32 a4Ratings[] = {
    BMP_ICON_SET_BARS_ONE_QUARTER, BMP_ICON_SET_BARS_HALF,
    BMP_ICON_SET_BARS_THREE_QUARTER, BMP_ICON_SET_BARS_FULL
};

const sal_Int32 a5Ratings[] = {
    BMP_ICON_SET_BARS_EMPTY, BMP_ICON_SET_BARS_ONE_QUARTER, BMP_ICON_SET_BARS_HALF,
    BMP_ICON_SET_BARS_THREE_QUARTER, BMP_ICON_SET_BARS_FULL
};

struct ScIconSetBitmapMap {
    ScIconSetType eType;
    const sal_Int32* nBitmaps;
};

static const ScIconSetBitmapMap aBitmapMap[] = {
    { IconSet_3Arrows, a3Arrows },
    { IconSet_3ArrowsGray, a3ArrowsGray },
    { IconSet_3Flags, a3Flags },
    { IconSet_3Signs, a3Signs },
    { IconSet_3Symbols, a3Symbols1 },
    { IconSet_3Symbols2, a3Symbols1 },
    { IconSet_3TrafficLights1, a3TrafficLights1 },
    { IconSet_3TrafficLights2, a3TrafficLights2 },
    { IconSet_ColorSmilies, a3ColorSmilies },
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
            break;
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
