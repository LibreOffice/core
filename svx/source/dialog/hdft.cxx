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
#include "hdft.hrc"
#include <svl/intitem.hxx>

#define _SVX_HDFT_CXX

#include <svx/hdft.hxx>
#include <svx/pageitem.hxx>

#include "svx/dlgutil.hxx"
#include <svx/dialmgr.hxx>
#include "svx/htmlmode.hxx"

#include <editeng/brshitem.hxx>
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

    SvxHFPage( pParent, RID_SVXPAGE_HEADER, rAttr, SID_ATTR_PAGE_HEADERSET )

{
}

// -----------------------------------------------------------------------

SvxFooterPage::SvxFooterPage( Window* pParent, const SfxItemSet& rAttr ) :

    SvxHFPage( pParent, RID_SVXPAGE_FOOTER, rAttr, SID_ATTR_PAGE_FOOTERSET )

{
}

// -----------------------------------------------------------------------

SvxHFPage::SvxHFPage( Window* pParent, sal_uInt16 nResId, const SfxItemSet& rAttr, sal_uInt16 nSetId ) :

    SfxTabPage( pParent, SVX_RES( nResId ), rAttr ),

    aFrm            ( this, SVX_RES( FL_FRAME ) ),
    aTurnOnBox      ( this, SVX_RES( CB_TURNON ) ),
    aCntSharedBox   ( this, SVX_RES( CB_SHARED ) ),
    aCntSharedFirstBox   ( this, SVX_RES( CB_SHARED_FIRST ) ),
    aLMLbl          ( this, SVX_RES( FT_LMARGIN ) ),
    aLMEdit         ( this, SVX_RES( ED_LMARGIN ) ),
    aRMLbl          ( this, SVX_RES( FT_RMARGIN ) ),
    aRMEdit         ( this, SVX_RES( ED_RMARGIN ) ),
    aDistFT         ( this, SVX_RES( FT_DIST ) ),
    aDistEdit       ( this, SVX_RES( ED_DIST ) ),
    aDynSpacingCB   ( this, SVX_RES( CB_DYNSPACING ) ),
    aHeightFT       ( this, SVX_RES( FT_HEIGHT ) ),
    aHeightEdit     ( this, SVX_RES( ED_HEIGHT ) ),
    aHeightDynBtn   ( this, SVX_RES( CB_HEIGHT_DYN ) ),
    aBspWin         ( this, SVX_RES( WN_BSP ) ),
    aBackgroundBtn  ( this, SVX_RES( BTN_EXTRAS ) ),

    nId                         ( nSetId ),
    pBBSet                      ( NULL ),
    bDisableQueryBox            ( sal_False ),
    bEnableBackgroundSelector   ( sal_True )

{
    InitHandler();
    aBspWin.EnableRTL( sal_False );

    // This Page needs ExchangeSupport
    SetExchangeSupport();

    FreeResource();

    // Set metrics
    FieldUnit eFUnit = GetModuleFieldUnit( rAttr );
    SetFieldUnit( aDistEdit, eFUnit );
    SetFieldUnit( aHeightEdit, eFUnit );
    SetFieldUnit( aLMEdit, eFUnit );
    SetFieldUnit( aRMEdit, eFUnit );

    aTurnOnBox.SetAccessibleRelationMemberOf( &aFrm );
    aCntSharedBox.SetAccessibleRelationMemberOf( &aFrm );
    aCntSharedFirstBox.SetAccessibleRelationMemberOf( &aFrm );
    aLMLbl.SetAccessibleRelationMemberOf( &aFrm );
    aLMEdit.SetAccessibleRelationMemberOf( &aFrm );
    aRMLbl.SetAccessibleRelationMemberOf( &aFrm );
    aRMEdit.SetAccessibleRelationMemberOf( &aFrm );
    aDistFT.SetAccessibleRelationMemberOf( &aFrm );
    aDistEdit.SetAccessibleRelationMemberOf( &aFrm );
    aDynSpacingCB.SetAccessibleRelationMemberOf( &aFrm );
    aHeightFT.SetAccessibleRelationMemberOf( &aFrm );
    aHeightEdit.SetAccessibleRelationMemberOf( &aFrm );
    aHeightDynBtn.SetAccessibleRelationMemberOf( &aFrm );
    aBackgroundBtn.SetAccessibleRelationMemberOf(&aFrm);
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

    aSet.Put( SfxBoolItem( nWOn,      aTurnOnBox.IsChecked() ) );
    aSet.Put( SfxBoolItem( nWDynamic, aHeightDynBtn.IsChecked() ) );
    aSet.Put( SfxBoolItem( nWShared,  aCntSharedBox.IsChecked() ) );
    aSet.Put( SfxBoolItem( nWSharedFirst,  aCntSharedFirstBox.IsChecked() ) );
    if(aDynSpacingCB.IsVisible() && SFX_WHICH_MAX > nWDynSpacing)
    {
        SfxBoolItem* pBoolItem = (SfxBoolItem*)pPool->GetDefaultItem(nWDynSpacing).Clone();
        pBoolItem->SetValue(aDynSpacingCB.IsChecked());
        aSet.Put(*pBoolItem);
        delete pBoolItem;
    }

    // Size
    SvxSizeItem aSizeItem( (const SvxSizeItem&)rOldSet.Get( nWSize ) );
    Size        aSize( aSizeItem.GetSize() );
    long        nDist = GetCoreValue( aDistEdit, eUnit );
    long        nH    = GetCoreValue( aHeightEdit, eUnit );

    nH += nDist; // add distance
    aSize.Height() = nH;
    aSizeItem.SetSize( aSize );
    aSet.Put( aSizeItem );

    // Margins
    SvxLRSpaceItem aLR( nWLRSpace );
    aLR.SetLeft( (sal_uInt16)GetCoreValue( aLMEdit, eUnit ) );
    aLR.SetRight( (sal_uInt16)GetCoreValue( aRMEdit, eUnit ) );
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
        const SfxItemSet* _pSet;
        const SfxPoolItem* pItem;

        if ( SFX_ITEM_SET ==
             GetItemSet().GetItemState( GetWhich( nId ), sal_False, &pItem ) )
        {
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

    // Evaluate header-/footer- attributes
    const SvxSetItem* pSetItem = 0;

    if ( SFX_ITEM_SET == rSet.GetItemState( GetWhich(nId), sal_False,
                                            (const SfxPoolItem**)&pSetItem ) )
    {
        const SfxItemSet& rHeaderSet = pSetItem->GetItemSet();
        const SfxBoolItem& rHeaderOn =
            (const SfxBoolItem&)rHeaderSet.Get(GetWhich(SID_ATTR_PAGE_ON));

        aTurnOnBox.Check(rHeaderOn.GetValue());

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
            if(aDynSpacingCB.IsVisible())
            {
                const SfxBoolItem& rDynSpacing =
                    (const SfxBoolItem&)rHeaderSet.Get(GetWhich(SID_ATTR_HDFT_DYNAMIC_SPACING));
                aDynSpacingCB.Check(rDynSpacing.GetValue());
            }


            if ( nId == SID_ATTR_PAGE_HEADERSET )
            {   // Header
                SetMetricValue( aDistEdit, rUL.GetLower(), eUnit );
                SetMetricValue( aHeightEdit, rSize.GetSize().Height() - rUL.GetLower(), eUnit );
            }
            else
            {   // Footer
                SetMetricValue( aDistEdit, rUL.GetUpper(), eUnit );
                SetMetricValue( aHeightEdit, rSize.GetSize().Height() - rUL.GetUpper(), eUnit );
            }

            aHeightDynBtn.Check(rDynamic.GetValue());
            SetMetricValue( aLMEdit, rLR.GetLeft(), eUnit );
            SetMetricValue( aRMEdit, rLR.GetRight(), eUnit );
            aCntSharedBox.Check(rShared.GetValue());
            if (pSharedFirst)
                aCntSharedFirstBox.Check(pSharedFirst->GetValue());
            else
                aCntSharedFirstBox.Hide();
        }
        else
            pSetItem = 0;
    }
    else
    {
        // defaults for distance and height
        long nDefaultDist = DEF_DIST_WRITER;
        const SfxPoolItem* pExt1 = GetItem( rSet, SID_ATTR_PAGE_EXT1 );
        const SfxPoolItem* pExt2 = GetItem( rSet, SID_ATTR_PAGE_EXT2 );

        if ( pExt1 && pExt1->ISA(SfxBoolItem) && pExt2 && pExt2->ISA(SfxBoolItem) )
            nDefaultDist = DEF_DIST_CALC;

        SetMetricValue( aDistEdit, nDefaultDist, SFX_MAPUNIT_100TH_MM );
        SetMetricValue( aHeightEdit, 500, SFX_MAPUNIT_100TH_MM );
    }

    if ( !pSetItem )
    {
        aTurnOnBox.Check( sal_False );
        aHeightDynBtn.Check( sal_True );
        aCntSharedBox.Check( sal_True );
        aCntSharedFirstBox.Check( sal_True );
    }

    TurnOnHdl(0);

    aTurnOnBox.SaveValue();
    aDistEdit.SaveValue();
    aHeightEdit.SaveValue();
    aHeightDynBtn.SaveValue();
    aLMEdit.SaveValue();
    aRMEdit.SaveValue();
    aCntSharedBox.SaveValue();
    RangeHdl( 0 );

    const SfxPoolItem* pItem = 0;
    SfxObjectShell* pShell;
    if(SFX_ITEM_SET == rSet.GetItemState(SID_HTML_MODE, sal_False, &pItem) ||
        ( 0 != (pShell = SfxObjectShell::Current()) &&
                    0 != (pItem = pShell->GetItem(SID_HTML_MODE))))
    {
        sal_uInt16 nHtmlMode = 0;
        nHtmlMode = ((SfxUInt16Item*)pItem)->GetValue();
        if(nHtmlMode & HTMLMODE_ON)
        {
            aCntSharedBox.Hide();
            aBackgroundBtn.Hide();
        }
    }

}

void SvxHFPage::InitHandler()
{
    aTurnOnBox.SetClickHdl(LINK(this,   SvxHFPage, TurnOnHdl));
    aDistEdit.SetModifyHdl(LINK(this,   SvxHFPage, DistModify));
    aDistEdit.SetLoseFocusHdl(LINK(this, SvxHFPage, RangeHdl));

    aHeightEdit.SetModifyHdl(LINK(this,     SvxHFPage, HeightModify));
    aHeightEdit.SetLoseFocusHdl(LINK(this,SvxHFPage,RangeHdl));

    aLMEdit.SetModifyHdl(LINK(this,         SvxHFPage, BorderModify));
    aLMEdit.SetLoseFocusHdl(LINK(this,  SvxHFPage, RangeHdl));
    aRMEdit.SetModifyHdl(LINK(this,         SvxHFPage, BorderModify));
    aRMEdit.SetLoseFocusHdl(LINK(this,  SvxHFPage, RangeHdl));
    aBackgroundBtn.SetClickHdl(LINK(this,SvxHFPage, BackgroundHdl));
}

IMPL_LINK( SvxHFPage, TurnOnHdl, CheckBox *, pBox )
{
    if ( aTurnOnBox.IsChecked() )
    {
        aDistFT.Enable();
        aDistEdit.Enable();
        aDynSpacingCB.Enable();
        aHeightFT.Enable();
        aHeightEdit.Enable();
        aHeightDynBtn.Enable();
        aLMLbl.Enable();
        aLMEdit.Enable();
        aRMLbl.Enable();
        aRMEdit.Enable();

        sal_uInt16 nUsage = aBspWin.GetUsage();

        if( nUsage == SVX_PAGE_RIGHT || nUsage == SVX_PAGE_LEFT )
            aCntSharedBox.Disable();
        else
        {
            aCntSharedBox.Enable();
            aCntSharedFirstBox.Enable();
        }
        aBackgroundBtn.Enable();
    }
    else
    {
        sal_Bool bDelete = sal_True;

        if ( !bDisableQueryBox && pBox && aTurnOnBox.GetSavedValue() == sal_True )
            bDelete = ( QueryBox( this, SVX_RES( RID_SVXQBX_DELETE_HEADFOOT ) ).Execute() == RET_YES );

        if ( bDelete )
        {
            aDistFT.Disable();
            aDistEdit.Disable();
            aDynSpacingCB.Enable(sal_False);
            aHeightFT.Disable();
            aHeightEdit.Disable();
            aHeightDynBtn.Disable();

            aLMLbl.Disable();
            aLMEdit.Disable();
            aRMLbl.Disable();
            aRMEdit.Disable();

            aCntSharedBox.Disable();
            aBackgroundBtn.Disable();
        }
        else
            aTurnOnBox.Check();
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
             GetItemSet().GetItemState( GetWhich( nId ), sal_False, &pItem ) )
            // if there is one that is already set, then use this
            pBBSet->Put( ( (SvxSetItem*)pItem)->GetItemSet() );

        if ( SFX_ITEM_SET ==
             GetItemSet().GetItemState( nInner, sal_False, &pItem ) )
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
                aBspWin.SetHdColor( rItem.GetColor() );
            else
                aBspWin.SetFtColor( rItem.GetColor() );
        }

        //----------------------------------------------------------------

        nWhich = GetWhich( SID_ATTR_BORDER_OUTER );

        if ( pBBSet->GetItemState( nWhich ) == SFX_ITEM_SET )
        {
            const SvxBoxItem& rItem = (const SvxBoxItem&)pBBSet->Get( nWhich );

            if ( nId == SID_ATTR_PAGE_HEADERSET )
                aBspWin.SetHdBorder( rItem );
            else
                aBspWin.SetFtBorder( rItem );
        }

        UpdateExample();
    }

    return 0;
}

void SvxHFPage::UpdateExample()
{
    if ( nId == SID_ATTR_PAGE_HEADERSET )
    {
        aBspWin.SetHeader( aTurnOnBox.IsChecked() );
        aBspWin.SetHdHeight( GetCoreValue( aHeightEdit, SFX_MAPUNIT_TWIP ) );
        aBspWin.SetHdDist( GetCoreValue( aDistEdit, SFX_MAPUNIT_TWIP ) );
        aBspWin.SetHdLeft( GetCoreValue( aLMEdit, SFX_MAPUNIT_TWIP ) );
        aBspWin.SetHdRight( GetCoreValue( aRMEdit, SFX_MAPUNIT_TWIP ) );
    }
    else
    {
        aBspWin.SetFooter( aTurnOnBox.IsChecked() );
        aBspWin.SetFtHeight( GetCoreValue( aHeightEdit, SFX_MAPUNIT_TWIP ) );
        aBspWin.SetFtDist( GetCoreValue( aDistEdit, SFX_MAPUNIT_TWIP ) );
        aBspWin.SetFtLeft( GetCoreValue( aLMEdit, SFX_MAPUNIT_TWIP ) );
        aBspWin.SetFtRight( GetCoreValue( aRMEdit, SFX_MAPUNIT_TWIP ) );
    }
    aBspWin.Invalidate();
}

void SvxHFPage::ResetBackground_Impl( const SfxItemSet& rSet )
{
    sal_uInt16 nWhich = GetWhich( SID_ATTR_PAGE_HEADERSET );

    if ( rSet.GetItemState( nWhich, sal_False ) == SFX_ITEM_SET )
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
                aBspWin.SetHdColor( rItem.GetColor() );
            }
            nWhich = GetWhich( SID_ATTR_BORDER_OUTER );

            if ( rTmpSet.GetItemState( nWhich ) == SFX_ITEM_SET )
            {
                const SvxBoxItem& rItem =
                    (const SvxBoxItem&)rTmpSet.Get( nWhich );
                aBspWin.SetHdBorder( rItem );
            }
        }
    }

    nWhich = GetWhich( SID_ATTR_PAGE_FOOTERSET );

    if ( rSet.GetItemState( nWhich, sal_False ) == SFX_ITEM_SET )
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
                aBspWin.SetFtColor( rItem.GetColor() );
            }
            nWhich = GetWhich( SID_ATTR_BORDER_OUTER );

            if ( rTmpSet.GetItemState( nWhich ) == SFX_ITEM_SET )
            {
                const SvxBoxItem& rItem =
                    (const SvxBoxItem&)rTmpSet.Get( nWhich );
                aBspWin.SetFtBorder( rItem );
            }
        }
    }
    nWhich = GetWhich( SID_ATTR_BRUSH );

    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_AVAILABLE )
    {
        const SvxBrushItem& rItem = (const SvxBrushItem&)rSet.Get( nWhich );
        aBspWin.SetColor( rItem.GetColor() );
        const Graphic* pGrf = rItem.GetGraphic();

        if ( pGrf )
        {
            Bitmap aBitmap = pGrf->GetBitmap();
            aBspWin.SetBitmap( &aBitmap );
        }
        else
            aBspWin.SetBitmap( NULL );
    }
    nWhich = GetWhich( SID_ATTR_BORDER_OUTER );

    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_AVAILABLE )
    {
        const SvxBoxItem& rItem = (const SvxBoxItem&)rSet.Get( nWhich );
        aBspWin.SetBorder( rItem );
    }
}

void SvxHFPage::ActivatePage( const SfxItemSet& rSet )
{
    const SfxPoolItem* pItem = GetItem( rSet, SID_ATTR_LRSPACE );

    if ( pItem )
    {
        // Set left and right margins
        const SvxLRSpaceItem& rLRSpace = (const SvxLRSpaceItem&)*pItem;

        aBspWin.SetLeft( rLRSpace.GetLeft() );
        aBspWin.SetRight( rLRSpace.GetRight() );
    }
    else
    {
        aBspWin.SetLeft( 0 );
        aBspWin.SetRight( 0 );
    }

    pItem = GetItem( rSet, SID_ATTR_ULSPACE );

    if ( pItem )
    {
        // Set top and bottom margins
        const SvxULSpaceItem& rULSpace = (const SvxULSpaceItem&)*pItem;

        aBspWin.SetTop( rULSpace.GetUpper() );
        aBspWin.SetBottom( rULSpace.GetLower() );
    }
    else
    {
        aBspWin.SetTop( 0 );
        aBspWin.SetBottom( 0 );
    }

    sal_uInt16 nUsage = SVX_PAGE_ALL;
    pItem = GetItem( rSet, SID_ATTR_PAGE );

    if ( pItem )
        nUsage = ( (const SvxPageItem*)pItem )->GetPageUsage();

    aBspWin.SetUsage( nUsage );

    if ( SVX_PAGE_RIGHT == nUsage || SVX_PAGE_LEFT == nUsage )
        aCntSharedBox.Disable();
    else
    {
        aCntSharedBox.Enable();
        aCntSharedFirstBox.Enable();
    }
    pItem = GetItem( rSet, SID_ATTR_PAGE_SIZE );

    if ( pItem )
    {
        // Orientation and Size from the PageItem
        const SvxSizeItem& rSize = (const SvxSizeItem&)*pItem;
        // if the size is already swapped (Landscape)
        aBspWin.SetSize( rSize.GetSize() );
    }

    // Evaluate Header attribute
    const SvxSetItem* pSetItem = 0;

    if ( SFX_ITEM_SET == rSet.GetItemState( GetWhich( SID_ATTR_PAGE_HEADERSET ),
                                            sal_False,
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

            aBspWin.SetHdHeight( rSize.GetSize().Height() - nDist );
            aBspWin.SetHdDist( nDist );
            aBspWin.SetHdLeft( rLR.GetLeft() );
            aBspWin.SetHdRight( rLR.GetRight() );
            aBspWin.SetHeader( sal_True );
        }
        else
            pSetItem = 0;
    }

    if ( !pSetItem )
    {
        aBspWin.SetHeader( sal_False );

        if ( SID_ATTR_PAGE_HEADERSET == nId )
        {
            aCntSharedBox.Disable();
            aCntSharedFirstBox.Disable();
        }
    }
    pSetItem = 0;

    if ( SFX_ITEM_SET == rSet.GetItemState( GetWhich( SID_ATTR_PAGE_FOOTERSET ),
                                            sal_False,
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

            aBspWin.SetFtHeight( rSize.GetSize().Height() - nDist );
            aBspWin.SetFtDist( nDist );
            aBspWin.SetFtLeft( rLR.GetLeft() );
            aBspWin.SetFtRight( rLR.GetRight() );
            aBspWin.SetFooter( sal_True );
        }
        else
            pSetItem = 0;
    }

    if ( !pSetItem )
    {
        aBspWin.SetFooter( sal_False );

        if ( SID_ATTR_PAGE_FOOTERSET == nId )
        {
            aCntSharedBox.Disable();
            aCntSharedFirstBox.Disable();
        }
    }

    pItem = GetItem( rSet, SID_ATTR_PAGE_EXT1 );

    if ( pItem && pItem->ISA(SfxBoolItem) )
    {
        aBspWin.SetTable( sal_True );
        aBspWin.SetHorz( ( (SfxBoolItem*)pItem )->GetValue() );
    }

    pItem = GetItem( rSet, SID_ATTR_PAGE_EXT2 );

    if ( pItem && pItem->ISA(SfxBoolItem) )
    {
        aBspWin.SetTable( sal_True );
        aBspWin.SetVert( ( (SfxBoolItem*)pItem )->GetValue() );
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
    long nHHeight = aBspWin.GetHdHeight();
    long nHDist   = aBspWin.GetHdDist();

    long nFHeight = aBspWin.GetFtHeight();
    long nFDist   = aBspWin.GetFtDist();

    long nHeight = Max( (long)MINBODY,
        static_cast<long>(aHeightEdit.Denormalize( aHeightEdit.GetValue( FUNIT_TWIP ) ) ) );
    long nDist   = aTurnOnBox.IsChecked() ?
        static_cast<long>(aDistEdit.Denormalize( aDistEdit.GetValue( FUNIT_TWIP ) )) : 0;

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
    long nBT = aBspWin.GetTop();
    long nBB = aBspWin.GetBottom();
    long nBL = aBspWin.GetLeft();
    long nBR = aBspWin.GetRight();

    long nH  = aBspWin.GetSize().Height();
    long nW  = aBspWin.GetSize().Width();

    // Borders
    if ( nId == SID_ATTR_PAGE_HEADERSET )
    {
        // Header
        nMin = ( nH - nBB - nBT ) / 5; // 20%
        nMax = Max( nH - nMin - nHDist - nFDist - nFHeight - nBB - nBT,
                    nMin );
        aHeightEdit.SetMax( aHeightEdit.Normalize( nMax ), FUNIT_TWIP );
        nMin = ( nH - nBB - nBT ) / 5; // 20%
        nDist = Max( nH - nMin - nHHeight - nFDist - nFHeight - nBB - nBT,
                     long(0) );
        aDistEdit.SetMax( aDistEdit.Normalize( nDist ), FUNIT_TWIP );
    }
    else
    {
        // Footer
        nMin = ( nH - nBT - nBB ) / 5; // 20%
        nMax = Max( nH - nMin - nFDist - nHDist - nHHeight - nBT - nBB,
                    nMin );
        aHeightEdit.SetMax( aHeightEdit.Normalize( nMax ), FUNIT_TWIP );
        nMin = ( nH - nBT - nBB ) / 5; // 20%
        nDist = Max( nH - nMin - nFHeight - nHDist - nHHeight - nBT - nBB,
                     long(0) );
        aDistEdit.SetMax( aDistEdit.Normalize( nDist ), FUNIT_TWIP );
    }

    // Limit Indentation
    nMax = nW - nBL - nBR -
           static_cast<long>(aRMEdit.Denormalize( aRMEdit.GetValue( FUNIT_TWIP ) )) - MINBODY;
    aLMEdit.SetMax( aLMEdit.Normalize( nMax ), FUNIT_TWIP );

    nMax = nW - nBL - nBR -
           static_cast<long>(aLMEdit.Denormalize( aLMEdit.GetValue( FUNIT_TWIP ) )) - MINBODY;
    aRMEdit.SetMax( aLMEdit.Normalize( nMax ), FUNIT_TWIP );
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
    aDynSpacingCB.Show();
    //move all following controls
    Window* aMoveWindows[] =
    {
        &aHeightFT,
        &aHeightEdit,
        &aHeightDynBtn,
        &aBackgroundBtn,
        0
    };
    sal_Int32 nOffset = aTurnOnBox.GetPosPixel().Y() - aCntSharedBox.GetPosPixel().Y();
    sal_Int32 nIdx = 0;
    while(aMoveWindows[nIdx])
        lcl_Move(*aMoveWindows[nIdx++], nOffset);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
