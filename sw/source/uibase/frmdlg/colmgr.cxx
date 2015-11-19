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

#include "hintids.hxx"
#include <editeng/lrspitem.hxx>

#include "frmmgr.hxx"
#include "frmfmt.hxx"
#include "colmgr.hxx"

// private methods

// set column width to current width
void FitToActualSize(SwFormatCol& rCol, sal_uInt16 nWidth)
{
    const sal_uInt16 nCount = rCol.GetColumns().size();
    for(sal_uInt16 i = 0; i < nCount; ++i)
    {
        const sal_uInt16 nTmp = rCol.CalcColWidth(i, nWidth);
        rCol.GetColumns()[i].SetWishWidth(nTmp);
    }
    rCol.SetWishWidth(nWidth);
}

// public methods

// set column quantity and Gutterwidth
void SwColMgr::SetCount(sal_uInt16 nCount, sal_uInt16  nGutterWidth)
{
    aFormatCol.Init(nCount, nGutterWidth, nWidth);
    aFormatCol.SetWishWidth(nWidth);
    aFormatCol.SetGutterWidth(nGutterWidth, nWidth);
}

sal_uInt16 SwColMgr::GetGutterWidth( sal_uInt16 nPos ) const
{
    sal_uInt16 nRet;
    if(nPos == USHRT_MAX )
        nRet = GetCount() > 1 ? aFormatCol.GetGutterWidth() : DEF_GUTTER_WIDTH;
    else
    {
        OSL_ENSURE(nPos < GetCount() - 1, "Spalte ueberindiziert" );
        const SwColumns& rCols = aFormatCol.GetColumns();
        nRet = rCols[nPos].GetRight() + rCols[nPos + 1].GetLeft();
    }
    return nRet;
}

void SwColMgr::SetGutterWidth(sal_uInt16 nGutterWidth, sal_uInt16 nPos )
{
    if(nPos == USHRT_MAX)
        aFormatCol.SetGutterWidth(nGutterWidth, nWidth);
    else
    {
        OSL_ENSURE(nPos < GetCount() - 1, "Spalte ueberindiziert" );
        SwColumns& rCols = aFormatCol.GetColumns();
        sal_uInt16 nGutterWidth2 = nGutterWidth / 2;
        rCols[nPos].SetRight(nGutterWidth2);
        rCols[nPos + 1].SetLeft(nGutterWidth2);
    }
}

// height separation line
short SwColMgr::GetLineHeightPercent() const
{
    return (short)aFormatCol.GetLineHeight();
}
void SwColMgr::SetLineHeightPercent(short nPercent)
{
    OSL_ENSURE(nPercent <= 100, "line height may be at most 100%");
    aFormatCol.SetLineHeight((sal_uInt8)nPercent);
}

// column width
sal_uInt16 SwColMgr::GetColWidth(sal_uInt16 nIdx) const
{
    OSL_ENSURE(nIdx < GetCount(), "Spaltenarray ueberindiziert.");
    return aFormatCol.CalcPrtColWidth(nIdx, nWidth);
}

void SwColMgr::SetColWidth(sal_uInt16 nIdx, sal_uInt16 nWd)
{
    OSL_ENSURE(nIdx < GetCount(), "Spaltenarray ueberindiziert.");
    aFormatCol.GetColumns()[nIdx].SetWishWidth(nWd);

}

// newly set size
void SwColMgr::SetActualWidth(sal_uInt16 nW)
{
    nWidth = nW;
    ::FitToActualSize(aFormatCol, nW);
}

// ctor
SwColMgr::SwColMgr(const SfxItemSet& rSet, sal_uInt16 nActWidth) :
    aFormatCol(static_cast<const SwFormatCol&>(rSet.Get(RES_COL))),
    nWidth(nActWidth)
{
    if(nWidth == USHRT_MAX)
    {
        nWidth = (sal_uInt16)static_cast<const SwFormatFrameSize&>(rSet.Get(RES_FRM_SIZE)).GetWidth();
        if (nWidth < MINLAY)
            nWidth = USHRT_MAX;
        const SvxLRSpaceItem &rLR = static_cast<const SvxLRSpaceItem&>(rSet.Get(RES_LR_SPACE));
        nWidth = nWidth - (sal_uInt16)rLR.GetLeft();
        nWidth = nWidth - (sal_uInt16)rLR.GetRight();
    }
    ::FitToActualSize(aFormatCol, nWidth);
}

SwColMgr::~SwColMgr()
{
}

void SwColMgr::SetLineWidthAndColor(::editeng::SvxBorderStyle eStyle, sal_uLong nLWidth, const Color& rCol)
{
    aFormatCol.SetLineStyle(eStyle);
    aFormatCol.SetLineWidth(nLWidth);
    aFormatCol.SetLineColor(rCol);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
