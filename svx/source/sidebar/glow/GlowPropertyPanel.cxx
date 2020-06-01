/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include "GlowPropertyPanel.hxx"

#include <sfx2/dispatch.hxx>
#include <svx/colorbox.hxx>
#include <svx/sdmetitm.hxx>
#include <svx/sdooitm.hxx>
#include <svx/sdprcitm.hxx>
#include <svx/svddef.hxx>
#include <svx/svxids.hrc>
#include <svx/xcolit.hxx>
#include <rtl/math.hxx>

namespace svx::sidebar
{
GlowPropertyPanel::GlowPropertyPanel(vcl::Window* pParent,
                                     const css::uno::Reference<css::frame::XFrame>& rxFrame,
                                     SfxBindings* pBindings)
    : PanelLayout(pParent, "GlowPropertyPanel", "svx/ui/sidebarglow.ui", rxFrame)
    , maGlowColorController(SID_ATTR_GLOW_COLOR, *pBindings, *this)
    , maGlowRadiusController(SID_ATTR_GLOW_RADIUS, *pBindings, *this)
    , maGlowTransparencyController(SID_ATTR_GLOW_TRANSPARENCY, *pBindings, *this)
    , mpBindings(pBindings)
    , mxGlowRadius(m_xBuilder->weld_metric_spin_button("LB_GLOW_RADIUS", FieldUnit::POINT))
    , mxLBGlowColor(new ColorListBox(m_xBuilder->weld_menu_button("LB_GLOW_COLOR"), GetFrameWeld()))
    , mxGlowTransparency(
          m_xBuilder->weld_metric_spin_button("LB_GLOW_TRANSPARENCY", FieldUnit::PERCENT))
    , mxFTRadius(m_xBuilder->weld_label("radius"))
    , mxFTColor(m_xBuilder->weld_label("color"))
    , mxFTTransparency(m_xBuilder->weld_label("transparency"))
{
    Initialize();
}

GlowPropertyPanel::~GlowPropertyPanel() { disposeOnce(); }

void GlowPropertyPanel::dispose()
{
    mxFTRadius.reset();
    mxGlowRadius.reset();
    mxFTColor.reset();
    mxLBGlowColor.reset();
    mxFTTransparency.reset();
    mxGlowTransparency.reset();

    maGlowColorController.dispose();
    maGlowRadiusController.dispose();
    maGlowTransparencyController.dispose();
    PanelLayout::dispose();
}

void GlowPropertyPanel::Initialize()
{
    mxLBGlowColor->SetSelectHdl(LINK(this, GlowPropertyPanel, ModifyGlowColorHdl));
    mxGlowRadius->connect_value_changed(LINK(this, GlowPropertyPanel, ModifyGlowRadiusHdl));
    mxGlowTransparency->connect_value_changed(
        LINK(this, GlowPropertyPanel, ModifyGlowTransparencyHdl));
}

IMPL_LINK_NOARG(GlowPropertyPanel, ModifyGlowColorHdl, ColorListBox&, void)
{
    XColorItem aItem(SDRATTR_GLOW_COLOR, mxLBGlowColor->GetSelectEntryColor());
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_GLOW_COLOR, SfxCallMode::RECORD, { &aItem });
}

IMPL_LINK_NOARG(GlowPropertyPanel, ModifyGlowRadiusHdl, weld::MetricSpinButton&, void)
{
    SdrMetricItem aItem(SDRATTR_GLOW_RADIUS, mxGlowRadius->get_value(FieldUnit::MM_100TH));
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_GLOW_RADIUS, SfxCallMode::RECORD, { &aItem });
}

IMPL_LINK_NOARG(GlowPropertyPanel, ModifyGlowTransparencyHdl, weld::MetricSpinButton&, void)
{
    SdrPercentItem aItem(SDRATTR_GLOW_TRANSPARENCY,
                         mxGlowTransparency->get_value(FieldUnit::PERCENT));
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_GLOW_TRANSPARENCY, SfxCallMode::RECORD,
                                             { &aItem });
}

void GlowPropertyPanel::UpdateControls()
{
    const bool bEnabled = mxGlowRadius->get_value(FieldUnit::MM_100TH) != 0;
    mxLBGlowColor->set_sensitive(bEnabled);
    mxGlowTransparency->set_sensitive(bEnabled);
    mxFTColor->set_sensitive(bEnabled);
    mxFTTransparency->set_sensitive(bEnabled);
}

void GlowPropertyPanel::NotifyItemUpdate(sal_uInt16 nSID, SfxItemState eState,
                                         const SfxPoolItem* pState)
{
    switch (nSID)
    {
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

VclPtr<vcl::Window>
GlowPropertyPanel::Create(vcl::Window* pParent,
                          const css::uno::Reference<css::frame::XFrame>& rxFrame,
                          SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw css::lang::IllegalArgumentException(
            "no parent Window given to GlowPropertyPanel::Create", nullptr, 0);
    if (!rxFrame.is())
        throw css::lang::IllegalArgumentException("no XFrame given to GlowPropertyPanel::Create",
                                                  nullptr, 1);
    if (pBindings == nullptr)
        throw css::lang::IllegalArgumentException(
            "no SfxBindings given to GlowPropertyPanel::Create", nullptr, 2);

    return VclPtr<GlowPropertyPanel>::Create(pParent, rxFrame, pBindings);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
