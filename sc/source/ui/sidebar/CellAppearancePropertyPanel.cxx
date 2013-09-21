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
#include <CellAppearancePropertyPanel.hxx>
#include <CellAppearancePropertyPanel.hrc>
#include "sc.hrc"
#include "scresid.hxx"
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/fixed.hxx>
#include <svx/tbxcolorupdate.hxx>
#include <svl/eitem.hxx>
#include <editeng/borderline.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/lineitem.hxx>
#include <vcl/svapp.hxx>
#include <svx/sidebar/ColorControl.hxx>
#include <boost/bind.hpp>
#include <svx/sidebar/PopupContainer.hxx>
#include <CellLineStyleControl.hxx>
#include <CellLineStylePopup.hxx>
#include <CellBorderUpdater.hxx>
#include <CellBorderStyleControl.hxx>
#include <CellBorderStylePopup.hxx>

using namespace css;
using namespace cssu;

const char UNO_BACKGROUNDCOLOR[] = ".uno:BackgroundColor";
const char UNO_SETBORDERSTYLE[] = ".uno:SetBorderStyle";
const char UNO_LINESTYLE[] = ".uno:LineStyle";
const char UNO_FRAMELINECOLOR[] = ".uno:FrameLineColor";
const char UNO_TOGGLESHEETGRID[] = ".uno:ToggleSheetGrid";

#define A2S(pString) (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(pString)))

//////////////////////////////////////////////////////////////////////////////
// helpers

namespace
{
    Color GetTransparentColor(void)
    {
        return COL_TRANSPARENT;
    }
} // end of anonymous namespace

//////////////////////////////////////////////////////////////////////////////
// namespace open

namespace sc { namespace sidebar {

//////////////////////////////////////////////////////////////////////////////

svx::sidebar::PopupControl* CellAppearancePropertyPanel::CreateFillColorPopupControl(svx::sidebar::PopupContainer* pParent)
{
    const ScResId aResId(VS_NOFILLCOLOR);

    return new svx::sidebar::ColorControl(
        pParent,
        mpBindings,
        ScResId(RID_POPUPPANEL_CELLAPPEARANCE_FILLCOLOR),
        ScResId(VS_FILLCOLOR),
        ::boost::bind(GetTransparentColor),
        ::boost::bind(&CellAppearancePropertyPanel::SetFillColor, this, _1, _2),
        pParent,
        &aResId);
}

void CellAppearancePropertyPanel::SetFillColor(
    const String& /*rsColorName*/,
    const Color aColor)
{
    const SvxColorItem aColorItem(aColor, SID_BACKGROUND_COLOR);
    mpBindings->GetDispatcher()->Execute(SID_BACKGROUND_COLOR, SFX_CALLMODE_RECORD, &aColorItem, 0L);
    maBackColor = aColor;
}

//////////////////////////////////////////////////////////////////////////////

svx::sidebar::PopupControl* CellAppearancePropertyPanel::CreateLineColorPopupControl(svx::sidebar::PopupContainer* pParent)
{
    return new svx::sidebar::ColorControl(
        pParent,
        mpBindings,
        ScResId(RID_POPUPPANEL_CELLAPPEARANCE_LINECOLOR),
        ScResId(VS_LINECOLOR),
        ::boost::bind(GetTransparentColor),
        ::boost::bind(&CellAppearancePropertyPanel::SetLineColor, this, _1, _2),
        pParent,
        0);
}

void CellAppearancePropertyPanel::SetLineColor(
    const String& /*rsColorName*/,
    const Color aColor)
{
    const SvxColorItem aColorItem(aColor, SID_FRAME_LINECOLOR);
    mpBindings->GetDispatcher()->Execute(SID_FRAME_LINECOLOR, SFX_CALLMODE_RECORD, &aColorItem, 0L);
    maLineColor = aColor;
}

//////////////////////////////////////////////////////////////////////////////

svx::sidebar::PopupControl* CellAppearancePropertyPanel::CreateCellLineStylePopupControl(svx::sidebar::PopupContainer* pParent)
{
    return new CellLineStyleControl(pParent, *this);
}

void CellAppearancePropertyPanel::EndCellLineStylePopupMode(void)
{
    if(mpCellLineStylePopup.get())
    {
        mpCellLineStylePopup->Hide();
    }
}

//////////////////////////////////////////////////////////////////////////////

svx::sidebar::PopupControl* CellAppearancePropertyPanel::CreateCellBorderStylePopupControl(svx::sidebar::PopupContainer* pParent)
{
    return new CellBorderStyleControl(pParent, *this);
}

void CellAppearancePropertyPanel::EndCellBorderStylePopupMode(void)
{
    if(mpCellBorderStylePopup.get())
    {
        mpCellBorderStylePopup->Hide();
    }
}

//////////////////////////////////////////////////////////////////////////////

CellAppearancePropertyPanel::CellAppearancePropertyPanel(
    Window* pParent,
    const cssu::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
:   PanelLayout(pParent, "CellAppearancePropertyPanel", "modules/scalc/ui/sidebarcellappearance.ui", rxFrame),

    maBackColorControl(SID_BACKGROUND_COLOR, *pBindings, *this),
    maLineColorControl(SID_FRAME_LINECOLOR, *pBindings, *this),
    maLineStyleControl(SID_FRAME_LINESTYLE, *pBindings, *this),
    maBorderOuterControl(SID_ATTR_BORDER_OUTER, *pBindings, *this),
    maBorderInnerControl(SID_ATTR_BORDER_INNER, *pBindings, *this),
    maGridShowControl(SID_SCGRIDSHOW, *pBindings, *this),
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

    maBackColor(COL_TRANSPARENT),
    maLineColor(COL_BLACK),
    maTLBRColor(COL_BLACK),
    maBLTRColor(COL_BLACK),
    mnIn(0),
    mnOut(0),
    mnDis(0),
    mnTLBRIn(0),
    mnTLBROut(0),
    mnTLBRDis(0),
    mnBLTRIn(0),
    mnBLTROut(0),
    mnBLTRDis(0),
    mbBackColorAvailable(true),
    mbLineColorAvailable(true),
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

    maFillColorPopup(this, ::boost::bind(&CellAppearancePropertyPanel::CreateFillColorPopupControl, this, _1)),
    maLineColorPopup(this, ::boost::bind(&CellAppearancePropertyPanel::CreateLineColorPopupControl, this, _1)),
    mpCellLineStylePopup(),
    mpCellBorderStylePopup(),

    mxFrame(rxFrame),
    maContext(),
    mpBindings(pBindings)
{
    get(mpTBFillColor,  "cellbackgroundcolor");
    get(mpTBCellBorder, "cellbordertype");
    get(mpTBLineStyle,  "borderlinestyle");
    get(mpTBLineColor,  "borderlinecolor");
    get(mpCBXShowGrid,  "cellgridlines");

    mpFillColorUpdater.reset( new ::svx::ToolboxButtonColorUpdater(SID_ATTR_BRUSH,
        mpTBFillColor->GetItemId( UNO_BACKGROUNDCOLOR ),
        mpTBFillColor) );
    mpLineColorUpdater.reset( new ::svx::ToolboxButtonColorUpdater(SID_FRAME_LINECOLOR,
        mpTBLineColor->GetItemId( UNO_FRAMELINECOLOR ),
        mpTBLineColor) );
    mpCellBorderUpdater.reset( new CellBorderUpdater(
        mpTBCellBorder->GetItemId( UNO_SETBORDERSTYLE ), *mpTBCellBorder) );

    Initialize();
}

//////////////////////////////////////////////////////////////////////////////

CellAppearancePropertyPanel::~CellAppearancePropertyPanel()
{
}

//////////////////////////////////////////////////////////////////////////////

void CellAppearancePropertyPanel::Initialize()
{
    const sal_uInt16 nIdBkColor = mpTBFillColor->GetItemId( UNO_BACKGROUNDCOLOR );
    mpTBFillColor->SetItemBits( nIdBkColor, mpTBFillColor->GetItemBits( nIdBkColor ) | TIB_DROPDOWNONLY );
    Link aLink = LINK(this, CellAppearancePropertyPanel, TbxBKColorSelectHdl);
    mpTBFillColor->SetDropdownClickHdl ( aLink );
    mpTBFillColor->SetSelectHdl ( aLink );

    const sal_uInt16 nIdBorderType  = mpTBCellBorder->GetItemId( UNO_SETBORDERSTYLE );
    mpTBCellBorder->SetItemImage( nIdBorderType, maIMGCellBorder );
    mpTBCellBorder->SetItemBits( nIdBorderType, mpTBCellBorder->GetItemBits( nIdBorderType ) | TIB_DROPDOWNONLY );
    aLink = LINK(this, CellAppearancePropertyPanel, TbxCellBorderSelectHdl);
    mpTBCellBorder->SetDropdownClickHdl ( aLink );
    mpTBCellBorder->SetSelectHdl ( aLink );

    const sal_uInt16 nIdBorderLineStyle = mpTBLineStyle->GetItemId( UNO_LINESTYLE );
    mpTBLineStyle->SetItemImage( nIdBorderLineStyle, maIMGLineStyle1 );
    mpTBLineStyle->SetItemBits( nIdBorderLineStyle, mpTBLineStyle->GetItemBits( nIdBorderLineStyle ) | TIB_DROPDOWNONLY );
    aLink = LINK(this, CellAppearancePropertyPanel, TbxLineStyleSelectHdl);
    mpTBLineStyle->SetDropdownClickHdl ( aLink );
    mpTBLineStyle->SetSelectHdl ( aLink );
    mpTBLineStyle->Disable();

    const sal_uInt16 nIdBorderLinecolor = mpTBLineColor->GetItemId( UNO_FRAMELINECOLOR );
    mpTBLineColor->SetItemBits( nIdBorderLinecolor, mpTBLineColor->GetItemBits( nIdBorderLinecolor ) | TIB_DROPDOWNONLY );
    aLink = LINK(this, CellAppearancePropertyPanel, TbxLineColorSelectHdl);
    mpTBLineColor->SetDropdownClickHdl ( aLink );
    mpTBLineColor->SetSelectHdl ( aLink );
    mpTBLineColor->Disable();

    aLink = LINK(this, CellAppearancePropertyPanel, CBOXGridShowClkHdl);
    mpCBXShowGrid->SetClickHdl ( aLink );

    mpTBLineColor->SetAccessibleRelationLabeledBy(mpTBLineColor);
    mpTBLineStyle->SetAccessibleRelationLabeledBy(mpTBLineStyle);
}

//////////////////////////////////////////////////////////////////////////////

IMPL_LINK(CellAppearancePropertyPanel, TbxBKColorSelectHdl, ToolBox*, pToolBox)
{
    const OUString aCommand(pToolBox->GetItemCommand(pToolBox->GetCurItemId()));

    if(aCommand == UNO_BACKGROUNDCOLOR)
    {
        maFillColorPopup.Show(*pToolBox);
        maFillColorPopup.SetCurrentColor(maBackColor, mbBackColorAvailable);
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////////

IMPL_LINK(CellAppearancePropertyPanel, TbxLineColorSelectHdl, ToolBox*, pToolBox)
{
    const OUString aCommand(pToolBox->GetItemCommand(pToolBox->GetCurItemId()));

    if(aCommand == UNO_FRAMELINECOLOR)
    {
        maLineColorPopup.Show(*pToolBox);
        maLineColorPopup.SetCurrentColor(maLineColor, mbLineColorAvailable);
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////////

IMPL_LINK(CellAppearancePropertyPanel, TbxCellBorderSelectHdl, ToolBox*, pToolBox)
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
                    ::boost::bind(&CellAppearancePropertyPanel::CreateCellBorderStylePopupControl, this, _1)));
        }

        if(mpCellBorderStylePopup.get())
        {
            mpCellBorderStylePopup->Show(*pToolBox);
        }
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////////

IMPL_LINK(CellAppearancePropertyPanel, TbxLineStyleSelectHdl, ToolBox*, pToolBox)
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
                    ::boost::bind(&CellAppearancePropertyPanel::CreateCellLineStylePopupControl, this, _1)));
        }

        if(mpCellLineStylePopup.get())
        {
            mpCellLineStylePopup->SetLineStyleSelect(mnOut, mnIn, mnDis);
            mpCellLineStylePopup->Show(*pToolBox);
        }
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////////

IMPL_LINK(CellAppearancePropertyPanel, CBOXGridShowClkHdl, void*, EMPTYARG)
{
    bool bState = mpCBXShowGrid->IsChecked();
    SfxBoolItem aItem( SID_SCGRIDSHOW , bState);
    GetBindings()->GetDispatcher()->Execute(SID_SCGRIDSHOW, SFX_CALLMODE_RECORD, &aItem, false, 0L);
    return 0;
}

//////////////////////////////////////////////////////////////////////////////

CellAppearancePropertyPanel* CellAppearancePropertyPanel::Create (
    Window* pParent,
    const cssu::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
{
    if (pParent == NULL)
        throw lang::IllegalArgumentException(A2S("no parent Window given to CellAppearancePropertyPanel::Create"), NULL, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException(A2S("no XFrame given to CellAppearancePropertyPanel::Create"), NULL, 1);
    if (pBindings == NULL)
        throw lang::IllegalArgumentException(A2S("no SfxBindings given to CellAppearancePropertyPanel::Create"), NULL, 2);

    return new CellAppearancePropertyPanel(
        pParent,
        rxFrame,
        pBindings);
}

//////////////////////////////////////////////////////////////////////////////

void CellAppearancePropertyPanel::DataChanged(
    const DataChangedEvent& rEvent)
{
    (void)rEvent;
}

//////////////////////////////////////////////////////////////////////////////

void CellAppearancePropertyPanel::HandleContextChange(
    const ::sfx2::sidebar::EnumContext aContext)
{
    if(maContext == aContext)
    {
        // Nothing to do.
        return;
    }

    maContext = aContext;



    // todo
}

//////////////////////////////////////////////////////////////////////////////

void CellAppearancePropertyPanel::NotifyItemUpdate(
    sal_uInt16 nSID,
    SfxItemState eState,
    const SfxPoolItem* pState,
    const bool bIsEnabled)
{
    (void)bIsEnabled;

    switch(nSID)
    {
    case SID_BACKGROUND_COLOR:
        if(eState >= SFX_ITEM_DEFAULT)
        {
            const SvxColorItem* pSvxColorItem = dynamic_cast< const SvxColorItem* >(pState);

            if(pSvxColorItem)
            {
                maBackColor = ((const SvxColorItem*)pState)->GetValue();
                mbBackColorAvailable = true;
                mpFillColorUpdater->Update(maBackColor);
                break;
            }
        }

        mbBackColorAvailable = false;
        maBackColor.SetColor(COL_TRANSPARENT);
        mpFillColorUpdater->Update(COL_TRANSPARENT);
        break;
    case SID_FRAME_LINECOLOR:
        if( eState == SFX_ITEM_DONTCARE)
        {
            mbLineColorAvailable = true;
            maLineColor.SetColor( COL_TRANSPARENT );
            UpdateControlState();
            break;
        }

        if(eState >= SFX_ITEM_DEFAULT && pState && pState->ISA(SvxColorItem) )
        {
            const SvxColorItem* pSvxColorItem = dynamic_cast< const SvxColorItem* >(pState);

            if(pSvxColorItem)
            {
                maLineColor = ((const SvxColorItem*)pState)->GetValue();
                if(maLineColor == COL_AUTO)
                    mbLineColorAvailable = false;
                else
                {
                    mbLineColorAvailable = true;
                //  mpLineColorUpdater->Update(maLineColor);
                }

                UpdateControlState();
                break;
            }
        }

        mbLineColorAvailable = false;
        maLineColor.SetColor(COL_AUTO);
        //  mpLineColorUpdater->Update(maLineColor);
        UpdateControlState();
        break;
    case SID_FRAME_LINESTYLE:
        if( eState == SFX_ITEM_DONTCARE )
        {
            mbBorderStyleAvailable = true;
            mnIn = 0;
            mnOut = 0;
            mnDis = 0;
            SetStyleIcon();
            break;
        }

        if(eState >= SFX_ITEM_DEFAULT)
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
        if(eState >= SFX_ITEM_DEFAULT)
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

                if(!Application::GetSettings().GetLayoutRTL())
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
        if(eState >= SFX_ITEM_DEFAULT)
        {
            const SvxBoxInfoItem* pBoxInfoItem = dynamic_cast< const SvxBoxInfoItem* >(pState);

            if(pBoxInfoItem)
            {
                bool bLeft(false), bRight(false), bTop(false), bBottom(false);

                mbVer = false, mbHor = false;

                if(!pBoxInfoItem->IsValid( VALID_VERT )  || pBoxInfoItem->GetVert())
                    mbVer = true;

                if(!pBoxInfoItem->IsValid( VALID_HORI )  || pBoxInfoItem->GetHori())
                    mbHor = true;

                if(!pBoxInfoItem->IsValid( VALID_LEFT ) || mbLeft)
                    bLeft = true;

                if(!pBoxInfoItem->IsValid( VALID_RIGHT ) || mbRight)
                    bRight = true;

                if(!pBoxInfoItem->IsValid( VALID_TOP ) || mbTop)
                    bTop = true;

                if(!pBoxInfoItem->IsValid( VALID_BOTTOM ) || mbBottom)
                    bBottom = true;

                if(!Application::GetSettings().GetLayoutRTL())
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
        if( eState == SFX_ITEM_DONTCARE )
        {
            mbTLBR = true;
            maTLBRColor.SetColor(COL_TRANSPARENT);
            mnTLBRIn = mnTLBROut = mnTLBRDis = 0;
            UpdateControlState();
            break;
        }

        if(eState >= SFX_ITEM_DEFAULT)
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
                    maTLBRColor = aLine->GetColor();
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
        if( eState == SFX_ITEM_DONTCARE )
        {
            mbBLTR = true;
            maBLTRColor.SetColor( COL_TRANSPARENT );
            mnBLTRIn = mnBLTROut = mnBLTRDis = 0;
            UpdateControlState();
            break;
        }

        if(eState >= SFX_ITEM_DEFAULT)
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
                    maBLTRColor = aLine->GetColor();
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
    case SID_SCGRIDSHOW:
        if(eState >= SFX_ITEM_DEFAULT)
        {
            const SfxBoolItem* pItem = dynamic_cast< const SfxBoolItem* >(pState);

            if(pItem)
            {
                const bool bVal = pItem->GetValue();

                if(bVal)
                    mpCBXShowGrid->Check(true);
                else
                    mpCBXShowGrid->Check(false);
            }
        }
        break;
    }
}

//////////////////////////////////////////////////////////////////////////////

SfxBindings* CellAppearancePropertyPanel::GetBindings()
{
    return mpBindings;
}

//////////////////////////////////////////////////////////////////////////////

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

        //set line color state
        if( mbLineColorAvailable && !mbTLBR && !mbBLTR )
            mpLineColorUpdater->Update(maLineColor);
        else if( !mbLineColorAvailable && mbTLBR && !mbBLTR )
            mpLineColorUpdater->Update(maTLBRColor);
        else if ( !mbLineColorAvailable && !mbTLBR && mbBLTR )
            mpLineColorUpdater->Update(maBLTRColor);
        else if( !mbLineColorAvailable && mbTLBR && mbBLTR)
        {
            if( maTLBRColor == maBLTRColor)
                mpLineColorUpdater->Update(maBLTRColor);
            else
                mpLineColorUpdater->Update(COL_TRANSPARENT);
        }
        else if( mbLineColorAvailable && mbTLBR && !mbBLTR )
        {
            if( maTLBRColor == maLineColor)
                mpLineColorUpdater->Update(maLineColor);
            else
                mpLineColorUpdater->Update(COL_TRANSPARENT);
        }
        else if( mbLineColorAvailable && !mbTLBR && mbBLTR )
        {
            if( maBLTRColor == maLineColor)
                mpLineColorUpdater->Update(maLineColor);
            else
                mpLineColorUpdater->Update(COL_TRANSPARENT);
        }
        else
            mpLineColorUpdater->Update(COL_TRANSPARENT);

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

//////////////////////////////////////////////////////////////////////////////
// namespace close

}} // end of namespace ::sc::sidebar

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
