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
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <svx/dlgutil.hxx>
#include <svx/rulritem.hxx>
#include <svx/svdtrans.hxx>
#include <svx/spacinglistbox.hxx>
#include <svx/samecontentlistbox.hxx>
#include "PageFooterPanel.hxx"
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <cmdid.h>

#include <com/sun/star/lang/IllegalArgumentException.hpp>

namespace sw::sidebar{

VclPtr<PanelLayout> PageFooterPanel::Create(
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

void PageFooterPanel::SetMarginsAndSpacingFieldUnit()
{
    SpacingListBox::Fill(IsInch(meFUnit) ? SpacingType::SPACING_INCH : SpacingType::SPACING_CM, *mxFooterSpacingLB);
    SpacingListBox::Fill(IsInch(meFUnit) ? SpacingType::MARGINS_INCH : SpacingType::MARGINS_CM, *mxFooterMarginPresetLB);
}

PageFooterPanel::PageFooterPanel(
    vcl::Window* pParent,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
    SfxBindings* pBindings) :
    PanelLayout(pParent, "PageFooterPanel", "modules/swriter/ui/pagefooterpanel.ui", rxFrame),
    mpBindings( pBindings ),
    maHFToggleController(SID_ATTR_PAGE_FOOTER, *pBindings, *this),
    maMetricController(SID_ATTR_METRIC, *pBindings,*this),
    maFooterLRMarginController(SID_ATTR_PAGE_FOOTER_LRMARGIN, *pBindings, *this),
    maFooterSpacingController(SID_ATTR_PAGE_FOOTER_SPACING, *pBindings, *this),
    maFooterLayoutController(SID_ATTR_PAGE_FOOTER_LAYOUT, *pBindings, *this),
    meFUnit(GetModuleFieldUnit()),
    aCustomEntry(),
    mpFooterItem( new SfxBoolItem(SID_ATTR_PAGE_FOOTER) ),
    mpFooterLRMarginItem( new SvxLongLRSpaceItem(0, 0, SID_ATTR_PAGE_FOOTER_LRMARGIN)),
    mpFooterSpacingItem( new SvxLongULSpaceItem(0, 0, SID_ATTR_PAGE_FOOTER_SPACING)),
    mpFooterLayoutItem( new SfxInt16Item(SID_ATTR_PAGE_FOOTER_LAYOUT)),
    mxFooterToggle(m_xBuilder->weld_check_button("footertoggle")),
    mxFooterSpacingLB(m_xBuilder->weld_combo_box("spacingpreset")),
    mxFooterMarginPresetLB(m_xBuilder->weld_combo_box("footermarginpreset")),
    mxFooterLayoutLB(m_xBuilder->weld_combo_box("samecontentLB")),
    mxCustomEntry(m_xBuilder->weld_label("customlabel"))
{
    Initialize();
}

PageFooterPanel::~PageFooterPanel()
{
    disposeOnce();
}

void PageFooterPanel::dispose()
{
    mxFooterToggle.reset();
    maMetricController.dispose();
    mxFooterSpacingLB.reset();
    mxFooterLayoutLB.reset();
    mxFooterMarginPresetLB.reset();
    mxCustomEntry.reset();

    PanelLayout::dispose();
}

FieldUnit PageFooterPanel::GetCurrentUnit(SfxItemState eState, const SfxPoolItem* pState)
{
    FieldUnit eUnit;

    if (pState && eState >= SfxItemState::DEFAULT)
        eUnit = static_cast<FieldUnit>(static_cast<const SfxUInt16Item*>(pState)->GetValue());
    else
        eUnit = GetModuleFieldUnit();

    return eUnit;
}

void PageFooterPanel::Initialize()
{
    SameContentListBox::Fill(*mxFooterLayoutLB);

    SetMarginsAndSpacingFieldUnit();

    aCustomEntry = mxCustomEntry->get_label();
    mxFooterToggle->connect_toggled( LINK(this, PageFooterPanel, FooterToggleHdl) );
    mxFooterMarginPresetLB->connect_changed( LINK(this, PageFooterPanel, FooterLRMarginHdl));
    mxFooterSpacingLB->connect_changed( LINK(this, PageFooterPanel, FooterSpacingHdl));
    mxFooterLayoutLB->connect_changed( LINK(this, PageFooterPanel, FooterLayoutHdl));

    mpBindings->Invalidate(SID_ATTR_METRIC);
    mpBindings->Invalidate(SID_ATTR_PAGE_FOOTER);
    mpBindings->Invalidate(SID_ATTR_PAGE_FOOTER_LRMARGIN);
    mpBindings->Invalidate(SID_ATTR_PAGE_FOOTER_SPACING);
    mpBindings->Invalidate(SID_ATTR_PAGE_FOOTER_LAYOUT);
}

void PageFooterPanel::UpdateFooterCheck()
{
    if (mxFooterToggle->get_active())
    {
        mxFooterSpacingLB->set_sensitive(true);
        mxFooterLayoutLB->set_sensitive(true);
        mxFooterMarginPresetLB->set_sensitive(true);
    }
    else
    {
        mxFooterSpacingLB->set_sensitive(false);
        mxFooterLayoutLB->set_sensitive(false);
        mxFooterMarginPresetLB->set_sensitive(false);
    }
}

void PageFooterPanel::UpdateMarginControl()
{
    sal_uInt16 nLeft = mpFooterLRMarginItem->GetLeft();
    sal_uInt16 nRight = mpFooterLRMarginItem->GetRight();
    sal_uInt16 nCount = mxFooterMarginPresetLB->get_count();
    if(nLeft == nRight)
    {
        for (sal_uInt16 i = 0; i < nCount; ++i)
        {
            if (mxFooterMarginPresetLB->get_id(i).toUInt32() == nLeft)
            {
                mxFooterMarginPresetLB->set_active(i);
                int nCustomEntry = mxFooterMarginPresetLB->find_text(aCustomEntry);
                if (nCustomEntry != -1)
                    mxFooterMarginPresetLB->remove(nCustomEntry);
                return;
            }
        }
    }
    mxFooterMarginPresetLB->append_text(aCustomEntry);
    mxFooterMarginPresetLB->set_active_text(aCustomEntry);
}

void PageFooterPanel::UpdateSpacingControl()
{
    sal_uInt16 nBottom = mpFooterSpacingItem->GetUpper();
    sal_uInt16 nCount = mxFooterSpacingLB->get_count();
    for (sal_uInt16 i = 0; i < nCount; ++i)
    {
        if (mxFooterSpacingLB->get_id(i).toUInt32() == nBottom)
        {
            mxFooterSpacingLB->set_active(i);
            int nCustomEntry = mxFooterSpacingLB->find_text(aCustomEntry);
            if (nCustomEntry != -1)
                mxFooterSpacingLB->remove(nCustomEntry);
            return;
        }
    }
    mxFooterSpacingLB->append_text(aCustomEntry);
    mxFooterSpacingLB->set_active_text(aCustomEntry);
}

void PageFooterPanel::UpdateLayoutControl()
{
    sal_uInt16 nLayout = mpFooterLayoutItem->GetValue();
    mxFooterLayoutLB->set_active(nLayout);
}

void PageFooterPanel::NotifyItemUpdate(
    const sal_uInt16 nSid,
    const SfxItemState eState,
    const SfxPoolItem* pState)
{
    if (IsDisposed())
        return;

    switch(nSid)
    {
        case SID_ATTR_PAGE_FOOTER:
        {
            if(eState >= SfxItemState::DEFAULT &&
                dynamic_cast<const SfxBoolItem*>( pState) )
            {
                mpFooterItem.reset( static_cast<SfxBoolItem*>(pState->Clone()) );
                mxFooterToggle->set_active(mpFooterItem->GetValue());
                UpdateFooterCheck();
            }
        }
        break;
        case SID_ATTR_PAGE_FOOTER_LRMARGIN:
        {
            if(eState >= SfxItemState::DEFAULT &&
                dynamic_cast<const SvxLongLRSpaceItem*>( pState) )
            {
                mpFooterLRMarginItem.reset( static_cast<SvxLongLRSpaceItem*>(pState->Clone()) );
                UpdateMarginControl();
            }
        }
        break;
        case SID_ATTR_PAGE_FOOTER_SPACING:
        {
            if(eState >= SfxItemState::DEFAULT &&
                dynamic_cast<const SvxLongULSpaceItem*>( pState) )
            {
                mpFooterSpacingItem.reset(static_cast<SvxLongULSpaceItem*>(pState->Clone()) );
                UpdateSpacingControl();
            }
        }
        break;
        case SID_ATTR_PAGE_FOOTER_LAYOUT:
        {
            if(eState >= SfxItemState::DEFAULT &&
                dynamic_cast<const SfxInt16Item*>( pState) )
            {
                mpFooterLayoutItem.reset(static_cast<SfxInt16Item*>(pState->Clone()) );
                UpdateLayoutControl();
            }
        }
        break;
        case SID_ATTR_METRIC:
        {
            FieldUnit eFUnit = GetCurrentUnit(eState, pState);
            if (meFUnit != eFUnit)
            {
                meFUnit = eFUnit;
                SetMarginsAndSpacingFieldUnit();
                UpdateSpacingControl();
                UpdateMarginControl();
            }
        }
        break;
        default:
            break;
    }
}

IMPL_LINK_NOARG( PageFooterPanel, FooterToggleHdl, weld::ToggleButton&, void )
{
    bool IsChecked = mxFooterToggle->get_active();
    mpFooterItem->SetValue(IsChecked);
    GetBindings()->GetDispatcher()->ExecuteList( SID_ATTR_PAGE_FOOTER, SfxCallMode::RECORD, { mpFooterItem.get() } );
    UpdateFooterCheck();
}

IMPL_LINK_NOARG( PageFooterPanel, FooterLRMarginHdl, weld::ComboBox&, void )
{
    sal_uInt16 nVal = mxFooterMarginPresetLB->get_active_id().toUInt32();
    mpFooterLRMarginItem->SetLeft(nVal);
    mpFooterLRMarginItem->SetRight(nVal);
    GetBindings()->GetDispatcher()->ExecuteList( SID_ATTR_PAGE_FOOTER_LRMARGIN,
                                                 SfxCallMode::RECORD, { mpFooterLRMarginItem.get() } );
}

IMPL_LINK_NOARG( PageFooterPanel, FooterSpacingHdl, weld::ComboBox&, void )
{
    sal_uInt16 nVal = mxFooterSpacingLB->get_active_id().toUInt32();
    mpFooterSpacingItem->SetUpper(nVal);
    GetBindings()->GetDispatcher()->ExecuteList( SID_ATTR_PAGE_FOOTER_SPACING,
                                                 SfxCallMode::RECORD, { mpFooterSpacingItem.get() } );

}
IMPL_LINK_NOARG( PageFooterPanel, FooterLayoutHdl, weld::ComboBox&, void )
{
    sal_uInt16 nVal = mxFooterLayoutLB->get_active();
    mpFooterLayoutItem->SetValue(nVal);
    GetBindings()->GetDispatcher()->ExecuteList( SID_ATTR_PAGE_FOOTER_LAYOUT,
                                                 SfxCallMode::RECORD, { mpFooterLayoutItem.get() } );
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
