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

#include "GraphicPropertyPanel.hxx"
#include <svx/strings.hrc>
#include <svx/svxids.hrc>
#include <svx/dialmgr.hxx>
#include <svl/intitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <svl/itemset.hxx>

using namespace css;
using namespace css::uno;


// namespace open

namespace svx::sidebar {


GraphicPropertyPanel::GraphicPropertyPanel(
    weld::Widget* pParent,
    SfxBindings* pBindings)
:   PanelLayout(pParent, u"GraphicPropertyPanel"_ustr, u"svx/ui/sidebargraphic.ui"_ustr),
    maBrightControl(SID_ATTR_GRAF_LUMINANCE, *pBindings, *this),
    maContrastControl(SID_ATTR_GRAF_CONTRAST, *pBindings, *this),
    maTransparenceControl(SID_ATTR_GRAF_TRANSPARENCE, *pBindings, *this),
    maRedControl(SID_ATTR_GRAF_RED, *pBindings, *this),
    maGreenControl(SID_ATTR_GRAF_GREEN, *pBindings, *this),
    maBlueControl(SID_ATTR_GRAF_BLUE, *pBindings, *this),
    maGammaControl(SID_ATTR_GRAF_GAMMA, *pBindings, *this),
    maModeControl(SID_ATTR_GRAF_MODE, *pBindings, *this),
    mpBindings(pBindings),
    mxMtrBrightness(m_xBuilder->weld_metric_spin_button(u"setbrightness"_ustr, FieldUnit::PERCENT)),
    mxMtrContrast(m_xBuilder->weld_metric_spin_button(u"setcontrast"_ustr, FieldUnit::PERCENT)),
    mxLBColorMode(m_xBuilder->weld_combo_box(u"setcolormode"_ustr)),
    mxMtrTrans(m_xBuilder->weld_metric_spin_button(u"setgraphtransparency"_ustr, FieldUnit::PERCENT))
{
    mxLBColorMode->set_size_request(mxLBColorMode->get_preferred_size().Width(), -1);
    Initialize();
}

GraphicPropertyPanel::~GraphicPropertyPanel()
{
    mxMtrBrightness.reset();
    mxMtrContrast.reset();
    mxLBColorMode.reset();
    mxMtrTrans.reset();

    maBrightControl.dispose();
    maContrastControl.dispose();
    maTransparenceControl.dispose();
    maRedControl.dispose();
    maGreenControl.dispose();
    maBlueControl.dispose();
    maGammaControl.dispose();
    maModeControl.dispose();
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
    const SfxUInt16Item aTransItem( SID_ATTR_GRAF_TRANSPARENCE, nTrans );
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_GRAF_TRANSPARENCE,
            SfxCallMode::RECORD, { &aTransItem });
}


IMPL_LINK_NOARG( GraphicPropertyPanel, ClickColorModeHdl, weld::ComboBox&, void )
{
    const sal_Int16 nTrans = mxLBColorMode->get_active();
    const SfxUInt16Item aTransItem( SID_ATTR_GRAF_MODE, nTrans );
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_GRAF_MODE,
            SfxCallMode::RECORD, { &aTransItem });
}

std::unique_ptr<PanelLayout> GraphicPropertyPanel::Create (
    weld::Widget* pParent,
    SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException(u"no parent Window given to GraphicPropertyPanel::Create"_ustr, nullptr, 0);
    if (pBindings == nullptr)
        throw lang::IllegalArgumentException(u"no SfxBindings given to GraphicPropertyPanel::Create"_ustr, nullptr, 2);

    return std::make_unique<GraphicPropertyPanel>(pParent, pBindings);
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

                if(pState)
                {
                    const sal_uInt16 nTrans = static_cast< const SfxUInt16Item* >(pState)->GetValue();
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
    }
}

// namespace close

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
