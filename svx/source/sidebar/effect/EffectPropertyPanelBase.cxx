/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <svx/sidebar/EffectPropertyPanelBase.hxx>

#include <svx/colorbox.hxx>
#include <svx/sdmetitm.hxx>
#include <svx/sdprcitm.hxx>
#include <svx/svddef.hxx>
#include <svx/svxids.hrc>
#include <svx/xcolit.hxx>
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

IMPL_LINK_NOARG(EffectPropertyPanelBase, ModifyGlowColorHdl, ColorListBox&, void)
{
    const XColorItem aItem(SDRATTR_GLOW_COLOR, mxLBGlowColor->GetSelectEntryColor());
    setGlowColor(aItem);
}

IMPL_LINK_NOARG(EffectPropertyPanelBase, ModifyGlowRadiusHdl, weld::MetricSpinButton&, void)
{
    const SdrMetricItem aItem(SDRATTR_GLOW_RADIUS, mxGlowRadius->get_value(FieldUnit::MM_100TH));
    setGlowRadius(aItem);
}

IMPL_LINK_NOARG(EffectPropertyPanelBase, ModifyGlowTransparencyHdl, weld::MetricSpinButton&, void)
{
    const SdrPercentItem aItem(SDRATTR_GLOW_TRANSPARENCY,
                               mxGlowTransparency->get_value(FieldUnit::PERCENT));
    setGlowTransparency(aItem);
}

void EffectPropertyPanelBase::UpdateControls() const
{
    const bool bEnabled = mxGlowRadius->get_value(FieldUnit::MM_100TH) != 0;
    mxLBGlowColor->set_sensitive(bEnabled);
    mxGlowTransparency->set_sensitive(bEnabled);
    mxFTColor->set_sensitive(bEnabled);
    mxFTTransparency->set_sensitive(bEnabled);
}

void EffectPropertyPanelBase::updateGlowRadius(const bool bDefaultOrSet,
                                               const SfxPoolItem* pState) const
{
    if (bDefaultOrSet)
    {
        if (const auto* pRadiusItem = dynamic_cast<const SdrMetricItem*>(pState))
        {
            mxGlowRadius->set_value(pRadiusItem->GetValue(), FieldUnit::MM_100TH);
        }
    }
}

void EffectPropertyPanelBase::updateGlowColor(const bool bDefaultOrSet,
                                              const SfxPoolItem* pState) const
{
    if (bDefaultOrSet)
    {
        if (const auto* pColorItem = dynamic_cast<const XColorItem*>(pState))
        {
            mxLBGlowColor->SelectEntry(pColorItem->GetColorValue());
        }
    }
}

void EffectPropertyPanelBase::updateGlowTransparency(const bool bDefaultOrSet,
                                                     const SfxPoolItem* pState) const
{
    if (bDefaultOrSet)
    {
        if (const auto pItem = dynamic_cast<const SdrPercentItem*>(pState))
        {
            mxGlowTransparency->set_value(pItem->GetValue(), FieldUnit::PERCENT);
        }
    }
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
        case SID_ATTR_GLOW_COLOR:
            updateGlowColor(bDefaultOrSet, pState);
            break;
        case SID_ATTR_GLOW_RADIUS:
            updateGlowRadius(bDefaultOrSet, pState);
            break;
        case SID_ATTR_GLOW_TRANSPARENCY:
            updateGlowTransparency(bDefaultOrSet, pState);
            break;
    }
    UpdateControls();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
