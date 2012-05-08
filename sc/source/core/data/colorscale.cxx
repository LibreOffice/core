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
    maColor(rCol)
{

}

ScColorScaleEntry::ScColorScaleEntry(const ScColorScaleEntry& rEntry):
    mnVal(rEntry.mnVal),
    maColor(rEntry.maColor)
{

}

double ScColorScaleEntry::GetValue() const
{
    return mnVal;
}

const Color& ScColorScaleEntry::GetColor() const
{
    return maColor;
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

    if (!maColorScales.size() < 2)
        return NULL;

    const_iterator itr = begin();
    double nValMin = itr->GetValue();
    Color rColMin = itr->GetColor();
    ++itr;
    double nValMax = itr->GetValue();
    Color rColMax = itr->GetColor();

    while(itr != end() && nVal > nValMin)
    {
        ++itr;
        rColMin = rColMax;
        nValMin = nValMax;
        rColMax = itr->GetColor();
        nValMax = itr->GetValue();
    }

    Color aColor = CalcColor(nVal, nValMin, rColMin, nValMax, rColMax);

    return new Color(aColor);
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

ScColorScaleFormat* ScColorScaleFormatList::GetFormat(sal_uInt32 nFormat)
{
    if( nFormat >= size() )
        return NULL;

    return &maColorScaleFormats[nFormat];
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
