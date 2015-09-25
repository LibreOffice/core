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
#include <tabfrm.hxx>

SwTableFormat::SwTableFormat( SwAttrPool& rPool, const sal_Char* pFormatNm,
                    SwFrameFormat *pDrvdFrm )
    : SwFrameFormat( rPool, pFormatNm, pDrvdFrm, RES_FRMFMT, aTableSetRange )
    {
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

void SwTableFormat::SetLayoutSplit( const bool& rNew )
{
    SetFormatAttr( SwFormatLayoutSplit( rNew ) );
}

void SwTableFormat::SetCollapsingBorders( const bool& rNew )
{
    SetFormatAttr( SfxBoolItem( RES_COLLAPSING_BORDERS, rNew ) );
}

void SwTableFormat::SetRowSplit( const bool& rNew )
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

bool SwTableFormat::GetLayoutSplit() const
{
    return SwFormat::GetLayoutSplit().GetValue();
}

bool SwTableFormat::GetCollapsingBorders() const
{
    return (static_cast<const SfxBoolItem&>( GetFormatAttr( RES_COLLAPSING_BORDERS ) )).GetValue();
}

bool SwTableFormat::GetRowSplit() const
{
    return SwFormat::GetRowSplit().GetValue();
}

sal_uInt16 SwTableFormat::GetRepeatHeading() const
{
    const SfxPoolItem* pItem;

    if( SfxItemState::SET == GetItemState( FN_PARAM_TABLE_HEADLINE, false, &pItem ) )
        return ((const SfxUInt16Item*)pItem)->GetValue();

    return 0;
}

bool SwTableFormat::IsFont() const
{
    for( sal_uInt8 n = 0; n < 16; ++n )
    {
        if( GetBoxFormat( n )->IsFont() )
            return true;
    }
    return false;
}

bool SwTableFormat::IsJustify() const
{
    for( sal_uInt8 n = 0; n < 16; ++n )
    {
        if( GetBoxFormat( n )->IsJustify() )
            return true;
    }
    return false;
}

bool SwTableFormat::IsFrame() const
{
    for( sal_uInt8 n = 0; n < 16; ++n )
    {
        if( GetBoxFormat( n )->IsFrame() )
            return true;
    }
    return false;
}

bool SwTableFormat::IsBackground() const
{
    for( sal_uInt8 n = 0; n < 16; ++n )
    {
        if( GetBoxFormat( n )->IsBackground() )
            return true;
    }
    return false;
}

bool SwTableFormat::IsValueFormat() const
{
    for( sal_uInt8 n = 0; n < 16; ++n )
    {
        if( GetBoxFormat( n )->IsValueFormat() )
            return true;
    }
    return false;
}

void SwTableFormat::RestoreTableProperties( SwTableFormat* pSrcFormat, SwTable &rTable )
{
    SwTableFormat *pHardFormat = rTable.GetFrameFormat();
    if( !pHardFormat )
        return;

    SwDoc *pDoc = pHardFormat->GetDoc();
    if( !pDoc )
        return;

    SwTableFormat *pTableStyle = (SwTableFormat*)pHardFormat->GetRegisteredIn();
    bool bRowSplit = true;
    sal_uInt16 nRepeatHeading = 0;

    if( pSrcFormat )
    {
        pHardFormat->RegisterToFormat( *pSrcFormat );
        bRowSplit = pSrcFormat->GetRowSplit();
        nRepeatHeading = pSrcFormat->GetRepeatHeading();
    }
    else if( pTableStyle )
        pTableStyle->Remove( pHardFormat );

    AssignFormatParents( pSrcFormat, rTable );

    SwEditShell *pShell = pDoc->GetEditShell();
    pDoc->SetRowSplit( *pShell->getShellCrsr( false ), SwFormatRowSplit( bRowSplit ) );

    rTable.SetRowsToRepeat( nRepeatHeading );
}

SwTableFormat* SwTableFormat::StoreTableProperties( const SwTable &rTable )
{
    SwTableFormat *pHardFormat = rTable.GetFrameFormat();
    if( !pHardFormat )
        return NULL;

    return (SwTableFormat*)pHardFormat->GetRegisteredIn();
}

void SwTableFormat::AssignFormatParents( SwTableFormat* pSrcFormat, SwTable &rTable )
{
    SwTableFormat *pHardFormat = rTable.GetFrameFormat();
    if( !pHardFormat )
        return;

    if( pSrcFormat && pSrcFormat->IsDefault() )
        return;

    if( pSrcFormat )
        pHardFormat->GetAttrSet().SetParent( &pSrcFormat->GetAttrSet() );
    else
        pHardFormat->GetAttrSet().SetParent( NULL );

    AssignLineParents( pSrcFormat, rTable );

    SwIterator<SwTabFrm,SwFormat> aIter( *rTable.GetFrameFormat() );
    for( SwTabFrm* pLast = aIter.First(); pLast; pLast = aIter.Next() )
        if( pLast->GetTable() == &rTable )
        {
            pLast->InvalidateAll();
            pLast->SetCompletePaint();
        }

    ((SwModify*)pHardFormat)->CheckCaching( RES_BOX );
}

void SwTableFormat::AssignLineParents( SwTableFormat* pSrcFormat, SwTable &rTable )
{
    sal_uInt16 nLines = rTable.GetTabLines().size();
    for( sal_uInt16 n = 0; n < nLines; ++n )
    {
        SwTableLineFormat* pLineFormat = (SwTableLineFormat*)rTable.GetTabLines()[ n ]->ClaimFrameFormat();
        SwTableLineFormat* pFormat = 0;

        if( pSrcFormat )
        {
            if( !n )
                pFormat = pSrcFormat->GetFirstLineFormat();
            else if( n == nLines - 1 )
                pFormat = pSrcFormat->GetLastLineFormat();
            else if( n & 1 )
                pFormat = pSrcFormat->GetEvenLineFormat();
            else
                pFormat = pSrcFormat->GetOddLineFormat();
        }

        if( pFormat )
        {
            pLineFormat->RegisterToFormat( *pFormat );
            pLineFormat->GetAttrSet().SetParent( &pFormat->GetAttrSet() );
        }
        else if( pLineFormat->GetRegisteredIn() )
        {
            ((SwTableLineFormat*)pLineFormat->GetRegisteredIn())->Remove( pLineFormat );
            pLineFormat->GetAttrSet().SetParent( NULL );
        }

        AssignBoxParents( pFormat, *rTable.GetTabLines()[ n ] );

        ((SwModify*)pLineFormat)->CheckCaching( RES_BOX );
    }
}

void SwTableFormat::AssignBoxParents( SwTableLineFormat* pSrcLineFormat, SwTableLine &rLine )
{
    sal_uInt16 nBoxes = rLine.GetTabBoxes().size();
    for( sal_uInt16 n = 0; n < nBoxes; ++n )
    {
        SwTableBoxFormat* pBoxFormat = (SwTableBoxFormat*)rLine.GetTabBoxes()[ n ]->ClaimFrameFormat();
        SwTableBoxFormat* pFormat = 0;

        if( pSrcLineFormat )
        {
            if( !n )
                pFormat = pSrcLineFormat->GetFirstBoxFormat();
            else if( n == nBoxes - 1 )
                pFormat = pSrcLineFormat->GetLastBoxFormat();
            else if( n & 1 )
                pFormat = pSrcLineFormat->GetEvenBoxFormat();
            else
                pFormat = pSrcLineFormat->GetOddBoxFormat();
        }

        if( pFormat )
        {
            pBoxFormat->RegisterToFormat( *pFormat );
            pBoxFormat->GetAttrSet().SetParent( &pFormat->GetAttrSet() );
        }
        else if( pBoxFormat->GetRegisteredIn() )
        {
            ((SwTableBoxFormat*)pBoxFormat->GetRegisteredIn())->Remove( pBoxFormat );
            pBoxFormat->GetAttrSet().SetParent( NULL );
        }

        if( rLine.GetTabBoxes()[ n ]->GetTabLines().size() )
            AssignLineParents_Complex( pSrcLineFormat, pFormat, *rLine.GetTabBoxes()[ n ] );

        ((SwModify*)pBoxFormat)->CheckCaching( RES_BOX );
    }
}

void SwTableFormat::AssignLineParents_Complex( SwTableLineFormat* pSrcLineFormat, SwTableBoxFormat* pSrcBoxFormat, SwTableBox& rBox )
{
    sal_uInt16 nLines = rBox.GetTabLines().size();
    for( sal_uInt16 n = 0; n < nLines; ++n )
    {
        SwTableLineFormat* pLineFormat = (SwTableLineFormat*)rBox.GetTabLines()[ n ]->ClaimFrameFormat();

        if( pSrcLineFormat )
        {
            pLineFormat->RegisterToFormat( *pSrcLineFormat );
            pLineFormat->GetAttrSet().SetParent( &pSrcLineFormat->GetAttrSet() );
        }
        else
        {
            ((SwTableLineFormat*)pLineFormat->GetRegisteredIn())->Remove( pLineFormat );
            pLineFormat->GetAttrSet().SetParent( NULL );
        }

        AssignBoxParents_Complex( pSrcLineFormat, pSrcBoxFormat, *rBox.GetTabLines()[ n ] );

        ((SwModify*)pLineFormat)->CheckCaching( RES_BOX );
    }
}

void SwTableFormat::AssignBoxParents_Complex( SwTableLineFormat* pSrcLineFormat, SwTableBoxFormat* pSrcBoxFormat, SwTableLine& rLine )
{
    sal_uInt16 nBoxes = rLine.GetTabBoxes().size();
    for( sal_uInt16 n = 0; n < nBoxes; ++n )
    {
        SwTableBoxFormat* pBoxFormat = (SwTableBoxFormat*)rLine.GetTabBoxes()[ n ]->ClaimFrameFormat();

        if( pSrcBoxFormat )
        {
            pBoxFormat->RegisterToFormat( *pSrcBoxFormat );
            pBoxFormat->GetAttrSet().SetParent( &pSrcBoxFormat->GetAttrSet() );
        }
        else
        {
            ((SwTableBoxFormat*)pBoxFormat->GetRegisteredIn())->Remove( pBoxFormat );
            pBoxFormat->GetAttrSet().SetParent( NULL );
        }

        if( rLine.GetTabBoxes()[ n ]->GetTabLines().size() )
            AssignLineParents_Complex( pSrcLineFormat, pSrcBoxFormat, *rLine.GetTabBoxes()[ n ] );

        ((SwModify*)pBoxFormat)->CheckCaching( RES_BOX );
    }
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
    {}

SwTableBoxFormat::SwTableBoxFormat( SwAttrPool& rPool, const OUString &rFormatNm,
                    SwFrameFormat *pDrvdFrm )
    : SwFrameFormat( rPool, rFormatNm, pDrvdFrm, RES_FRMFMT, aTableBoxSetRange )
    {}

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

bool SwTableBoxFormat::IsFont() const
{
    return ( SfxItemState::SET == GetItemState( RES_CHRATR_FONT ) )
        || ( SfxItemState::SET == GetItemState( RES_CHRATR_FONTSIZE ) )
        || ( SfxItemState::SET == GetItemState( RES_CHRATR_WEIGHT ) )
        || ( SfxItemState::SET == GetItemState( RES_CHRATR_POSTURE ) )
        || ( SfxItemState::SET == GetItemState( RES_CHRATR_CJK_FONT ) )
        || ( SfxItemState::SET == GetItemState( RES_CHRATR_CJK_FONTSIZE ) )
        || ( SfxItemState::SET == GetItemState( RES_CHRATR_CJK_WEIGHT ) )
        || ( SfxItemState::SET == GetItemState( RES_CHRATR_CJK_POSTURE ) )
        || ( SfxItemState::SET == GetItemState( RES_CHRATR_CTL_FONT ) )
        || ( SfxItemState::SET == GetItemState( RES_CHRATR_CTL_FONTSIZE ) )
        || ( SfxItemState::SET == GetItemState( RES_CHRATR_CTL_WEIGHT ) )
        || ( SfxItemState::SET == GetItemState( RES_CHRATR_CTL_POSTURE ) )
        || ( SfxItemState::SET == GetItemState( RES_CHRATR_UNDERLINE ) )
        || ( SfxItemState::SET == GetItemState( RES_CHRATR_OVERLINE ) )
        || ( SfxItemState::SET == GetItemState( RES_CHRATR_CROSSEDOUT ) )
        || ( SfxItemState::SET == GetItemState( RES_CHRATR_CONTOUR ) )
        || ( SfxItemState::SET == GetItemState( RES_CHRATR_SHADOWED ) )
        || ( SfxItemState::SET == GetItemState( RES_CHRATR_COLOR ) );
}

bool SwTableBoxFormat::IsJustify() const
{
    return ( SfxItemState::SET == GetItemState( RES_PARATR_ADJUST ) );
}

bool SwTableBoxFormat::IsFrame() const
{
    return ( SfxItemState::SET == GetItemState( RES_BOX ) );
}

bool SwTableBoxFormat::IsBackground() const
{
    return ( SfxItemState::SET == GetItemState( RES_BACKGROUND ) );
}

bool SwTableBoxFormat::IsValueFormat() const
{
    return !sNumFormatString.isEmpty();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
