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

#include <tools/shl.hxx>
#include <svl/itemiter.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/module.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/graph.hxx>

#include <sfx2/sfxsids.hrc>
#include <svx/svxids.hrc>
#include <svx/dialogs.hrc>

#include <svl/intitem.hxx>

#include <svx/hdft.hxx>
#include <svx/pageitem.hxx>

#include "svx/dlgutil.hxx"
#include <svx/dialmgr.hxx>
#include <sfx2/htmlmode.hxx>

#include <editeng/brushitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/boxitem.hxx>

#include <svx/svxdlg.hxx>
// static ----------------------------------------------------------------

// Word 97 incompatibility (#i19922#)
static const long MINBODY = 56;  // 1mm in twips rounded

// default distance to Header or footer
static const long DEF_DIST_WRITER = 500;    // 5mm (Writer)
static const long DEF_DIST_CALC = 250;      // 2.5mm (Calc)

static sal_uInt16 pRanges[] =
{
    SID_ATTR_BRUSH,          SID_ATTR_BRUSH,
    SID_ATTR_BORDER_OUTER,   SID_ATTR_BORDER_OUTER,
    SID_ATTR_BORDER_INNER,   SID_ATTR_BORDER_INNER,
    SID_ATTR_BORDER_SHADOW,  SID_ATTR_BORDER_SHADOW,
    SID_ATTR_LRSPACE,        SID_ATTR_LRSPACE,
    SID_ATTR_ULSPACE,        SID_ATTR_ULSPACE,
    SID_ATTR_PAGE_SIZE,      SID_ATTR_PAGE_SIZE,
    SID_ATTR_PAGE_HEADERSET, SID_ATTR_PAGE_HEADERSET,
    SID_ATTR_PAGE_FOOTERSET, SID_ATTR_PAGE_FOOTERSET,
    SID_ATTR_PAGE_ON,        SID_ATTR_PAGE_ON,
    SID_ATTR_PAGE_DYNAMIC,   SID_ATTR_PAGE_DYNAMIC,
    SID_ATTR_PAGE_SHARED,    SID_ATTR_PAGE_SHARED,
    SID_ATTR_PAGE_SHARED_FIRST,    SID_ATTR_PAGE_SHARED_FIRST,
    SID_ATTR_HDFT_DYNAMIC_SPACING, SID_ATTR_HDFT_DYNAMIC_SPACING,
    0
};

namespace svx {

    bool ShowBorderBackgroundDlg( Window* pParent, SfxItemSet* pBBSet,
            bool bEnableBackgroundSelector )
    {
        bool bRes = false;
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        if(pFact)
        {
            SfxAbstractTabDialog* pDlg = pFact->CreateSvxBorderBackgroundDlg( pParent, *pBBSet, bEnableBackgroundSelector );
            DBG_ASSERT(pDlg, "Dialogdiet fail!");
            if ( pDlg->Execute() == RET_OK && pDlg->GetOutputItemSet() )
            {
                SfxItemIter aIter( *pDlg->GetOutputItemSet() );
                const SfxPoolItem* pItem = aIter.FirstItem();

                while ( pItem )
                {
                    if ( !IsInvalidItem( pItem ) )
                        pBBSet->Put( *pItem );
                    pItem = aIter.NextItem();
                }
                bRes = true;
            }
            delete pDlg;
        }
        return bRes;
    }
}

// returns the Which values to the range


sal_uInt16* SvxHeaderPage::GetRanges()
{
    return pRanges;
}

//------------------------------------------------------------------------

SfxTabPage* SvxHeaderPage::Create( Window* pParent, const SfxItemSet& rSet )
{
    return new SvxHeaderPage( pParent, rSet );
}

//------------------------------------------------------------------------

sal_uInt16* SvxFooterPage::GetRanges()
{
    return pRanges;
}

// -----------------------------------------------------------------------

SfxTabPage* SvxFooterPage::Create( Window* pParent, const SfxItemSet& rSet )
{
    return new SvxFooterPage( pParent, rSet );
}

// -----------------------------------------------------------------------

SvxHeaderPage::SvxHeaderPage( Window* pParent, const SfxItemSet& rAttr ) :

    SvxHFPage( pParent, rAttr, SID_ATTR_PAGE_HEADERSET )

{
}

// -----------------------------------------------------------------------

SvxFooterPage::SvxFooterPage( Window* pParent, const SfxItemSet& rAttr ) :

    SvxHFPage( pParent, rAttr, SID_ATTR_PAGE_FOOTERSET )

{
}

// -----------------------------------------------------------------------

SvxHFPage::SvxHFPage( Window* pParent, const SfxItemSet& rSet, sal_uInt16 nSetId ) :

    SfxTabPage( pParent, "HFFormatPage", "svx/ui/headfootformatpage.ui", rSet ),
    nId                         ( nSetId ),
    pBBSet                      ( NULL ),
    bDisableQueryBox            ( sal_False ),
    bEnableBackgroundSelector   ( sal_True )

{
    get(m_pCntSharedBox,"checkSameLR");
    get(m_pCntSharedFirstBox,"checkSameFP");
    get(m_pLMEdit,"spinMargLeft");
    get(m_pLMLbl,"labelLeftMarg");
    get(m_pRMEdit,"spinMargRight");
    get(m_pRMLbl,"labelRightMarg");
    get(m_pDistEdit,"spinSpacing");
    get(m_pDistFT,"labelSpacing");
    get(m_pDynSpacingCB,"checkDynSpacing");
    get(m_pHeightFT,"labelHeight");
    get(m_pHeightEdit,"spinHeight");
    get(m_pHeightDynBtn,"checkAutofit");
    get(m_pBspWin,"drawingareaPageHF");
    get(m_pBackgroundBtn,"buttonMore");

    //swap header <-> footer in UI
    if(nId == SID_ATTR_PAGE_FOOTERSET)
    {
        get(m_pPageLbl,"labelFooterFormat");
        get(m_pTurnOnBox, "checkFooterOn");
    }
    else //Header
    {
        get(m_pPageLbl,"labelHeaderFormat");
        get(m_pTurnOnBox, "checkHeaderOn");
    }
    m_pTurnOnBox->Show();
    m_pPageLbl->Show();

    InitHandler();
    m_pBspWin->EnableRTL( sal_False );

    // This Page needs ExchangeSupport
    SetExchangeSupport();


    // Set metrics
    FieldUnit eFUnit = GetModuleFieldUnit( rSet );
    SetFieldUnit( *m_pDistEdit, eFUnit );
    SetFieldUnit( *m_pHeightEdit, eFUnit );
    SetFieldUnit( *m_pLMEdit, eFUnit );
    SetFieldUnit( *m_pRMEdit, eFUnit );
}

// -----------------------------------------------------------------------

SvxHFPage::~SvxHFPage()
{
    delete pBBSet;
}

// -----------------------------------------------------------------------

sal_Bool SvxHFPage::FillItemSet( SfxItemSet& rSet )
{
    const sal_uInt16        nWSize      = GetWhich( SID_ATTR_PAGE_SIZE );
    const sal_uInt16        nWLRSpace   = GetWhich( SID_ATTR_LRSPACE );
    const sal_uInt16        nWULSpace   = GetWhich( SID_ATTR_ULSPACE );
    const sal_uInt16        nWOn        = GetWhich( SID_ATTR_PAGE_ON );
    const sal_uInt16        nWDynamic   = GetWhich( SID_ATTR_PAGE_DYNAMIC );
    const sal_uInt16        nWDynSpacing = GetWhich( SID_ATTR_HDFT_DYNAMIC_SPACING );
    const sal_uInt16        nWShared    = GetWhich( SID_ATTR_PAGE_SHARED );
    const sal_uInt16        nWSharedFirst = GetWhich( SID_ATTR_PAGE_SHARED_FIRST );
    const sal_uInt16        nWBrush     = GetWhich( SID_ATTR_BRUSH );
    const sal_uInt16        nWBox       = GetWhich( SID_ATTR_BORDER_OUTER );
    const sal_uInt16        nWBoxInfo   = GetWhich( SID_ATTR_BORDER_INNER );
    const sal_uInt16        nWShadow    = GetWhich( SID_ATTR_BORDER_SHADOW );
    const sal_uInt16        aWhichTab[] = { nWSize,     nWSize,
                                        nWLRSpace,  nWLRSpace,
                                        nWULSpace,  nWULSpace,
                                        nWOn,       nWOn,
                                        nWDynamic,  nWDynamic,
                                        nWShared,   nWShared,
                                        nWSharedFirst,   nWSharedFirst,
                                        nWBrush,    nWBrush,
                                        nWBoxInfo,  nWBoxInfo,
                                        nWBox,      nWBox,
                                        nWShadow,   nWShadow,
                                        nWDynSpacing, nWDynSpacing,
                                        0 };
    const SfxItemSet&   rOldSet     = GetItemSet();
    SfxItemPool*        pPool       = rOldSet.GetPool();
    DBG_ASSERT( pPool, "no pool :-(" );
    SfxMapUnit          eUnit       = pPool->GetMetric( nWSize );
    SfxItemSet          aSet        ( *pPool, aWhichTab );

    //--------------------------------------------------------------------

    aSet.Put( SfxBoolItem( nWOn,      m_pTurnOnBox->IsChecked() ) );
    aSet.Put( SfxBoolItem( nWDynamic, m_pHeightDynBtn->IsChecked() ) );
    aSet.Put( SfxBoolItem( nWShared,  m_pCntSharedBox->IsChecked() ) );
    if(m_pCntSharedFirstBox->IsVisible())
        aSet.Put( SfxBoolItem( nWSharedFirst,  m_pCntSharedFirstBox->IsChecked() ) );
    if(m_pDynSpacingCB->IsVisible() && SFX_WHICH_MAX > nWDynSpacing)
    {
        SfxBoolItem* pBoolItem = (SfxBoolItem*)pPool->GetDefaultItem(nWDynSpacing).Clone();
        pBoolItem->SetValue(m_pDynSpacingCB->IsChecked());
        aSet.Put(*pBoolItem);
        delete pBoolItem;
    }

    // Size
    SvxSizeItem aSizeItem( (const SvxSizeItem&)rOldSet.Get( nWSize ) );
    Size        aSize( aSizeItem.GetSize() );
    long        nDist = GetCoreValue( *m_pDistEdit, eUnit );
    long        nH    = GetCoreValue( *m_pHeightEdit, eUnit );

    nH += nDist; // add distance
    aSize.Height() = nH;
    aSizeItem.SetSize( aSize );
    aSet.Put( aSizeItem );

    // Margins
    SvxLRSpaceItem aLR( nWLRSpace );
    aLR.SetLeft( (sal_uInt16)GetCoreValue( *m_pLMEdit, eUnit ) );
    aLR.SetRight( (sal_uInt16)GetCoreValue( *m_pRMEdit, eUnit ) );
    aSet.Put( aLR );

    SvxULSpaceItem aUL( nWULSpace );
    if ( nId == SID_ATTR_PAGE_HEADERSET )
        aUL.SetLower( (sal_uInt16)nDist );
    else
        aUL.SetUpper( (sal_uInt16)nDist );
    aSet.Put( aUL );

    // Background and border?
    if ( pBBSet )
        aSet.Put( *pBBSet );
    else
    {
        const SfxPoolItem* pItem;

        if ( SFX_ITEM_SET ==
             GetItemSet().GetItemState( GetWhich( nId ), false, &pItem ) )
        {
            const SfxItemSet* _pSet;
            _pSet = &( (SvxSetItem*)pItem )->GetItemSet();

            if ( _pSet->GetItemState( nWBrush ) == SFX_ITEM_SET )
                aSet.Put( (const SvxBrushItem&)_pSet->Get( nWBrush ) );
            if ( _pSet->GetItemState( nWBoxInfo ) == SFX_ITEM_SET )
                aSet.Put( (const SvxBoxInfoItem&)_pSet->Get( nWBoxInfo ) );
            if ( _pSet->GetItemState( nWBox ) == SFX_ITEM_SET )
                aSet.Put( (const SvxBoxItem&)_pSet->Get( nWBox ) );
            if ( _pSet->GetItemState( nWShadow ) == SFX_ITEM_SET )
                aSet.Put( (const SvxShadowItem&)_pSet->Get( nWShadow ) );
        }
    }

    // Flush the SetItem
    SvxSetItem aSetItem( GetWhich( nId ), aSet );
    rSet.Put( aSetItem );

    return sal_True;
}

// -----------------------------------------------------------------------
void SvxHFPage::Reset( const SfxItemSet& rSet )
{
    ActivatePage( rSet );
    ResetBackground_Impl( rSet );

    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Where is the pool" );
    SfxMapUnit eUnit = pPool->GetMetric( GetWhich( SID_ATTR_PAGE_SIZE ) );

    //hide "same content on first page when this is calc
    bool bIsCalc = false;
    const SfxPoolItem* pExt1 = GetItem(rSet, SID_ATTR_PAGE_EXT1);
    const SfxPoolItem* pExt2 = GetItem(rSet, SID_ATTR_PAGE_EXT2);
    if (pExt1 && pExt1->ISA(SfxBoolItem) && pExt2 && pExt2->ISA(SfxBoolItem))
        bIsCalc = true;
    m_pCntSharedFirstBox->Show(!bIsCalc);

    // Evaluate header-/footer- attributes
    const SvxSetItem* pSetItem = 0;

    if ( SFX_ITEM_SET == rSet.GetItemState( GetWhich(nId), false,
                                            (const SfxPoolItem**)&pSetItem ) )
    {
        const SfxItemSet& rHeaderSet = pSetItem->GetItemSet();
        const SfxBoolItem& rHeaderOn =
            (const SfxBoolItem&)rHeaderSet.Get(GetWhich(SID_ATTR_PAGE_ON));

        m_pTurnOnBox->Check(rHeaderOn.GetValue());

        if ( rHeaderOn.GetValue() )
        {
            const SfxBoolItem& rDynamic =
                (const SfxBoolItem&)rHeaderSet.Get( GetWhich( SID_ATTR_PAGE_DYNAMIC ) );
            const SfxBoolItem& rShared =
                (const SfxBoolItem&)rHeaderSet.Get( GetWhich( SID_ATTR_PAGE_SHARED ) );
            const SfxBoolItem* pSharedFirst = 0;
            if (rHeaderSet.HasItem(GetWhich(SID_ATTR_PAGE_SHARED_FIRST)))
                pSharedFirst = (const SfxBoolItem*)&rHeaderSet.Get( GetWhich( SID_ATTR_PAGE_SHARED_FIRST ) );
            const SvxSizeItem& rSize =
                (const SvxSizeItem&)rHeaderSet.Get( GetWhich( SID_ATTR_PAGE_SIZE ) );
            const SvxULSpaceItem& rUL =
                (const SvxULSpaceItem&)rHeaderSet.Get( GetWhich( SID_ATTR_ULSPACE ) );
            const SvxLRSpaceItem& rLR =
                (const SvxLRSpaceItem&)rHeaderSet.Get( GetWhich( SID_ATTR_LRSPACE ) );
            if(m_pDynSpacingCB->IsVisible())
            {
                const SfxBoolItem& rDynSpacing =
                    (const SfxBoolItem&)rHeaderSet.Get(GetWhich(SID_ATTR_HDFT_DYNAMIC_SPACING));
                m_pDynSpacingCB->Check(rDynSpacing.GetValue());
            }


            if ( nId == SID_ATTR_PAGE_HEADERSET )
            {   // Header
                SetMetricValue( *m_pDistEdit, rUL.GetLower(), eUnit );
                SetMetricValue( *m_pHeightEdit, rSize.GetSize().Height() - rUL.GetLower(), eUnit );
            }
            else
            {   // Footer
                SetMetricValue( *m_pDistEdit, rUL.GetUpper(), eUnit );
                SetMetricValue( *m_pHeightEdit, rSize.GetSize().Height() - rUL.GetUpper(), eUnit );
            }

            m_pHeightDynBtn->Check(rDynamic.GetValue());
            SetMetricValue( *m_pLMEdit, rLR.GetLeft(), eUnit );
            SetMetricValue( *m_pRMEdit, rLR.GetRight(), eUnit );
            m_pCntSharedBox->Check(rShared.GetValue());
            if (pSharedFirst)
                m_pCntSharedFirstBox->Check(pSharedFirst->GetValue());
            else
                m_pCntSharedFirstBox->Hide();
        }
        else
            pSetItem = 0;
    }
    else
    {
        // defaults for distance and height
        long nDefaultDist = bIsCalc ? DEF_DIST_CALC : DEF_DIST_WRITER;
        SetMetricValue( *m_pDistEdit, nDefaultDist, SFX_MAPUNIT_100TH_MM );
        SetMetricValue( *m_pHeightEdit, 500, SFX_MAPUNIT_100TH_MM );
    }

    if ( !pSetItem )
    {
        m_pTurnOnBox->Check( sal_False );
        m_pHeightDynBtn->Check( sal_True );
        m_pCntSharedBox->Check( sal_True );
        m_pCntSharedFirstBox->Check( sal_True );
    }

    TurnOnHdl(0);

    m_pTurnOnBox->SaveValue();
    m_pDistEdit->SaveValue();
    m_pHeightEdit->SaveValue();
    m_pHeightDynBtn->SaveValue();
    m_pLMEdit->SaveValue();
    m_pRMEdit->SaveValue();
    m_pCntSharedBox->SaveValue();
    RangeHdl( 0 );

    const SfxPoolItem* pItem = 0;
    SfxObjectShell* pShell;
    if(SFX_ITEM_SET == rSet.GetItemState(SID_HTML_MODE, false, &pItem) ||
        ( 0 != (pShell = SfxObjectShell::Current()) &&
                    0 != (pItem = pShell->GetItem(SID_HTML_MODE))))
    {
        sal_uInt16 nHtmlMode = 0;
        nHtmlMode = ((SfxUInt16Item*)pItem)->GetValue();
        if(nHtmlMode & HTMLMODE_ON)
        {
            m_pCntSharedBox->Hide();
            m_pBackgroundBtn->Hide();
        }
    }

}

void SvxHFPage::InitHandler()
{
    m_pTurnOnBox->SetClickHdl(LINK(this,   SvxHFPage, TurnOnHdl));
    m_pDistEdit->SetModifyHdl(LINK(this,   SvxHFPage, DistModify));
    m_pDistEdit->SetLoseFocusHdl(LINK(this, SvxHFPage, RangeHdl));

    m_pHeightEdit->SetModifyHdl(LINK(this,     SvxHFPage, HeightModify));
    m_pHeightEdit->SetLoseFocusHdl(LINK(this,SvxHFPage,RangeHdl));

    m_pLMEdit->SetModifyHdl(LINK(this,         SvxHFPage, BorderModify));
    m_pLMEdit->SetLoseFocusHdl(LINK(this,  SvxHFPage, RangeHdl));
    m_pRMEdit->SetModifyHdl(LINK(this,         SvxHFPage, BorderModify));
    m_pRMEdit->SetLoseFocusHdl(LINK(this,  SvxHFPage, RangeHdl));
    m_pBackgroundBtn->SetClickHdl(LINK(this,SvxHFPage, BackgroundHdl));
}

IMPL_LINK( SvxHFPage, TurnOnHdl, CheckBox *, pBox )
{
    if ( m_pTurnOnBox->IsChecked() )
    {
        m_pDistFT->Enable();
        m_pDistEdit->Enable();
        m_pDynSpacingCB->Enable();
        m_pHeightFT->Enable();
        m_pHeightEdit->Enable();
        m_pHeightDynBtn->Enable();
        m_pLMLbl->Enable();
        m_pLMEdit->Enable();
        m_pRMLbl->Enable();
        m_pRMEdit->Enable();

        sal_uInt16 nUsage = m_pBspWin->GetUsage();

        if( nUsage == SVX_PAGE_RIGHT || nUsage == SVX_PAGE_LEFT )
            m_pCntSharedBox->Disable();
        else
        {
            m_pCntSharedBox->Enable();
            m_pCntSharedFirstBox->Enable();
        }
        m_pBackgroundBtn->Enable();
    }
    else
    {
        bool bDelete = true;

        if ( !bDisableQueryBox && pBox && m_pTurnOnBox->GetSavedValue() == sal_True )
        {
            short nResult;
            if (nId == SID_ATTR_PAGE_HEADERSET)
                nResult = DeleteHeaderDialog(this).Execute();
            else
                nResult = DeleteFooterDialog(this).Execute();
            bDelete = nResult == RET_YES;
        }

        if ( bDelete )
        {
            m_pDistFT->Disable();
            m_pDistEdit->Disable();
            m_pDynSpacingCB->Enable(false);
            m_pHeightFT->Disable();
            m_pHeightEdit->Disable();
            m_pHeightDynBtn->Disable();

            m_pLMLbl->Disable();
            m_pLMEdit->Disable();
            m_pRMLbl->Disable();
            m_pRMEdit->Disable();

            m_pCntSharedBox->Disable();
            m_pBackgroundBtn->Disable();
            m_pCntSharedFirstBox->Disable();
        }
        else
            m_pTurnOnBox->Check();
    }
    UpdateExample();
    return 0;
}

IMPL_LINK_NOARG_INLINE_START(SvxHFPage, DistModify)
{
    UpdateExample();
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SvxHFPage, DistModify)

IMPL_LINK_NOARG_INLINE_START(SvxHFPage, HeightModify)
{
    UpdateExample();

    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SvxHFPage, HeightModify)

IMPL_LINK_NOARG_INLINE_START(SvxHFPage, BorderModify)
{
    UpdateExample();
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SvxHFPage, BorderModify)

IMPL_LINK_NOARG(SvxHFPage, BackgroundHdl)
{
    if ( !pBBSet )
    {
        // Use only the necessary items for border and background
        sal_uInt16 nBrush = GetWhich( SID_ATTR_BRUSH );
        sal_uInt16 nOuter = GetWhich( SID_ATTR_BORDER_OUTER );
        sal_uInt16 nInner = GetWhich( SID_ATTR_BORDER_INNER, sal_False );
        sal_uInt16 nShadow = GetWhich( SID_ATTR_BORDER_SHADOW );

        // Create an empty set
        pBBSet = new SfxItemSet( *GetItemSet().GetPool(), nBrush, nBrush,
                                 nOuter, nOuter, nInner, nInner,
                                 nShadow, nShadow, 0 );
        const SfxPoolItem* pItem;

        if ( SFX_ITEM_SET ==
             GetItemSet().GetItemState( GetWhich( nId ), false, &pItem ) )
            // if there is one that is already set, then use this
            pBBSet->Put( ( (SvxSetItem*)pItem)->GetItemSet() );

        if ( SFX_ITEM_SET ==
             GetItemSet().GetItemState( nInner, false, &pItem ) )
            // The set InfoItem is always required
            pBBSet->Put( *pItem );
    }

    if ( svx::ShowBorderBackgroundDlg( this, pBBSet, bEnableBackgroundSelector ) )
    {
        //----------------------------------------------------------------

        sal_uInt16 nWhich = GetWhich( SID_ATTR_BRUSH );

        if ( pBBSet->GetItemState( nWhich ) == SFX_ITEM_SET )
        {
            const SvxBrushItem& rItem = (const SvxBrushItem&)pBBSet->Get( nWhich );
            if ( nId == SID_ATTR_PAGE_HEADERSET )
                m_pBspWin->SetHdColor( rItem.GetColor() );
            else
                m_pBspWin->SetFtColor( rItem.GetColor() );
        }

        //----------------------------------------------------------------

        nWhich = GetWhich( SID_ATTR_BORDER_OUTER );

        if ( pBBSet->GetItemState( nWhich ) == SFX_ITEM_SET )
        {
            const SvxBoxItem& rItem = (const SvxBoxItem&)pBBSet->Get( nWhich );

            if ( nId == SID_ATTR_PAGE_HEADERSET )
                m_pBspWin->SetHdBorder( rItem );
            else
                m_pBspWin->SetFtBorder( rItem );
        }

        UpdateExample();
    }

    return 0;
}

void SvxHFPage::UpdateExample()
{
    if ( nId == SID_ATTR_PAGE_HEADERSET )
    {
        m_pBspWin->SetHeader( m_pTurnOnBox->IsChecked() );
        m_pBspWin->SetHdHeight( GetCoreValue( *m_pHeightEdit, SFX_MAPUNIT_TWIP ) );
        m_pBspWin->SetHdDist( GetCoreValue( *m_pDistEdit, SFX_MAPUNIT_TWIP ) );
        m_pBspWin->SetHdLeft( GetCoreValue( *m_pLMEdit, SFX_MAPUNIT_TWIP ) );
        m_pBspWin->SetHdRight( GetCoreValue( *m_pRMEdit, SFX_MAPUNIT_TWIP ) );
    }
    else
    {
        m_pBspWin->SetFooter( m_pTurnOnBox->IsChecked() );
        m_pBspWin->SetFtHeight( GetCoreValue( *m_pHeightEdit, SFX_MAPUNIT_TWIP ) );
        m_pBspWin->SetFtDist( GetCoreValue( *m_pDistEdit, SFX_MAPUNIT_TWIP ) );
        m_pBspWin->SetFtLeft( GetCoreValue( *m_pLMEdit, SFX_MAPUNIT_TWIP ) );
        m_pBspWin->SetFtRight( GetCoreValue( *m_pRMEdit, SFX_MAPUNIT_TWIP ) );
    }
    m_pBspWin->Invalidate();
}

void SvxHFPage::ResetBackground_Impl( const SfxItemSet& rSet )
{
    sal_uInt16 nWhich = GetWhich( SID_ATTR_PAGE_HEADERSET );

    if ( rSet.GetItemState( nWhich, false ) == SFX_ITEM_SET )
    {
        const SvxSetItem& rSetItem =
            (const SvxSetItem&)rSet.Get( nWhich, sal_False );
        const SfxItemSet& rTmpSet = rSetItem.GetItemSet();
        const SfxBoolItem& rOn =
            (const SfxBoolItem&)rTmpSet.Get( GetWhich( SID_ATTR_PAGE_ON ) );

        if ( rOn.GetValue() )
        {
            nWhich = GetWhich( SID_ATTR_BRUSH );

            if ( rTmpSet.GetItemState( nWhich ) == SFX_ITEM_SET )
            {
                const SvxBrushItem& rItem = (const SvxBrushItem&)rTmpSet.Get( nWhich );
                m_pBspWin->SetHdColor( rItem.GetColor() );
            }
            nWhich = GetWhich( SID_ATTR_BORDER_OUTER );

            if ( rTmpSet.GetItemState( nWhich ) == SFX_ITEM_SET )
            {
                const SvxBoxItem& rItem =
                    (const SvxBoxItem&)rTmpSet.Get( nWhich );
                m_pBspWin->SetHdBorder( rItem );
            }
        }
    }

    nWhich = GetWhich( SID_ATTR_PAGE_FOOTERSET );

    if ( rSet.GetItemState( nWhich, false ) == SFX_ITEM_SET )
    {
        const SvxSetItem& rSetItem =
            (const SvxSetItem&)rSet.Get( nWhich, sal_False );
        const SfxItemSet& rTmpSet = rSetItem.GetItemSet();
        const SfxBoolItem& rOn =
            (const SfxBoolItem&)rTmpSet.Get( GetWhich( SID_ATTR_PAGE_ON ) );

        if ( rOn.GetValue() )
        {
            nWhich = GetWhich( SID_ATTR_BRUSH );

            if ( rTmpSet.GetItemState( nWhich ) == SFX_ITEM_SET )
            {
                const SvxBrushItem& rItem = (const SvxBrushItem&)rTmpSet.Get( nWhich );
                m_pBspWin->SetFtColor( rItem.GetColor() );
            }
            nWhich = GetWhich( SID_ATTR_BORDER_OUTER );

            if ( rTmpSet.GetItemState( nWhich ) == SFX_ITEM_SET )
            {
                const SvxBoxItem& rItem =
                    (const SvxBoxItem&)rTmpSet.Get( nWhich );
                m_pBspWin->SetFtBorder( rItem );
            }
        }
    }
    nWhich = GetWhich( SID_ATTR_BRUSH );

    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_AVAILABLE )
    {
        const SvxBrushItem& rItem = (const SvxBrushItem&)rSet.Get( nWhich );
        m_pBspWin->SetColor( rItem.GetColor() );
        const Graphic* pGrf = rItem.GetGraphic();

        if ( pGrf )
        {
            Bitmap aBitmap = pGrf->GetBitmap();
            m_pBspWin->SetBitmap( &aBitmap );
        }
        else
            m_pBspWin->SetBitmap( NULL );
    }
    nWhich = GetWhich( SID_ATTR_BORDER_OUTER );

    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_AVAILABLE )
    {
        const SvxBoxItem& rItem = (const SvxBoxItem&)rSet.Get( nWhich );
        m_pBspWin->SetBorder( rItem );
    }
}

void SvxHFPage::ActivatePage( const SfxItemSet& rSet )
{
    const SfxPoolItem* pItem = GetItem( rSet, SID_ATTR_LRSPACE );

    if ( pItem )
    {
        // Set left and right margins
        const SvxLRSpaceItem& rLRSpace = (const SvxLRSpaceItem&)*pItem;

        m_pBspWin->SetLeft( rLRSpace.GetLeft() );
        m_pBspWin->SetRight( rLRSpace.GetRight() );
    }
    else
    {
        m_pBspWin->SetLeft( 0 );
        m_pBspWin->SetRight( 0 );
    }

    pItem = GetItem( rSet, SID_ATTR_ULSPACE );

    if ( pItem )
    {
        // Set top and bottom margins
        const SvxULSpaceItem& rULSpace = (const SvxULSpaceItem&)*pItem;

        m_pBspWin->SetTop( rULSpace.GetUpper() );
        m_pBspWin->SetBottom( rULSpace.GetLower() );
    }
    else
    {
        m_pBspWin->SetTop( 0 );
        m_pBspWin->SetBottom( 0 );
    }

    sal_uInt16 nUsage = SVX_PAGE_ALL;
    pItem = GetItem( rSet, SID_ATTR_PAGE );

    if ( pItem )
        nUsage = ( (const SvxPageItem*)pItem )->GetPageUsage();

    m_pBspWin->SetUsage( nUsage );

    if ( SVX_PAGE_RIGHT == nUsage || SVX_PAGE_LEFT == nUsage )
        m_pCntSharedBox->Disable();
    else
    {
        m_pCntSharedBox->Enable();
        m_pCntSharedFirstBox->Enable();
    }
    pItem = GetItem( rSet, SID_ATTR_PAGE_SIZE );

    if ( pItem )
    {
        // Orientation and Size from the PageItem
        const SvxSizeItem& rSize = (const SvxSizeItem&)*pItem;
        // if the size is already swapped (Landscape)
        m_pBspWin->SetSize( rSize.GetSize() );
    }

    // Evaluate Header attribute
    const SvxSetItem* pSetItem = 0;

    if ( SFX_ITEM_SET == rSet.GetItemState( GetWhich( SID_ATTR_PAGE_HEADERSET ),
                                            false,
                                            (const SfxPoolItem**)&pSetItem ) )
    {
        const SfxItemSet& rHeaderSet = pSetItem->GetItemSet();
        const SfxBoolItem& rHeaderOn =
            (const SfxBoolItem&)rHeaderSet.Get( GetWhich( SID_ATTR_PAGE_ON ) );

        if ( rHeaderOn.GetValue() )
        {
            const SvxSizeItem& rSize = (const SvxSizeItem&)
                rHeaderSet.Get( GetWhich( SID_ATTR_PAGE_SIZE ) );
            const SvxULSpaceItem& rUL = (const SvxULSpaceItem&)
                rHeaderSet.Get( GetWhich(SID_ATTR_ULSPACE ) );
            const SvxLRSpaceItem& rLR = (const SvxLRSpaceItem&)
                rHeaderSet.Get( GetWhich( SID_ATTR_LRSPACE ) );
            long nDist = rUL.GetLower();

            m_pBspWin->SetHdHeight( rSize.GetSize().Height() - nDist );
            m_pBspWin->SetHdDist( nDist );
            m_pBspWin->SetHdLeft( rLR.GetLeft() );
            m_pBspWin->SetHdRight( rLR.GetRight() );
            m_pBspWin->SetHeader( sal_True );
        }
        else
            pSetItem = 0;
    }

    if ( !pSetItem )
    {
        m_pBspWin->SetHeader( sal_False );

        if ( SID_ATTR_PAGE_HEADERSET == nId )
        {
            m_pCntSharedBox->Disable();
            m_pCntSharedFirstBox->Disable();
        }
    }
    pSetItem = 0;

    if ( SFX_ITEM_SET == rSet.GetItemState( GetWhich( SID_ATTR_PAGE_FOOTERSET ),
                                            false,
                                            (const SfxPoolItem**)&pSetItem ) )
    {
        const SfxItemSet& rFooterSet = pSetItem->GetItemSet();
        const SfxBoolItem& rFooterOn =
            (const SfxBoolItem&)rFooterSet.Get( GetWhich( SID_ATTR_PAGE_ON ) );

        if ( rFooterOn.GetValue() )
        {
            const SvxSizeItem& rSize = (const SvxSizeItem&)
                rFooterSet.Get( GetWhich( SID_ATTR_PAGE_SIZE ) );
            const SvxULSpaceItem& rUL = (const SvxULSpaceItem&)
                rFooterSet.Get( GetWhich( SID_ATTR_ULSPACE ) );
            const SvxLRSpaceItem& rLR = (const SvxLRSpaceItem&)
                rFooterSet.Get( GetWhich( SID_ATTR_LRSPACE ) );
            long nDist = rUL.GetUpper();

            m_pBspWin->SetFtHeight( rSize.GetSize().Height() - nDist );
            m_pBspWin->SetFtDist( nDist );
            m_pBspWin->SetFtLeft( rLR.GetLeft() );
            m_pBspWin->SetFtRight( rLR.GetRight() );
            m_pBspWin->SetFooter( sal_True );
        }
        else
            pSetItem = 0;
    }

    if ( !pSetItem )
    {
        m_pBspWin->SetFooter( sal_False );

        if ( SID_ATTR_PAGE_FOOTERSET == nId )
        {
            m_pCntSharedBox->Disable();
            m_pCntSharedFirstBox->Disable();
        }
    }

    pItem = GetItem( rSet, SID_ATTR_PAGE_EXT1 );

    if ( pItem && pItem->ISA(SfxBoolItem) )
    {
        m_pBspWin->SetTable( sal_True );
        m_pBspWin->SetHorz( ( (SfxBoolItem*)pItem )->GetValue() );
    }

    pItem = GetItem( rSet, SID_ATTR_PAGE_EXT2 );

    if ( pItem && pItem->ISA(SfxBoolItem) )
    {
        m_pBspWin->SetTable( sal_True );
        m_pBspWin->SetVert( ( (SfxBoolItem*)pItem )->GetValue() );
    }
    ResetBackground_Impl( rSet );
    RangeHdl( 0 );
}

int SvxHFPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( *_pSet );
    return LEAVE_PAGE;
}

IMPL_LINK_NOARG(SvxHFPage, RangeHdl)
{
    long nHHeight = m_pBspWin->GetHdHeight();
    long nHDist   = m_pBspWin->GetHdDist();

    long nFHeight = m_pBspWin->GetFtHeight();
    long nFDist   = m_pBspWin->GetFtDist();

    long nHeight = std::max( (long)MINBODY,
        static_cast<long>(m_pHeightEdit->Denormalize( m_pHeightEdit->GetValue( FUNIT_TWIP ) ) ) );
    long nDist   = m_pTurnOnBox->IsChecked() ?
        static_cast<long>(m_pDistEdit->Denormalize( m_pDistEdit->GetValue( FUNIT_TWIP ) )) : 0;

    long nMin;
    long nMax;

    if ( nId == SID_ATTR_PAGE_HEADERSET )
    {
        nHHeight = nHeight;
        nHDist   = nDist;
    }
    else
    {
        nFHeight = nHeight;
        nFDist   = nDist;
    }

    // Current values of the side edges
    long nBT = m_pBspWin->GetTop();
    long nBB = m_pBspWin->GetBottom();
    long nBL = m_pBspWin->GetLeft();
    long nBR = m_pBspWin->GetRight();

    long nH  = m_pBspWin->GetSize().Height();
    long nW  = m_pBspWin->GetSize().Width();

    // Borders
    if ( nId == SID_ATTR_PAGE_HEADERSET )
    {
        // Header
        nMin = ( nH - nBB - nBT ) / 5; // 20%
        nMax = std::max( nH - nMin - nHDist - nFDist - nFHeight - nBB - nBT,
                    nMin );
        m_pHeightEdit->SetMax( m_pHeightEdit->Normalize( nMax ), FUNIT_TWIP );
        nMin = ( nH - nBB - nBT ) / 5; // 20%
        nDist = std::max( nH - nMin - nHHeight - nFDist - nFHeight - nBB - nBT,
                     long(0) );
        m_pDistEdit->SetMax( m_pDistEdit->Normalize( nDist ), FUNIT_TWIP );
    }
    else
    {
        // Footer
        nMin = ( nH - nBT - nBB ) / 5; // 20%
        nMax = std::max( nH - nMin - nFDist - nHDist - nHHeight - nBT - nBB,
                    nMin );
        m_pHeightEdit->SetMax( m_pHeightEdit->Normalize( nMax ), FUNIT_TWIP );
        nMin = ( nH - nBT - nBB ) / 5; // 20%
        nDist = std::max( nH - nMin - nFHeight - nHDist - nHHeight - nBT - nBB,
                     long(0) );
        m_pDistEdit->SetMax( m_pDistEdit->Normalize( nDist ), FUNIT_TWIP );
    }

    // Limit Indentation
    nMax = nW - nBL - nBR -
           static_cast<long>(m_pRMEdit->Denormalize( m_pRMEdit->GetValue( FUNIT_TWIP ) )) - MINBODY;
    m_pLMEdit->SetMax( m_pLMEdit->Normalize( nMax ), FUNIT_TWIP );

    nMax = nW - nBL - nBR -
           static_cast<long>(m_pLMEdit->Denormalize( m_pLMEdit->GetValue( FUNIT_TWIP ) )) - MINBODY;
    m_pRMEdit->SetMax( m_pLMEdit->Normalize( nMax ), FUNIT_TWIP );
    return 0;
}

static void lcl_Move(Window& rWin, sal_Int32 nDiff)
{
    Point aPos(rWin.GetPosPixel());
    aPos.Y() -= nDiff;
    rWin.SetPosPixel(aPos);
}
void SvxHFPage::EnableDynamicSpacing()
{
    m_pDynSpacingCB->Show();
    //move all following controls
    Window* aMoveWindows[] =
    {
        m_pHeightFT,
        m_pHeightEdit,
        m_pHeightDynBtn,
        m_pBackgroundBtn,
        0
    };
    sal_Int32 nOffset = m_pTurnOnBox->GetPosPixel().Y() - m_pCntSharedBox->GetPosPixel().Y();
    sal_Int32 nIdx = 0;
    while(aMoveWindows[nIdx])
        lcl_Move(*aMoveWindows[nIdx++], nOffset);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
