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

#include "WrapPropertyPanel.hxx"

#include <editeng/editids.hrc>
#include <svx/spacinglistbox.hxx>
#include <svx/svdtrans.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <svl/itemset.hxx>
#include <hintids.hxx>
#include <strings.hrc>
#include <uitool.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

namespace sw::sidebar {

std::unique_ptr<PanelLayout> WrapPropertyPanel::Create (
    weld::Widget* pParent,
    const css::uno::Reference< css::frame::XFrame >& rxFrame,
    SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw css::lang::IllegalArgumentException(u"no parent Window given to WrapPropertyPanel::Create"_ustr, nullptr, 0);
    if ( ! rxFrame.is())
        throw css::lang::IllegalArgumentException(u"no XFrame given to WrapPropertyPanel::Create"_ustr, nullptr, 1);
    if (pBindings == nullptr)
        throw css::lang::IllegalArgumentException(u"no SfxBindings given to WrapPropertyPanel::Create"_ustr, nullptr, 2);

    return std::make_unique<WrapPropertyPanel>(pParent, rxFrame, pBindings);
}

WrapPropertyPanel::WrapPropertyPanel(
    weld::Widget* pParent,
    const css::uno::Reference< css::frame::XFrame >& rxFrame,
    SfxBindings* pBindings )
    : PanelLayout(pParent, u"WrapPropertyPanel"_ustr, u"modules/swriter/ui/sidebarwrap.ui"_ustr)
    , mpBindings(pBindings)
    // spacing
    , m_nTop(0)
    , m_nBottom(0)
    , m_nLeft(0)
    , m_nRight(0)
    // resources
    , m_aCustomEntry(SwResId(STR_WRAP_PANEL_CUSTOM_STR))
    // controller items
    , maSwLRSpacingControl(SID_ATTR_LRSPACE, *pBindings, *this)
    , maSwULSpacingControl(SID_ATTR_ULSPACE, *pBindings, *this)
    , mxWrapOptions(m_xBuilder->weld_toolbar(u"wrapoptions"_ustr))
    , mxWrapOptionsDispatch(new ToolbarUnoDispatcher(*mxWrapOptions, *m_xBuilder, rxFrame))
    , mxSpacingLB(m_xBuilder->weld_combo_box(u"spacingLB"_ustr))
{
    FieldUnit eMetric = ::GetDfltMetric(false);
    SpacingListBox::Fill(IsInch(eMetric) ? SpacingType::SPACING_INCH : SpacingType::SPACING_CM, *mxSpacingLB);

    Initialize();
}

WrapPropertyPanel::~WrapPropertyPanel()
{

    mxWrapOptionsDispatch.reset();
    mxWrapOptions.reset();

    maSwLRSpacingControl.dispose();
    maSwULSpacingControl.dispose();
}

void WrapPropertyPanel::Initialize()
{
    mxSpacingLB->connect_changed(LINK(this, WrapPropertyPanel, SpacingLBHdl));

    mpBindings->Update( SID_ATTR_LRSPACE );
    mpBindings->Update( SID_ATTR_ULSPACE );
}

void WrapPropertyPanel::UpdateSpacingLB()
{
    if( (m_nLeft == m_nRight) && (m_nTop == m_nBottom) && (m_nLeft == m_nTop) )
    {
        sal_Int32 nCount = mxSpacingLB->get_count();
        for (sal_Int32 i = 0; i < nCount; i++)
        {
            if (mxSpacingLB->get_id(i).toUInt32() == m_nLeft)
            {
                mxSpacingLB->set_active(i);
                int nCustomEntry = mxSpacingLB->find_text(m_aCustomEntry);
                if (nCustomEntry != -1)
                    mxSpacingLB->remove(nCustomEntry);
                return;
            }
        }
    }

    if (mxSpacingLB->find_text(m_aCustomEntry) == -1)
        mxSpacingLB->append_text(m_aCustomEntry);
    mxSpacingLB->set_active_text(m_aCustomEntry);
}

IMPL_LINK(WrapPropertyPanel, SpacingLBHdl, weld::ComboBox&, rBox, void)
{
    sal_uInt16 nVal = rBox.get_active_id().toUInt32();

    SvxLRSpaceItem aLRItem(nVal, nVal, 0, RES_LR_SPACE);
    SvxULSpaceItem aULItem(nVal, nVal, RES_UL_SPACE);

    m_nTop = m_nBottom = m_nLeft = m_nRight = nVal;
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_LRSPACE,
            SfxCallMode::RECORD, { &aLRItem });
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_ULSPACE,
            SfxCallMode::RECORD, { &aULItem });
}

void WrapPropertyPanel::NotifyItemUpdate(
    const sal_uInt16 nSId,
    const SfxItemState eState,
    const SfxPoolItem* pState)
{
    switch(nSId)
    {
        case SID_ATTR_LRSPACE:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                const SvxLRSpaceItem* pItem = dynamic_cast< const SvxLRSpaceItem* >(pState);
                if(pItem)
                {
                    m_nLeft = pItem->GetLeft();
                    m_nRight = pItem->GetRight();

                    UpdateSpacingLB();
                }
            }
        }
        break;
        case SID_ATTR_ULSPACE:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                const SvxULSpaceItem* pItem = dynamic_cast< const SvxULSpaceItem* >(pState);
                if(pItem)
                {
                    m_nTop = pItem->GetUpper();
                    m_nBottom = pItem->GetLower();

                    UpdateSpacingLB();
                }
            }
        }
        break;
    }
}

} // end of namespace ::sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
