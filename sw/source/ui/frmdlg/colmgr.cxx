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
 Beschreibung:  Spaltenbreite auf aktuelle Breite einstellen
------------------------------------------------------------------------*/
void FitToActualSize(SwFmtCol& rCol, USHORT nWidth)
{
    const USHORT nCount = rCol.GetColumns().Count();
    for(USHORT i = 0; i < nCount; ++i)
    {
        const USHORT nTmp = rCol.CalcColWidth(i, nWidth);
        rCol.GetColumns()[i]->SetWishWidth(nTmp);
    }
    rCol.SetWishWidth(nWidth);
}

// PUBLIC METHODES -------------------------------------------------------
/*------------------------------------------------------------------------
 Beschreibung:  Setzen Spaltenanzahl und Gutterwidth
------------------------------------------------------------------------*/
void SwColMgr::SetCount(USHORT nCount, USHORT  nGutterWidth)
{
    aFmtCol.Init(nCount, nGutterWidth, nWidth);
    aFmtCol.SetWishWidth(nWidth);
    aFmtCol.SetGutterWidth(nGutterWidth, nWidth);
}

USHORT SwColMgr::GetGutterWidth( USHORT nPos ) const
{
    USHORT nRet;
    if(nPos == USHRT_MAX )
        nRet = GetCount() > 1 ? aFmtCol.GetGutterWidth() : DEF_GUTTER_WIDTH;
    else
    {
        DBG_ASSERT(nPos < GetCount() - 1, "Spalte ueberindiziert" );
        const SwColumns& rCols = aFmtCol.GetColumns();
        nRet = rCols.GetObject(nPos)->GetRight() + rCols.GetObject(nPos + 1)->GetLeft();
    }
    return nRet;
}

void SwColMgr::SetGutterWidth(USHORT nGutterWidth, USHORT nPos )
{
    if(nPos == USHRT_MAX)
        aFmtCol.SetGutterWidth(nGutterWidth, nWidth);
    else
    {
        DBG_ASSERT(nPos < GetCount() - 1, "Spalte ueberindiziert" );
        SwColumns& rCols = aFmtCol.GetColumns();
        USHORT nGutterWidth2 = nGutterWidth / 2;
        rCols.GetObject(nPos)->SetRight(nGutterWidth2);
        rCols.GetObject(nPos + 1)->SetLeft(nGutterWidth2);
    }
}

/*------------------------------------------------------------------------
 Beschreibung:  Hoehe Trennlinie
------------------------------------------------------------------------*/
short SwColMgr::GetLineHeightPercent() const
{
    return (short)aFmtCol.GetLineHeight();
}

void SwColMgr::SetLineHeightPercent(short nPercent)
{
    ASSERT(nPercent <= 100, LineHeight darf nur bis 100 % gross  sein);
    aFmtCol.SetLineHeight((BYTE)nPercent);
}

/*------------------------------------------------------------------------
 Beschreibung:  Spaltenbreite
------------------------------------------------------------------------*/
USHORT SwColMgr::GetColWidth(USHORT nIdx) const
{
    ASSERT(nIdx < GetCount(), Spaltenarray ueberindiziert.);
    return aFmtCol.CalcPrtColWidth(nIdx, nWidth);
}

void SwColMgr::SetColWidth(USHORT nIdx, USHORT nWd)
{
    ASSERT(nIdx < GetCount(), Spaltenarray ueberindiziert.);
    aFmtCol.GetColumns()[nIdx]->SetWishWidth(nWd);

}

/*--------------------------------------------------------------------
    Beschreibung:   Groesse neu setzen
 --------------------------------------------------------------------*/
void SwColMgr::SetActualWidth(USHORT nW)
{
    nWidth = nW;
    ::FitToActualSize(aFmtCol, nW);
}

/*--------------------------------------------------------------------
    Beschreibung: ctor
 --------------------------------------------------------------------*/
SwColMgr::SwColMgr(const SfxItemSet& rSet, USHORT nActWidth) :
    aFmtCol((const SwFmtCol&)rSet.Get(RES_COL)),
    nWidth(nActWidth)
{
    if(nWidth == USHRT_MAX)
    {
        nWidth = (USHORT)((const SwFmtFrmSize&)rSet.Get(RES_FRM_SIZE)).GetWidth();
        if (nWidth < MINLAY)
            nWidth = USHRT_MAX;
        const SvxLRSpaceItem &rLR = (const SvxLRSpaceItem&)rSet.Get(RES_LR_SPACE);
        nWidth = nWidth - (USHORT)rLR.GetLeft();
        nWidth = nWidth - (USHORT)rLR.GetRight();
    }
    ::FitToActualSize(aFmtCol, nWidth);
}

SwColMgr::~SwColMgr()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
