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
    mpPageItem( new SvxPageItem(SID_ATTR_PAGE) ),
    meFUnit(GetModuleFieldUnit()),
    meLastFUnit(GetModuleFieldUnit()),
    meUnit()
{
    get(mpPaperSizeBox, "papersize");
    get(mpPaperWidth, "paperwidth");
    get(mpPaperHeight, "paperheight");
    get(mpPaperOrientation, "paperorientation");
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

    maMetricController.dispose();
    maPaperOrientationController.dispose();
    maPaperSizeController.dispose();
    mpPageItem.reset();

    PanelLayout::dispose();
}

void PageFormatPanel::Initialize()
{
    mpPaperSizeBox->FillPaperSizeEntries( PaperSizeStd );
    mpPaperSizeBox->SetDropDownLineCount(6);
    meUnit = maPaperSizeController.GetCoreMetric();
    SetFieldUnit( *mpPaperWidth, meFUnit );
    SetFieldUnit( *mpPaperHeight, meFUnit );

    const SvtOptionsDrawinglayer aDrawinglayerOpt;
    mpPaperWidth->SetMax(mpPaperWidth->Normalize(aDrawinglayerOpt.GetMaximumPaperWidth()), FUNIT_CM);
    mpPaperWidth->SetLast(mpPaperWidth->Normalize(aDrawinglayerOpt.GetMaximumPaperWidth()), FUNIT_CM);
    mpPaperHeight->SetMax(mpPaperHeight->Normalize(aDrawinglayerOpt.GetMaximumPaperHeight()), FUNIT_CM);
    mpPaperHeight->SetLast(mpPaperHeight->Normalize(aDrawinglayerOpt.GetMaximumPaperHeight()), FUNIT_CM);

    mpPaperSizeBox->SetSelectHdl( LINK(this, PageFormatPanel, PaperFormatModifyHdl ));
    mpPaperOrientation->SetSelectHdl( LINK(this, PageFormatPanel, PaperFormatModifyHdl ));
    mpPaperHeight->SetModifyHdl( LINK(this, PageFormatPanel, PaperSizeModifyHdl ));
    mpPaperWidth->SetModifyHdl( LINK(this, PageFormatPanel, PaperSizeModifyHdl ));

    mpBindings->Update(SID_ATTR_METRIC);
    mpBindings->Update(SID_ATTR_PAGE);
    mpBindings->Update(SID_ATTR_PAGE_SIZE);
}

void PageFormatPanel::NotifyItemUpdate(
    const sal_uInt16 nSId,
    const SfxItemState eState,
    const SfxPoolItem* pState,
    const bool bIsEnabled)
{
    (void)bIsEnabled;

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

                if(mpPaperOrientation->GetSelectEntryPos() == 1)
                   Swap(aPaperSize);

                Paper ePaper = SvxPaperInfo::GetSvxPaper(aPaperSize, static_cast<MapUnit>(meUnit),true);
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
        default:
            break;
    }
}

IMPL_LINK_NOARG_TYPED(PageFormatPanel, PaperFormatModifyHdl, ListBox&, void)
{
    Paper ePaper = mpPaperSizeBox->GetSelection();
    Size  aSize(SvxPaperInfo::GetPaperSize(ePaper, (MapUnit)(meUnit)));

    if(mpPaperOrientation->GetSelectEntryPos() == 1)
        Swap(aSize);

    mpPageItem->SetLandscape(mpPaperOrientation->GetSelectEntryPos());
    SvxSizeItem aSizeItem(SID_ATTR_PAGE_SIZE, aSize);
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_SIZE, SfxCallMode::RECORD, { &aSizeItem, mpPageItem.get() });
}

IMPL_LINK_NOARG_TYPED(PageFormatPanel, PaperSizeModifyHdl, Edit&, void)
{
    Size aSize( GetCoreValue( *mpPaperWidth, meUnit ), GetCoreValue( *mpPaperHeight, meUnit));
    SvxSizeItem aSizeItem(SID_ATTR_PAGE_SIZE, aSize);
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_SIZE, SfxCallMode::RECORD, { &aSizeItem });
}

FieldUnit PageFormatPanel::GetCurrentUnit( SfxItemState eState, const SfxPoolItem* pState )
{
    FieldUnit eUnit = FUNIT_NONE;

    if ( pState && eState >= SfxItemState::DEFAULT )
        eUnit = (FieldUnit) static_cast<const SfxUInt16Item*>(pState)->GetValue();
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
                    eUnit = (FieldUnit) static_cast<const SfxUInt16Item*>(pItem)->GetValue();
            }
            else
            {
                SAL_WARN("sw.sidebar", "GetModuleFieldUnit(): no module found");
            }
        }
    }

    return eUnit;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
