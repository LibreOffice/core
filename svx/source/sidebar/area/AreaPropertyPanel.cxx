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
#include "svx/sidebar/PopupContainer.hxx"
#include "AreaTransparencyGradientControl.hxx"

#include <sfx2/sidebar/ResourceDefinitions.hrc>
#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/sidebar/ControlFactory.hxx>
#include <AreaPropertyPanel.hxx>
#include <AreaPropertyPanel.hrc>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <sfx2/objsh.hxx>
#include <svx/xfltrit.hxx>
#include <svx/xflftrit.hxx>
#include <svx/xtable.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <helpid.hrc>
#include <svtools/valueset.hxx>
#include <unotools/pathoptions.hxx>
#include <svx/svxitems.hrc>
#include <vcl/toolbox.hxx>
#include <svtools/toolbarmenu.hxx>
#include <svx/sidebar/ColorControl.hxx>

#include <boost/bind.hpp>

using namespace css;
using namespace cssu;
using ::sfx2::sidebar::Theme;

const char UNO_SIDEBARCOLOR[]    = ".uno:sidebarcolor";
const char UNO_SIDEBARGRADIENT[] = ".uno:sidebargradient";

#define A2S(pString) (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(pString)))

namespace svx { namespace sidebar {

const sal_Int32 AreaPropertyPanel::DEFAULT_CENTERX = 50;
const sal_Int32 AreaPropertyPanel::DEFAULT_CENTERY = 50;
const sal_Int32 AreaPropertyPanel::DEFAULT_ANGLE = 0;
const sal_Int32 AreaPropertyPanel::DEFAULT_STARTVALUE = 0;
const sal_Int32 AreaPropertyPanel::DEFAULT_ENDVALUE = 16777215;
const sal_Int32 AreaPropertyPanel::DEFAULT_BORDER = 0;


AreaPropertyPanel::AreaPropertyPanel(
    Window* pParent,
    const cssu::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
    : PanelLayout(pParent, "AreaPropertyPanel", "svx/ui/sidebararea.ui", rxFrame),
      meLastXFS(-1),
      maLastColor(Color(COL_DEFAULT_SHAPE_FILLING)),
      mnLastPosGradient(0),
      mnLastPosHatch(0),
      mnLastPosBitmap(0),
      mnLastTransSolid(50),
      maGradientLinear(),
      maGradientAxial(),
      maGradientRadial(),
      maGradientElliptical(),
      maGradientSquare(),
      maGradientRect(),
      mpStyleItem(),
      mpColorItem(),
      mpFillGradientItem(),
      mpHatchItem(),
      mpBitmapItem(),
      maStyleControl(SID_ATTR_FILL_STYLE, *pBindings, *this),
      maColorControl(SID_ATTR_FILL_COLOR, *pBindings, *this),
      maGradientControl(SID_ATTR_FILL_GRADIENT, *pBindings, *this),
      maHatchControl(SID_ATTR_FILL_HATCH, *pBindings, *this),
      maBitmapControl(SID_ATTR_FILL_BITMAP, *pBindings, *this),
      maColorTableControl(SID_COLOR_TABLE, *pBindings, *this),
      maGradientListControl(SID_GRADIENT_LIST, *pBindings, *this),
      maHatchListControl(SID_HATCH_LIST, *pBindings, *this),
      maBitmapListControl(SID_BITMAP_LIST, *pBindings, *this),
      maFillTransparenceController(SID_ATTR_FILL_TRANSPARENCE, *pBindings, *this),
      maFillFloatTransparenceController(SID_ATTR_FILL_FLOATTRANSPARENCE, *pBindings, *this),
      maImgAxial(SVX_RES(IMG_AXIAL)),
      maImgElli(SVX_RES(IMG_ELLI)),
      maImgQuad(SVX_RES(IMG_QUAD)),
      maImgRadial(SVX_RES(IMG_RADIAL)),
      maImgSquare(SVX_RES(IMG_SQUARE)),
      maImgLinear(SVX_RES(IMG_LINEAR)),
      maImgColor(SVX_RES(IMG_COLOR)),
      maTrGrPopup(this, ::boost::bind(&AreaPropertyPanel::CreateTransparencyGradientControl, this, _1)),
      maColorPopup(this, ::boost::bind(&AreaPropertyPanel::CreateColorPopupControl, this, _1)),
      mpFloatTransparenceItem(),
      mpTransparanceItem(),
      mxFrame(rxFrame),
      mpBindings(pBindings),
      mbColorAvail(true)
{
    get(mpColorTextFT,    "filllabel");
    get(mpLbFillType,     "fillstyle");
    get(mpLbFillAttr,     "fillattr");
    get(mpTrspTextFT,     "transparencylabel");
    get(mpToolBoxColor,   "selectcolor");
    get(mpLBTransType,    "transtype");
    get(mpMTRTransparent, "settransparency");
    get(mpBTNGradient,    "selectgradient");

    const sal_uInt16 nIdColor = mpToolBoxColor->GetItemId(UNO_SIDEBARCOLOR);
    mpColorUpdater.reset(new ::svx::ToolboxButtonColorUpdater(SID_ATTR_FILL_COLOR, nIdColor, mpToolBoxColor)),

    Initialize();
}



AreaPropertyPanel::~AreaPropertyPanel()
{
}



void AreaPropertyPanel::Initialize()
{
    maGradientLinear.SetXOffset(DEFAULT_CENTERX);
    maGradientLinear.SetYOffset(DEFAULT_CENTERY);
    maGradientLinear.SetAngle(DEFAULT_ANGLE);
    maGradientLinear.SetStartColor(Color(DEFAULT_STARTVALUE));
    maGradientLinear.SetEndColor(Color(DEFAULT_ENDVALUE));
    maGradientLinear.SetBorder(DEFAULT_BORDER);
    maGradientLinear.SetGradientStyle(XGRAD_LINEAR);

    maGradientAxial = maGradientLinear;
    maGradientAxial.SetGradientStyle(XGRAD_AXIAL);

    maGradientRadial = maGradientLinear;
    maGradientRadial.SetGradientStyle(XGRAD_RADIAL);

    maGradientElliptical = maGradientLinear;
    maGradientElliptical.SetGradientStyle(XGRAD_ELLIPTICAL);

    maGradientSquare = maGradientLinear;
    maGradientSquare.SetGradientStyle(XGRAD_SQUARE);

    maGradientRect = maGradientLinear;
    maGradientRect.SetGradientStyle(XGRAD_RECT);

    mpLbFillType->SetAccessibleName(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Fill")));    //wj acc
    mpLbFillAttr->SetAccessibleName(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Fill")));    //wj acc

    Link aLink = LINK( this, AreaPropertyPanel, SelectFillTypeHdl );
    mpLbFillType->SetSelectHdl( aLink );

    aLink = LINK( this, AreaPropertyPanel, SelectFillAttrHdl );
    mpLbFillAttr->SetSelectHdl( aLink );

    //add  for new color picker
    mpLbFillAttr->Hide();
    const sal_uInt16 nIdColor = mpToolBoxColor->GetItemId(UNO_SIDEBARCOLOR);
    mpToolBoxColor->SetItemImage(nIdColor, maImgColor);
    mpToolBoxColor->SetItemBits( nIdColor, mpToolBoxColor->GetItemBits( nIdColor ) | TIB_DROPDOWNONLY );
    mpToolBoxColor->SetItemText(nIdColor, msHelpFillAttr);

    aLink = LINK(this, AreaPropertyPanel, ToolBoxColorDropHdl);
    mpToolBoxColor->SetDropdownClickHdl ( aLink );
    mpToolBoxColor->SetSelectHdl ( aLink );

    //add end
    mpLBTransType->SetSelectHdl(LINK(this, AreaPropertyPanel, ChangeTrgrTypeHdl_Impl));
    mpLBTransType->SetAccessibleName(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Transparency")));    //wj acc

    mpMTRTransparent->SetValue( 50 );
    mpMTRTransparent->SetModifyHdl(LINK(this, AreaPropertyPanel, ModifyTransparentHdl_Impl));
    mpMTRTransparent->SetAccessibleName(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Transparency")));    //wj acc

    const sal_uInt16 nIdGradient = mpBTNGradient->GetItemId(UNO_SIDEBARGRADIENT);
    mpBTNGradient->SetItemBits( nIdGradient, mpBTNGradient->GetItemBits( nIdGradient ) | TIB_DROPDOWNONLY );
    aLink = LINK( this, AreaPropertyPanel, ClickTrGrHdl_Impl );
    mpBTNGradient->SetDropdownClickHdl( aLink );
    mpBTNGradient->SetSelectHdl( aLink );
    mpBTNGradient->SetItemImage(nIdGradient,maImgLinear);
    mpBTNGradient->Hide();

    mpLbFillType->SetAccessibleRelationLabeledBy(mpColorTextFT);
    mpLbFillAttr->SetAccessibleRelationLabeledBy(mpLbFillAttr);
    mpToolBoxColor->SetAccessibleRelationLabeledBy(mpToolBoxColor);
    mpLBTransType->SetAccessibleRelationLabeledBy(mpTrspTextFT);
    mpMTRTransparent->SetAccessibleRelationLabeledBy(mpMTRTransparent);
    mpBTNGradient->SetAccessibleRelationLabeledBy(mpBTNGradient);

    SetupIcons();
}



IMPL_LINK( AreaPropertyPanel, SelectFillTypeHdl, ListBox *, pToolBox )
{
    const XFillStyle eXFS = (XFillStyle)mpLbFillType->GetSelectEntryPos();

    if((XFillStyle)meLastXFS != eXFS)
    {
        mpLbFillAttr->Clear();
        SfxObjectShell* pSh = SfxObjectShell::Current();
        const XFillStyleItem aXFillStyleItem(eXFS);

        // #i122676# Do no longer trigger two Execute calls, one for SID_ATTR_FILL_STYLE
        // and one for setting the fill attribute itself, but add two SfxPoolItems to the
        // call to get just one action at the SdrObject and to create only one Undo action, too.
        // Checked that this works in all apps.
        switch( eXFS )
        {
            case XFILL_NONE:
            {
                mpLbFillAttr->Show();
                mpToolBoxColor->Hide();
                mpLbFillType->Selected();
                mpLbFillAttr->Disable();

                // #i122676# need to call a single SID_ATTR_FILL_STYLE change
                GetBindings()->GetDispatcher()->Execute(
                    SID_ATTR_FILL_STYLE, SFX_CALLMODE_RECORD, &aXFillStyleItem, 0L);
                break;
            }
            case XFILL_SOLID:
            {
                mpLbFillAttr->Hide();
                mpToolBoxColor->Show();
                const String aTmpStr;
                const Color aColor = maLastColor;
                const XFillColorItem aXFillColorItem( aTmpStr, aColor );

                // #i122676# change FillStyle and Color in one call
                GetBindings()->GetDispatcher()->Execute(
                    SID_ATTR_FILL_COLOR, SFX_CALLMODE_RECORD, &aXFillColorItem, &aXFillStyleItem, 0L);
                break;
            }
            case XFILL_GRADIENT:
            {
                mpLbFillAttr->Show();
                mpToolBoxColor->Hide();

                if(pSh && pSh->GetItem(SID_GRADIENT_LIST))
                {
                    if(!mpLbFillAttr->GetEntryCount())
                    {
                        const SvxGradientListItem aItem(*(const SvxGradientListItem*)(pSh->GetItem(SID_GRADIENT_LIST)));
                        mpLbFillAttr->Enable();
                        mpLbFillAttr->Clear();
                        mpLbFillAttr->Fill(aItem.GetGradientList());
                    }

                    mpLbFillAttr->AdaptDropDownLineCountToMaximum();

                    if(LISTBOX_ENTRY_NOTFOUND != mnLastPosGradient)
                    {
                        const SvxGradientListItem aItem(*(const SvxGradientListItem*)(pSh->GetItem(SID_GRADIENT_LIST)));

                        if(mnLastPosGradient < aItem.GetGradientList()->Count())
                        {
                            const XGradient aGradient = aItem.GetGradientList()->GetGradient(mnLastPosGradient)->GetGradient();
                            const XFillGradientItem aXFillGradientItem(mpLbFillAttr->GetEntry(mnLastPosGradient), aGradient);

                            // #i122676# change FillStyle and Gradient in one call
                            GetBindings()->GetDispatcher()->Execute(
                                SID_ATTR_FILL_GRADIENT, SFX_CALLMODE_RECORD, &aXFillGradientItem, &aXFillStyleItem, 0L);
                            mpLbFillAttr->SelectEntryPos(mnLastPosGradient);
                        }
                    }
                }
                else
                {
                    mpLbFillAttr->Disable();
                }
                break;
            }
            case XFILL_HATCH:
            {
                mpLbFillAttr->Show();
                mpToolBoxColor->Hide();

                if(pSh && pSh->GetItem(SID_HATCH_LIST))
                {
                    if(!mpLbFillAttr->GetEntryCount())
                    {
                        const SvxHatchListItem aItem( *(const SvxHatchListItem*)(pSh->GetItem(SID_HATCH_LIST)));
                        mpLbFillAttr->Enable();
                        mpLbFillAttr->Clear();
                        mpLbFillAttr->Fill(aItem.GetHatchList());
                    }

                    mpLbFillAttr->AdaptDropDownLineCountToMaximum();

                    if(LISTBOX_ENTRY_NOTFOUND != mnLastPosHatch)
                    {
                        const SvxHatchListItem aItem(*(const SvxHatchListItem*)(pSh->GetItem(SID_HATCH_LIST)));

                        if(mnLastPosHatch < aItem.GetHatchList()->Count())
                        {
                            const XHatch aHatch = aItem.GetHatchList()->GetHatch(mnLastPosHatch)->GetHatch();
                            const XFillHatchItem aXFillHatchItem(mpLbFillAttr->GetSelectEntry(), aHatch);

                            // #i122676# change FillStyle and Hatch in one call
                            GetBindings()->GetDispatcher()->Execute(
                                SID_ATTR_FILL_HATCH, SFX_CALLMODE_RECORD, &aXFillHatchItem, &aXFillStyleItem, 0L);
                            mpLbFillAttr->SelectEntryPos(mnLastPosHatch);
                        }
                    }
                }
                else
                {
                    mpLbFillAttr->Disable();
                }
                break;
            }
            case XFILL_BITMAP:
            {
                mpLbFillAttr->Show();
                mpToolBoxColor->Hide();

                if(pSh && pSh->GetItem(SID_BITMAP_LIST))
                {
                    if(!mpLbFillAttr->GetEntryCount())
                    {
                        const SvxBitmapListItem aItem( *(const SvxBitmapListItem*)(pSh->GetItem(SID_BITMAP_LIST)));
                        mpLbFillAttr->Enable();
                        mpLbFillAttr->Clear();
                        mpLbFillAttr->Fill(aItem.GetBitmapList());
                    }

                    mpLbFillAttr->AdaptDropDownLineCountToMaximum();

                    if(LISTBOX_ENTRY_NOTFOUND != mnLastPosBitmap)
                    {
                        const SvxBitmapListItem aItem(*(const SvxBitmapListItem*)(pSh->GetItem(SID_BITMAP_LIST)));

                        if(mnLastPosBitmap < aItem.GetBitmapList()->Count())
                        {
                            const XBitmapEntry* pXBitmapEntry = aItem.GetBitmapList()->GetBitmap(mnLastPosBitmap);
                            const XFillBitmapItem aXFillBitmapItem(mpLbFillAttr->GetSelectEntry(), pXBitmapEntry->GetGraphicObject());

                            // #i122676# change FillStyle and Bitmap in one call
                            GetBindings()->GetDispatcher()->Execute(
                                SID_ATTR_FILL_BITMAP, SFX_CALLMODE_RECORD, &aXFillBitmapItem, &aXFillStyleItem, 0L);
                            mpLbFillAttr->SelectEntryPos(mnLastPosBitmap);
                        }
                    }
                }
                else
                {
                    mpLbFillAttr->Disable();
                }
                break;
            }
        }

        meLastXFS = (sal_uInt16)eXFS;

        if(XFILL_NONE != eXFS)
        {
            if(pToolBox)
            {
                mpLbFillType->Selected();
            }
        }
    }

    return 0;
}



IMPL_LINK( AreaPropertyPanel, SelectFillAttrHdl, ListBox*, pToolBox )
{
    const XFillStyle eXFS = (XFillStyle)mpLbFillType->GetSelectEntryPos();
    const XFillStyleItem aXFillStyleItem(eXFS);
    SfxObjectShell* pSh = SfxObjectShell::Current();

    if(pToolBox)
    {
        // #i122676# dependent from bFillStyleChange, do execute a single or two
        // changes in one Execute call
        const bool bFillStyleChange((XFillStyle) meLastXFS != eXFS);

        switch(eXFS)
        {
            case XFILL_SOLID:
            {
                if(bFillStyleChange)
                {
                    // #i122676# Single FillStyle change call needed here
                    GetBindings()->GetDispatcher()->Execute(SID_ATTR_FILL_STYLE, SFX_CALLMODE_RECORD, &aXFillStyleItem, 0L);
                }
                break;
            }
            case XFILL_GRADIENT:
            {
                sal_uInt16 nPos = mpLbFillAttr->GetSelectEntryPos();

                if(LISTBOX_ENTRY_NOTFOUND == nPos)
                {
                    nPos = mnLastPosGradient;
                }

                if(LISTBOX_ENTRY_NOTFOUND != nPos && pSh && pSh->GetItem(SID_GRADIENT_LIST))
                {
                    const SvxGradientListItem aItem(*(const SvxGradientListItem*)(pSh->GetItem(SID_GRADIENT_LIST)));

                    if(nPos < aItem.GetGradientList()->Count())
                    {
                        const XGradient aGradient = aItem.GetGradientList()->GetGradient(nPos)->GetGradient();
                        const XFillGradientItem aXFillGradientItem(mpLbFillAttr->GetSelectEntry(), aGradient);

                        // #i122676# Change FillStale and Gradinet in one call
                        GetBindings()->GetDispatcher()->Execute(
                            SID_ATTR_FILL_GRADIENT, SFX_CALLMODE_RECORD, &aXFillGradientItem,
                            bFillStyleChange ? &aXFillStyleItem : 0L, 0L);
                    }
                }

                if(LISTBOX_ENTRY_NOTFOUND != nPos)
                {
                    mnLastPosGradient = nPos;
                }
                break;
            }
            case XFILL_HATCH:
            {
                sal_uInt16 nPos = mpLbFillAttr->GetSelectEntryPos();

                if(LISTBOX_ENTRY_NOTFOUND == nPos)
                {
                    nPos = mnLastPosHatch;
                }

                if(LISTBOX_ENTRY_NOTFOUND != nPos && pSh && pSh->GetItem(SID_HATCH_LIST))
                {
                    const SvxHatchListItem aItem(*(const SvxHatchListItem*)(pSh->GetItem(SID_HATCH_LIST)));

                    if(nPos < aItem.GetHatchList()->Count())
                    {
                        const XHatch aHatch = aItem.GetHatchList()->GetHatch(nPos)->GetHatch();
                        const XFillHatchItem aXFillHatchItem( mpLbFillAttr->GetSelectEntry(), aHatch);

                        // #i122676# Change FillStale and Hatch in one call
                        GetBindings()->GetDispatcher()->Execute(
                            SID_ATTR_FILL_HATCH, SFX_CALLMODE_RECORD, &aXFillHatchItem,
                            bFillStyleChange ? &aXFillStyleItem : 0L, 0L);
                    }
                }

                if(LISTBOX_ENTRY_NOTFOUND != nPos)
                {
                    mnLastPosHatch = nPos;
                }
                break;
            }
            case XFILL_BITMAP:
            {
                sal_uInt16 nPos = mpLbFillAttr->GetSelectEntryPos();

                if(LISTBOX_ENTRY_NOTFOUND == nPos)
                {
                    nPos = mnLastPosBitmap;
                }

                if(LISTBOX_ENTRY_NOTFOUND != nPos && pSh && pSh->GetItem(SID_BITMAP_LIST))
                {
                    const SvxBitmapListItem aItem(*(const SvxBitmapListItem*)(pSh->GetItem(SID_BITMAP_LIST)));

                    if(nPos < aItem.GetBitmapList()->Count())
                    {
                        const XBitmapEntry* pXBitmapEntry = aItem.GetBitmapList()->GetBitmap(nPos);
                        const XFillBitmapItem aXFillBitmapItem(mpLbFillAttr->GetSelectEntry(), pXBitmapEntry->GetGraphicObject());

                        // #i122676# Change FillStale and Bitmap in one call
                        GetBindings()->GetDispatcher()->Execute(
                            SID_ATTR_FILL_BITMAP, SFX_CALLMODE_RECORD, &aXFillBitmapItem,
                            bFillStyleChange ? &aXFillStyleItem : 0L, 0L);
                    }
                }

                if(LISTBOX_ENTRY_NOTFOUND != nPos)
                {
                    mnLastPosBitmap = nPos;
                }
                break;
            }
            default: break;
        }
    }

    return 0;
}


IMPL_LINK(AreaPropertyPanel, ToolBoxColorDropHdl, ToolBox*, pToolBox)
{
    const OUString aCommand(pToolBox->GetItemCommand(pToolBox->GetCurItemId()));

    if(UNO_SIDEBARCOLOR == aCommand)
    {
        maColorPopup.Show(*pToolBox);

        if (mpColorItem)
        {
            maColorPopup.SetCurrentColor(mpColorItem->GetColorValue(), mbColorAvail);
        }
        else
        {
            maColorPopup.SetCurrentColor(COL_WHITE, false);
        }
    }

    return 0;
}



void AreaPropertyPanel::SetColor (
    const String& rsColorName,
    const Color aColor)
{
    const XFillColorItem aXFillColorItem(rsColorName, aColor);
    mpBindings->GetDispatcher()->Execute(SID_ATTR_FILL_COLOR, SFX_CALLMODE_RECORD, &aXFillColorItem, 0L);
    maLastColor = aColor;
}




PopupControl* AreaPropertyPanel::CreateTransparencyGradientControl (PopupContainer* pParent)
{
    return new AreaTransparencyGradientControl(pParent, *this);
}




PopupControl* AreaPropertyPanel::CreateColorPopupControl (PopupContainer* pParent)
{
    return new ColorControl(
        pParent,
        mpBindings,
        SVX_RES(RID_POPUPPANEL_AERAPAGE_COLOR),
        SVX_RES(VS_COLOR),
        ::boost::bind(&AreaPropertyPanel::GetLastColor, this),
        ::boost::bind(&AreaPropertyPanel::SetColor, this, _1,_2),
        pParent,
        0);
}




void AreaPropertyPanel::SetupIcons(void)
{
    if(Theme::GetBoolean(Theme::Bool_UseSymphonyIcons))
    {
        // todo
    }
    else
    {
        // todo
    }
}



AreaPropertyPanel* AreaPropertyPanel::Create (
    Window* pParent,
    const cssu::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
{
    if (pParent == NULL)
        throw lang::IllegalArgumentException(A2S("no parent Window given to AreaPropertyPanel::Create"), NULL, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException(A2S("no XFrame given to AreaPropertyPanel::Create"), NULL, 1);
    if (pBindings == NULL)
        throw lang::IllegalArgumentException(A2S("no SfxBindings given to AreaPropertyPanel::Create"), NULL, 2);

    return new AreaPropertyPanel(
        pParent,
        rxFrame,
        pBindings);
}



void AreaPropertyPanel::DataChanged(
    const DataChangedEvent& rEvent)
{
    (void)rEvent;

    SetupIcons();
}



void AreaPropertyPanel::ImpUpdateTransparencies()
{
    if(mpTransparanceItem.get() && mpFloatTransparenceItem.get())
    {
        bool bZeroValue(false);

        if(mpTransparanceItem.get())
        {
            const sal_uInt16 nValue(mpTransparanceItem->GetValue());

            if(!nValue)
            {
                bZeroValue = true;
            }
            else if(nValue <= 100)
            {
                mpLBTransType->Enable();
                mpTrspTextFT->Enable();
                mpLBTransType->SelectEntryPos(1);
                mpBTNGradient->Hide();
                mpMTRTransparent->Show();
                mpMTRTransparent->Enable();
                mpMTRTransparent->SetValue(nValue);
            }

            if(!bZeroValue)
            {
                maTrGrPopup.Hide();
            }
        }

        if(bZeroValue && mpFloatTransparenceItem.get())
        {
            if(mpFloatTransparenceItem->IsEnabled())
            {
                const XGradient& rGradient = mpFloatTransparenceItem->GetGradientValue();
                sal_uInt16 nEntryPos(0);
                Image* pImage = 0;

                mpLBTransType->Enable();
                mpTrspTextFT->Enable();
                mpMTRTransparent->Hide();
                mpBTNGradient->Enable();
                mpBTNGradient->Show();

                switch(rGradient.GetGradientStyle())
                {
                    case XGRAD_LINEAR:
                    {
                        nEntryPos = 2;
                        pImage = &maImgLinear;
                        break;
                    }
                    case XGRAD_AXIAL:
                    {
                        nEntryPos = 3;
                        pImage = &maImgAxial;
                        break;
                    }
                    case XGRAD_RADIAL:
                    {
                        nEntryPos = 4;
                        pImage = &maImgRadial;
                        break;
                    }
                    case XGRAD_ELLIPTICAL:
                    {
                        nEntryPos = 5;
                        pImage = &maImgElli;
                        break;
                    }
                    case XGRAD_SQUARE:
                    {
                        nEntryPos = 6;
                        pImage = &maImgQuad;
                        break;
                    }
                    case XGRAD_RECT:
                    {
                        nEntryPos = 7;
                        pImage = &maImgSquare;
                        break;
                    }
                }

                const sal_uInt16 nIdGradient = mpBTNGradient->GetItemId(UNO_SIDEBARGRADIENT);
                mpLBTransType->SelectEntryPos(nEntryPos);
                mpBTNGradient->SetItemImage(nIdGradient, *pImage);
                bZeroValue = false;
            }
            else
            {
                bZeroValue = true;
            }
        }

        if(bZeroValue)
        {
            mpLBTransType->Enable();
            mpTrspTextFT->Enable();
            mpLBTransType->SelectEntryPos(0);
            mpBTNGradient->Hide();
            mpMTRTransparent->Enable();
            mpMTRTransparent->Show();
            mpMTRTransparent->SetValue(0);
        }
    }
    else
    {
        // no transparency at all
        mpLBTransType->SetNoSelection();
        mpLBTransType->Disable();
        mpTrspTextFT->Disable();
        mpMTRTransparent->Disable();
        mpMTRTransparent->Show();
        mpBTNGradient->Disable();
        mpBTNGradient->Hide();
    }
}



void AreaPropertyPanel::NotifyItemUpdate(
    sal_uInt16 nSID,
    SfxItemState eState,
    const SfxPoolItem* pState,
    const bool bIsEnabled)
{
    (void)bIsEnabled;
    const bool bDisabled(SFX_ITEM_DISABLED == eState);

    switch(nSID)
    {
        case SID_ATTR_FILL_TRANSPARENCE:
        case SID_ATTR_FILL_FLOATTRANSPARENCE:
        {
            bool bFillTransparenceChanged(false);

            if(SID_ATTR_FILL_TRANSPARENCE == nSID)
            {
                bFillTransparenceChanged = true;

                if(eState >= SFX_ITEM_AVAILABLE)
                {
                    const SfxUInt16Item* pItem = dynamic_cast< const SfxUInt16Item* >(pState);

                    if(pItem && (!mpTransparanceItem || *pItem != *mpTransparanceItem))
                    {
                        mpTransparanceItem.reset((SfxUInt16Item*)pItem->Clone());
                    }
                    else
                    {
                        mpTransparanceItem.reset();
                    }
                }
                else
                {
                    mpTransparanceItem.reset();
                }
            }
            else // if(SID_ATTR_FILL_FLOATTRANSPARENCE == nSID)
            {
                bFillTransparenceChanged = true;

                if(eState >= SFX_ITEM_AVAILABLE)
                {
                    const XFillFloatTransparenceItem* pItem = dynamic_cast< const XFillFloatTransparenceItem* >(pState);

                    if(pItem && (!mpFloatTransparenceItem || *pItem != *mpFloatTransparenceItem))
                    {
                        mpFloatTransparenceItem.reset((XFillFloatTransparenceItem*)pItem->Clone());
                    }
                    else
                    {
                        mpFloatTransparenceItem.reset();
                    }
                }
                else
                {
                    mpFloatTransparenceItem.reset();
                }
            }

            if(bFillTransparenceChanged)
            {
                // update transparency settings dependent of mpTransparanceItem and mpFloatTransparenceItem
                ImpUpdateTransparencies();
            }
            break;
        }
        case SID_ATTR_FILL_STYLE:
        {
            if(bDisabled)
            {
                mpLbFillType->Disable();
                mpColorTextFT->Disable();
                mpLbFillType->SetNoSelection();
                mpLbFillAttr->Show();
                mpLbFillAttr->Disable();
                mpLbFillAttr->SetNoSelection();
                mpToolBoxColor->Hide();
                meLastXFS = -1;
                mpStyleItem.reset();
            }

            if(eState >= SFX_ITEM_AVAILABLE)
            {
                const XFillStyleItem* pItem = dynamic_cast< const XFillStyleItem* >(pState);

                if(pItem)
                {
                    mpStyleItem.reset(dynamic_cast< XFillStyleItem* >(pItem->Clone()));
                    mpLbFillType->Enable();
                    mpColorTextFT->Enable();
                    XFillStyle eXFS = (XFillStyle)mpStyleItem->GetValue();
                    meLastXFS = eXFS;
                    mpLbFillType->SelectEntryPos(sal::static_int_cast< sal_uInt16 >(eXFS));

                    if(XFILL_NONE == eXFS)
                    {
                        mpLbFillAttr->SetNoSelection();
                        mpLbFillAttr->Disable();
                    }

                    Update();
                    break;
                }
            }

            mpLbFillType->SetNoSelection();
            mpLbFillAttr->Show();
            mpLbFillAttr->Disable();
            mpLbFillAttr->SetNoSelection();
            mpToolBoxColor->Hide();
            meLastXFS = -1;
            mpStyleItem.reset();
            break;
        }
        case SID_ATTR_FILL_COLOR:
        {
            if(SFX_ITEM_AVAILABLE == eState)
            {
                mpColorItem.reset(pState ? (XFillColorItem*)pState->Clone() : 0);
            }

            if(mpStyleItem && XFILL_SOLID == (XFillStyle)mpStyleItem->GetValue())
            {
                mpLbFillAttr->Hide();
                mpToolBoxColor->Show();

                if(SFX_ITEM_AVAILABLE == eState)
                {
                    mpToolBoxColor->Enable();
                    mbColorAvail = true;
                    // maLastColor = mpColorItem->GetColorValue();
                    Update();
                }
                else if(SFX_ITEM_DISABLED == eState)
                {
                    mpToolBoxColor->Disable();
                    mbColorAvail = false;
                    mpColorUpdater->Update(COL_WHITE);
                }
                else
                {
                    mbColorAvail = false;
                    mpColorUpdater->Update(COL_WHITE);
                }
            }
            break;
        }
        case SID_ATTR_FILL_GRADIENT:
        {
            if(SFX_ITEM_AVAILABLE == eState)
            {
                mpFillGradientItem.reset(pState ? (XFillGradientItem*)pState->Clone() : 0);
            }

            if(mpStyleItem && XFILL_GRADIENT == (XFillStyle)mpStyleItem->GetValue())
            {
                mpLbFillAttr->Show();
                mpToolBoxColor->Hide();

                if(SFX_ITEM_AVAILABLE == eState)
                {
                    mpLbFillAttr->Enable();
                    Update();
                }
                else if(SFX_ITEM_DISABLED == eState )
                {
                    mpLbFillAttr->Disable();
                    mpLbFillAttr->SetNoSelection();
                }
                else
                {
                    mpLbFillAttr->SetNoSelection();
                }
            }
            break;
        }
        case SID_ATTR_FILL_HATCH:
        {
            if(SFX_ITEM_AVAILABLE == eState)
            {
                mpHatchItem.reset(pState ? (XFillHatchItem*)pState->Clone() : 0);
            }

            if(mpStyleItem && XFILL_HATCH == (XFillStyle)mpStyleItem->GetValue())
            {
                mpLbFillAttr->Show();
                mpToolBoxColor->Hide();

                if(SFX_ITEM_AVAILABLE == eState)
                {
                    mpLbFillAttr->Enable();
                    Update();
                }
                else if(SFX_ITEM_DISABLED == eState )
                {
                    mpLbFillAttr->Disable();
                    mpLbFillAttr->SetNoSelection();
                }
                else
                {
                    mpLbFillAttr->SetNoSelection();
                }
            }
            break;
        }
        case SID_ATTR_FILL_BITMAP:
        {
            if(SFX_ITEM_AVAILABLE == eState)
            {
                mpBitmapItem.reset(pState ? (XFillBitmapItem*)pState->Clone() : 0);
            }

            if(mpStyleItem && XFILL_BITMAP == (XFillStyle)mpStyleItem->GetValue())
            {
                mpLbFillAttr->Show();
                mpToolBoxColor->Hide();

                if(SFX_ITEM_AVAILABLE == eState)
                {
                    mpLbFillAttr->Enable();
                    Update();
                }
                else if(SFX_ITEM_DISABLED == eState )
                {
                    mpLbFillAttr->Disable();
                    mpLbFillAttr->SetNoSelection();
                }
                else
                {
                    mpLbFillAttr->SetNoSelection();
                }
            }
            break;
        }
        case SID_COLOR_TABLE:
        {
            if(SFX_ITEM_AVAILABLE == eState)
            {
                if(mpStyleItem && XFILL_SOLID == (XFillStyle)mpStyleItem->GetValue())
                {
                    if(mpColorItem)
                    {
                        const Color aColor = mpColorItem->GetColorValue();
                        const SfxObjectShell* pSh = SfxObjectShell::Current();
                        const SvxColorListItem aItem(*(const SvxColorListItem*)(pSh->GetItem(SID_COLOR_TABLE)));

                        mpLbFillAttr->Clear();
                        mpLbFillAttr->Enable();
                        mpLbFillAttr->Fill(aItem.GetColorList());
                        mpLbFillAttr->SelectEntry(aColor);
                    }
                    else
                    {
                        mpLbFillAttr->SetNoSelection();
                    }
                }
            }
            break;
        }
        case SID_GRADIENT_LIST:
        {
            if(SFX_ITEM_AVAILABLE == eState)
            {
                if(mpStyleItem && XFILL_GRADIENT == (XFillStyle)mpStyleItem->GetValue())
                {
                    if(mpFillGradientItem)
                    {
                        const String aString( mpFillGradientItem->GetName() );
                        const SfxObjectShell* pSh = SfxObjectShell::Current();
                        const SvxGradientListItem aItem( *(const SvxGradientListItem*)(pSh->GetItem(SID_GRADIENT_LIST)));

                        mpLbFillAttr->Clear();
                        mpLbFillAttr->Enable();
                        mpLbFillAttr->Fill(aItem.GetGradientList());
                        mpLbFillAttr->SelectEntry(aString);
                    }
                    else
                    {
                        mpLbFillAttr->SetNoSelection();
                    }
                }
            }
            break;
        }
        case SID_HATCH_LIST:
        {
            if(SFX_ITEM_AVAILABLE == eState)
            {
                if(mpStyleItem && XFILL_HATCH == (XFillStyle)mpStyleItem->GetValue())
                {
                    if(mpHatchItem)
                    {
                        const String aString( mpHatchItem->GetName() );
                        const SfxObjectShell* pSh = SfxObjectShell::Current();
                        const SvxHatchListItem aItem(*(const SvxHatchListItem*)(pSh->GetItem(SID_HATCH_LIST)));

                        mpLbFillAttr->Clear();
                        mpLbFillAttr->Enable();
                        mpLbFillAttr->Fill(aItem.GetHatchList());
                        mpLbFillAttr->SelectEntry(aString);
                    }
                    else
                    {
                        mpLbFillAttr->SetNoSelection();
                    }
                }
            }
            break;
        }
        case SID_BITMAP_LIST:
        {
            if(SFX_ITEM_AVAILABLE == eState)
            {
                if(mpStyleItem && XFILL_BITMAP == (XFillStyle)mpStyleItem->GetValue())
                {
                    if(mpBitmapItem)
                    {
                        const String aString( mpBitmapItem->GetName() );
                        const SfxObjectShell* pSh = SfxObjectShell::Current();
                        const SvxBitmapListItem aItem(*(const SvxBitmapListItem*)(pSh->GetItem(SID_BITMAP_LIST)));

                        mpLbFillAttr->Clear();
                        mpLbFillAttr->Enable();
                        mpLbFillAttr->Fill(aItem.GetBitmapList());
                        mpLbFillAttr->SelectEntry(aString);
                    }
                    else
                    {
                        mpLbFillAttr->SetNoSelection();
                    }
                }
            }
            break;
        }
    }
}



SfxBindings* AreaPropertyPanel::GetBindings()
{
    return mpBindings;
}



void AreaPropertyPanel::Update()
{
    if(mpStyleItem)
    {
        const XFillStyle eXFS = (XFillStyle)mpStyleItem->GetValue();
        SfxObjectShell* pSh = SfxObjectShell::Current();

        switch( eXFS )
        {
            case XFILL_NONE:
            {
                mpLbFillAttr->Show();
                mpToolBoxColor->Hide();
                break;
            }
            case XFILL_SOLID:
            {
                if(mpColorItem)
                {
                    mpLbFillAttr->Hide();
                    mpToolBoxColor->Show();
                    mpColorUpdater->Update(mpColorItem->GetColorValue());
                }
                else
                {
                    mpColorUpdater->Update(COL_WHITE);
                }
                break;
            }
            case XFILL_GRADIENT:
            {
                mpLbFillAttr->Show();
                mpToolBoxColor->Hide();

                if(pSh && pSh->GetItem(SID_GRADIENT_LIST))
                {
                    const SvxGradientListItem aItem(*(const SvxGradientListItem*)(pSh->GetItem(SID_GRADIENT_LIST)));
                    mpLbFillAttr->Enable();
                    mpLbFillAttr->Clear();
                    mpLbFillAttr->Fill(aItem.GetGradientList());

                    if(mpFillGradientItem)
                    {
                        const String aString(mpFillGradientItem->GetName());

                        mpLbFillAttr->SelectEntry(aString);
                    }
                    else
                    {
                        mpLbFillAttr->SetNoSelection();
                    }
                }
                else
                {
                    mpLbFillAttr->SetNoSelection();
                }
                break;
            }
            case XFILL_HATCH:
            {
                mpLbFillAttr->Show();
                mpToolBoxColor->Hide();

                if(pSh && pSh->GetItem(SID_HATCH_LIST))
                {
                    const SvxHatchListItem aItem(*(const SvxHatchListItem*)(pSh->GetItem(SID_HATCH_LIST)));
                    mpLbFillAttr->Enable();
                    mpLbFillAttr->Clear();
                    mpLbFillAttr->Fill(aItem.GetHatchList());

                    if(mpHatchItem)
                    {
                        const String aString(mpHatchItem->GetName());

                        mpLbFillAttr->SelectEntry( aString );
                    }
                    else
                    {
                        mpLbFillAttr->SetNoSelection();
                    }
                }
                else
                {
                    mpLbFillAttr->SetNoSelection();
                }
                break;
            }
            case XFILL_BITMAP:
            {
                mpLbFillAttr->Show();
                mpToolBoxColor->Hide();

                if(pSh && pSh->GetItem(SID_BITMAP_LIST))
                {
                    const SvxBitmapListItem aItem(*(const SvxBitmapListItem*)(pSh->GetItem(SID_BITMAP_LIST)));
                    mpLbFillAttr->Enable();
                    mpLbFillAttr->Clear();
                    mpLbFillAttr->Fill(aItem.GetBitmapList());

                    if(mpBitmapItem)
                    {
                        const String aString(mpBitmapItem->GetName());

                        mpLbFillAttr->SelectEntry(aString);
                    }
                    else
                    {
                        mpLbFillAttr->SetNoSelection();
                    }
                }
                else
                {
                    mpLbFillAttr->SetNoSelection();
                }
                break;
            }
            default:
                OSL_ENSURE(false, "Non supported FillType (!)");
            break;
        }
    }
}



IMPL_LINK( AreaPropertyPanel, ImplPopupModeEndHdl, FloatingWindow*, EMPTYARG )
{
    return 0;
}



IMPL_LINK( AreaPropertyPanel, ClickTrGrHdl_Impl, ToolBox*, pToolBox )
{
    maTrGrPopup.Rearrange(mpFloatTransparenceItem.get());
    OSL_ASSERT( pToolBox->GetItemCommand(pToolBox->GetCurItemId()) == UNO_SIDEBARGRADIENT);
    maTrGrPopup.Show(*pToolBox);

    return (0L);
}



IMPL_LINK(AreaPropertyPanel, ChangeTrgrTypeHdl_Impl, void *, EMPTYARG)
{
    sal_uInt16 nSelectType = mpLBTransType->GetSelectEntryPos();
    bool bGradient = false;
    sal_uInt16 nTrans = 0;

    if(!nSelectType)
    {
        mpBTNGradient->Hide();
        mpMTRTransparent->Show();
        mpMTRTransparent->Enable();
        mpMTRTransparent->SetValue(0);
    }
    else if(1 == nSelectType)
    {
        mpBTNGradient->Hide();
        mpMTRTransparent->Show();
        nTrans = mnLastTransSolid;
        mpMTRTransparent->SetValue(nTrans);
        mpLBTransType->SelectEntryPos(1);
        mpMTRTransparent->Enable();
    }
    else
    {
        mpBTNGradient->Show();

        const sal_uInt16 nIdGradient = mpBTNGradient->GetItemId(UNO_SIDEBARGRADIENT);
        switch (nSelectType)
        {
            case 2:
                mpBTNGradient->SetItemImage(nIdGradient, maImgLinear);
                break;
            case 3:
                mpBTNGradient->SetItemImage(nIdGradient, maImgAxial);
                break;
            case 4:
                mpBTNGradient->SetItemImage(nIdGradient, maImgRadial);
                break;
            case 5:
                mpBTNGradient->SetItemImage(nIdGradient, maImgElli );
                break;
            case 6:
                mpBTNGradient->SetItemImage(nIdGradient, maImgQuad );
                break;
            case 7:
                mpBTNGradient->SetItemImage(nIdGradient, maImgSquare);
                break;
        }

        mpMTRTransparent->Hide();
        mpBTNGradient->Enable();
        bGradient = true;
    }

    const XFillTransparenceItem aLinearItem(nTrans);
    GetBindings()->GetDispatcher()->Execute( SID_ATTR_FILL_TRANSPARENCE, SFX_CALLMODE_RECORD, &aLinearItem, 0L );

    if(nSelectType > 1)
    {
        nSelectType -= 2;
    }

    XGradient aTmpGradient;

    switch(nSelectType)
    {
        case XGRAD_LINEAR:
            aTmpGradient = maGradientLinear;
            break;
        case XGRAD_AXIAL:
            aTmpGradient = maGradientAxial;
            break;
        case XGRAD_RADIAL:
            aTmpGradient = maGradientRadial;
            break;
        case XGRAD_ELLIPTICAL:
            aTmpGradient = maGradientElliptical;
            break;
        case XGRAD_SQUARE:
            aTmpGradient = maGradientSquare;
            break;
        case XGRAD_RECT:
            aTmpGradient = maGradientRect;
            break;
    }

    SfxItemPool* pPool = 0;
    const XFillFloatTransparenceItem aGradientItem(pPool, aTmpGradient, sal_Bool(bGradient));
    GetBindings()->GetDispatcher()->Execute( SID_ATTR_FILL_FLOATTRANSPARENCE, SFX_CALLMODE_RECORD, &aGradientItem, 0L );

    return( 0L );
}



IMPL_LINK(AreaPropertyPanel, ModifyTransparentHdl_Impl, void*, EMPTYARG)
{
    const sal_uInt16 nTrans = (sal_uInt16)mpMTRTransparent->GetValue();
    mnLastTransSolid = nTrans;
    const sal_uInt16 nSelectType = mpLBTransType->GetSelectEntryPos();

    if(nTrans && !nSelectType)
    {
        mpLBTransType->SelectEntryPos(1);
    }

    const XFillTransparenceItem aLinearItem(nTrans);
    GetBindings()->GetDispatcher()->Execute( SID_ATTR_FILL_TRANSPARENCE, SFX_CALLMODE_RECORD, &aLinearItem, 0L );

    return 0L;
}


Color AreaPropertyPanel::GetLastColor (void) const
{
    return maLastColor;
}




XGradient AreaPropertyPanel::GetGradient (const XGradientStyle eStyle) const
{
    switch (eStyle)
    {
        default:
        case XGRAD_LINEAR:
            return maGradientLinear;
        case XGRAD_AXIAL:
            return maGradientAxial;
        case XGRAD_RADIAL:
            return maGradientRadial;
        case XGRAD_ELLIPTICAL:
            return maGradientElliptical;
        case XGRAD_SQUARE:
            return maGradientSquare;
        case XGRAD_RECT:
            return maGradientRect;
    }
}




void AreaPropertyPanel::SetGradient (const XGradient& rGradient)
{
    switch (rGradient.GetGradientStyle())
    {
        case XGRAD_LINEAR:
            maGradientLinear = rGradient;
            break;
        case XGRAD_AXIAL:
            maGradientAxial = rGradient;
            break;
        case XGRAD_RADIAL:
            maGradientRadial = rGradient;
            break;
        case XGRAD_ELLIPTICAL:
            maGradientElliptical = rGradient;
            break;
        case XGRAD_SQUARE:
            maGradientSquare = rGradient;
            break;
        case XGRAD_RECT:
            maGradientRect = rGradient;
            break;
    }
}




sal_Int32 AreaPropertyPanel::GetSelectedTransparencyTypeIndex (void) const
{
    return mpLBTransType->GetSelectEntryPos();
}

} } // end of namespace svx::sidebar

// eof
