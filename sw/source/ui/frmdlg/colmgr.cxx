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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#include "hintids.hxx"
#include <editeng/lrspitem.hxx>

#include "frmmgr.hxx"
#include "frmfmt.hxx"
#include "colmgr.hxx"


// PRIVATE METHODES ------------------------------------------------------
/*------------------------------------------------------------------------
 Description:   set column width to current width
------------------------------------------------------------------------*/
void FitToActualSize(SwFmtCol& rCol, sal_uInt16 nWidth)
{
    const sal_uInt16 nCount = rCol.GetColumns().Count();
    for(sal_uInt16 i = 0; i < nCount; ++i)
    {
        const sal_uInt16 nTmp = rCol.CalcColWidth(i, nWidth);
        rCol.GetColumns()[i]->SetWishWidth(nTmp);
    }
    rCol.SetWishWidth(nWidth);
}

// PUBLIC METHODES -------------------------------------------------------
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
        nRet = rCols.GetObject(nPos)->GetRight() + rCols.GetObject(nPos + 1)->GetLeft();
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
        rCols.GetObject(nPos)->SetRight(nGutterWidth2);
        rCols.GetObject(nPos + 1)->SetLeft(nGutterWidth2);
    }
}

/*------------------------------------------------------------------------
 Description:   height seperation line
------------------------------------------------------------------------*/
short SwColMgr::GetLineHeightPercent() const
{
    return (short)aFmtCol.GetLineHeight();
}

void SwColMgr::SetLineHeightPercent(short nPercent)
{
    OSL_ENSURE(nPercent <= 100, "line height may only be 100 \%");
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
    aFmtCol.GetColumns()[nIdx]->SetWishWidth(nWd);

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
