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

#include <sfx2/sidebar/SidebarController.hxx>
#include <sfx2/sidebar/ControlFactory.hxx>
#include <svx/sidebar/AreaPropertyPanelBase.hxx>
#include <svx/svxids.hrc>
#include <sfx2/objsh.hxx>
#include <svx/xfltrit.hxx>
#include <svx/xflftrit.hxx>
#include <svx/xtable.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <svtools/valueset.hxx>
#include <unotools/pathoptions.hxx>
#include <svx/svxitems.hrc>
#include <vcl/toolbox.hxx>
#include <vcl/salbtype.hxx>
#include <svtools/toolbarmenu.hxx>
#include <svx/tbcontrl.hxx>
#include <sfx2/opengrf.hxx>
#include <bitmaps.hlst>

using namespace css;
using namespace css::uno;

const char UNO_SIDEBARGRADIENT[] = ".uno:sidebargradient";

namespace svx { namespace sidebar {

enum eFillStyle
{
    NONE,
    SOLID,
    GRADIENT,
    HATCH,
    BITMAP,
    PATTERN
};

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
      mnLastPosHatch(0),
      mnLastPosBitmap(0),
      mnLastPosPattern(0),
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
      maImgAxial(StockImage::Yes, BMP_AXIAL),
      maImgElli(StockImage::Yes, BMP_ELLI),
      maImgQuad(StockImage::Yes, BMP_QUAD),
      maImgRadial(StockImage::Yes, BMP_RADIAL),
      maImgSquare(StockImage::Yes, BMP_SQUARE),
      maImgLinear(StockImage::Yes, BMP_LINEAR),
      mpFloatTransparenceItem(),
      mpTransparanceItem()
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
    get(mpBmpImport, "bmpimport");

    Initialize();
}

AreaPropertyPanelBase::~AreaPropertyPanelBase()
{
    disposeOnce();
}

void AreaPropertyPanelBase::dispose()
{
    mxTrGrPopup.disposeAndClear();
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
    mpBmpImport.clear();

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


    mpLbFillType->SetSelectHdl( LINK( this, AreaPropertyPanelBase, SelectFillTypeHdl ) );

    Link<ListBox&,void> aLink = LINK( this, AreaPropertyPanelBase, SelectFillAttrHdl );
    Link<SvxColorListBox&,void> aLink3 = LINK( this, AreaPropertyPanelBase, SelectFillColorHdl );
    mpLbFillAttr->SetSelectHdl( aLink );
    mpGradientStyle->SetSelectHdl( aLink );
    mpLbFillGradFrom->SetSelectHdl( aLink3 );
    mpLbFillGradTo->SetSelectHdl( aLink3 );
    mpMTRAngle->SetModifyHdl(LINK(this,AreaPropertyPanelBase, ChangeGradientAngle));

    mpLBTransType->SetSelectHdl(LINK(this, AreaPropertyPanelBase, ChangeTrgrTypeHdl_Impl));

    SetTransparency( 50 );
    mpMTRTransparent->SetModifyHdl(LINK(this, AreaPropertyPanelBase, ModifyTransparentHdl_Impl));
    mpSldTransparent->SetSlideHdl(LINK(this, AreaPropertyPanelBase, ModifyTransSliderHdl));

    const sal_uInt16 nIdGradient = mpBTNGradient->GetItemId(UNO_SIDEBARGRADIENT);
    mpBTNGradient->SetItemBits( nIdGradient, mpBTNGradient->GetItemBits( nIdGradient ) | ToolBoxItemBits::DROPDOWNONLY );
    Link<ToolBox *, void> aLink2 = LINK( this, AreaPropertyPanelBase, ClickTrGrHdl_Impl );
    mpBTNGradient->SetDropdownClickHdl( aLink2 );
    mpBTNGradient->SetSelectHdl( aLink2 );
    mpBTNGradient->SetItemImage(nIdGradient,maImgLinear);
    mpBTNGradient->Hide();
    mpBmpImport->SetClickHdl( LINK(this, AreaPropertyPanelBase, ClickImportBitmapHdl));
}

void AreaPropertyPanelBase::SetTransparency(sal_uInt16 nVal)
{
    mpSldTransparent->SetThumbPos(nVal);
    mpMTRTransparent->SetValue(nVal);
}

IMPL_LINK_NOARG(AreaPropertyPanelBase, ClickImportBitmapHdl, Button*, void)
{
    SvxOpenGraphicDialog aDlg("Import", GetFrameWeld());
    aDlg.EnableLink(false);
    if( aDlg.Execute() == ERRCODE_NONE )
    {
        Graphic aGraphic;
        EnterWait();
        ErrCode nError = aDlg.GetGraphic( aGraphic );
        LeaveWait();
        if( nError == ERRCODE_NONE )
        {
            XBitmapListRef pList = SfxObjectShell::Current()->GetItem(SID_BITMAP_LIST)->GetBitmapList();
            INetURLObject   aURL( aDlg.GetPath() );
            OUString aFileName =  aURL.GetName().getToken( 0, '.' );
            OUString aName = aFileName;
            long j = 1;
            bool bValidBitmapName = false;
            while( !bValidBitmapName )
            {
                bValidBitmapName = true;
                for( long i = 0; i < pList->Count() && bValidBitmapName; i++ )
                {
                    if( aName == pList->GetBitmap(i)->GetName() )
                    {
                        bValidBitmapName = false;
                        aName = aFileName + OUString::number(j++);
                    }
                }
            }

            pList->Insert(std::make_unique<XBitmapEntry>(aGraphic, aName));
            pList->Save();
            mpLbFillAttr->Clear();
            mpLbFillAttr->Fill(pList);
            mpLbFillAttr->SelectEntry(aName);
            SelectFillAttrHdl(*mpLbFillAttr);
        }
    }
}

IMPL_LINK_NOARG(AreaPropertyPanelBase, SelectFillTypeHdl, ListBox&, void)
{
    sal_Int32 nPos = static_cast<eFillStyle>(mpLbFillType->GetSelectedEntryPos());
    mpLbFillAttr->Clear();
    SfxObjectShell* pSh = SfxObjectShell::Current();
    if(!pSh)
        return;

    // #i122676# Do no longer trigger two Execute calls, one for SID_ATTR_FILL_STYLE
    // and one for setting the fill attribute itself, but add two SfxPoolItems to the
    // call to get just one action at the SdrObject and to create only one Undo action, too.
    // Checked that this works in all apps.
    switch( nPos )
    {
        default:
        case NONE:
        {
            mpLbFillAttr->Show();
            mpLbFillGradFrom->Hide();
            mpLbFillGradTo->Hide();
            mpGradientStyle->Hide();
            mpMTRAngle->Hide();
            mpToolBoxColor->Hide();
            mpBmpImport->Hide();
            mpLbFillType->Selected();
            mpLbFillAttr->Disable();

            // #i122676# need to call a single SID_ATTR_FILL_STYLE change
            setFillStyle(XFillStyleItem(drawing::FillStyle_NONE));
            break;
        }
        case SOLID:
        {
            mpLbFillAttr->Hide();
            mpLbFillGradFrom->Hide();
            mpLbFillGradTo->Hide();
            mpGradientStyle->Hide();
            mpMTRAngle->Hide();
            mpBmpImport->Hide();
            mpToolBoxColor->Show();
            const OUString aTmpStr;
            const Color aColor = mpColorItem ? mpColorItem->GetColorValue() : COL_AUTO;
            const XFillColorItem aXFillColorItem( aTmpStr, aColor );

            // #i122676# change FillStyle and Color in one call
            XFillStyleItem aXFillStyleItem(drawing::FillStyle_SOLID);
            setFillStyleAndColor(&aXFillStyleItem, aXFillColorItem);
            break;
        }
        case GRADIENT:
        {
            mpLbFillAttr->Hide();
            mpLbFillGradFrom->Show();
            mpLbFillGradTo->Show();
            mpGradientStyle->Show();
            mpMTRAngle->Show();
            mpToolBoxColor->Hide();
            mpBmpImport->Hide();

            mpLbFillAttr->Enable();
            mpLbFillGradTo->Enable();
            mpLbFillGradFrom->Enable();
            mpGradientStyle->Enable();
            mpMTRAngle->Enable();
            mpLbFillAttr->Clear();

            const SvxGradientListItem * pItem = pSh->GetItem(SID_GRADIENT_LIST);

            if(0 < pItem->GetGradientList()->Count())
            {
                const XGradient aGradient = pItem->GetGradientList()->GetGradient(0)->GetGradient();
                const OUString aName = pItem->GetGradientList()->GetGradient(0)->GetName();
                const XFillGradientItem aXFillGradientItem(aName, aGradient);

                // #i122676# change FillStyle and Gradient in one call
                XFillStyleItem aXFillStyleItem(drawing::FillStyle_GRADIENT);
                setFillStyleAndGradient(&aXFillStyleItem, aXFillGradientItem);
                mpLbFillGradFrom->SelectEntry(aGradient.GetStartColor());
                mpLbFillGradTo->SelectEntry(aGradient.GetEndColor());

                mpMTRAngle->SetValue(aGradient.GetAngle() / 10);
                css::awt::GradientStyle eXGS = aGradient.GetGradientStyle();
                mpGradientStyle->SelectEntryPos(sal::static_int_cast< sal_Int32 >( eXGS ));
            }
            break;
        }
        case HATCH:
        {
            mpLbFillAttr->Show();
            mpLbFillGradFrom->Hide();
            mpLbFillGradTo->Hide();
            mpMTRAngle->Hide();
            mpGradientStyle->Hide();
            mpToolBoxColor->Hide();
            mpBmpImport->Hide();

            const SvxHatchListItem* pItem( pSh->GetItem(SID_HATCH_LIST) );
            if(pItem)
            {
                const XHatchListRef& pXHatchList(pItem->GetHatchList());
                mpLbFillAttr->Enable();
                mpLbFillAttr->Clear();
                mpLbFillAttr->Fill(pXHatchList);

                mpLbFillAttr->AdaptDropDownLineCountToMaximum();

                if(LISTBOX_ENTRY_NOTFOUND != mnLastPosHatch)
                {
                    if(mnLastPosHatch < pXHatchList->Count())
                    {
                        const XHatch aHatch = pXHatchList->GetHatch(mnLastPosHatch)->GetHatch();
                        const OUString aName = pXHatchList->GetHatch(mnLastPosHatch)->GetName();
                        const XFillHatchItem aXFillHatchItem(aName, aHatch);

                        // #i122676# change FillStyle and Hatch in one call
                        XFillStyleItem aXFillStyleItem(drawing::FillStyle_HATCH);
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
        case BITMAP:
        case PATTERN:
        {
            mpLbFillAttr->Show();
            mpLbFillAttr->Enable();
            mpLbFillAttr->Clear();
            mpLbFillGradFrom->Hide();
            mpLbFillGradTo->Hide();
            mpMTRAngle->Hide();
            mpGradientStyle->Hide();
            mpToolBoxColor->Hide();

            OUString aName;
            GraphicObject aBitmap;
            if(nPos == static_cast< sal_Int32 >(BITMAP))
            {
                mpBmpImport->Show();
                const SvxBitmapListItem* pItem = pSh->GetItem(SID_BITMAP_LIST);
                if(pItem)
                {
                    const XBitmapListRef& pXBitmapList(pItem->GetBitmapList());
                    mpLbFillAttr->Fill(pXBitmapList);

                    mpLbFillAttr->AdaptDropDownLineCountToMaximum();

                    if(LISTBOX_ENTRY_NOTFOUND != mnLastPosBitmap)
                    {
                        if(mnLastPosBitmap < pXBitmapList->Count())
                        {
                            const XBitmapEntry* pXBitmapEntry = pXBitmapList->GetBitmap(mnLastPosBitmap);
                            aBitmap = pXBitmapEntry->GetGraphicObject();
                            aName = pXBitmapEntry->GetName();
                            mpLbFillAttr->SelectEntryPos(mnLastPosBitmap);
                        }
                    }
                }
                else
                {
                    mpLbFillAttr->Hide();
                }
            }
            else if(nPos == static_cast< sal_Int32 >(PATTERN))
            {
                mpBmpImport->Hide();
                const SvxPatternListItem* pItem = pSh->GetItem(SID_PATTERN_LIST);
                if(pItem)
                {
                    const XPatternListRef& pXPatternList(pItem->GetPatternList());
                    mpLbFillAttr->Fill(pXPatternList);

                    mpLbFillAttr->AdaptDropDownLineCountToMaximum();
                    if(LISTBOX_ENTRY_NOTFOUND != mnLastPosPattern)
                    {
                        if(mnLastPosPattern < pXPatternList->Count())
                        {
                            const XBitmapEntry* pXPatternEntry = pXPatternList->GetBitmap(mnLastPosPattern);
                            aBitmap = pXPatternEntry->GetGraphicObject();
                            aName = pXPatternEntry->GetName();
                            mpLbFillAttr->SelectEntryPos(mnLastPosPattern);
                        }
                    }
                }
                else
                {
                    mpLbFillAttr->Hide();
                }
            }
            const XFillBitmapItem aXFillBitmapItem( aName, aBitmap );
            const XFillStyleItem aXFillStyleItem(drawing::FillStyle_BITMAP);
            setFillStyleAndBitmap(&aXFillStyleItem, aXFillBitmapItem);
            break;
        }
    }

    meLastXFS = static_cast<sal_uInt16>(nPos);

    if(eFillStyle::NONE != static_cast<eFillStyle>(nPos))
    {
        mpLbFillType->Selected();
    }
}

IMPL_LINK_NOARG(AreaPropertyPanelBase, SelectFillColorHdl, SvxColorListBox&, void)
{
    SelectFillAttrHdl_Impl();
}

IMPL_LINK_NOARG(AreaPropertyPanelBase, SelectFillAttrHdl, ListBox&, void)
{
    SelectFillAttrHdl_Impl();
}

IMPL_LINK_NOARG(AreaPropertyPanelBase, ChangeGradientAngle, Edit&, void)
{
    SelectFillAttrHdl_Impl();
}

void AreaPropertyPanelBase::DataChanged(
    const DataChangedEvent& /*rEvent*/)
{
}

void AreaPropertyPanelBase::SelectFillAttrHdl_Impl()
{
    sal_Int32 nPosFillStyle = static_cast<eFillStyle>(mpLbFillType->GetSelectedEntryPos());
    SfxObjectShell* pSh = SfxObjectShell::Current();

    // #i122676# dependent from bFillStyleChange, do execute a single or two
    // changes in one Execute call
    const bool bFillStyleChange(static_cast<eFillStyle>(meLastXFS) != static_cast<eFillStyle>(nPosFillStyle));

    switch(nPosFillStyle)
    {
        case eFillStyle::NONE:
        {
            if(bFillStyleChange)
            {
                XFillStyleItem aXFillStyleItem(drawing::FillStyle_NONE);
                setFillStyle(aXFillStyleItem);
            }
            break;
        }
        case eFillStyle::SOLID:
        {
            if(bFillStyleChange)
            {
                // #i122676# Single FillStyle change call needed here
                XFillStyleItem aXFillStyleItem(drawing::FillStyle_SOLID);
                setFillStyle(aXFillStyleItem);
            }
            break;
        }
        case eFillStyle::GRADIENT:
        {

            if(pSh && pSh->GetItem(SID_COLOR_TABLE))
            {
                XGradient aGradient;
                aGradient.SetAngle(mpMTRAngle->GetValue() * 10);
                aGradient.SetGradientStyle(static_cast<css::awt::GradientStyle>(mpGradientStyle->GetSelectedEntryPos()));
                aGradient.SetStartColor(mpLbFillGradFrom->GetSelectEntryColor());
                aGradient.SetEndColor(mpLbFillGradTo->GetSelectEntryColor());

                const XFillGradientItem aXFillGradientItem(mpLbFillAttr->GetSelectedEntry(), aGradient);

                    // #i122676# Change FillStyle and Gradinet in one call
                XFillStyleItem aXFillStyleItem(drawing::FillStyle_GRADIENT);
                setFillStyleAndGradient(bFillStyleChange ? &aXFillStyleItem : nullptr, aXFillGradientItem);
            }
            break;
        }
        case eFillStyle::HATCH:
        {
            sal_Int32 nPos = mpLbFillAttr->GetSelectedEntryPos();

            if(LISTBOX_ENTRY_NOTFOUND == nPos)
            {
                nPos = mnLastPosHatch;
            }

            if(LISTBOX_ENTRY_NOTFOUND != nPos && pSh && pSh->GetItem(SID_HATCH_LIST))
            {
                const SvxHatchListItem * pItem = pSh->GetItem(SID_HATCH_LIST);

                if(nPos < pItem->GetHatchList()->Count())
                {
                    const XHatch aHatch = pItem->GetHatchList()->GetHatch(nPos)->GetHatch();
                    const XFillHatchItem aXFillHatchItem( mpLbFillAttr->GetSelectedEntry(), aHatch);

                    // #i122676# Change FillStyle and Hatch in one call
                    XFillStyleItem aXFillStyleItem(drawing::FillStyle_HATCH);
                    setFillStyleAndHatch(bFillStyleChange ? &aXFillStyleItem : nullptr, aXFillHatchItem);
                }
            }

            if(LISTBOX_ENTRY_NOTFOUND != nPos)
            {
                mnLastPosHatch = nPos;
            }
            break;
        }
        case eFillStyle::BITMAP:
        {
            sal_Int32 nPos = mpLbFillAttr->GetSelectedEntryPos();

            if(LISTBOX_ENTRY_NOTFOUND == nPos)
            {
                nPos = mnLastPosBitmap;
            }

            if(LISTBOX_ENTRY_NOTFOUND != nPos && pSh && pSh->GetItem(SID_BITMAP_LIST))
            {
                const SvxBitmapListItem * pItem = pSh->GetItem(SID_BITMAP_LIST);

                if(nPos < pItem->GetBitmapList()->Count())
                {
                    const XBitmapEntry* pXBitmapEntry = pItem->GetBitmapList()->GetBitmap(nPos);
                    const XFillBitmapItem aXFillBitmapItem(mpLbFillAttr->GetSelectedEntry(), pXBitmapEntry->GetGraphicObject());

                    // #i122676# Change FillStyle and Bitmap in one call
                    XFillStyleItem aXFillStyleItem(drawing::FillStyle_BITMAP);
                    setFillStyleAndBitmap(bFillStyleChange ? &aXFillStyleItem : nullptr, aXFillBitmapItem);
                }
            }

            if(LISTBOX_ENTRY_NOTFOUND != nPos)
            {
                mnLastPosBitmap = nPos;
            }
            break;
        }
        case eFillStyle::PATTERN:
        {
            sal_Int32 nPos = mpLbFillAttr->GetSelectedEntryPos();

            if(LISTBOX_ENTRY_NOTFOUND == nPos)
            {
                nPos = mnLastPosPattern;
            }

            if(LISTBOX_ENTRY_NOTFOUND != nPos && pSh && pSh->GetItem(SID_PATTERN_LIST))
            {
                const SvxPatternListItem * pItem = pSh->GetItem(SID_PATTERN_LIST);

                if(nPos < pItem->GetPatternList()->Count())
                {
                    const XBitmapEntry* pXPatternEntry = pItem->GetPatternList()->GetBitmap(nPos);
                    const XFillBitmapItem aXFillBitmapItem(mpLbFillAttr->GetSelectedEntry(), pXPatternEntry->GetGraphicObject());

                    // #i122676# Change FillStyle and Bitmap in one call
                    XFillStyleItem aXFillStyleItem(drawing::FillStyle_BITMAP);
                    setFillStyleAndBitmap(bFillStyleChange ? &aXFillStyleItem : nullptr, aXFillBitmapItem);
                }
            }

            if(LISTBOX_ENTRY_NOTFOUND != nPos)
            {
                mnLastPosPattern = nPos;
            }
            break;
        }
    }
}

void AreaPropertyPanelBase::ImpUpdateTransparencies()
{
    if(mpTransparanceItem.get() || mpFloatTransparenceItem.get())
    {
        bool bZeroValue(false);

        if (mpTransparanceItem)
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

            if (!bZeroValue && mxTrGrPopup)
            {
                mxTrGrPopup->EndPopupMode();
            }
        }

        if(bZeroValue && mpFloatTransparenceItem.get())
        {
            if(mpFloatTransparenceItem->IsEnabled())
            {
                const XGradient& rGradient = mpFloatTransparenceItem->GetGradientValue();
                sal_Int32 nEntryPos(0);
                Image* pImage = nullptr;

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

void AreaPropertyPanelBase::updateFillTransparence(bool bDisabled, bool bDefaultOrSet, const SfxPoolItem* pState)
{
    if (bDisabled)
    {
        mpTransparanceItem.reset();
        return;
    }
    else if (bDefaultOrSet)
    {
        if (pState)
        {
            const SfxUInt16Item* pItem = static_cast<const SfxUInt16Item*>(pState);
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

void AreaPropertyPanelBase::updateFillFloatTransparence(bool bDisabled, bool bDefaultOrSet, const SfxPoolItem* pState)
{
    if (bDisabled)
    {
        mpFloatTransparenceItem.reset();
        return;
    }

    if (bDefaultOrSet)
    {
        if (pState)
        {
            const XFillFloatTransparenceItem* pItem = static_cast<const XFillFloatTransparenceItem*>(pState);
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

void AreaPropertyPanelBase::updateFillStyle(bool bDisabled, bool bDefaultOrSet, const SfxPoolItem* pState)
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
    else if (bDefaultOrSet && pState)
    {
        const XFillStyleItem* pItem = static_cast<const XFillStyleItem*>(pState);
        mpStyleItem.reset(dynamic_cast< XFillStyleItem* >(pItem->Clone()));
        mpLbFillType->Enable();
        mpColorTextFT->Enable();
        drawing::FillStyle eXFS = mpStyleItem->GetValue();
        eFillStyle nPos = NONE;
        switch(eXFS)
        {
            default:
            case drawing::FillStyle_NONE:
            {
                mpLbFillAttr->SetNoSelection();
                mpLbFillAttr->Disable();
                nPos = NONE;
                break;
            }
            case drawing::FillStyle_SOLID:
                nPos = SOLID;
                break;
            case drawing::FillStyle_GRADIENT:
                nPos = GRADIENT;
                break;
            case drawing::FillStyle_HATCH:
                nPos = HATCH;
                break;
            case drawing::FillStyle_BITMAP:
            {
                if(mpBitmapItem)
                {
                    if(!mpBitmapItem->isPattern())
                        nPos = BITMAP;
                    else
                        nPos = PATTERN;
                }
                else
                    nPos = BITMAP;
                break;
            }
        }
        meLastXFS = static_cast< sal_uInt16 >(mpLbFillType->GetSelectedEntryPos());
        mpLbFillType->SelectEntryPos(static_cast< sal_Int32 >(nPos));
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

void AreaPropertyPanelBase::updateFillGradient(bool bDisabled, bool bDefaultOrSet, const SfxPoolItem* pState)
{
    if (bDefaultOrSet)
    {
        const XFillGradientItem* pItem = static_cast<const XFillGradientItem*>(pState);
        mpFillGradientItem.reset(pItem ? static_cast<XFillGradientItem*>(pItem->Clone()) : nullptr);
    }

    if(mpStyleItem && drawing::FillStyle_GRADIENT == mpStyleItem->GetValue())
    {
        mpLbFillAttr->Hide();
        mpLbFillGradFrom->Show();
        mpLbFillGradTo->Show();
        mpMTRAngle->Show();
        mpGradientStyle->Show();
        mpToolBoxColor->Hide();

        if (bDefaultOrSet)
        {
            mpLbFillType->SelectEntryPos(GRADIENT);
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

void AreaPropertyPanelBase::updateFillHatch(bool bDisabled, bool bDefaultOrSet, const SfxPoolItem* pState)
{
    if (bDefaultOrSet)
    {
        const XFillHatchItem* pItem = static_cast<const XFillHatchItem*>(pState);
        mpHatchItem.reset(pItem ? static_cast<XFillHatchItem*>(pItem->Clone()) : nullptr);
    }

    if(mpStyleItem && drawing::FillStyle_HATCH == mpStyleItem->GetValue())
    {
        mpLbFillAttr->Show();
        mpToolBoxColor->Hide();

        if (bDefaultOrSet)
        {
            mpLbFillAttr->Enable();
            mpLbFillType->SelectEntryPos(HATCH);
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

void AreaPropertyPanelBase::updateFillColor(bool bDefaultOrSet, const SfxPoolItem* pState)
{
    if (bDefaultOrSet)
    {
        const XFillColorItem* pItem = static_cast<const XFillColorItem*>(pState);
        mpColorItem.reset(pItem ? static_cast<XFillColorItem*>(pItem->Clone()) : nullptr);
    }

    if(mpStyleItem && drawing::FillStyle_SOLID == mpStyleItem->GetValue())
    {
        mpLbFillAttr->Hide();
        mpToolBoxColor->Show();
        mpLbFillType->SelectEntryPos(SOLID);
        Update();
    }
}

void AreaPropertyPanelBase::updateFillBitmap(bool bDisabled, bool bDefaultOrSet, const SfxPoolItem* pState)
{
    if (bDefaultOrSet)
    {
        const XFillBitmapItem* pItem = static_cast<const XFillBitmapItem*>(pState);
        mpBitmapItem.reset(pItem ? static_cast<XFillBitmapItem*>(pItem->Clone()) : nullptr);
    }

    if(mpStyleItem && drawing::FillStyle_BITMAP == mpStyleItem->GetValue())
    {
        mpLbFillAttr->Show();
        mpToolBoxColor->Hide();

        if (bDefaultOrSet)
        {
            if(mpBitmapItem->isPattern())
                mpLbFillType->SelectEntryPos(PATTERN);
            else
                mpLbFillType->SelectEntryPos(BITMAP);
            Update();
        }
        else if(bDisabled)
        {
            mpLbFillAttr->Hide();
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
            updateFillTransparence(bDisabled, bDefaultOrSet, pState);
        break;
        case SID_ATTR_FILL_FLOATTRANSPARENCE:
            updateFillFloatTransparence(bDisabled, bDefaultOrSet, pState);
        break;
        case SID_ATTR_FILL_STYLE:
            updateFillStyle(bDisabled, bDefaultOrSet, pState);
        break;
        case SID_ATTR_FILL_COLOR:
            updateFillColor(bDefaultOrSet, pState);
        break;
        case SID_ATTR_FILL_GRADIENT:
            updateFillGradient(bDisabled, bDefaultOrSet, pState);
        break;
        case SID_ATTR_FILL_HATCH:
            updateFillHatch(bDisabled, bDefaultOrSet, pState);
        break;
        case SID_ATTR_FILL_BITMAP:
            updateFillBitmap(bDisabled, bDefaultOrSet, pState);
        break;
        case SID_GRADIENT_LIST:
        {
            if(bDefault)
            {
                if(mpStyleItem && drawing::FillStyle_GRADIENT == mpStyleItem->GetValue())
                {
                    if(mpFillGradientItem)
                    {
                        const OUString aString( mpFillGradientItem->GetName() );
                        const SfxObjectShell* pSh = SfxObjectShell::Current();

                        mpLbFillAttr->Clear();
                        mpLbFillAttr->Enable();
                        mpLbFillAttr->Fill(pSh->GetItem(SID_GRADIENT_LIST)->GetGradientList());
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
                if(mpStyleItem && drawing::FillStyle_HATCH == mpStyleItem->GetValue())
                {
                    if(mpHatchItem)
                    {
                        const OUString aString( mpHatchItem->GetName() );
                        const SfxObjectShell* pSh = SfxObjectShell::Current();

                        mpLbFillAttr->Clear();
                        mpLbFillAttr->Enable();
                        mpLbFillAttr->Fill(pSh->GetItem(SID_HATCH_LIST)->GetHatchList());
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
        case SID_PATTERN_LIST:
        {
            if(bDefault)
            {
                if(mpStyleItem && drawing::FillStyle_BITMAP == mpStyleItem->GetValue())
                {
                    if(mpBitmapItem)
                    {
                        const OUString aString( mpBitmapItem->GetName() );
                        const SfxObjectShell* pSh = SfxObjectShell::Current();
                        mpLbFillAttr->Clear();
                        mpLbFillAttr->Show();
                        if(nSID == SID_BITMAP_LIST)
                        {
                            mpLbFillAttr->Fill(pSh->GetItem(SID_BITMAP_LIST)->GetBitmapList());
                        }
                        else if(nSID == SID_PATTERN_LIST)
                        {
                            mpLbFillAttr->Fill(pSh->GetItem(SID_PATTERN_LIST)->GetPatternList());
                        }
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

void AreaPropertyPanelBase::Update()
{
        const eFillStyle eXFS = static_cast<eFillStyle>(mpLbFillType->GetSelectedEntryPos());
        SfxObjectShell* pSh = SfxObjectShell::Current();

        switch( eXFS )
        {
            case eFillStyle::NONE:
            {
                mpLbFillAttr->Show();
                mpLbFillGradFrom->Hide();
                mpLbFillGradTo->Hide();
                mpMTRAngle->Hide();
                mpGradientStyle->Hide();
                mpToolBoxColor->Hide();
                mpBmpImport->Hide();
                break;
            }
            case eFillStyle::SOLID:
            {
                if(mpColorItem)
                {
                    mpLbFillAttr->Hide();
                    mpLbFillGradFrom->Hide();
                    mpLbFillGradTo->Hide();
                    mpMTRAngle->Hide();
                    mpGradientStyle->Hide();
                    mpToolBoxColor->Show();
                    mpBmpImport->Hide();
                }
                break;
            }
            case eFillStyle::GRADIENT:
            {
                mpLbFillAttr->Hide();
                mpLbFillGradFrom->Show();
                mpLbFillGradTo->Show();
                mpMTRAngle->Enable();
                mpMTRAngle->Show();
                mpGradientStyle->Show();
                mpToolBoxColor->Hide();
                mpBmpImport->Hide();

                if(pSh && pSh->GetItem(SID_GRADIENT_LIST))
                {
                    mpLbFillAttr->Enable();
                    mpLbFillAttr->Clear();
                    mpLbFillAttr->Fill(pSh->GetItem(SID_GRADIENT_LIST)->GetGradientList());
                    mpLbFillGradTo->SetNoSelection();
                    mpLbFillGradFrom->SetNoSelection();
                    if (mpFillGradientItem)
                    {
                        const OUString aString(mpFillGradientItem->GetName());
                        mpLbFillAttr->SelectEntry(aString);
                        const XGradient aGradient = mpFillGradientItem->GetGradientValue();
                        mpLbFillGradFrom->SelectEntry(aGradient.GetStartColor());
                        mpLbFillGradTo->SelectEntry(aGradient.GetEndColor());
                        mpGradientStyle->SelectEntryPos(sal::static_int_cast< sal_Int32 >( aGradient.GetGradientStyle() ));
                        if(mpGradientStyle->GetSelectedEntryPos() == sal_Int32(GradientStyle::Radial))
                            mpMTRAngle->Disable();
                        else
                            mpMTRAngle->SetValue( aGradient.GetAngle() /10 );
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
            case eFillStyle::HATCH:
            {
                mpLbFillAttr->Show();
                mpLbFillGradFrom->Hide();
                mpLbFillGradTo->Hide();
                mpMTRAngle->Hide();
                mpGradientStyle->Hide();
                mpToolBoxColor->Hide();
                mpBmpImport->Hide();
                mpBmpImport->Hide();

                if(pSh && pSh->GetItem(SID_HATCH_LIST))
                {
                    mpLbFillAttr->Enable();
                    mpLbFillAttr->Clear();
                    mpLbFillAttr->Fill(pSh->GetItem(SID_HATCH_LIST)->GetHatchList());

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
            case eFillStyle::BITMAP:
            case eFillStyle::PATTERN:
            {
                mpLbFillAttr->Show();
                mpLbFillAttr->Enable();
                mpLbFillAttr->Clear();
                mpToolBoxColor->Hide();
                mpLbFillGradFrom->Hide();
                mpLbFillGradTo->Hide();
                mpMTRAngle->Hide();
                mpGradientStyle->Hide();

                if(mpBitmapItem)
                {
                    if(pSh && pSh->GetItem(SID_BITMAP_LIST) && eXFS == BITMAP)
                    {
                        mpBmpImport->Show();
                        mpLbFillType->SelectEntryPos(sal_uInt32(BITMAP));
                        mpLbFillAttr->Fill(pSh->GetItem(SID_BITMAP_LIST)->GetBitmapList());

                        const OUString aString(mpBitmapItem->GetName());
                        mpLbFillAttr->SelectEntry(aString);
                    }
                    else if(pSh && pSh->GetItem(SID_PATTERN_LIST) && eXFS == PATTERN)
                    {
                        mpBmpImport->Hide();
                        mpLbFillType->SelectEntryPos(sal_uInt32(PATTERN));
                        mpLbFillAttr->Fill(pSh->GetItem(SID_PATTERN_LIST)->GetPatternList());

                        const OUString aString(mpBitmapItem->GetName());
                        mpLbFillAttr->SelectEntry(aString);
                    }
                }
                else
                    mpLbFillAttr->SetNoSelection();
                break;
            }
            default:
                OSL_ENSURE(false, "Non supported FillType (!)");
            break;
        }
}

IMPL_LINK_NOARG(AreaPropertyPanelBase, ModifyTransSliderHdl, Slider*, void)
{
    const sal_uInt16 nVal = mpSldTransparent->GetThumbPos();
    SetTransparency(nVal);
    const XFillTransparenceItem aLinearItem(nVal);
    setFillTransparence(aLinearItem);
}

IMPL_LINK( AreaPropertyPanelBase, ClickTrGrHdl_Impl, ToolBox*, pToolBox, void )
{
    if (!mxTrGrPopup)
        mxTrGrPopup = VclPtr<AreaTransparencyGradientPopup>::Create(*this);
    if (mpFloatTransparenceItem)
        mxTrGrPopup->Rearrange(mpFloatTransparenceItem.get());
    OSL_ASSERT( pToolBox->GetItemCommand(pToolBox->GetCurItemId()) == UNO_SIDEBARGRADIENT);
    mxTrGrPopup->StartPopupMode(pToolBox, FloatWinPopupFlags::GrabFocus);
}

IMPL_LINK_NOARG(AreaPropertyPanelBase, ChangeTrgrTypeHdl_Impl, ListBox&, void)
{
    sal_Int32 nSelectType = mpLBTransType->GetSelectedEntryPos();
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

    switch(static_cast<css::awt::GradientStyle>(nSelectType))
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

    const XFillFloatTransparenceItem aGradientItem(aTmpGradient, bGradient);
    setFillFloatTransparence(aGradientItem);
}

IMPL_LINK_NOARG(AreaPropertyPanelBase, ModifyTransparentHdl_Impl, Edit&, void)
{
    const sal_uInt16 nTrans = static_cast<sal_uInt16>(mpMTRTransparent->GetValue());
    mnLastTransSolid = nTrans;
    SetTransparency(nTrans);
    const sal_Int32 nSelectType = mpLBTransType->GetSelectedEntryPos();

    if(nTrans && !nSelectType)
    {
        mpLBTransType->SelectEntryPos(1);
    }

    const XFillTransparenceItem aLinearItem(nTrans);
    setFillTransparence(aLinearItem);
}

const XGradient& AreaPropertyPanelBase::GetGradient (const css::awt::GradientStyle eStyle) const
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
    return mpLBTransType->GetSelectedEntryPos();
}

} } // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
