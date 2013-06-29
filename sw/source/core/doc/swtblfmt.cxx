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
    {
        SetBreak( SvxFmtBreakItem( SVX_BREAK_NONE, RES_BREAK ) );
        SetKeepWithNextPara( SvxFmtKeepItem( sal_False, RES_KEEP ) );
        SetLayoutSplit( sal_True );
        SetCollapsingBorders( sal_True );
        SetRowSplit( sal_True );
        SetRepeatHeading( 0 );
        SetShadow( SvxShadowItem( RES_SHADOW ) );

        SwDoc* pDoc = GetDoc();

        m_pFstLineFmt = pDoc->MakeTableLineFmt();
        m_pLstLineFmt = pDoc->MakeTableLineFmt();
        m_pOddLineFmt = pDoc->MakeTableLineFmt();
        m_pEvnLineFmt = pDoc->MakeTableLineFmt();

        m_pFstColFmt = pDoc->MakeTableLineFmt();
        m_pLstColFmt = pDoc->MakeTableLineFmt();
        m_pOddColFmt = pDoc->MakeTableLineFmt();
        m_pEvnColFmt = pDoc->MakeTableLineFmt();
    }

SwTableFmt::SwTableFmt( SwAttrPool& rPool, const String &rFmtNm,
                    SwFrmFmt *pDrvdFrm )
    : SwFrmFmt( rPool, rFmtNm, pDrvdFrm, RES_FRMFMT, aTableSetRange )
    {
        SetBreak( SvxFmtBreakItem( SVX_BREAK_NONE, RES_BREAK ) );
        SetKeepWithNextPara( SvxFmtKeepItem( sal_False, RES_KEEP ) );
        SetLayoutSplit( sal_True );
        SetCollapsingBorders( sal_True );
        SetRowSplit( sal_True );
        SetRepeatHeading( 0 );
        SetShadow( SvxShadowItem( RES_SHADOW ) );

        SwDoc* pDoc = GetDoc();

        m_pFstLineFmt = pDoc->MakeTableLineFmt();
        m_pLstLineFmt = pDoc->MakeTableLineFmt();
        m_pOddLineFmt = pDoc->MakeTableLineFmt();
        m_pEvnLineFmt = pDoc->MakeTableLineFmt();

        m_pFstColFmt = pDoc->MakeTableLineFmt();
        m_pLstColFmt = pDoc->MakeTableLineFmt();
        m_pOddColFmt = pDoc->MakeTableLineFmt();
        m_pEvnColFmt = pDoc->MakeTableLineFmt();
    }

SwTableFmt::SwTableFmt( SwTableFmt& rNew )
    : SwFrmFmt( (SwFrmFmt)rNew )
    {
        *m_pFstLineFmt = SwTableLineFmt( *rNew.GetFirstLineFmt() );
        *m_pLstLineFmt = SwTableLineFmt( *rNew.GetLastLineFmt() );
        *m_pOddLineFmt = SwTableLineFmt( *rNew.GetOddLineFmt() );
        *m_pEvnLineFmt = SwTableLineFmt( *rNew.GetEvenLineFmt() );

        *m_pFstColFmt = SwTableLineFmt( *rNew.GetFirstColFmt() );
        *m_pLstColFmt = SwTableLineFmt( *rNew.GetLastColFmt() );
        *m_pOddColFmt = SwTableLineFmt( *rNew.GetOddColFmt() );
        *m_pEvnColFmt = SwTableLineFmt( *rNew.GetEvenColFmt() );
    }

SwTableFmt& SwTableFmt::operator=( const SwTableFmt& rNew )
    {
        if (&rNew == this)
            return *this;

        m_pFstLineFmt = rNew.m_pFstLineFmt;
        m_pLstLineFmt = rNew.m_pLstLineFmt;
        m_pOddLineFmt = rNew.m_pOddLineFmt;
        m_pEvnLineFmt = rNew.m_pEvnLineFmt;

        m_pFstColFmt = rNew.m_pFstColFmt;
        m_pLstColFmt = rNew.m_pLstColFmt;
        m_pOddColFmt = rNew.m_pOddColFmt;
        m_pEvnColFmt = rNew.m_pEvnColFmt;

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

void SwTableFmt::SetRowSplit( const sal_Bool& rNew )
{
    SetFmtAttr( SwFmtRowSplit( rNew ) );
}

void SwTableFmt::SetRepeatHeading( const sal_uInt16& rNew )
{
    SetFmtAttr( SfxUInt16Item( FN_PARAM_TABLE_HEADLINE, rNew ) );
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

sal_Bool SwTableFmt::GetRowSplit() const
{
    return SwFmt::GetRowSplit().GetValue();
}

sal_uInt16 SwTableFmt::GetRepeatHeading() const
{
    return (static_cast<const SfxUInt16Item&>( GetFmtAttr( FN_PARAM_TABLE_HEADLINE ) )).GetValue();
}

void SwTableFmt::CopyTableFormatInfo( SwTableFmt* pTableFormat )
{
    *m_pFstLineFmt = SwTableLineFmt ( *pTableFormat->GetFirstLineFmt() );
    *m_pLstLineFmt = SwTableLineFmt ( *pTableFormat->GetLastLineFmt() );
    *m_pOddLineFmt = SwTableLineFmt ( *pTableFormat->GetOddLineFmt() );
    *m_pEvnLineFmt = SwTableLineFmt ( *pTableFormat->GetEvenLineFmt() );

    *m_pFstColFmt = SwTableLineFmt ( *pTableFormat->GetFirstColFmt() );
    *m_pLstColFmt = SwTableLineFmt ( *pTableFormat->GetLastColFmt() );
    *m_pOddColFmt = SwTableLineFmt ( *pTableFormat->GetOddColFmt() );
    *m_pEvnColFmt = SwTableLineFmt ( *pTableFormat->GetEvenColFmt() );
}

SwTableLineFmt::SwTableLineFmt( SwAttrPool& rPool, const sal_Char* pFmtNm,
                    SwFrmFmt *pDrvdFrm )
    : SwFrmFmt( rPool, pFmtNm, pDrvdFrm, RES_FRMFMT, aTableLineSetRange )
    {
        SwDoc* pDoc = GetDoc();

        m_pFstBoxFmt = pDoc->MakeTableBoxFmt();
        m_pLstBoxFmt = pDoc->MakeTableBoxFmt();
        m_pOddBoxFmt = pDoc->MakeTableBoxFmt();
        m_pEvnBoxFmt = pDoc->MakeTableBoxFmt();
    }

SwTableLineFmt::SwTableLineFmt( SwAttrPool& rPool, const String &rFmtNm,
                    SwFrmFmt *pDrvdFrm )
    : SwFrmFmt( rPool, rFmtNm, pDrvdFrm, RES_FRMFMT, aTableLineSetRange )
    {
        SwDoc* pDoc = GetDoc();

        m_pFstBoxFmt = pDoc->MakeTableBoxFmt();
        m_pLstBoxFmt = pDoc->MakeTableBoxFmt();
        m_pOddBoxFmt = pDoc->MakeTableBoxFmt();
        m_pEvnBoxFmt = pDoc->MakeTableBoxFmt();
    }

SwTableLineFmt::SwTableLineFmt( SwTableLineFmt& rNew )
    : SwFrmFmt( (SwFrmFmt)rNew )
    {
        *m_pFstBoxFmt = SwTableBoxFmt( *rNew.GetFirstBoxFmt() );
        *m_pLstBoxFmt = SwTableBoxFmt( *rNew.GetLastBoxFmt() );
        *m_pOddBoxFmt = SwTableBoxFmt( *rNew.GetOddBoxFmt() );
        *m_pEvnBoxFmt = SwTableBoxFmt( *rNew.GetEvenBoxFmt() );
    }

SwTableLineFmt& SwTableLineFmt::operator=( const SwTableLineFmt& rNew )
    {
        if (&rNew == this)
                return *this;

        m_pFstBoxFmt = rNew.m_pFstBoxFmt;
        m_pLstBoxFmt = rNew.m_pLstBoxFmt;
        m_pOddBoxFmt = rNew.m_pOddBoxFmt;
        m_pEvnBoxFmt = rNew.m_pEvnBoxFmt;

        return *this;
    }

SwTableBoxFmt::SwTableBoxFmt( SwAttrPool& rPool, const sal_Char* pFmtNm,
                    SwFrmFmt *pDrvdFrm )
    : SwFrmFmt( rPool, pFmtNm, pDrvdFrm, RES_FRMFMT, aTableBoxSetRange )
    {
        SetFont( *(SvxFontItem*)GetDfltAttr( RES_CHRATR_FONT ) );
        SetHeight( SvxFontHeightItem( 240, 100, RES_CHRATR_FONTSIZE) );
        SetWeight( SvxWeightItem( WEIGHT_NORMAL, RES_CHRATR_WEIGHT ) );
        SetPosture( SvxPostureItem( ITALIC_NONE, RES_CHRATR_POSTURE ) );

        SetCJKFont( *(SvxFontItem*)GetDfltAttr( RES_CHRATR_CJK_FONT ) );
        SetCJKHeight( SvxFontHeightItem( 240, 100, RES_CHRATR_CJK_FONTSIZE) );
        SetCJKWeight( SvxWeightItem( WEIGHT_NORMAL, RES_CHRATR_CJK_WEIGHT ) );
        SetCJKPosture( SvxPostureItem( ITALIC_NONE, RES_CHRATR_CJK_POSTURE ) );

        SetCTLFont( *(SvxFontItem*)GetDfltAttr( RES_CHRATR_CTL_FONT ) );
        SetCTLHeight( SvxFontHeightItem( 240, 100, RES_CHRATR_CTL_FONTSIZE) );
        SetCTLWeight( SvxWeightItem( WEIGHT_NORMAL, RES_CHRATR_CTL_WEIGHT ) );
        SetCTLPosture( SvxPostureItem( ITALIC_NONE, RES_CHRATR_CTL_POSTURE ) );

        SetUnderline( SvxUnderlineItem( UNDERLINE_NONE, RES_CHRATR_UNDERLINE ) );
        SetOverline( SvxOverlineItem( UNDERLINE_NONE, RES_CHRATR_OVERLINE ) );
        SetCrossedOut( SvxCrossedOutItem( STRIKEOUT_NONE, RES_CHRATR_CROSSEDOUT ) );
        SetContour( SvxContourItem( sal_False, RES_CHRATR_CONTOUR ) );
        SetShadowed( SvxShadowedItem( sal_False, RES_CHRATR_SHADOWED ) );
        SetColor( SvxColorItem( RES_CHRATR_COLOR ) );
        SvxBoxItem rNew = SvxBoxItem( RES_BOX );
        rNew.SetDistance( 55 );
        SetBox( rNew );
        SetBackground( SvxBrushItem( RES_BACKGROUND ) );
        SetAdjust( SvxAdjustItem( SVX_ADJUST_LEFT, RES_PARATR_ADJUST ) );
        SetTextOrientation( SvxFrameDirectionItem( FRMDIR_ENVIRONMENT, RES_FRAMEDIR ) );
        SetVerticalAlignment( SwFmtVertOrient( 0, com::sun::star::text::VertOrientation::NONE, com::sun::star::text::RelOrientation::FRAME ) );

        eSysLanguage = eNumFmtLanguage = ::GetAppLanguage();
    }

SwTableBoxFmt::SwTableBoxFmt( SwAttrPool& rPool, const String &rFmtNm,
                    SwFrmFmt *pDrvdFrm )
    : SwFrmFmt( rPool, rFmtNm, pDrvdFrm, RES_FRMFMT, aTableBoxSetRange )
    {
        SetFont( *(SvxFontItem*)GetDfltAttr( RES_CHRATR_FONT ) );
        SetHeight( SvxFontHeightItem( 240, 100, RES_CHRATR_FONTSIZE) );
        SetWeight( SvxWeightItem( WEIGHT_NORMAL, RES_CHRATR_WEIGHT ) );
        SetPosture( SvxPostureItem( ITALIC_NONE, RES_CHRATR_POSTURE ) );

        SetCJKFont( *(SvxFontItem*)GetDfltAttr( RES_CHRATR_CJK_FONT ) );
        SetCJKHeight( SvxFontHeightItem( 240, 100, RES_CHRATR_CJK_FONTSIZE) );
        SetCJKWeight( SvxWeightItem( WEIGHT_NORMAL, RES_CHRATR_CJK_WEIGHT ) );
        SetCJKPosture( SvxPostureItem( ITALIC_NONE, RES_CHRATR_CJK_POSTURE ) );

        SetCTLFont( *(SvxFontItem*)GetDfltAttr( RES_CHRATR_CTL_FONT ) );
        SetCTLHeight( SvxFontHeightItem( 240, 100, RES_CHRATR_CTL_FONTSIZE) );
        SetCTLWeight( SvxWeightItem( WEIGHT_NORMAL, RES_CHRATR_CTL_WEIGHT ) );
        SetCTLPosture( SvxPostureItem( ITALIC_NONE, RES_CHRATR_CTL_POSTURE ) );

        SetUnderline( SvxUnderlineItem( UNDERLINE_NONE, RES_CHRATR_UNDERLINE ) );
        SetOverline( SvxOverlineItem( UNDERLINE_NONE, RES_CHRATR_OVERLINE ) );
        SetCrossedOut( SvxCrossedOutItem( STRIKEOUT_NONE, RES_CHRATR_CROSSEDOUT ) );
        SetContour( SvxContourItem( sal_False, RES_CHRATR_CONTOUR ) );
        SetShadowed( SvxShadowedItem( sal_False, RES_CHRATR_SHADOWED ) );
        SetColor( SvxColorItem( RES_CHRATR_COLOR ) );
        SvxBoxItem rNew = SvxBoxItem( RES_BOX );
        rNew.SetDistance( 55 );
        SetBox( rNew );
        SetBackground( SvxBrushItem( RES_BACKGROUND ) );
        SetAdjust( SvxAdjustItem( SVX_ADJUST_LEFT, RES_PARATR_ADJUST ) );
        SetTextOrientation( SvxFrameDirectionItem( FRMDIR_ENVIRONMENT, RES_FRAMEDIR ) );
        SetVerticalAlignment( SwFmtVertOrient( 0, com::sun::star::text::VertOrientation::NONE, com::sun::star::text::RelOrientation::FRAME ) );

        eSysLanguage = eNumFmtLanguage = ::GetAppLanguage();
    }

void SwTableBoxFmt::SetFont( const SvxFontItem& rNew )
{
    SetFmtAttr( rNew );
}

void SwTableBoxFmt::SetHeight( const SvxFontHeightItem& rNew )
{
    SetFmtAttr( rNew );
}

void SwTableBoxFmt::SetWeight( const SvxWeightItem& rNew )
{
    SetFmtAttr( rNew );
}

void SwTableBoxFmt::SetPosture( const SvxPostureItem& rNew )
{
    SetFmtAttr( rNew );
}

void SwTableBoxFmt::SetCJKFont( const SvxFontItem& rNew )
{
    SetFmtAttr( rNew );
}

void SwTableBoxFmt::SetCJKHeight( const SvxFontHeightItem& rNew )
{
    SetFmtAttr( rNew );
}

void SwTableBoxFmt::SetCJKWeight( const SvxWeightItem& rNew )
{
    SetFmtAttr( rNew );
}

void SwTableBoxFmt::SetCJKPosture( const SvxPostureItem& rNew )
{
    SetFmtAttr( rNew );
}

void SwTableBoxFmt::SetCTLFont( const SvxFontItem& rNew )
{
    SetFmtAttr( rNew );
}

void SwTableBoxFmt::SetCTLHeight( const SvxFontHeightItem& rNew )
{
    SetFmtAttr( rNew );
}

void SwTableBoxFmt::SetCTLWeight( const SvxWeightItem& rNew )
{
    SetFmtAttr( rNew );
}

void SwTableBoxFmt::SetCTLPosture( const SvxPostureItem& rNew )
{
    SetFmtAttr( rNew );
}

void SwTableBoxFmt::SetUnderline( const SvxUnderlineItem& rNew )
{
    SetFmtAttr( rNew );
}

void SwTableBoxFmt::SetOverline( const SvxOverlineItem& rNew )
{
    SetFmtAttr( rNew );
}

void SwTableBoxFmt::SetCrossedOut( const SvxCrossedOutItem& rNew )
{
    SetFmtAttr( rNew );
}

void SwTableBoxFmt::SetContour( const SvxContourItem& rNew )
{
    SetFmtAttr( rNew );
}

void SwTableBoxFmt::SetShadowed( const SvxShadowedItem& rNew )
{
    SetFmtAttr( rNew );
}

void SwTableBoxFmt::SetColor( const SvxColorItem& rNew )
{
    SetFmtAttr( rNew );
}

void SwTableBoxFmt::SetBox( const SvxBoxItem& rNew )
{
    SetFmtAttr( rNew );
}

void SwTableBoxFmt::SetBackground( const SvxBrushItem& rNew )
{
    SetFmtAttr( rNew );
}

void SwTableBoxFmt::SetAdjust( const SvxAdjustItem& rNew )
{
    SvxAdjustItem rTmp = SwFmt::GetAdjust();
    rTmp.SetAdjust( rNew.GetAdjust() );
    rTmp.SetOneWord( rNew.GetOneWord() );
    rTmp.SetLastBlock( rNew.GetLastBlock() );
    SetFmtAttr( rTmp );
}

void SwTableBoxFmt::SetTextOrientation( const SvxFrameDirectionItem& rNew )
{
    SetFmtAttr( rNew );
}

void SwTableBoxFmt::SetVerticalAlignment( const SwFmtVertOrient& rNew )
{
    SetFmtAttr( rNew );
}

void SwTableBoxFmt::SetValueFormat( const String& rFmt, LanguageType eLng, LanguageType eSys )
{
    sNumFmtString = rFmt;
    eNumFmtLanguage = eLng;
    eSysLanguage = eSys;
}

const SvxFrameDirectionItem& SwTableBoxFmt::GetTextOrientation() const
{
    return SwFmt::GetFrmDir();
}

const SwFmtVertOrient& SwTableBoxFmt::GetVerticalAlignment() const
{
    return SwFmt::GetVertOrient();
}

void SwTableBoxFmt::GetValueFormat( String& rFmt, LanguageType& rLng, LanguageType& rSys ) const
{
    rFmt = sNumFmtString;
    rLng = eNumFmtLanguage;
    rSys = eSysLanguage;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
