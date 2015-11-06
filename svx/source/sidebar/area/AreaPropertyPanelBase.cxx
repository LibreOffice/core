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
#include <sfx2/sidebar/SidebarController.hxx>
#include <sfx2/sidebar/ControlFactory.hxx>
#include <svx/sidebar/AreaPropertyPanelBase.hxx>
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
#include <svx/tbcontrl.hxx>

#include <boost/bind.hpp>

using namespace css;
using namespace css::uno;

const char UNO_SIDEBARGRADIENT[] = ".uno:sidebargradient";

namespace svx { namespace sidebar {

const sal_Int32 AreaPropertyPanelBase::DEFAULT_CENTERX = 50;
const sal_Int32 AreaPropertyPanelBase::DEFAULT_CENTERY = 50;
const sal_Int32 AreaPropertyPanelBase::DEFAULT_ANGLE = 0;
const sal_Int32 AreaPropertyPanelBase::DEFAULT_STARTVALUE = 0;
const sal_Int32 AreaPropertyPanelBase::DEFAULT_ENDVALUE = 16777215;
const sal_Int32 AreaPropertyPanelBase::DEFAULT_BORDER = 0;

AreaPropertyPanelBase::AreaPropertyPanelBase(
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame)
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
      maImgAxial(SVX_RES(IMG_AXIAL)),
      maImgElli(SVX_RES(IMG_ELLI)),
      maImgQuad(SVX_RES(IMG_QUAD)),
      maImgRadial(SVX_RES(IMG_RADIAL)),
      maImgSquare(SVX_RES(IMG_SQUARE)),
      maImgLinear(SVX_RES(IMG_LINEAR)),
      maTrGrPopup(this, ::boost::bind(&AreaPropertyPanelBase::CreateTransparencyGradientControl, this, _1)),
      mpFloatTransparenceItem(),
      mpTransparanceItem(),
      mxFrame(rxFrame),
      mpSidebarController()
{
    get(mpColorTextFT,    "filllabel");
    get(mpLbFillType,     "fillstyle");
    get(mpLbFillAttr,     "fillattr");
    get(mpTrspTextFT,     "transparencylabel");
    get(mpToolBoxColor,   "selectcolor");
    get(mpLBTransType,    "transtype");
    get(mpMTRTransparent, "settransparency");
    get(mpSldTransparent, "transparencyslider");
    get(mpBTNGradient,    "selectgradient");
    get(mpMTRAngle, "gradangle");
    get(mpLbFillGradFrom, "fillgrad1");
    get(mpLbFillGradTo, "fillgrad2");
    get(mpGradientStyle, "gradientstyle");

    Initialize();
}

AreaPropertyPanelBase::~AreaPropertyPanelBase()
{
    disposeOnce();
}

void AreaPropertyPanelBase::dispose()
{
    mpColorTextFT.clear();
    mpLbFillType.clear();
    mpLbFillAttr.clear();
    mpToolBoxColor.clear();
    mpTrspTextFT.clear();
    mpLBTransType.clear();
    mpMTRTransparent.clear();
    mpSldTransparent.clear();
    mpBTNGradient.clear();
    mpMTRAngle.clear();
    mpLbFillGradFrom.clear();
    mpLbFillGradTo.clear();
    mpGradientStyle.clear();

    PanelLayout::dispose();
}

void AreaPropertyPanelBase::Initialize()
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

    mpLbFillType->SetAccessibleName("Fill");    //wj acc
    mpLbFillAttr->SetAccessibleName("Fill");    //wj acc

    mpLbFillType->SetSelectHdl( LINK( this, AreaPropertyPanelBase, SelectFillTypeHdl ) );

    Link<ListBox&,void> aLink = LINK( this, AreaPropertyPanelBase, SelectFillAttrHdl );
    mpLbFillAttr->SetSelectHdl( aLink );
    mpGradientStyle->SetSelectHdl( aLink );
    mpLbFillGradFrom->SetSelectHdl( aLink );
    mpLbFillGradTo->SetSelectHdl( aLink );

    mpLBTransType->SetSelectHdl(LINK(this, AreaPropertyPanelBase, ChangeTrgrTypeHdl_Impl));
    mpLBTransType->SetAccessibleName("Transparency");    //wj acc

    SetTransparency( 50 );
    mpMTRTransparent->SetModifyHdl(LINK(this, AreaPropertyPanelBase, ModifyTransparentHdl_Impl));
    mpSldTransparent->SetSlideHdl(LINK(this, AreaPropertyPanelBase, ModifyTransSliderHdl));
    mpMTRTransparent->SetAccessibleName("Transparency");    //wj acc

    const sal_uInt16 nIdGradient = mpBTNGradient->GetItemId(UNO_SIDEBARGRADIENT);
    mpBTNGradient->SetItemBits( nIdGradient, mpBTNGradient->GetItemBits( nIdGradient ) | ToolBoxItemBits::DROPDOWNONLY );
    Link<ToolBox *, void> aLink2 = LINK( this, AreaPropertyPanelBase, ClickTrGrHdl_Impl );
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

    mpSidebarController = sfx2::sidebar::SidebarController::GetSidebarControllerForFrame(mxFrame);
}

void AreaPropertyPanelBase::SetTransparency(sal_uInt16 nVal)
{
    mpSldTransparent->SetThumbPos(nVal);
    mpMTRTransparent->SetValue(nVal);
}

IMPL_LINK_NOARG_TYPED(AreaPropertyPanelBase, SelectFillTypeHdl, ListBox&, void)
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
                mpLbFillGradFrom->Hide();
                mpLbFillGradTo->Hide();
                mpGradientStyle->Hide();
                mpMTRAngle->Hide();
                mpToolBoxColor->Hide();
                mpLbFillType->Selected();
                mpLbFillAttr->Disable();

                // #i122676# need to call a single SID_ATTR_FILL_STYLE change
                setFillStyle(aXFillStyleItem);
                break;
            }
            case drawing::FillStyle_SOLID:
            {
                mpLbFillAttr->Hide();
                mpLbFillGradFrom->Hide();
                mpLbFillGradTo->Hide();
                mpGradientStyle->Hide();
                mpMTRAngle->Hide();
                mpToolBoxColor->Show();
                const OUString aTmpStr;
                const Color aColor = mpColorItem ? mpColorItem->GetColorValue() : COL_AUTO;
                const XFillColorItem aXFillColorItem( aTmpStr, aColor );

                // #i122676# change FillStyle and Color in one call
                setFillStyleAndColor(&aXFillStyleItem, aXFillColorItem);
                break;
            }
            case drawing::FillStyle_GRADIENT:
            {
                mpLbFillAttr->Hide();
                mpLbFillGradFrom->Show();
                mpLbFillGradTo->Show();
                mpGradientStyle->Show();
                mpMTRAngle->Show();
                mpToolBoxColor->Hide();

                const SvxColorListItem* pColorListItem = static_cast<const SvxColorListItem*>(pSh ? pSh->GetItem(SID_COLOR_TABLE) : nullptr);
                if (pColorListItem)
                {
                    mpLbFillAttr->Enable();
                    mpLbFillGradTo->Enable();
                    mpLbFillGradFrom->Enable();
                    mpGradientStyle->Enable();
                    mpMTRAngle->Enable();
                    mpLbFillAttr->Clear();
                    mpLbFillGradTo->Clear();
                    mpLbFillGradFrom->Clear();
                    mpLbFillGradTo->Fill(pColorListItem->GetColorList());
                    mpLbFillGradFrom->Fill(pColorListItem->GetColorList());

                    mpLbFillGradFrom->AdaptDropDownLineCountToMaximum();
                    mpLbFillGradTo->AdaptDropDownLineCountToMaximum();

                    if(LISTBOX_ENTRY_NOTFOUND != mnLastPosGradient)
                    {
                        const SvxGradientListItem aItem(*static_cast<const SvxGradientListItem*>(pSh->GetItem(SID_GRADIENT_LIST)));

                        if(mnLastPosGradient < aItem.GetGradientList()->Count())
                        {
                            const XGradient aGradient = aItem.GetGradientList()->GetGradient(mnLastPosGradient)->GetGradient();
                            const XFillGradientItem aXFillGradientItem(aGradient);

                            // #i122676# change FillStyle and Gradient in one call
                            setFillStyleAndGradient(&aXFillStyleItem, aXFillGradientItem);
                            mpLbFillGradFrom->SelectEntry(aGradient.GetStartColor());
                            if(mpLbFillGradFrom->GetSelectEntryCount() == 0)
                            {
                                mpLbFillGradFrom->InsertEntry(aGradient.GetStartColor(), OUString());
                                mpLbFillGradFrom->SelectEntry(aGradient.GetStartColor());
                            }
                            mpLbFillGradTo->SelectEntry(aGradient.GetEndColor());
                            if(mpLbFillGradTo->GetSelectEntryCount() == 0)
                            {
                                mpLbFillGradTo->InsertEntry(aGradient.GetEndColor(), OUString());
                                mpLbFillGradTo->SelectEntry(aGradient.GetEndColor());
                            }

                            mpMTRAngle->SetValue(aGradient.GetAngle() / 10);
                            css::awt::GradientStyle eXGS = aGradient.GetGradientStyle();
                            mpGradientStyle->SelectEntryPos(sal::static_int_cast< sal_Int32 >( eXGS ));

                        }
                    }
                }
                else
                {
                    mpLbFillGradFrom->Disable();
                    mpLbFillGradTo->Disable();
                    mpMTRAngle->Disable();
                    mpGradientStyle->Disable();
                }
                break;
            }
            case drawing::FillStyle_HATCH:
            {
                mpLbFillAttr->Show();
                mpLbFillGradFrom->Hide();
                mpLbFillGradTo->Hide();
                mpMTRAngle->Hide();
                mpGradientStyle->Hide();
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
                            setFillStyleAndHatch(&aXFillStyleItem, aXFillHatchItem);
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
                mpLbFillGradFrom->Hide();
                mpLbFillGradTo->Hide();
                mpMTRAngle->Hide();
                mpGradientStyle->Hide();
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
                            setFillStyleAndBitmap(&aXFillStyleItem, aXFillBitmapItem);
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
            mpLbFillType->Selected();
        }
    }

    mpSidebarController->NotifyResize();
}

IMPL_LINK_NOARG_TYPED(AreaPropertyPanelBase, SelectFillAttrHdl, ListBox&, void)
{
    const drawing::FillStyle eXFS = (drawing::FillStyle)mpLbFillType->GetSelectEntryPos();
    const XFillStyleItem aXFillStyleItem(eXFS);
    SfxObjectShell* pSh = SfxObjectShell::Current();

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
                setFillStyle(aXFillStyleItem);
            }
            break;
        }
        case drawing::FillStyle_GRADIENT:
        {

            if(pSh && pSh->GetItem(SID_COLOR_TABLE))
            {
                XGradient aGradient;
                aGradient.SetAngle(mpMTRAngle->GetValue() * 10);
                aGradient.SetGradientStyle((css::awt::GradientStyle)mpGradientStyle->GetSelectEntryPos());
                aGradient.SetStartColor(mpLbFillGradFrom->GetSelectEntryColor());
                aGradient.SetEndColor(mpLbFillGradTo->GetSelectEntryColor());

                const XFillGradientItem aXFillGradientItem(mpLbFillAttr->GetSelectEntry(), aGradient);

                    // #i122676# Change FillStyle and Gradinet in one call
                setFillStyleAndGradient(bFillStyleChange ? &aXFillStyleItem : NULL, aXFillGradientItem);
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

                    // #i122676# Change FillStyle and Hatch in one call
                    setFillStyleAndHatch(bFillStyleChange ? &aXFillStyleItem : NULL, aXFillHatchItem);
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

                    // #i122676# Change FillStyle and Bitmap in one call
                    setFillStyleAndBitmap(bFillStyleChange ? &aXFillStyleItem : NULL, aXFillBitmapItem);
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
    mpSidebarController->NotifyResize();
}

VclPtr<PopupControl> AreaPropertyPanelBase::CreateTransparencyGradientControl (PopupContainer* pParent)
{
    return VclPtrInstance<AreaTransparencyGradientControl>(pParent, *this);
}

void AreaPropertyPanelBase::DataChanged(
    const DataChangedEvent& /*rEvent*/)
{
}

void AreaPropertyPanelBase::ImpUpdateTransparencies()
{
    if(mpTransparanceItem.get() || mpFloatTransparenceItem.get())
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
                mpSldTransparent->Show();
                mpMTRTransparent->Enable();
                mpSldTransparent->Enable();
                SetTransparency(nValue);
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
                mpSldTransparent->Hide();
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
            mpSldTransparent->Enable();
            mpMTRTransparent->Show();
            mpSldTransparent->Show();
            SetTransparency(0);
        }
    }
    else
    {
        // no transparency at all
        mpLBTransType->SetNoSelection();
        mpLBTransType->Disable();
        mpTrspTextFT->Disable();
        mpMTRTransparent->Disable();
        mpSldTransparent->Disable();
        mpMTRTransparent->Show();
        mpSldTransparent->Show();
        mpBTNGradient->Disable();
        mpBTNGradient->Hide();
    }
}

void AreaPropertyPanelBase::updateFillTransparence(bool bDisabled, bool bDefault, const SfxUInt16Item* pItem)
{
    if (bDisabled)
    {
        mpTransparanceItem.reset();
        return;
    }
    else if(bDefault)
    {
        if(pItem)
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

    // update transparency settings dependent of mpTransparanceItem and mpFloatTransparenceItem
    ImpUpdateTransparencies();
}

void AreaPropertyPanelBase::updateFillFloatTransparence(bool bDisabled, bool bDefault, const XFillFloatTransparenceItem* pItem)
{
    if (bDisabled)
    {
        mpFloatTransparenceItem.reset();
        return;
    }

    if(bDefault)
    {
        if(pItem)
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

    // update transparency settings dependent of mpTransparanceItem and mpFloatTransparenceItem
    ImpUpdateTransparencies();
}

void AreaPropertyPanelBase::updateFillStyle(bool bDisabled, bool bDefault, const XFillStyleItem* pItem)
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
    else if(bDefault && pItem)
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
        return;
    }

    mpLbFillType->SetNoSelection();
    mpLbFillAttr->Show();
    mpLbFillAttr->Disable();
    mpLbFillAttr->SetNoSelection();
    mpToolBoxColor->Hide();
    meLastXFS = static_cast<sal_uInt16>(-1);
    mpStyleItem.reset();
}

void AreaPropertyPanelBase::updateFillGradient(bool bDisabled, bool bDefault, const XFillGradientItem* pItem)
{
    if(bDefault)
    {
        mpFillGradientItem.reset(pItem ? static_cast<XFillGradientItem*>(pItem->Clone()) : 0);
    }

    if(mpStyleItem && drawing::FillStyle_GRADIENT == (drawing::FillStyle)mpStyleItem->GetValue())
    {
        mpLbFillAttr->Hide();
        mpLbFillGradFrom->Show();
        mpLbFillGradTo->Show();
        mpMTRAngle->Show();
        mpGradientStyle->Show();
        mpToolBoxColor->Hide();

        if(bDefault)
        {
            Update();
        }
        else if(bDisabled)
        {
            mpLbFillGradFrom->SetNoSelection();
            mpLbFillGradTo->SetNoSelection();
            mpLbFillGradFrom->Disable();
            mpLbFillGradTo->Disable();
            mpMTRAngle->Disable();
            mpGradientStyle->Disable();
        }
        else
        {
            mpLbFillGradFrom->SetNoSelection();
            mpLbFillGradTo->SetNoSelection();
        }
    }
}

void AreaPropertyPanelBase::updateFillHatch(bool bDisabled, bool bDefault, const XFillHatchItem* pItem)
{
    if(bDefault)
    {
        mpHatchItem.reset(pItem ? static_cast<XFillHatchItem*>(pItem->Clone()) : 0);
    }

    if(mpStyleItem && drawing::FillStyle_HATCH == (drawing::FillStyle)mpStyleItem->GetValue())
    {
        mpLbFillAttr->Show();
        mpToolBoxColor->Hide();

        if(bDefault)
        {
            mpLbFillAttr->Enable();
            Update();
        }
        else if(bDisabled)
        {
            mpLbFillAttr->Disable();
            mpLbFillAttr->SetNoSelection();
        }
        else
        {
            mpLbFillAttr->SetNoSelection();
        }
    }
}

void AreaPropertyPanelBase::updateFillColor(bool bDefault, const XFillColorItem* pItem)
{
    if(bDefault)
    {
        mpColorItem.reset(pItem ? static_cast<XFillColorItem*>(pItem->Clone()) : 0);
    }

    if(mpStyleItem && drawing::FillStyle_SOLID == (drawing::FillStyle)mpStyleItem->GetValue())
    {
        mpLbFillAttr->Hide();
        mpToolBoxColor->Show();

        Update();
    }
}

void AreaPropertyPanelBase::updateFillBitmap(bool bDisabled, bool bDefault, const XFillBitmapItem* pItem)
{
    if(bDefault)
    {
        mpBitmapItem.reset(pItem ? static_cast<XFillBitmapItem*>(pItem->Clone()) : 0);
    }

    if(mpStyleItem && drawing::FillStyle_BITMAP == (drawing::FillStyle)mpStyleItem->GetValue())
    {
        mpLbFillAttr->Show();
        mpToolBoxColor->Hide();

        if(bDefault)
        {
            mpLbFillAttr->Enable();
            Update();
        }
        else if(bDisabled)
        {
            mpLbFillAttr->Disable();
            mpLbFillAttr->SetNoSelection();
        }
        else
        {
            mpLbFillAttr->SetNoSelection();
        }
    }
}

void AreaPropertyPanelBase::NotifyItemUpdate(
    sal_uInt16 nSID,
    SfxItemState eState,
    const SfxPoolItem* pState,
    const bool /*bIsEnabled*/)
{
    const bool bDisabled(SfxItemState::DISABLED == eState);
    const bool bDefaultOrSet(SfxItemState::DEFAULT <= eState);
    const bool bDefault(SfxItemState::DEFAULT == eState);

    switch(nSID)
    {
        case SID_ATTR_FILL_TRANSPARENCE:
            updateFillTransparence(bDisabled, bDefaultOrSet,
                    static_cast<const SfxUInt16Item*>(pState));
        break;
        case SID_ATTR_FILL_FLOATTRANSPARENCE:
            updateFillFloatTransparence(bDisabled, bDefaultOrSet,
                    static_cast<const XFillFloatTransparenceItem*>(pState));
        break;
        case SID_ATTR_FILL_STYLE:
            updateFillStyle(bDisabled, bDefaultOrSet,
                    static_cast<const XFillStyleItem*>(pState));
        break;
        case SID_ATTR_FILL_COLOR:
            updateFillColor(bDefaultOrSet,
                    static_cast<const XFillColorItem*>(pState));
        break;
        case SID_ATTR_FILL_GRADIENT:
            updateFillGradient(bDisabled, bDefaultOrSet,
                    static_cast<const XFillGradientItem*>(pState));
        break;
        case SID_ATTR_FILL_HATCH:
            updateFillHatch(bDisabled, bDefaultOrSet,
                    static_cast<const XFillHatchItem*>(pState));
        break;
        case SID_ATTR_FILL_BITMAP:
            updateFillBitmap(bDisabled, bDefaultOrSet,
                    static_cast<const XFillBitmapItem*>(pState));
        break;
        case SID_GRADIENT_LIST:
        {
            if(bDefault)
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
            if(bDefault)
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
            if(bDefault)
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
    mpSidebarController->NotifyResize();
}

void AreaPropertyPanelBase::Update()
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
                mpLbFillGradFrom->Hide();
                mpLbFillGradTo->Hide();
                mpMTRAngle->Hide();
                mpGradientStyle->Hide();
                mpToolBoxColor->Hide();
                break;
            }
            case drawing::FillStyle_SOLID:
            {
                if(mpColorItem)
                {
                    mpLbFillAttr->Hide();
                    mpLbFillGradFrom->Hide();
                    mpLbFillGradTo->Hide();
                    mpMTRAngle->Hide();
                    mpGradientStyle->Hide();
                    mpToolBoxColor->Show();
                }
                break;
            }
            case drawing::FillStyle_GRADIENT:
            {
                mpLbFillAttr->Hide();
                mpLbFillGradFrom->Show();
                mpLbFillGradTo->Show();
                mpMTRAngle->Enable();
                mpMTRAngle->Show();
                mpGradientStyle->Show();
                mpToolBoxColor->Hide();

                if(pSh && pSh->GetItem(SID_GRADIENT_LIST))
                {
                    const SvxGradientListItem aItem(*static_cast<const SvxGradientListItem*>(pSh->GetItem(SID_GRADIENT_LIST)));
                    mpLbFillAttr->Enable();
                    mpLbFillAttr->Clear();
                    mpLbFillAttr->Fill(aItem.GetGradientList());
                    const SvxColorListItem aColorItem(*static_cast<const SvxColorListItem*>(pSh->GetItem(SID_COLOR_TABLE)));
                    mpLbFillGradFrom->Fill(aColorItem.GetColorList());
                    mpLbFillGradTo->Fill(aColorItem.GetColorList());
                    mpLbFillGradTo->SetNoSelection();
                    mpLbFillGradFrom->SetNoSelection();
                    if(mpFillGradientItem)
                    {
                        const OUString aString(mpFillGradientItem->GetName());
                        mpLbFillAttr->SelectEntry(aString);
                        const XGradient pGradient = mpFillGradientItem->GetGradientValue();
                        mpLbFillGradFrom->SelectEntry(pGradient.GetStartColor());
                        if(mpLbFillGradFrom->GetSelectEntryCount() == 0)
                        {
                            mpLbFillGradFrom->InsertEntry(pGradient.GetStartColor(), OUString());
                            mpLbFillGradFrom->SelectEntry(pGradient.GetStartColor());
                        }
                        mpLbFillGradTo->SelectEntry(pGradient.GetEndColor());
                        if(mpLbFillGradTo->GetSelectEntryCount() == 0)
                        {
                            mpLbFillGradTo->InsertEntry(pGradient.GetEndColor(), OUString());
                            mpLbFillGradTo->SelectEntry(pGradient.GetEndColor());
                        }
                        mpGradientStyle->SelectEntryPos(sal::static_int_cast< sal_Int32 >( pGradient.GetGradientStyle() ));
                        if(mpGradientStyle->GetSelectEntryPos() == GradientStyle_RADIAL)
                            mpMTRAngle->Disable();
                        else
                            mpMTRAngle->SetValue( pGradient.GetAngle() /10 );
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
                mpLbFillGradFrom->Hide();
                mpLbFillGradTo->Hide();
                mpMTRAngle->Hide();
                mpGradientStyle->Hide();
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
                mpLbFillGradFrom->Hide();
                mpLbFillGradTo->Hide();
                mpMTRAngle->Hide();
                mpGradientStyle->Hide();

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

IMPL_LINK_NOARG_TYPED(AreaPropertyPanelBase, ModifyTransSliderHdl, Slider*, void)
{
    const sal_uInt16 nVal = mpSldTransparent->GetThumbPos();
    SetTransparency(nVal);
    const XFillTransparenceItem aLinearItem(nVal);
    setFillTransparence(aLinearItem);
}

IMPL_LINK_TYPED( AreaPropertyPanelBase, ClickTrGrHdl_Impl, ToolBox*, pToolBox, void )
{
    maTrGrPopup.Rearrange(mpFloatTransparenceItem.get());
    OSL_ASSERT( pToolBox->GetItemCommand(pToolBox->GetCurItemId()) == UNO_SIDEBARGRADIENT);
    maTrGrPopup.Show(*pToolBox);
}

IMPL_LINK_NOARG_TYPED(AreaPropertyPanelBase, ChangeTrgrTypeHdl_Impl, ListBox&, void)
{
    sal_Int32 nSelectType = mpLBTransType->GetSelectEntryPos();
    bool bGradient = false;
    sal_uInt16 nTrans = 0;

    if(!nSelectType)
    {
        mpBTNGradient->Hide();
        mpMTRTransparent->Show();
        mpSldTransparent->Show();
        mpMTRTransparent->Enable();
        mpSldTransparent->Enable();
        SetTransparency(0);
    }
    else if(1 == nSelectType)
    {
        mpBTNGradient->Hide();
        mpMTRTransparent->Show();
        mpSldTransparent->Show();
        nTrans = mnLastTransSolid;
        mpMTRTransparent->SetValue(nTrans);
        mpLBTransType->SelectEntryPos(1);
        mpMTRTransparent->Enable();
        mpSldTransparent->Enable();
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
        mpSldTransparent->Hide();
        mpBTNGradient->Enable();
        bGradient = true;
    }

    const XFillTransparenceItem aLinearItem(nTrans);
    setFillTransparence(aLinearItem);

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
    setFillFloatTransparence(aGradientItem);
}

IMPL_LINK_NOARG_TYPED(AreaPropertyPanelBase, ModifyTransparentHdl_Impl, Edit&, void)
{
    const sal_uInt16 nTrans = (sal_uInt16)mpMTRTransparent->GetValue();
    mnLastTransSolid = nTrans;
    const sal_Int32 nSelectType = mpLBTransType->GetSelectEntryPos();

    if(nTrans && !nSelectType)
    {
        mpLBTransType->SelectEntryPos(1);
    }

    const XFillTransparenceItem aLinearItem(nTrans);
    setFillTransparence(aLinearItem);
}

XGradient AreaPropertyPanelBase::GetGradient (const css::awt::GradientStyle eStyle) const
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

void AreaPropertyPanelBase::SetGradient (const XGradient& rGradient)
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

sal_Int32 AreaPropertyPanelBase::GetSelectedTransparencyTypeIndex() const
{
    return mpLBTransType->GetSelectEntryPos();
}

} } // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
