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
#include <sfx2/sidebar/Layouter.hxx>
#include <GraphicPropertyPanel.hxx>
#include <GraphicPropertyPanel.hrc>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>
#include <svl/intitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include "svx/dialogs.hrc"

using namespace css;
using namespace cssu;
using ::sfx2::sidebar::Layouter;
using ::sfx2::sidebar::Theme;

#define A2S(pString) (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(pString)))

//////////////////////////////////////////////////////////////////////////////


namespace svx { namespace sidebar {

//////////////////////////////////////////////////////////////////////////////

GraphicPropertyPanel::GraphicPropertyPanel(
    Window* pParent,
    const cssu::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
:   Control(
        pParent,
        SVX_RES(RID_SIDEBAR_GRAPHIC_PANEL)),
    mpFtBrightness(new FixedText(this, SVX_RES(FT_BRIGHTNESS))),
    mpMtrBrightness(new MetricField(this, SVX_RES(MTR_BRIGHTNESS))),
    mpFtContrast(new FixedText(this, SVX_RES(FT_CONTRAST))),
    mpMtrContrast(new MetricField(this, SVX_RES(MTR_CONTRAST))),
    mpFtColorMode(new FixedText(this, SVX_RES(FT_COLOR_MODE))),
    mpLBColorMode(new ListBox(this, SVX_RES(LB_COLOR_MODE))),
    mpFtTrans(new FixedText(this, SVX_RES(FT_TRANSPARENT))),
    mpMtrTrans(new MetricField(this, SVX_RES(MTR_TRANSPARENT))),
    mpMtrRed(new MetricField(this, SVX_RES(MF_RED))),
    mpMtrGreen(new MetricField(this, SVX_RES(MF_GREEN))),
    mpMtrBlue(new MetricField(this, SVX_RES(MF_BLUE))),
    mpMtrGamma(new MetricField(this, SVX_RES(MF_GAMMA))),
    maBrightControl(SID_ATTR_GRAF_LUMINANCE, *pBindings, *this),
    maContrastControl(SID_ATTR_GRAF_CONTRAST, *pBindings, *this),
    maTransparenceControl(SID_ATTR_GRAF_TRANSPARENCE, *pBindings, *this),
    maRedControl(SID_ATTR_GRAF_RED, *pBindings, *this),
    maGreenControl(SID_ATTR_GRAF_GREEN, *pBindings, *this),
    maBlueControl(SID_ATTR_GRAF_BLUE, *pBindings, *this),
    maGammaControl(SID_ATTR_GRAF_GAMMA, *pBindings, *this),
    maModeControl(SID_ATTR_GRAF_MODE, *pBindings, *this),
    maImgRed(this, SVX_RES(IMG_RED)),
    maImgGreen(this, SVX_RES(IMG_GREEN)),
    maImgBlue(this, SVX_RES(IMG_BLUE)),
    maImgGamma(this, SVX_RES(IMG_GAMMA)),
    mxFrame(rxFrame),
    mpBindings(pBindings),
    maLayouter(*this)
{
    Initialize();
    FreeResource();

    // Setup the grid layouter.
    maLayouter.GetCell(0,0).SetControl(*mpFtBrightness).SetGridWidth(2);
    maLayouter.GetCell(1,0).SetControl(*mpMtrBrightness).SetGridWidth(2);

    maLayouter.GetCell(0,3).SetControl(*mpFtContrast).SetGridWidth(2);
    maLayouter.GetCell(1,3).SetControl(*mpMtrContrast).SetGridWidth(2);

    maLayouter.GetCell(2,0).SetControl(*mpFtColorMode).SetGridWidth(2);
    maLayouter.GetCell(3,0).SetControl(*mpLBColorMode).SetGridWidth(2);

    maLayouter.GetCell(2,3).SetControl(*mpFtTrans).SetGridWidth(2);
    maLayouter.GetCell(3,3).SetControl(*mpMtrTrans).SetGridWidth(2);

    maLayouter.GetCell(4,0).SetControl(maImgRed).SetFixedWidth();
    maLayouter.GetCell(4,1).SetControl(*mpMtrRed);

    maLayouter.GetCell(5,0).SetControl(maImgBlue).SetFixedWidth();
    maLayouter.GetCell(5,1).SetControl(*mpMtrBlue);

    maLayouter.GetCell(4,3).SetControl(maImgGreen).SetFixedWidth();
    maLayouter.GetCell(4,4).SetControl(*mpMtrGreen);
    maLayouter.GetCell(5,3).SetControl(maImgGamma).SetFixedWidth();
    maLayouter.GetCell(5,4).SetControl(*mpMtrGamma);

    maLayouter.GetColumn(0)
        .SetWeight(0)
        .SetLeftPadding(Layouter::MapWidth(*this,SECTIONPAGE_MARGIN_HORIZONTAL));
    maLayouter.GetColumn(1)
        .SetWeight(1)
        .SetMinimumWidth(Layouter::MapWidth(*this, MBOX_WIDTH - 10));
    maLayouter.GetColumn(2)
        .SetWeight(0)
        .SetMinimumWidth(Layouter::MapWidth(*this, CONTROL_SPACING_HORIZONTAL));
    maLayouter.GetColumn(3)
        .SetWeight(0);
    maLayouter.GetColumn(4)
        .SetWeight(1)
        .SetMinimumWidth(Layouter::MapWidth(*this, MBOX_WIDTH - 10))
        .SetRightPadding(Layouter::MapWidth(*this,SECTIONPAGE_MARGIN_HORIZONTAL));

    // Make controls that display text handle short widths more
    // graceful.
    Layouter::PrepareForLayouting(*mpFtBrightness);
    Layouter::PrepareForLayouting(*mpFtContrast);
    Layouter::PrepareForLayouting(*mpFtColorMode);
    Layouter::PrepareForLayouting(*mpFtTrans);
}

//////////////////////////////////////////////////////////////////////////////

GraphicPropertyPanel::~GraphicPropertyPanel()
{
}

//////////////////////////////////////////////////////////////////////////////

void GraphicPropertyPanel::Initialize()
{
    mpFtBrightness->SetBackground(Wallpaper());
    mpFtContrast->SetBackground(Wallpaper());
    mpFtColorMode->SetBackground(Wallpaper());
    mpFtTrans->SetBackground(Wallpaper());

    mpMtrBrightness->SetModifyHdl( LINK( this, GraphicPropertyPanel, ModifyBrightnessHdl ) );
    mpMtrBrightness->SetAccessibleName(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Brightness")));
    mpMtrContrast->SetModifyHdl( LINK( this, GraphicPropertyPanel, ModifyContrastHdl ) );
    mpMtrContrast->SetAccessibleName(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Contrast")));
    mpMtrTrans->SetModifyHdl( LINK( this, GraphicPropertyPanel, ModifyTransHdl ) );
    mpMtrTrans->SetAccessibleName(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Transparency")));

    mpLBColorMode->InsertEntry(String(SVX_RES(RID_SVXSTR_GRAFMODE_STANDARD)));
    mpLBColorMode->InsertEntry(String(SVX_RES(RID_SVXSTR_GRAFMODE_GREYS)));
    mpLBColorMode->InsertEntry(String(SVX_RES(RID_SVXSTR_GRAFMODE_MONO)));
    mpLBColorMode->InsertEntry(String(SVX_RES(RID_SVXSTR_GRAFMODE_WATERMARK)));
    mpLBColorMode->SetSelectHdl( LINK( this, GraphicPropertyPanel, ClickColorModeHdl ));
    mpLBColorMode->SetAccessibleName(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Color mode")));

    mpMtrRed->SetModifyHdl( LINK( this, GraphicPropertyPanel, RedHdl ) );
    mpMtrGreen->SetModifyHdl( LINK( this, GraphicPropertyPanel, GreenHdl ) );
    mpMtrBlue->SetModifyHdl( LINK( this, GraphicPropertyPanel, BlueHdl ) );
    mpMtrGamma->SetModifyHdl( LINK( this, GraphicPropertyPanel, GammaHdl ) );
    mpMtrRed->SetAccessibleName(mpMtrRed->GetQuickHelpText());
    mpMtrGreen->SetAccessibleName(mpMtrGreen->GetQuickHelpText());
    mpMtrBlue->SetAccessibleName(mpMtrBlue->GetQuickHelpText());
    mpMtrGamma->SetAccessibleName(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Gamma value")));

    mpMtrRed->SetAccessibleRelationLabeledBy(mpMtrRed.get());
    mpMtrGreen->SetAccessibleRelationLabeledBy(mpMtrGreen.get());
    mpMtrBlue->SetAccessibleRelationLabeledBy(mpMtrBlue.get());
    mpMtrGamma->SetAccessibleRelationLabeledBy(mpMtrGamma.get());
    mpMtrBrightness->SetAccessibleRelationLabeledBy(mpFtBrightness.get());
    mpMtrContrast->SetAccessibleRelationLabeledBy(mpFtContrast.get());
    mpMtrTrans->SetAccessibleRelationLabeledBy(mpFtTrans.get());
    mpLBColorMode->SetAccessibleRelationLabeledBy(mpFtColorMode.get());

    // Fix left position of some controls that may be wrong due to
    // rounding errors.
    const sal_Int32 nRight0 (mpLBColorMode->GetPosPixel().X() + mpLBColorMode->GetSizePixel().Width());
    const sal_Int32 nRight1 (mpMtrTrans->GetPosPixel().X() + mpMtrTrans->GetSizePixel().Width());
    mpMtrRed->SetPosPixel(Point(
            nRight0 - mpMtrRed->GetSizePixel().Width(),
            mpMtrRed->GetPosPixel().Y()));
    mpMtrBlue->SetPosPixel(Point(
            nRight0 - mpMtrBlue->GetSizePixel().Width(),
            mpMtrBlue->GetPosPixel().Y()));
    mpMtrGreen->SetPosPixel(Point(
            nRight1 - mpMtrGreen->GetSizePixel().Width(),
            mpMtrGreen->GetPosPixel().Y()));
    mpMtrGamma->SetPosPixel(Point(
            nRight1 - mpMtrGamma->GetSizePixel().Width(),
            mpMtrGamma->GetPosPixel().Y()));
}




//////////////////////////////////////////////////////////////////////////////

IMPL_LINK( GraphicPropertyPanel, ModifyBrightnessHdl, void *, EMPTYARG )
{
    const sal_Int16 nBright = mpMtrBrightness->GetValue();
    const SfxInt16Item aBrightItem( SID_ATTR_GRAF_LUMINANCE, nBright );
    GetBindings()->GetDispatcher()->Execute(SID_ATTR_GRAF_LUMINANCE, SFX_CALLMODE_RECORD, &aBrightItem, 0L);
    return 0L;
}

//////////////////////////////////////////////////////////////////////////////

IMPL_LINK( GraphicPropertyPanel, ModifyContrastHdl, void *, EMPTYARG )
{
    const sal_Int16 nContrast = mpMtrContrast->GetValue();
    const SfxInt16Item aContrastItem( SID_ATTR_GRAF_CONTRAST, nContrast );
    GetBindings()->GetDispatcher()->Execute(SID_ATTR_GRAF_CONTRAST, SFX_CALLMODE_RECORD, &aContrastItem, 0L);
    return 0L;
}

//////////////////////////////////////////////////////////////////////////////

IMPL_LINK( GraphicPropertyPanel, ModifyTransHdl, void *, EMPTYARG )
{
    const sal_Int16 nTrans = mpMtrTrans->GetValue();
    const SfxInt16Item aTransItem( SID_ATTR_GRAF_TRANSPARENCE, nTrans );
    GetBindings()->GetDispatcher()->Execute(SID_ATTR_GRAF_TRANSPARENCE, SFX_CALLMODE_RECORD, &aTransItem, 0L);
    return 0L;
}

//////////////////////////////////////////////////////////////////////////////

IMPL_LINK_NOARG( GraphicPropertyPanel, ClickColorModeHdl )
{
    const sal_Int16 nTrans = mpLBColorMode->GetSelectEntryPos();
    const SfxInt16Item aTransItem( SID_ATTR_GRAF_MODE, nTrans );
    GetBindings()->GetDispatcher()->Execute(SID_ATTR_GRAF_MODE, SFX_CALLMODE_RECORD, &aTransItem, 0L);
    return 0L;
}

//////////////////////////////////////////////////////////////////////////////

IMPL_LINK( GraphicPropertyPanel, RedHdl, void*, EMPTYARG )
{
    const sal_Int16 nRed = mpMtrRed->GetValue();
    const SfxInt16Item aRedItem( SID_ATTR_GRAF_RED, nRed );
    GetBindings()->GetDispatcher()->Execute(SID_ATTR_GRAF_RED, SFX_CALLMODE_RECORD, &aRedItem, 0L);
    return 0L;
}

//////////////////////////////////////////////////////////////////////////////

IMPL_LINK( GraphicPropertyPanel, GreenHdl, void*, EMPTYARG )
{
    const sal_Int16 nGreen = mpMtrGreen->GetValue();
    const SfxInt16Item aGreenItem( SID_ATTR_GRAF_GREEN, nGreen );
    GetBindings()->GetDispatcher()->Execute(SID_ATTR_GRAF_GREEN, SFX_CALLMODE_RECORD, &aGreenItem, 0L);
    return 0L;
}

//////////////////////////////////////////////////////////////////////////////

IMPL_LINK(GraphicPropertyPanel, BlueHdl, void *, EMPTYARG)
{
    const sal_Int16 nBlue = mpMtrBlue->GetValue();
    const SfxInt16Item aBlueItem( SID_ATTR_GRAF_BLUE, nBlue );
    GetBindings()->GetDispatcher()->Execute(SID_ATTR_GRAF_BLUE, SFX_CALLMODE_RECORD, &aBlueItem, 0L);
    return 0L;
}

//////////////////////////////////////////////////////////////////////////////

IMPL_LINK(GraphicPropertyPanel, GammaHdl, void *, EMPTYARG)
{
    const sal_Int32 nGamma = mpMtrGamma->GetValue();
    const SfxInt32Item nGammaItem( SID_ATTR_GRAF_GAMMA, nGamma );
    GetBindings()->GetDispatcher()->Execute(SID_ATTR_GRAF_GAMMA, SFX_CALLMODE_RECORD, &nGammaItem, 0L);
    return 0L;
}

//////////////////////////////////////////////////////////////////////////////

void GraphicPropertyPanel::SetupIcons(void)
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

//////////////////////////////////////////////////////////////////////////////

GraphicPropertyPanel* GraphicPropertyPanel::Create (
    Window* pParent,
    const cssu::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
{
    if (pParent == NULL)
        throw lang::IllegalArgumentException(A2S("no parent Window given to GraphicPropertyPanel::Create"), NULL, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException(A2S("no XFrame given to GraphicPropertyPanel::Create"), NULL, 1);
    if (pBindings == NULL)
        throw lang::IllegalArgumentException(A2S("no SfxBindings given to GraphicPropertyPanel::Create"), NULL, 2);

    return new GraphicPropertyPanel(
        pParent,
        rxFrame,
        pBindings);
}

//////////////////////////////////////////////////////////////////////////////

void GraphicPropertyPanel::DataChanged(
    const DataChangedEvent& rEvent)
{
    (void)rEvent;

    SetupIcons();
}

//////////////////////////////////////////////////////////////////////////////

void GraphicPropertyPanel::NotifyItemUpdate(
    sal_uInt16 nSID,
    SfxItemState eState,
    const SfxPoolItem* pState,
    const bool bIsEnabled)
{
    (void)bIsEnabled;

    switch( nSID )
    {
        case SID_ATTR_GRAF_LUMINANCE:
        {
            if(eState >= SFX_ITEM_AVAILABLE)
            {
                mpMtrBrightness->Enable();
                const SfxInt16Item* pItem = dynamic_cast< const SfxInt16Item* >(pState);

                if(pItem)
                {
                    const sal_Int64 nBright = pItem->GetValue();
                    mpMtrBrightness->SetValue(nBright);
                }
            }
            else if(SFX_ITEM_DISABLED == eState)
            {
                mpMtrBrightness->Disable();
            }
            else
            {
                mpMtrBrightness->Enable();
                mpMtrBrightness->SetText(String());
            }
            break;
        }
        case SID_ATTR_GRAF_CONTRAST:
        {
            if(eState >= SFX_ITEM_AVAILABLE)
            {
                mpMtrContrast->Enable();
                const SfxInt16Item* pItem = dynamic_cast< const SfxInt16Item* >(pState);

                if(pItem)
                {
                    const sal_Int64 nContrast = pItem->GetValue();
                    mpMtrContrast->SetValue(nContrast);
                }
            }
            else if(SFX_ITEM_DISABLED == eState)
            {
                mpMtrContrast->Disable();
            }
            else
            {
                mpMtrContrast->Enable();
                mpMtrContrast->SetText(String());
            }
            break;
        }
        case SID_ATTR_GRAF_TRANSPARENCE:
        {
            if(eState >= SFX_ITEM_AVAILABLE)
            {
                mpMtrTrans->Enable();
                const SfxUInt16Item* pItem = dynamic_cast< const SfxUInt16Item* >(pState);

                if(pItem)
                {
                    const sal_Int64 nTrans = pItem->GetValue();
                    mpMtrTrans->SetValue(nTrans);
                }
            }
            else if(SFX_ITEM_DISABLED == eState)
            {
                mpMtrTrans->Disable();
            }
            else
            {
                mpMtrTrans->Enable();
                mpMtrTrans->SetText(String());
            }
            break;
        }
        case SID_ATTR_GRAF_MODE:
        {
            if(eState >= SFX_ITEM_AVAILABLE)
            {
                mpLBColorMode->Enable();
                const SfxUInt16Item* pItem = dynamic_cast< const SfxUInt16Item* >(pState);

                if(pItem)
                {
                    const sal_Int64 nTrans = pItem->GetValue();
                    mpLBColorMode->SelectEntryPos(nTrans);
                }
            }
            else if(SFX_ITEM_DISABLED == eState)
            {
                mpLBColorMode->Disable();
            }
            else
            {
                mpLBColorMode->Enable();
                mpLBColorMode->SetNoSelection();
            }
            break;
        }
        case SID_ATTR_GRAF_RED:
        {
            if(eState >= SFX_ITEM_AVAILABLE)
            {
                mpMtrRed->Enable();
                const SfxInt16Item* pItem = dynamic_cast< const SfxInt16Item* >(pState);

                if(pItem)
                {
                    const sal_Int64 nRed = pItem->GetValue();
                    mpMtrRed->SetValue( nRed );
                }
            }
            else if(SFX_ITEM_DISABLED == eState)
            {
                mpMtrRed->Disable();
            }
            else
            {
                mpMtrRed->Enable();
                mpMtrRed->SetText(String());
            }
            break;
        }
        case SID_ATTR_GRAF_GREEN:
        {
            if(eState >= SFX_ITEM_AVAILABLE)
            {
                mpMtrGreen->Enable();
                const SfxInt16Item* pItem = dynamic_cast< const SfxInt16Item* >(pState);

                if(pItem)
                {
                    const sal_Int64 nGreen = pItem->GetValue();
                    mpMtrGreen->SetValue( nGreen );
                }
            }
            else if(SFX_ITEM_DISABLED == eState)
            {
                mpMtrGreen->Disable();
            }
            else
            {
                mpMtrGreen->Enable();
                mpMtrGreen->SetText(String());
            }
            break;
        }
        case SID_ATTR_GRAF_BLUE:
        {
            if(eState >= SFX_ITEM_AVAILABLE)
            {
                mpMtrBlue->Enable();
                const SfxInt16Item* pItem = dynamic_cast< const SfxInt16Item* >(pState);

                if(pItem)
                {
                    const sal_Int64 nBlue = pItem->GetValue();
                    mpMtrBlue->SetValue( nBlue );
                }
            }
            else if(SFX_ITEM_DISABLED == eState)
            {
                mpMtrBlue->Disable();
            }
            else
            {
                mpMtrBlue->Enable();
                mpMtrBlue->SetText(String());
            }
            break;
        }
        case SID_ATTR_GRAF_GAMMA:
        {
            if(eState >= SFX_ITEM_AVAILABLE)
            {
                mpMtrGamma->Enable();
                const SfxUInt32Item* pItem = dynamic_cast< const SfxUInt32Item* >(pState);

                if(pItem)
                {
                    const sal_Int64 nGamma = pItem->GetValue();
                    mpMtrGamma->SetValue( nGamma );
                }
            }
            else if(SFX_ITEM_DISABLED == eState)
            {
                mpMtrGamma->Disable();
            }
            else
            {
                mpMtrGamma->Enable();
                mpMtrGamma->SetText(String());
            }
            break;
        }
    }
}




SfxBindings* GraphicPropertyPanel::GetBindings()
{
    return mpBindings;
}




void GraphicPropertyPanel::Resize (void)
{
    maLayouter.Layout();
}



}} // end of namespace ::svx::sidebar
