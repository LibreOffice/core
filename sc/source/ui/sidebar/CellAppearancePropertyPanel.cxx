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

#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/sidebar/ControlFactory.hxx>
#include "CellAppearancePropertyPanel.hxx"
#include <sc.hrc>
#include <bitmaps.hlst>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/fixed.hxx>
#include <svl/eitem.hxx>
#include <editeng/borderline.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/lineitem.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/virdev.hxx>
#include "CellLineStyleControl.hxx"
#include "CellBorderStyleControl.hxx"

using namespace css;
using namespace css::uno;

const char UNO_SETBORDERSTYLE[] = ".uno:SetBorderStyle";
const char UNO_LINESTYLE[] = ".uno:LineStyle";

// namespace open

namespace sc { namespace sidebar {

CellAppearancePropertyPanel::CellAppearancePropertyPanel(
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
:   PanelLayout(pParent, "CellAppearancePropertyPanel", "modules/scalc/ui/sidebarcellappearance.ui", rxFrame),

    maLineStyleControl(SID_FRAME_LINESTYLE, *pBindings, *this),
    maBorderOuterControl(SID_ATTR_BORDER_OUTER, *pBindings, *this),
    maBorderInnerControl(SID_ATTR_BORDER_INNER, *pBindings, *this),
    maGridShowControl(FID_TAB_TOGGLE_GRID, *pBindings, *this),
    maBorderTLBRControl(SID_ATTR_BORDER_DIAG_TLBR, *pBindings, *this),
    maBorderBLTRControl(SID_ATTR_BORDER_DIAG_BLTR, *pBindings, *this),

    maIMGCellBorder(StockImage::Yes, RID_BMP_CELL_BORDER),
    maIMGLineStyle1(StockImage::Yes, RID_BMP_LINE_STYLE1),
    maIMGLineStyle2(StockImage::Yes, RID_BMP_LINE_STYLE2),
    maIMGLineStyle3(StockImage::Yes, RID_BMP_LINE_STYLE3),
    maIMGLineStyle4(StockImage::Yes, RID_BMP_LINE_STYLE4),
    maIMGLineStyle5(StockImage::Yes, RID_BMP_LINE_STYLE5),
    maIMGLineStyle6(StockImage::Yes, RID_BMP_LINE_STYLE6),
    maIMGLineStyle7(StockImage::Yes, RID_BMP_LINE_STYLE7),
    maIMGLineStyle8(StockImage::Yes, RID_BMP_LINE_STYLE8),
    maIMGLineStyle9(StockImage::Yes, RID_BMP_LINE_STYLE9),

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
    mxCellLineStylePopup(),
    mxCellBorderStylePopup(),
    maContext(),
    mpBindings(pBindings)
{
    get(mpTBCellBorder, "cellbordertype");
    get(mpTBLineStyle,  "borderlinestyle");
    get(mpTBLineColor,  "borderlinecolor");

    Initialize();
}

CellAppearancePropertyPanel::~CellAppearancePropertyPanel()
{
    disposeOnce();
}

void CellAppearancePropertyPanel::dispose()
{
    mpTBCellBorder.clear();
    mpTBLineStyle.clear();
    mpTBLineColor.clear();

    mxCellBorderStylePopup.disposeAndClear();
    mxCellLineStylePopup.disposeAndClear();
    maLineStyleControl.dispose();
    maBorderOuterControl.dispose();
    maBorderInnerControl.dispose();
    maGridShowControl.dispose();
    maBorderTLBRControl.dispose();
    maBorderBLTRControl.dispose();

    PanelLayout::dispose();
}

void CellAppearancePropertyPanel::Initialize()
{
    const sal_uInt16 nIdBorderType  = mpTBCellBorder->GetItemId( UNO_SETBORDERSTYLE );
    mpTBCellBorder->SetItemImage( nIdBorderType, maIMGCellBorder );
    mpTBCellBorder->SetItemBits( nIdBorderType, mpTBCellBorder->GetItemBits( nIdBorderType ) | ToolBoxItemBits::DROPDOWNONLY );
    Link<ToolBox *, void> aLink = LINK(this, CellAppearancePropertyPanel, TbxCellBorderSelectHdl);
    mpTBCellBorder->SetDropdownClickHdl ( aLink );
    mpTBCellBorder->SetSelectHdl ( aLink );

    const sal_uInt16 nIdBorderLineStyle = mpTBLineStyle->GetItemId( UNO_LINESTYLE );
    mpTBLineStyle->SetItemImage( nIdBorderLineStyle, maIMGLineStyle1 );
    mpTBLineStyle->SetItemBits( nIdBorderLineStyle, mpTBLineStyle->GetItemBits( nIdBorderLineStyle ) | ToolBoxItemBits::DROPDOWNONLY );
    aLink = LINK(this, CellAppearancePropertyPanel, TbxLineStyleSelectHdl);
    mpTBLineStyle->SetDropdownClickHdl ( aLink );
    mpTBLineStyle->SetSelectHdl ( aLink );
    mpTBLineStyle->Disable();

    mpTBLineColor->Disable();
}

IMPL_LINK(CellAppearancePropertyPanel, TbxCellBorderSelectHdl, ToolBox*, pToolBox, void)
{
    const OUString aCommand(pToolBox->GetItemCommand(pToolBox->GetCurItemId()));

    if (aCommand == UNO_SETBORDERSTYLE)
    {
        if (!mxCellBorderStylePopup)
            mxCellBorderStylePopup = VclPtr<CellBorderStylePopup>::Create(GetBindings()->GetDispatcher());
        mxCellBorderStylePopup->StartPopupMode(pToolBox, FloatWinPopupFlags::GrabFocus);
    }
}

IMPL_LINK(CellAppearancePropertyPanel, TbxLineStyleSelectHdl, ToolBox*, pToolBox, void)
{
    const OUString aCommand(pToolBox->GetItemCommand(pToolBox->GetCurItemId()));

    if (aCommand == UNO_LINESTYLE)
    {
        if (!mxCellLineStylePopup)
            mxCellLineStylePopup = VclPtr<CellLineStylePopup>::Create(GetBindings()->GetDispatcher());
        mxCellLineStylePopup->SetLineStyleSelect(mnOutWidth, mnInWidth, mnDistance);
        mxCellLineStylePopup->StartPopupMode(pToolBox, FloatWinPopupFlags::GrabFocus);
    }
}

VclPtr<vcl::Window> CellAppearancePropertyPanel::Create (
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to CellAppearancePropertyPanel::Create", nullptr, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to CellAppearancePropertyPanel::Create", nullptr, 1);
    if (pBindings == nullptr)
        throw lang::IllegalArgumentException("no SfxBindings given to CellAppearancePropertyPanel::Create", nullptr, 2);

    return VclPtr<CellAppearancePropertyPanel>::Create(
                        pParent, rxFrame, pBindings);
}

void CellAppearancePropertyPanel::DataChanged(
    const DataChangedEvent&)
{}

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
    const SfxPoolItem* pState,
    const bool)
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
    const sal_uInt16 nIdBorderLineStyle = mpTBLineStyle->GetItemId( UNO_LINESTYLE );

    //FIXME: update for new line border possibilities
    if(mnOutWidth == DEF_LINE_WIDTH_0 && mnInWidth == 0 && mnDistance == 0)    //1
        mpTBLineStyle->SetItemImage(nIdBorderLineStyle, maIMGLineStyle1);
    else if(mnOutWidth == DEF_LINE_WIDTH_2 && mnInWidth == 0 && mnDistance == 0) //2
        mpTBLineStyle->SetItemImage(nIdBorderLineStyle, maIMGLineStyle2);
    else if(mnOutWidth == DEF_LINE_WIDTH_3 && mnInWidth == 0 && mnDistance == 0) //3
        mpTBLineStyle->SetItemImage(nIdBorderLineStyle, maIMGLineStyle3);
    else if(mnOutWidth == DEF_LINE_WIDTH_4 && mnInWidth == 0 && mnDistance == 0) //4
        mpTBLineStyle->SetItemImage(nIdBorderLineStyle, maIMGLineStyle4);
    else if(mnOutWidth == DEF_LINE_WIDTH_0 && mnInWidth == DEF_LINE_WIDTH_0 && mnDistance == DEF_LINE_WIDTH_1) //5
        mpTBLineStyle->SetItemImage(nIdBorderLineStyle, maIMGLineStyle5);
    else if(mnOutWidth == DEF_LINE_WIDTH_0 && mnInWidth == DEF_LINE_WIDTH_0 && mnDistance == DEF_LINE_WIDTH_2) //6
        mpTBLineStyle->SetItemImage(nIdBorderLineStyle, maIMGLineStyle6);
    else if(mnOutWidth == DEF_LINE_WIDTH_1 && mnInWidth == DEF_LINE_WIDTH_2 && mnDistance == DEF_LINE_WIDTH_1) //7
        mpTBLineStyle->SetItemImage(nIdBorderLineStyle, maIMGLineStyle7);
    else if(mnOutWidth == DEF_LINE_WIDTH_2 && mnInWidth == DEF_LINE_WIDTH_0 && mnDistance == DEF_LINE_WIDTH_2) //8
        mpTBLineStyle->SetItemImage(nIdBorderLineStyle, maIMGLineStyle8);
    else if(mnOutWidth == DEF_LINE_WIDTH_2 && mnInWidth == DEF_LINE_WIDTH_2 && mnDistance == DEF_LINE_WIDTH_2) //9
        mpTBLineStyle->SetItemImage(nIdBorderLineStyle, maIMGLineStyle9);
    else
        mpTBLineStyle->SetItemImage(nIdBorderLineStyle, maIMGLineStyle1);
}

void CellAppearancePropertyPanel::UpdateControlState()
{
    if(mbOuterBorder || mbInnerBorder || mbDiagTLBR || mbDiagBLTR)
    {
        mpTBLineColor->Enable();
        mpTBLineStyle->Enable();

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
        mpTBLineColor->Disable();
        mpTBLineStyle->Disable();
    }
}

void CellAppearancePropertyPanel::UpdateCellBorder(bool bTop, bool bBot, bool bLeft, bool bRight, bool bVer, bool bHor)
{
    const Size aBmpSize = maIMGCellBorder.GetBitmapEx().GetSizePixel();

    ScopedVclPtr<VirtualDevice> pVirDev(VclPtr<VirtualDevice>::Create(*Application::GetDefaultDevice(),
            DeviceFormat::DEFAULT, DeviceFormat::DEFAULT));
    pVirDev->SetOutputSizePixel(aBmpSize);
    pVirDev->SetBackground(COL_TRANSPARENT);
    pVirDev->Erase();
    pVirDev->SetLineColor( ::Application::GetSettings().GetStyleSettings().GetFieldTextColor() ) ;
    pVirDev->SetFillColor(COL_BLACK);

    const int btnId = mpTBCellBorder->GetItemId( UNO_SETBORDERSTYLE );

    if(aBmpSize.Width() == 43 && aBmpSize.Height() == 43)
    {
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
        mpTBCellBorder->SetItemOverlayImage( btnId, Image( pVirDev->GetBitmapEx(Point(0,0), aBmpSize) ) );
    }

    mpTBCellBorder->SetItemImage( btnId, maIMGCellBorder );
}
// namespace close

}} // end of namespace ::sc::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
