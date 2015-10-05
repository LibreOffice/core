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

#include <sfx2/sidebar/ResourceDefinitions.hrc>
#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/sidebar/ControlFactory.hxx>
#include "CellAppearancePropertyPanel.hxx"
#include <CellAppearancePropertyPanel.hrc>
#include "sc.hrc"
#include "scresid.hxx"
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/fixed.hxx>
#include <svl/eitem.hxx>
#include <editeng/borderline.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/lineitem.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <boost/bind.hpp>
#include <svx/sidebar/PopupContainer.hxx>
#include "CellLineStyleControl.hxx"
#include "CellLineStylePopup.hxx"
#include "CellBorderUpdater.hxx"
#include "CellBorderStyleControl.hxx"
#include "CellBorderStylePopup.hxx"

using namespace css;
using namespace css::uno;

const char UNO_SETBORDERSTYLE[] = ".uno:SetBorderStyle";
const char UNO_LINESTYLE[] = ".uno:LineStyle";

// namespace open

namespace sc { namespace sidebar {

svx::sidebar::PopupControl* CellAppearancePropertyPanel::CreateCellLineStylePopupControl(svx::sidebar::PopupContainer* pParent)
{
    return VclPtr<CellLineStyleControl>::Create(pParent, *this);
}

void CellAppearancePropertyPanel::EndCellLineStylePopupMode()
{
    if(mpCellLineStylePopup.get())
    {
        mpCellLineStylePopup->Hide();
    }
}

svx::sidebar::PopupControl* CellAppearancePropertyPanel::CreateCellBorderStylePopupControl(svx::sidebar::PopupContainer* pParent)
{
    return VclPtr<CellBorderStyleControl>::Create(pParent, *this);
}

void CellAppearancePropertyPanel::EndCellBorderStylePopupMode()
{
    if(mpCellBorderStylePopup.get())
    {
        mpCellBorderStylePopup->Hide();
    }
}

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

    maIMGCellBorder(ScResId(IMG_CELL_BORDER)),
    maIMGLineStyle1(ScResId(IMG_LINE_STYLE1)),
    maIMGLineStyle2(ScResId(IMG_LINE_STYLE2)),
    maIMGLineStyle3(ScResId(IMG_LINE_STYLE3)),
    maIMGLineStyle4(ScResId(IMG_LINE_STYLE4)),
    maIMGLineStyle5(ScResId(IMG_LINE_STYLE5)),
    maIMGLineStyle6(ScResId(IMG_LINE_STYLE6)),
    maIMGLineStyle7(ScResId(IMG_LINE_STYLE7)),
    maIMGLineStyle8(ScResId(IMG_LINE_STYLE8)),
    maIMGLineStyle9(ScResId(IMG_LINE_STYLE9)),

    mnIn(0),
    mnOut(0),
    mnDis(0),
    mnTLBRIn(0),
    mnTLBROut(0),
    mnTLBRDis(0),
    mnBLTRIn(0),
    mnBLTROut(0),
    mnBLTRDis(0),
    mbBorderStyleAvailable(true),
    mbLeft(false),
    mbRight(false),
    mbTop(false),
    mbBottom(false),
    mbVer(false),
    mbHor(false),
    mbOuterBorder(false),
    mbInnerBorder(false),
    mbTLBR(false),
    mbBLTR(false),

    mpCellLineStylePopup(),
    mpCellBorderStylePopup(),

    mxFrame(rxFrame),
    maContext(),
    mpBindings(pBindings)
{
    get(mpTBCellBorder, "cellbordertype");
    get(mpTBLineStyle,  "borderlinestyle");
    get(mpTBLineColor,  "borderlinecolor");

    mpCellBorderUpdater.reset( new CellBorderUpdater(
        mpTBCellBorder->GetItemId( UNO_SETBORDERSTYLE ), *mpTBCellBorder) );

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

    mpTBLineColor->SetAccessibleRelationLabeledBy(mpTBLineColor);
    mpTBLineStyle->SetAccessibleRelationLabeledBy(mpTBLineStyle);
}

IMPL_LINK_TYPED(CellAppearancePropertyPanel, TbxCellBorderSelectHdl, ToolBox*, pToolBox, void)
{
    const OUString aCommand(pToolBox->GetItemCommand(pToolBox->GetCurItemId()));

    if(aCommand == UNO_SETBORDERSTYLE)
    {
        // create popup on demand
        if(!mpCellBorderStylePopup.get())
        {
            mpCellBorderStylePopup.reset(
                new CellBorderStylePopup(
                    this,
                    [this] (svx::sidebar::PopupContainer* pParent) { return this->CreateCellBorderStylePopupControl(pParent); } ));
        }

        if(mpCellBorderStylePopup.get())
        {
            mpCellBorderStylePopup->Show(*pToolBox);
        }
    }
}

IMPL_LINK_TYPED(CellAppearancePropertyPanel, TbxLineStyleSelectHdl, ToolBox*, pToolBox, void)
{
    const OUString aCommand(pToolBox->GetItemCommand(pToolBox->GetCurItemId()));

    if(aCommand == UNO_LINESTYLE)
    {
        // create popup on demand
        if(!mpCellLineStylePopup.get())
        {
            mpCellLineStylePopup.reset(
                new CellLineStylePopup(
                    this,
                    [this] (svx::sidebar::PopupContainer* pParent) { return this->CreateCellBorderStylePopupControl(pParent); } ));
        }

        if(mpCellLineStylePopup.get())
        {
            mpCellLineStylePopup->SetLineStyleSelect(mnOut, mnIn, mnDis);
            mpCellLineStylePopup->Show(*pToolBox);
        }
    }
}

VclPtr<vcl::Window> CellAppearancePropertyPanel::Create (
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
{
    if (pParent == NULL)
        throw lang::IllegalArgumentException("no parent Window given to CellAppearancePropertyPanel::Create", NULL, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to CellAppearancePropertyPanel::Create", NULL, 1);
    if (pBindings == NULL)
        throw lang::IllegalArgumentException("no SfxBindings given to CellAppearancePropertyPanel::Create", NULL, 2);

    return VclPtr<CellAppearancePropertyPanel>::Create(
                        pParent, rxFrame, pBindings);
}

void CellAppearancePropertyPanel::DataChanged(
    const DataChangedEvent& rEvent)
{
    (void)rEvent;
}

void CellAppearancePropertyPanel::HandleContextChange(const ::sfx2::sidebar::EnumContext& rContext)
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
    const bool bIsEnabled)
{
    (void)bIsEnabled;

    switch(nSID)
    {
    case SID_FRAME_LINESTYLE:
        if( eState == SfxItemState::DONTCARE )
        {
            mbBorderStyleAvailable = true;
            mnIn = 0;
            mnOut = 0;
            mnDis = 0;
            SetStyleIcon();
            break;
        }

        if(eState >= SfxItemState::DEFAULT)
        {
            const SvxLineItem* pSvxLineItem = dynamic_cast< const SvxLineItem* >(pState);

            if(pSvxLineItem)
            {
                const editeng::SvxBorderLine* mbLineItem = pSvxLineItem->GetLine();
                mnIn = mbLineItem->GetInWidth();
                mnOut = mbLineItem->GetOutWidth();
                mnDis = mbLineItem->GetDistance();

                if(mnIn == 0 && mnOut == 0 && mnDis == 0)
                    mbBorderStyleAvailable = false;
                else
                    mbBorderStyleAvailable = true;

                SetStyleIcon();
                break;
            }
        }

        mbBorderStyleAvailable = false;
        SetStyleIcon();
        break;
    case SID_ATTR_BORDER_OUTER:
        if(eState >= SfxItemState::DEFAULT)
        {
            const SvxBoxItem* pBoxItem = dynamic_cast< const SvxBoxItem* >(pState);

            if(pBoxItem)
            {
                mbLeft=false, mbRight=false, mbTop=false, mbBottom=false;

                if(pBoxItem->GetLeft())
                    mbLeft = true;

                if(pBoxItem->GetRight())
                    mbRight = true;

                if(pBoxItem->GetTop())
                    mbTop = true;

                if(pBoxItem->GetBottom())
                    mbBottom = true;

                if(!AllSettings::GetLayoutRTL())
                    mpCellBorderUpdater->UpdateCellBorder(mbTop, mbBottom, mbLeft, mbRight, maIMGCellBorder, mbVer, mbHor);
                else
                    mpCellBorderUpdater->UpdateCellBorder(mbTop, mbBottom, mbRight, mbLeft, maIMGCellBorder, mbVer, mbHor);

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

                mbVer = false, mbHor = false;

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
                    mpCellBorderUpdater->UpdateCellBorder(bTop, bBottom, bLeft, bRight, maIMGCellBorder, mbVer, mbHor);
                else
                    mpCellBorderUpdater->UpdateCellBorder(bTop, bBottom, bRight, bLeft, maIMGCellBorder, mbVer, mbHor);

                if(mbVer || mbHor || bLeft || bRight || bTop || bBottom)
                    mbInnerBorder = true;
                else
                    mbInnerBorder = false;

                UpdateControlState();
            }
        }
        break;
    case SID_ATTR_BORDER_DIAG_TLBR:
        if( eState == SfxItemState::DONTCARE )
        {
            mbTLBR = true;
            mnTLBRIn = mnTLBROut = mnTLBRDis = 0;
            UpdateControlState();
            break;
        }

        if(eState >= SfxItemState::DEFAULT)
        {
            const SvxLineItem* pItem = dynamic_cast< const SvxLineItem* >(pState);

            if(pItem)
            {
                const editeng::SvxBorderLine* aLine = pItem->GetLine();

                if(!aLine)
                {
                    mbTLBR = false;
                }
                else
                {
                    mbTLBR = true;
                    mnTLBRIn = aLine->GetInWidth();
                    mnTLBROut = aLine->GetOutWidth();
                    mnTLBRDis = aLine->GetDistance();

                    if(mnTLBRIn == 0 && mnTLBROut == 0 && mnTLBRDis == 0)
                        mbTLBR = false;
                }

                UpdateControlState();
                break;
            }
        }

        mbTLBR = false;
        UpdateControlState();
        break;
    case SID_ATTR_BORDER_DIAG_BLTR:
        if( eState == SfxItemState::DONTCARE )
        {
            mbBLTR = true;
            mnBLTRIn = mnBLTROut = mnBLTRDis = 0;
            UpdateControlState();
            break;
        }

        if(eState >= SfxItemState::DEFAULT)
        {
            const SvxLineItem* pItem = dynamic_cast< const SvxLineItem* >(pState);

            if(pItem)
            {
                const editeng::SvxBorderLine* aLine = pItem->GetLine();

                if(!aLine)
                {
                    mbBLTR = false;
                }
                else
                {
                    mbBLTR = true;
                    mnBLTRIn = aLine->GetInWidth();
                    mnBLTROut = aLine->GetOutWidth();
                    mnBLTRDis = aLine->GetDistance();

                    if(mnBLTRIn == 0 && mnBLTROut == 0 && mnBLTRDis == 0)
                        mbBLTR = false;
                }

                UpdateControlState();
            }
            break;
        }

        mbBLTR = false;
        UpdateControlState();
        break;
    }
}

void CellAppearancePropertyPanel::SetStyleIcon()
{
    const sal_uInt16 nIdBorderLineStyle = mpTBLineStyle->GetItemId( UNO_LINESTYLE );

    //FIXME: update for new line border possibilities
    if(mnOut == DEF_LINE_WIDTH_0 && mnIn == 0 && mnDis == 0)    //1
        mpTBLineStyle->SetItemImage(nIdBorderLineStyle, maIMGLineStyle1);
    else if(mnOut == DEF_LINE_WIDTH_2 && mnIn == 0 && mnDis == 0) //2
        mpTBLineStyle->SetItemImage(nIdBorderLineStyle, maIMGLineStyle2);
    else if(mnOut == DEF_LINE_WIDTH_3 && mnIn == 0 && mnDis == 0) //3
        mpTBLineStyle->SetItemImage(nIdBorderLineStyle, maIMGLineStyle3);
    else if(mnOut == DEF_LINE_WIDTH_4 && mnIn == 0 && mnDis == 0) //4
        mpTBLineStyle->SetItemImage(nIdBorderLineStyle, maIMGLineStyle4);
    else if(mnOut == DEF_LINE_WIDTH_0 && mnIn == DEF_LINE_WIDTH_0 && mnDis == DEF_LINE_WIDTH_1) //5
        mpTBLineStyle->SetItemImage(nIdBorderLineStyle, maIMGLineStyle5);
    else if(mnOut == DEF_LINE_WIDTH_0 && mnIn == DEF_LINE_WIDTH_0 && mnDis == DEF_LINE_WIDTH_2) //6
        mpTBLineStyle->SetItemImage(nIdBorderLineStyle, maIMGLineStyle6);
    else if(mnOut == DEF_LINE_WIDTH_1 && mnIn == DEF_LINE_WIDTH_2 && mnDis == DEF_LINE_WIDTH_1) //7
        mpTBLineStyle->SetItemImage(nIdBorderLineStyle, maIMGLineStyle7);
    else if(mnOut == DEF_LINE_WIDTH_2 && mnIn == DEF_LINE_WIDTH_0 && mnDis == DEF_LINE_WIDTH_2) //8
        mpTBLineStyle->SetItemImage(nIdBorderLineStyle, maIMGLineStyle8);
    else if(mnOut == DEF_LINE_WIDTH_2 && mnIn == DEF_LINE_WIDTH_2 && mnDis == DEF_LINE_WIDTH_2) //9
        mpTBLineStyle->SetItemImage(nIdBorderLineStyle, maIMGLineStyle9);
    else
        mpTBLineStyle->SetItemImage(nIdBorderLineStyle, maIMGLineStyle1);
}

void CellAppearancePropertyPanel::UpdateControlState()
{
    if(mbOuterBorder || mbInnerBorder || mbTLBR || mbBLTR)
    {
        mpTBLineColor->Enable();
        mpTBLineStyle->Enable();

        //set line style state
        if( mbBorderStyleAvailable && !mbTLBR && !mbBLTR )
        {
        }
        else if( !mbBorderStyleAvailable && mbTLBR && !mbBLTR )
        {
            mnIn = mnTLBRIn;
            mnOut = mnTLBROut;
            mnDis = mnTLBRDis;
        }
        else if ( !mbBorderStyleAvailable && !mbTLBR && mbBLTR )
        {
            mnIn = mnBLTRIn;
            mnOut = mnBLTROut;
            mnDis = mnBLTRDis;
        }
        else if( !mbBorderStyleAvailable && mbTLBR && mbBLTR)
        {
            if( mnTLBRIn == mnBLTRIn && mnTLBROut == mnBLTROut && mnTLBRDis == mnBLTRDis)
            {
                mnIn = mnTLBRIn;
                mnOut = mnTLBROut;
                mnDis = mnTLBRDis;
            }
            else
            {
                mnIn = 0;
                mnOut = 0;
                mnDis = 0;
            }
        }
        else if( mbBorderStyleAvailable && mbTLBR && !mbBLTR )
        {
            if( mnTLBRIn != mnIn || mnTLBROut != mnOut || mnTLBRDis != mnDis)
            {
                mnIn = 0;
                mnOut = 0;
                mnDis = 0;
            }
        }
        else if( mbBorderStyleAvailable && !mbTLBR && mbBLTR )
        {
            if(  mnBLTRIn != mnIn || mnBLTROut != mnOut || mnBLTRDis != mnDis )
            {
                mnIn = 0;
                mnOut = 0;
                mnDis = 0;
            }
        }
        else
        {
            mnIn = 0;
            mnOut = 0;
            mnDis = 0;
        }
        SetStyleIcon();
    }
    else
    {
        mpTBLineColor->Disable();
        mpTBLineStyle->Disable();
    }
}

// namespace close

}} // end of namespace ::sc::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
