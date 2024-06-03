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
#include <svl/itemset.hxx>
#include <svx/dlgutil.hxx>
#include <svx/rulritem.hxx>
#include <svx/svdtrans.hxx>
#include <svx/spacinglistbox.hxx>
#include <svx/samecontentlistbox.hxx>
#include "PageHeaderPanel.hxx"
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <cmdid.h>

#include <com/sun/star/lang/IllegalArgumentException.hpp>

namespace sw::sidebar{

std::unique_ptr<PanelLayout> PageHeaderPanel::Create(
    weld::Widget* pParent,
    SfxBindings* pBindings)
{
    if( pParent == nullptr )
        throw ::com::sun::star::lang::IllegalArgumentException(u"no parent window given to PageHeaderPanel::Create"_ustr, nullptr, 0);
    if( pBindings == nullptr )
        throw ::com::sun::star::lang::IllegalArgumentException(u"no SfxBindings given to PageHeaderPanel::Create"_ustr, nullptr, 0);

    return std::make_unique<PageHeaderPanel>(pParent, pBindings);
}

void PageHeaderPanel::SetMarginsAndSpacingFieldUnit()
{
    SpacingListBox::Fill(IsInch(meFUnit) ? SpacingType::SPACING_INCH : SpacingType::SPACING_CM, *mxHeaderSpacingLB);
    SpacingListBox::Fill(IsInch(meFUnit) ? SpacingType::MARGINS_INCH : SpacingType::MARGINS_CM, *mxHeaderMarginPresetLB);
}

PageHeaderPanel::PageHeaderPanel(
    weld::Widget* pParent,
    SfxBindings* pBindings
    ) :
    PanelLayout(pParent, u"PageHeaderPanel"_ustr, u"modules/swriter/ui/pageheaderpanel.ui"_ustr),
    mpBindings( pBindings ),
    maHFToggleController(SID_ATTR_PAGE_HEADER, *pBindings, *this),
    maMetricController(SID_ATTR_METRIC, *pBindings,*this),
    maHeaderLRMarginController(SID_ATTR_PAGE_HEADER_LRMARGIN, *pBindings, *this),
    maHeaderSpacingController(SID_ATTR_PAGE_HEADER_SPACING, *pBindings, *this),
    maHeaderLayoutController(SID_ATTR_PAGE_HEADER_LAYOUT, *pBindings, *this),
    meFUnit(GetModuleFieldUnit()),
    mpHeaderItem( new SfxBoolItem(SID_ATTR_PAGE_HEADER) ),
    mpHeaderLRMarginItem( new SvxLongLRSpaceItem(0, 0, SID_ATTR_PAGE_HEADER_LRMARGIN)),
    mpHeaderSpacingItem( new SvxLongULSpaceItem(0, 0, SID_ATTR_PAGE_HEADER_SPACING)),
    mpHeaderLayoutItem( new SfxInt16Item(SID_ATTR_PAGE_HEADER_LAYOUT)),
    mxHeaderToggle(m_xBuilder->weld_check_button(u"headertoggle"_ustr)),
    mxHeaderSpacingLB(m_xBuilder->weld_combo_box(u"spacingpreset"_ustr)),
    mxHeaderMarginPresetLB(m_xBuilder->weld_combo_box(u"headermarginpreset"_ustr)),
    mxHeaderLayoutLB(m_xBuilder->weld_combo_box(u"samecontentLB"_ustr)),
    mxCustomEntry(m_xBuilder->weld_label(u"customlabel"_ustr))
{
    Initialize();
}

PageHeaderPanel::~PageHeaderPanel()
{
    mxHeaderToggle.reset();
    mxHeaderSpacingLB.reset();
    mxHeaderLayoutLB.reset();
    mxHeaderMarginPresetLB.reset();
    mxCustomEntry.reset();
}

FieldUnit PageHeaderPanel::GetCurrentUnit(SfxItemState eState, const SfxPoolItem* pState)
{
    FieldUnit eUnit;

    if (pState && eState >= SfxItemState::DEFAULT)
        eUnit = static_cast<FieldUnit>(static_cast<const SfxUInt16Item*>(pState)->GetValue());
    else
        eUnit = GetModuleFieldUnit();

    return eUnit;
}

void PageHeaderPanel::Initialize()
{
    SameContentListBox::Fill(*mxHeaderLayoutLB);

    SetMarginsAndSpacingFieldUnit();

    m_aCustomEntry = mxCustomEntry->get_label();
    mxHeaderToggle->connect_toggled( LINK(this, PageHeaderPanel, HeaderToggleHdl) );
    mxHeaderMarginPresetLB->connect_changed( LINK(this, PageHeaderPanel, HeaderLRMarginHdl));
    mxHeaderSpacingLB->connect_changed( LINK(this, PageHeaderPanel, HeaderSpacingHdl));
    mxHeaderLayoutLB->connect_changed( LINK(this, PageHeaderPanel, HeaderLayoutHdl));

    mpBindings->Invalidate(SID_ATTR_METRIC);
    mpBindings->Invalidate(SID_ATTR_PAGE_HEADER);
    mpBindings->Invalidate(SID_ATTR_PAGE_HEADER_LRMARGIN);
    mpBindings->Invalidate(SID_ATTR_PAGE_HEADER_SPACING);
    mpBindings->Invalidate(SID_ATTR_PAGE_HEADER_LAYOUT);
}

void PageHeaderPanel::UpdateHeaderCheck()
{
    if (mxHeaderToggle->get_active())
    {
        mxHeaderSpacingLB->set_sensitive(true);
        mxHeaderLayoutLB->set_sensitive(true);
        mxHeaderMarginPresetLB->set_sensitive(true);
    }
    else
    {
        mxHeaderSpacingLB->set_sensitive(false);
        mxHeaderLayoutLB->set_sensitive(false);
        mxHeaderMarginPresetLB->set_sensitive(false);
    }
}

void PageHeaderPanel::UpdateMarginControl()
{
    sal_uInt16 nLeft = mpHeaderLRMarginItem->GetLeft();
    sal_uInt16 nRight = mpHeaderLRMarginItem->GetRight();
    sal_uInt16 nCount = mxHeaderMarginPresetLB->get_count();
    if(nLeft == nRight)
    {
        for (sal_uInt16 i = 0; i < nCount; ++i)
        {
            if (mxHeaderMarginPresetLB->get_id(i).toUInt32() == nLeft)
            {
                mxHeaderMarginPresetLB->set_active(i);
                int nCustomEntry = mxHeaderMarginPresetLB->find_text(m_aCustomEntry);
                if (nCustomEntry != -1)
                    mxHeaderMarginPresetLB->remove(nCustomEntry);
                return;
            }
        }
    }
    mxHeaderMarginPresetLB->append_text(m_aCustomEntry);
    mxHeaderMarginPresetLB->set_active_text(m_aCustomEntry);
}

void PageHeaderPanel::UpdateSpacingControl()
{
    sal_uInt16 nBottom = mpHeaderSpacingItem->GetLower();
    sal_uInt16 nCount = mxHeaderSpacingLB->get_count();
    for (sal_uInt16 i = 0; i < nCount; ++i)
    {
        if (mxHeaderSpacingLB->get_id(i).toUInt32() == nBottom)
        {
            mxHeaderSpacingLB->set_active(i);
            int nCustomEntry = mxHeaderSpacingLB->find_text(m_aCustomEntry);
            if (nCustomEntry != -1)
                mxHeaderSpacingLB->remove(nCustomEntry);
            return;
        }
    }
    mxHeaderSpacingLB->append_text(m_aCustomEntry);
    mxHeaderSpacingLB->set_active_text(m_aCustomEntry);
}

void PageHeaderPanel::UpdateLayoutControl()
{
    sal_uInt16 nLayout = mpHeaderLayoutItem->GetValue();
    mxHeaderLayoutLB->set_active(nLayout);
}

void PageHeaderPanel::NotifyItemUpdate(
    const sal_uInt16 nSid,
    const SfxItemState eState,
    const SfxPoolItem* pState)
{
    if (!mxHeaderToggle) //disposed
        return;

    switch(nSid)
    {
        case SID_ATTR_PAGE_HEADER:
        {
            if(eState >= SfxItemState::DEFAULT &&
                dynamic_cast<const SfxBoolItem*>( pState) )
            {
                mpHeaderItem.reset( static_cast<SfxBoolItem*>(pState->Clone()) );
                mxHeaderToggle->set_active(mpHeaderItem->GetValue());
                UpdateHeaderCheck();
            }
        }
        break;
        case SID_ATTR_PAGE_HEADER_LRMARGIN:
        {
            if(eState >= SfxItemState::DEFAULT &&
                dynamic_cast<const SvxLongLRSpaceItem*>( pState) )
            {
                mpHeaderLRMarginItem.reset( static_cast<SvxLongLRSpaceItem*>(pState->Clone()) );
                UpdateMarginControl();
            }
        }
        break;
        case SID_ATTR_PAGE_HEADER_SPACING:
        {
            if(eState >= SfxItemState::DEFAULT &&
                dynamic_cast<const SvxLongULSpaceItem*>( pState) )
            {
                mpHeaderSpacingItem.reset(static_cast<SvxLongULSpaceItem*>(pState->Clone()) );
                UpdateSpacingControl();
            }
        }
        break;
        case SID_ATTR_PAGE_HEADER_LAYOUT:
        {
            if(eState >= SfxItemState::DEFAULT &&
                dynamic_cast<const SfxInt16Item*>( pState) )
            {
                mpHeaderLayoutItem.reset(static_cast<SfxInt16Item*>(pState->Clone()) );
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

IMPL_LINK_NOARG( PageHeaderPanel, HeaderToggleHdl, weld::Toggleable&, void )
{
    bool IsChecked = mxHeaderToggle->get_active();
    mpHeaderItem->SetValue(IsChecked);
    GetBindings()->GetDispatcher()->ExecuteList( SID_ATTR_PAGE_HEADER, SfxCallMode::RECORD, { mpHeaderItem.get() } );
    UpdateHeaderCheck();
}

IMPL_LINK_NOARG( PageHeaderPanel, HeaderLRMarginHdl, weld::ComboBox&, void )
{
    sal_uInt16 nVal = mxHeaderMarginPresetLB->get_active_id().toUInt32();
    mpHeaderLRMarginItem->SetLeft(nVal);
    mpHeaderLRMarginItem->SetRight(nVal);
    GetBindings()->GetDispatcher()->ExecuteList( SID_ATTR_PAGE_HEADER_LRMARGIN,
                                                 SfxCallMode::RECORD, { mpHeaderLRMarginItem.get() } );
}

IMPL_LINK_NOARG( PageHeaderPanel, HeaderSpacingHdl, weld::ComboBox&, void )
{
    sal_uInt16 nVal = mxHeaderSpacingLB->get_active_id().toUInt32();
    mpHeaderSpacingItem->SetLower(nVal);
    GetBindings()->GetDispatcher()->ExecuteList( SID_ATTR_PAGE_HEADER_SPACING,
                                                 SfxCallMode::RECORD, { mpHeaderSpacingItem.get() } );
}
IMPL_LINK_NOARG( PageHeaderPanel, HeaderLayoutHdl, weld::ComboBox&, void )
{
    sal_uInt16 nVal = mxHeaderLayoutLB->get_active();
    mpHeaderLayoutItem->SetValue(nVal);
    GetBindings()->GetDispatcher()->ExecuteList( SID_ATTR_PAGE_HEADER_LAYOUT,
                                                 SfxCallMode::RECORD, { mpHeaderLayoutItem.get() } );
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
