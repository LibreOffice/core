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
/*------------------------------------------------------------------------
 Description:   set column width to current width
------------------------------------------------------------------------*/
void FitToActualSize(SwFmtCol& rCol, sal_uInt16 nWidth)
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
/*------------------------------------------------------------------------
 Description:   set column quantity and Gutterwidth
------------------------------------------------------------------------*/
void SwColMgr::SetCount(sal_uInt16 nCount, sal_uInt16  nGutterWidth)
{
    aFmtCol.Init(nCount, nGutterWidth, nWidth);
    aFmtCol.SetWishWidth(nWidth);
    aFmtCol.SetGutterWidth(nGutterWidth, nWidth);
}

sal_uInt16 SwColMgr::GetGutterWidth( sal_uInt16 nPos ) const
{
    sal_uInt16 nRet;
    if(nPos == USHRT_MAX )
        nRet = GetCount() > 1 ? aFmtCol.GetGutterWidth() : DEF_GUTTER_WIDTH;
    else
    {
        OSL_ENSURE(nPos < GetCount() - 1, "Spalte ueberindiziert" );
        const SwColumns& rCols = aFmtCol.GetColumns();
        nRet = rCols[nPos].GetRight() + rCols[nPos + 1].GetLeft();
    }
    return nRet;
}

void SwColMgr::SetGutterWidth(sal_uInt16 nGutterWidth, sal_uInt16 nPos )
{
    if(nPos == USHRT_MAX)
        aFmtCol.SetGutterWidth(nGutterWidth, nWidth);
    else
    {
        OSL_ENSURE(nPos < GetCount() - 1, "Spalte ueberindiziert" );
        SwColumns& rCols = aFmtCol.GetColumns();
        sal_uInt16 nGutterWidth2 = nGutterWidth / 2;
        rCols[nPos].SetRight(nGutterWidth2);
        rCols[nPos + 1].SetLeft(nGutterWidth2);
    }
}

/*------------------------------------------------------------------------
 Description:   height separation line
------------------------------------------------------------------------*/
short SwColMgr::GetLineHeightPercent() const
{
    return (short)aFmtCol.GetLineHeight();
}

void SwColMgr::SetLineHeightPercent(short nPercent)
{
    OSL_ENSURE(nPercent <= 100, "line height may be at most 100 %");
    aFmtCol.SetLineHeight((sal_uInt8)nPercent);
}

/*------------------------------------------------------------------------
 Description:   column width
------------------------------------------------------------------------*/
sal_uInt16 SwColMgr::GetColWidth(sal_uInt16 nIdx) const
{
    OSL_ENSURE(nIdx < GetCount(), "Spaltenarray ueberindiziert.");
    return aFmtCol.CalcPrtColWidth(nIdx, nWidth);
}

void SwColMgr::SetColWidth(sal_uInt16 nIdx, sal_uInt16 nWd)
{
    OSL_ENSURE(nIdx < GetCount(), "Spaltenarray ueberindiziert.");
    aFmtCol.GetColumns()[nIdx].SetWishWidth(nWd);

}

/*--------------------------------------------------------------------
    Description:    newly set size
 --------------------------------------------------------------------*/
void SwColMgr::SetActualWidth(sal_uInt16 nW)
{
    nWidth = nW;
    ::FitToActualSize(aFmtCol, nW);
}

/*--------------------------------------------------------------------
    Description: ctor
 --------------------------------------------------------------------*/
SwColMgr::SwColMgr(const SfxItemSet& rSet, sal_uInt16 nActWidth) :
    aFmtCol((const SwFmtCol&)rSet.Get(RES_COL)),
    nWidth(nActWidth)
{
    if(nWidth == USHRT_MAX)
    {
        nWidth = (sal_uInt16)((const SwFmtFrmSize&)rSet.Get(RES_FRM_SIZE)).GetWidth();
        if (nWidth < MINLAY)
            nWidth = USHRT_MAX;
        const SvxLRSpaceItem &rLR = (const SvxLRSpaceItem&)rSet.Get(RES_LR_SPACE);
        nWidth = nWidth - (sal_uInt16)rLR.GetLeft();
        nWidth = nWidth - (sal_uInt16)rLR.GetRight();
    }
    ::FitToActualSize(aFmtCol, nWidth);
}

SwColMgr::~SwColMgr()
{
}

void SwColMgr::SetLineWidthAndColor(::editeng::SvxBorderStyle eStyle, sal_uLong nLWidth, const Color& rCol)
{
    aFmtCol.SetLineStyle(eStyle);
    aFmtCol.SetLineWidth(nLWidth);
    aFmtCol.SetLineColor(rCol);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
