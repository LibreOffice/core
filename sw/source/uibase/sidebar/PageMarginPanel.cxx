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
#include <swtypes.hxx>
#include <svl/intitem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/paperinf.hxx>
#include <svx/svxids.hrc>
#include <svx/dlgutil.hxx>
#include <svx/rulritem.hxx>
#include "PageMarginPanel.hxx"
#include <sfx2/sidebar/ControlFactory.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>
#include "cmdid.h"
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/document/XUndoManagerSupplier.hpp>
#include <svtools/unitconv.hxx>
#include <svtools/optionsdrawinglayer.hxx>
#include <cstdlib>

#define SWPAGE_NARROW_VALUE    720
#define SWPAGE_NORMAL_VALUE    1136
#define SWPAGE_WIDE_VALUE1     1440
#define SWPAGE_WIDE_VALUE2     2880
#define SWPAGE_WIDE_VALUE3     1800

namespace sw { namespace sidebar{

VclPtr<vcl::Window> PageMarginPanel::Create(
    vcl::Window* pParent,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
    SfxBindings* pBindings)
{
    if( pParent == NULL )
        throw ::com::sun::star::lang::IllegalArgumentException("no parent window given to PageMarginPanel::Create", NULL, 0);
    if( !rxFrame.is() )
        throw ::com::sun::star::lang::IllegalArgumentException("no XFrame given to PageMarginPanel::Create", NULL, 0);
    if( pBindings == NULL )
        throw ::com::sun::star::lang::IllegalArgumentException("no SfxBindings given to PageMarginPanel::Create", NULL, 0);

    return VclPtr<PageMarginPanel>::Create(pParent, rxFrame, pBindings);
}

PageMarginPanel::PageMarginPanel(
    vcl::Window* pParent,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
    SfxBindings* pBindings
    ) :
    PanelLayout(pParent, "PageMarginPanel" , "modules/swriter/ui/pagemarginpanel.ui", rxFrame),
    mpBindings(pBindings),
    meFUnit(GetModuleFieldUnit()),
    meUnit(),
    mpPageLRMarginItem( new SvxLongLRSpaceItem( 0, 0, SID_ATTR_PAGE_LRSPACE ) ),
    mpPageULMarginItem( new SvxLongULSpaceItem( 0, 0, SID_ATTR_PAGE_ULSPACE ) ),
    mpPageItem( new SvxPageItem( SID_ATTR_PAGE ) ),
    maSwPageLRControl(SID_ATTR_PAGE_LRSPACE, *pBindings, *this),
    maSwPageULControl(SID_ATTR_PAGE_ULSPACE, *pBindings, *this),
    maSwPageSizeControl(SID_ATTR_PAGE_SIZE, *pBindings, *this),
    maSwPageMetricControl(SID_ATTR_METRIC, *pBindings, *this)
{
    get(mpMarginSelectBox, "marginLB");
    get(mpLeftMarginEdit, "leftmargin");
    get(mpRightMarginEdit, "rightmargin");
    get(mpTopMarginEdit, "topmargin");
    get(mpBottomMarginEdit, "bottommargin");

    Initialize();
}

PageMarginPanel::~PageMarginPanel()
{
    disposeOnce();
}

void PageMarginPanel::dispose()
{
    mpMarginSelectBox.disposeAndClear();
    mpLeftMarginEdit.disposeAndClear();
    mpRightMarginEdit.disposeAndClear();
    mpTopMarginEdit.disposeAndClear();
    mpBottomMarginEdit.disposeAndClear();

    mpPageLRMarginItem.reset();
    mpPageULMarginItem.reset();
    mpPageItem.reset();

    maSwPageULControl.dispose();
    maSwPageLRControl.dispose();
    maSwPageSizeControl.dispose();
    maSwPageMetricControl.dispose();

    PanelLayout::dispose();
}

void PageMarginPanel::Initialize()
{
    meUnit = maSwPageSizeControl.GetCoreMetric();

    SetFieldUnit(*mpLeftMarginEdit, meFUnit);
    SetFieldUnit(*mpRightMarginEdit, meFUnit);
    SetFieldUnit(*mpTopMarginEdit, meFUnit);
    SetFieldUnit(*mpBottomMarginEdit, meFUnit);

    const SvtOptionsDrawinglayer aDrawinglayerOpt;
    mpLeftMarginEdit->SetMax(mpLeftMarginEdit->Normalize(aDrawinglayerOpt.GetMaximumPaperLeftMargin()), FUNIT_MM);
    mpRightMarginEdit->SetMax(mpRightMarginEdit->Normalize(aDrawinglayerOpt.GetMaximumPaperRightMargin()), FUNIT_MM);
    mpTopMarginEdit->SetMax(mpTopMarginEdit->Normalize(aDrawinglayerOpt.GetMaximumPaperTopMargin()), FUNIT_MM);
    mpBottomMarginEdit->SetMax(mpBottomMarginEdit->Normalize(aDrawinglayerOpt.GetMaximumPaperBottomMargin()), FUNIT_MM);

    mpLeftMarginEdit->SetModifyHdl( LINK(this, PageMarginPanel, ModifyLRMarginHdl) );
    mpRightMarginEdit->SetModifyHdl( LINK(this, PageMarginPanel, ModifyLRMarginHdl) );
    mpTopMarginEdit->SetModifyHdl( LINK(this, PageMarginPanel, ModifyULMarginHdl) );
    mpBottomMarginEdit->SetModifyHdl( LINK(this, PageMarginPanel, ModifyULMarginHdl) );
    mpMarginSelectBox->SetSelectHdl( LINK(this, PageMarginPanel, ModifyPresetMarginHdl));

    mpBindings->Update( SID_ATTR_METRIC );
    mpBindings->Update( SID_ATTR_PAGE );
    mpBindings->Update( SID_ATTR_PAGE_LRSPACE );
    mpBindings->Update( SID_ATTR_PAGE_ULSPACE );

    SetMarginValues();
    UpdateMarginBox();
}

void PageMarginPanel::MetricState( SfxItemState eState, const SfxPoolItem* pState )
{
    meFUnit = FUNIT_NONE;
    if ( pState && eState >= SfxItemState::DEFAULT )
    {
        meFUnit = (FieldUnit)static_cast<const SfxUInt16Item*>(pState )->GetValue();
    }
    else
    {
        SfxViewFrame* pFrame = SfxViewFrame::Current();
        SfxObjectShell* pSh = NULL;
        if ( pFrame )
            pSh = pFrame->GetObjectShell();
        if ( pSh )
        {
            SfxModule* pModule = pSh->GetModule();
            if ( pModule )
            {
                const SfxPoolItem* pItem = pModule->GetItem( SID_ATTR_METRIC );
                if ( pItem )
                    meFUnit = (FieldUnit)static_cast<const SfxUInt16Item*>(pItem )->GetValue();
            }
            else
            {
                SAL_WARN("sw.ui", "<PageMarginPanel::MetricState(..)>: no module found");
            }
        }
    }

    SetFieldUnit( *mpLeftMarginEdit.get(), meFUnit );
    SetFieldUnit( *mpRightMarginEdit.get(), meFUnit );
    SetFieldUnit( *mpTopMarginEdit.get(), meFUnit );
    SetFieldUnit( *mpBottomMarginEdit.get(), meFUnit );

}

void PageMarginPanel::ExecuteMarginLRChange( const long nPageLeftMargin, const long nPageRightMargin )
{
    mpPageLRMarginItem->SetLeft( nPageLeftMargin );
    mpPageLRMarginItem->SetRight( nPageRightMargin );
    GetBindings()->GetDispatcher()->ExecuteList( SID_ATTR_PAGE_LRSPACE, SfxCallMode::RECORD, { mpPageLRMarginItem.get() });
}

void PageMarginPanel::ExecuteMarginULChange( const long nPageTopMargin, const long nPageBottomMargin )
{
    mpPageULMarginItem->SetUpper( nPageTopMargin );
    mpPageULMarginItem->SetLower( nPageBottomMargin );
    GetBindings()->GetDispatcher()->ExecuteList( SID_ATTR_PAGE_ULSPACE, SfxCallMode::RECORD, { mpPageULMarginItem.get() });
}

void PageMarginPanel::NotifyItemUpdate(
    const sal_uInt16 nSId,
    const SfxItemState eState,
    const SfxPoolItem* pState,
    const bool bIsEnabled)
{
    (void)bIsEnabled;
    switch( nSId )
    {
        case SID_ATTR_PAGE_LRSPACE:
        {
            if ( eState >= SfxItemState::DEFAULT &&
             pState && dynamic_cast< const SvxLongLRSpaceItem *>( pState ) !=  nullptr )
            {
                mpPageLRMarginItem.reset( static_cast<SvxLongLRSpaceItem*>(pState->Clone()) );
                SetMarginValues();
                UpdateMarginBox();
            }
        }
        break;
        case SID_ATTR_PAGE_ULSPACE:
        {
            if ( eState >= SfxItemState::DEFAULT &&
                pState && dynamic_cast< const SvxLongULSpaceItem *>( pState ) !=  nullptr )
            {
                mpPageULMarginItem.reset( static_cast<SvxLongULSpaceItem*>(pState->Clone()) );
                SetMarginValues();
                UpdateMarginBox();
            }
        }
        break;
        case SID_ATTR_PAGE:
        {
            if ( eState >= SfxItemState::DEFAULT &&
             pState && dynamic_cast< const SvxPageItem *>( pState ) !=  nullptr )
                mpPageItem.reset( static_cast<SvxPageItem*>(pState->Clone()) );
        }
        break;
        case SID_ATTR_METRIC:
            MetricState(eState, pState);
        break;
        default:
            break;
    }
}

IMPL_LINK_NOARG_TYPED( PageMarginPanel, ModifyLRMarginHdl, Edit&, void )
{
    mnPageLeftMargin = GetCoreValue( *mpLeftMarginEdit.get(), meUnit );
    mnPageRightMargin = GetCoreValue( *mpRightMarginEdit.get(), meUnit );
    ExecuteMarginLRChange( mnPageLeftMargin, mnPageRightMargin );
}

IMPL_LINK_NOARG_TYPED( PageMarginPanel, ModifyULMarginHdl, Edit&, void )
{
    mnPageTopMargin = GetCoreValue( *mpTopMarginEdit.get(), meUnit );
    mnPageBottomMargin = GetCoreValue( *mpBottomMarginEdit.get(), meUnit );
    ExecuteMarginULChange( mnPageTopMargin, mnPageBottomMargin );
}

IMPL_LINK_NOARG_TYPED( PageMarginPanel, ModifyPresetMarginHdl, ListBox&, void )
{
    bool bMirrored = false;
    bool bApplyNewPageMargins = true;
    switch ( mpMarginSelectBox->GetSelectEntryPos() )
    {
        case 0:
            mnPageLeftMargin = SWPAGE_NARROW_VALUE;
            mnPageRightMargin = SWPAGE_NARROW_VALUE;
            mnPageTopMargin = SWPAGE_NARROW_VALUE;
            mnPageBottomMargin = SWPAGE_NARROW_VALUE;
            bMirrored = false;
            break;
        case 1:
            mnPageLeftMargin = SWPAGE_NORMAL_VALUE;
            mnPageRightMargin = SWPAGE_NORMAL_VALUE;
            mnPageTopMargin = SWPAGE_NORMAL_VALUE;
            mnPageBottomMargin = SWPAGE_NORMAL_VALUE;
            bMirrored = false;
            break;
        case 2:
            mnPageLeftMargin = SWPAGE_WIDE_VALUE2;
            mnPageRightMargin = SWPAGE_WIDE_VALUE2;
            mnPageTopMargin = SWPAGE_WIDE_VALUE1;
            mnPageBottomMargin = SWPAGE_WIDE_VALUE1;
            bMirrored = false;
            break;
        case 3:
            mnPageLeftMargin = SWPAGE_WIDE_VALUE3;
            mnPageRightMargin = SWPAGE_WIDE_VALUE1;
            mnPageTopMargin = SWPAGE_WIDE_VALUE1;
            mnPageBottomMargin = SWPAGE_WIDE_VALUE1;
            bMirrored = true;
            break;
        default:
            bApplyNewPageMargins = false;
            break;
    }

    if(bApplyNewPageMargins)
    {
        ExecuteMarginLRChange( mnPageLeftMargin, mnPageRightMargin );
        ExecuteMarginULChange( mnPageTopMargin, mnPageBottomMargin );
        if(bMirrored != (mpPageItem->GetPageUsage() == SVX_PAGE_MIRROR))
        {
            mpPageItem->SetPageUsage( bMirrored ? SVX_PAGE_MIRROR : SVX_PAGE_ALL );
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE,
                                                        SfxCallMode::RECORD, { mpPageItem.get() });
        }
    }
}

void PageMarginPanel::SetMarginValues()
{
    mnPageLeftMargin = mpPageLRMarginItem->GetLeft();
    mnPageRightMargin = mpPageLRMarginItem->GetRight();
    mnPageTopMargin = mpPageULMarginItem->GetUpper();
    mnPageBottomMargin = mpPageULMarginItem->GetLower();

    SetMetricValue( *mpLeftMarginEdit.get(), mnPageLeftMargin, meUnit );
    SetMetricValue( *mpRightMarginEdit.get(), mnPageRightMargin, meUnit );
    SetMetricValue( *mpTopMarginEdit.get(), mnPageTopMargin, meUnit );
    SetMetricValue( *mpBottomMarginEdit.get(), mnPageBottomMargin, meUnit );
}

void PageMarginPanel::UpdateMarginBox()
{
    const long cThreshold = 5;
    bool bMirrored = (mpPageItem->GetPageUsage() == SVX_PAGE_MIRROR);
    if( std::abs(mnPageLeftMargin - SWPAGE_NARROW_VALUE) <= cThreshold &&
        std::abs(mnPageRightMargin - SWPAGE_NARROW_VALUE) <= cThreshold &&
        std::abs(mnPageTopMargin - SWPAGE_NARROW_VALUE) <= cThreshold &&
        std::abs(mnPageBottomMargin - SWPAGE_NARROW_VALUE) <= cThreshold &&
        !bMirrored )
    {
        mpMarginSelectBox->SelectEntryPos(0);
    }
    else if( std::abs(mnPageLeftMargin - SWPAGE_NORMAL_VALUE) <= cThreshold &&
        std::abs(mnPageRightMargin - SWPAGE_NORMAL_VALUE) <= cThreshold &&
        std::abs(mnPageTopMargin - SWPAGE_NORMAL_VALUE) <= cThreshold &&
        std::abs(mnPageBottomMargin - SWPAGE_NORMAL_VALUE) <= cThreshold &&
        !bMirrored )
    {
        mpMarginSelectBox->SelectEntryPos(1);
    }
    else if( std::abs(mnPageLeftMargin - SWPAGE_WIDE_VALUE2) <= cThreshold &&
        std::abs(mnPageRightMargin - SWPAGE_WIDE_VALUE2) <= cThreshold &&
        std::abs(mnPageTopMargin - SWPAGE_WIDE_VALUE1) <= cThreshold &&
        std::abs(mnPageBottomMargin - SWPAGE_WIDE_VALUE1) <= cThreshold &&
        !bMirrored )
    {
        mpMarginSelectBox->SelectEntryPos(2);
    }
    else if( std::abs(mnPageLeftMargin - SWPAGE_WIDE_VALUE3) <= cThreshold &&
        std::abs(mnPageRightMargin - SWPAGE_WIDE_VALUE1) <= cThreshold &&
        std::abs(mnPageTopMargin - SWPAGE_WIDE_VALUE1) <= cThreshold &&
        std::abs(mnPageBottomMargin - SWPAGE_WIDE_VALUE1) <= cThreshold &&
        bMirrored )
    {
        mpMarginSelectBox->SelectEntryPos(3);
    }
    else
    {
        mpMarginSelectBox->SelectEntryPos(4);
    }
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
