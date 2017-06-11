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
#include <sfx2/sidebar/ControlFactory.hxx>
#include <GraphicPropertyPanel.hxx>
#include <svx/strings.hrc>
#include <svx/svxids.hrc>
#include <svx/dialmgr.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>
#include <svl/intitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>

using namespace css;
using namespace css::uno;


// namespace open

namespace svx { namespace sidebar {


GraphicPropertyPanel::GraphicPropertyPanel(
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
:   PanelLayout(pParent, "GraphicPropertyPanel", "svx/ui/sidebargraphic.ui", rxFrame),
    maBrightControl(SID_ATTR_GRAF_LUMINANCE, *pBindings, *this),
    maContrastControl(SID_ATTR_GRAF_CONTRAST, *pBindings, *this),
    maTransparenceControl(SID_ATTR_GRAF_TRANSPARENCE, *pBindings, *this),
    maRedControl(SID_ATTR_GRAF_RED, *pBindings, *this),
    maGreenControl(SID_ATTR_GRAF_GREEN, *pBindings, *this),
    maBlueControl(SID_ATTR_GRAF_BLUE, *pBindings, *this),
    maGammaControl(SID_ATTR_GRAF_GAMMA, *pBindings, *this),
    maModeControl(SID_ATTR_GRAF_MODE, *pBindings, *this),
    mpBindings(pBindings)
{
    get(mpMtrBrightness, "setbrightness");
    get(mpMtrContrast, "setcontrast");
    get(mpLBColorMode, "setcolormode");
    mpLBColorMode->set_width_request(mpLBColorMode->get_preferred_size().Width());
    get(mpMtrTrans, "settransparency");
    get(mpMtrRed, "setred");
    get(mpMtrGreen, "setgreen");
    get(mpMtrBlue, "setblue");
    get(mpMtrGamma, "setgamma");
    Initialize();
}

GraphicPropertyPanel::~GraphicPropertyPanel()
{
    disposeOnce();
}

void GraphicPropertyPanel::dispose()
{
    mpMtrBrightness.clear();
    mpMtrContrast.clear();
    mpLBColorMode.clear();
    mpMtrTrans.clear();
    mpMtrRed.clear();
    mpMtrGreen.clear();
    mpMtrBlue.clear();
    mpMtrGamma.clear();

    maBrightControl.dispose();
    maContrastControl.dispose();
    maTransparenceControl.dispose();
    maRedControl.dispose();
    maGreenControl.dispose();
    maBlueControl.dispose();
    maGammaControl.dispose();
    maModeControl.dispose();

    PanelLayout::dispose();
}


void GraphicPropertyPanel::Initialize()
{
    mpMtrBrightness->SetModifyHdl( LINK( this, GraphicPropertyPanel, ModifyBrightnessHdl ) );
    mpMtrContrast->SetModifyHdl( LINK( this, GraphicPropertyPanel, ModifyContrastHdl ) );
    mpMtrTrans->SetModifyHdl( LINK( this, GraphicPropertyPanel, ModifyTransHdl ) );

    mpLBColorMode->InsertEntry(SvxResId(RID_SVXSTR_GRAFMODE_STANDARD));
    mpLBColorMode->InsertEntry(SvxResId(RID_SVXSTR_GRAFMODE_GREYS));
    mpLBColorMode->InsertEntry(SvxResId(RID_SVXSTR_GRAFMODE_MONO));
    mpLBColorMode->InsertEntry(SvxResId(RID_SVXSTR_GRAFMODE_WATERMARK));
    mpLBColorMode->SetSelectHdl( LINK( this, GraphicPropertyPanel, ClickColorModeHdl ));

    mpMtrRed->SetModifyHdl( LINK( this, GraphicPropertyPanel, RedHdl ) );
    mpMtrGreen->SetModifyHdl( LINK( this, GraphicPropertyPanel, GreenHdl ) );
    mpMtrBlue->SetModifyHdl( LINK( this, GraphicPropertyPanel, BlueHdl ) );
    mpMtrGamma->SetModifyHdl( LINK( this, GraphicPropertyPanel, GammaHdl ) );

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


IMPL_LINK_NOARG( GraphicPropertyPanel, ModifyBrightnessHdl, Edit&, void )
{
    const sal_Int16 nBright = mpMtrBrightness->GetValue();
    const SfxInt16Item aBrightItem( SID_ATTR_GRAF_LUMINANCE, nBright );
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_GRAF_LUMINANCE,
            SfxCallMode::RECORD, { &aBrightItem });
}


IMPL_LINK_NOARG( GraphicPropertyPanel, ModifyContrastHdl, Edit&, void )
{
    const sal_Int16 nContrast = mpMtrContrast->GetValue();
    const SfxInt16Item aContrastItem( SID_ATTR_GRAF_CONTRAST, nContrast );
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_GRAF_CONTRAST,
            SfxCallMode::RECORD, { &aContrastItem });
}


IMPL_LINK_NOARG( GraphicPropertyPanel, ModifyTransHdl, Edit&, void )
{
    const sal_Int16 nTrans = mpMtrTrans->GetValue();
    const SfxInt16Item aTransItem( SID_ATTR_GRAF_TRANSPARENCE, nTrans );
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_GRAF_TRANSPARENCE,
            SfxCallMode::RECORD, { &aTransItem });
}


IMPL_LINK_NOARG( GraphicPropertyPanel, ClickColorModeHdl, ListBox&, void )
{
    const sal_Int16 nTrans = mpLBColorMode->GetSelectEntryPos();
    const SfxInt16Item aTransItem( SID_ATTR_GRAF_MODE, nTrans );
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_GRAF_MODE,
            SfxCallMode::RECORD, { &aTransItem });
}


IMPL_LINK_NOARG( GraphicPropertyPanel, RedHdl, Edit&, void )
{
    const sal_Int16 nRed = mpMtrRed->GetValue();
    const SfxInt16Item aRedItem( SID_ATTR_GRAF_RED, nRed );
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_GRAF_RED,
            SfxCallMode::RECORD, { &aRedItem });
}


IMPL_LINK_NOARG( GraphicPropertyPanel, GreenHdl, Edit&, void )
{
    const sal_Int16 nGreen = mpMtrGreen->GetValue();
    const SfxInt16Item aGreenItem( SID_ATTR_GRAF_GREEN, nGreen );
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_GRAF_GREEN,
            SfxCallMode::RECORD, { &aGreenItem });
}


IMPL_LINK_NOARG(GraphicPropertyPanel, BlueHdl, Edit&, void)
{
    const sal_Int16 nBlue = mpMtrBlue->GetValue();
    const SfxInt16Item aBlueItem( SID_ATTR_GRAF_BLUE, nBlue );
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_GRAF_BLUE,
            SfxCallMode::RECORD, { &aBlueItem });
}


IMPL_LINK_NOARG(GraphicPropertyPanel, GammaHdl, Edit&, void)
{
    const sal_Int32 nGamma = mpMtrGamma->GetValue();
    const SfxInt32Item nGammaItem( SID_ATTR_GRAF_GAMMA, nGamma );
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_GRAF_GAMMA,
            SfxCallMode::RECORD, { &nGammaItem });
}


VclPtr<vcl::Window> GraphicPropertyPanel::Create (
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to GraphicPropertyPanel::Create", nullptr, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to GraphicPropertyPanel::Create", nullptr, 1);
    if (pBindings == nullptr)
        throw lang::IllegalArgumentException("no SfxBindings given to GraphicPropertyPanel::Create", nullptr, 2);

    return VclPtr<GraphicPropertyPanel>::Create(
                pParent,
                rxFrame,
                pBindings);
}


void GraphicPropertyPanel::DataChanged(
    const DataChangedEvent& /*rEvent*/)
{
}


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
            if(eState >= SfxItemState::DEFAULT)
            {
                mpMtrBrightness->Enable();
                const SfxInt16Item* pItem = dynamic_cast< const SfxInt16Item* >(pState);

                if(pItem)
                {
                    const sal_Int64 nBright = pItem->GetValue();
                    mpMtrBrightness->SetValue(nBright);
                }
            }
            else if(SfxItemState::DISABLED == eState)
            {
                mpMtrBrightness->Disable();
            }
            else
            {
                mpMtrBrightness->Enable();
                mpMtrBrightness->SetText(OUString());
            }
            break;
        }
        case SID_ATTR_GRAF_CONTRAST:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                mpMtrContrast->Enable();
                const SfxInt16Item* pItem = dynamic_cast< const SfxInt16Item* >(pState);

                if(pItem)
                {
                    const sal_Int64 nContrast = pItem->GetValue();
                    mpMtrContrast->SetValue(nContrast);
                }
            }
            else if(SfxItemState::DISABLED == eState)
            {
                mpMtrContrast->Disable();
            }
            else
            {
                mpMtrContrast->Enable();
                mpMtrContrast->SetText(OUString());
            }
            break;
        }
        case SID_ATTR_GRAF_TRANSPARENCE:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                mpMtrTrans->Enable();
                const SfxUInt16Item* pItem = dynamic_cast< const SfxUInt16Item* >(pState);

                if(pItem)
                {
                    const sal_Int64 nTrans = pItem->GetValue();
                    mpMtrTrans->SetValue(nTrans);
                }
            }
            else if(SfxItemState::DISABLED == eState)
            {
                mpMtrTrans->Disable();
            }
            else
            {
                mpMtrTrans->Enable();
                mpMtrTrans->SetText(OUString());
            }
            break;
        }
        case SID_ATTR_GRAF_MODE:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                mpLBColorMode->Enable();
                const SfxUInt16Item* pItem = dynamic_cast< const SfxUInt16Item* >(pState);

                if(pItem)
                {
                    const sal_Int64 nTrans = pItem->GetValue();
                    mpLBColorMode->SelectEntryPos(nTrans);
                }
            }
            else if(SfxItemState::DISABLED == eState)
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
            if(eState >= SfxItemState::DEFAULT)
            {
                mpMtrRed->Enable();
                const SfxInt16Item* pItem = dynamic_cast< const SfxInt16Item* >(pState);

                if(pItem)
                {
                    const sal_Int64 nRed = pItem->GetValue();
                    mpMtrRed->SetValue( nRed );
                }
            }
            else if(SfxItemState::DISABLED == eState)
            {
                mpMtrRed->Disable();
            }
            else
            {
                mpMtrRed->Enable();
                mpMtrRed->SetText(OUString());
            }
            break;
        }
        case SID_ATTR_GRAF_GREEN:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                mpMtrGreen->Enable();
                const SfxInt16Item* pItem = dynamic_cast< const SfxInt16Item* >(pState);

                if(pItem)
                {
                    const sal_Int64 nGreen = pItem->GetValue();
                    mpMtrGreen->SetValue( nGreen );
                }
            }
            else if(SfxItemState::DISABLED == eState)
            {
                mpMtrGreen->Disable();
            }
            else
            {
                mpMtrGreen->Enable();
                mpMtrGreen->SetText(OUString());
            }
            break;
        }
        case SID_ATTR_GRAF_BLUE:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                mpMtrBlue->Enable();
                const SfxInt16Item* pItem = dynamic_cast< const SfxInt16Item* >(pState);

                if(pItem)
                {
                    const sal_Int64 nBlue = pItem->GetValue();
                    mpMtrBlue->SetValue( nBlue );
                }
            }
            else if(SfxItemState::DISABLED == eState)
            {
                mpMtrBlue->Disable();
            }
            else
            {
                mpMtrBlue->Enable();
                mpMtrBlue->SetText(OUString());
            }
            break;
        }
        case SID_ATTR_GRAF_GAMMA:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                mpMtrGamma->Enable();
                const SfxUInt32Item* pItem = dynamic_cast< const SfxUInt32Item* >(pState);

                if(pItem)
                {
                    const sal_Int64 nGamma = pItem->GetValue();
                    mpMtrGamma->SetValue( nGamma );
                }
            }
            else if(SfxItemState::DISABLED == eState)
            {
                mpMtrGamma->Disable();
            }
            else
            {
                mpMtrGamma->Enable();
                mpMtrGamma->SetText(OUString());
            }
            break;
        }
    }
}


// namespace close

}} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
