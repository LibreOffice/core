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

#include <swtblfmt.hxx>

SwTableFmt::SwTableFmt( SwAttrPool& rPool, const sal_Char* pFmtNm,
                    SwFrmFmt *pDrvdFrm )
    : SwFrmFmt( rPool, pFmtNm, pDrvdFrm, RES_FRMFMT, aTableSetRange )
    , m_aRepeatHeading( 0 )
    , m_bRowSplit( sal_True )
    {
        SetBreak( SvxFmtBreakItem( SVX_BREAK_NONE, RES_BREAK ) );
        SetKeepWithNextPara( SvxFmtKeepItem( sal_False, RES_KEEP ) );
        SetLayoutSplit( sal_True );
        SetCollapsingBorders( sal_True );
        SetShadow( SvxShadowItem( RES_SHADOW ) );

        pFstLineFmt = NULL;
        pLstLineFmt = NULL;
        pOddLineFmt = NULL;
        pEvnLineFmt = NULL;

        pFstColFmt = NULL;
        pLstColFmt = NULL;
        pOddColFmt = NULL;
        pEvnColFmt = NULL;
    }

SwTableFmt::SwTableFmt( SwAttrPool& rPool, const String &rFmtNm,
                    SwFrmFmt *pDrvdFrm )
    : SwFrmFmt( rPool, rFmtNm, pDrvdFrm, RES_FRMFMT, aTableSetRange )
    , m_aRepeatHeading( 0 )
    , m_bRowSplit( sal_True )
    {
        SetBreak( SvxFmtBreakItem( SVX_BREAK_NONE, RES_BREAK ) );
        SetKeepWithNextPara( SvxFmtKeepItem( sal_False, RES_KEEP ) );
        SetLayoutSplit( sal_True );
        SetCollapsingBorders( sal_True );
        SetShadow( SvxShadowItem( RES_SHADOW ) );

        pFstLineFmt = NULL;
        pLstLineFmt = NULL;
        pOddLineFmt = NULL;
        pEvnLineFmt = NULL;

        pFstColFmt = NULL;
        pLstColFmt = NULL;
        pOddColFmt = NULL;
        pEvnColFmt = NULL;
    }

SwTableFmt& SwTableFmt::operator=( const SwTableFmt& rNew )
    {
        if (&rNew == this)
            return *this;

        m_aRepeatHeading = rNew.m_aRepeatHeading;
        m_bRowSplit = rNew.m_bRowSplit;

        pFstLineFmt = rNew.pFstLineFmt;
        pLstLineFmt = rNew.pLstLineFmt;
        pOddLineFmt = rNew.pOddLineFmt;
        pEvnLineFmt = rNew.pEvnLineFmt;

        pFstColFmt = rNew.pFstColFmt;
        pLstColFmt = rNew.pLstColFmt;
        pOddColFmt = rNew.pOddColFmt;
        pEvnColFmt = rNew.pEvnColFmt;

        return *this;
    }

void SwTableFmt::SetBreak( const SvxFmtBreakItem& rNew )
{
    SetFmtAttr( rNew );
}

void SwTableFmt::SetPageDesc( const SwFmtPageDesc& rNew )
{
    SetFmtAttr( rNew );
}

void SwTableFmt::SetKeepWithNextPara( const SvxFmtKeepItem& rNew )
{
    SetFmtAttr( rNew );
}

void SwTableFmt::SetLayoutSplit( const sal_Bool& rNew )
{
    SetFmtAttr( SwFmtLayoutSplit( rNew ) );
}

void SwTableFmt::SetCollapsingBorders( const sal_Bool& rNew )
{
    SetFmtAttr( SfxBoolItem( RES_COLLAPSING_BORDERS, rNew ) );
}

void SwTableFmt::SetShadow( const SvxShadowItem& rNew )
{
    SetFmtAttr( rNew );
}

const SvxFmtKeepItem& SwTableFmt::GetKeepWithNextPara() const
{
    return SwFmt::GetKeep();
}

sal_Bool SwTableFmt::GetLayoutSplit() const
{
    return SwFmt::GetLayoutSplit().GetValue();
}

sal_Bool SwTableFmt::GetCollapsingBorders() const
{
    return (static_cast<const SfxBoolItem&>( GetFmtAttr( RES_COLLAPSING_BORDERS ) )).GetValue();
}

SwTableLineFmt::SwTableLineFmt( SwAttrPool& rPool, const sal_Char* pFmtNm,
                    SwFrmFmt *pDrvdFrm )
    : SwFrmFmt( rPool, pFmtNm, pDrvdFrm, RES_FRMFMT, aTableLineSetRange )
    {
        pFstBoxFmt = NULL;
        pLstBoxFmt = NULL;
        pOddBoxFmt = NULL;
        pEvnBoxFmt = NULL;
    }

SwTableLineFmt::SwTableLineFmt( SwAttrPool& rPool, const String &rFmtNm,
                    SwFrmFmt *pDrvdFrm )
    : SwFrmFmt( rPool, rFmtNm, pDrvdFrm, RES_FRMFMT, aTableLineSetRange )
    {
        pFstBoxFmt = NULL;
        pLstBoxFmt = NULL;
        pOddBoxFmt = NULL;
        pEvnBoxFmt = NULL;
    }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
