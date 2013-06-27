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

SwTableFormat::SwTableFormat( SwAttrPool& rPool, const sal_Char* pFormatNm,
                    SwFrameFormat *pDrvdFrm )
    : SwFrameFormat( rPool, pFormatNm, pDrvdFrm, RES_FRMFMT, aTableSetRange )
    , m_aRepeatHeading( 0 )
    , m_bRowSplit( sal_True )
    {
        SetBreak( SvxFormatBreakItem( SVX_BREAK_NONE, RES_BREAK ) );
        SetKeepWithNextPara( SvxFormatKeepItem( sal_False, RES_KEEP ) );
        SetLayoutSplit( sal_True );
        SetCollapsingBorders( sal_True );
        SetShadow( SvxShadowItem( RES_SHADOW ) );

        pFstLineFormat = NULL;
        pLstLineFormat = NULL;
        pOddLineFormat = NULL;
        pEvnLineFormat = NULL;

        pFstColFormat = NULL;
        pLstColFormat = NULL;
        pOddColFormat = NULL;
        pEvnColFormat = NULL;
    }

SwTableFormat::SwTableFormat( SwAttrPool& rPool, const OUString &rFormatNm,
                    SwFrameFormat *pDrvdFrm )
    : SwFrameFormat( rPool, rFormatNm, pDrvdFrm, RES_FRMFMT, aTableSetRange )
    , m_aRepeatHeading( 0 )
    , m_bRowSplit( sal_True )
    {
        SetBreak( SvxFormatBreakItem( SVX_BREAK_NONE, RES_BREAK ) );
        SetKeepWithNextPara( SvxFormatKeepItem( sal_False, RES_KEEP ) );
        SetLayoutSplit( sal_True );
        SetCollapsingBorders( sal_True );
        SetShadow( SvxShadowItem( RES_SHADOW ) );

        pFstLineFormat = NULL;
        pLstLineFormat = NULL;
        pOddLineFormat = NULL;
        pEvnLineFormat = NULL;

        pFstColFormat = NULL;
        pLstColFormat = NULL;
        pOddColFormat = NULL;
        pEvnColFormat = NULL;
    }

SwTableFormat& SwTableFormat::operator=( const SwTableFormat& rNew )
    {
        if (&rNew == this)
            return *this;

        m_aRepeatHeading = rNew.m_aRepeatHeading;
        m_bRowSplit = rNew.m_bRowSplit;

        pFstLineFormat = rNew.pFstLineFormat;
        pLstLineFormat = rNew.pLstLineFormat;
        pOddLineFormat = rNew.pOddLineFormat;
        pEvnLineFormat = rNew.pEvnLineFormat;

        pFstColFormat = rNew.pFstColFormat;
        pLstColFormat = rNew.pLstColFormat;
        pOddColFormat = rNew.pOddColFormat;
        pEvnColFormat = rNew.pEvnColFormat;

        return *this;
    }

void SwTableFormat::SetBreak( const SvxFormatBreakItem& rNew )
{
    SetFormatAttr( rNew );
}

void SwTableFormat::SetPageDesc( const SwFormatPageDesc& rNew )
{
    SetFormatAttr( rNew );
}

void SwTableFormat::SetKeepWithNextPara( const SvxFormatKeepItem& rNew )
{
    SetFormatAttr( rNew );
}

void SwTableFormat::SetLayoutSplit( const sal_Bool& rNew )
{
    SetFormatAttr( SwFormatLayoutSplit( rNew ) );
}

void SwTableFormat::SetCollapsingBorders( const sal_Bool& rNew )
{
    SetFormatAttr( SfxBoolItem( RES_COLLAPSING_BORDERS, rNew ) );
}

void SwTableFormat::SetShadow( const SvxShadowItem& rNew )
{
    SetFormatAttr( rNew );
}

const SvxFormatBreakItem& SwTableFormat::GetBreak() const
{
    return static_cast<const SvxFormatBreakItem&>( GetFormatAttr( RES_BREAK ) );
}

const SwFormatPageDesc& SwTableFormat::GetPageDesc() const
{
    return static_cast<const SwFormatPageDesc&>( GetFormatAttr( RES_PAGEDESC ) );
}

const SvxFormatKeepItem& SwTableFormat::GetKeepWithNextPara() const
{
    return static_cast<const SvxFormatKeepItem&>( GetFormatAttr( RES_KEEP ) );
}

sal_Bool SwTableFormat::GetLayoutSplit() const
{
    return (static_cast<const SwFormatLayoutSplit&>( GetFormatAttr( RES_LAYOUT_SPLIT ) )).GetValue();
}

sal_Bool SwTableFormat::GetCollapsingBorders() const
{
    return (static_cast<const SfxBoolItem&>( GetFormatAttr( RES_COLLAPSING_BORDERS ) )).GetValue();
}

const SvxShadowItem& SwTableFormat::GetShadow() const
{
    return static_cast<const SvxShadowItem&>( GetFormatAttr( RES_SHADOW ) );
}

SwTableLineFormat::SwTableLineFormat( SwAttrPool& rPool, const sal_Char* pFormatNm,
                    SwFrameFormat *pDrvdFrm )
    : SwFrameFormat( rPool, pFormatNm, pDrvdFrm, RES_FRMFMT, aTableLineSetRange )
    {
        pFstBoxFormat = NULL;
        pLstBoxFormat = NULL;
        pOddBoxFormat = NULL;
        pEvnBoxFormat = NULL;
    }

SwTableLineFormat::SwTableLineFormat( SwAttrPool& rPool, const OUString &rFormatNm,
                    SwFrameFormat *pDrvdFrm )
    : SwFrameFormat( rPool, rFormatNm, pDrvdFrm, RES_FRMFMT, aTableLineSetRange )
    {
        pFstBoxFormat = NULL;
        pLstBoxFormat = NULL;
        pOddBoxFormat = NULL;
        pEvnBoxFormat = NULL;
    }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
