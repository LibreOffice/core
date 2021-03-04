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
#include <sal/log.hxx>
#include <swtypes.hxx>
#include <svl/intitem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/paperinf.hxx>
#include <svx/dlgutil.hxx>
#include <svx/rulritem.hxx>
#include <svx/svdtrans.hxx>
#include "PageFormatPanel.hxx"
#include "PageMarginUtils.hxx"
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/module.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <pageformatpanel.hrc>
#include <cmdid.h>
#include <svtools/optionsdrawinglayer.hxx>
#include <unotools/localedatawrapper.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>

#include <com/sun/star/lang/IllegalArgumentException.hpp>

namespace sw::sidebar{

VclPtr<PanelLayout> PageFormatPanel::Create(
    vcl::Window* pParent,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
    SfxBindings* pBindings)
{
    if( pParent == nullptr )
        throw ::com::sun::star::lang::IllegalArgumentException("no parent window given to PageFormatPanel::Create", nullptr, 0);
    if( !rxFrame.is() )
        throw ::com::sun::star::lang::IllegalArgumentException("no XFrame given to PageFormatPanel::Create", nullptr, 0);

    return VclPtr<PageFormatPanel>::Create(pParent, rxFrame, pBindings);
}

void PageFormatPanel::SetMarginFieldUnit()
{
    auto nSelected = mxMarginSelectBox->get_active();
    mxMarginSelectBox->clear();

    const LocaleDataWrapper& rLocaleData = Application::GetSettings().GetLocaleDataWrapper();
    if (IsInch(meFUnit))
    {
        OUString sSuffix = weld::MetricSpinButton::MetricToString(FieldUnit::INCH);
        for (size_t i = 0; i < SAL_N_ELEMENTS(RID_PAGEFORMATPANEL_MARGINS_INCH); ++i)
        {
            OUString sStr = rLocaleData.getNum(RID_PAGEFORMATPANEL_MARGINS_INCH[i].second, 2, true, false) + sSuffix;
            mxMarginSelectBox->append_text(SwResId(RID_PAGEFORMATPANEL_MARGINS_INCH[i].first).replaceFirst("%1", sStr));
        }
    }
    else
    {
        OUString sSuffix = weld::MetricSpinButton::MetricToString(FieldUnit::CM);
        for (size_t i = 0; i < SAL_N_ELEMENTS(RID_PAGEFORMATPANEL_MARGINS_CM); ++i)
        {
            OUString sStr = rLocaleData.getNum(RID_PAGEFORMATPANEL_MARGINS_CM[i].second, 2, true, false) + " " + sSuffix;
            mxMarginSelectBox->append_text(SwResId(RID_PAGEFORMATPANEL_MARGINS_CM[i].first).replaceFirst("%1", sStr));
        }
    }
    mxMarginSelectBox->set_active(nSelected);
}

PageFormatPanel::PageFormatPanel(
    vcl::Window* pParent,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
    SfxBindings* pBindings) :
    PanelLayout(pParent, "PageFormatPanel", "modules/swriter/ui/pageformatpanel.ui", rxFrame),
    mpBindings( pBindings ),
    mxPaperSizeBox(new SvxPaperSizeListBox(m_xBuilder->weld_combo_box("papersize"))),
    mxPaperWidth(new SvxRelativeField(m_xBuilder->weld_metric_spin_button("paperwidth", FieldUnit::CM))),
    mxPaperHeight(new SvxRelativeField(m_xBuilder->weld_metric_spin_button("paperheight", FieldUnit::CM))),
    mxPaperOrientation(m_xBuilder->weld_combo_box("paperorientation")),
    mxMarginSelectBox(m_xBuilder->weld_combo_box("marginLB")),
    mxCustomEntry(m_xBuilder->weld_label("customlabel")),
    maPaperSizeController(SID_ATTR_PAGE_SIZE, *pBindings, *this),
    maPaperOrientationController(SID_ATTR_PAGE, *pBindings, *this),
    maMetricController(SID_ATTR_METRIC, *pBindings,*this),
    maSwPageLRControl(SID_ATTR_PAGE_LRSPACE, *pBindings, *this),
    maSwPageULControl(SID_ATTR_PAGE_ULSPACE, *pBindings, *this),
    mpPageItem( new SvxPageItem(SID_ATTR_PAGE) ),
    mpPageLRMarginItem( new SvxLongLRSpaceItem( 0, 0, SID_ATTR_PAGE_LRSPACE ) ),
    mpPageULMarginItem( new SvxLongULSpaceItem( 0, 0, SID_ATTR_PAGE_ULSPACE ) ),
    meFUnit(GetModuleFieldUnit()),
    meUnit(),
    aCustomEntry()
{
    Initialize();
}

PageFormatPanel::~PageFormatPanel()
{
    disposeOnce();
}

void PageFormatPanel::dispose()
{
    mxPaperSizeBox.reset();
    mxPaperWidth.reset();
    mxPaperHeight.reset();
    mxPaperOrientation.reset();
    mxMarginSelectBox.reset();
    mxCustomEntry.reset();

    maMetricController.dispose();
    maPaperOrientationController.dispose();
    maPaperSizeController.dispose();
    maSwPageLRControl.dispose();
    maSwPageULControl.dispose();
    mpPageULMarginItem.reset();
    mpPageLRMarginItem.reset();
    mpPageItem.reset();

    PanelLayout::dispose();
}

void PageFormatPanel::Initialize()
{
    mxPaperSizeBox->FillPaperSizeEntries( PaperSizeApp::Std );
    maPaperSizeController.SetFallbackCoreMetric(MapUnit::MapTwip);
    meUnit = maPaperSizeController.GetCoreMetric();
    mxPaperWidth->SetFieldUnit(meFUnit);
    mxPaperHeight->SetFieldUnit(meFUnit);
    SetMarginFieldUnit();
    aCustomEntry = mxCustomEntry->get_label();

    const SvtOptionsDrawinglayer aDrawinglayerOpt;
    mxPaperWidth->set_max(mxPaperWidth->normalize(aDrawinglayerOpt.GetMaximumPaperWidth()), FieldUnit::CM);
    mxPaperHeight->set_max(mxPaperHeight->normalize(aDrawinglayerOpt.GetMaximumPaperHeight()), FieldUnit::CM);

    mxPaperSizeBox->connect_changed( LINK(this, PageFormatPanel, PaperFormatModifyHdl ));
    mxPaperOrientation->connect_changed( LINK(this, PageFormatPanel, PaperFormatModifyHdl ));
    mxPaperHeight->connect_value_changed( LINK(this, PageFormatPanel, PaperSizeModifyHdl ));
    mxPaperWidth->connect_value_changed( LINK(this, PageFormatPanel, PaperSizeModifyHdl ));
    mxMarginSelectBox->connect_changed( LINK(this, PageFormatPanel, PaperModifyMarginHdl));

    mpBindings->Update(SID_ATTR_METRIC);
    mpBindings->Update(SID_ATTR_PAGE);
    mpBindings->Update(SID_ATTR_PAGE_SIZE);
    mpBindings->Update(SID_ATTR_PAGE_LRSPACE);
    mpBindings->Update(SID_ATTR_PAGE_ULSPACE);

    UpdateMarginBox();
}

void PageFormatPanel::NotifyItemUpdate(
    const sal_uInt16 nSId,
    const SfxItemState eState,
    const SfxPoolItem* pState)
{
    switch(nSId)
    {
        case SID_ATTR_PAGE_SIZE:
        {
            const SvxSizeItem* pSizeItem = nullptr;
            if (eState >= SfxItemState::DEFAULT)
                pSizeItem = dynamic_cast< const SvxSizeItem* >(pState);
            if (pSizeItem)
            {
                Size aPaperSize = pSizeItem->GetSize();

                mxPaperWidth->set_value(mxPaperWidth->normalize(aPaperSize.Width()), FieldUnit::TWIP);
                mxPaperHeight->set_value(mxPaperHeight->normalize(aPaperSize.Height()), FieldUnit::TWIP);

                if(mxPaperOrientation->get_active() == 1)
                   Swap(aPaperSize);

                Paper ePaper = SvxPaperInfo::GetSvxPaper(aPaperSize, meUnit);
                mxPaperSizeBox->set_active_id( ePaper );
            }
        }
        break;
        case SID_ATTR_METRIC:
        {
            meUnit = maPaperSizeController.GetCoreMetric();
            FieldUnit eFUnit = GetCurrentUnit(eState, pState);
            if (eFUnit != meFUnit)
            {
                meFUnit = eFUnit;
                mxPaperHeight->SetFieldUnit(meFUnit);
                mxPaperWidth->SetFieldUnit(meFUnit);
                SetMarginFieldUnit();
                UpdateMarginBox();
            }
        }
        break;
        case SID_ATTR_PAGE:
        {
            if ( eState >= SfxItemState::DEFAULT &&
                 dynamic_cast< const SvxPageItem *>( pState ) )
            {
                mpPageItem.reset( static_cast<SvxPageItem*>(pState->Clone()) );
                if ( mpPageItem->IsLandscape() )
                    mxPaperOrientation->set_active(1);
                else
                    mxPaperOrientation->set_active(0);
            }
        }
        break;
        case SID_ATTR_PAGE_LRSPACE:
        {
            if ( eState >= SfxItemState::DEFAULT &&
                 dynamic_cast< const SvxLongLRSpaceItem *>( pState ) )
            {
                mpPageLRMarginItem.reset( static_cast<SvxLongLRSpaceItem*>(pState->Clone()) );
                UpdateMarginBox();
            }
        }
        break;
        case SID_ATTR_PAGE_ULSPACE:
        {
            if ( eState >= SfxItemState::DEFAULT &&
                dynamic_cast< const SvxLongULSpaceItem *>( pState ) )
            {
                mpPageULMarginItem.reset( static_cast<SvxLongULSpaceItem*>(pState->Clone()) );
                UpdateMarginBox();
            }
        }
        break;
        default:
            break;
    }
}

IMPL_LINK_NOARG(PageFormatPanel, PaperFormatModifyHdl, weld::ComboBox&, void)
{
    Paper ePaper = mxPaperSizeBox->get_active_id();
    Size  aSize;

    if(ePaper!=PAPER_USER)
       aSize = SvxPaperInfo::GetPaperSize(ePaper, meUnit);
    else
       aSize = Size(mxPaperWidth->GetCoreValue(meUnit), mxPaperHeight->GetCoreValue(meUnit));

    if (mxPaperOrientation->get_active() == 1 || ePaper==PAPER_USER)
        Swap(aSize);

    mpPageItem->SetLandscape(mxPaperOrientation->get_active() == 1);
    SvxSizeItem aSizeItem(SID_ATTR_PAGE_SIZE, aSize);
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_SIZE, SfxCallMode::RECORD, { &aSizeItem, mpPageItem.get() });
}

IMPL_LINK_NOARG(PageFormatPanel, PaperSizeModifyHdl, weld::MetricSpinButton&, void)
{
    Size aSize(mxPaperWidth->GetCoreValue(meUnit), mxPaperHeight->GetCoreValue(meUnit));
    SvxSizeItem aSizeItem(SID_ATTR_PAGE_SIZE, aSize);
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_SIZE, SfxCallMode::RECORD, { &aSizeItem });
}

IMPL_LINK_NOARG(PageFormatPanel, PaperModifyMarginHdl, weld::ComboBox&, void)
{
    bool bMirrored = false;
    bool bApplyNewPageMargins = true;
    switch (mxMarginSelectBox->get_active())
    {
        case 0:
            SetNone(mnPageLeftMargin, mnPageRightMargin, mnPageTopMargin, mnPageBottomMargin, bMirrored);
            break;
        case 1:
            SetNarrow(mnPageLeftMargin, mnPageRightMargin, mnPageTopMargin, mnPageBottomMargin, bMirrored);
            break;
        case 2:
            SetModerate(mnPageLeftMargin, mnPageRightMargin, mnPageTopMargin, mnPageBottomMargin, bMirrored);
            break;
        case 3:
            SetNormal075(mnPageLeftMargin, mnPageRightMargin, mnPageTopMargin, mnPageBottomMargin, bMirrored);
            break;
        case 4:
            SetNormal100(mnPageLeftMargin, mnPageRightMargin, mnPageTopMargin, mnPageBottomMargin, bMirrored);
            break;
        case 5:
            SetNormal125(mnPageLeftMargin, mnPageRightMargin, mnPageTopMargin, mnPageBottomMargin, bMirrored);
            break;
        case 6:
            SetWide(mnPageLeftMargin, mnPageRightMargin, mnPageTopMargin, mnPageBottomMargin, bMirrored);
            break;
        case 7:
            SetMirrored(mnPageLeftMargin, mnPageRightMargin, mnPageTopMargin, mnPageBottomMargin, bMirrored);
            break;
        default:
            bApplyNewPageMargins = false;
            break;
    }

    if(bApplyNewPageMargins)
    {
        ExecuteMarginLRChange( mnPageLeftMargin, mnPageRightMargin );
        ExecuteMarginULChange( mnPageTopMargin, mnPageBottomMargin );
        if(bMirrored != (mpPageItem->GetPageUsage() == SvxPageUsage::Mirror))
        {
            mpPageItem->SetPageUsage( bMirrored ? SvxPageUsage::Mirror : SvxPageUsage::All );
            mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_PAGE,
                                                        SfxCallMode::RECORD, { mpPageItem.get() });
        }
    }
}

FieldUnit PageFormatPanel::GetCurrentUnit( SfxItemState eState, const SfxPoolItem* pState )
{
    FieldUnit eUnit = FieldUnit::NONE;

    if ( pState && eState >= SfxItemState::DEFAULT )
        eUnit = static_cast<FieldUnit>(static_cast<const SfxUInt16Item*>(pState)->GetValue());
    else
    {
        SfxViewFrame* pFrame = SfxViewFrame::Current();
        SfxObjectShell* pSh = nullptr;
        if ( pFrame )
            pSh = pFrame->GetObjectShell();
        if ( pSh )
        {
            SfxModule* pModule = pSh->GetModule();
            if ( pModule )
            {
                const SfxPoolItem* pItem = pModule->GetItem( SID_ATTR_METRIC );
                if ( pItem )
                    eUnit = static_cast<FieldUnit>(static_cast<const SfxUInt16Item*>(pItem)->GetValue());
            }
            else
            {
                SAL_WARN("sw.ui", "GetModuleFieldUnit(): no module found");
            }
        }
    }

    return eUnit;
}

void PageFormatPanel::ExecuteMarginLRChange( const tools::Long nPageLeftMargin, const tools::Long nPageRightMargin )
{
    mpPageLRMarginItem->SetLeft( nPageLeftMargin );
    mpPageLRMarginItem->SetRight( nPageRightMargin );
    mpBindings->GetDispatcher()->ExecuteList( SID_ATTR_PAGE_LRSPACE, SfxCallMode::RECORD, { mpPageLRMarginItem.get() });
}

void PageFormatPanel::ExecuteMarginULChange(const tools::Long nPageTopMargin, const tools::Long nPageBottomMargin)
{
    mpPageULMarginItem->SetUpper( nPageTopMargin );
    mpPageULMarginItem->SetLower( nPageBottomMargin );
    mpBindings->GetDispatcher()->ExecuteList( SID_ATTR_PAGE_ULSPACE, SfxCallMode::RECORD, { mpPageULMarginItem.get() });
}

void PageFormatPanel::UpdateMarginBox()
{
    mnPageLeftMargin = mpPageLRMarginItem->GetLeft();
    mnPageRightMargin = mpPageLRMarginItem->GetRight();
    mnPageTopMargin = mpPageULMarginItem->GetUpper();
    mnPageBottomMargin = mpPageULMarginItem->GetLower();

    int nCustomEntry = mxMarginSelectBox->find_text(aCustomEntry);

    bool bMirrored = (mpPageItem->GetPageUsage() == SvxPageUsage::Mirror);
    if( IsNone(mnPageLeftMargin, mnPageRightMargin, mnPageTopMargin, mnPageBottomMargin, bMirrored) )
    {
        mxMarginSelectBox->set_active(0);
        if (nCustomEntry != -1)
            mxMarginSelectBox->remove(nCustomEntry);
    }
    else if( IsNarrow(mnPageLeftMargin, mnPageRightMargin, mnPageTopMargin, mnPageBottomMargin, bMirrored) )
    {
        mxMarginSelectBox->set_active(1);
        if (nCustomEntry != -1)
            mxMarginSelectBox->remove(nCustomEntry);
    }
    else if( IsModerate(mnPageLeftMargin, mnPageRightMargin, mnPageTopMargin, mnPageBottomMargin, bMirrored) )
    {
        mxMarginSelectBox->set_active(2);
        if (nCustomEntry != -1)
            mxMarginSelectBox->remove(nCustomEntry);
    }
    else if( IsNormal075(mnPageLeftMargin, mnPageRightMargin, mnPageTopMargin, mnPageBottomMargin, bMirrored) )
    {
        mxMarginSelectBox->set_active(3);
        if (nCustomEntry != -1)
            mxMarginSelectBox->remove(nCustomEntry);
    }
    else if( IsNormal100(mnPageLeftMargin, mnPageRightMargin, mnPageTopMargin, mnPageBottomMargin, bMirrored) )
    {
        mxMarginSelectBox->set_active(4);
        if (nCustomEntry != -1)
            mxMarginSelectBox->remove(nCustomEntry);
    }
    else if( IsNormal125(mnPageLeftMargin, mnPageRightMargin, mnPageTopMargin, mnPageBottomMargin, bMirrored) )
    {
        mxMarginSelectBox->set_active(5);
        if (nCustomEntry != -1)
            mxMarginSelectBox->remove(nCustomEntry);
    }
    else if( IsWide(mnPageLeftMargin, mnPageRightMargin, mnPageTopMargin, mnPageBottomMargin, bMirrored) )
    {
        mxMarginSelectBox->set_active(6);
        if (nCustomEntry != -1)
            mxMarginSelectBox->remove(nCustomEntry);
    }
    else if( IsMirrored(mnPageLeftMargin, mnPageRightMargin, mnPageTopMargin, mnPageBottomMargin, bMirrored) )
    {
        mxMarginSelectBox->set_active(7);
        if (nCustomEntry != -1)
            mxMarginSelectBox->remove(nCustomEntry);
    }
    else
    {
        if (nCustomEntry == -1)
            mxMarginSelectBox->append_text(aCustomEntry);
        mxMarginSelectBox->set_active_text(aCustomEntry);
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
