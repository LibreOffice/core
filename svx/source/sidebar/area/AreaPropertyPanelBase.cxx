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

#include <osl/diagnose.h>
#include <svx/sidebar/AreaPropertyPanelBase.hxx>
#include <svx/drawitem.hxx>
#include <svx/itemwin.hxx>
#include <svx/svxids.hrc>
#include <sfx2/objsh.hxx>
#include <svx/xfltrit.hxx>
#include <svx/xflftrit.hxx>
#include <svx/xtable.hxx>
#include <sfx2/opengrf.hxx>
#include <sfx2/weldutils.hxx>
#include <tools/urlobj.hxx>
#include <bitmaps.hlst>
#include <comphelper/lok.hxx>

using namespace css;
using namespace css::uno;

const char SIDEBARGRADIENT[] = "sidebargradient";

namespace svx::sidebar {

namespace {

enum eFillStyle
{
    NONE,
    SOLID,
    GRADIENT,
    HATCH,
    BITMAP,
    PATTERN
};

}

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
      mxColorTextFT(m_xBuilder->weld_label("filllabel")),
      mxLbFillType(m_xBuilder->weld_combo_box("fillstylearea")),
      mxLbFillAttr(m_xBuilder->weld_combo_box("fillattrhb")),
      mxLbFillGradFrom(new ColorListBox(m_xBuilder->weld_menu_button("fillgrad1"), GetFrameWeld())),
      mxLbFillGradTo(new ColorListBox(m_xBuilder->weld_menu_button("fillgrad2"), GetFrameWeld())),
      mxToolBoxColor(m_xBuilder->weld_toolbar("selectcolor")),
      mxColorDispatch(new ToolbarUnoDispatcher(*mxToolBoxColor, *m_xBuilder, rxFrame)),
      mxTrspTextFT(m_xBuilder->weld_label("transparencylabel")),
      mxLBTransType(m_xBuilder->weld_combo_box("transtype")),
      mxMTRTransparent(m_xBuilder->weld_metric_spin_button("settransparency", FieldUnit::PERCENT)),
      mxSldTransparent(m_xBuilder->weld_scale("transparencyslider")),
      mxBTNGradient(m_xBuilder->weld_toolbar("selectgradient")),
      mxMTRAngle(m_xBuilder->weld_metric_spin_button("gradangle", FieldUnit::DEGREE)),
      mxGradientStyle(m_xBuilder->weld_combo_box("gradientstyle")),
      mxBmpImport(m_xBuilder->weld_button("bmpimport")),
      mpStyleItem(),
      mpColorItem(),
      mpFillGradientItem(),
      mpHatchItem(),
      mpBitmapItem(),
      maImgAxial(BMP_AXIAL),
      maImgElli(BMP_ELLI),
      maImgQuad(BMP_QUAD),
      maImgRadial(BMP_RADIAL),
      maImgSquare(BMP_SQUARE),
      maImgLinear(BMP_LINEAR),
      mpFloatTransparenceItem(),
      mpTransparanceItem()
{
    mpPanel = dynamic_cast<sfx2::sidebar::Panel*>(pParent);

    Initialize();

    m_pInitialFocusWidget = mxLbFillType.get();
}

AreaPropertyPanelBase::~AreaPropertyPanelBase()
{
    disposeOnce();
}

void AreaPropertyPanelBase::dispose()
{
    mxTrGrPopup.reset();
    mxColorTextFT.reset();
    mxLbFillType.reset();
    mxLbFillAttr.reset();
    mxColorDispatch.reset();
    mxToolBoxColor.reset();
    mxTrspTextFT.reset();
    mxLBTransType.reset();
    mxMTRTransparent.reset();
    mxSldTransparent.reset();
    mxBTNGradient.reset();
    mxMTRAngle.reset();
    mxLbFillGradFrom.reset();
    mxLbFillGradTo.reset();
    mxGradientStyle.reset();
    mxBmpImport.reset();
    mpPanel.clear();

    PanelLayout::dispose();
}

void AreaPropertyPanelBase::Initialize()
{
    SvxFillTypeBox::Fill(*mxLbFillType);

    mxLbFillAttr->set_size_request(42, -1);

    maGradientLinear.SetXOffset(DEFAULT_CENTERX);
    maGradientLinear.SetYOffset(DEFAULT_CENTERY);
    maGradientLinear.SetAngle(Degree10(DEFAULT_ANGLE));
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


    mxLbFillType->connect_changed( LINK( this, AreaPropertyPanelBase, SelectFillTypeHdl ) );

    Link<weld::ComboBox&,void> aLink = LINK( this, AreaPropertyPanelBase, SelectFillAttrHdl );
    mxLbFillAttr->connect_changed( aLink );
    mxGradientStyle->connect_changed( aLink );
    Link<ColorListBox&,void> aLink3 = LINK( this, AreaPropertyPanelBase, SelectFillColorHdl );
    mxLbFillGradFrom->SetSelectHdl( aLink3 );
    mxLbFillGradTo->SetSelectHdl( aLink3 );
    mxMTRAngle->connect_value_changed(LINK(this,AreaPropertyPanelBase, ChangeGradientAngle));

    // https://gerrit.libreoffice.org/c/core/+/87313 set a small width to force widgets to
    // take their final width from other widgets in the grid
    mxLbFillGradFrom->get_widget().set_size_request(42, -1);
    mxLbFillGradTo->get_widget().set_size_request(42, -1);

    mxLBTransType->connect_changed(LINK(this, AreaPropertyPanelBase, ChangeTrgrTypeHdl_Impl));

    SetTransparency( 50 );
    mxMTRTransparent->connect_value_changed(LINK(this, AreaPropertyPanelBase, ModifyTransparentHdl_Impl));
    mxSldTransparent->connect_value_changed(LINK(this, AreaPropertyPanelBase, ModifyTransSliderHdl));

    mxTrGrPopup = std::make_unique<AreaTransparencyGradientPopup>(*this, mxBTNGradient.get());

    mxBTNGradient->set_item_popover(SIDEBARGRADIENT, mxTrGrPopup->getTopLevel());
    mxBTNGradient->connect_clicked(LINK(this, AreaPropertyPanelBase, ToolbarHdl_Impl));

    mxBTNGradient->set_item_icon_name(SIDEBARGRADIENT, maImgLinear);
    mxBTNGradient->hide();
    mxBmpImport->connect_clicked( LINK(this, AreaPropertyPanelBase, ClickImportBitmapHdl));
}

IMPL_LINK_NOARG(AreaPropertyPanelBase, ToolbarHdl_Impl, const OString&, void)
{
    mxBTNGradient->set_menu_item_active(SIDEBARGRADIENT, !mxBTNGradient->get_menu_item_active(SIDEBARGRADIENT));
}

void AreaPropertyPanelBase::SetTransparency(sal_uInt16 nVal)
{
    mxSldTransparent->set_value(nVal);
    mxMTRTransparent->set_value(nVal, FieldUnit::PERCENT);
}

IMPL_LINK_NOARG(AreaPropertyPanelBase, ClickImportBitmapHdl, weld::Button&, void)
{
    SvxOpenGraphicDialog aDlg("Import", GetFrameWeld());
    aDlg.EnableLink(false);
    if( aDlg.Execute() != ERRCODE_NONE )
        return;

    Graphic aGraphic;
    EnterWait();
    ErrCode nError = aDlg.GetGraphic( aGraphic );
    LeaveWait();
    if( nError != ERRCODE_NONE )
        return;

    XBitmapListRef pList = SfxObjectShell::Current()->GetItem(SID_BITMAP_LIST)->GetBitmapList();
    INetURLObject   aURL( aDlg.GetPath() );
    OUString aFileName = aURL.GetLastName().getToken(0, '.');
    OUString aName = aFileName;
    tools::Long j = 1;
    bool bValidBitmapName = false;
    while( !bValidBitmapName )
    {
        bValidBitmapName = true;
        for( tools::Long i = 0; i < pList->Count() && bValidBitmapName; i++ )
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
    mxLbFillAttr->clear();
    SvxFillAttrBox::Fill(*mxLbFillAttr, pList);
    mxLbFillAttr->set_active_text(aName);
    SelectFillAttrHdl(*mxLbFillAttr);
}

IMPL_LINK_NOARG(AreaPropertyPanelBase, SelectFillTypeHdl, weld::ComboBox&, void)
{
    sal_Int32 nPos = static_cast<eFillStyle>(mxLbFillType->get_active());
    mxLbFillAttr->clear();
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
            mxLbFillAttr->show();
            mxLbFillGradFrom->hide();
            mxLbFillGradTo->hide();
            mxGradientStyle->hide();
            mxMTRAngle->hide();
            mxToolBoxColor->hide();
            mxBmpImport->hide();
            mxLbFillAttr->set_sensitive(false);

            // #i122676# need to call a single SID_ATTR_FILL_STYLE change
            setFillStyle(XFillStyleItem(drawing::FillStyle_NONE));
            break;
        }
        case SOLID:
        {
            mxLbFillAttr->hide();
            mxLbFillGradFrom->hide();
            mxLbFillGradTo->hide();
            mxGradientStyle->hide();
            mxMTRAngle->hide();
            mxBmpImport->hide();
            mxToolBoxColor->show();
            const Color aColor = mpColorItem ? mpColorItem->GetColorValue() : COL_AUTO;
            const XFillColorItem aXFillColorItem( "", aColor );

            // #i122676# change FillStyle and Color in one call
            XFillStyleItem aXFillStyleItem(drawing::FillStyle_SOLID);
            setFillStyleAndColor(&aXFillStyleItem, aXFillColorItem);
            break;
        }
        case GRADIENT:
        {
            mxLbFillAttr->hide();
            mxLbFillGradFrom->show();
            mxLbFillGradTo->show();
            mxGradientStyle->show();
            mxMTRAngle->show();
            mxToolBoxColor->hide();
            mxBmpImport->hide();

            mxLbFillAttr->set_sensitive(true);
            mxLbFillGradTo->set_sensitive(true);
            mxLbFillGradFrom->set_sensitive(true);
            mxGradientStyle->set_sensitive(true);
            mxMTRAngle->set_sensitive(true);
            mxLbFillAttr->clear();

            const SvxGradientListItem * pItem = pSh->GetItem(SID_GRADIENT_LIST);

            if(0 < pItem->GetGradientList()->Count())
            {
                const XGradient aGradient = pItem->GetGradientList()->GetGradient(0)->GetGradient();
                const OUString aName = pItem->GetGradientList()->GetGradient(0)->GetName();
                const XFillGradientItem aXFillGradientItem(aName, aGradient);

                // #i122676# change FillStyle and Gradient in one call
                XFillStyleItem aXFillStyleItem(drawing::FillStyle_GRADIENT);
                setFillStyleAndGradient(&aXFillStyleItem, aXFillGradientItem);
                mxLbFillGradFrom->SelectEntry(aGradient.GetStartColor());
                mxLbFillGradTo->SelectEntry(aGradient.GetEndColor());

                mxMTRAngle->set_value(aGradient.GetAngle().get() / 10, FieldUnit::DEGREE);
                css::awt::GradientStyle eXGS = aGradient.GetGradientStyle();
                mxGradientStyle->set_active(sal::static_int_cast< sal_Int32 >( eXGS ));
            }
            break;
        }
        case HATCH:
        {
            mxLbFillAttr->show();
            mxLbFillGradFrom->hide();
            mxLbFillGradTo->hide();
            mxMTRAngle->hide();
            mxGradientStyle->hide();
            mxToolBoxColor->hide();
            mxBmpImport->hide();

            const SvxHatchListItem* pItem( pSh->GetItem(SID_HATCH_LIST) );
            if(pItem)
            {
                const XHatchListRef& pXHatchList(pItem->GetHatchList());
                mxLbFillAttr->set_sensitive(true);
                mxLbFillAttr->clear();
                SvxFillAttrBox::Fill(*mxLbFillAttr, pXHatchList);

                if (mnLastPosHatch != -1)
                {
                    if(mnLastPosHatch < pXHatchList->Count())
                    {
                        const XHatch aHatch = pXHatchList->GetHatch(mnLastPosHatch)->GetHatch();
                        const OUString aName = pXHatchList->GetHatch(mnLastPosHatch)->GetName();
                        const XFillHatchItem aXFillHatchItem(aName, aHatch);

                        // #i122676# change FillStyle and Hatch in one call
                        XFillStyleItem aXFillStyleItem(drawing::FillStyle_HATCH);
                        setFillStyleAndHatch(&aXFillStyleItem, aXFillHatchItem);
                        mxLbFillAttr->set_active(mnLastPosHatch);
                    }
                }
            }
            else
            {
                mxLbFillAttr->set_sensitive(false);
            }
            break;
        }
        case BITMAP:
        case PATTERN:
        {
            mxLbFillAttr->show();
            mxLbFillAttr->set_sensitive(true);
            mxLbFillAttr->clear();
            mxLbFillGradFrom->hide();
            mxLbFillGradTo->hide();
            mxMTRAngle->hide();
            mxGradientStyle->hide();
            mxToolBoxColor->hide();

            OUString aName;
            GraphicObject aBitmap;
            if(nPos == static_cast< sal_Int32 >(BITMAP))
            {
                if (!comphelper::LibreOfficeKit::isActive())
                    mxBmpImport->show();
                const SvxBitmapListItem* pItem = pSh->GetItem(SID_BITMAP_LIST);
                if(pItem)
                {
                    const XBitmapListRef& pXBitmapList(pItem->GetBitmapList());
                    SvxFillAttrBox::Fill(*mxLbFillAttr, pXBitmapList);

                    if (mnLastPosBitmap != -1)
                    {
                        if(mnLastPosBitmap < pXBitmapList->Count())
                        {
                            const XBitmapEntry* pXBitmapEntry = pXBitmapList->GetBitmap(mnLastPosBitmap);
                            aBitmap = pXBitmapEntry->GetGraphicObject();
                            aName = pXBitmapEntry->GetName();
                            mxLbFillAttr->set_active(mnLastPosBitmap);
                        }
                    }
                }
                else
                {
                    mxLbFillAttr->hide();
                }
            }
            else if(nPos == static_cast< sal_Int32 >(PATTERN))
            {
                mxBmpImport->hide();
                const SvxPatternListItem* pItem = pSh->GetItem(SID_PATTERN_LIST);
                if(pItem)
                {
                    const XPatternListRef& pXPatternList(pItem->GetPatternList());
                    SvxFillAttrBox::Fill(*mxLbFillAttr, pXPatternList);

                    if (mnLastPosPattern != -1)
                    {
                        if(mnLastPosPattern < pXPatternList->Count())
                        {
                            const XBitmapEntry* pXPatternEntry = pXPatternList->GetBitmap(mnLastPosPattern);
                            aBitmap = pXPatternEntry->GetGraphicObject();
                            aName = pXPatternEntry->GetName();
                            mxLbFillAttr->set_active(mnLastPosPattern);
                        }
                    }
                }
                else
                {
                    mxLbFillAttr->hide();
                }
            }
            const XFillBitmapItem aXFillBitmapItem( aName, aBitmap );
            const XFillStyleItem aXFillStyleItem(drawing::FillStyle_BITMAP);
            setFillStyleAndBitmap(&aXFillStyleItem, aXFillBitmapItem);
            break;
        }
    }

    meLastXFS = static_cast<sal_uInt16>(nPos);

    if(mpPanel)
        mpPanel->TriggerDeckLayouting();
}

IMPL_LINK_NOARG(AreaPropertyPanelBase, SelectFillColorHdl, ColorListBox&, void)
{
    SelectFillAttrHdl_Impl();
}

IMPL_LINK_NOARG(AreaPropertyPanelBase, SelectFillAttrHdl, weld::ComboBox&, void)
{
    SelectFillAttrHdl_Impl();
}

IMPL_LINK_NOARG(AreaPropertyPanelBase, ChangeGradientAngle, weld::MetricSpinButton&, void)
{
    SelectFillAttrHdl_Impl();
}

void AreaPropertyPanelBase::DataChanged(
    const DataChangedEvent& /*rEvent*/)
{
}

void AreaPropertyPanelBase::SelectFillAttrHdl_Impl()
{
    sal_Int32 nPosFillStyle = static_cast<eFillStyle>(mxLbFillType->get_active());
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
                aGradient.SetAngle(Degree10(mxMTRAngle->get_value(FieldUnit::DEGREE) * 10));
                aGradient.SetGradientStyle(static_cast<css::awt::GradientStyle>(mxGradientStyle->get_active()));
                aGradient.SetStartColor(mxLbFillGradFrom->GetSelectEntryColor());
                aGradient.SetEndColor(mxLbFillGradTo->GetSelectEntryColor());

                const XFillGradientItem aXFillGradientItem(mxLbFillAttr->get_active_text(), aGradient);

                    // #i122676# Change FillStyle and Gradient in one call
                XFillStyleItem aXFillStyleItem(drawing::FillStyle_GRADIENT);
                setFillStyleAndGradient(bFillStyleChange ? &aXFillStyleItem : nullptr, aXFillGradientItem);
            }
            break;
        }
        case eFillStyle::HATCH:
        {
            sal_Int32 nPos = mxLbFillAttr->get_active();

            if (nPos == -1)
            {
                nPos = mnLastPosHatch;
            }

            if (nPos != -1 && pSh && pSh->GetItem(SID_HATCH_LIST))
            {
                const SvxHatchListItem * pItem = pSh->GetItem(SID_HATCH_LIST);

                if(nPos < pItem->GetHatchList()->Count())
                {
                    const XHatch aHatch = pItem->GetHatchList()->GetHatch(nPos)->GetHatch();
                    const XFillHatchItem aXFillHatchItem( mxLbFillAttr->get_active_text(), aHatch);

                    // #i122676# Change FillStyle and Hatch in one call
                    XFillStyleItem aXFillStyleItem(drawing::FillStyle_HATCH);
                    setFillStyleAndHatch(bFillStyleChange ? &aXFillStyleItem : nullptr, aXFillHatchItem);
                }
            }

            if (nPos != -1)
            {
                mnLastPosHatch = nPos;
            }
            break;
        }
        case eFillStyle::BITMAP:
        {
            sal_Int32 nPos = mxLbFillAttr->get_active();

            if (nPos == -1)
            {
                nPos = mnLastPosBitmap;
            }

            if (nPos != -1 && pSh && pSh->GetItem(SID_BITMAP_LIST))
            {
                const SvxBitmapListItem * pItem = pSh->GetItem(SID_BITMAP_LIST);

                if(nPos < pItem->GetBitmapList()->Count())
                {
                    const XBitmapEntry* pXBitmapEntry = pItem->GetBitmapList()->GetBitmap(nPos);
                    const XFillBitmapItem aXFillBitmapItem(mxLbFillAttr->get_active_text(), pXBitmapEntry->GetGraphicObject());

                    // #i122676# Change FillStyle and Bitmap in one call
                    XFillStyleItem aXFillStyleItem(drawing::FillStyle_BITMAP);
                    setFillStyleAndBitmap(bFillStyleChange ? &aXFillStyleItem : nullptr, aXFillBitmapItem);
                }
            }

            if (nPos != -1)
            {
                mnLastPosBitmap = nPos;
            }
            break;
        }
        case eFillStyle::PATTERN:
        {
            sal_Int32 nPos = mxLbFillAttr->get_active();

            if (nPos == -1)
            {
                nPos = mnLastPosPattern;
            }

            if (nPos != -1 && pSh && pSh->GetItem(SID_PATTERN_LIST))
            {
                const SvxPatternListItem * pItem = pSh->GetItem(SID_PATTERN_LIST);

                if(nPos < pItem->GetPatternList()->Count())
                {
                    const XBitmapEntry* pXPatternEntry = pItem->GetPatternList()->GetBitmap(nPos);
                    const XFillBitmapItem aXFillBitmapItem(mxLbFillAttr->get_active_text(), pXPatternEntry->GetGraphicObject());

                    // #i122676# Change FillStyle and Bitmap in one call
                    XFillStyleItem aXFillStyleItem(drawing::FillStyle_BITMAP);
                    setFillStyleAndBitmap(bFillStyleChange ? &aXFillStyleItem : nullptr, aXFillBitmapItem);
                }
            }

            if (nPos != -1)
            {
                mnLastPosPattern = nPos;
            }
            break;
        }
    }
    if(mpPanel)
        mpPanel->TriggerDeckLayouting();
}

void AreaPropertyPanelBase::ImpUpdateTransparencies()
{
    if(mpTransparanceItem || mpFloatTransparenceItem)
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
                mxLBTransType->set_sensitive(true);
                mxTrspTextFT->set_sensitive(true);
                mxLBTransType->set_active(1);
                mxBTNGradient->hide();
                mxMTRTransparent->show();
                mxSldTransparent->show();
                mxMTRTransparent->set_sensitive(true);
                mxSldTransparent->set_sensitive(true);
                SetTransparency(nValue);
            }

            if (!bZeroValue && mxTrGrPopup)
            {
                mxBTNGradient->set_menu_item_active(SIDEBARGRADIENT, false);
            }
        }

        if(bZeroValue && mpFloatTransparenceItem)
        {
            if(mpFloatTransparenceItem->IsEnabled())
            {
                const XGradient& rGradient = mpFloatTransparenceItem->GetGradientValue();
                sal_Int32 nEntryPos(0);
                OUString* pImage = nullptr;

                mxLBTransType->set_sensitive(true);
                mxTrspTextFT->set_sensitive(true);
                mxMTRTransparent->hide();
                mxSldTransparent->hide();
                mxBTNGradient->set_sensitive(true);
                mxBTNGradient->show();

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
                mxLBTransType->set_active(nEntryPos);
                mxBTNGradient->set_item_icon_name(SIDEBARGRADIENT, *pImage);
                mxTrGrPopup->Rearrange(mpFloatTransparenceItem.get());
                bZeroValue = false;
            }
            else
            {
                bZeroValue = true;
            }
        }

        if(bZeroValue)
        {
            mxLBTransType->set_sensitive(true);
            mxTrspTextFT->set_sensitive(true);
            mxLBTransType->set_active(0);
            mxBTNGradient->hide();
            mxMTRTransparent->set_sensitive(true);
            mxSldTransparent->set_sensitive(true);
            mxMTRTransparent->show();
            mxSldTransparent->show();
            SetTransparency(0);
        }
    }
    else
    {
        // no transparency at all
        mxLBTransType->set_active(-1);
        mxLBTransType->set_sensitive(false);
        mxTrspTextFT->set_sensitive(false);
        mxMTRTransparent->set_sensitive(false);
        mxSldTransparent->set_sensitive(false);
        mxMTRTransparent->show();
        mxSldTransparent->show();
        mxBTNGradient->set_sensitive(false);
        mxBTNGradient->hide();
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
            mpTransparanceItem.reset(pItem->Clone());
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
            mpFloatTransparenceItem.reset(pItem->Clone());
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
        mxLbFillType->set_sensitive(false);
        mxColorTextFT->set_sensitive(false);
        mxLbFillType->set_active(-1);
        mxLbFillAttr->show();
        mxLbFillAttr->set_sensitive(false);
        mxLbFillAttr->set_active(-1);
        mxToolBoxColor->hide();
        meLastXFS = static_cast<sal_uInt16>(-1);
        mpStyleItem.reset();
    }
    else if (bDefaultOrSet && pState)
    {
        const XFillStyleItem* pItem = static_cast<const XFillStyleItem*>(pState);
        mpStyleItem.reset(pItem->Clone());
        mxLbFillType->set_sensitive(true);
        mxColorTextFT->set_sensitive(true);
        drawing::FillStyle eXFS = mpStyleItem->GetValue();
        eFillStyle nPos = NONE;
        switch(eXFS)
        {
            default:
            case drawing::FillStyle_NONE:
            {
                mxLbFillAttr->set_active(-1);
                mxLbFillAttr->set_sensitive(false);
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
        meLastXFS = static_cast< sal_uInt16 >(mxLbFillType->get_active());
        mxLbFillType->set_active(static_cast< sal_Int32 >(nPos));
        Update();
        return;
    }

    mxLbFillType->set_active(-1);
    mxLbFillAttr->show();
    mxLbFillAttr->set_sensitive(false);
    mxLbFillAttr->set_active(-1);
    mxToolBoxColor->hide();
    meLastXFS = static_cast<sal_uInt16>(-1);
    mpStyleItem.reset();
    if(mpPanel)
        mpPanel->TriggerDeckLayouting();
}

void AreaPropertyPanelBase::updateFillGradient(bool bDisabled, bool bDefaultOrSet, const SfxPoolItem* pState)
{
    if (bDefaultOrSet)
    {
        const XFillGradientItem* pItem = static_cast<const XFillGradientItem*>(pState);
        mpFillGradientItem.reset(pItem ? pItem->Clone() : nullptr);
    }

    if(mpStyleItem && drawing::FillStyle_GRADIENT == mpStyleItem->GetValue())
    {
        mxLbFillAttr->hide();
        mxLbFillGradFrom->show();
        mxLbFillGradTo->show();
        mxMTRAngle->show();
        mxGradientStyle->show();
        mxToolBoxColor->hide();

        if (bDefaultOrSet)
        {
            mxLbFillType->set_active(GRADIENT);
            Update();
        }
        else if(bDisabled)
        {
            mxLbFillGradFrom->SetNoSelection();
            mxLbFillGradTo->SetNoSelection();
            mxLbFillGradFrom->set_sensitive(false);
            mxLbFillGradTo->set_sensitive(false);
            mxMTRAngle->set_sensitive(false);
            mxGradientStyle->set_sensitive(false);
        }
        else
        {
            mxLbFillGradFrom->SetNoSelection();
            mxLbFillGradTo->SetNoSelection();
        }
    }
    if(mpPanel)
        mpPanel->TriggerDeckLayouting();
}

void AreaPropertyPanelBase::updateFillHatch(bool bDisabled, bool bDefaultOrSet, const SfxPoolItem* pState)
{
    if (bDefaultOrSet)
    {
        const XFillHatchItem* pItem = static_cast<const XFillHatchItem*>(pState);
        mpHatchItem.reset(pItem ? pItem->Clone() : nullptr);
    }

    if(mpStyleItem && drawing::FillStyle_HATCH == mpStyleItem->GetValue())
    {
        mxLbFillAttr->show();
        mxToolBoxColor->hide();

        if (bDefaultOrSet)
        {
            mxLbFillAttr->set_sensitive(true);
            mxLbFillType->set_active(HATCH);
            Update();
        }
        else if(bDisabled)
        {
            mxLbFillAttr->set_sensitive(false);
            mxLbFillAttr->set_active(-1);
        }
        else
        {
            mxLbFillAttr->set_active(-1);
        }
    }
    if(mpPanel)
        mpPanel->TriggerDeckLayouting();
}

void AreaPropertyPanelBase::updateFillColor(bool bDefaultOrSet, const SfxPoolItem* pState)
{
    if (bDefaultOrSet)
    {
        const XFillColorItem* pItem = static_cast<const XFillColorItem*>(pState);
        mpColorItem.reset(pItem ? pItem->Clone() : nullptr);
    }

    if(mpStyleItem && drawing::FillStyle_SOLID == mpStyleItem->GetValue())
    {
        mxLbFillAttr->hide();
        mxToolBoxColor->show();
        mxLbFillType->set_active(SOLID);
        Update();
    }
    if(mpPanel)
        mpPanel->TriggerDeckLayouting();
}

void AreaPropertyPanelBase::updateFillBitmap(bool bDisabled, bool bDefaultOrSet, const SfxPoolItem* pState)
{
    if (bDefaultOrSet)
    {
        const XFillBitmapItem* pItem = static_cast<const XFillBitmapItem*>(pState);
        mpBitmapItem.reset(pItem ? pItem->Clone() : nullptr);
    }

    if(mpStyleItem && drawing::FillStyle_BITMAP == mpStyleItem->GetValue())
    {
        mxLbFillAttr->show();
        mxToolBoxColor->hide();

        if (bDefaultOrSet)
        {
            if(mpBitmapItem->isPattern())
                mxLbFillType->set_active(PATTERN);
            else
                mxLbFillType->set_active(BITMAP);
            Update();
        }
        else if(bDisabled)
        {
            mxLbFillAttr->hide();
            mxLbFillAttr->set_active(-1);
        }
        else
        {
            mxLbFillAttr->set_active(-1);
        }
    }
    if(mpPanel)
        mpPanel->TriggerDeckLayouting();
}

void AreaPropertyPanelBase::NotifyItemUpdate(
    sal_uInt16 nSID,
    SfxItemState eState,
    const SfxPoolItem* pState)
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

                        mxLbFillAttr->clear();
                        mxLbFillAttr->set_sensitive(true);
                        SvxFillAttrBox::Fill(*mxLbFillAttr, pSh->GetItem(SID_GRADIENT_LIST)->GetGradientList());
                        mxLbFillAttr->set_active_text(aString);
                    }
                    else
                    {
                        mxLbFillAttr->set_active(-1);
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

                        mxLbFillAttr->clear();
                        mxLbFillAttr->set_sensitive(true);
                        SvxFillAttrBox::Fill(*mxLbFillAttr, pSh->GetItem(SID_HATCH_LIST)->GetHatchList());
                        mxLbFillAttr->set_active_text(aString);
                    }
                    else
                    {
                        mxLbFillAttr->set_active(-1);
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
                        mxLbFillAttr->clear();
                        mxLbFillAttr->show();
                        if(nSID == SID_BITMAP_LIST)
                        {
                            SvxFillAttrBox::Fill(*mxLbFillAttr, pSh->GetItem(SID_BITMAP_LIST)->GetBitmapList());
                        }
                        else if(nSID == SID_PATTERN_LIST)
                        {
                            SvxFillAttrBox::Fill(*mxLbFillAttr, pSh->GetItem(SID_PATTERN_LIST)->GetPatternList());
                        }
                        mxLbFillAttr->set_active_text(aString);
                    }
                    else
                    {
                        mxLbFillAttr->set_active(-1);
                    }
                }
            }
            break;
        }
    }
}

void AreaPropertyPanelBase::Update()
{
        const eFillStyle eXFS = static_cast<eFillStyle>(mxLbFillType->get_active());
        SfxObjectShell* pSh = SfxObjectShell::Current();

        switch( eXFS )
        {
            case eFillStyle::NONE:
            {
                mxLbFillAttr->show();
                mxLbFillGradFrom->hide();
                mxLbFillGradTo->hide();
                mxMTRAngle->hide();
                mxGradientStyle->hide();
                mxToolBoxColor->hide();
                mxBmpImport->hide();
                break;
            }
            case eFillStyle::SOLID:
            {
                if(mpColorItem)
                {
                    mxLbFillAttr->hide();
                    mxLbFillGradFrom->hide();
                    mxLbFillGradTo->hide();
                    mxMTRAngle->hide();
                    mxGradientStyle->hide();
                    mxToolBoxColor->show();
                    mxBmpImport->hide();
                }
                break;
            }
            case eFillStyle::GRADIENT:
            {
                mxLbFillAttr->hide();
                mxLbFillGradFrom->show();
                mxLbFillGradTo->show();
                mxMTRAngle->set_sensitive(true);
                mxMTRAngle->show();
                mxGradientStyle->show();
                mxToolBoxColor->hide();
                mxBmpImport->hide();

                if(pSh && pSh->GetItem(SID_GRADIENT_LIST))
                {
                    mxLbFillAttr->set_sensitive(true);
                    mxLbFillAttr->clear();
                    SvxFillAttrBox::Fill(*mxLbFillAttr, pSh->GetItem(SID_GRADIENT_LIST)->GetGradientList());
                    mxLbFillGradTo->SetNoSelection();
                    mxLbFillGradFrom->SetNoSelection();
                    if (mpFillGradientItem)
                    {
                        const OUString aString(mpFillGradientItem->GetName());
                        mxLbFillAttr->set_active_text(aString);
                        const XGradient aGradient = mpFillGradientItem->GetGradientValue();
                        mxLbFillGradFrom->SelectEntry(aGradient.GetStartColor());
                        mxLbFillGradTo->SelectEntry(aGradient.GetEndColor());
                        mxGradientStyle->set_active(sal::static_int_cast< sal_Int32 >( aGradient.GetGradientStyle() ));
                        if(mxGradientStyle->get_active() == sal_Int32(GradientStyle::Radial))
                            mxMTRAngle->set_sensitive(false);
                        else
                            mxMTRAngle->set_value(aGradient.GetAngle().get() / 10, FieldUnit::DEGREE);
                    }
                    else
                    {
                        mxLbFillAttr->set_active(-1);
                    }
                }
                else
                {
                    mxLbFillAttr->set_active(-1);
                }
                break;
            }
            case eFillStyle::HATCH:
            {
                mxLbFillAttr->show();
                mxLbFillGradFrom->hide();
                mxLbFillGradTo->hide();
                mxMTRAngle->hide();
                mxGradientStyle->hide();
                mxToolBoxColor->hide();
                mxBmpImport->hide();
                mxBmpImport->hide();

                if(pSh && pSh->GetItem(SID_HATCH_LIST))
                {
                    mxLbFillAttr->set_sensitive(true);
                    mxLbFillAttr->clear();
                    SvxFillAttrBox::Fill(*mxLbFillAttr, pSh->GetItem(SID_HATCH_LIST)->GetHatchList());

                    if(mpHatchItem)
                    {
                        const OUString aString(mpHatchItem->GetName());

                        mxLbFillAttr->set_active_text( aString );
                    }
                    else
                    {
                        mxLbFillAttr->set_active(-1);
                    }
                }
                else
                {
                    mxLbFillAttr->set_active(-1);
                }
                break;
            }
            case eFillStyle::BITMAP:
            case eFillStyle::PATTERN:
            {
                mxLbFillAttr->show();
                mxLbFillAttr->set_sensitive(true);
                mxLbFillAttr->clear();
                mxToolBoxColor->hide();
                mxLbFillGradFrom->hide();
                mxLbFillGradTo->hide();
                mxMTRAngle->hide();
                mxGradientStyle->hide();

                if(mpBitmapItem)
                {
                    if(pSh && pSh->GetItem(SID_BITMAP_LIST) && eXFS == BITMAP)
                    {
                        if (!comphelper::LibreOfficeKit::isActive())
                            mxBmpImport->show();
                        mxLbFillType->set_active(sal_uInt32(BITMAP));
                        SvxFillAttrBox::Fill(*mxLbFillAttr, pSh->GetItem(SID_BITMAP_LIST)->GetBitmapList());

                        const OUString aString(mpBitmapItem->GetName());
                        mxLbFillAttr->set_active_text(aString);
                    }
                    else if(pSh && pSh->GetItem(SID_PATTERN_LIST) && eXFS == PATTERN)
                    {
                        mxBmpImport->hide();
                        mxLbFillType->set_active(sal_uInt32(PATTERN));
                        SvxFillAttrBox::Fill(*mxLbFillAttr, pSh->GetItem(SID_PATTERN_LIST)->GetPatternList());

                        const OUString aString(mpBitmapItem->GetName());
                        mxLbFillAttr->set_active_text(aString);
                    }
                }
                else
                    mxLbFillAttr->set_active(-1);
                break;
            }
            default:
                OSL_ENSURE(false, "Non supported FillType (!)");
            break;
        }
        if(mpPanel)
            mpPanel->TriggerDeckLayouting();
}

IMPL_LINK_NOARG(AreaPropertyPanelBase, ModifyTransSliderHdl, weld::Scale&, void)
{
    const sal_uInt16 nVal = mxSldTransparent->get_value();
    SetTransparency(nVal);
    const XFillTransparenceItem aLinearItem(nVal);
    setFillTransparence(aLinearItem);
}

IMPL_LINK_NOARG(AreaPropertyPanelBase, ChangeTrgrTypeHdl_Impl, weld::ComboBox&, void)
{
    sal_Int32 nSelectType = mxLBTransType->get_active();
    bool bGradient = false;
    sal_uInt16 nTrans = 0;

    if(!nSelectType)
    {
        mxBTNGradient->hide();
        mxMTRTransparent->show();
        mxSldTransparent->show();
        mxMTRTransparent->set_sensitive(true);
        mxSldTransparent->set_sensitive(true);
        SetTransparency(0);
    }
    else if(1 == nSelectType)
    {
        mxBTNGradient->hide();
        mxMTRTransparent->show();
        mxSldTransparent->show();
        nTrans = mnLastTransSolid;
        mxMTRTransparent->set_value(nTrans, FieldUnit::PERCENT);
        mxLBTransType->set_active(1);
        mxMTRTransparent->set_sensitive(true);
        mxSldTransparent->set_sensitive(true);
    }
    else
    {
        mxBTNGradient->show();

        switch (nSelectType)
        {
            case 2:
                mxBTNGradient->set_item_icon_name(SIDEBARGRADIENT, maImgLinear);
                break;
            case 3:
                mxBTNGradient->set_item_icon_name(SIDEBARGRADIENT, maImgAxial);
                break;
            case 4:
                mxBTNGradient->set_item_icon_name(SIDEBARGRADIENT, maImgRadial);
                break;
            case 5:
                mxBTNGradient->set_item_icon_name(SIDEBARGRADIENT, maImgElli);
                break;
            case 6:
                mxBTNGradient->set_item_icon_name(SIDEBARGRADIENT, maImgQuad);
                break;
            case 7:
                mxBTNGradient->set_item_icon_name(SIDEBARGRADIENT, maImgSquare);
                break;
        }

        mxMTRTransparent->hide();
        mxSldTransparent->hide();
        mxBTNGradient->set_sensitive(true);
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

IMPL_LINK_NOARG(AreaPropertyPanelBase, ModifyTransparentHdl_Impl, weld::MetricSpinButton&, void)
{
    const sal_uInt16 nTrans = static_cast<sal_uInt16>(mxMTRTransparent->get_value(FieldUnit::PERCENT));
    mnLastTransSolid = nTrans;
    SetTransparency(nTrans);
    const sal_Int32 nSelectType = mxLBTransType->get_active();

    if(nTrans && !nSelectType)
    {
        mxLBTransType->set_active(1);
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
    return mxLBTransType->get_active();
}

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
