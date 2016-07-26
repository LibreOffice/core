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
#include <memory>

//UUUU
#include <svx/xdef.hxx>
#include <svx/xenum.hxx>
#include <svx/xfillit0.hxx>
#include <svx/unobrushitemhelper.hxx>
#include <sfx2/request.hxx>

using namespace com::sun::star;

// Word 97 incompatibility (#i19922#)
static const long MINBODY = 56;  // 1mm in twips rounded

// default distance to Header or footer
static const long DEF_DIST_WRITER = 500;    // 5mm (Writer)
static const long DEF_DIST_CALC = 250;      // 2.5mm (Calc)

const sal_uInt16 SvxHFPage::pRanges[] =
{
    SID_ATTR_BRUSH,          SID_ATTR_BRUSH,

    //UUUU Support DrawingLayer FillStyles (no real call to below GetRanges()
    // detected, still do the complete transition)
    XATTR_FILL_FIRST,        XATTR_FILL_LAST,

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

    bool ShowBorderBackgroundDlg( vcl::Window* pParent, SfxItemSet* pBBSet,
            bool bEnableBackgroundSelector )
    {
        bool bRes = false;
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        if(pFact)
        {
            std::unique_ptr<SfxAbstractTabDialog> pDlg(pFact->CreateSvxBorderBackgroundDlg( pParent, *pBBSet, bEnableBackgroundSelector ));
            DBG_ASSERT(pDlg, "Dialog creation failed!");
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
        }
        return bRes;
    }
}

VclPtr<SfxTabPage> SvxHeaderPage::Create( vcl::Window* pParent, const SfxItemSet* rSet )
{
    return VclPtr<SvxHeaderPage>::Create( pParent, *rSet );
}


VclPtr<SfxTabPage> SvxFooterPage::Create( vcl::Window* pParent, const SfxItemSet* rSet )
{
    return VclPtr<SvxFooterPage>::Create( pParent, *rSet );
}


SvxHeaderPage::SvxHeaderPage( vcl::Window* pParent, const SfxItemSet& rAttr ) :

    SvxHFPage( pParent, rAttr, SID_ATTR_PAGE_HEADERSET )

{
}


SvxFooterPage::SvxFooterPage( vcl::Window* pParent, const SfxItemSet& rAttr ) :

    SvxHFPage( pParent, rAttr, SID_ATTR_PAGE_FOOTERSET )

{
}


SvxHFPage::SvxHFPage( vcl::Window* pParent, const SfxItemSet& rSet, sal_uInt16 nSetId ) :
    SfxTabPage(pParent, "HFFormatPage", "svx/ui/headfootformatpage.ui", &rSet),
    nId(nSetId),
    pBBSet(nullptr),
    // bitfield
    mbDisableQueryBox(false),
    mbEnableBackgroundSelector(true),
    mbEnableDrawingLayerFillStyles(false)
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
    m_pBspWin->EnableRTL( false );

    // This Page needs ExchangeSupport
    SetExchangeSupport();


    // Set metrics
    FieldUnit eFUnit = GetModuleFieldUnit( rSet );
    SetFieldUnit( *m_pDistEdit, eFUnit );
    SetFieldUnit( *m_pHeightEdit, eFUnit );
    SetFieldUnit( *m_pLMEdit, eFUnit );
    SetFieldUnit( *m_pRMEdit, eFUnit );
}


SvxHFPage::~SvxHFPage()
{
    disposeOnce();
}

void SvxHFPage::dispose()
{
    delete pBBSet;
    m_pPageLbl.clear();
    m_pTurnOnBox.clear();
    m_pCntSharedBox.clear();
    m_pCntSharedFirstBox.clear();
    m_pLMLbl.clear();
    m_pLMEdit.clear();
    m_pRMLbl.clear();
    m_pRMEdit.clear();
    m_pDistFT.clear();
    m_pDistEdit.clear();
    m_pDynSpacingCB.clear();
    m_pHeightFT.clear();
    m_pHeightEdit.clear();
    m_pHeightDynBtn.clear();
    m_pBspWin.clear();
    m_pBackgroundBtn.clear();
    SfxTabPage::dispose();
}


bool SvxHFPage::FillItemSet( SfxItemSet* rSet )
{
    const sal_uInt16 nWSize = GetWhich(SID_ATTR_PAGE_SIZE);
    const sal_uInt16 nWLRSpace = GetWhich(SID_ATTR_LRSPACE);
    const sal_uInt16 nWULSpace = GetWhich(SID_ATTR_ULSPACE);
    const sal_uInt16 nWOn = GetWhich(SID_ATTR_PAGE_ON);
    const sal_uInt16 nWDynamic = GetWhich(SID_ATTR_PAGE_DYNAMIC);
    const sal_uInt16 nWDynSpacing = GetWhich(SID_ATTR_HDFT_DYNAMIC_SPACING);
    const sal_uInt16 nWShared = GetWhich(SID_ATTR_PAGE_SHARED);
    const sal_uInt16 nWSharedFirst = GetWhich( SID_ATTR_PAGE_SHARED_FIRST );
    const sal_uInt16 nWBrush = GetWhich(SID_ATTR_BRUSH);
    const sal_uInt16 nWBox = GetWhich(SID_ATTR_BORDER_OUTER);
    const sal_uInt16 nWBoxInfo = GetWhich(SID_ATTR_BORDER_INNER);
    const sal_uInt16 nWShadow = GetWhich(SID_ATTR_BORDER_SHADOW);

    const sal_uInt16 aWhichTab[] = {
        nWSize, nWSize,
        nWLRSpace, nWLRSpace,
        nWULSpace, nWULSpace,
        nWOn, nWOn,
        nWDynamic, nWDynamic,
        nWShared, nWShared,
        nWSharedFirst, nWSharedFirst,
        nWBrush, nWBrush,
        nWBoxInfo, nWBoxInfo,
        nWBox, nWBox,
        nWShadow, nWShadow,
        nWDynSpacing, nWDynSpacing,

        //UUUU take over DrawingLayer FillStyles
        XATTR_FILL_FIRST, XATTR_FILL_LAST,                // [1014

        0, 0};

    const SfxItemSet& rOldSet = GetItemSet();
    SfxItemPool* pPool = rOldSet.GetPool();
    DBG_ASSERT(pPool,"no pool :-(");
    SfxMapUnit eUnit = pPool->GetMetric(nWSize);
    SfxItemSet aSet(*pPool,aWhichTab);

    if(mbEnableDrawingLayerFillStyles)
    {
        //UUUU When using the XATTR_FILLSTYLE DrawingLayer FillStyle definition
        // extra action has to be done here since the pool default is drawing::FillStyle_SOLID
        // instead of drawing::FillStyle_NONE (to have the default blue fill color at start).
        aSet.Put(XFillStyleItem(drawing::FillStyle_NONE));
    }

    aSet.Put( SfxBoolItem( nWOn,      m_pTurnOnBox->IsChecked() ) );
    aSet.Put( SfxBoolItem( nWDynamic, m_pHeightDynBtn->IsChecked() ) );
    aSet.Put( SfxBoolItem( nWShared,  m_pCntSharedBox->IsChecked() ) );
    if(m_pCntSharedFirstBox->IsVisible())
        aSet.Put( SfxBoolItem( nWSharedFirst,  m_pCntSharedFirstBox->IsChecked() ) );
    if(m_pDynSpacingCB->IsVisible() && SFX_WHICH_MAX > nWDynSpacing)
    {
        std::unique_ptr<SfxBoolItem> pBoolItem(static_cast<SfxBoolItem*>(pPool->GetDefaultItem(nWDynSpacing).Clone()));
        pBoolItem->SetValue(m_pDynSpacingCB->IsChecked());
        aSet.Put(*pBoolItem);
    }

    // Size
    SvxSizeItem aSizeItem( static_cast<const SvxSizeItem&>(rOldSet.Get( nWSize )) );
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
    if (pBBSet)
    {
        aSet.Put(*pBBSet);
    }
    else
    {
        const SfxPoolItem* pItem;

        if(SfxItemState::SET == GetItemSet().GetItemState(GetWhich(nId), false, &pItem))
        {
            const SfxItemSet* _pSet = &(static_cast< const SvxSetItem* >(pItem)->GetItemSet());

            if(_pSet->GetItemState(nWBrush) == SfxItemState::SET)
            {
                aSet.Put(_pSet->Get(nWBrush));
            }

            if(_pSet->GetItemState(nWBoxInfo) == SfxItemState::SET)
            {
                aSet.Put(_pSet->Get(nWBoxInfo));
            }

            if(_pSet->GetItemState(nWBox) == SfxItemState::SET)
            {
                aSet.Put(_pSet->Get(nWBox));
            }

            if(_pSet->GetItemState(nWShadow) == SfxItemState::SET)
            {
                aSet.Put(_pSet->Get(nWShadow));
            }

            //UUUU take care of [XATTR_XATTR_FILL_FIRST .. XATTR_FILL_LAST]
            for(sal_uInt16 nFillStyleId(XATTR_FILL_FIRST); nFillStyleId <= XATTR_FILL_LAST; nFillStyleId++)
            {
                if(_pSet->GetItemState(nFillStyleId) == SfxItemState::SET)
                {
                    aSet.Put(_pSet->Get(nFillStyleId));
                }
            }
        }
    }

    // Flush the SetItem
    SvxSetItem aSetItem( GetWhich( nId ), aSet );
    rSet->Put( aSetItem );

    return true;
}


void SvxHFPage::Reset( const SfxItemSet* rSet )
{
    ActivatePage( *rSet );
    ResetBackground_Impl( *rSet );

    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Where is the pool" );
    SfxMapUnit eUnit = pPool->GetMetric( GetWhich( SID_ATTR_PAGE_SIZE ) );

    //hide "same content on first page when this is calc
    bool bIsCalc = false;
    const SfxPoolItem* pExt1 = GetItem(*rSet, SID_ATTR_PAGE_EXT1);
    const SfxPoolItem* pExt2 = GetItem(*rSet, SID_ATTR_PAGE_EXT2);
    if (pExt1 && dynamic_cast<const SfxBoolItem*>(pExt1) != nullptr && pExt2 && dynamic_cast<const SfxBoolItem*>(pExt2) != nullptr )
        bIsCalc = true;
    m_pCntSharedFirstBox->Show(!bIsCalc);

    // Evaluate header-/footer- attributes
    const SvxSetItem* pSetItem = nullptr;

    if ( SfxItemState::SET == rSet->GetItemState( GetWhich(nId), false,
                                            reinterpret_cast<const SfxPoolItem**>(&pSetItem) ) )
    {
        const SfxItemSet& rHeaderSet = pSetItem->GetItemSet();
        const SfxBoolItem& rHeaderOn =
            static_cast<const SfxBoolItem&>(rHeaderSet.Get(GetWhich(SID_ATTR_PAGE_ON)));

        m_pTurnOnBox->Check(rHeaderOn.GetValue());

        if ( rHeaderOn.GetValue() )
        {
            const SfxBoolItem& rDynamic =
                static_cast<const SfxBoolItem&>(rHeaderSet.Get( GetWhich( SID_ATTR_PAGE_DYNAMIC ) ));
            const SfxBoolItem& rShared =
                static_cast<const SfxBoolItem&>(rHeaderSet.Get( GetWhich( SID_ATTR_PAGE_SHARED ) ));
            const SfxBoolItem* pSharedFirst = nullptr;
            if (rHeaderSet.HasItem(GetWhich(SID_ATTR_PAGE_SHARED_FIRST)))
                pSharedFirst = static_cast<const SfxBoolItem*>(&rHeaderSet.Get( GetWhich( SID_ATTR_PAGE_SHARED_FIRST ) ));
            const SvxSizeItem& rSize =
                static_cast<const SvxSizeItem&>(rHeaderSet.Get( GetWhich( SID_ATTR_PAGE_SIZE ) ));
            const SvxULSpaceItem& rUL =
                static_cast<const SvxULSpaceItem&>(rHeaderSet.Get( GetWhich( SID_ATTR_ULSPACE ) ));
            const SvxLRSpaceItem& rLR =
                static_cast<const SvxLRSpaceItem&>(rHeaderSet.Get( GetWhich( SID_ATTR_LRSPACE ) ));
            if(m_pDynSpacingCB->IsVisible())
            {
                const SfxBoolItem& rDynSpacing =
                    static_cast<const SfxBoolItem&>(rHeaderSet.Get(GetWhich(SID_ATTR_HDFT_DYNAMIC_SPACING)));
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
            pSetItem = nullptr;
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
        m_pTurnOnBox->Check( false );
        m_pHeightDynBtn->Check();
        m_pCntSharedBox->Check();
        m_pCntSharedFirstBox->Check();
    }

    TurnOnHdl(nullptr);

    m_pTurnOnBox->SaveValue();
    m_pDistEdit->SaveValue();
    m_pHeightEdit->SaveValue();
    m_pHeightDynBtn->SaveValue();
    m_pLMEdit->SaveValue();
    m_pRMEdit->SaveValue();
    m_pCntSharedBox->SaveValue();
    RangeHdl();

    const SfxPoolItem* pItem = nullptr;
    SfxObjectShell* pShell;
    if(SfxItemState::SET == rSet->GetItemState(SID_HTML_MODE, false, &pItem) ||
        ( nullptr != (pShell = SfxObjectShell::Current()) &&
                    nullptr != (pItem = pShell->GetItem(SID_HTML_MODE))))
    {
        sal_uInt16 nHtmlMode = 0;
        nHtmlMode = static_cast<const SfxUInt16Item*>(pItem)->GetValue();
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
    m_pDistEdit->SetLoseFocusHdl(LINK(this, SvxHFPage, RangeFocusHdl));

    m_pHeightEdit->SetModifyHdl(LINK(this,     SvxHFPage, HeightModify));
    m_pHeightEdit->SetLoseFocusHdl(LINK(this,SvxHFPage,RangeFocusHdl));

    m_pLMEdit->SetModifyHdl(LINK(this,         SvxHFPage, BorderModify));
    m_pLMEdit->SetLoseFocusHdl(LINK(this,  SvxHFPage, RangeFocusHdl));
    m_pRMEdit->SetModifyHdl(LINK(this,         SvxHFPage, BorderModify));
    m_pRMEdit->SetLoseFocusHdl(LINK(this,  SvxHFPage, RangeFocusHdl));
    m_pBackgroundBtn->SetClickHdl(LINK(this,SvxHFPage, BackgroundHdl));
}

IMPL_LINK_TYPED( SvxHFPage, TurnOnHdl, Button *, pButton, void )
{
    CheckBox* pBox = static_cast<CheckBox*>(pButton);
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

        if ( !mbDisableQueryBox && pBox && m_pTurnOnBox->GetSavedValue() == TRISTATE_TRUE )
        {
            short nResult;
            if (nId == SID_ATTR_PAGE_HEADERSET)
                nResult = ScopedVclPtrInstance<DeleteHeaderDialog>(this)->Execute();
            else
                nResult = ScopedVclPtrInstance<DeleteFooterDialog>(this)->Execute();
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
}

IMPL_LINK_NOARG_TYPED(SvxHFPage, DistModify, Edit&, void)
{
    UpdateExample();
}

IMPL_LINK_NOARG_TYPED(SvxHFPage, HeightModify, Edit&, void)
{
    UpdateExample();
}

IMPL_LINK_NOARG_TYPED(SvxHFPage, BorderModify, Edit&, void)
{
    UpdateExample();
}

IMPL_LINK_NOARG_TYPED(SvxHFPage, BackgroundHdl, Button*, void)
{
    if(!pBBSet)
    {
        // Use only the necessary items for border and background
        const sal_uInt16 nOuter(GetWhich(SID_ATTR_BORDER_OUTER));
        const sal_uInt16 nInner(GetWhich(SID_ATTR_BORDER_INNER, false));
        const sal_uInt16 nShadow(GetWhich(SID_ATTR_BORDER_SHADOW));

        if(mbEnableDrawingLayerFillStyles)
        {
            pBBSet = new SfxItemSet(
                *GetItemSet().GetPool(),
                XATTR_FILL_FIRST, XATTR_FILL_LAST,  // DrawingLayer FillStyle definitions
                SID_COLOR_TABLE, SID_BITMAP_LIST,   // XPropertyLists for Color, Gradient, Hatch and Graphic fills
                nOuter, nOuter,
                nInner, nInner,
                nShadow, nShadow,
                0, 0);

            //UUUU copy items for XPropertyList entries from the DrawModel so that
            // the Area TabPage can access them
            static const sal_uInt16 nCopyFlags[] = {
                SID_COLOR_TABLE,
                SID_GRADIENT_LIST,
                SID_HATCH_LIST,
                SID_BITMAP_LIST,
                0
            };

            for(sal_uInt16 a(0); nCopyFlags[a]; a++)
            {
                const SfxPoolItem* pItem = GetItemSet().GetItem(nCopyFlags[a]);

                if(pItem)
                {
                    pBBSet->Put(*pItem);
                }
                else
                {
                    OSL_ENSURE(false, "XPropertyList missing (!)");
                }
            }
        }
        else
        {
            const sal_uInt16 nBrush(GetWhich(SID_ATTR_BRUSH));

            pBBSet = new SfxItemSet(
                *GetItemSet().GetPool(),
                nBrush, nBrush,
                nOuter, nOuter,
                nInner, nInner,
                nShadow, nShadow,
                0, 0);
        }

        const SfxPoolItem* pItem;

        if(SfxItemState::SET == GetItemSet().GetItemState(GetWhich(nId), false, &pItem))
        {
            // If a SfxItemSet from the SetItem for SID_ATTR_PAGE_HEADERSET or
            // SID_ATTR_PAGE_FOOTERSET exists, use its content
            pBBSet->Put(static_cast<const SvxSetItem*>(pItem)->GetItemSet());
        }
        else
        {
            if(mbEnableDrawingLayerFillStyles)
            {
                //UUUU The style for header/footer is not yet created, need to reset
                // XFillStyleItem to drawing::FillStyle_NONE which is the same as in the style
                // initialization. This needs to be done since the pool default for
                // XFillStyleItem is drawing::FillStyle_SOLID
                pBBSet->Put(XFillStyleItem(drawing::FillStyle_NONE));
            }
        }

        if(SfxItemState::SET == GetItemSet().GetItemState(nInner, false, &pItem))
        {
            // The set InfoItem is always required
            pBBSet->Put(*pItem);
        }
    }

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();

    if(pFact)
    {
        //UUUU
        SfxAbstractTabDialog* pDlg = pFact->CreateSvxBorderBackgroundDlg(
            this,
            *pBBSet,
            mbEnableBackgroundSelector,
            mbEnableDrawingLayerFillStyles);

        DBG_ASSERT(pDlg,"Dialog creation failed!");
        if(RET_OK == pDlg->Execute() && pDlg->GetOutputItemSet())
        {
            SfxItemIter aIter(*pDlg->GetOutputItemSet());
            const SfxPoolItem* pItem = aIter.FirstItem();

            while(pItem)
            {
                if(!IsInvalidItem(pItem))
                {
                    pBBSet->Put(*pItem);
                }

                pItem = aIter.NextItem();
            }

            {
                drawinglayer::attribute::SdrAllFillAttributesHelperPtr aFillAttributes;

                if(mbEnableDrawingLayerFillStyles)
                {
                    //UUUU create FillAttributes directly from DrawingLayer FillStyle entries
                    aFillAttributes.reset(new drawinglayer::attribute::SdrAllFillAttributesHelper(*pBBSet));
                }
                else
                {
                    const sal_uInt16 nWhich = GetWhich(SID_ATTR_BRUSH);

                    if(pBBSet->GetItemState(nWhich) == SfxItemState::SET)
                    {
                        //UUUU create FillAttributes from SvxBrushItem
                        const SvxBrushItem& rItem = static_cast< const SvxBrushItem& >(pBBSet->Get(nWhich));
                        SfxItemSet aTempSet(*pBBSet->GetPool(), XATTR_FILL_FIRST, XATTR_FILL_LAST);

                        setSvxBrushItemAsFillAttributesToTargetSet(rItem, aTempSet);
                        aFillAttributes.reset(new drawinglayer::attribute::SdrAllFillAttributesHelper(aTempSet));
                    }
                }

                if(SID_ATTR_PAGE_HEADERSET == nId)
                {
                    //m_pBspWin->SetHdColor(rItem.GetColor());
                    m_pBspWin->setHeaderFillAttributes(aFillAttributes);
                }
                else
                {
                    //m_pBspWin->SetFtColor(rItem.GetColor());
                    m_pBspWin->setFooterFillAttributes(aFillAttributes);
                }
            }

            {
                const sal_uInt16 nWhich = GetWhich(SID_ATTR_BORDER_OUTER);

                if(pBBSet->GetItemState(nWhich) == SfxItemState::SET)
                {
                    const SvxBoxItem& rItem = static_cast<const SvxBoxItem&>(pBBSet->Get(nWhich));

                    if(nId == SID_ATTR_PAGE_HEADERSET)
                        m_pBspWin->SetHdBorder(rItem);
                    else
                        m_pBspWin->SetFtBorder(rItem);
                }
            }

        }

        UpdateExample();
    }
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
    sal_uInt16 nWhich(GetWhich(SID_ATTR_PAGE_HEADERSET));

    if (SfxItemState::SET == rSet.GetItemState(nWhich, false))
    {
        const SvxSetItem& rSetItem = static_cast< const SvxSetItem& >(rSet.Get(nWhich, false));
        const SfxItemSet& rTmpSet = rSetItem.GetItemSet();
        const SfxBoolItem& rOn = static_cast< const SfxBoolItem& >(rTmpSet.Get(GetWhich(SID_ATTR_PAGE_ON)));

        if(rOn.GetValue())
        {
            drawinglayer::attribute::SdrAllFillAttributesHelperPtr aHeaderFillAttributes;

            if(mbEnableDrawingLayerFillStyles)
            {
                //UUUU create FillAttributes directly from DrawingLayer FillStyle entries
                aHeaderFillAttributes.reset(new drawinglayer::attribute::SdrAllFillAttributesHelper(rTmpSet));
            }
            else
            {
                nWhich = GetWhich(SID_ATTR_BRUSH);

                if(SfxItemState::SET == rTmpSet.GetItemState(nWhich))
                {
                    //UUUU create FillAttributes from SvxBrushItem
                    const SvxBrushItem& rItem = static_cast< const SvxBrushItem& >(rTmpSet.Get(nWhich));
                    SfxItemSet aTempSet(*rTmpSet.GetPool(), XATTR_FILL_FIRST, XATTR_FILL_LAST);

                    setSvxBrushItemAsFillAttributesToTargetSet(rItem, aTempSet);
                    aHeaderFillAttributes.reset(new drawinglayer::attribute::SdrAllFillAttributesHelper(aTempSet));
                }
            }

            m_pBspWin->setHeaderFillAttributes(aHeaderFillAttributes);
            nWhich = GetWhich(SID_ATTR_BORDER_OUTER);

            if(rTmpSet.GetItemState(nWhich) == SfxItemState::SET)
            {
                const SvxBoxItem& rItem =
                    static_cast<const SvxBoxItem&>(rTmpSet.Get(nWhich));
                m_pBspWin->SetHdBorder(rItem);
            }
        }
    }

    nWhich = GetWhich(SID_ATTR_PAGE_FOOTERSET);

    if (SfxItemState::SET == rSet.GetItemState(nWhich, false))
    {
        const SvxSetItem& rSetItem = static_cast< const SvxSetItem& >(rSet.Get(nWhich, false));
        const SfxItemSet& rTmpSet = rSetItem.GetItemSet();
        const SfxBoolItem& rOn = static_cast< const SfxBoolItem& >(rTmpSet.Get(GetWhich(SID_ATTR_PAGE_ON)));

        if(rOn.GetValue())
        {
            drawinglayer::attribute::SdrAllFillAttributesHelperPtr aFooterFillAttributes;

            if(mbEnableDrawingLayerFillStyles)
            {
                //UUUU create FillAttributes directly from DrawingLayer FillStyle entries
                aFooterFillAttributes.reset(new drawinglayer::attribute::SdrAllFillAttributesHelper(rTmpSet));
            }
            else
            {
                nWhich = GetWhich(SID_ATTR_BRUSH);

                if(SfxItemState::SET == rTmpSet.GetItemState(nWhich))
                {
                    //UUUU create FillAttributes from SvxBrushItem
                    const SvxBrushItem& rItem = static_cast< const SvxBrushItem& >(rTmpSet.Get(nWhich));
                    SfxItemSet aTempSet(*rTmpSet.GetPool(), XATTR_FILL_FIRST, XATTR_FILL_LAST);

                    setSvxBrushItemAsFillAttributesToTargetSet(rItem, aTempSet);
                    aFooterFillAttributes.reset(new drawinglayer::attribute::SdrAllFillAttributesHelper(aTempSet));
                }
            }

            m_pBspWin->setFooterFillAttributes(aFooterFillAttributes);
            nWhich = GetWhich(SID_ATTR_BORDER_OUTER);

            if(rTmpSet.GetItemState(nWhich) == SfxItemState::SET)
            {
                const SvxBoxItem& rItem = static_cast< const SvxBoxItem& >(rTmpSet.Get(nWhich));
                m_pBspWin->SetFtBorder(rItem);
            }
        }
    }

    drawinglayer::attribute::SdrAllFillAttributesHelperPtr aPageFillAttributes;

    if(mbEnableDrawingLayerFillStyles)
    {
        //UUUU create FillAttributes directly from DrawingLayer FillStyle entries
        aPageFillAttributes.reset(new drawinglayer::attribute::SdrAllFillAttributesHelper(rSet));
    }
    else
    {
        nWhich = GetWhich(SID_ATTR_BRUSH);

        if(rSet.GetItemState(nWhich) >= SfxItemState::DEFAULT)
        {
            //UUUU create FillAttributes from SvxBrushItem
            const SvxBrushItem& rItem = static_cast< const SvxBrushItem& >(rSet.Get(nWhich));
            SfxItemSet aTempSet(*rSet.GetPool(), XATTR_FILL_FIRST, XATTR_FILL_LAST);

            setSvxBrushItemAsFillAttributesToTargetSet(rItem, aTempSet);
            aPageFillAttributes.reset(new drawinglayer::attribute::SdrAllFillAttributesHelper(aTempSet));
        }
    }

    m_pBspWin->setPageFillAttributes(aPageFillAttributes);
    nWhich = GetWhich(SID_ATTR_BORDER_OUTER);

    if(rSet.GetItemState(nWhich) >= SfxItemState::DEFAULT)
    {
        const SvxBoxItem& rItem = static_cast< const SvxBoxItem& >(rSet.Get(nWhich));
        m_pBspWin->SetBorder(rItem);
    }
}

void SvxHFPage::ActivatePage( const SfxItemSet& rSet )
{
    const SfxPoolItem* pItem = GetItem( rSet, SID_ATTR_LRSPACE );

    if ( pItem )
    {
        // Set left and right margins
        const SvxLRSpaceItem& rLRSpace = static_cast<const SvxLRSpaceItem&>(*pItem);

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
        const SvxULSpaceItem& rULSpace = static_cast<const SvxULSpaceItem&>(*pItem);

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
        nUsage = static_cast<const SvxPageItem*>(pItem)->GetPageUsage();

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
        const SvxSizeItem& rSize = static_cast<const SvxSizeItem&>(*pItem);
        // if the size is already swapped (Landscape)
        m_pBspWin->SetSize( rSize.GetSize() );
    }

    // Evaluate Header attribute
    const SvxSetItem* pSetItem = nullptr;

    if ( SfxItemState::SET == rSet.GetItemState( GetWhich( SID_ATTR_PAGE_HEADERSET ),
                                            false,
                                            reinterpret_cast<const SfxPoolItem**>(&pSetItem) ) )
    {
        const SfxItemSet& rHeaderSet = pSetItem->GetItemSet();
        const SfxBoolItem& rHeaderOn =
            static_cast<const SfxBoolItem&>(rHeaderSet.Get( GetWhich( SID_ATTR_PAGE_ON ) ));

        if ( rHeaderOn.GetValue() )
        {
            const SvxSizeItem& rSize = static_cast<const SvxSizeItem&>(
                rHeaderSet.Get( GetWhich( SID_ATTR_PAGE_SIZE ) ));
            const SvxULSpaceItem& rUL = static_cast<const SvxULSpaceItem&>(
                rHeaderSet.Get( GetWhich(SID_ATTR_ULSPACE ) ));
            const SvxLRSpaceItem& rLR = static_cast<const SvxLRSpaceItem&>(
                rHeaderSet.Get( GetWhich( SID_ATTR_LRSPACE ) ));
            long nDist = rUL.GetLower();

            m_pBspWin->SetHdHeight( rSize.GetSize().Height() - nDist );
            m_pBspWin->SetHdDist( nDist );
            m_pBspWin->SetHdLeft( rLR.GetLeft() );
            m_pBspWin->SetHdRight( rLR.GetRight() );
            m_pBspWin->SetHeader( true );
        }
        else
            pSetItem = nullptr;
    }

    if ( !pSetItem )
    {
        m_pBspWin->SetHeader( false );

        if ( SID_ATTR_PAGE_HEADERSET == nId )
        {
            m_pCntSharedBox->Disable();
            m_pCntSharedFirstBox->Disable();
        }
    }
    pSetItem = nullptr;

    if ( SfxItemState::SET == rSet.GetItemState( GetWhich( SID_ATTR_PAGE_FOOTERSET ),
                                            false,
                                            reinterpret_cast<const SfxPoolItem**>(&pSetItem) ) )
    {
        const SfxItemSet& rFooterSet = pSetItem->GetItemSet();
        const SfxBoolItem& rFooterOn =
            static_cast<const SfxBoolItem&>(rFooterSet.Get( GetWhich( SID_ATTR_PAGE_ON ) ));

        if ( rFooterOn.GetValue() )
        {
            const SvxSizeItem& rSize = static_cast<const SvxSizeItem&>(
                rFooterSet.Get( GetWhich( SID_ATTR_PAGE_SIZE ) ));
            const SvxULSpaceItem& rUL = static_cast<const SvxULSpaceItem&>(
                rFooterSet.Get( GetWhich( SID_ATTR_ULSPACE ) ));
            const SvxLRSpaceItem& rLR = static_cast<const SvxLRSpaceItem&>(
                rFooterSet.Get( GetWhich( SID_ATTR_LRSPACE ) ));
            long nDist = rUL.GetUpper();

            m_pBspWin->SetFtHeight( rSize.GetSize().Height() - nDist );
            m_pBspWin->SetFtDist( nDist );
            m_pBspWin->SetFtLeft( rLR.GetLeft() );
            m_pBspWin->SetFtRight( rLR.GetRight() );
            m_pBspWin->SetFooter( true );
        }
        else
            pSetItem = nullptr;
    }

    if ( !pSetItem )
    {
        m_pBspWin->SetFooter( false );

        if ( SID_ATTR_PAGE_FOOTERSET == nId )
        {
            m_pCntSharedBox->Disable();
            m_pCntSharedFirstBox->Disable();
        }
    }

    pItem = GetItem( rSet, SID_ATTR_PAGE_EXT1 );

    if ( pItem && dynamic_cast<const SfxBoolItem*>( pItem) !=  nullptr )
    {
        m_pBspWin->SetTable( true );
        m_pBspWin->SetHorz( static_cast<const SfxBoolItem*>(pItem)->GetValue() );
    }

    pItem = GetItem( rSet, SID_ATTR_PAGE_EXT2 );

    if ( pItem && dynamic_cast<const SfxBoolItem*>( pItem) !=  nullptr )
    {
        m_pBspWin->SetTable( true );
        m_pBspWin->SetVert( static_cast<const SfxBoolItem*>(pItem)->GetValue() );
    }
    ResetBackground_Impl( rSet );
    RangeHdl();
}

SfxTabPage::sfxpg SvxHFPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( _pSet );
    return LEAVE_PAGE;
}

IMPL_LINK_NOARG_TYPED(SvxHFPage, RangeFocusHdl, Control&, void)
{
    RangeHdl();
}
void SvxHFPage::RangeHdl()
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
}

static void lcl_Move(vcl::Window& rWin, sal_Int32 nDiff)
{
    Point aPos(rWin.GetPosPixel());
    aPos.Y() -= nDiff;
    rWin.SetPosPixel(aPos);
}

void SvxHFPage::EnableDynamicSpacing()
{
    m_pDynSpacingCB->Show();
    //move all following controls
    vcl::Window* aMoveWindows[] =
    {
        m_pHeightFT,
        m_pHeightEdit,
        m_pHeightDynBtn,
        m_pBackgroundBtn,
        nullptr
    };
    sal_Int32 nOffset = m_pTurnOnBox->GetPosPixel().Y() - m_pCntSharedBox->GetPosPixel().Y();
    sal_Int32 nIdx = 0;
    while(aMoveWindows[nIdx])
        lcl_Move(*aMoveWindows[nIdx++], nOffset);
}

void SvxHFPage::PageCreated(const SfxAllItemSet &rSet)
{
    //UUUU
    const SfxBoolItem* pSupportDrawingLayerFillStyleItem = rSet.GetItem<SfxBoolItem>(SID_DRAWINGLAYER_FILLSTYLES, false);

    if(pSupportDrawingLayerFillStyleItem)
    {
        const bool bNew(pSupportDrawingLayerFillStyleItem->GetValue());

        EnableDrawingLayerFillStyles(bNew);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
