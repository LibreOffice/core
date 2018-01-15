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
#include "PageFormatPanel.hxx"
#include "PageMarginUtils.hxx"
#include <sfx2/sidebar/ControlFactory.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>
#include <cmdid.h>
#include <svtools/unitconv.hxx>
#include <svtools/optionsdrawinglayer.hxx>

namespace sw { namespace sidebar{

VclPtr<vcl::Window> PageFormatPanel::Create(
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

PageFormatPanel::PageFormatPanel(
    vcl::Window* pParent,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
    SfxBindings* pBindings) :
    PanelLayout(pParent, "PageFormatPanel", "modules/swriter/ui/pageformatpanel.ui", rxFrame),
    mpBindings( pBindings ),
    maPaperSizeController(SID_ATTR_PAGE_SIZE, *pBindings, *this),
    maPaperOrientationController(SID_ATTR_PAGE, *pBindings, *this),
    maMetricController(SID_ATTR_METRIC, *pBindings,*this),
    maSwPageLRControl(SID_ATTR_PAGE_LRSPACE, *pBindings, *this),
    maSwPageULControl(SID_ATTR_PAGE_ULSPACE, *pBindings, *this),
    mpPageItem( new SvxPageItem(SID_ATTR_PAGE) ),
    mpPageLRMarginItem( new SvxLongLRSpaceItem( 0, 0, SID_ATTR_PAGE_LRSPACE ) ),
    mpPageULMarginItem( new SvxLongULSpaceItem( 0, 0, SID_ATTR_PAGE_ULSPACE ) ),
    meFUnit(GetModuleFieldUnit()),
    meLastFUnit(GetModuleFieldUnit()),
    meUnit(),
    aCustomEntry()
{
    get(mpPaperSizeBox, "papersize");
    get(mpPaperWidth, "paperwidth");
    get(mpPaperHeight, "paperheight");
    get(mpPaperOrientation, "paperorientation");
    get(mpMarginSelectBox, "marginLB");
    get(mpCustomEntry, "customlabel");
    Initialize();
}

PageFormatPanel::~PageFormatPanel()
{
    disposeOnce();
}

void PageFormatPanel::dispose()
{
    mpPaperSizeBox.disposeAndClear();
    mpPaperWidth.disposeAndClear();
    mpPaperHeight.disposeAndClear();
    mpPaperOrientation.disposeAndClear();
    mpMarginSelectBox.disposeAndClear();
    mpCustomEntry.clear();

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
    mpPaperSizeBox->FillPaperSizeEntries( PaperSizeApp::Std );
    mpPaperSizeBox->SetDropDownLineCount(6);
    meUnit = maPaperSizeController.GetCoreMetric();
    SetFieldUnit( *mpPaperWidth, meFUnit );
    SetFieldUnit( *mpPaperHeight, meFUnit );
    aCustomEntry = mpCustomEntry->GetText();

    const SvtOptionsDrawinglayer aDrawinglayerOpt;
    mpPaperWidth->SetMax(mpPaperWidth->Normalize(aDrawinglayerOpt.GetMaximumPaperWidth()), FUNIT_CM);
    mpPaperWidth->SetLast(mpPaperWidth->Normalize(aDrawinglayerOpt.GetMaximumPaperWidth()), FUNIT_CM);
    mpPaperHeight->SetMax(mpPaperHeight->Normalize(aDrawinglayerOpt.GetMaximumPaperHeight()), FUNIT_CM);
    mpPaperHeight->SetLast(mpPaperHeight->Normalize(aDrawinglayerOpt.GetMaximumPaperHeight()), FUNIT_CM);

    mpPaperSizeBox->SetSelectHdl( LINK(this, PageFormatPanel, PaperFormatModifyHdl ));
    mpPaperOrientation->SetSelectHdl( LINK(this, PageFormatPanel, PaperFormatModifyHdl ));
    mpPaperHeight->SetModifyHdl( LINK(this, PageFormatPanel, PaperSizeModifyHdl ));
    mpPaperWidth->SetModifyHdl( LINK(this, PageFormatPanel, PaperSizeModifyHdl ));
    mpMarginSelectBox->SetSelectHdl( LINK(this, PageFormatPanel, PaperModifyMarginHdl));

    mpBindings->Update(SID_ATTR_METRIC);
    mpBindings->Update(SID_ATTR_PAGE);
    mpBindings->Update(SID_ATTR_PAGE_SIZE);
    mpBindings->Update( SID_ATTR_PAGE_LRSPACE );
    mpBindings->Update( SID_ATTR_PAGE_ULSPACE );

    UpdateMarginBox();
}

void PageFormatPanel::NotifyItemUpdate(
    const sal_uInt16 nSId,
    const SfxItemState eState,
    const SfxPoolItem* pState,
    const bool)
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

                mpPaperWidth->SetValue( mpPaperWidth->Normalize( aPaperSize.Width() ), FUNIT_TWIP );
                mpPaperHeight->SetValue( mpPaperHeight->Normalize( aPaperSize.Height() ), FUNIT_TWIP );

                if(mpPaperOrientation->GetSelectedEntryPos() == 1)
                   Swap(aPaperSize);

                Paper ePaper = SvxPaperInfo::GetSvxPaper(aPaperSize, meUnit);
                mpPaperSizeBox->SetSelection( ePaper );
            }
        }
        break;
        case SID_ATTR_METRIC:
        {
            meUnit = maPaperSizeController.GetCoreMetric();
            meFUnit = GetCurrentUnit(eState, pState);
            if(meFUnit != meLastFUnit)
            {
                SetFieldUnit( *mpPaperHeight, meFUnit );
                SetFieldUnit( *mpPaperWidth, meFUnit );
            }
            meLastFUnit = meFUnit;
        }
        break;
        case SID_ATTR_PAGE:
        {
            if ( eState >= SfxItemState::DEFAULT &&
                pState && dynamic_cast< const SvxPageItem *>( pState ) !=  nullptr )
            {
                mpPageItem.reset( static_cast<SvxPageItem*>(pState->Clone()) );
                if ( mpPageItem->IsLandscape() )
                    mpPaperOrientation->SelectEntryPos(1);
                else
                    mpPaperOrientation->SelectEntryPos(0);
            }
        }
        break;
        case SID_ATTR_PAGE_LRSPACE:
        {
            if ( eState >= SfxItemState::DEFAULT &&
             pState && dynamic_cast< const SvxLongLRSpaceItem *>( pState ) !=  nullptr )
            {
                mpPageLRMarginItem.reset( static_cast<SvxLongLRSpaceItem*>(pState->Clone()) );
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
                UpdateMarginBox();
            }
        }
        break;
        default:
            break;
    }
}

IMPL_LINK_NOARG(PageFormatPanel, PaperFormatModifyHdl, ListBox&, void)
{
    Paper ePaper = mpPaperSizeBox->GetSelection();
    Size  aSize(SvxPaperInfo::GetPaperSize(ePaper, meUnit));

    if(mpPaperOrientation->GetSelectedEntryPos() == 1)
        Swap(aSize);

    mpPageItem->SetLandscape(mpPaperOrientation->GetSelectedEntryPos() == 1);
    SvxSizeItem aSizeItem(SID_ATTR_PAGE_SIZE, aSize);
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_SIZE, SfxCallMode::RECORD, { &aSizeItem, mpPageItem.get() });
}

IMPL_LINK_NOARG(PageFormatPanel, PaperSizeModifyHdl, Edit&, void)
{
    Size aSize( GetCoreValue( *mpPaperWidth, meUnit ), GetCoreValue( *mpPaperHeight, meUnit));
    SvxSizeItem aSizeItem(SID_ATTR_PAGE_SIZE, aSize);
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_SIZE, SfxCallMode::RECORD, { &aSizeItem });
}

IMPL_LINK_NOARG(PageFormatPanel, PaperModifyMarginHdl, ListBox&, void)
{
    bool bMirrored = false;
    bool bApplyNewPageMargins = true;
    switch ( mpMarginSelectBox->GetSelectedEntryPos() )
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
    FieldUnit eUnit = FUNIT_NONE;

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

void PageFormatPanel::ExecuteMarginLRChange( const long nPageLeftMargin, const long nPageRightMargin )
{
    mpPageLRMarginItem->SetLeft( nPageLeftMargin );
    mpPageLRMarginItem->SetRight( nPageRightMargin );
    mpBindings->GetDispatcher()->ExecuteList( SID_ATTR_PAGE_LRSPACE, SfxCallMode::RECORD, { mpPageLRMarginItem.get() });
}

void PageFormatPanel::ExecuteMarginULChange(const long nPageTopMargin, const long nPageBottomMargin)
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

    bool bMirrored = (mpPageItem->GetPageUsage() == SvxPageUsage::Mirror);
    if( IsNone(mnPageLeftMargin, mnPageRightMargin, mnPageTopMargin, mnPageBottomMargin, bMirrored) )
    {
        mpMarginSelectBox->SelectEntryPos(0);
        mpMarginSelectBox->RemoveEntry(aCustomEntry);
    }
    else if( IsNarrow(mnPageLeftMargin, mnPageRightMargin, mnPageTopMargin, mnPageBottomMargin, bMirrored) )
    {
        mpMarginSelectBox->SelectEntryPos(1);
        mpMarginSelectBox->RemoveEntry(aCustomEntry);
    }
    else if( IsModerate(mnPageLeftMargin, mnPageRightMargin, mnPageTopMargin, mnPageBottomMargin, bMirrored) )
    {
        mpMarginSelectBox->SelectEntryPos(2);
        mpMarginSelectBox->RemoveEntry(aCustomEntry);
    }
    else if( IsNormal075(mnPageLeftMargin, mnPageRightMargin, mnPageTopMargin, mnPageBottomMargin, bMirrored) )
    {
        mpMarginSelectBox->SelectEntryPos(3);
        mpMarginSelectBox->RemoveEntry(aCustomEntry);
    }
    else if( IsNormal100(mnPageLeftMargin, mnPageRightMargin, mnPageTopMargin, mnPageBottomMargin, bMirrored) )
    {
        mpMarginSelectBox->SelectEntryPos(4);
        mpMarginSelectBox->RemoveEntry(aCustomEntry);
    }
    else if( IsNormal125(mnPageLeftMargin, mnPageRightMargin, mnPageTopMargin, mnPageBottomMargin, bMirrored) )
    {
        mpMarginSelectBox->SelectEntryPos(5);
        mpMarginSelectBox->RemoveEntry(aCustomEntry);
    }
    else if( IsWide(mnPageLeftMargin, mnPageRightMargin, mnPageTopMargin, mnPageBottomMargin, bMirrored) )
    {
        mpMarginSelectBox->SelectEntryPos(6);
        mpMarginSelectBox->RemoveEntry(aCustomEntry);
    }
    else if( IsMirrored(mnPageLeftMargin, mnPageRightMargin, mnPageTopMargin, mnPageBottomMargin, bMirrored) )
    {
        mpMarginSelectBox->SelectEntryPos(7);
        mpMarginSelectBox->RemoveEntry(aCustomEntry);
    }
    else
    {
        if(mpMarginSelectBox->GetEntryPos(aCustomEntry) == LISTBOX_ENTRY_NOTFOUND)
            mpMarginSelectBox->InsertEntry(aCustomEntry);
        mpMarginSelectBox->SelectEntry(aCustomEntry);
    }
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
