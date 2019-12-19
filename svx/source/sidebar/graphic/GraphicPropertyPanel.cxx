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
#include "GraphicPropertyPanel.hxx"
#include <svx/strings.hrc>
#include <svx/svxids.hrc>
#include <svx/dialmgr.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>
#include <svl/intitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

using namespace css;
using namespace css::uno;


// namespace open

namespace svx { namespace sidebar {


GraphicPropertyPanel::GraphicPropertyPanel(
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
:   PanelLayout(pParent, "GraphicPropertyPanel", "svx/ui/sidebargraphic.ui", rxFrame, true),
    maBrightControl(SID_ATTR_GRAF_LUMINANCE, *pBindings, *this),
    maContrastControl(SID_ATTR_GRAF_CONTRAST, *pBindings, *this),
    maTransparenceControl(SID_ATTR_GRAF_TRANSPARENCE, *pBindings, *this),
    maRedControl(SID_ATTR_GRAF_RED, *pBindings, *this),
    maGreenControl(SID_ATTR_GRAF_GREEN, *pBindings, *this),
    maBlueControl(SID_ATTR_GRAF_BLUE, *pBindings, *this),
    maGammaControl(SID_ATTR_GRAF_GAMMA, *pBindings, *this),
    maModeControl(SID_ATTR_GRAF_MODE, *pBindings, *this),
    mpBindings(pBindings),
    mxMtrBrightness(m_xBuilder->weld_metric_spin_button("setbrightness", FieldUnit::PERCENT)),
    mxMtrContrast(m_xBuilder->weld_metric_spin_button("setcontrast", FieldUnit::PERCENT)),
    mxLBColorMode(m_xBuilder->weld_combo_box("setcolormode")),
    mxMtrTrans(m_xBuilder->weld_metric_spin_button("setgraphtransparency", FieldUnit::PERCENT)),
    mxMtrRed(m_xBuilder->weld_metric_spin_button("setred", FieldUnit::PERCENT)),
    mxMtrGreen(m_xBuilder->weld_metric_spin_button("setgreen", FieldUnit::PERCENT)),
    mxMtrBlue(m_xBuilder->weld_metric_spin_button("setblue", FieldUnit::PERCENT)),
    mxMtrGamma(m_xBuilder->weld_spin_button("setgamma"))
{
    mxLBColorMode->set_size_request(mxLBColorMode->get_preferred_size().Width(), -1);
    Initialize();
}

GraphicPropertyPanel::~GraphicPropertyPanel()
{
    disposeOnce();
}

void GraphicPropertyPanel::dispose()
{
    mxMtrBrightness.reset();
    mxMtrContrast.reset();
    mxLBColorMode.reset();
    mxMtrTrans.reset();
    mxMtrRed.reset();
    mxMtrGreen.reset();
    mxMtrBlue.reset();
    mxMtrGamma.reset();

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
    mxMtrBrightness->connect_value_changed( LINK( this, GraphicPropertyPanel, ModifyBrightnessHdl ) );
    mxMtrContrast->connect_value_changed( LINK( this, GraphicPropertyPanel, ModifyContrastHdl ) );
    mxMtrTrans->connect_value_changed( LINK( this, GraphicPropertyPanel, ModifyTransHdl ) );

    mxLBColorMode->append_text(SvxResId(RID_SVXSTR_GRAFMODE_STANDARD));
    mxLBColorMode->append_text(SvxResId(RID_SVXSTR_GRAFMODE_GREYS));
    mxLBColorMode->append_text(SvxResId(RID_SVXSTR_GRAFMODE_MONO));
    mxLBColorMode->append_text(SvxResId(RID_SVXSTR_GRAFMODE_WATERMARK));
    mxLBColorMode->connect_changed( LINK( this, GraphicPropertyPanel, ClickColorModeHdl ));

    mxMtrRed->connect_value_changed( LINK( this, GraphicPropertyPanel, RedHdl ) );
    mxMtrGreen->connect_value_changed( LINK( this, GraphicPropertyPanel, GreenHdl ) );
    mxMtrBlue->connect_value_changed( LINK( this, GraphicPropertyPanel, BlueHdl ) );
    mxMtrGamma->connect_value_changed( LINK( this, GraphicPropertyPanel, GammaHdl ) );
}

IMPL_LINK_NOARG( GraphicPropertyPanel, ModifyBrightnessHdl, weld::MetricSpinButton&, void )
{
    const sal_Int16 nBright = mxMtrBrightness->get_value(FieldUnit::PERCENT);
    const SfxInt16Item aBrightItem( SID_ATTR_GRAF_LUMINANCE, nBright );
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_GRAF_LUMINANCE,
            SfxCallMode::RECORD, { &aBrightItem });
}


IMPL_LINK_NOARG( GraphicPropertyPanel, ModifyContrastHdl, weld::MetricSpinButton&, void )
{
    const sal_Int16 nContrast = mxMtrContrast->get_value(FieldUnit::PERCENT);
    const SfxInt16Item aContrastItem( SID_ATTR_GRAF_CONTRAST, nContrast );
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_GRAF_CONTRAST,
            SfxCallMode::RECORD, { &aContrastItem });
}


IMPL_LINK_NOARG( GraphicPropertyPanel, ModifyTransHdl, weld::MetricSpinButton&, void )
{
    const sal_Int16 nTrans = mxMtrTrans->get_value(FieldUnit::PERCENT);
    const SfxInt16Item aTransItem( SID_ATTR_GRAF_TRANSPARENCE, nTrans );
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_GRAF_TRANSPARENCE,
            SfxCallMode::RECORD, { &aTransItem });
}


IMPL_LINK_NOARG( GraphicPropertyPanel, ClickColorModeHdl, weld::ComboBox&, void )
{
    const sal_Int16 nTrans = mxLBColorMode->get_active();
    const SfxInt16Item aTransItem( SID_ATTR_GRAF_MODE, nTrans );
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_GRAF_MODE,
            SfxCallMode::RECORD, { &aTransItem });
}


IMPL_LINK_NOARG( GraphicPropertyPanel, RedHdl, weld::MetricSpinButton&, void )
{
    const sal_Int16 nRed = mxMtrRed->get_value(FieldUnit::PERCENT);
    const SfxInt16Item aRedItem( SID_ATTR_GRAF_RED, nRed );
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_GRAF_RED,
            SfxCallMode::RECORD, { &aRedItem });
}


IMPL_LINK_NOARG( GraphicPropertyPanel, GreenHdl, weld::MetricSpinButton&, void )
{
    const sal_Int16 nGreen = mxMtrGreen->get_value(FieldUnit::PERCENT);
    const SfxInt16Item aGreenItem( SID_ATTR_GRAF_GREEN, nGreen );
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_GRAF_GREEN,
            SfxCallMode::RECORD, { &aGreenItem });
}


IMPL_LINK_NOARG(GraphicPropertyPanel, BlueHdl, weld::MetricSpinButton&, void)
{
    const sal_Int16 nBlue = mxMtrBlue->get_value(FieldUnit::PERCENT);
    const SfxInt16Item aBlueItem( SID_ATTR_GRAF_BLUE, nBlue );
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_GRAF_BLUE,
            SfxCallMode::RECORD, { &aBlueItem });
}


IMPL_LINK_NOARG(GraphicPropertyPanel, GammaHdl, weld::SpinButton&, void)
{
    const sal_Int32 nGamma = mxMtrGamma->get_value();
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
    const SfxPoolItem* pState)
{
    switch( nSID )
    {
        case SID_ATTR_GRAF_LUMINANCE:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                mxMtrBrightness->set_sensitive(true);
                const SfxInt16Item* pItem = dynamic_cast< const SfxInt16Item* >(pState);

                if(pItem)
                {
                    const sal_Int64 nBright = pItem->GetValue();
                    mxMtrBrightness->set_value(nBright, FieldUnit::PERCENT);
                }
            }
            else if(SfxItemState::DISABLED == eState)
            {
                mxMtrBrightness->set_sensitive(false);
            }
            else
            {
                mxMtrBrightness->set_sensitive(true);
                mxMtrBrightness->set_text(OUString());
            }
            break;
        }
        case SID_ATTR_GRAF_CONTRAST:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                mxMtrContrast->set_sensitive(true);
                const SfxInt16Item* pItem = dynamic_cast< const SfxInt16Item* >(pState);

                if(pItem)
                {
                    const sal_Int64 nContrast = pItem->GetValue();
                    mxMtrContrast->set_value(nContrast, FieldUnit::PERCENT);
                }
            }
            else if(SfxItemState::DISABLED == eState)
            {
                mxMtrContrast->set_sensitive(false);
            }
            else
            {
                mxMtrContrast->set_sensitive(true);
                mxMtrContrast->set_text(OUString());
            }
            break;
        }
        case SID_ATTR_GRAF_TRANSPARENCE:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                mxMtrTrans->set_sensitive(true);
                const SfxUInt16Item* pItem = dynamic_cast< const SfxUInt16Item* >(pState);

                if(pItem)
                {
                    const sal_Int64 nTrans = pItem->GetValue();
                    mxMtrTrans->set_value(nTrans, FieldUnit::PERCENT);
                }
            }
            else if(SfxItemState::DISABLED == eState)
            {
                mxMtrTrans->set_sensitive(false);
            }
            else
            {
                mxMtrTrans->set_sensitive(true);
                mxMtrTrans->set_text(OUString());
            }
            break;
        }
        case SID_ATTR_GRAF_MODE:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                mxLBColorMode->set_sensitive(true);
                const SfxUInt16Item* pItem = dynamic_cast< const SfxUInt16Item* >(pState);

                if(pItem)
                {
                    const sal_Int64 nTrans = pItem->GetValue();
                    mxLBColorMode->set_active(nTrans);
                }
            }
            else if(SfxItemState::DISABLED == eState)
            {
                mxLBColorMode->set_sensitive(false);
            }
            else
            {
                mxLBColorMode->set_sensitive(true);
                mxLBColorMode->set_active(-1);
            }
            break;
        }
        case SID_ATTR_GRAF_RED:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                mxMtrRed->set_sensitive(true);
                const SfxInt16Item* pItem = dynamic_cast< const SfxInt16Item* >(pState);

                if(pItem)
                {
                    const sal_Int64 nRed = pItem->GetValue();
                    mxMtrRed->set_value(nRed, FieldUnit::PERCENT);
                }
            }
            else if(SfxItemState::DISABLED == eState)
            {
                mxMtrRed->set_sensitive(false);
            }
            else
            {
                mxMtrRed->set_sensitive(true);
                mxMtrRed->set_text(OUString());
            }
            break;
        }
        case SID_ATTR_GRAF_GREEN:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                mxMtrGreen->set_sensitive(true);
                const SfxInt16Item* pItem = dynamic_cast< const SfxInt16Item* >(pState);

                if(pItem)
                {
                    const sal_Int64 nGreen = pItem->GetValue();
                    mxMtrGreen->set_value(nGreen, FieldUnit::PERCENT);
                }
            }
            else if(SfxItemState::DISABLED == eState)
            {
                mxMtrGreen->set_sensitive(false);
            }
            else
            {
                mxMtrGreen->set_sensitive(true);
                mxMtrGreen->set_text(OUString());
            }
            break;
        }
        case SID_ATTR_GRAF_BLUE:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                mxMtrBlue->set_sensitive(true);
                const SfxInt16Item* pItem = dynamic_cast< const SfxInt16Item* >(pState);

                if(pItem)
                {
                    const sal_Int64 nBlue = pItem->GetValue();
                    mxMtrBlue->set_value(nBlue, FieldUnit::PERCENT);
                }
            }
            else if(SfxItemState::DISABLED == eState)
            {
                mxMtrBlue->set_sensitive(false);
            }
            else
            {
                mxMtrBlue->set_sensitive(true);
                mxMtrBlue->set_text(OUString());
            }
            break;
        }
        case SID_ATTR_GRAF_GAMMA:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                mxMtrGamma->set_sensitive(true);
                const SfxUInt32Item* pItem = dynamic_cast< const SfxUInt32Item* >(pState);

                if(pItem)
                {
                    const sal_Int64 nGamma = pItem->GetValue();
                    mxMtrGamma->set_value( nGamma );
                }
            }
            else if(SfxItemState::DISABLED == eState)
            {
                mxMtrGamma->set_sensitive(false);
            }
            else
            {
                mxMtrGamma->set_sensitive(true);
                mxMtrGamma->set_text(OUString());
            }
            break;
        }
    }
}


// namespace close

}} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
