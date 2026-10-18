/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <svx/sidebar/EffectPropertyPanelBase.hxx>

#include <svx/EffectItems.hxx>
#include <svx/colorbox.hxx>
#include <svx/sdmetitm.hxx>
#include <svx/svddef.hxx>
#include <svx/svxids.hrc>
#include <svl/itemset.hxx>

namespace svx::sidebar
{
EffectPropertyPanelBase::EffectPropertyPanelBase(weld::Widget* pParent)
    : PanelLayout(pParent, u"EffectPropertyPanel"_ustr, u"svx/ui/sidebareffect.ui"_ustr)
    , mxFTTransparency(m_xBuilder->weld_label(u"transparency"_ustr))
    , mxGlowRadius(m_xBuilder->weld_metric_spin_button(u"LB_GLOW_RADIUS"_ustr, FieldUnit::POINT))
    , mxLBGlowColor(new ColorListBox(m_xBuilder->weld_menu_button(u"LB_GLOW_COLOR"_ustr),
                                     [this] { return GetFrameWeld(); }))
    , mxGlowTransparency(
          m_xBuilder->weld_metric_spin_button(u"LB_GLOW_TRANSPARENCY"_ustr, FieldUnit::PERCENT))
    , mxFTColor(m_xBuilder->weld_label(u"glowcolorlabel"_ustr))
    , mxSoftEdgeRadius(
          m_xBuilder->weld_metric_spin_button(u"SB_SOFTEDGE_RADIUS"_ustr, FieldUnit::POINT))
{
    Initialize();
}

EffectPropertyPanelBase::~EffectPropertyPanelBase()
{
    mxGlowRadius.reset();
    mxLBGlowColor.reset();
    mxGlowTransparency.reset();
    mxFTColor.reset();
    mxFTTransparency.reset();
    mxSoftEdgeRadius.reset();
}

void EffectPropertyPanelBase::Initialize()
{
    mxGlowRadius->connect_value_changed(LINK(this, EffectPropertyPanelBase, ModifyGlowRadiusHdl));
    mxLBGlowColor->SetSelectHdl(LINK(this, EffectPropertyPanelBase, ModifyGlowColorHdl));
    mxGlowTransparency->connect_value_changed(
        LINK(this, EffectPropertyPanelBase, ModifyGlowTransparencyHdl));
    mxSoftEdgeRadius->connect_value_changed(
        LINK(this, EffectPropertyPanelBase, ModifySoftEdgeRadiusHdl));
}

IMPL_LINK_NOARG(EffectPropertyPanelBase, ModifySoftEdgeRadiusHdl, weld::MetricSpinButton&, void)
{
    SdrMetricItem aItem(SDRATTR_SOFTEDGE_RADIUS, mxSoftEdgeRadius->get_value(FieldUnit::MM_100TH));
    setSoftEdgeRadius(aItem);
}

SdrGlowEffectItem EffectPropertyPanelBase::getGlow() const
{
    if (mpGlow)
        return *mpGlow;
    return SdrGlowEffectItem(SDRATTR_GLOW);
}

void EffectPropertyPanelBase::changeGlow(const SdrGlowEffectItem& rGlow)
{
    // The next change starts from this one, also before the state of the selection comes back.
    mpGlow.reset(rGlow.Clone());
    setGlow(rGlow);
}

IMPL_LINK_NOARG(EffectPropertyPanelBase, ModifyGlowColorHdl, ColorListBox&, void)
{
    SdrGlowEffectItem aGlow(getGlow());
    aGlow.setColor(mxLBGlowColor->GetSelectEntryColor());
    changeGlow(aGlow);
}

IMPL_LINK_NOARG(EffectPropertyPanelBase, ModifyGlowRadiusHdl, weld::MetricSpinButton&, void)
{
    SdrGlowEffectItem aGlow(getGlow());
    aGlow.setRadius(gfx::Length::hmm(mxGlowRadius->get_value(FieldUnit::MM_100TH)));
    changeGlow(aGlow);
}

IMPL_LINK_NOARG(EffectPropertyPanelBase, ModifyGlowTransparencyHdl, weld::MetricSpinButton&, void)
{
    SdrGlowEffectItem aGlow(getGlow());
    aGlow.setTransparency(mxGlowTransparency->get_value(FieldUnit::PERCENT));
    changeGlow(aGlow);
}

void EffectPropertyPanelBase::UpdateControls() const
{
    const bool bEnabled = mxGlowRadius->get_value(FieldUnit::MM_100TH) != 0;
    mxLBGlowColor->set_sensitive(bEnabled);
    mxGlowTransparency->set_sensitive(bEnabled);
    mxFTColor->set_sensitive(bEnabled);
    mxFTTransparency->set_sensitive(bEnabled);
}

void EffectPropertyPanelBase::updateGlow(const bool bDefaultOrSet, const SfxPoolItem* pState)
{
    const auto* pGlow = bDefaultOrSet ? dynamic_cast<const SdrGlowEffectItem*>(pState) : nullptr;
    if (!pGlow)
    {
        mpGlow.reset();
        return;
    }

    mpGlow.reset(pGlow->Clone());
    mxGlowRadius->set_value(pGlow->getEffect().maRadius.as_hmm<sal_Int64>(), FieldUnit::MM_100TH);
    mxLBGlowColor->SelectEntry(pGlow->getColor());
    mxGlowTransparency->set_value(pGlow->getTransparency(), FieldUnit::PERCENT);
}

void EffectPropertyPanelBase::updateSoftEdgeRadius(const bool bDefaultOrSet,
                                                   const SfxPoolItem* pState) const
{
    if (bDefaultOrSet)
    {
        if (const auto* pRadiusItem = dynamic_cast<const SdrMetricItem*>(pState))
        {
            mxSoftEdgeRadius->set_value(pRadiusItem->GetValue(), FieldUnit::MM_100TH);
        }
    }
}

void EffectPropertyPanelBase::NotifyItemUpdate(const sal_uInt16 nSID, const SfxItemState eState,
                                               const SfxPoolItem* pState)
{
    const bool bDefaultOrSet(SfxItemState::DEFAULT <= eState);
    switch (nSID)
    {
        case SID_ATTR_SOFTEDGE_RADIUS:
            updateSoftEdgeRadius(bDefaultOrSet, pState);
            break;
        case SID_ATTR_GLOW:
            updateGlow(bDefaultOrSet, pState);
            break;
    }
    UpdateControls();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
