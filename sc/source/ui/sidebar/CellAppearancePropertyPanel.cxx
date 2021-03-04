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

#include "CellAppearancePropertyPanel.hxx"
#include <sc.hrc>
#include <bitmaps.hlst>
#include <sfx2/bindings.hxx>
#include <sfx2/weldutils.hxx>
#include <svtools/toolbarmenu.hxx>
#include <editeng/borderline.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/lineitem.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/virdev.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include "CellLineStyleControl.hxx"
#include "CellBorderStyleControl.hxx"

using namespace css;
using namespace css::uno;

constexpr OStringLiteral SETBORDERSTYLE = "SetBorderStyle";
constexpr OStringLiteral LINESTYLE = "LineStyle";

// namespace open

namespace sc::sidebar {

CellAppearancePropertyPanel::CellAppearancePropertyPanel(
    weld::Widget* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
:   PanelLayout(pParent, "CellAppearancePropertyPanel", "modules/scalc/ui/sidebarcellappearance.ui"),

    mxTBCellBorder(m_xBuilder->weld_toolbar("cellbordertype")),
    mxTBCellBackground(m_xBuilder->weld_toolbar("cellbackgroundcolor")),
    mxBackColorDispatch(new ToolbarUnoDispatcher(*mxTBCellBackground, *m_xBuilder, rxFrame)),
    mxTBLineStyle(m_xBuilder->weld_toolbar("borderlinestyle")),
    mxTBLineColor(m_xBuilder->weld_toolbar("borderlinecolor")),
    mxLineColorDispatch(new ToolbarUnoDispatcher(*mxTBLineColor, *m_xBuilder, rxFrame)),

    mbCellBorderPopoverCreated(false),
    mbLinePopoverCreated(false),

    maLineStyleControl(SID_FRAME_LINESTYLE, *pBindings, *this),
    maBorderOuterControl(SID_ATTR_BORDER_OUTER, *pBindings, *this),
    maBorderInnerControl(SID_ATTR_BORDER_INNER, *pBindings, *this),
    maGridShowControl(FID_TAB_TOGGLE_GRID, *pBindings, *this),
    maBorderTLBRControl(SID_ATTR_BORDER_DIAG_TLBR, *pBindings, *this),
    maBorderBLTRControl(SID_ATTR_BORDER_DIAG_BLTR, *pBindings, *this),

    maIMGCellBorder(StockImage::Yes, RID_BMP_CELL_BORDER),
    msIMGCellBorder(RID_BMP_CELL_BORDER),
    msIMGLineStyle1(RID_BMP_LINE_STYLE1),
    msIMGLineStyle2(RID_BMP_LINE_STYLE2),
    msIMGLineStyle3(RID_BMP_LINE_STYLE3),
    msIMGLineStyle4(RID_BMP_LINE_STYLE4),
    msIMGLineStyle5(RID_BMP_LINE_STYLE5),
    msIMGLineStyle6(RID_BMP_LINE_STYLE6),
    msIMGLineStyle7(RID_BMP_LINE_STYLE7),
    msIMGLineStyle8(RID_BMP_LINE_STYLE8),
    msIMGLineStyle9(RID_BMP_LINE_STYLE9),

    mnInWidth(0),
    mnOutWidth(0),
    mnDistance(0),
    mnDiagTLBRInWidth(0),
    mnDiagTLBROutWidth(0),
    mnDiagTLBRDistance(0),
    mnDiagBLTRInWidth(0),
    mnDiagBLTROutWidth(0),
    mnDiagBLTRDistance(0),
    mbBorderStyleAvailable(true),
    mbLeft(false),
    mbRight(false),
    mbTop(false),
    mbBottom(false),
    mbVer(false),
    mbHor(false),
    mbOuterBorder(false),
    mbInnerBorder(false),
    mbDiagTLBR(false),
    mbDiagBLTR(false),
    maContext(),
    mpBindings(pBindings)
{
    Initialize();
}

CellAppearancePropertyPanel::~CellAppearancePropertyPanel()
{
    mxCellBorderPopoverContainer.reset();
    mxTBCellBorder.reset();
    mxBackColorDispatch.reset();
    mxTBCellBackground.reset();
    mxLinePopoverContainer.reset();
    mxTBLineStyle.reset();
    mxLineColorDispatch.reset();
    mxTBLineColor.reset();

    maLineStyleControl.dispose();
    maBorderOuterControl.dispose();
    maBorderInnerControl.dispose();
    maGridShowControl.dispose();
    maBorderTLBRControl.dispose();
    maBorderBLTRControl.dispose();
}

void CellAppearancePropertyPanel::Initialize()
{
    mxTBCellBorder->set_item_icon_name(SETBORDERSTYLE, msIMGCellBorder);
    mxCellBorderPopoverContainer.reset(new ToolbarPopupContainer(mxTBCellBorder.get()));
    mxTBCellBorder->set_item_popover(SETBORDERSTYLE, mxCellBorderPopoverContainer->getTopLevel());
    mxTBCellBorder->connect_clicked(LINK(this, CellAppearancePropertyPanel, TbxCellBorderSelectHdl));
    mxTBCellBorder->connect_menu_toggled(LINK(this, CellAppearancePropertyPanel, TbxCellBorderMenuHdl));

    mxTBLineStyle->set_item_icon_name(LINESTYLE, msIMGLineStyle1);
    mxLinePopoverContainer.reset(new ToolbarPopupContainer(mxTBLineStyle.get()));
    mxTBLineStyle->set_item_popover(LINESTYLE, mxLinePopoverContainer->getTopLevel());
    mxTBLineStyle->connect_clicked(LINK(this, CellAppearancePropertyPanel, TbxLineStyleSelectHdl));
    mxTBLineStyle->connect_menu_toggled(LINK(this, CellAppearancePropertyPanel, TbxLineStyleMenuHdl));
    mxTBLineStyle->set_sensitive(false);

    mxTBLineColor->set_sensitive(false);
}

IMPL_LINK_NOARG(CellAppearancePropertyPanel, TbxCellBorderSelectHdl, const OString&, void)
{
    mxTBCellBorder->set_menu_item_active(SETBORDERSTYLE, !mxTBCellBorder->get_menu_item_active(SETBORDERSTYLE));
}

IMPL_LINK_NOARG(CellAppearancePropertyPanel, TbxCellBorderMenuHdl, const OString&, void)
{
    if (!mxTBCellBorder->get_menu_item_active(SETBORDERSTYLE))
        return;
    if (!mbCellBorderPopoverCreated)
    {
        mxCellBorderPopoverContainer->setPopover(std::make_unique<CellBorderStylePopup>(mxTBCellBorder.get(), SETBORDERSTYLE, GetBindings()->GetDispatcher()));
        mbCellBorderPopoverCreated = true;
    }
    mxCellBorderPopoverContainer->getPopover()->GrabFocus();
}

IMPL_LINK_NOARG(CellAppearancePropertyPanel, TbxLineStyleSelectHdl, const OString&, void)
{
    mxTBLineStyle->set_menu_item_active(LINESTYLE, !mxTBLineStyle->get_menu_item_active(LINESTYLE));
}

IMPL_LINK_NOARG(CellAppearancePropertyPanel, TbxLineStyleMenuHdl, const OString&, void)
{
    if (!mxTBLineStyle->get_menu_item_active(LINESTYLE))
        return;
    if (!mbLinePopoverCreated)
    {
        mxLinePopoverContainer->setPopover(std::make_unique<CellLineStylePopup>(mxTBLineStyle.get(), LINESTYLE, GetBindings()->GetDispatcher()));
        mbLinePopoverCreated = true;
    }
    auto pPopup = static_cast<CellLineStylePopup*>(mxLinePopoverContainer->getPopover());
    pPopup->SetLineStyleSelect(mnOutWidth, mnInWidth, mnDistance);
    pPopup->GrabFocus();
}

std::unique_ptr<PanelLayout> CellAppearancePropertyPanel::Create (
    weld::Widget* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to CellAppearancePropertyPanel::Create", nullptr, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to CellAppearancePropertyPanel::Create", nullptr, 1);
    if (pBindings == nullptr)
        throw lang::IllegalArgumentException("no SfxBindings given to CellAppearancePropertyPanel::Create", nullptr, 2);

    return std::make_unique<CellAppearancePropertyPanel>(pParent, rxFrame, pBindings);
}

void CellAppearancePropertyPanel::HandleContextChange(const vcl::EnumContext& rContext)
{
    if (maContext == rContext)
    {
        // Nothing to do.
        return;
    }

    maContext = rContext;
}

void CellAppearancePropertyPanel::NotifyItemUpdate(
    sal_uInt16 nSID,
    SfxItemState eState,
    const SfxPoolItem* pState)
{
    switch(nSID)
    {
    case SID_FRAME_LINESTYLE:
        mbBorderStyleAvailable = false;
        if( eState == SfxItemState::DONTCARE )
        {
            mbBorderStyleAvailable = true;
            mnInWidth = 0;
            mnOutWidth = 0;
            mnDistance = 0;
        }
        else if(eState >= SfxItemState::DEFAULT)
        {
            const SvxLineItem* pSvxLineItem = dynamic_cast< const SvxLineItem* >(pState);
            if(pSvxLineItem)
            {
                const editeng::SvxBorderLine* pLineItem = pSvxLineItem->GetLine();
                mnInWidth = pLineItem->GetInWidth();
                mnOutWidth = pLineItem->GetOutWidth();
                mnDistance = pLineItem->GetDistance();
                mbBorderStyleAvailable = !(mnInWidth == 0 && mnOutWidth == 0 && mnDistance == 0);
            }
        }
        SetStyleIcon();
        break;
    case SID_ATTR_BORDER_OUTER:
        if(eState >= SfxItemState::DEFAULT)
        {
            const SvxBoxItem* pBoxItem = dynamic_cast< const SvxBoxItem* >(pState);

            if(pBoxItem)
            {
                mbLeft=false;
                mbRight=false;
                mbTop=false;
                mbBottom=false;

                if(pBoxItem->GetLeft())
                    mbLeft = true;

                if(pBoxItem->GetRight())
                    mbRight = true;

                if(pBoxItem->GetTop())
                    mbTop = true;

                if(pBoxItem->GetBottom())
                    mbBottom = true;

                if(!AllSettings::GetLayoutRTL())
                    UpdateCellBorder(mbTop, mbBottom, mbLeft, mbRight, mbVer, mbHor);
                else
                    UpdateCellBorder(mbTop, mbBottom, mbRight, mbLeft, mbVer, mbHor);

                if(mbLeft || mbRight || mbTop || mbBottom)
                    mbOuterBorder = true;
                else
                    mbOuterBorder = false;

                UpdateControlState();
            }
        }
        break;
    case SID_ATTR_BORDER_INNER:
        if(eState >= SfxItemState::DEFAULT)
        {
            const SvxBoxInfoItem* pBoxInfoItem = dynamic_cast< const SvxBoxInfoItem* >(pState);
            if(pBoxInfoItem)
            {
                bool bLeft(false), bRight(false), bTop(false), bBottom(false);

                mbVer = false;
                mbHor = false;

                if(!pBoxInfoItem->IsValid( SvxBoxInfoItemValidFlags::VERT )  || pBoxInfoItem->GetVert())
                    mbVer = true;

                if(!pBoxInfoItem->IsValid( SvxBoxInfoItemValidFlags::HORI )  || pBoxInfoItem->GetHori())
                    mbHor = true;

                if(!pBoxInfoItem->IsValid( SvxBoxInfoItemValidFlags::LEFT ) || mbLeft)
                    bLeft = true;

                if(!pBoxInfoItem->IsValid( SvxBoxInfoItemValidFlags::RIGHT ) || mbRight)
                    bRight = true;

                if(!pBoxInfoItem->IsValid( SvxBoxInfoItemValidFlags::TOP ) || mbTop)
                    bTop = true;

                if(!pBoxInfoItem->IsValid( SvxBoxInfoItemValidFlags::BOTTOM ) || mbBottom)
                    bBottom = true;

                if(!AllSettings::GetLayoutRTL())
                    UpdateCellBorder(bTop, bBottom, bLeft, bRight, mbVer, mbHor);
                else
                    UpdateCellBorder(bTop, bBottom, bRight, bLeft, mbVer, mbHor);

                if(mbVer || mbHor || bLeft || bRight || bTop || bBottom)
                    mbInnerBorder = true;
                else
                    mbInnerBorder = false;

                UpdateControlState();
            }
        }
        break;
    case SID_ATTR_BORDER_DIAG_TLBR:
        mbDiagTLBR = false;
        if( eState == SfxItemState::DONTCARE )
        {
            mbDiagTLBR = true;
            mnDiagTLBRInWidth = mnDiagTLBROutWidth = mnDiagTLBRDistance = 0;
        }
        else if(eState >= SfxItemState::DEFAULT)
        {
            const SvxLineItem* pItem = dynamic_cast< const SvxLineItem* >(pState);
            if(pItem)
            {
                const editeng::SvxBorderLine* aLine = pItem->GetLine();
                if(aLine)
                {
                    mnDiagTLBRInWidth = aLine->GetInWidth();
                    mnDiagTLBROutWidth = aLine->GetOutWidth();
                    mnDiagTLBRDistance = aLine->GetDistance();

                    mbDiagTLBR = !(mnDiagTLBRInWidth == 0 && mnDiagTLBROutWidth == 0 && mnDiagTLBRDistance == 0);
                }
            }
        }
        UpdateControlState();
        break;
    case SID_ATTR_BORDER_DIAG_BLTR:
        mbDiagBLTR = false;
        if( eState == SfxItemState::DONTCARE )
        {
            mbDiagBLTR = true;
            mnDiagBLTRInWidth = mnDiagBLTROutWidth = mnDiagBLTRDistance = 0;
        }
        else if(eState >= SfxItemState::DEFAULT)
        {
            const SvxLineItem* pItem = dynamic_cast< const SvxLineItem* >(pState);
            if(pItem)
            {
                const editeng::SvxBorderLine* aLine = pItem->GetLine();

                if(aLine)
                {
                    mnDiagBLTRInWidth = aLine->GetInWidth();
                    mnDiagBLTROutWidth = aLine->GetOutWidth();
                    mnDiagBLTRDistance = aLine->GetDistance();

                    mbDiagBLTR = !(mnDiagBLTRInWidth == 0 && mnDiagBLTROutWidth == 0 && mnDiagBLTRDistance == 0);
                }
            }
        }
        UpdateControlState();
        break;
    }
}

void CellAppearancePropertyPanel::SetStyleIcon()
{
    //FIXME: update for new line border possibilities
    if(mnOutWidth == DEF_LINE_WIDTH_0 && mnInWidth == 0 && mnDistance == 0)    //1
        mxTBLineStyle->set_item_icon_name(LINESTYLE, msIMGLineStyle1);
    else if(mnOutWidth == DEF_LINE_WIDTH_2 && mnInWidth == 0 && mnDistance == 0) //2
        mxTBLineStyle->set_item_icon_name(LINESTYLE, msIMGLineStyle2);
    else if(mnOutWidth == DEF_LINE_WIDTH_3 && mnInWidth == 0 && mnDistance == 0) //3
        mxTBLineStyle->set_item_icon_name(LINESTYLE, msIMGLineStyle3);
    else if(mnOutWidth == DEF_LINE_WIDTH_4 && mnInWidth == 0 && mnDistance == 0) //4
        mxTBLineStyle->set_item_icon_name(LINESTYLE, msIMGLineStyle4);
    else if(mnOutWidth == DEF_LINE_WIDTH_0 && mnInWidth == DEF_LINE_WIDTH_0 && mnDistance == DEF_LINE_WIDTH_1) //5
        mxTBLineStyle->set_item_icon_name(LINESTYLE, msIMGLineStyle5);
    else if(mnOutWidth == DEF_LINE_WIDTH_0 && mnInWidth == DEF_LINE_WIDTH_0 && mnDistance == DEF_LINE_WIDTH_2) //6
        mxTBLineStyle->set_item_icon_name(LINESTYLE, msIMGLineStyle6);
    else if(mnOutWidth == DEF_LINE_WIDTH_1 && mnInWidth == DEF_LINE_WIDTH_2 && mnDistance == DEF_LINE_WIDTH_1) //7
        mxTBLineStyle->set_item_icon_name(LINESTYLE, msIMGLineStyle7);
    else if(mnOutWidth == DEF_LINE_WIDTH_2 && mnInWidth == DEF_LINE_WIDTH_0 && mnDistance == DEF_LINE_WIDTH_2) //8
        mxTBLineStyle->set_item_icon_name(LINESTYLE, msIMGLineStyle8);
    else if(mnOutWidth == DEF_LINE_WIDTH_2 && mnInWidth == DEF_LINE_WIDTH_2 && mnDistance == DEF_LINE_WIDTH_2) //9
        mxTBLineStyle->set_item_icon_name(LINESTYLE, msIMGLineStyle9);
    else
        mxTBLineStyle->set_item_icon_name(LINESTYLE, msIMGLineStyle1);
}

void CellAppearancePropertyPanel::UpdateControlState()
{
    if(mbOuterBorder || mbInnerBorder || mbDiagTLBR || mbDiagBLTR)
    {
        mxTBLineColor->set_sensitive(true);
        mxTBLineStyle->set_sensitive(true);

        //set line style state
        if( mbBorderStyleAvailable && !mbDiagTLBR && !mbDiagBLTR )
        {
        }
        else if( !mbBorderStyleAvailable && mbDiagTLBR && !mbDiagBLTR )
        {
            mnInWidth = mnDiagTLBRInWidth;
            mnOutWidth = mnDiagTLBROutWidth;
            mnDistance = mnDiagTLBRDistance;
        }
        else if ( !mbBorderStyleAvailable && !mbDiagTLBR && mbDiagBLTR )
        {
            mnInWidth = mnDiagBLTRInWidth;
            mnOutWidth = mnDiagBLTROutWidth;
            mnDistance = mnDiagBLTRDistance;
        }
        else if( !mbBorderStyleAvailable && mbDiagTLBR && mbDiagBLTR)
        {
            if( mnDiagTLBRInWidth == mnDiagBLTRInWidth && mnDiagTLBROutWidth == mnDiagBLTROutWidth && mnDiagTLBRDistance == mnDiagBLTRDistance)
            {
                mnInWidth = mnDiagTLBRInWidth;
                mnOutWidth = mnDiagTLBROutWidth;
                mnDistance = mnDiagTLBRDistance;
            }
            else
            {
                mnInWidth = 0;
                mnOutWidth = 0;
                mnDistance = 0;
            }
        }
        else if( mbBorderStyleAvailable && mbDiagTLBR && !mbDiagBLTR )
        {
            if( mnDiagTLBRInWidth != mnInWidth || mnDiagTLBROutWidth != mnOutWidth || mnDiagTLBRDistance != mnDistance)
            {
                mnInWidth = 0;
                mnOutWidth = 0;
                mnDistance = 0;
            }
        }
        else if( mbBorderStyleAvailable && !mbDiagTLBR && mbDiagBLTR )
        {
            if(  mnDiagBLTRInWidth != mnInWidth || mnDiagBLTROutWidth != mnOutWidth || mnDiagBLTRDistance != mnDistance )
            {
                mnInWidth = 0;
                mnOutWidth = 0;
                mnDistance = 0;
            }
        }
        else
        {
            mnInWidth = 0;
            mnOutWidth = 0;
            mnDistance = 0;
        }
        SetStyleIcon();
    }
    else
    {
        mxTBLineColor->set_sensitive(false);
        mxTBLineStyle->set_sensitive(false);
    }
}

void CellAppearancePropertyPanel::UpdateCellBorder(bool bTop, bool bBot, bool bLeft, bool bRight, bool bVer, bool bHor)
{
    const Size aBmpSize = maIMGCellBorder.GetBitmapEx().GetSizePixel();

    if (aBmpSize.Width() == 43 && aBmpSize.Height() == 43)
    {
        ScopedVclPtr<VirtualDevice> pVirDev(mxTBCellBorder->create_virtual_device());
        pVirDev->SetOutputSizePixel(aBmpSize);
        pVirDev->SetLineColor( ::Application::GetSettings().GetStyleSettings().GetFieldTextColor() ) ;
        pVirDev->SetFillColor(COL_BLACK);
        pVirDev->DrawImage(Point(0, 0), maIMGCellBorder);
        Point aTL(2, 1), aTR(42,1), aBL(2, 41), aBR(42, 41), aHL(2,21), aHR(42, 21), aVT(22,1), aVB(22, 41);
        if(bLeft)
            pVirDev->DrawLine( aTL,aBL );
        if(bRight)
            pVirDev->DrawLine( aTR,aBR );
        if(bTop)
            pVirDev->DrawLine( aTL,aTR );
        if(bBot)
            pVirDev->DrawLine( aBL,aBR );
        if(bVer)
            pVirDev->DrawLine( aVT,aVB );
        if(bHor)
            pVirDev->DrawLine( aHL,aHR );
        mxTBCellBorder->set_item_image(SETBORDERSTYLE, pVirDev);
    }
    else
        mxTBCellBorder->set_item_icon_name(SETBORDERSTYLE, msIMGCellBorder);
}
// namespace close

} // end of namespace ::sc::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
