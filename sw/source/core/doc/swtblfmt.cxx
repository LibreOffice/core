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

#include <editsh.hxx>
#include <swtable.hxx>
#include <swtblfmt.hxx>

SwTableFormat::SwTableFormat( SwAttrPool& rPool, const sal_Char* pFormatNm,
                    SwFrameFormat *pDrvdFrm )
    : SwFrameFormat( rPool, pFormatNm, pDrvdFrm, RES_FRMFMT, aTableSetRange )
    {
        SetBreak( SvxFormatBreakItem( SVX_BREAK_NONE, RES_BREAK ) );
        SetKeepWithNextPara( SvxFormatKeepItem( sal_False, RES_KEEP ) );
        SetLayoutSplit( sal_True );
        SetCollapsingBorders( sal_True );
        SetRowSplit( sal_True );
        SetRepeatHeading( 0 );
        SetShadow( SvxShadowItem( RES_SHADOW ) );

        SwDoc* pDoc = GetDoc();

        m_pFstLineFormat.reset( pDoc->MakeTableLineFormat() );
        m_pLstLineFormat.reset( pDoc->MakeTableLineFormat() );
        m_pOddLineFormat.reset( pDoc->MakeTableLineFormat() );
        m_pEvnLineFormat.reset( pDoc->MakeTableLineFormat() );

        m_pFstColFormat.reset( pDoc->MakeTableLineFormat() );
        m_pLstColFormat.reset( pDoc->MakeTableLineFormat() );
        m_pOddColFormat.reset( pDoc->MakeTableLineFormat() );
        m_pEvnColFormat.reset( pDoc->MakeTableLineFormat() );
    }

SwTableFormat::SwTableFormat( SwAttrPool& rPool, const OUString &rFormatNm,
                    SwFrameFormat *pDrvdFrm )
    : SwFrameFormat( rPool, rFormatNm, pDrvdFrm, RES_FRMFMT, aTableSetRange )
    {
        SetBreak( SvxFormatBreakItem( SVX_BREAK_NONE, RES_BREAK ) );
        SetKeepWithNextPara( SvxFormatKeepItem( sal_False, RES_KEEP ) );
        SetLayoutSplit( sal_True );
        SetCollapsingBorders( sal_True );
        SetRowSplit( sal_True );
        SetRepeatHeading( 0 );
        SetShadow( SvxShadowItem( RES_SHADOW ) );

        SwDoc* pDoc = GetDoc();

        m_pFstLineFormat.reset( pDoc->MakeTableLineFormat() );
        m_pLstLineFormat.reset( pDoc->MakeTableLineFormat() );
        m_pOddLineFormat.reset( pDoc->MakeTableLineFormat() );
        m_pEvnLineFormat.reset( pDoc->MakeTableLineFormat() );

        m_pFstColFormat.reset( pDoc->MakeTableLineFormat() );
        m_pLstColFormat.reset( pDoc->MakeTableLineFormat() );
        m_pOddColFormat.reset( pDoc->MakeTableLineFormat() );
        m_pEvnColFormat.reset( pDoc->MakeTableLineFormat() );
    }

SwTableFormat::SwTableFormat( const SwTableFormat& rNew )
    : SwFrameFormat( (SwFrameFormat)rNew )
    {
        m_pFstLineFormat.reset( new SwTableLineFormat( *rNew.m_pFstLineFormat.get() ) );
        m_pLstLineFormat.reset( new SwTableLineFormat( *rNew.m_pLstLineFormat.get() ) );
        m_pOddLineFormat.reset( new SwTableLineFormat( *rNew.m_pOddLineFormat.get() ) );
        m_pEvnLineFormat.reset( new SwTableLineFormat( *rNew.m_pEvnLineFormat.get() ) );

        m_pFstColFormat.reset( new SwTableLineFormat( *rNew.m_pFstColFormat.get() ) );
        m_pLstColFormat.reset( new SwTableLineFormat( *rNew.m_pLstColFormat.get() ) );
        m_pOddColFormat.reset( new SwTableLineFormat( *rNew.m_pOddColFormat.get() ) );
        m_pEvnColFormat.reset( new SwTableLineFormat( *rNew.m_pEvnColFormat.get() ) );
    }

SwTableFormat& SwTableFormat::operator=( const SwTableFormat& rNew )
    {
        if (&rNew == this)
            return *this;

        SwFrameFormat::operator=( rNew );

        m_pFstLineFormat.reset( new SwTableLineFormat( *rNew.m_pFstLineFormat.get() ) );
        m_pLstLineFormat.reset( new SwTableLineFormat( *rNew.m_pLstLineFormat.get() ) );
        m_pOddLineFormat.reset( new SwTableLineFormat( *rNew.m_pOddLineFormat.get() ) );
        m_pEvnLineFormat.reset( new SwTableLineFormat( *rNew.m_pEvnLineFormat.get() ) );

        m_pFstColFormat.reset( new SwTableLineFormat( *rNew.m_pFstColFormat.get() ) );
        m_pLstColFormat.reset( new SwTableLineFormat( *rNew.m_pLstColFormat.get() ) );
        m_pOddColFormat.reset( new SwTableLineFormat( *rNew.m_pOddColFormat.get() ) );
        m_pEvnColFormat.reset( new SwTableLineFormat( *rNew.m_pEvnColFormat.get() ) );

        return *this;
    }

void SwTableFormat::SetBoxFormat( const SwTableBoxFormat& rNew, sal_uInt8 nPos )
{
    OSL_ENSURE( nPos < 16, "wrong area" );

    sal_uInt8 nLine = nPos / 4;
    sal_uInt8 nBox = nPos % 4;

    SwTableLineFormat* pLine = 0;

    switch( nLine )
    {
        case 0:
            pLine = m_pFstLineFormat.get(); break;
        case 1:
            pLine = m_pOddLineFormat.get(); break;
        case 2:
            pLine = m_pEvnLineFormat.get(); break;
        case 3:
            pLine = m_pLstLineFormat.get(); break;
        // TODO Extend for columns
    }

    switch( nBox )
    {
        case 0:
            pLine->SetFirstBoxFormat( new SwTableBoxFormat( rNew ) ); break;
        case 1:
            pLine->SetOddBoxFormat( new SwTableBoxFormat( rNew ) ); break;
        case 2:
            pLine->SetEvenBoxFormat( new SwTableBoxFormat( rNew ) ); break;
        case 3:
            pLine->SetLastBoxFormat( new SwTableBoxFormat( rNew ) ); break;
    }
}

SwTableBoxFormat* SwTableFormat::GetBoxFormat( sal_uInt8 nPos ) const
{
    OSL_ENSURE( nPos < 16, "wrong area" );

    sal_uInt8 nLine = nPos / 4;
    sal_uInt8 nBox = nPos % 4;

    SwTableLineFormat* pLine = 0;
    SwTableBoxFormat* pRet;

    switch( nLine )
    {
        case 0:
            pLine = m_pFstLineFormat.get(); break;
        case 1:
            pLine = m_pOddLineFormat.get(); break;
        case 2:
            pLine = m_pEvnLineFormat.get(); break;
        case 3:
            pLine = m_pLstLineFormat.get(); break;
        // TODO Extend for columns
    }

    switch( nBox )
    {
        case 0:
            pRet = pLine->GetFirstBoxFormat(); break;
        case 1:
            pRet = pLine->GetOddBoxFormat(); break;
        case 2:
            pRet = pLine->GetEvenBoxFormat(); break;
        case 3:
            pRet = pLine->GetLastBoxFormat(); break;
    }

    return pRet;
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

void SwTableFormat::SetRowSplit( const sal_Bool& rNew )
{
    SetFormatAttr( SwFormatRowSplit( rNew ) );
}

void SwTableFormat::SetRepeatHeading( const sal_uInt16& rNew )
{
    SetFormatAttr( SfxUInt16Item( FN_PARAM_TABLE_HEADLINE, rNew ) );
}

void SwTableFormat::SetShadow( const SvxShadowItem& rNew )
{
    SetFormatAttr( rNew );
}

const SvxFormatKeepItem& SwTableFormat::GetKeepWithNextPara() const
{
    return SwFormat::GetKeep();
}

sal_Bool SwTableFormat::GetLayoutSplit() const
{
    return SwFormat::GetLayoutSplit().GetValue();
}

sal_Bool SwTableFormat::GetCollapsingBorders() const
{
    return (static_cast<const SfxBoolItem&>( GetFormatAttr( RES_COLLAPSING_BORDERS ) )).GetValue();
}

sal_Bool SwTableFormat::GetRowSplit() const
{
    return SwFormat::GetRowSplit().GetValue();
}

sal_uInt16 SwTableFormat::GetRepeatHeading() const
{
    const SfxPoolItem* pItem;

    if( SfxItemState::SET == GetItemState( FN_PARAM_TABLE_HEADLINE, sal_False, &pItem ) )
        return ((const SfxUInt16Item*)pItem)->GetValue();

    return 0;
}

void SwTableFormat::RestoreTableProperties( SwTableFormat* pSrcFormat, SwTable &table )
{
    SwTableFormat *pHardFormat = table.GetFrameFormat();
    if( !pHardFormat )
        return;

    SwDoc *pDoc = pHardFormat->GetDoc();
    if( !pDoc )
        return;

    SwTableFormat *pTableStyle = (SwTableFormat*)pHardFormat->GetRegisteredIn();
    sal_Bool bRowSplit = sal_True;
    sal_uInt16 nRepeatHeading = 0;

    if( pSrcFormat )
    {
        pHardFormat->RegisterToFormat( *pSrcFormat );
        bRowSplit = pSrcFormat->GetRowSplit();
        nRepeatHeading = pSrcFormat->GetRepeatHeading();
    }
    else
        pTableStyle->Remove( pHardFormat );

    SwEditShell *pShell = pDoc->GetEditShell();
    pDoc->SetRowSplit( *pShell->getShellCrsr( false ), SwFormatRowSplit( bRowSplit ) );

    table.SetRowsToRepeat( nRepeatHeading );
}

SwTableFormat* SwTableFormat::StoreTableProperties( const SwTable &table )
{
    SwTableFormat *pHardFormat = table.GetFrameFormat();
    if( !pHardFormat )
        return NULL;

    return (SwTableFormat*)pHardFormat->GetRegisteredIn();
}

SwTableLineFormat::SwTableLineFormat( SwAttrPool& rPool, const sal_Char* pFormatNm,
                    SwFrameFormat *pDrvdFrm )
    : SwFrameFormat( rPool, pFormatNm, pDrvdFrm, RES_FRMFMT, aTableLineSetRange )
    {
        SwDoc* pDoc = GetDoc();

        m_pFstBoxFormat.reset( pDoc->MakeTableBoxFormat() );
        m_pLstBoxFormat.reset( pDoc->MakeTableBoxFormat() );
        m_pOddBoxFormat.reset( pDoc->MakeTableBoxFormat() );
        m_pEvnBoxFormat.reset( pDoc->MakeTableBoxFormat() );
    }

SwTableLineFormat::SwTableLineFormat( SwAttrPool& rPool, const OUString &rFormatNm,
                    SwFrameFormat *pDrvdFrm )
    : SwFrameFormat( rPool, rFormatNm, pDrvdFrm, RES_FRMFMT, aTableLineSetRange )
    {
        SwDoc* pDoc = GetDoc();

        m_pFstBoxFormat.reset( pDoc->MakeTableBoxFormat() );
        m_pLstBoxFormat.reset( pDoc->MakeTableBoxFormat() );
        m_pOddBoxFormat.reset( pDoc->MakeTableBoxFormat() );
        m_pEvnBoxFormat.reset( pDoc->MakeTableBoxFormat() );
    }

SwTableLineFormat::SwTableLineFormat( const SwTableLineFormat& rNew )
    : SwFrameFormat( (SwFrameFormat)rNew )
    {
        m_pFstBoxFormat.reset( new SwTableBoxFormat( *rNew.m_pFstBoxFormat.get() ) );
        m_pLstBoxFormat.reset( new SwTableBoxFormat( *rNew.m_pLstBoxFormat.get() ) );
        m_pOddBoxFormat.reset( new SwTableBoxFormat( *rNew.m_pOddBoxFormat.get() ) );
        m_pEvnBoxFormat.reset( new SwTableBoxFormat( *rNew.m_pEvnBoxFormat.get() ) );
    }

SwTableLineFormat& SwTableLineFormat::operator=( const SwTableLineFormat& rNew )
    {
        if (&rNew == this)
                return *this;

        SwFrameFormat::operator=( rNew );

        m_pFstBoxFormat.reset( new SwTableBoxFormat( *rNew.m_pFstBoxFormat.get() ) );
        m_pLstBoxFormat.reset( new SwTableBoxFormat( *rNew.m_pLstBoxFormat.get() ) );
        m_pOddBoxFormat.reset( new SwTableBoxFormat( *rNew.m_pOddBoxFormat.get() ) );
        m_pEvnBoxFormat.reset( new SwTableBoxFormat( *rNew.m_pEvnBoxFormat.get() ) );

        return *this;
    }

SwTableBoxFormat::SwTableBoxFormat( SwAttrPool& rPool, const sal_Char* pFormatNm,
                    SwFrameFormat *pDrvdFrm )
    : SwFrameFormat( rPool, pFormatNm, pDrvdFrm, RES_FRMFMT, aTableBoxSetRange )
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
        SetVerticalAlignment( SwFormatVertOrient( 0, com::sun::star::text::VertOrientation::NONE, com::sun::star::text::RelOrientation::FRAME ) );

        eSysLanguage = eNumFormatLanguage = ::GetAppLanguage();
    }

SwTableBoxFormat::SwTableBoxFormat( SwAttrPool& rPool, const OUString &rFormatNm,
                    SwFrameFormat *pDrvdFrm )
    : SwFrameFormat( rPool, rFormatNm, pDrvdFrm, RES_FRMFMT, aTableBoxSetRange )
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
        SetVerticalAlignment( SwFormatVertOrient( 0, com::sun::star::text::VertOrientation::NONE, com::sun::star::text::RelOrientation::FRAME ) );

        eSysLanguage = eNumFormatLanguage = ::GetAppLanguage();
    }

void SwTableBoxFormat::SetFont( const SvxFontItem& rNew )
{
    SetFormatAttr( rNew );
}

void SwTableBoxFormat::SetHeight( const SvxFontHeightItem& rNew )
{
    SetFormatAttr( rNew );
}

void SwTableBoxFormat::SetWeight( const SvxWeightItem& rNew )
{
    SetFormatAttr( rNew );
}

void SwTableBoxFormat::SetPosture( const SvxPostureItem& rNew )
{
    SetFormatAttr( rNew );
}

void SwTableBoxFormat::SetCJKFont( const SvxFontItem& rNew )
{
    SetFormatAttr( rNew );
}

void SwTableBoxFormat::SetCJKHeight( const SvxFontHeightItem& rNew )
{
    SetFormatAttr( rNew );
}

void SwTableBoxFormat::SetCJKWeight( const SvxWeightItem& rNew )
{
    SetFormatAttr( rNew );
}

void SwTableBoxFormat::SetCJKPosture( const SvxPostureItem& rNew )
{
    SetFormatAttr( rNew );
}

void SwTableBoxFormat::SetCTLFont( const SvxFontItem& rNew )
{
    SetFormatAttr( rNew );
}

void SwTableBoxFormat::SetCTLHeight( const SvxFontHeightItem& rNew )
{
    SetFormatAttr( rNew );
}

void SwTableBoxFormat::SetCTLWeight( const SvxWeightItem& rNew )
{
    SetFormatAttr( rNew );
}

void SwTableBoxFormat::SetCTLPosture( const SvxPostureItem& rNew )
{
    SetFormatAttr( rNew );
}

void SwTableBoxFormat::SetUnderline( const SvxUnderlineItem& rNew )
{
    SetFormatAttr( rNew );
}

void SwTableBoxFormat::SetOverline( const SvxOverlineItem& rNew )
{
    SetFormatAttr( rNew );
}

void SwTableBoxFormat::SetCrossedOut( const SvxCrossedOutItem& rNew )
{
    SetFormatAttr( rNew );
}

void SwTableBoxFormat::SetContour( const SvxContourItem& rNew )
{
    SetFormatAttr( rNew );
}

void SwTableBoxFormat::SetShadowed( const SvxShadowedItem& rNew )
{
    SetFormatAttr( rNew );
}

void SwTableBoxFormat::SetColor( const SvxColorItem& rNew )
{
    SetFormatAttr( rNew );
}

void SwTableBoxFormat::SetBox( const SvxBoxItem& rNew )
{
    SetFormatAttr( rNew );
}

void SwTableBoxFormat::SetBackground( const SvxBrushItem& rNew )
{
    SetFormatAttr( rNew );
}

void SwTableBoxFormat::SetAdjust( const SvxAdjustItem& rNew )
{
    SvxAdjustItem rTmp = SwFormat::GetAdjust();
    rTmp.SetAdjust( rNew.GetAdjust() );
    rTmp.SetOneWord( rNew.GetOneWord() );
    rTmp.SetLastBlock( rNew.GetLastBlock() );
    SetFormatAttr( rTmp );
}

void SwTableBoxFormat::SetTextOrientation( const SvxFrameDirectionItem& rNew )
{
    SetFormatAttr( rNew );
}

void SwTableBoxFormat::SetVerticalAlignment( const SwFormatVertOrient& rNew )
{
    SetFormatAttr( rNew );
}

void SwTableBoxFormat::SetValueFormat( const OUString& rFormat, LanguageType eLng, LanguageType eSys )
{
    sNumFormatString = rFormat;
    eNumFormatLanguage = eLng;
    eSysLanguage = eSys;
}

const SvxFontHeightItem& SwTableBoxFormat::GetHeight() const
{
    return SwFormat::GetSize();
}

const SvxFontHeightItem& SwTableBoxFormat::GetCJKHeight() const
{
    return SwFormat::GetCJKSize();
}

const SvxFontHeightItem& SwTableBoxFormat::GetCTLHeight() const
{
    return SwFormat::GetCTLSize();
}

const SvxFrameDirectionItem& SwTableBoxFormat::GetTextOrientation() const
{
    return SwFormat::GetFrmDir();
}

const SwFormatVertOrient& SwTableBoxFormat::GetVerticalAlignment() const
{
    return SwFormat::GetVertOrient();
}

void SwTableBoxFormat::GetValueFormat( OUString& rFormat, LanguageType& rLng, LanguageType& rSys ) const
{
    rFormat = sNumFormatString;
    rLng = eNumFormatLanguage;
    rSys = eSysLanguage;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
