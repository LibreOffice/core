/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include "EffectPropertyPanel.hxx"

#include <sfx2/dispatch.hxx>
#include <svx/colorbox.hxx>
#include <svx/sdmetitm.hxx>
#include <svx/sdprcitm.hxx>
#include <svx/svddef.hxx>
#include <svx/svxids.hrc>
#include <svx/xcolit.hxx>

namespace svx::sidebar
{
EffectPropertyPanel::EffectPropertyPanel(weld::Widget* pParent, SfxBindings* pBindings)
    : PanelLayout(pParent, "EffectPropertyPanel", "svx/ui/sidebareffect.ui")
    , maGlowColorController(SID_ATTR_GLOW_COLOR, *pBindings, *this)
    , maGlowRadiusController(SID_ATTR_GLOW_RADIUS, *pBindings, *this)
    , maGlowTransparencyController(SID_ATTR_GLOW_TRANSPARENCY, *pBindings, *this)
    , mxFTTransparency(m_xBuilder->weld_label("transparency"))
    , maSoftEdgeRadiusController(SID_ATTR_SOFTEDGE_RADIUS, *pBindings, *this)
    , mpBindings(pBindings)
    , mxGlowRadius(m_xBuilder->weld_metric_spin_button("LB_GLOW_RADIUS", FieldUnit::POINT))
    , mxLBGlowColor(new ColorListBox(m_xBuilder->weld_menu_button("LB_GLOW_COLOR"),
                                     [this] { return GetFrameWeld(); }))
    , mxGlowTransparency(
          m_xBuilder->weld_metric_spin_button("LB_GLOW_TRANSPARENCY", FieldUnit::PERCENT))
    , mxFTRadiusSoftEdge(m_xBuilder->weld_label("radiussoftedge"))
    , mxFTRadiusGlow(m_xBuilder->weld_label("radiusglow"))
    , mxFTColor(m_xBuilder->weld_label("glowcolorlabel"))
    , mxSoftEdgeRadius(m_xBuilder->weld_metric_spin_button("SB_SOFTEDGE_RADIUS", FieldUnit::POINT))
{
    Initialize();
}

EffectPropertyPanel::~EffectPropertyPanel()
{
    mxGlowRadius.reset();
    mxLBGlowColor.reset();
    mxGlowTransparency.reset();
    mxFTRadiusSoftEdge.reset();
    mxFTColor.reset();
    mxFTTransparency.reset();
    mxSoftEdgeRadius.reset();
    mxFTRadiusGlow.reset();

    maGlowColorController.dispose();
    maGlowRadiusController.dispose();
    maGlowTransparencyController.dispose();
    maSoftEdgeRadiusController.dispose();
}

void EffectPropertyPanel::Initialize()
{
    mxGlowRadius->connect_value_changed(LINK(this, EffectPropertyPanel, ModifyGlowRadiusHdl));
    mxLBGlowColor->SetSelectHdl(LINK(this, EffectPropertyPanel, ModifyGlowColorHdl));
    mxGlowTransparency->connect_value_changed(
        LINK(this, EffectPropertyPanel, ModifyGlowTransparencyHdl));
    mxSoftEdgeRadius->connect_value_changed(
        LINK(this, EffectPropertyPanel, ModifySoftEdgeRadiusHdl));
}

IMPL_LINK_NOARG(EffectPropertyPanel, ModifySoftEdgeRadiusHdl, weld::MetricSpinButton&, void)
{
    SdrMetricItem aItem(SDRATTR_SOFTEDGE_RADIUS, mxSoftEdgeRadius->get_value(FieldUnit::MM_100TH));
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_SOFTEDGE_RADIUS, SfxCallMode::RECORD,
                                             { &aItem });
}

IMPL_LINK_NOARG(EffectPropertyPanel, ModifyGlowColorHdl, ColorListBox&, void)
{
    XColorItem aItem(SDRATTR_GLOW_COLOR, mxLBGlowColor->GetSelectEntryColor());
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_GLOW_COLOR, SfxCallMode::RECORD, { &aItem });
}

IMPL_LINK_NOARG(EffectPropertyPanel, ModifyGlowRadiusHdl, weld::MetricSpinButton&, void)
{
    SdrMetricItem aItem(SDRATTR_GLOW_RADIUS, mxGlowRadius->get_value(FieldUnit::MM_100TH));
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_GLOW_RADIUS, SfxCallMode::RECORD, { &aItem });
}

IMPL_LINK_NOARG(EffectPropertyPanel, ModifyGlowTransparencyHdl, weld::MetricSpinButton&, void)
{
    SdrPercentItem aItem(SDRATTR_GLOW_TRANSPARENCY,
                         mxGlowTransparency->get_value(FieldUnit::PERCENT));
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_GLOW_TRANSPARENCY, SfxCallMode::RECORD,
                                             { &aItem });
}

void EffectPropertyPanel::UpdateControls()
{
    const bool bEnabled = mxGlowRadius->get_value(FieldUnit::MM_100TH) != 0;
    mxLBGlowColor->set_sensitive(bEnabled);
    mxGlowTransparency->set_sensitive(bEnabled);
    mxFTColor->set_sensitive(bEnabled);
    mxFTTransparency->set_sensitive(bEnabled);
}

void EffectPropertyPanel::NotifyItemUpdate(sal_uInt16 nSID, SfxItemState eState,
                                           const SfxPoolItem* pState)
{
    switch (nSID)
    {
        case SID_ATTR_SOFTEDGE_RADIUS:
        {
            if (eState >= SfxItemState::DEFAULT)
            {
                const SdrMetricItem* pRadiusItem = dynamic_cast<const SdrMetricItem*>(pState);
                if (pRadiusItem)
                {
                    mxSoftEdgeRadius->set_value(pRadiusItem->GetValue(), FieldUnit::MM_100TH);
                }
            }
        }
        break;
        case SID_ATTR_GLOW_COLOR:
        {
            if (eState >= SfxItemState::DEFAULT)
            {
                const XColorItem* pColorItem = dynamic_cast<const XColorItem*>(pState);
                if (pColorItem)
                {
                    mxLBGlowColor->SelectEntry(pColorItem->GetColorValue());
                }
            }
        }
        break;
        case SID_ATTR_GLOW_RADIUS:
        {
            if (eState >= SfxItemState::DEFAULT)
            {
                const SdrMetricItem* pRadiusItem = dynamic_cast<const SdrMetricItem*>(pState);
                if (pRadiusItem)
                {
                    mxGlowRadius->set_value(pRadiusItem->GetValue(), FieldUnit::MM_100TH);
                }
            }
        }
        break;
        case SID_ATTR_GLOW_TRANSPARENCY:
        {
            if (eState >= SfxItemState::DEFAULT)
            {
                if (auto pItem = dynamic_cast<const SdrPercentItem*>(pState))
                {
                    mxGlowTransparency->set_value(pItem->GetValue(), FieldUnit::PERCENT);
                }
            }
        }
        break;
    }
    UpdateControls();
}

std::unique_ptr<PanelLayout> EffectPropertyPanel::Create(weld::Widget* pParent,
                                                         SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw css::lang::IllegalArgumentException(
            "no parent Window given to EffectPropertyPanel::Create", nullptr, 0);
    if (pBindings == nullptr)
        throw css::lang::IllegalArgumentException(
            "no SfxBindings given to EffectPropertyPanel::Create", nullptr, 2);

    return std::make_unique<EffectPropertyPanel>(pParent, pBindings);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
