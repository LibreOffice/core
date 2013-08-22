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
        SwDoc* pDoc = GetDoc();

        m_pFstLineFmt.reset( pDoc->MakeTableLineFmt() );
        m_pLstLineFmt.reset( pDoc->MakeTableLineFmt() );
        m_pOddLineFmt.reset( pDoc->MakeTableLineFmt() );
        m_pEvnLineFmt.reset( pDoc->MakeTableLineFmt() );

        m_pFstColFmt.reset( pDoc->MakeTableLineFmt() );
        m_pLstColFmt.reset( pDoc->MakeTableLineFmt() );
        m_pOddColFmt.reset( pDoc->MakeTableLineFmt() );
        m_pEvnColFmt.reset( pDoc->MakeTableLineFmt() );
    }

SwTableFmt::SwTableFmt( SwAttrPool& rPool, const String &rFmtNm,
                    SwFrmFmt *pDrvdFrm )
    : SwFrmFmt( rPool, rFmtNm, pDrvdFrm, RES_FRMFMT, aTableSetRange )
    {
        SwDoc* pDoc = GetDoc();

        m_pFstLineFmt.reset( pDoc->MakeTableLineFmt() );
        m_pLstLineFmt.reset( pDoc->MakeTableLineFmt() );
        m_pOddLineFmt.reset( pDoc->MakeTableLineFmt() );
        m_pEvnLineFmt.reset( pDoc->MakeTableLineFmt() );

        m_pFstColFmt.reset( pDoc->MakeTableLineFmt() );
        m_pLstColFmt.reset( pDoc->MakeTableLineFmt() );
        m_pOddColFmt.reset( pDoc->MakeTableLineFmt() );
        m_pEvnColFmt.reset( pDoc->MakeTableLineFmt() );
    }

SwTableFmt::SwTableFmt( const SwTableFmt& rNew )
    : SwFrmFmt( (SwFrmFmt)rNew )
    {
        m_pFstLineFmt.reset( new SwTableLineFmt( *rNew.m_pFstLineFmt.get() ) );
        m_pLstLineFmt.reset( new SwTableLineFmt( *rNew.m_pLstLineFmt.get() ) );
        m_pOddLineFmt.reset( new SwTableLineFmt( *rNew.m_pOddLineFmt.get() ) );
        m_pEvnLineFmt.reset( new SwTableLineFmt( *rNew.m_pEvnLineFmt.get() ) );

        m_pFstColFmt.reset( new SwTableLineFmt( *rNew.m_pFstColFmt.get() ) );
        m_pLstColFmt.reset( new SwTableLineFmt( *rNew.m_pLstColFmt.get() ) );
        m_pOddColFmt.reset( new SwTableLineFmt( *rNew.m_pOddColFmt.get() ) );
        m_pEvnColFmt.reset( new SwTableLineFmt( *rNew.m_pEvnColFmt.get() ) );
    }

SwTableFmt& SwTableFmt::operator=( const SwTableFmt& rNew )
    {
        if (&rNew == this)
            return *this;

        SwFrmFmt::operator=( rNew );

        m_pFstLineFmt.reset( new SwTableLineFmt( *rNew.m_pFstLineFmt.get() ) );
        m_pLstLineFmt.reset( new SwTableLineFmt( *rNew.m_pLstLineFmt.get() ) );
        m_pOddLineFmt.reset( new SwTableLineFmt( *rNew.m_pOddLineFmt.get() ) );
        m_pEvnLineFmt.reset( new SwTableLineFmt( *rNew.m_pEvnLineFmt.get() ) );

        m_pFstColFmt.reset( new SwTableLineFmt( *rNew.m_pFstColFmt.get() ) );
        m_pLstColFmt.reset( new SwTableLineFmt( *rNew.m_pLstColFmt.get() ) );
        m_pOddColFmt.reset( new SwTableLineFmt( *rNew.m_pOddColFmt.get() ) );
        m_pEvnColFmt.reset( new SwTableLineFmt( *rNew.m_pEvnColFmt.get() ) );

        return *this;
    }

void SwTableFmt::SetBoxFmt( const SwTableBoxFmt& rNew, sal_uInt8 nPos )
{
    OSL_ENSURE( nPos < 16, "wrong area" );

    sal_uInt8 nLine = nPos / 4;
    sal_uInt8 nBox = nPos % 4;

    SwTableLineFmt* pLine = 0;

    switch( nLine )
    {
        case 0:
            pLine = m_pFstLineFmt.get(); break;
        case 1:
            pLine = m_pOddLineFmt.get(); break;
        case 2:
            pLine = m_pEvnLineFmt.get(); break;
        case 3:
            pLine = m_pLstLineFmt.get(); break;
        // TODO Extend for columns
    }

    switch( nBox )
    {
        case 0:
            pLine->SetFirstBoxFmt( new SwTableBoxFmt( rNew ) ); break;
        case 1:
            pLine->SetOddBoxFmt( new SwTableBoxFmt( rNew ) ); break;
        case 2:
            pLine->SetEvenBoxFmt( new SwTableBoxFmt( rNew ) ); break;
        case 3:
            pLine->SetLastBoxFmt( new SwTableBoxFmt( rNew ) ); break;
    }
}

SwTableBoxFmt* SwTableFmt::GetBoxFmt( sal_uInt8 nPos ) const
{
    OSL_ENSURE( nPos < 16, "wrong area" );

    sal_uInt8 nLine = nPos / 4;
    sal_uInt8 nBox = nPos % 4;

    SwTableLineFmt* pLine = 0;
    SwTableBoxFmt* pRet;

    switch( nLine )
    {
        case 0:
            pLine = m_pFstLineFmt.get(); break;
        case 1:
            pLine = m_pOddLineFmt.get(); break;
        case 2:
            pLine = m_pEvnLineFmt.get(); break;
        case 3:
            pLine = m_pLstLineFmt.get(); break;
        // TODO Extend for columns
    }

    switch( nBox )
    {
        case 0:
            pRet = pLine->GetFirstBoxFmt(); break;
        case 1:
            pRet = pLine->GetOddBoxFmt(); break;
        case 2:
            pRet = pLine->GetEvenBoxFmt(); break;
        case 3:
            pRet = pLine->GetLastBoxFmt(); break;
    }

    return pRet;
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
    const SfxPoolItem* pItem;

    if( SFX_ITEM_SET == GetItemState( FN_PARAM_TABLE_HEADLINE, sal_False, &pItem ) )
        return ((const SfxUInt16Item*)pItem)->GetValue();

    return 0;
}

void SwTableFmt::RestoreTableProperties( SwTableFmt* pSrcFmt, SwTable &rTable )
{
    SwTableFmt *pHardFmt = rTable.GetTableFmt();
    if( !pHardFmt )
        return;

    SwDoc *pDoc = pHardFmt->GetDoc();
    if( !pDoc )
        return;

    SwTableFmt *pTableStyle = (SwTableFmt*)pHardFmt->GetRegisteredIn();
    sal_Bool bRowSplit = sal_True;
    sal_uInt16 nRepeatHeading = 0;

    if( pSrcFmt )
    {
        pHardFmt->RegisterToFormat( *pSrcFmt );
        bRowSplit = pSrcFmt->GetRowSplit();
        nRepeatHeading = pSrcFmt->GetRepeatHeading();
    }
    else if( pTableStyle )
        pTableStyle->Remove( pHardFmt );

    AssignFormatParents( pSrcFmt, rTable );

    SwEditShell *pShell = pDoc->GetEditShell();
    pDoc->SetRowSplit( *pShell->getShellCrsr( false ), SwFmtRowSplit( bRowSplit ) );

    rTable.SetRowsToRepeat( nRepeatHeading );
}

SwTableFmt* SwTableFmt::StoreTableProperties( const SwTable &rTable )
{
    SwTableFmt *pHardFmt = rTable.GetTableFmt();
    if( !pHardFmt )
        return NULL;

    return (SwTableFmt*)pHardFmt->GetRegisteredIn();
}

void SwTableFmt::AssignFormatParents( SwTableFmt* pSrcFmt, SwTable &rTable )
{
    SwTableFmt *pHardFmt = rTable.GetTableFmt();
    if( !pHardFmt )
        return;

    if( pSrcFmt )
        pHardFmt->GetAttrSet().SetParent( &pSrcFmt->GetAttrSet() );
    else
        pHardFmt->GetAttrSet().SetParent( NULL );

    AssignLineParents( pSrcFmt, rTable );

    SwIterator<SwTabFrm,SwFmt> aIter( *rTable.GetFrmFmt() );
    for( SwTabFrm* pLast = aIter.First(); pLast; pLast = aIter.Next() )
        if( pLast->GetTable() == &rTable )
        {
            pLast->InvalidateAll();
            pLast->SetCompletePaint();
        }

    ((SwModify*)pHardFmt)->CheckCaching( RES_BOX );
}

void SwTableFmt::AssignLineParents( SwTableFmt* pSrcFmt, SwTable &rTable )
{
    sal_uInt16 nLines = rTable.GetTabLines().size();
    for( sal_uInt16 n = 0; n < nLines; ++n )
    {
        SwTableLineFmt* pLineFmt = (SwTableLineFmt*)rTable.GetTabLines()[ n ]->ClaimFrmFmt();
        SwTableLineFmt* pFmt = 0;

        if( pSrcFmt )
        {
            if( !n )
                pFmt = pSrcFmt->GetFirstLineFmt();
            else if( n == nLines - 1 )
                pFmt = pSrcFmt->GetLastLineFmt();
            else if( n & 1 )
                pFmt = pSrcFmt->GetEvenLineFmt();
            else
                pFmt = pSrcFmt->GetOddLineFmt();
        }

        if( pFmt )
        {
            pLineFmt->RegisterToFormat( *pFmt );
            pLineFmt->GetAttrSet().SetParent( &pFmt->GetAttrSet() );
        }
        else if( pLineFmt->GetRegisteredIn() )
        {
            ((SwTableLineFmt*)pLineFmt->GetRegisteredIn())->Remove( pLineFmt );
            pLineFmt->GetAttrSet().SetParent( NULL );
        }

        AssignBoxParents( pFmt, *rTable.GetTabLines()[ n ] );

        ((SwModify*)pLineFmt)->CheckCaching( RES_BOX );
    }
}

void SwTableFmt::AssignBoxParents( SwTableLineFmt* pSrcLineFmt, SwTableLine &rLine )
{
    sal_uInt16 nBoxes = rLine.GetTabBoxes().size();
    for( sal_uInt16 n = 0; n < nBoxes; ++n )
    {
        SwTableBoxFmt* pBoxFmt = (SwTableBoxFmt*)rLine.GetTabBoxes()[ n ]->ClaimFrmFmt();
        SwTableBoxFmt* pFmt = 0;

        if( pSrcLineFmt )
        {
            if( !n )
                pFmt = pSrcLineFmt->GetFirstBoxFmt();
            else if( n == nBoxes - 1 )
                pFmt = pSrcLineFmt->GetLastBoxFmt();
            else if( n & 1 )
                pFmt = pSrcLineFmt->GetEvenBoxFmt();
            else
                pFmt = pSrcLineFmt->GetOddBoxFmt();
        }

        if( pFmt )
        {
            pBoxFmt->RegisterToFormat( *pFmt );
            pBoxFmt->GetAttrSet().SetParent( &pFmt->GetAttrSet() );
        }
        else if( pBoxFmt->GetRegisteredIn() )
        {
            ((SwTableBoxFmt*)pBoxFmt->GetRegisteredIn())->Remove( pBoxFmt );
            pBoxFmt->GetAttrSet().SetParent( NULL );
        }

        if( rLine.GetTabBoxes()[ n ]->GetTabLines().size() )
            AssignLineParents_Complex( pSrcLineFmt, pFmt, *rLine.GetTabBoxes()[ n ] );

        ((SwModify*)pBoxFmt)->CheckCaching( RES_BOX );
    }
}

void SwTableFmt::AssignLineParents_Complex( SwTableLineFmt* pSrcLineFmt, SwTableBoxFmt* pSrcBoxFmt, SwTableBox& rBox )
{
    sal_uInt16 nLines = rBox.GetTabLines().size();
    for( sal_uInt16 n = 0; n < nLines; ++n )
    {
        SwTableLineFmt* pLineFmt = (SwTableLineFmt*)rBox.GetTabLines()[ n ]->ClaimFrmFmt();

        if( pSrcLineFmt )
        {
            pLineFmt->RegisterToFormat( *pSrcLineFmt );
            pLineFmt->GetAttrSet().SetParent( &pSrcLineFmt->GetAttrSet() );
        }
        else
        {
            ((SwTableLineFmt*)pLineFmt->GetRegisteredIn())->Remove( pLineFmt );
            pLineFmt->GetAttrSet().SetParent( NULL );
        }

        AssignBoxParents_Complex( pSrcLineFmt, pSrcBoxFmt, *rBox.GetTabLines()[ n ] );

        ((SwModify*)pLineFmt)->CheckCaching( RES_BOX );
    }
}

void SwTableFmt::AssignBoxParents_Complex( SwTableLineFmt* pSrcLineFmt, SwTableBoxFmt* pSrcBoxFmt, SwTableLine& rLine )
{
    sal_uInt16 nBoxes = rLine.GetTabBoxes().size();
    for( sal_uInt16 n = 0; n < nBoxes; ++n )
    {
        SwTableBoxFmt* pBoxFmt = (SwTableBoxFmt*)rLine.GetTabBoxes()[ n ]->ClaimFrmFmt();

        if( pSrcBoxFmt )
        {
            pBoxFmt->RegisterToFormat( *pSrcBoxFmt );
            pBoxFmt->GetAttrSet().SetParent( &pSrcBoxFmt->GetAttrSet() );
        }
        else
        {
            ((SwTableBoxFmt*)pBoxFmt->GetRegisteredIn())->Remove( pBoxFmt );
            pBoxFmt->GetAttrSet().SetParent( NULL );
        }

        if( rLine.GetTabBoxes()[ n ]->GetTabLines().size() )
            AssignLineParents_Complex( pSrcLineFmt, pSrcBoxFmt, *rLine.GetTabBoxes()[ n ] );

        ((SwModify*)pBoxFmt)->CheckCaching( RES_BOX );
    }
}

SwTableLineFmt::SwTableLineFmt( SwAttrPool& rPool, const sal_Char* pFmtNm,
                    SwFrmFmt *pDrvdFrm )
    : SwFrmFmt( rPool, pFmtNm, pDrvdFrm, RES_FRMFMT, aTableLineSetRange )
    {
        SwDoc* pDoc = GetDoc();

        m_pFstBoxFmt.reset( pDoc->MakeTableBoxFmt() );
        m_pLstBoxFmt.reset( pDoc->MakeTableBoxFmt() );
        m_pOddBoxFmt.reset( pDoc->MakeTableBoxFmt() );
        m_pEvnBoxFmt.reset( pDoc->MakeTableBoxFmt() );
    }

SwTableLineFmt::SwTableLineFmt( SwAttrPool& rPool, const String &rFmtNm,
                    SwFrmFmt *pDrvdFrm )
    : SwFrmFmt( rPool, rFmtNm, pDrvdFrm, RES_FRMFMT, aTableLineSetRange )
    {
        SwDoc* pDoc = GetDoc();

        m_pFstBoxFmt.reset( pDoc->MakeTableBoxFmt() );
        m_pLstBoxFmt.reset( pDoc->MakeTableBoxFmt() );
        m_pOddBoxFmt.reset( pDoc->MakeTableBoxFmt() );
        m_pEvnBoxFmt.reset( pDoc->MakeTableBoxFmt() );
    }

SwTableLineFmt::SwTableLineFmt( const SwTableLineFmt& rNew )
    : SwFrmFmt( (SwFrmFmt)rNew )
    {
        m_pFstBoxFmt.reset( new SwTableBoxFmt( *rNew.m_pFstBoxFmt.get() ) );
        m_pLstBoxFmt.reset( new SwTableBoxFmt( *rNew.m_pLstBoxFmt.get() ) );
        m_pOddBoxFmt.reset( new SwTableBoxFmt( *rNew.m_pOddBoxFmt.get() ) );
        m_pEvnBoxFmt.reset( new SwTableBoxFmt( *rNew.m_pEvnBoxFmt.get() ) );
    }

SwTableLineFmt& SwTableLineFmt::operator=( const SwTableLineFmt& rNew )
    {
        if (&rNew == this)
                return *this;

        SwFrmFmt::operator=( rNew );

        m_pFstBoxFmt.reset( new SwTableBoxFmt( *rNew.m_pFstBoxFmt.get() ) );
        m_pLstBoxFmt.reset( new SwTableBoxFmt( *rNew.m_pLstBoxFmt.get() ) );
        m_pOddBoxFmt.reset( new SwTableBoxFmt( *rNew.m_pOddBoxFmt.get() ) );
        m_pEvnBoxFmt.reset( new SwTableBoxFmt( *rNew.m_pEvnBoxFmt.get() ) );

        return *this;
    }

SwTableBoxFmt::SwTableBoxFmt( SwAttrPool& rPool, const sal_Char* pFmtNm,
                    SwFrmFmt *pDrvdFrm )
    : SwFrmFmt( rPool, pFmtNm, pDrvdFrm, RES_FRMFMT, aTableBoxSetRange )
    {}

SwTableBoxFmt::SwTableBoxFmt( SwAttrPool& rPool, const String &rFmtNm,
                    SwFrmFmt *pDrvdFrm )
    : SwFrmFmt( rPool, rFmtNm, pDrvdFrm, RES_FRMFMT, aTableBoxSetRange )
    {}

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

const SvxFontHeightItem& SwTableBoxFmt::GetHeight() const
{
    return SwFmt::GetSize();
}

const SvxFontHeightItem& SwTableBoxFmt::GetCJKHeight() const
{
    return SwFmt::GetCJKSize();
}

const SvxFontHeightItem& SwTableBoxFmt::GetCTLHeight() const
{
    return SwFmt::GetCTLSize();
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
