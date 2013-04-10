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
:   Control(
        pParent,
        ScResId(RID_PROPERTYPANEL_SC_APPEAR)),

    mpFTFillColor(new FixedText(this, ScResId(FT_BK_COLOR))),
    mpTBFillColorBackground(sfx2::sidebar::ControlFactory::CreateToolBoxBackground(this)),
    mpTBFillColor(sfx2::sidebar::ControlFactory::CreateToolBox(mpTBFillColorBackground.get(), ScResId(TB_BK_COLOR))),
    mpFillColorUpdater(new ::svx::ToolboxButtonColorUpdater(SID_ATTR_BRUSH, TBI_BK_COLOR, mpTBFillColor.get(), TBX_UPDATER_MODE_CHAR_COLOR_NEW)),

    mpFTCellBorder(new FixedText(this, ScResId(FT_BORDER))),
    mpTBCellBorderBackground(sfx2::sidebar::ControlFactory::CreateToolBoxBackground(this)),
    mpTBCellBorder(sfx2::sidebar::ControlFactory::CreateToolBox(mpTBCellBorderBackground.get(), ScResId(TB_APP_BORDER))),
    mpCellBorderUpdater(new CellBorderUpdater(TBI_BORDER, *mpTBCellBorder)),

    mpTBLineStyleBackground(sfx2::sidebar::ControlFactory::CreateToolBoxBackground(this)),
    mpTBLineStyle(sfx2::sidebar::ControlFactory::CreateToolBox(mpTBLineStyleBackground.get(), ScResId(TB_BORDER_LINE_STYLE))),

    mpTBLineColorBackground(sfx2::sidebar::ControlFactory::CreateToolBoxBackground(this)),
    mpTBLineColor(sfx2::sidebar::ControlFactory::CreateToolBox(mpTBLineColorBackground.get(), ScResId(TB_BORDER_LINE_COLOR))),
    mpLineColorUpdater(new ::svx::ToolboxButtonColorUpdater(SID_FRAME_LINECOLOR, TBI_LINE_COLOR, mpTBLineColor.get(), TBX_UPDATER_MODE_CHAR_COLOR_NEW)),

    mpCBXShowGrid(new CheckBox(this, ScResId(CBX_SHOW_GRID))),

    maBackColorControl(SID_BACKGROUND_COLOR, *pBindings, *this),
    maLineColorControl(SID_FRAME_LINECOLOR, *pBindings, *this),
    maLineStyleControl(SID_FRAME_LINESTYLE, *pBindings, *this),
    maBorderOuterControl(SID_ATTR_BORDER_OUTER, *pBindings, *this),
    maBorderInnerControl(SID_ATTR_BORDER_INNER, *pBindings, *this),
    maGridShowControl(SID_SCGRIDSHOW, *pBindings, *this),
    maBorderTLBRControl(SID_ATTR_BORDER_DIAG_TLBR, *pBindings, *this),
    maBorderBLTRControl(SID_ATTR_BORDER_DIAG_BLTR, *pBindings, *this),

    maIMGBKColor(ScResId(IMG_BK_COLOR)),
    maIMGCellBorder(ScResId(IMG_CELL_BORDER)),
    maIMGLineColor(ScResId(IMG_LINE_COLOR)),
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
    Initialize();
    FreeResource();
}

//////////////////////////////////////////////////////////////////////////////

CellAppearancePropertyPanel::~CellAppearancePropertyPanel()
{
    // Destroy the toolboxes, then their background windows.
    mpTBFillColor.reset();
    mpTBCellBorder.reset();
    mpTBLineStyle.reset();
    mpTBLineColor.reset();

    mpTBFillColorBackground.reset();
    mpTBCellBorderBackground.reset();
    mpTBLineStyleBackground.reset();
    mpTBLineColorBackground.reset();
}

//////////////////////////////////////////////////////////////////////////////

void CellAppearancePropertyPanel::Initialize()
{
    mpTBFillColor->SetItemImage(TBI_BK_COLOR, maIMGBKColor);
    mpTBFillColor->SetItemBits( TBI_BK_COLOR, mpTBFillColor->GetItemBits( TBI_BK_COLOR ) | TIB_DROPDOWNONLY );
    mpTBFillColor->SetQuickHelpText(TBI_BK_COLOR,String(ScResId(STR_QH_BK_COLOR))); //Add
    Size aTbxSize1( mpTBFillColor->CalcWindowSizePixel() );
    mpTBFillColor->SetOutputSizePixel( aTbxSize1 );
    mpTBFillColor->SetBackground(Wallpaper());
    mpTBFillColor->SetPaintTransparent(true);
    Link aLink = LINK(this, CellAppearancePropertyPanel, TbxBKColorSelectHdl);
    mpTBFillColor->SetDropdownClickHdl ( aLink );
    mpTBFillColor->SetSelectHdl ( aLink );

    mpTBCellBorder->SetItemImage(TBI_BORDER, maIMGCellBorder);
    mpTBCellBorder->SetItemBits( TBI_BORDER, mpTBCellBorder->GetItemBits( TBI_BORDER ) | TIB_DROPDOWNONLY );
    mpTBCellBorder->SetQuickHelpText(TBI_BORDER,String(ScResId(STR_QH_BORDER)));    //Add
    Size aTbxSize2( mpTBCellBorder->CalcWindowSizePixel() );
    mpTBCellBorder->SetOutputSizePixel( aTbxSize2 );
    mpTBCellBorder->SetBackground(Wallpaper());
    mpTBCellBorder->SetPaintTransparent(true);
    aLink = LINK(this, CellAppearancePropertyPanel, TbxCellBorderSelectHdl);
    mpTBCellBorder->SetDropdownClickHdl ( aLink );
    mpTBCellBorder->SetSelectHdl ( aLink );

    mpTBLineStyle->SetItemImage(TBI_LINE_STYLE, maIMGLineStyle1);
    mpTBLineStyle->SetItemBits( TBI_LINE_STYLE, mpTBLineStyle->GetItemBits( TBI_LINE_STYLE ) | TIB_DROPDOWNONLY );
    mpTBLineStyle->SetQuickHelpText(TBI_LINE_STYLE,String(ScResId(STR_QH_BORDER_LINE_STYLE)));  //Add
    Size aTbxSize3( mpTBLineStyle->CalcWindowSizePixel() );
    mpTBLineStyle->SetOutputSizePixel( aTbxSize3 );
    mpTBLineStyle->SetBackground(Wallpaper());
    mpTBLineStyle->SetPaintTransparent(true);
    aLink = LINK(this, CellAppearancePropertyPanel, TbxLineStyleSelectHdl);
    mpTBLineStyle->SetDropdownClickHdl ( aLink );
    mpTBLineStyle->SetSelectHdl ( aLink );
    mpTBLineStyle->Disable();

    mpTBLineColor->SetItemImage(TBI_LINE_COLOR, maIMGLineColor);
    mpTBLineColor->SetItemBits( TBI_LINE_COLOR, mpTBLineColor->GetItemBits( TBI_LINE_COLOR ) | TIB_DROPDOWNONLY );
    mpTBLineColor->SetQuickHelpText(TBI_LINE_COLOR,String(ScResId(STR_QH_BORDER_LINE_COLOR)));  //Add
    Size aTbxSize4( mpTBLineColor->CalcWindowSizePixel() );
    mpTBLineColor->SetOutputSizePixel( aTbxSize4 );
    mpTBLineColor->SetBackground(Wallpaper());
    mpTBLineColor->SetPaintTransparent(true);
    aLink = LINK(this, CellAppearancePropertyPanel, TbxLineColorSelectHdl);
    mpTBLineColor->SetDropdownClickHdl ( aLink );
    mpTBLineColor->SetSelectHdl ( aLink );
    mpTBLineColor->Disable();

    aLink = LINK(this, CellAppearancePropertyPanel, CBOXGridShowClkHdl);
    mpCBXShowGrid->SetClickHdl ( aLink );

    mpTBFillColor->SetAccessibleRelationLabeledBy(mpFTFillColor.get());
    mpTBLineColor->SetAccessibleRelationLabeledBy(mpTBLineColor.get());
    mpTBCellBorder->SetAccessibleRelationLabeledBy(mpFTCellBorder.get());
    mpTBLineStyle->SetAccessibleRelationLabeledBy(mpTBLineStyle.get());
}

//////////////////////////////////////////////////////////////////////////////

IMPL_LINK(CellAppearancePropertyPanel, TbxBKColorSelectHdl, ToolBox*, pToolBox)
{
    sal_uInt16 nId = pToolBox->GetCurItemId();
    if(nId == TBI_BK_COLOR)
    {
        maFillColorPopup.Show(*pToolBox);
        maFillColorPopup.SetCurrentColor(maBackColor, mbBackColorAvailable);
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////////

IMPL_LINK(CellAppearancePropertyPanel, TbxLineColorSelectHdl, ToolBox*, pToolBox)
{
    sal_uInt16 nId = pToolBox->GetCurItemId();
    if(nId == TBI_LINE_COLOR)
    {
        maLineColorPopup.Show(*pToolBox);
        maLineColorPopup.SetCurrentColor(maLineColor, mbLineColorAvailable);
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////////

IMPL_LINK(CellAppearancePropertyPanel, TbxCellBorderSelectHdl, ToolBox*, pToolBox)
{
    sal_uInt16 nId = pToolBox->GetCurItemId();

    if(nId == TBI_BORDER)
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
    sal_uInt16 nId = pToolBox->GetCurItemId();
    if(nId == TBI_LINE_STYLE)
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
    const SfxPoolItem* pState)
{
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
    //FIXME: update for new line border possibilities
    if(mnOut == DEF_LINE_WIDTH_0 && mnIn == 0 && mnDis == 0)    //1
        mpTBLineStyle->SetItemImage(TBI_LINE_STYLE, maIMGLineStyle1);
    else if(mnOut == DEF_LINE_WIDTH_2 && mnIn == 0 && mnDis == 0) //2
        mpTBLineStyle->SetItemImage(TBI_LINE_STYLE, maIMGLineStyle2);
    else if(mnOut == DEF_LINE_WIDTH_3 && mnIn == 0 && mnDis == 0) //3
        mpTBLineStyle->SetItemImage(TBI_LINE_STYLE, maIMGLineStyle3);
    else if(mnOut == DEF_LINE_WIDTH_4 && mnIn == 0 && mnDis == 0) //4
        mpTBLineStyle->SetItemImage(TBI_LINE_STYLE, maIMGLineStyle4);
    else if(mnOut == DEF_LINE_WIDTH_0 && mnIn == DEF_LINE_WIDTH_0 && mnDis == DEF_LINE_WIDTH_1) //5
        mpTBLineStyle->SetItemImage(TBI_LINE_STYLE, maIMGLineStyle5);
    else if(mnOut == DEF_LINE_WIDTH_0 && mnIn == DEF_LINE_WIDTH_0 && mnDis == DEF_LINE_WIDTH_2) //6
        mpTBLineStyle->SetItemImage(TBI_LINE_STYLE, maIMGLineStyle6);
    else if(mnOut == DEF_LINE_WIDTH_1 && mnIn == DEF_LINE_WIDTH_2 && mnDis == DEF_LINE_WIDTH_1) //7
        mpTBLineStyle->SetItemImage(TBI_LINE_STYLE, maIMGLineStyle7);
    else if(mnOut == DEF_LINE_WIDTH_2 && mnIn == DEF_LINE_WIDTH_0 && mnDis == DEF_LINE_WIDTH_2) //8
        mpTBLineStyle->SetItemImage(TBI_LINE_STYLE, maIMGLineStyle8);
    else if(mnOut == DEF_LINE_WIDTH_2 && mnIn == DEF_LINE_WIDTH_2 && mnDis == DEF_LINE_WIDTH_2) //9
        mpTBLineStyle->SetItemImage(TBI_LINE_STYLE, maIMGLineStyle9);
    else
        mpTBLineStyle->SetItemImage(TBI_LINE_STYLE, maIMGLineStyle1);
}

//////////////////////////////////////////////////////////////////////////////

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
