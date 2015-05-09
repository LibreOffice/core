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

#include <boost/bind.hpp>

using namespace css;
using namespace css::uno;
using ::sfx2::sidebar::Theme;

const char UNO_SIDEBARGRADIENT[] = ".uno:sidebargradient";

namespace svx { namespace sidebar {

const sal_Int32 AreaPropertyPanel::DEFAULT_CENTERX = 50;
const sal_Int32 AreaPropertyPanel::DEFAULT_CENTERY = 50;
const sal_Int32 AreaPropertyPanel::DEFAULT_ANGLE = 0;
const sal_Int32 AreaPropertyPanel::DEFAULT_STARTVALUE = 0;
const sal_Int32 AreaPropertyPanel::DEFAULT_ENDVALUE = 16777215;
const sal_Int32 AreaPropertyPanel::DEFAULT_BORDER = 0;


AreaPropertyPanel::AreaPropertyPanel(
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
    : PanelLayout(pParent, "AreaPropertyPanel", "svx/ui/sidebararea.ui", rxFrame),
      meLastXFS(static_cast<sal_uInt16>(-1)),
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
      maTrGrPopup(this, ::boost::bind(&AreaPropertyPanel::CreateTransparencyGradientControl, this, _1)),
      mpFloatTransparenceItem(),
      mpTransparanceItem(),
      mxFrame(rxFrame),
      mpBindings(pBindings)
{
    get(mpColorTextFT,    "filllabel");
    get(mpLbFillType,     "fillstyle");
    get(mpLbFillAttr,     "fillattr");
    get(mpTrspTextFT,     "transparencylabel");
    get(mpToolBoxColor,   "selectcolor");
    get(mpLBTransType,    "transtype");
    get(mpMTRTransparent, "settransparency");
    get(mpBTNGradient,    "selectgradient");

    Initialize();
}


AreaPropertyPanel::~AreaPropertyPanel()
{
    disposeOnce();
}

void AreaPropertyPanel::dispose()
{
    mpColorTextFT.clear();
    mpLbFillType.clear();
    mpLbFillAttr.clear();
    mpToolBoxColor.clear();
    mpTrspTextFT.clear();
    mpLBTransType.clear();
    mpMTRTransparent.clear();
    mpBTNGradient.clear();

    maStyleControl.dispose();
    maColorControl.dispose();
    maGradientControl.dispose();
    maHatchControl.dispose();
    maBitmapControl.dispose();
    maGradientListControl.dispose();
    maHatchListControl.dispose();
    maBitmapListControl.dispose();
    maFillTransparenceController.dispose();
    maFillFloatTransparenceController.dispose();

    PanelLayout::dispose();
}

void AreaPropertyPanel::Initialize()
{
    maGradientLinear.SetXOffset(DEFAULT_CENTERX);
    maGradientLinear.SetYOffset(DEFAULT_CENTERY);
    maGradientLinear.SetAngle(DEFAULT_ANGLE);
    maGradientLinear.SetStartColor(Color(DEFAULT_STARTVALUE));
    maGradientLinear.SetEndColor(Color(DEFAULT_ENDVALUE));
    maGradientLinear.SetBorder(DEFAULT_BORDER);
    maGradientLinear.SetGradientStyle(css::awt::GradientStyle_LINEAR);

    maGradientAxial = maGradientLinear;
    maGradientAxial.SetGradientStyle(css::awt::GradientStyle_AXIAL);

    maGradientRadial = maGradientLinear;
    maGradientRadial.SetGradientStyle(css::awt::GradientStyle_RADIAL);

    maGradientElliptical = maGradientLinear;
    maGradientElliptical.SetGradientStyle(css::awt::GradientStyle_ELLIPTICAL);

    maGradientSquare = maGradientLinear;
    maGradientSquare.SetGradientStyle(css::awt::GradientStyle_SQUARE);

    maGradientRect = maGradientLinear;
    maGradientRect.SetGradientStyle(css::awt::GradientStyle_RECT);

    mpLbFillType->SetAccessibleName(OUString( "Fill"));    //wj acc
    mpLbFillAttr->SetAccessibleName(OUString( "Fill"));    //wj acc

    Link<> aLink = LINK( this, AreaPropertyPanel, SelectFillTypeHdl );
    mpLbFillType->SetSelectHdl( aLink );

    aLink = LINK( this, AreaPropertyPanel, SelectFillAttrHdl );
    mpLbFillAttr->SetSelectHdl( aLink );

    mpLBTransType->SetSelectHdl(LINK(this, AreaPropertyPanel, ChangeTrgrTypeHdl_Impl));
    mpLBTransType->SetAccessibleName(OUString( "Transparency"));    //wj acc

    mpMTRTransparent->SetValue( 50 );
    mpMTRTransparent->SetModifyHdl(LINK(this, AreaPropertyPanel, ModifyTransparentHdl_Impl));
    mpMTRTransparent->SetAccessibleName(OUString( "Transparency"));    //wj acc

    const sal_uInt16 nIdGradient = mpBTNGradient->GetItemId(UNO_SIDEBARGRADIENT);
    mpBTNGradient->SetItemBits( nIdGradient, mpBTNGradient->GetItemBits( nIdGradient ) | ToolBoxItemBits::DROPDOWNONLY );
    Link<ToolBox *, void> aLink2 = LINK( this, AreaPropertyPanel, ClickTrGrHdl_Impl );
    mpBTNGradient->SetDropdownClickHdl( aLink2 );
    mpBTNGradient->SetSelectHdl( aLink2 );
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

IMPL_LINK(AreaPropertyPanel, SelectFillTypeHdl, ListBox *, pToolBox)
{
    const drawing::FillStyle eXFS = (drawing::FillStyle)mpLbFillType->GetSelectEntryPos();

    if((drawing::FillStyle)meLastXFS != eXFS)
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
            default:
            case drawing::FillStyle_NONE:
            {
                mpLbFillAttr->Show();
                mpToolBoxColor->Hide();
                mpLbFillType->Selected();
                mpLbFillAttr->Disable();

                // #i122676# need to call a single SID_ATTR_FILL_STYLE change
                GetBindings()->GetDispatcher()->Execute(
                    SID_ATTR_FILL_STYLE, SfxCallMode::RECORD, &aXFillStyleItem, 0L);
                break;
            }
            case drawing::FillStyle_SOLID:
            {
                mpLbFillAttr->Hide();
                mpToolBoxColor->Show();
                const OUString aTmpStr;
                const Color aColor = mpColorItem->GetColorValue();
                const XFillColorItem aXFillColorItem( aTmpStr, aColor );

                // #i122676# change FillStyle and Color in one call
                GetBindings()->GetDispatcher()->Execute(
                    SID_ATTR_FILL_COLOR, SfxCallMode::RECORD, &aXFillColorItem, &aXFillStyleItem, 0L);
                break;
            }
            case drawing::FillStyle_GRADIENT:
            {
                mpLbFillAttr->Show();
                mpToolBoxColor->Hide();

                if(pSh && pSh->GetItem(SID_GRADIENT_LIST))
                {
                    if(!mpLbFillAttr->GetEntryCount())
                    {
                        const SvxGradientListItem aItem(*static_cast<const SvxGradientListItem*>(pSh->GetItem(SID_GRADIENT_LIST)));
                        mpLbFillAttr->Enable();
                        mpLbFillAttr->Clear();
                        mpLbFillAttr->Fill(aItem.GetGradientList());
                    }

                    mpLbFillAttr->AdaptDropDownLineCountToMaximum();

                    if(LISTBOX_ENTRY_NOTFOUND != mnLastPosGradient)
                    {
                        const SvxGradientListItem aItem(*static_cast<const SvxGradientListItem*>(pSh->GetItem(SID_GRADIENT_LIST)));

                        if(mnLastPosGradient < aItem.GetGradientList()->Count())
                        {
                            const XGradient aGradient = aItem.GetGradientList()->GetGradient(mnLastPosGradient)->GetGradient();
                            const XFillGradientItem aXFillGradientItem(mpLbFillAttr->GetEntry(mnLastPosGradient), aGradient);

                            // #i122676# change FillStyle and Gradient in one call
                            GetBindings()->GetDispatcher()->Execute(
                                SID_ATTR_FILL_GRADIENT, SfxCallMode::RECORD, &aXFillGradientItem, &aXFillStyleItem, 0L);
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
            case drawing::FillStyle_HATCH:
            {
                mpLbFillAttr->Show();
                mpToolBoxColor->Hide();

                if(pSh && pSh->GetItem(SID_HATCH_LIST))
                {
                    if(!mpLbFillAttr->GetEntryCount())
                    {
                        const SvxHatchListItem aItem( *static_cast<const SvxHatchListItem*>(pSh->GetItem(SID_HATCH_LIST)));
                        mpLbFillAttr->Enable();
                        mpLbFillAttr->Clear();
                        mpLbFillAttr->Fill(aItem.GetHatchList());
                    }

                    mpLbFillAttr->AdaptDropDownLineCountToMaximum();

                    if(LISTBOX_ENTRY_NOTFOUND != mnLastPosHatch)
                    {
                        const SvxHatchListItem aItem(*static_cast<const SvxHatchListItem*>(pSh->GetItem(SID_HATCH_LIST)));

                        if(mnLastPosHatch < aItem.GetHatchList()->Count())
                        {
                            const XHatch aHatch = aItem.GetHatchList()->GetHatch(mnLastPosHatch)->GetHatch();
                            const XFillHatchItem aXFillHatchItem(mpLbFillAttr->GetSelectEntry(), aHatch);

                            // #i122676# change FillStyle and Hatch in one call
                            GetBindings()->GetDispatcher()->Execute(
                                SID_ATTR_FILL_HATCH, SfxCallMode::RECORD, &aXFillHatchItem, &aXFillStyleItem, 0L);
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
            case drawing::FillStyle_BITMAP:
            {
                mpLbFillAttr->Show();
                mpToolBoxColor->Hide();

                if(pSh && pSh->GetItem(SID_BITMAP_LIST))
                {
                    if(!mpLbFillAttr->GetEntryCount())
                    {
                        const SvxBitmapListItem aItem( *static_cast<const SvxBitmapListItem*>(pSh->GetItem(SID_BITMAP_LIST)));
                        mpLbFillAttr->Enable();
                        mpLbFillAttr->Clear();
                        mpLbFillAttr->Fill(aItem.GetBitmapList());
                    }

                    mpLbFillAttr->AdaptDropDownLineCountToMaximum();

                    if(LISTBOX_ENTRY_NOTFOUND != mnLastPosBitmap)
                    {
                        const SvxBitmapListItem aItem(*static_cast<const SvxBitmapListItem*>(pSh->GetItem(SID_BITMAP_LIST)));

                        if(mnLastPosBitmap < aItem.GetBitmapList()->Count())
                        {
                            const XBitmapEntry* pXBitmapEntry = aItem.GetBitmapList()->GetBitmap(mnLastPosBitmap);
                            const XFillBitmapItem aXFillBitmapItem(mpLbFillAttr->GetSelectEntry(), pXBitmapEntry->GetGraphicObject());

                            // #i122676# change FillStyle and Bitmap in one call
                            GetBindings()->GetDispatcher()->Execute(
                                SID_ATTR_FILL_BITMAP, SfxCallMode::RECORD, &aXFillBitmapItem, &aXFillStyleItem, 0L);
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

        if(drawing::FillStyle_NONE != eXFS)
        {
            if(pToolBox)
            {
                mpLbFillType->Selected();
            }
        }
    }

    return 0;
}

IMPL_LINK(AreaPropertyPanel, SelectFillAttrHdl, ListBox*, pToolBox)
{
    const drawing::FillStyle eXFS = (drawing::FillStyle)mpLbFillType->GetSelectEntryPos();
    const XFillStyleItem aXFillStyleItem(eXFS);
    SfxObjectShell* pSh = SfxObjectShell::Current();

    if(pToolBox)
    {
        // #i122676# dependent from bFillStyleChange, do execute a single or two
        // changes in one Execute call
        const bool bFillStyleChange((drawing::FillStyle) meLastXFS != eXFS);

        switch(eXFS)
        {
            case drawing::FillStyle_SOLID:
            {
                if(bFillStyleChange)
                {
                    // #i122676# Single FillStyle change call needed here
                    GetBindings()->GetDispatcher()->Execute(SID_ATTR_FILL_STYLE, SfxCallMode::RECORD, &aXFillStyleItem, 0L);
                }
                break;
            }
            case drawing::FillStyle_GRADIENT:
            {
                sal_Int32 nPos = mpLbFillAttr->GetSelectEntryPos();

                if(LISTBOX_ENTRY_NOTFOUND == nPos)
                {
                    nPos = mnLastPosGradient;
                }

                if(LISTBOX_ENTRY_NOTFOUND != nPos && pSh && pSh->GetItem(SID_GRADIENT_LIST))
                {
                    const SvxGradientListItem aItem(*static_cast<const SvxGradientListItem*>(pSh->GetItem(SID_GRADIENT_LIST)));

                    if(nPos < aItem.GetGradientList()->Count())
                    {
                        const XGradient aGradient = aItem.GetGradientList()->GetGradient(nPos)->GetGradient();
                        const XFillGradientItem aXFillGradientItem(mpLbFillAttr->GetSelectEntry(), aGradient);

                        // #i122676# Change FillStale and Gradinet in one call
                        GetBindings()->GetDispatcher()->Execute(
                            SID_ATTR_FILL_GRADIENT, SfxCallMode::RECORD, &aXFillGradientItem,
                            bFillStyleChange ? &aXFillStyleItem : 0L, 0L);
                    }
                }

                if(LISTBOX_ENTRY_NOTFOUND != nPos)
                {
                    mnLastPosGradient = nPos;
                }
                break;
            }
            case drawing::FillStyle_HATCH:
            {
                sal_Int32 nPos = mpLbFillAttr->GetSelectEntryPos();

                if(LISTBOX_ENTRY_NOTFOUND == nPos)
                {
                    nPos = mnLastPosHatch;
                }

                if(LISTBOX_ENTRY_NOTFOUND != nPos && pSh && pSh->GetItem(SID_HATCH_LIST))
                {
                    const SvxHatchListItem aItem(*static_cast<const SvxHatchListItem*>(pSh->GetItem(SID_HATCH_LIST)));

                    if(nPos < aItem.GetHatchList()->Count())
                    {
                        const XHatch aHatch = aItem.GetHatchList()->GetHatch(nPos)->GetHatch();
                        const XFillHatchItem aXFillHatchItem( mpLbFillAttr->GetSelectEntry(), aHatch);

                        // #i122676# Change FillStale and Hatch in one call
                        GetBindings()->GetDispatcher()->Execute(
                            SID_ATTR_FILL_HATCH, SfxCallMode::RECORD, &aXFillHatchItem,
                            bFillStyleChange ? &aXFillStyleItem : 0L, 0L);
                    }
                }

                if(LISTBOX_ENTRY_NOTFOUND != nPos)
                {
                    mnLastPosHatch = nPos;
                }
                break;
            }
            case drawing::FillStyle_BITMAP:
            {
                sal_Int32 nPos = mpLbFillAttr->GetSelectEntryPos();

                if(LISTBOX_ENTRY_NOTFOUND == nPos)
                {
                    nPos = mnLastPosBitmap;
                }

                if(LISTBOX_ENTRY_NOTFOUND != nPos && pSh && pSh->GetItem(SID_BITMAP_LIST))
                {
                    const SvxBitmapListItem aItem(*static_cast<const SvxBitmapListItem*>(pSh->GetItem(SID_BITMAP_LIST)));

                    if(nPos < aItem.GetBitmapList()->Count())
                    {
                        const XBitmapEntry* pXBitmapEntry = aItem.GetBitmapList()->GetBitmap(nPos);
                        const XFillBitmapItem aXFillBitmapItem(mpLbFillAttr->GetSelectEntry(), pXBitmapEntry->GetGraphicObject());

                        // #i122676# Change FillStale and Bitmap in one call
                        GetBindings()->GetDispatcher()->Execute(
                            SID_ATTR_FILL_BITMAP, SfxCallMode::RECORD, &aXFillBitmapItem,
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


VclPtr<PopupControl> AreaPropertyPanel::CreateTransparencyGradientControl (PopupContainer* pParent)
{
    return VclPtrInstance<AreaTransparencyGradientControl>(pParent, *this);
}




void AreaPropertyPanel::SetupIcons()
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



VclPtr<vcl::Window> AreaPropertyPanel::Create (
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
{
    if (pParent == NULL)
        throw lang::IllegalArgumentException("no parent Window given to AreaPropertyPanel::Create", NULL, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to AreaPropertyPanel::Create", NULL, 1);
    if (pBindings == NULL)
        throw lang::IllegalArgumentException("no SfxBindings given to AreaPropertyPanel::Create", NULL, 2);

    return VclPtr<AreaPropertyPanel>::Create(
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
                sal_Int32 nEntryPos(0);
                Image* pImage = 0;

                mpLBTransType->Enable();
                mpTrspTextFT->Enable();
                mpMTRTransparent->Hide();
                mpBTNGradient->Enable();
                mpBTNGradient->Show();

                switch(rGradient.GetGradientStyle())
                {
                    default:
                    case css::awt::GradientStyle_LINEAR:
                    {
                        nEntryPos = 2;
                        pImage = &maImgLinear;
                        break;
                    }
                    case css::awt::GradientStyle_AXIAL:
                    {
                        nEntryPos = 3;
                        pImage = &maImgAxial;
                        break;
                    }
                    case css::awt::GradientStyle_RADIAL:
                    {
                        nEntryPos = 4;
                        pImage = &maImgRadial;
                        break;
                    }
                    case css::awt::GradientStyle_ELLIPTICAL:
                    {
                        nEntryPos = 5;
                        pImage = &maImgElli;
                        break;
                    }
                    case css::awt::GradientStyle_SQUARE:
                    {
                        nEntryPos = 6;
                        pImage = &maImgQuad;
                        break;
                    }
                    case css::awt::GradientStyle_RECT:
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
    const bool bDisabled(SfxItemState::DISABLED == eState);

    switch(nSID)
    {
        case SID_ATTR_FILL_TRANSPARENCE:
        case SID_ATTR_FILL_FLOATTRANSPARENCE:
        {
            bool bFillTransparenceChanged(false);

            if(SID_ATTR_FILL_TRANSPARENCE == nSID)
            {
                bFillTransparenceChanged = true;

                if(eState >= SfxItemState::DEFAULT)
                {
                    const SfxUInt16Item* pItem = dynamic_cast< const SfxUInt16Item* >(pState);

                    if(pItem && (!mpTransparanceItem || *pItem != *mpTransparanceItem))
                    {
                        mpTransparanceItem.reset(static_cast<SfxUInt16Item*>(pItem->Clone()));
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

                if(eState >= SfxItemState::DEFAULT)
                {
                    const XFillFloatTransparenceItem* pItem = dynamic_cast< const XFillFloatTransparenceItem* >(pState);

                    if(pItem && (!mpFloatTransparenceItem || *pItem != *mpFloatTransparenceItem))
                    {
                        mpFloatTransparenceItem.reset(static_cast<XFillFloatTransparenceItem*>(pItem->Clone()));
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
                meLastXFS = static_cast<sal_uInt16>(-1);
                mpStyleItem.reset();
            }

            if(eState >= SfxItemState::DEFAULT)
            {
                const XFillStyleItem* pItem = dynamic_cast< const XFillStyleItem* >(pState);

                if(pItem)
                {
                    mpStyleItem.reset(dynamic_cast< XFillStyleItem* >(pItem->Clone()));
                    mpLbFillType->Enable();
                    mpColorTextFT->Enable();
                    drawing::FillStyle eXFS = (drawing::FillStyle)mpStyleItem->GetValue();
                    meLastXFS = eXFS;
                    mpLbFillType->SelectEntryPos(sal::static_int_cast< sal_Int32 >(eXFS));

                    if(drawing::FillStyle_NONE == eXFS)
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
            meLastXFS = static_cast<sal_uInt16>(-1);
            mpStyleItem.reset();
            break;
        }
        case SID_ATTR_FILL_COLOR:
        {
            if(SfxItemState::DEFAULT == eState)
            {
                mpColorItem.reset(pState ? static_cast<XFillColorItem*>(pState->Clone()) : 0);
            }

            if(mpStyleItem && drawing::FillStyle_SOLID == (drawing::FillStyle)mpStyleItem->GetValue())
            {
                mpLbFillAttr->Hide();
                mpToolBoxColor->Show();

                Update();
            }
            break;
        }
        case SID_ATTR_FILL_GRADIENT:
        {
            if(SfxItemState::DEFAULT == eState)
            {
                mpFillGradientItem.reset(pState ? static_cast<XFillGradientItem*>(pState->Clone()) : 0);
            }

            if(mpStyleItem && drawing::FillStyle_GRADIENT == (drawing::FillStyle)mpStyleItem->GetValue())
            {
                mpLbFillAttr->Show();
                mpToolBoxColor->Hide();

                if(SfxItemState::DEFAULT == eState)
                {
                    mpLbFillAttr->Enable();
                    Update();
                }
                else if(SfxItemState::DISABLED == eState )
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
            if(SfxItemState::DEFAULT == eState)
            {
                mpHatchItem.reset(pState ? static_cast<XFillHatchItem*>(pState->Clone()) : 0);
            }

            if(mpStyleItem && drawing::FillStyle_HATCH == (drawing::FillStyle)mpStyleItem->GetValue())
            {
                mpLbFillAttr->Show();
                mpToolBoxColor->Hide();

                if(SfxItemState::DEFAULT == eState)
                {
                    mpLbFillAttr->Enable();
                    Update();
                }
                else if(SfxItemState::DISABLED == eState )
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
            if(SfxItemState::DEFAULT == eState)
            {
                mpBitmapItem.reset(pState ? static_cast<XFillBitmapItem*>(pState->Clone()) : 0);
            }

            if(mpStyleItem && drawing::FillStyle_BITMAP == (drawing::FillStyle)mpStyleItem->GetValue())
            {
                mpLbFillAttr->Show();
                mpToolBoxColor->Hide();

                if(SfxItemState::DEFAULT == eState)
                {
                    mpLbFillAttr->Enable();
                    Update();
                }
                else if(SfxItemState::DISABLED == eState )
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
        case SID_GRADIENT_LIST:
        {
            if(SfxItemState::DEFAULT == eState)
            {
                if(mpStyleItem && drawing::FillStyle_GRADIENT == (drawing::FillStyle)mpStyleItem->GetValue())
                {
                    if(mpFillGradientItem)
                    {
                        const OUString aString( mpFillGradientItem->GetName() );
                        const SfxObjectShell* pSh = SfxObjectShell::Current();
                        const SvxGradientListItem aItem( *static_cast<const SvxGradientListItem*>(pSh->GetItem(SID_GRADIENT_LIST)));

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
            if(SfxItemState::DEFAULT == eState)
            {
                if(mpStyleItem && drawing::FillStyle_HATCH == (drawing::FillStyle)mpStyleItem->GetValue())
                {
                    if(mpHatchItem)
                    {
                        const OUString aString( mpHatchItem->GetName() );
                        const SfxObjectShell* pSh = SfxObjectShell::Current();
                        const SvxHatchListItem aItem(*static_cast<const SvxHatchListItem*>(pSh->GetItem(SID_HATCH_LIST)));

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
            if(SfxItemState::DEFAULT == eState)
            {
                if(mpStyleItem && drawing::FillStyle_BITMAP == (drawing::FillStyle)mpStyleItem->GetValue())
                {
                    if(mpBitmapItem)
                    {
                        const OUString aString( mpBitmapItem->GetName() );
                        const SfxObjectShell* pSh = SfxObjectShell::Current();
                        const SvxBitmapListItem aItem(*static_cast<const SvxBitmapListItem*>(pSh->GetItem(SID_BITMAP_LIST)));

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






void AreaPropertyPanel::Update()
{
    if(mpStyleItem)
    {
        const drawing::FillStyle eXFS = (drawing::FillStyle)mpStyleItem->GetValue();
        SfxObjectShell* pSh = SfxObjectShell::Current();

        switch( eXFS )
        {
            case drawing::FillStyle_NONE:
            {
                mpLbFillAttr->Show();
                mpToolBoxColor->Hide();
                break;
            }
            case drawing::FillStyle_SOLID:
            {
                if(mpColorItem)
                {
                    mpLbFillAttr->Hide();
                    mpToolBoxColor->Show();
                }
                break;
            }
            case drawing::FillStyle_GRADIENT:
            {
                mpLbFillAttr->Show();
                mpToolBoxColor->Hide();

                if(pSh && pSh->GetItem(SID_GRADIENT_LIST))
                {
                    const SvxGradientListItem aItem(*static_cast<const SvxGradientListItem*>(pSh->GetItem(SID_GRADIENT_LIST)));
                    mpLbFillAttr->Enable();
                    mpLbFillAttr->Clear();
                    mpLbFillAttr->Fill(aItem.GetGradientList());

                    if(mpFillGradientItem)
                    {
                        const OUString aString(mpFillGradientItem->GetName());

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
            case drawing::FillStyle_HATCH:
            {
                mpLbFillAttr->Show();
                mpToolBoxColor->Hide();

                if(pSh && pSh->GetItem(SID_HATCH_LIST))
                {
                    const SvxHatchListItem aItem(*static_cast<const SvxHatchListItem*>(pSh->GetItem(SID_HATCH_LIST)));
                    mpLbFillAttr->Enable();
                    mpLbFillAttr->Clear();
                    mpLbFillAttr->Fill(aItem.GetHatchList());

                    if(mpHatchItem)
                    {
                        const OUString aString(mpHatchItem->GetName());

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
            case drawing::FillStyle_BITMAP:
            {
                mpLbFillAttr->Show();
                mpToolBoxColor->Hide();

                if(pSh && pSh->GetItem(SID_BITMAP_LIST))
                {
                    const SvxBitmapListItem aItem(*static_cast<const SvxBitmapListItem*>(pSh->GetItem(SID_BITMAP_LIST)));
                    mpLbFillAttr->Enable();
                    mpLbFillAttr->Clear();
                    mpLbFillAttr->Fill(aItem.GetBitmapList());

                    if(mpBitmapItem)
                    {
                        const OUString aString(mpBitmapItem->GetName());

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



IMPL_LINK_TYPED( AreaPropertyPanel, ClickTrGrHdl_Impl, ToolBox*, pToolBox, void )
{
    maTrGrPopup.Rearrange(mpFloatTransparenceItem.get());
    OSL_ASSERT( pToolBox->GetItemCommand(pToolBox->GetCurItemId()) == UNO_SIDEBARGRADIENT);
    maTrGrPopup.Show(*pToolBox);
}



IMPL_LINK_NOARG(AreaPropertyPanel, ChangeTrgrTypeHdl_Impl)
{
    sal_Int32 nSelectType = mpLBTransType->GetSelectEntryPos();
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
    GetBindings()->GetDispatcher()->Execute( SID_ATTR_FILL_TRANSPARENCE, SfxCallMode::RECORD, &aLinearItem, 0L );

    if(nSelectType > 1)
    {
        nSelectType -= 2;
    }

    XGradient aTmpGradient;

    switch(nSelectType)
    {
        case css::awt::GradientStyle_LINEAR:
            aTmpGradient = maGradientLinear;
            break;
        case css::awt::GradientStyle_AXIAL:
            aTmpGradient = maGradientAxial;
            break;
        case css::awt::GradientStyle_RADIAL:
            aTmpGradient = maGradientRadial;
            break;
        case css::awt::GradientStyle_ELLIPTICAL:
            aTmpGradient = maGradientElliptical;
            break;
        case css::awt::GradientStyle_SQUARE:
            aTmpGradient = maGradientSquare;
            break;
        case css::awt::GradientStyle_RECT:
            aTmpGradient = maGradientRect;
            break;
        default:
            break;
    }

    SfxItemPool* pPool = 0;
    const XFillFloatTransparenceItem aGradientItem(pPool, aTmpGradient, bGradient);
    GetBindings()->GetDispatcher()->Execute( SID_ATTR_FILL_FLOATTRANSPARENCE, SfxCallMode::RECORD, &aGradientItem, 0L );

    return 0L;
}



IMPL_LINK_NOARG(AreaPropertyPanel, ModifyTransparentHdl_Impl)
{
    const sal_uInt16 nTrans = (sal_uInt16)mpMTRTransparent->GetValue();
    mnLastTransSolid = nTrans;
    const sal_Int32 nSelectType = mpLBTransType->GetSelectEntryPos();

    if(nTrans && !nSelectType)
    {
        mpLBTransType->SelectEntryPos(1);
    }

    const XFillTransparenceItem aLinearItem(nTrans);
    GetBindings()->GetDispatcher()->Execute( SID_ATTR_FILL_TRANSPARENCE, SfxCallMode::RECORD, &aLinearItem, 0L );

    return 0L;
}






XGradient AreaPropertyPanel::GetGradient (const css::awt::GradientStyle eStyle) const
{
    switch (eStyle)
    {
        default:
        case css::awt::GradientStyle_LINEAR:
            return maGradientLinear;
        case css::awt::GradientStyle_AXIAL:
            return maGradientAxial;
        case css::awt::GradientStyle_RADIAL:
            return maGradientRadial;
        case css::awt::GradientStyle_ELLIPTICAL:
            return maGradientElliptical;
        case css::awt::GradientStyle_SQUARE:
            return maGradientSquare;
        case css::awt::GradientStyle_RECT:
            return maGradientRect;
    }
}




void AreaPropertyPanel::SetGradient (const XGradient& rGradient)
{
    switch (rGradient.GetGradientStyle())
    {
        case css::awt::GradientStyle_LINEAR:
            maGradientLinear = rGradient;
            break;
        case css::awt::GradientStyle_AXIAL:
            maGradientAxial = rGradient;
            break;
        case css::awt::GradientStyle_RADIAL:
            maGradientRadial = rGradient;
            break;
        case css::awt::GradientStyle_ELLIPTICAL:
            maGradientElliptical = rGradient;
            break;
        case css::awt::GradientStyle_SQUARE:
            maGradientSquare = rGradient;
            break;
        case css::awt::GradientStyle_RECT:
            maGradientRect = rGradient;
            break;
        default:
            break;
    }
}




sal_Int32 AreaPropertyPanel::GetSelectedTransparencyTypeIndex() const
{
    return mpLBTransType->GetSelectEntryPos();
}

} } // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
