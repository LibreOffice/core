/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include "TextEffectPropertyPanel.hxx"

#include <sfx2/dispatch.hxx>
#include <svx/colorbox.hxx>
#include <svx/sdmetitm.hxx>
#include <svx/sdprcitm.hxx>
#include <svx/svddef.hxx>
#include <svx/svxids.hrc>
#include <svx/xcolit.hxx>
#include <svl/itemset.hxx>

namespace svx::sidebar
{
TextEffectPropertyPanel::TextEffectPropertyPanel(weld::Widget* pParent, SfxBindings* pBindings)
    : PanelLayout(pParent, u"TextEffectPropertyPanel"_ustr, u"svx/ui/sidebartexteffect.ui"_ustr)
    , maTGlowColorController(SID_ATTR_GLOW_TEXT_COLOR, *pBindings, *this)
    , maTGlowRadiusController(SID_ATTR_GLOW_TEXT_RADIUS, *pBindings, *this)
    , maTGlowTransparencyController(SID_ATTR_GLOW_TEXT_TRANSPARENCY, *pBindings, *this)
    , mxFTTTransparency(m_xBuilder->weld_label(u"texttransparency"_ustr))
    , mpBindings(pBindings)
    , mxTGlowRadius(
          m_xBuilder->weld_metric_spin_button(u"LB_GLOW_TEXT_RADIUS"_ustr, FieldUnit::POINT))
    , mxLBTGlowColor(new ColorListBox(m_xBuilder->weld_menu_button(u"LB_GLOW_TEXT_COLOR"_ustr),
                                      [this] { return GetFrameWeld(); }))
    , mxTGlowTransparency(m_xBuilder->weld_metric_spin_button(u"LB_GLOW_TEXT_TRANSPARENCY"_ustr,
                                                              FieldUnit::PERCENT))
    , mxFTTColor(m_xBuilder->weld_label(u"glowtextcolorlabel"_ustr))
{
    Initialize();
}

TextEffectPropertyPanel::~TextEffectPropertyPanel()
{
    mxTGlowRadius.reset();
    mxLBTGlowColor.reset();
    mxTGlowTransparency.reset();
    mxFTTColor.reset();
    mxFTTTransparency.reset();

    maTGlowColorController.dispose();
    maTGlowRadiusController.dispose();
    maTGlowTransparencyController.dispose();
}

void TextEffectPropertyPanel::Initialize()
{
    mxTGlowRadius->connect_value_changed(LINK(this, TextEffectPropertyPanel, ModifyTGlowRadiusHdl));
    mxLBTGlowColor->SetSelectHdl(LINK(this, TextEffectPropertyPanel, ModifyTGlowColorHdl));
    mxTGlowTransparency->connect_value_changed(
        LINK(this, TextEffectPropertyPanel, ModifyTGlowTransparencyHdl));
}

IMPL_LINK_NOARG(TextEffectPropertyPanel, ModifyTGlowRadiusHdl, weld::MetricSpinButton&, void)
{
    SdrMetricItem aItem(SDRATTR_GLOW_TEXT_RADIUS, mxTGlowRadius->get_value(FieldUnit::MM_100TH));
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_GLOW_TEXT_RADIUS, SfxCallMode::RECORD,
                                             { &aItem });
}

IMPL_LINK_NOARG(TextEffectPropertyPanel, ModifyTGlowColorHdl, ColorListBox&, void)
{
    XColorItem aItem(SDRATTR_GLOW_TEXT_COLOR, mxLBTGlowColor->GetSelectEntryColor());
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_GLOW_TEXT_COLOR, SfxCallMode::RECORD,
                                             { &aItem });
}

IMPL_LINK_NOARG(TextEffectPropertyPanel, ModifyTGlowTransparencyHdl, weld::MetricSpinButton&, void)
{
    SdrPercentItem aItem(SDRATTR_GLOW_TEXT_TRANSPARENCY,
                         mxTGlowTransparency->get_value(FieldUnit::PERCENT));
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_GLOW_TEXT_TRANSPARENCY, SfxCallMode::RECORD,
                                             { &aItem });
}

void TextEffectPropertyPanel::UpdateControls()
{
    const bool bTEnabled = mxTGlowRadius->get_value(FieldUnit::MM_100TH) != 0;
    mxLBTGlowColor->set_sensitive(bTEnabled);
    mxTGlowTransparency->set_sensitive(bTEnabled);
    mxFTTColor->set_sensitive(bTEnabled);
    mxFTTTransparency->set_sensitive(bTEnabled);
}

void TextEffectPropertyPanel::NotifyItemUpdate(sal_uInt16 nSID, SfxItemState eState,
                                               const SfxPoolItem* pState)
{
    switch (nSID)
    {
        case SID_ATTR_GLOW_TEXT_COLOR:
        {
            if (eState >= SfxItemState::DEFAULT)
            {
                const XColorItem* pColorItem = dynamic_cast<const XColorItem*>(pState);
                if (pColorItem)
                {
                    mxLBTGlowColor->SelectEntry(pColorItem->GetColorValue());
                }
            }
        }
        break;
        case SID_ATTR_GLOW_TEXT_RADIUS:
        {
            if (eState >= SfxItemState::DEFAULT)
            {
                const SdrMetricItem* pRadiusItem = dynamic_cast<const SdrMetricItem*>(pState);
                if (pRadiusItem)
                {
                    mxTGlowRadius->set_value(pRadiusItem->GetValue(), FieldUnit::MM_100TH);
                }
            }
        }
        break;
        case SID_ATTR_GLOW_TEXT_TRANSPARENCY:
        {
            if (eState >= SfxItemState::DEFAULT)
            {
                if (auto pItem = dynamic_cast<const SdrPercentItem*>(pState))
                {
                    mxTGlowTransparency->set_value(pItem->GetValue(), FieldUnit::PERCENT);
                }
            }
        }
        break;
    }
    UpdateControls();
}

std::unique_ptr<PanelLayout> TextEffectPropertyPanel::Create(weld::Widget* pParent,
                                                             SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw css::lang::IllegalArgumentException(
            u"no parent Window given to TextEffectPropertyPanel::Create"_ustr, nullptr, 0);
    if (pBindings == nullptr)
        throw css::lang::IllegalArgumentException(
            u"no SfxBindings given to TextEffectPropertyPanel::Create"_ustr, nullptr, 2);

    return std::make_unique<TextEffectPropertyPanel>(pParent, pBindings);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
