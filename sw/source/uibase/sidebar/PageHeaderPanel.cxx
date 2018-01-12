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
#include "PageHeaderPanel.hxx"
#include <sfx2/sidebar/ControlFactory.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>
#include <cmdid.h>

namespace sw { namespace sidebar{

VclPtr<vcl::Window> PageHeaderPanel::Create(
    vcl::Window* pParent,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
    SfxBindings* pBindings)
{
    if( pParent == nullptr )
        throw ::com::sun::star::lang::IllegalArgumentException("no parent window given to PageHeaderPanel::Create", nullptr, 0);
    if( !rxFrame.is() )
        throw ::com::sun::star::lang::IllegalArgumentException("no XFrame given to PageHeaderPanel::Create", nullptr, 0);
    if( pBindings == nullptr )
        throw ::com::sun::star::lang::IllegalArgumentException("no SfxBindings given to PageHeaderPanel::Create", nullptr, 0);

    return VclPtr<PageHeaderPanel>::Create(pParent, rxFrame, pBindings);
}

PageHeaderPanel::PageHeaderPanel(
    vcl::Window* pParent,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
    SfxBindings* pBindings
    ) :
    PanelLayout(pParent, "PageHeaderPanel", "modules/swriter/ui/pageheaderpanel.ui", rxFrame),
    mpBindings( pBindings ),
    maHFToggleController(SID_ATTR_PAGE_HEADER, *pBindings, *this),
    maHeaderLRMarginController(SID_ATTR_PAGE_HEADER_LRMARGIN, *pBindings, *this),
    maHeaderSpacingController(SID_ATTR_PAGE_HEADER_SPACING, *pBindings, *this),
    maHeaderLayoutController(SID_ATTR_PAGE_HEADER_LAYOUT, *pBindings, *this),
    aCustomEntry(),
    mpHeaderItem( new SfxBoolItem(SID_ATTR_PAGE_HEADER) ),
    mpHeaderLRMarginItem( new SvxLongLRSpaceItem(0, 0, SID_ATTR_PAGE_HEADER_LRMARGIN)),
    mpHeaderSpacingItem( new SvxLongULSpaceItem(0, 0, SID_ATTR_PAGE_HEADER_SPACING)),
    mpHeaderLayoutItem( new SfxInt16Item(SID_ATTR_PAGE_HEADER_LAYOUT))
{
    get(mpHeaderToggle, "headertoggle");
    get(mpHeaderSpacingLB, "spacingpreset");
    get(mpHeaderLayoutLB, "samecontentLB");
    get(mpHeaderMarginPresetLB, "headermarginpreset");
    get(mpCustomEntry, "customlabel");

    Initialize();
}

PageHeaderPanel::~PageHeaderPanel()
{
    disposeOnce();
}

void PageHeaderPanel::dispose()
{
    mpHeaderToggle.disposeAndClear();
    mpHeaderSpacingLB.disposeAndClear();
    mpHeaderLayoutLB.disposeAndClear();
    mpHeaderMarginPresetLB.disposeAndClear();
    mpCustomEntry.clear();

    PanelLayout::dispose();
}

void PageHeaderPanel::Initialize()
{
    aCustomEntry = mpCustomEntry->GetText();
    mpHeaderToggle->SetClickHdl( LINK(this, PageHeaderPanel, HeaderToggleHdl) );
    mpHeaderMarginPresetLB->SetSelectHdl( LINK(this, PageHeaderPanel, HeaderLRMarginHdl));
    mpHeaderSpacingLB->SetSelectHdl( LINK(this, PageHeaderPanel, HeaderSpacingHdl));
    mpHeaderLayoutLB->SetSelectHdl( LINK(this, PageHeaderPanel, HeaderLayoutHdl));

    mpBindings->Invalidate(SID_ATTR_PAGE_HEADER);
    mpBindings->Invalidate(SID_ATTR_PAGE_HEADER_LRMARGIN);
    mpBindings->Invalidate(SID_ATTR_PAGE_HEADER_SPACING);
    mpBindings->Invalidate(SID_ATTR_PAGE_HEADER_LAYOUT);
}

void PageHeaderPanel::UpdateHeaderCheck()
{
    if(mpHeaderToggle->IsChecked())
    {
        mpHeaderSpacingLB->Enable();
        mpHeaderLayoutLB->Enable();
        mpHeaderMarginPresetLB->Enable();
    }
    else
    {
        mpHeaderSpacingLB->Disable();
        mpHeaderLayoutLB->Disable();
        mpHeaderMarginPresetLB->Disable();
    }
}

void PageHeaderPanel::UpdateMarginControl()
{
    sal_uInt16 nLeft = mpHeaderLRMarginItem->GetLeft();
    sal_uInt16 nRight = mpHeaderLRMarginItem->GetRight();
    sal_uInt16 nCount = mpHeaderMarginPresetLB->GetEntryCount();
    if(nLeft == nRight)
    {
        for(sal_uInt16 i = 0; i < nCount; i++)
        {
            if(reinterpret_cast<sal_uLong>(mpHeaderMarginPresetLB->GetEntryData(i)) == nLeft )
            {
                mpHeaderMarginPresetLB->SelectEntryPos(i);
                mpHeaderMarginPresetLB->RemoveEntry(aCustomEntry);
                return;
            }
        }
    }
    mpHeaderMarginPresetLB->InsertEntry(aCustomEntry);
    mpHeaderMarginPresetLB->SelectEntry(aCustomEntry);
}

void PageHeaderPanel::UpdateSpacingControl()
{
    sal_uInt16 nBottom = mpHeaderSpacingItem->GetLower();
    sal_uInt16 nCount = mpHeaderSpacingLB->GetEntryCount();
    for(sal_uInt16 i = 0; i < nCount; i++)
    {
        if(reinterpret_cast<sal_uLong>(mpHeaderSpacingLB->GetEntryData(i)) == nBottom )
        {
            mpHeaderSpacingLB->SelectEntryPos(i);
            mpHeaderSpacingLB->RemoveEntry(aCustomEntry);
            return;
        }
    }
    mpHeaderSpacingLB->InsertEntry(aCustomEntry);
    mpHeaderSpacingLB->SelectEntry(aCustomEntry);
}

void PageHeaderPanel::UpdateLayoutControl()
{
    sal_uInt16 nLayout = mpHeaderLayoutItem->GetValue();
    mpHeaderLayoutLB->SelectEntryPos( nLayout );
}

void PageHeaderPanel::NotifyItemUpdate(
    const sal_uInt16 nSid,
    const SfxItemState eState,
    const SfxPoolItem* pState,
    const bool)
{
    if (IsDisposed())
        return;

    switch(nSid)
    {
        case SID_ATTR_PAGE_HEADER:
        {
            if(eState >= SfxItemState::DEFAULT &&
                pState && dynamic_cast<const SfxBoolItem*>( pState) !=  nullptr )
            {
                mpHeaderItem.reset( static_cast<SfxBoolItem*>(pState->Clone()) );
                mpHeaderToggle->Check(mpHeaderItem->GetValue());
                UpdateHeaderCheck();
            }
        }
        break;
        case SID_ATTR_PAGE_HEADER_LRMARGIN:
        {
            if(eState >= SfxItemState::DEFAULT &&
                pState && dynamic_cast<const SvxLongLRSpaceItem*>( pState) !=  nullptr )
            {
                mpHeaderLRMarginItem.reset( static_cast<SvxLongLRSpaceItem*>(pState->Clone()) );
                UpdateMarginControl();
            }
        }
        break;
        case SID_ATTR_PAGE_HEADER_SPACING:
        {
            if(eState >= SfxItemState::DEFAULT &&
                pState && dynamic_cast<const SvxLongULSpaceItem*>( pState) !=  nullptr )
            {
                mpHeaderSpacingItem.reset(static_cast<SvxLongULSpaceItem*>(pState->Clone()) );
                UpdateSpacingControl();
            }
        }
        break;
        case SID_ATTR_PAGE_HEADER_LAYOUT:
        {
            if(eState >= SfxItemState::DEFAULT &&
                pState && dynamic_cast<const SfxInt16Item*>( pState) !=  nullptr )
            {
                mpHeaderLayoutItem.reset(static_cast<SfxInt16Item*>(pState->Clone()) );
                UpdateLayoutControl();
            }
        }
        break;
        default:
            break;
    }
}

IMPL_LINK_NOARG( PageHeaderPanel, HeaderToggleHdl, Button*, void )
{
    bool IsChecked = mpHeaderToggle->IsChecked();
    mpHeaderItem->SetValue(IsChecked);
    GetBindings()->GetDispatcher()->ExecuteList( SID_ATTR_PAGE_HEADER, SfxCallMode::RECORD, { mpHeaderItem.get() } );
    UpdateHeaderCheck();
}

IMPL_LINK_NOARG( PageHeaderPanel, HeaderLRMarginHdl, ListBox&, void )
{
    sal_uInt16 nVal = static_cast<sal_uInt16>(reinterpret_cast<sal_uLong>(mpHeaderMarginPresetLB->GetSelectedEntryData()));
    mpHeaderLRMarginItem->SetLeft(nVal);
    mpHeaderLRMarginItem->SetRight(nVal);
    GetBindings()->GetDispatcher()->ExecuteList( SID_ATTR_PAGE_HEADER_LRMARGIN,
                                                 SfxCallMode::RECORD, { mpHeaderLRMarginItem.get() } );
}

IMPL_LINK_NOARG( PageHeaderPanel, HeaderSpacingHdl, ListBox&, void )
{
    sal_uInt16 nVal = static_cast<sal_uInt16>(reinterpret_cast<sal_uLong>(mpHeaderSpacingLB->GetSelectedEntryData()));
    mpHeaderSpacingItem->SetLower(nVal);
    GetBindings()->GetDispatcher()->ExecuteList( SID_ATTR_PAGE_HEADER_SPACING,
                                                 SfxCallMode::RECORD, { mpHeaderSpacingItem.get() } );
}
IMPL_LINK_NOARG( PageHeaderPanel, HeaderLayoutHdl, ListBox&, void )
{
    sal_uInt16 nVal = mpHeaderLayoutLB->GetSelectedEntryPos();
    mpHeaderLayoutItem->SetValue(nVal);
    GetBindings()->GetDispatcher()->ExecuteList( SID_ATTR_PAGE_HEADER_LAYOUT,
                                                 SfxCallMode::RECORD, { mpHeaderLayoutItem.get() } );
}


} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
