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

#include <sal/config.h>

#include <o3tl/unit_conversion.hxx>
#include <svl/itemiter.hxx>
#include <sfx2/objsh.hxx>
#include <svx/svxids.hrc>

#include <svl/intitem.hxx>
#include <svtools/unitconv.hxx>

#include <svx/hdft.hxx>
#include <svx/pageitem.hxx>

#include <svx/dlgutil.hxx>
#include <sfx2/htmlmode.hxx>
#include <osl/diagnose.h>

#include <editeng/brushitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/boxitem.hxx>

#include <svx/svxdlg.hxx>
#include <memory>

#include <svx/xdef.hxx>
#include <svx/xfillit0.hxx>
#include <svx/unobrushitemhelper.hxx>

using namespace com::sun::star;

// Word 97 incompatibility (#i19922#)
// #i19922# - tdf#126051 see cui/source/tabpages/page.cxx and sw/source/uibase/sidebar/PageMarginControl.hxx
constexpr tools::Long MINBODY = o3tl::toTwips(1, o3tl::Length::mm); // 1mm in twips rounded

// default distance to Header or footer
const tools::Long DEF_DIST_WRITER = 500;    // 5mm (Writer)
const tools::Long DEF_DIST_CALC = 250;      // 2.5mm (Calc)

const WhichRangesContainer SvxHFPage::pRanges(svl::Items<
    // Support DrawingLayer FillStyles (no real call to below GetRanges()
    // detected, still do the complete transition)
    XATTR_FILL_FIRST,        XATTR_FILL_LAST,

    SID_ATTR_BRUSH,          SID_ATTR_BRUSH,
    SID_ATTR_BORDER_INNER,   SID_ATTR_BORDER_INNER,
    SID_ATTR_BORDER_OUTER,   SID_ATTR_BORDER_OUTER,
    SID_ATTR_BORDER_SHADOW,  SID_ATTR_BORDER_SHADOW,
    SID_ATTR_LRSPACE,        SID_ATTR_LRSPACE,
    SID_ATTR_ULSPACE,        SID_ATTR_ULSPACE,
    SID_ATTR_PAGE_SIZE,      SID_ATTR_PAGE_SIZE,
    SID_ATTR_PAGE_HEADERSET, SID_ATTR_PAGE_HEADERSET,
    SID_ATTR_PAGE_FOOTERSET, SID_ATTR_PAGE_FOOTERSET,
    SID_ATTR_PAGE_ON,        SID_ATTR_PAGE_ON,
    SID_ATTR_PAGE_DYNAMIC,   SID_ATTR_PAGE_DYNAMIC,
    SID_ATTR_PAGE_SHARED,    SID_ATTR_PAGE_SHARED,
    SID_ATTR_HDFT_DYNAMIC_SPACING, SID_ATTR_HDFT_DYNAMIC_SPACING,
    SID_ATTR_PAGE_SHARED_FIRST,    SID_ATTR_PAGE_SHARED_FIRST
>);

namespace svx {

    bool ShowBorderBackgroundDlg(weld::Window* pParent, SfxItemSet* pBBSet)
    {
        bool bRes = false;
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        ScopedVclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateSvxBorderBackgroundDlg(pParent, *pBBSet, true /*bEnableDrawingLayerFillStyles*/));
        if ( pDlg->Execute() == RET_OK && pDlg->GetOutputItemSet() )
        {
            SfxItemIter aIter( *pDlg->GetOutputItemSet() );

            for (const SfxPoolItem* pItem = aIter.GetCurItem(); pItem; pItem = aIter.NextItem())
            {
                if ( !IsInvalidItem( pItem ) )
                    pBBSet->Put( *pItem );
            }
            bRes = true;
        }
        return bRes;
    }
}

std::unique_ptr<SfxTabPage> SvxHeaderPage::Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet )
{
    return std::make_unique<SvxHeaderPage>( pPage, pController, *rSet );
}

std::unique_ptr<SfxTabPage> SvxFooterPage::Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet )
{
    return std::make_unique<SvxFooterPage>( pPage, pController, *rSet );
}

SvxHeaderPage::SvxHeaderPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rAttr)
    : SvxHFPage( pPage, pController, rAttr, SID_ATTR_PAGE_HEADERSET )
{
}

SvxFooterPage::SvxFooterPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rAttr)
    : SvxHFPage( pPage, pController, rAttr, SID_ATTR_PAGE_FOOTERSET )
{
}

SvxHFPage::SvxHFPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet, sal_uInt16 nSetId)
    : SfxTabPage(pPage, pController, u"svx/ui/headfootformatpage.ui"_ustr, u"HFFormatPage"_ustr, &rSet)
    , nId(nSetId)
    , mbDisableQueryBox(false)
    , mbEnableDrawingLayerFillStyles(false)
    , m_xCntSharedBox(m_xBuilder->weld_check_button(u"checkSameLR"_ustr))
    , m_xCntSharedFirstBox(m_xBuilder->weld_check_button(u"checkSameFP"_ustr))
    , m_xLMLbl(m_xBuilder->weld_label(u"labelLeftMarg"_ustr))
    , m_xLMEdit(m_xBuilder->weld_metric_spin_button(u"spinMargLeft"_ustr, FieldUnit::CM))
    , m_xRMLbl(m_xBuilder->weld_label(u"labelRightMarg"_ustr))
    , m_xRMEdit(m_xBuilder->weld_metric_spin_button(u"spinMargRight"_ustr, FieldUnit::CM))
    , m_xDistFT(m_xBuilder->weld_label(u"labelSpacing"_ustr))
    , m_xDistEdit(m_xBuilder->weld_metric_spin_button(u"spinSpacing"_ustr, FieldUnit::CM))
    , m_xDynSpacingCB(m_xBuilder->weld_check_button(u"checkDynSpacing"_ustr))
    , m_xHeightFT(m_xBuilder->weld_label(u"labelHeight"_ustr))
    , m_xHeightEdit(m_xBuilder->weld_metric_spin_button(u"spinHeight"_ustr, FieldUnit::CM))
    , m_xHeightDynBtn(m_xBuilder->weld_check_button(u"checkAutofit"_ustr))
    , m_xBackgroundBtn(m_xBuilder->weld_button(u"buttonMore"_ustr))
    , m_xBspWin(new weld::CustomWeld(*m_xBuilder, u"drawingareaPageHF"_ustr, m_aBspWin))
{
    //swap header <-> footer in UI
    if (nId == SID_ATTR_PAGE_FOOTERSET)
    {
        m_xContainer->set_help_id(u"svx/ui/headfootformatpage/FFormatPage"_ustr);
        m_xPageLbl = m_xBuilder->weld_label(u"labelFooterFormat"_ustr);
        m_xTurnOnBox = m_xBuilder->weld_check_button(u"checkFooterOn"_ustr);

        /* Set custom HIDs for the Footer help page (shared/01/05040400.xhp)
        otherwise it would display the same extended help
        on both the Header and Footer tabs */
        m_xCntSharedBox->set_help_id( u"SVX_HID_FOOTER_CHECKSAMELR"_ustr );
        m_xCntSharedFirstBox->set_help_id( u"SVX_HID_FOOTER_CHECKSAMEFP"_ustr );
        m_xLMEdit->set_help_id( u"SVX_HID_FOOTER_SPINMARGLEFT"_ustr );
        m_xRMEdit->set_help_id( u"SVX_HID_FOOTER_SPINMARGRIGHT"_ustr );
        m_xDistEdit->set_help_id( u"SVX_HID_FOOTER_SPINSPACING"_ustr );
        m_xDynSpacingCB->set_help_id( u"SVX_HID_FOOTER_CHECKDYNSPACING"_ustr );
        m_xHeightEdit->set_help_id( u"SVX_HID_FOOTER_SPINHEIGHT"_ustr );
        m_xHeightDynBtn->set_help_id( u"SVX_HID_FOOTER_CHECKAUTOFIT"_ustr );
        m_xBackgroundBtn->set_help_id( u"SVX_HID_FOOTER_BUTTONMORE"_ustr );
    }
    else //Header
    {
        m_xContainer->set_help_id(u"svx/ui/headfootformatpage/HFormatPage"_ustr);
        m_xPageLbl = m_xBuilder->weld_label(u"labelHeaderFormat"_ustr);
        m_xTurnOnBox = m_xBuilder->weld_check_button(u"checkHeaderOn"_ustr);
    }
    m_xTurnOnBox->show();
    m_xPageLbl->show();

    InitHandler();
    m_aBspWin.EnableRTL(false);

    // This Page needs ExchangeSupport
    SetExchangeSupport();

    // Set metrics
    FieldUnit eFUnit = GetModuleFieldUnit( rSet );
    SetFieldUnit( *m_xDistEdit, eFUnit );
    SetFieldUnit( *m_xHeightEdit, eFUnit );
    SetFieldUnit( *m_xLMEdit, eFUnit );
    SetFieldUnit( *m_xRMEdit, eFUnit );
}

SvxHFPage::~SvxHFPage()
{
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

    const SfxItemSet& rOldSet = GetItemSet();
    SfxItemPool* pPool = rOldSet.GetPool();
    DBG_ASSERT(pPool,"no pool :-(");
    MapUnit eUnit = pPool->GetMetric(nWSize);
    // take over DrawingLayer FillStyles
    SfxItemSetFixed<XATTR_FILL_FIRST, XATTR_FILL_LAST> aSet(*pPool);
    // Keep it valid
    aSet.MergeRange(nWSize, nWSize);
    aSet.MergeRange(nWLRSpace, nWLRSpace);
    aSet.MergeRange(nWULSpace, nWULSpace);
    aSet.MergeRange(nWOn, nWOn);
    aSet.MergeRange(nWDynamic, nWDynamic);
    aSet.MergeRange(nWShared, nWShared);
    aSet.MergeRange(nWSharedFirst, nWSharedFirst);
    aSet.MergeRange(nWBrush, nWBrush);
    aSet.MergeRange(nWBoxInfo, nWBoxInfo);
    aSet.MergeRange(nWBox, nWBox);
    aSet.MergeRange(nWShadow, nWShadow);
    aSet.MergeRange(nWDynSpacing, nWDynSpacing);

    if(mbEnableDrawingLayerFillStyles)
    {
        // When using the XATTR_FILLSTYLE DrawingLayer FillStyle definition
        // extra action has to be done here since the pool default is drawing::FillStyle_SOLID
        // instead of drawing::FillStyle_NONE (to have the default blue fill color at start).
        aSet.Put(XFillStyleItem(drawing::FillStyle_NONE));
    }

    aSet.Put( SfxBoolItem( nWOn,      m_xTurnOnBox->get_active() ) );
    aSet.Put( SfxBoolItem( nWDynamic, m_xHeightDynBtn->get_active() ) );
    aSet.Put( SfxBoolItem( nWShared,  m_xCntSharedBox->get_active() ) );
    if(m_xCntSharedFirstBox->get_visible())
        aSet.Put(SfxBoolItem(nWSharedFirst,  m_xCntSharedFirstBox->get_active()));
    if (m_xDynSpacingCB->get_visible() && SfxItemPool::IsWhich(nWDynSpacing))
    {
        std::unique_ptr<SfxBoolItem> pBoolItem(static_cast<SfxBoolItem*>(pPool->GetUserOrPoolDefaultItem(nWDynSpacing).Clone()));
        pBoolItem->SetValue(m_xDynSpacingCB->get_active());
        aSet.Put(std::move(pBoolItem));
    }

    // Size
    SvxSizeItem aSizeItem( static_cast<const SvxSizeItem&>(rOldSet.Get( nWSize )) );
    Size        aSize( aSizeItem.GetSize() );
    tools::Long        nDist = GetCoreValue( *m_xDistEdit, eUnit );
    tools::Long        nH    = GetCoreValue( *m_xHeightEdit, eUnit );

    nH += nDist; // add distance
    aSize.setHeight( nH );
    aSizeItem.SetSize( aSize );
    aSet.Put( aSizeItem );

    // Margins
    SvxLRSpaceItem aLR( nWLRSpace );
    aLR.SetLeft( static_cast<sal_uInt16>(GetCoreValue( *m_xLMEdit, eUnit )) );
    aLR.SetRight( static_cast<sal_uInt16>(GetCoreValue( *m_xRMEdit, eUnit )) );
    aSet.Put( aLR );

    SvxULSpaceItem aUL( nWULSpace );
    if ( nId == SID_ATTR_PAGE_HEADERSET )
        aUL.SetLower( static_cast<sal_uInt16>(nDist) );
    else
        aUL.SetUpper( static_cast<sal_uInt16>(nDist) );
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

            // take care of [XATTR_XATTR_FILL_FIRST .. XATTR_FILL_LAST]
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
    SvxSetItem aSetItem( TypedWhichId<SvxSetItem>(GetWhich( nId )), aSet );
    rSet->Put( aSetItem );

    return true;
}


void SvxHFPage::Reset( const SfxItemSet* rSet )
{
    ActivatePage( *rSet );
    ResetBackground_Impl( *rSet );

    SfxItemPool* pPool = GetItemSet().GetPool();
    assert(pPool && "Where is the pool");
    MapUnit eUnit = pPool->GetMetric( GetWhich( SID_ATTR_PAGE_SIZE ) );

    // Evaluate header-/footer- attributes
    const SvxSetItem* pSetItem = nullptr;

    if ( SfxItemState::SET == rSet->GetItemState( GetWhich(nId), false,
                                            reinterpret_cast<const SfxPoolItem**>(&pSetItem) ) )
    {
        const SfxItemSet& rHeaderSet = pSetItem->GetItemSet();
        const SfxBoolItem& rHeaderOn =
            rHeaderSet.Get(GetWhich(SID_ATTR_PAGE_ON));

        m_xTurnOnBox->set_active(rHeaderOn.GetValue());

        if ( rHeaderOn.GetValue() )
        {
            const SfxBoolItem& rDynamic =
                rHeaderSet.Get( GetWhich( SID_ATTR_PAGE_DYNAMIC ) );
            const SfxBoolItem& rShared =
                rHeaderSet.Get( GetWhich( SID_ATTR_PAGE_SHARED ) );
            const SfxBoolItem* pSharedFirst = nullptr;
            if (rHeaderSet.HasItem(GetWhich(SID_ATTR_PAGE_SHARED_FIRST)))
                pSharedFirst = static_cast<const SfxBoolItem*>(&rHeaderSet.Get( GetWhich( SID_ATTR_PAGE_SHARED_FIRST ) ));
            const SvxSizeItem& rSize =
                rHeaderSet.Get( GetWhich( SID_ATTR_PAGE_SIZE ) );
            const SvxULSpaceItem& rUL = rHeaderSet.Get( GetWhich( SID_ATTR_ULSPACE ) );
            const SvxLRSpaceItem& rLR = rHeaderSet.Get( GetWhich( SID_ATTR_LRSPACE ) );
            if (m_xDynSpacingCB->get_visible())
            {
                const SfxBoolItem& rDynSpacing =
                    static_cast<const SfxBoolItem&>(rHeaderSet.Get(GetWhich(SID_ATTR_HDFT_DYNAMIC_SPACING)));
                m_xDynSpacingCB->set_active(rDynSpacing.GetValue());
            }


            if ( nId == SID_ATTR_PAGE_HEADERSET )
            {   // Header
                SetMetricValue( *m_xDistEdit, rUL.GetLower(), eUnit );
                SetMetricValue( *m_xHeightEdit, rSize.GetSize().Height() - rUL.GetLower(), eUnit );
            }
            else
            {   // Footer
                SetMetricValue( *m_xDistEdit, rUL.GetUpper(), eUnit );
                SetMetricValue( *m_xHeightEdit, rSize.GetSize().Height() - rUL.GetUpper(), eUnit );
            }

            m_xHeightDynBtn->set_active(rDynamic.GetValue());
            SetMetricValue( *m_xLMEdit, rLR.GetLeft(), eUnit );
            SetMetricValue( *m_xRMEdit, rLR.GetRight(), eUnit );
            m_xCntSharedBox->set_active(rShared.GetValue());
            if (pSharedFirst)
                m_xCntSharedFirstBox->set_active(pSharedFirst->GetValue());
        }
        else
            pSetItem = nullptr;
    }
    else
    {
        bool bIsCalc = false;
        const SfxPoolItem* pExt1 = GetItem(*rSet, SID_ATTR_PAGE_EXT1);
        const SfxPoolItem* pExt2 = GetItem(*rSet, SID_ATTR_PAGE_EXT2);
        if (dynamic_cast<const SfxBoolItem*>(pExt1) && dynamic_cast<const SfxBoolItem*>(pExt2) )
            bIsCalc = true;

        // defaults for distance and height
        tools::Long nDefaultDist = bIsCalc ? DEF_DIST_CALC : DEF_DIST_WRITER;
        SetMetricValue( *m_xDistEdit, nDefaultDist, MapUnit::Map100thMM );
        SetMetricValue( *m_xHeightEdit, 500, MapUnit::Map100thMM );
    }

    if ( !pSetItem )
    {
        m_xTurnOnBox->set_active(false);
        m_xHeightDynBtn->set_active(true);
        m_xCntSharedBox->set_active(true);
        m_xCntSharedFirstBox->set_active(true);
    }

    TurnOn(nullptr);

    m_xTurnOnBox->save_state();
    m_xDistEdit->save_value();
    m_xHeightEdit->save_value();
    m_xHeightDynBtn->save_state();
    m_xLMEdit->save_value();
    m_xRMEdit->save_value();
    m_xCntSharedBox->save_state();
    RangeHdl();

    SfxObjectShell* pShell;
    const SfxUInt16Item* pItem = rSet->GetItemIfSet(SID_HTML_MODE, false);
    if(pItem  ||
        ( nullptr != (pShell = SfxObjectShell::Current()) &&
                    nullptr != (pItem = pShell->GetItem(SID_HTML_MODE))))
    {
        sal_uInt16 nHtmlMode = pItem->GetValue();
        if (nHtmlMode & HTMLMODE_ON)
        {
            m_xCntSharedBox->hide();
            m_xBackgroundBtn->hide();
        }
    }

}

void SvxHFPage::InitHandler()
{
    m_xTurnOnBox->connect_toggled(LINK(this, SvxHFPage, TurnOnHdl));
    m_xDistEdit->connect_value_changed(LINK(this, SvxHFPage, ValueChangeHdl));
    m_xHeightEdit->connect_value_changed(LINK(this,SvxHFPage,ValueChangeHdl));

    m_xLMEdit->connect_value_changed(LINK(this,  SvxHFPage, ValueChangeHdl));
    m_xRMEdit->connect_value_changed(LINK(this,  SvxHFPage, ValueChangeHdl));
    m_xBackgroundBtn->connect_clicked(LINK(this,SvxHFPage, BackgroundHdl));
}

void SvxHFPage::TurnOn(const weld::Toggleable* pBox)
{
    if (m_xTurnOnBox->get_active())
    {
        m_xDistFT->set_sensitive(true);
        m_xDistEdit->set_sensitive(true);
        m_xDynSpacingCB->set_sensitive(true);
        m_xHeightFT->set_sensitive(true);
        m_xHeightEdit->set_sensitive(true);
        m_xHeightDynBtn->set_sensitive(true);
        m_xLMLbl->set_sensitive(true);
        m_xLMEdit->set_sensitive(true);
        m_xRMLbl->set_sensitive(true);
        m_xRMEdit->set_sensitive(true);

        SvxPageUsage nUsage = m_aBspWin.GetUsage();

        if( nUsage == SvxPageUsage::Right || nUsage == SvxPageUsage::Left )
            m_xCntSharedBox->set_sensitive(false);
        else
        {
            m_xCntSharedBox->set_sensitive(true);
            m_xCntSharedFirstBox->set_sensitive(true);
        }
        m_xBackgroundBtn->set_sensitive(true);
    }
    else
    {
        bool bDelete = true;

        if (!mbDisableQueryBox && pBox && m_xTurnOnBox->get_saved_state() == TRISTATE_TRUE)
        {
            short nResult;
            if (nId == SID_ATTR_PAGE_HEADERSET)
            {
                DeleteHeaderDialog aDlg(GetFrameWeld());
                nResult = aDlg.run();
            }
            else
            {
                DeleteFooterDialog aDlg(GetFrameWeld());
                nResult = aDlg.run();
            }
            bDelete = nResult == RET_YES;
        }

        if ( bDelete )
        {
            m_xDistFT->set_sensitive(false);
            m_xDistEdit->set_sensitive(false);
            m_xDynSpacingCB->set_sensitive(false);
            m_xHeightFT->set_sensitive(false);
            m_xHeightEdit->set_sensitive(false);
            m_xHeightDynBtn->set_sensitive(false);

            m_xLMLbl->set_sensitive(false);
            m_xLMEdit->set_sensitive(false);
            m_xRMLbl->set_sensitive(false);
            m_xRMEdit->set_sensitive(false);

            m_xCntSharedBox->set_sensitive(false);
            m_xBackgroundBtn->set_sensitive(false);
            m_xCntSharedFirstBox->set_sensitive(false);
        }
        else
            m_xTurnOnBox->set_active(true);
    }
    UpdateExample();
}

IMPL_LINK(SvxHFPage, TurnOnHdl, weld::Toggleable&, rBox, void)
{
    TurnOn(&rBox);
}

IMPL_LINK_NOARG(SvxHFPage, BackgroundHdl, weld::Button&, void)
{
    if(!pBBSet)
    {
        // Use only the necessary items for border and background
        const sal_uInt16 nOuter(GetWhich(SID_ATTR_BORDER_OUTER));
        const sal_uInt16 nInner(GetWhich(SID_ATTR_BORDER_INNER, false));
        const sal_uInt16 nShadow(GetWhich(SID_ATTR_BORDER_SHADOW));

        if(mbEnableDrawingLayerFillStyles)
        {
            pBBSet.reset(new SfxItemSetFixed
                            <XATTR_FILL_FIRST, XATTR_FILL_LAST,      // DrawingLayer FillStyle definitions
                             SID_COLOR_TABLE, SID_PATTERN_LIST> // XPropertyLists for Color, Gradient, Hatch and Graphic fills
                            (*GetItemSet().GetPool()));
            // Keep it valid
            pBBSet->MergeRange(nOuter, nOuter);
            pBBSet->MergeRange(nInner, nInner);
            pBBSet->MergeRange(nShadow, nShadow);

            // copy items for XPropertyList entries from the DrawModel so that
            // the Area TabPage can access them
            static const sal_uInt16 nCopyFlags[] = {
                SID_COLOR_TABLE,
                SID_GRADIENT_LIST,
                SID_HATCH_LIST,
                SID_BITMAP_LIST,
                SID_PATTERN_LIST,
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

            pBBSet.reset( new SfxItemSetFixed<XATTR_FILL_FIRST, XATTR_FILL_LAST>
                            (*GetItemSet().GetPool()) );
            // Keep it valid
            pBBSet->MergeRange(nBrush, nBrush);
            pBBSet->MergeRange(nOuter, nOuter);
            pBBSet->MergeRange(nInner, nInner);
            pBBSet->MergeRange(nShadow, nShadow);
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
                // The style for header/footer is not yet created, need to reset
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

    VclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateSvxBorderBackgroundDlg(
        GetFrameWeld(),
        *pBBSet,
        mbEnableDrawingLayerFillStyles));

    pDlg->StartExecuteAsync([pDlg, this](sal_Int32 nResult) {
        if (nResult == RET_OK && pDlg->GetOutputItemSet())
        {
            SfxItemIter aIter(*pDlg->GetOutputItemSet());

            for (const SfxPoolItem* pItem = aIter.GetCurItem(); pItem; pItem = aIter.NextItem())
            {
                if(!IsInvalidItem(pItem))
                {
                    pBBSet->Put(*pItem);
                }
            }

            {
                drawinglayer::attribute::SdrAllFillAttributesHelperPtr aFillAttributes;

                if (mbEnableDrawingLayerFillStyles)
                {
                    // create FillAttributes directly from DrawingLayer FillStyle entries
                    aFillAttributes =
                        std::make_shared<drawinglayer::attribute::SdrAllFillAttributesHelper>(*pBBSet);
                }
                else
                {
                    const sal_uInt16 nWhich = GetWhich(SID_ATTR_BRUSH);

                    if (pBBSet->GetItemState(nWhich) == SfxItemState::SET)
                    {
                        // create FillAttributes from SvxBrushItem
                        const SvxBrushItem& rItem
                            = static_cast<const SvxBrushItem&>(pBBSet->Get(nWhich));
                        SfxItemSetFixed<XATTR_FILL_FIRST, XATTR_FILL_LAST> aTempSet(*pBBSet->GetPool());

                        setSvxBrushItemAsFillAttributesToTargetSet(rItem, aTempSet);
                        aFillAttributes =
                            std::make_shared<drawinglayer::attribute::SdrAllFillAttributesHelper>(aTempSet);
                    }
                }

                if (SID_ATTR_PAGE_HEADERSET == nId)
                {
                    //m_aBspWin.SetHdColor(rItem.GetColor());
                    m_aBspWin.setHeaderFillAttributes(aFillAttributes);
                }
                else
                {
                    //m_aBspWin.SetFtColor(rItem.GetColor());
                    m_aBspWin.setFooterFillAttributes(aFillAttributes);
                }
            }
        }
        pDlg->disposeOnce();
    });

    UpdateExample();
}

void SvxHFPage::UpdateExample()
{
    if ( nId == SID_ATTR_PAGE_HEADERSET )
    {
        m_aBspWin.SetHeader( m_xTurnOnBox->get_active() );
        m_aBspWin.SetHdHeight( GetCoreValue( *m_xHeightEdit, MapUnit::MapTwip ) );
        m_aBspWin.SetHdDist( GetCoreValue( *m_xDistEdit, MapUnit::MapTwip ) );
        m_aBspWin.SetHdLeft( GetCoreValue( *m_xLMEdit, MapUnit::MapTwip ) );
        m_aBspWin.SetHdRight( GetCoreValue( *m_xRMEdit, MapUnit::MapTwip ) );
    }
    else
    {
        m_aBspWin.SetFooter( m_xTurnOnBox->get_active() );
        m_aBspWin.SetFtHeight( GetCoreValue( *m_xHeightEdit, MapUnit::MapTwip ) );
        m_aBspWin.SetFtDist( GetCoreValue( *m_xDistEdit, MapUnit::MapTwip ) );
        m_aBspWin.SetFtLeft( GetCoreValue( *m_xLMEdit, MapUnit::MapTwip ) );
        m_aBspWin.SetFtRight( GetCoreValue( *m_xRMEdit, MapUnit::MapTwip ) );
    }
    m_aBspWin.Invalidate();
}

void SvxHFPage::ResetBackground_Impl( const SfxItemSet& rSet )
{
    sal_uInt16 nWhich(GetWhich(SID_ATTR_PAGE_HEADERSET));

    if (SfxItemState::SET == rSet.GetItemState(nWhich, false))
    {
        const SvxSetItem& rSetItem = static_cast< const SvxSetItem& >(rSet.Get(nWhich, false));
        const SfxItemSet& rTmpSet = rSetItem.GetItemSet();
        const SfxBoolItem& rOn = rTmpSet.Get(GetWhich(SID_ATTR_PAGE_ON));

        if(rOn.GetValue())
        {
            drawinglayer::attribute::SdrAllFillAttributesHelperPtr aHeaderFillAttributes;

            if(mbEnableDrawingLayerFillStyles)
            {
                // create FillAttributes directly from DrawingLayer FillStyle entries
                aHeaderFillAttributes = std::make_shared<drawinglayer::attribute::SdrAllFillAttributesHelper>(rTmpSet);
            }
            else
            {
                nWhich = GetWhich(SID_ATTR_BRUSH);

                if(SfxItemState::SET == rTmpSet.GetItemState(nWhich))
                {
                    // create FillAttributes from SvxBrushItem
                    const SvxBrushItem& rItem = static_cast< const SvxBrushItem& >(rTmpSet.Get(nWhich));
                    SfxItemSetFixed<XATTR_FILL_FIRST, XATTR_FILL_LAST> aTempSet(*rTmpSet.GetPool());

                    setSvxBrushItemAsFillAttributesToTargetSet(rItem, aTempSet);
                    aHeaderFillAttributes = std::make_shared<drawinglayer::attribute::SdrAllFillAttributesHelper>(aTempSet);
                }
            }

            m_aBspWin.setHeaderFillAttributes(aHeaderFillAttributes);
        }
    }

    nWhich = GetWhich(SID_ATTR_PAGE_FOOTERSET);

    if (SfxItemState::SET == rSet.GetItemState(nWhich, false))
    {
        const SvxSetItem& rSetItem = static_cast< const SvxSetItem& >(rSet.Get(nWhich, false));
        const SfxItemSet& rTmpSet = rSetItem.GetItemSet();
        const SfxBoolItem& rOn = rTmpSet.Get(GetWhich(SID_ATTR_PAGE_ON));

        if(rOn.GetValue())
        {
            drawinglayer::attribute::SdrAllFillAttributesHelperPtr aFooterFillAttributes;

            if(mbEnableDrawingLayerFillStyles)
            {
                // create FillAttributes directly from DrawingLayer FillStyle entries
                aFooterFillAttributes = std::make_shared<drawinglayer::attribute::SdrAllFillAttributesHelper>(rTmpSet);
            }
            else
            {
                nWhich = GetWhich(SID_ATTR_BRUSH);

                if(SfxItemState::SET == rTmpSet.GetItemState(nWhich))
                {
                    // create FillAttributes from SvxBrushItem
                    const SvxBrushItem& rItem = static_cast< const SvxBrushItem& >(rTmpSet.Get(nWhich));
                    SfxItemSetFixed<XATTR_FILL_FIRST, XATTR_FILL_LAST> aTempSet(*rTmpSet.GetPool());

                    setSvxBrushItemAsFillAttributesToTargetSet(rItem, aTempSet);
                    aFooterFillAttributes = std::make_shared<drawinglayer::attribute::SdrAllFillAttributesHelper>(aTempSet);
                }
            }

            m_aBspWin.setFooterFillAttributes(aFooterFillAttributes);
        }
    }

    drawinglayer::attribute::SdrAllFillAttributesHelperPtr aPageFillAttributes;

    if(mbEnableDrawingLayerFillStyles)
    {
        // create FillAttributes directly from DrawingLayer FillStyle entries
        aPageFillAttributes = std::make_shared<drawinglayer::attribute::SdrAllFillAttributesHelper>(rSet);
    }
    else
    {
        nWhich = GetWhich(SID_ATTR_BRUSH);

        if(rSet.GetItemState(nWhich) >= SfxItemState::DEFAULT)
        {
            // create FillAttributes from SvxBrushItem
            const SvxBrushItem& rItem = static_cast< const SvxBrushItem& >(rSet.Get(nWhich));
            SfxItemSetFixed<XATTR_FILL_FIRST, XATTR_FILL_LAST> aTempSet(*rSet.GetPool());

            setSvxBrushItemAsFillAttributesToTargetSet(rItem, aTempSet);
            aPageFillAttributes = std::make_shared<drawinglayer::attribute::SdrAllFillAttributesHelper>(aTempSet);
        }
    }

    m_aBspWin.setPageFillAttributes(aPageFillAttributes);
}

void SvxHFPage::ActivatePage( const SfxItemSet& rSet )
{
    const SfxPoolItem* pItem = GetItem( rSet, SID_ATTR_LRSPACE );

    if ( pItem )
    {
        // Set left and right margins
        const SvxLRSpaceItem& rLRSpace = static_cast<const SvxLRSpaceItem&>(*pItem);

        m_aBspWin.SetLeft( rLRSpace.GetLeft() );
        m_aBspWin.SetRight( rLRSpace.GetRight() );
    }
    else
    {
        m_aBspWin.SetLeft( 0 );
        m_aBspWin.SetRight( 0 );
    }

    pItem = GetItem( rSet, SID_ATTR_ULSPACE );

    if ( pItem )
    {
        // Set top and bottom margins
        const SvxULSpaceItem& rULSpace = static_cast<const SvxULSpaceItem&>(*pItem);

        m_aBspWin.SetTop( rULSpace.GetUpper() );
        m_aBspWin.SetBottom( rULSpace.GetLower() );
    }
    else
    {
        m_aBspWin.SetTop( 0 );
        m_aBspWin.SetBottom( 0 );
    }

    SvxPageUsage nUsage = SvxPageUsage::All;
    pItem = GetItem( rSet, SID_ATTR_PAGE );

    if ( pItem )
        nUsage = static_cast<const SvxPageItem*>(pItem)->GetPageUsage();

    m_aBspWin.SetUsage( nUsage );

    if ( SvxPageUsage::Right == nUsage || SvxPageUsage::Left == nUsage )
        m_xCntSharedBox->set_sensitive(false);
    else
    {
        m_xCntSharedBox->set_sensitive(true);
        m_xCntSharedFirstBox->set_sensitive(true);
    }
    pItem = GetItem( rSet, SID_ATTR_PAGE_SIZE );

    if ( pItem )
    {
        // Orientation and Size from the PageItem
        const SvxSizeItem& rSize = static_cast<const SvxSizeItem&>(*pItem);
        // if the size is already swapped (Landscape)
        m_aBspWin.SetSize( rSize.GetSize() );
    }

    // Evaluate Header attribute
    const SvxSetItem* pSetItem = nullptr;

    if ( SfxItemState::SET == rSet.GetItemState( GetWhich( SID_ATTR_PAGE_HEADERSET ),
                                            false,
                                            reinterpret_cast<const SfxPoolItem**>(&pSetItem) ) )
    {
        const SfxItemSet& rHeaderSet = pSetItem->GetItemSet();
        const SfxBoolItem& rHeaderOn =
            rHeaderSet.Get( GetWhich( SID_ATTR_PAGE_ON ) );

        if ( rHeaderOn.GetValue() )
        {
            const SvxSizeItem& rSize =
                rHeaderSet.Get( GetWhich( SID_ATTR_PAGE_SIZE ) );
            const SvxULSpaceItem& rUL = rHeaderSet.Get( GetWhich(SID_ATTR_ULSPACE ) );
            const SvxLRSpaceItem& rLR = rHeaderSet.Get( GetWhich( SID_ATTR_LRSPACE ) );
            tools::Long nDist = rUL.GetLower();

            m_aBspWin.SetHdHeight( rSize.GetSize().Height() - nDist );
            m_aBspWin.SetHdDist( nDist );
            m_aBspWin.SetHdLeft( rLR.GetLeft() );
            m_aBspWin.SetHdRight( rLR.GetRight() );
            m_aBspWin.SetHeader( true );
        }
        else
            pSetItem = nullptr;
    }

    if ( !pSetItem )
    {
        m_aBspWin.SetHeader( false );

        if ( SID_ATTR_PAGE_HEADERSET == nId )
        {
            m_xCntSharedBox->set_sensitive(false);
            m_xCntSharedFirstBox->set_sensitive(false);
        }
    }
    pSetItem = nullptr;

    if ( SfxItemState::SET == rSet.GetItemState( GetWhich( SID_ATTR_PAGE_FOOTERSET ),
                                            false,
                                            reinterpret_cast<const SfxPoolItem**>(&pSetItem) ) )
    {
        const SfxItemSet& rFooterSet = pSetItem->GetItemSet();
        const SfxBoolItem& rFooterOn =
            rFooterSet.Get( GetWhich( SID_ATTR_PAGE_ON ) );

        if ( rFooterOn.GetValue() )
        {
            const SvxSizeItem& rSize =
                rFooterSet.Get( GetWhich( SID_ATTR_PAGE_SIZE ) );
            const SvxULSpaceItem& rUL = rFooterSet.Get( GetWhich( SID_ATTR_ULSPACE ) );
            const SvxLRSpaceItem& rLR = rFooterSet.Get( GetWhich( SID_ATTR_LRSPACE ) );
            tools::Long nDist = rUL.GetUpper();

            m_aBspWin.SetFtHeight( rSize.GetSize().Height() - nDist );
            m_aBspWin.SetFtDist( nDist );
            m_aBspWin.SetFtLeft( rLR.GetLeft() );
            m_aBspWin.SetFtRight( rLR.GetRight() );
            m_aBspWin.SetFooter( true );
        }
        else
            pSetItem = nullptr;
    }

    if ( !pSetItem )
    {
        m_aBspWin.SetFooter( false );

        if ( SID_ATTR_PAGE_FOOTERSET == nId )
        {
            m_xCntSharedBox->set_sensitive(false);
            m_xCntSharedFirstBox->set_sensitive(false);
        }
    }

    pItem = GetItem( rSet, SID_ATTR_PAGE_EXT1 );

    if ( auto pBoolItem = dynamic_cast<const SfxBoolItem*>( pItem) )
    {
        m_aBspWin.SetTable( true );
        m_aBspWin.SetHorz( pBoolItem->GetValue() );
    }

    pItem = GetItem( rSet, SID_ATTR_PAGE_EXT2 );

    if ( auto pBoolItem = dynamic_cast<const SfxBoolItem*>( pItem) )
    {
        m_aBspWin.SetTable( true );
        m_aBspWin.SetVert( pBoolItem->GetValue() );
    }
    ResetBackground_Impl( rSet );
    RangeHdl();
}

DeactivateRC SvxHFPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( _pSet );
    return DeactivateRC::LeavePage;
}

IMPL_LINK_NOARG(SvxHFPage, ValueChangeHdl, weld::MetricSpinButton&, void)
{
    UpdateExample();
    RangeHdl();
}

void SvxHFPage::RangeHdl()
{
    tools::Long nHHeight = m_aBspWin.GetHdHeight();
    tools::Long nHDist   = m_aBspWin.GetHdDist();

    tools::Long nFHeight = m_aBspWin.GetFtHeight();
    tools::Long nFDist   = m_aBspWin.GetFtDist();

    tools::Long nHeight = std::max(tools::Long(MINBODY),
        static_cast<tools::Long>(m_xHeightEdit->denormalize(m_xHeightEdit->get_value(FieldUnit::TWIP))));
    tools::Long nDist   = m_xTurnOnBox->get_active() ?
        static_cast<tools::Long>(m_xDistEdit->denormalize(m_xDistEdit->get_value(FieldUnit::TWIP))) : 0;

    tools::Long nMin;
    tools::Long nMax;

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
    tools::Long nBT = m_aBspWin.GetTop();
    tools::Long nBB = m_aBspWin.GetBottom();
    tools::Long nBL = m_aBspWin.GetLeft();
    tools::Long nBR = m_aBspWin.GetRight();

    tools::Long nH  = m_aBspWin.GetSize().Height();
    tools::Long nW  = m_aBspWin.GetSize().Width();

    // Borders
    if ( nId == SID_ATTR_PAGE_HEADERSET )
    {
        // Header
        nMin = ( nH - nBB - nBT ) / 5; // 20%
        nMax = std::max( nH - nMin - nHDist - nFDist - nFHeight - nBB - nBT,
                    nMin );
        m_xHeightEdit->set_max(m_xHeightEdit->normalize(nMax), FieldUnit::TWIP);
        nMin = ( nH - nBB - nBT ) / 5; // 20%
        nDist = std::max( nH - nMin - nHHeight - nFDist - nFHeight - nBB - nBT,
                     tools::Long(0) );
        m_xDistEdit->set_max(m_xDistEdit->normalize(nDist), FieldUnit::TWIP);
    }
    else
    {
        // Footer
        nMin = ( nH - nBT - nBB ) / 5; // 20%
        nMax = std::max( nH - nMin - nFDist - nHDist - nHHeight - nBT - nBB,
                    nMin );
        m_xHeightEdit->set_max(m_xHeightEdit->normalize(nMax), FieldUnit::TWIP);
        nMin = ( nH - nBT - nBB ) / 5; // 20%
        nDist = std::max( nH - nMin - nFHeight - nHDist - nHHeight - nBT - nBB,
                     tools::Long(0) );
        m_xDistEdit->set_max(m_xDistEdit->normalize(nDist), FieldUnit::TWIP);
    }

    // Limit Indentation
    nMax = nW - nBL - nBR -
           static_cast<tools::Long>(m_xRMEdit->denormalize(m_xRMEdit->get_value(FieldUnit::TWIP))) - MINBODY;
    m_xLMEdit->set_max(m_xLMEdit->normalize(nMax), FieldUnit::TWIP);

    nMax = nW - nBL - nBR -
           static_cast<tools::Long>(m_xLMEdit->denormalize(m_xLMEdit->get_value(FieldUnit::TWIP))) - MINBODY;
    m_xRMEdit->set_max(m_xLMEdit->normalize(nMax), FieldUnit::TWIP);
}

void SvxHFPage::EnableDynamicSpacing()
{
    m_xDynSpacingCB->show();
}

void SvxHFPage::PageCreated(const SfxAllItemSet &rSet)
{
    const SfxBoolItem* pSupportDrawingLayerFillStyleItem = rSet.GetItem<SfxBoolItem>(SID_DRAWINGLAYER_FILLSTYLES, false);

    if (pSupportDrawingLayerFillStyleItem)
    {
        const bool bNew(pSupportDrawingLayerFillStyleItem->GetValue());

        mbEnableDrawingLayerFillStyles = bNew;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
