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
#include <svl/eitem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/paperinf.hxx>
#include <svx/svxids.hrc>
#include <svx/dlgutil.hxx>
#include <svx/rulritem.hxx>
#include "PageFooterPanel.hxx"
#include <sfx2/sidebar/ControlFactory.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>
#include <cmdid.h>

namespace sw { namespace sidebar{

VclPtr<vcl::Window> PageFooterPanel::Create(
    vcl::Window* pParent,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
    SfxBindings* pBindings)
{
    if( pParent == nullptr )
        throw ::com::sun::star::lang::IllegalArgumentException("no parent window given to PageFooterPanel::Create", nullptr, 0);
    if( !rxFrame.is() )
        throw ::com::sun::star::lang::IllegalArgumentException("no XFrame given to PageFooterPanel::Create", nullptr, 0);

    return VclPtr<PageFooterPanel>::Create(pParent, rxFrame, pBindings);
}

PageFooterPanel::PageFooterPanel(
    vcl::Window* pParent,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
    SfxBindings* pBindings) :
    PanelLayout(pParent, "PageFooterPanel", "modules/swriter/ui/pagefooterpanel.ui", rxFrame),
    mpBindings( pBindings ),
    maHFToggleController(SID_ATTR_PAGE_FOOTER, *pBindings, *this),
    maFooterLRMarginController(SID_ATTR_PAGE_FOOTER_LRMARGIN, *pBindings, *this),
    maFooterSpacingController(SID_ATTR_PAGE_FOOTER_SPACING, *pBindings, *this),
    maFooterLayoutController(SID_ATTR_PAGE_FOOTER_LAYOUT, *pBindings, *this),
    aCustomEntry(),
    mpFooterItem( new SfxBoolItem(SID_ATTR_PAGE_FOOTER) ),
    mpFooterLRMarginItem( new SvxLongLRSpaceItem(0, 0, SID_ATTR_PAGE_FOOTER_LRMARGIN)),
    mpFooterSpacingItem( new SvxLongULSpaceItem(0, 0, SID_ATTR_PAGE_FOOTER_SPACING)),
    mpFooterLayoutItem( new SfxInt16Item(SID_ATTR_PAGE_FOOTER_LAYOUT))
{
    get(mpFooterToggle, "footertoggle");
    get(mpFooterSpacingLB, "spacingpreset");
    get(mpFooterLayoutLB, "samecontentLB");
    get(mpFooterMarginPresetLB, "footermarginpreset");
    get(mpCustomEntry, "customlabel");

    Initialize();
}

PageFooterPanel::~PageFooterPanel()
{
    disposeOnce();
}

void PageFooterPanel::dispose()
{
    mpFooterToggle.disposeAndClear();
    mpFooterSpacingLB.disposeAndClear();
    mpFooterLayoutLB.disposeAndClear();
    mpFooterMarginPresetLB.disposeAndClear();
    mpCustomEntry.clear();

    PanelLayout::dispose();
}

void PageFooterPanel::Initialize()
{
    aCustomEntry = mpCustomEntry->GetText();
    mpFooterToggle->SetClickHdl( LINK(this, PageFooterPanel, FooterToggleHdl) );
    mpFooterMarginPresetLB->SetSelectHdl( LINK(this, PageFooterPanel, FooterLRMarginHdl));
    mpFooterSpacingLB->SetSelectHdl( LINK(this, PageFooterPanel, FooterSpacingHdl));
    mpFooterLayoutLB->SetSelectHdl( LINK(this, PageFooterPanel, FooterLayoutHdl));

    mpBindings->Invalidate(SID_ATTR_PAGE_FOOTER);
    mpBindings->Invalidate(SID_ATTR_PAGE_FOOTER_LRMARGIN);
    mpBindings->Invalidate(SID_ATTR_PAGE_FOOTER_SPACING);
    mpBindings->Invalidate(SID_ATTR_PAGE_FOOTER_LAYOUT);
}

void PageFooterPanel::UpdateFooterCheck()
{
    if(mpFooterToggle->IsChecked())
    {
        mpFooterSpacingLB->Enable();
        mpFooterLayoutLB->Enable();
        mpFooterMarginPresetLB->Enable();
    }
    else
    {
        mpFooterSpacingLB->Disable();
        mpFooterLayoutLB->Disable();
        mpFooterMarginPresetLB->Disable();
    }
}

void PageFooterPanel::UpdateMarginControl()
{
    sal_uInt16 nLeft = mpFooterLRMarginItem->GetLeft();
    sal_uInt16 nRight = mpFooterLRMarginItem->GetRight();
    sal_uInt16 nCount = mpFooterMarginPresetLB->GetEntryCount();
    if(nLeft == nRight)
    {
        for(sal_uInt16 i = 0; i < nCount; i++)
        {
            if(reinterpret_cast<sal_uLong>(mpFooterMarginPresetLB->GetEntryData(i)) == nLeft )
            {
                mpFooterMarginPresetLB->SelectEntryPos(i);
                mpFooterMarginPresetLB->RemoveEntry(aCustomEntry);
                return;
            }
        }
    }
    mpFooterMarginPresetLB->InsertEntry(aCustomEntry);
    mpFooterMarginPresetLB->SelectEntry(aCustomEntry);
}

void PageFooterPanel::UpdateSpacingControl()
{
    sal_uInt16 nBottom = mpFooterSpacingItem->GetUpper();
    sal_uInt16 nCount = mpFooterSpacingLB->GetEntryCount();
    for(sal_uInt16 i = 0; i < nCount; i++)
    {
        if(reinterpret_cast<sal_uLong>(mpFooterSpacingLB->GetEntryData(i)) == nBottom )
        {
            mpFooterSpacingLB->SelectEntryPos(i);
            mpFooterSpacingLB->RemoveEntry(aCustomEntry);
            return;
        }
    }
    mpFooterSpacingLB->InsertEntry(aCustomEntry);
    mpFooterSpacingLB->SelectEntry(aCustomEntry);
}

void PageFooterPanel::UpdateLayoutControl()
{
    sal_uInt16 nLayout = mpFooterLayoutItem->GetValue();
    mpFooterLayoutLB->SelectEntryPos( nLayout );
}

void PageFooterPanel::NotifyItemUpdate(
    const sal_uInt16 nSid,
    const SfxItemState eState,
    const SfxPoolItem* pState,
    const bool)
{
    if (IsDisposed())
        return;

    switch(nSid)
    {
        case SID_ATTR_PAGE_FOOTER:
        {
            if(eState >= SfxItemState::DEFAULT &&
                pState && dynamic_cast<const SfxBoolItem*>( pState) !=  nullptr )
            {
                mpFooterItem.reset( static_cast<SfxBoolItem*>(pState->Clone()) );
                mpFooterToggle->Check(mpFooterItem->GetValue());
                UpdateFooterCheck();
            }
        }
        break;
        case SID_ATTR_PAGE_FOOTER_LRMARGIN:
        {
            if(eState >= SfxItemState::DEFAULT &&
                pState && dynamic_cast<const SvxLongLRSpaceItem*>( pState) !=  nullptr )
            {
                mpFooterLRMarginItem.reset( static_cast<SvxLongLRSpaceItem*>(pState->Clone()) );
                UpdateMarginControl();
            }
        }
        break;
        case SID_ATTR_PAGE_FOOTER_SPACING:
        {
            if(eState >= SfxItemState::DEFAULT &&
                pState && dynamic_cast<const SvxLongULSpaceItem*>( pState) !=  nullptr )
            {
                mpFooterSpacingItem.reset(static_cast<SvxLongULSpaceItem*>(pState->Clone()) );
                UpdateSpacingControl();
            }
        }
        break;
        case SID_ATTR_PAGE_FOOTER_LAYOUT:
        {
            if(eState >= SfxItemState::DEFAULT &&
                pState && dynamic_cast<const SfxInt16Item*>( pState) !=  nullptr )
            {
                mpFooterLayoutItem.reset(static_cast<SfxInt16Item*>(pState->Clone()) );
                UpdateLayoutControl();
            }
        }
        break;
        default:
            break;
    }
}

IMPL_LINK_NOARG( PageFooterPanel, FooterToggleHdl, Button*, void )
{
    bool IsChecked = mpFooterToggle->IsChecked();
    mpFooterItem->SetValue(IsChecked);
    GetBindings()->GetDispatcher()->ExecuteList( SID_ATTR_PAGE_FOOTER, SfxCallMode::RECORD, { mpFooterItem.get() } );
    UpdateFooterCheck();
}

IMPL_LINK_NOARG( PageFooterPanel, FooterLRMarginHdl, ListBox&, void )
{
    sal_uInt16 nVal = static_cast<sal_uInt16>(reinterpret_cast<sal_uLong>(mpFooterMarginPresetLB->GetSelectedEntryData()));
    mpFooterLRMarginItem->SetLeft(nVal);
    mpFooterLRMarginItem->SetRight(nVal);
    GetBindings()->GetDispatcher()->ExecuteList( SID_ATTR_PAGE_FOOTER_LRMARGIN,
                                                 SfxCallMode::RECORD, { mpFooterLRMarginItem.get() } );
}

IMPL_LINK_NOARG( PageFooterPanel, FooterSpacingHdl, ListBox&, void )
{
    sal_uInt16 nVal = static_cast<sal_uInt16>(reinterpret_cast<sal_uLong>(mpFooterSpacingLB->GetSelectedEntryData()));
    mpFooterSpacingItem->SetUpper(nVal);
    GetBindings()->GetDispatcher()->ExecuteList( SID_ATTR_PAGE_FOOTER_SPACING,
                                                 SfxCallMode::RECORD, { mpFooterSpacingItem.get() } );

}
IMPL_LINK_NOARG( PageFooterPanel, FooterLayoutHdl, ListBox&, void )
{
    sal_uInt16 nVal = mpFooterLayoutLB->GetSelectedEntryPos();
    mpFooterLayoutItem->SetValue(nVal);
    GetBindings()->GetDispatcher()->ExecuteList( SID_ATTR_PAGE_FOOTER_LAYOUT,
                                                 SfxCallMode::RECORD, { mpFooterLayoutItem.get() } );
}


} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
