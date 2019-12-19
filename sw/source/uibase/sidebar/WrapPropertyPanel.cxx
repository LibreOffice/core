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

#include <cmdid.h>
#include <swtypes.hxx>
#include <svx/spacinglistbox.hxx>
#include <svx/svxids.hrc>
#include <svx/svdtrans.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sidebar/ControlFactory.hxx>
#include <svl/eitem.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <hintids.hxx>
#include <uitool.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <comphelper/lok.hxx>

const char UNO_WRAPOFF[] = ".uno:WrapOff";
const char UNO_WRAPLEFT[] = ".uno:WrapLeft";
const char UNO_WRAPRIGHT[] = ".uno:WrapRight";
const char UNO_WRAPON[] = ".uno:WrapOn";
const char UNO_WRAPTHROUGH[] = ".uno:WrapThrough";
const char UNO_WRAPIDEAL[] = ".uno:WrapIdeal";

namespace sw { namespace sidebar {

VclPtr<vcl::Window> WrapPropertyPanel::Create (
    vcl::Window* pParent,
    const css::uno::Reference< css::frame::XFrame >& rxFrame,
    SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw css::lang::IllegalArgumentException("no parent Window given to WrapPropertyPanel::Create", nullptr, 0);
    if ( ! rxFrame.is())
        throw css::lang::IllegalArgumentException("no XFrame given to WrapPropertyPanel::Create", nullptr, 1);
    if (pBindings == nullptr)
        throw css::lang::IllegalArgumentException("no SfxBindings given to WrapPropertyPanel::Create", nullptr, 2);

    return VclPtr<WrapPropertyPanel>::Create(
                        pParent,
                        rxFrame,
                        pBindings);
}

WrapPropertyPanel::WrapPropertyPanel(
    vcl::Window* pParent,
    const css::uno::Reference< css::frame::XFrame >& rxFrame,
    SfxBindings* pBindings )
    : PanelLayout(pParent, "WrapPropertyPanel", "modules/swriter/ui/sidebarwrap.ui", rxFrame, true)
    , mxFrame( rxFrame )
    , mpBindings(pBindings)
    // spacing
    , nTop(0)
    , nBottom(0)
    , nLeft(0)
    , nRight(0)
    // resources
    , aCustomEntry()
    // controller items
    , maSwNoWrapControl(FN_FRAME_NOWRAP, *pBindings, *this)
    , maSwWrapLeftControl(FN_FRAME_WRAP, *pBindings, *this)
    , maSwWrapRightControl(FN_FRAME_WRAP_RIGHT, *pBindings, *this)
    , maSwWrapParallelControl(FN_FRAME_WRAP_LEFT, *pBindings, *this)
    , maSwWrapThroughControl(FN_FRAME_WRAPTHRU, *pBindings, *this)
    , maSwWrapIdealControl(FN_FRAME_WRAP_IDEAL, *pBindings, *this)
    , maSwEnableContourControl(FN_FRAME_WRAP_CONTOUR, *pBindings, *this)
    , maSwLRSpacingControl(SID_ATTR_LRSPACE, *pBindings, *this)
    , maSwULSpacingControl(SID_ATTR_ULSPACE, *pBindings, *this)
    , mxRBNoWrap(m_xBuilder->weld_radio_button("buttonnone"))
    , mxRBWrapLeft(m_xBuilder->weld_radio_button("buttonbefore"))
    , mxRBWrapRight(m_xBuilder->weld_radio_button("buttonafter"))
    , mxRBWrapParallel(m_xBuilder->weld_radio_button("buttonparallel"))
    , mxRBWrapThrough(m_xBuilder->weld_radio_button("buttonthrough"))
    , mxRBIdealWrap(m_xBuilder->weld_radio_button("buttonoptimal"))
    , mxEditContour(m_xBuilder->weld_button("editcontour"))
    , mxEnableContour(m_xBuilder->weld_check_button("enablecontour"))
    , mxSpacingLB(m_xBuilder->weld_combo_box("spacingLB"))
    , mxCustomEntry(m_xBuilder->weld_label("customlabel"))
{
    FieldUnit eMetric = ::GetDfltMetric(false);
    SpacingListBox::Fill(IsInch(eMetric) ? SpacingType::SPACING_INCH : SpacingType::SPACING_CM, *mxSpacingLB);

    Initialize();
}

WrapPropertyPanel::~WrapPropertyPanel()
{
    disposeOnce();
}

void WrapPropertyPanel::dispose()
{
    mxRBNoWrap.reset();
    mxRBWrapLeft.reset();
    mxRBWrapRight.reset();
    mxRBWrapParallel.reset();
    mxRBWrapThrough.reset();
    mxRBIdealWrap.reset();
    mxEnableContour.reset();
    mxEditContour.reset();
    mxSpacingLB.reset();
    mxCustomEntry.reset();

    maSwNoWrapControl.dispose();
    maSwWrapLeftControl.dispose();
    maSwWrapRightControl.dispose();
    maSwWrapParallelControl.dispose();
    maSwWrapThroughControl.dispose();
    maSwWrapIdealControl.dispose();
    maSwEnableContourControl.dispose();
    maSwLRSpacingControl.dispose();
    maSwULSpacingControl.dispose();

    PanelLayout::dispose();
}

void WrapPropertyPanel::Initialize()
{
    Link<weld::ToggleButton&,void> aLink = LINK(this, WrapPropertyPanel, WrapTypeHdl);
    mxRBNoWrap->connect_toggled(aLink);
    mxRBWrapLeft->connect_toggled(aLink);
    mxRBWrapRight->connect_toggled(aLink);
    mxRBWrapParallel->connect_toggled(aLink);
    mxRBWrapThrough->connect_toggled(aLink);
    mxRBIdealWrap->connect_toggled(aLink);

    Link<weld::Button&,void> EditContourLink = LINK(this, WrapPropertyPanel, EditContourHdl);
    mxEditContour->connect_clicked(EditContourLink);

    if (comphelper::LibreOfficeKit::isActive())
    {
        // Disable Edit Contour button for LOK purposes.
        mxEditContour->hide();
    }

    Link<weld::ToggleButton&, void> EnableContourLink = LINK(this,WrapPropertyPanel, EnableContourHdl);
    mxEnableContour->connect_toggled(EnableContourLink);
    mxSpacingLB->connect_changed(LINK(this, WrapPropertyPanel, SpacingLBHdl));

    mxRBNoWrap->set_image(vcl::CommandInfoProvider::GetXGraphicForCommand(UNO_WRAPOFF, mxFrame));
    if ( AllSettings::GetLayoutRTL() )
    {
        mxRBWrapLeft->set_image(vcl::CommandInfoProvider::GetXGraphicForCommand(UNO_WRAPRIGHT, mxFrame));
        mxRBWrapRight->set_image(vcl::CommandInfoProvider::GetXGraphicForCommand(UNO_WRAPLEFT, mxFrame));
    }
    else
    {
        mxRBWrapLeft->set_image(vcl::CommandInfoProvider::GetXGraphicForCommand(UNO_WRAPLEFT, mxFrame));
        mxRBWrapRight->set_image(vcl::CommandInfoProvider::GetXGraphicForCommand(UNO_WRAPRIGHT, mxFrame));
    }
    mxRBWrapParallel->set_image(vcl::CommandInfoProvider::GetXGraphicForCommand(UNO_WRAPON, mxFrame));
    mxRBWrapThrough->set_image(vcl::CommandInfoProvider::GetXGraphicForCommand(UNO_WRAPTHROUGH, mxFrame));
    mxRBIdealWrap->set_image(vcl::CommandInfoProvider::GetXGraphicForCommand(UNO_WRAPIDEAL, mxFrame));

    aCustomEntry = mxCustomEntry->get_label();

    mpBindings->Update( FN_FRAME_NOWRAP );
    mpBindings->Update( FN_FRAME_WRAP );
    mpBindings->Update( FN_FRAME_WRAP_RIGHT );
    mpBindings->Update( FN_FRAME_WRAP_LEFT );
    mpBindings->Update( FN_FRAME_WRAPTHRU );
    mpBindings->Update( FN_FRAME_WRAP_IDEAL );
    mpBindings->Update( FN_FRAME_WRAP_CONTOUR );
    mpBindings->Update( SID_ATTR_LRSPACE );
    mpBindings->Update( SID_ATTR_ULSPACE );

}

void WrapPropertyPanel::UpdateSpacingLB()
{
    if( (nLeft == nRight) && (nTop == nBottom) && (nLeft == nTop) )
    {
        sal_Int32 nCount = mxSpacingLB->get_count();
        for (sal_Int32 i = 0; i < nCount; i++)
        {
            if (mxSpacingLB->get_id(i).toUInt32() == nLeft)
            {
                mxSpacingLB->set_active(i);
                int nCustomEntry = mxSpacingLB->find_text(aCustomEntry);
                if (nCustomEntry != -1)
                    mxSpacingLB->remove(nCustomEntry);
                return;
            }
        }
    }

    if (mxSpacingLB->find_text(aCustomEntry) == -1)
        mxSpacingLB->append_text(aCustomEntry);
    mxSpacingLB->set_active_text(aCustomEntry);
}

IMPL_LINK_NOARG(WrapPropertyPanel, EditContourHdl, weld::Button&, void)
{
    SfxBoolItem aItem(SID_CONTOUR_DLG, true);
    mpBindings->GetDispatcher()->ExecuteList(SID_CONTOUR_DLG,
            SfxCallMode::RECORD, { &aItem });
}

IMPL_LINK_NOARG(WrapPropertyPanel, EnableContourHdl, weld::ToggleButton&, void)
{
    bool IsContour = mxEnableContour->get_active();
    SfxBoolItem aItem(FN_FRAME_WRAP_CONTOUR, IsContour);
    mpBindings->GetDispatcher()->ExecuteList(FN_FRAME_WRAP_CONTOUR,
            SfxCallMode::RECORD, { &aItem });
}

IMPL_LINK(WrapPropertyPanel, SpacingLBHdl, weld::ComboBox&, rBox, void)
{
    sal_uInt16 nVal = rBox.get_active_id().toUInt32();

    SvxLRSpaceItem aLRItem(nVal, nVal, 0, 0, RES_LR_SPACE);
    SvxULSpaceItem aULItem(nVal, nVal, RES_UL_SPACE);

    nTop = nBottom = nLeft = nRight = nVal;
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_LRSPACE,
            SfxCallMode::RECORD, { &aLRItem });
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_ULSPACE,
            SfxCallMode::RECORD, { &aULItem });
}

IMPL_LINK_NOARG(WrapPropertyPanel, WrapTypeHdl, weld::ToggleButton&, void)
{
    sal_uInt16 nSlot = 0;
    if ( mxRBWrapLeft->get_active() )
    {
        nSlot = FN_FRAME_WRAP_LEFT;
    }
    else if( mxRBWrapRight->get_active() )
    {
        nSlot = FN_FRAME_WRAP_RIGHT;
    }
    else if ( mxRBWrapParallel->get_active() )
    {
        nSlot = FN_FRAME_WRAP;
    }
    else if( mxRBWrapThrough->get_active() )
    {
        nSlot = FN_FRAME_WRAPTHRU;
    }
    else if( mxRBIdealWrap->get_active() )
    {
        nSlot = FN_FRAME_WRAP_IDEAL;
    }
    else
    {
        nSlot = FN_FRAME_NOWRAP;
    }
    SfxBoolItem bStateItem( nSlot, true );
    mpBindings->GetDispatcher()->ExecuteList(nSlot,
            SfxCallMode::RECORD, { &bStateItem });

}

void WrapPropertyPanel::UpdateEditContour()
{
    if (mxRBNoWrap->get_active() || mxRBWrapThrough->get_active())
    {
        mxEnableContour->set_active(false);
        mxEnableContour->set_sensitive(false);
    }
    else
    {
        mxEnableContour->set_sensitive(true);
    }

}

void WrapPropertyPanel::NotifyItemUpdate(
    const sal_uInt16 nSId,
    const SfxItemState eState,
    const SfxPoolItem* pState)
{
    if ( eState == SfxItemState::DEFAULT &&
        dynamic_cast< const SfxBoolItem *>( pState ) !=  nullptr )
    {
        //Set Radio Button enable
        mxRBNoWrap->set_sensitive(true);
        mxRBWrapLeft->set_sensitive(true);
        mxRBWrapRight->set_sensitive(true);
        mxRBWrapParallel->set_sensitive(true);
        mxRBWrapThrough->set_sensitive(true);
        mxRBIdealWrap->set_sensitive(true);
        mxEnableContour->set_sensitive(true);

        const SfxBoolItem* pBoolItem = static_cast< const SfxBoolItem* >( pState );
        switch( nSId )
        {
        case FN_FRAME_WRAP_RIGHT:
            mxRBWrapRight->set_active( pBoolItem->GetValue() );
            break;
        case FN_FRAME_WRAP_LEFT:
            mxRBWrapLeft->set_active( pBoolItem->GetValue() );
            break;
        case FN_FRAME_WRAPTHRU:
            mxRBWrapThrough->set_active( pBoolItem->GetValue() );
            break;
        case FN_FRAME_WRAP_IDEAL:
            mxRBIdealWrap->set_active( pBoolItem->GetValue() );
            break;
        case FN_FRAME_WRAP:
            mxRBWrapParallel->set_active( pBoolItem->GetValue() );
            break;
        case FN_FRAME_WRAP_CONTOUR:
            mxEnableContour->set_active( pBoolItem->GetValue() );
            break;
        case FN_FRAME_NOWRAP:
            mxRBNoWrap->set_active( pBoolItem->GetValue() );
            break;
        }
        UpdateEditContour();
    }
    switch(nSId)
    {
        case SID_ATTR_LRSPACE:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                const SvxLRSpaceItem* pItem = dynamic_cast< const SvxLRSpaceItem* >(pState);
                if(pItem)
                {
                    nLeft = pItem->GetLeft();
                    nRight = pItem->GetRight();

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
                    nTop = pItem->GetUpper();
                    nBottom = pItem->GetLower();

                    UpdateSpacingLB();
                }
            }
        }
        break;
    }
}

} } // end of namespace ::sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
