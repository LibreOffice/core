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
#include <svx/svddef.hxx>
#include <svx/svxids.hrc>
#include <svx/xcolit.hxx>
#include <rtl/math.hxx>

namespace
{
sal_Int32 EMU2Pt(sal_Int32 nEMU)
{
    return static_cast<sal_Int32>(rtl::math::round(nEMU / 12700.0));
}
sal_Int32 Pt2EMU(sal_Int32 nPt) { return nPt * 12700; }
}

namespace svx::sidebar
{
GlowPropertyPanel::GlowPropertyPanel(vcl::Window* pParent,
                                     const css::uno::Reference<css::frame::XFrame>& rxFrame,
                                     SfxBindings* pBindings)
    : PanelLayout(pParent, "GlowPropertyPanel", "svx/ui/sidebarglow.ui", rxFrame, true)
    , maGlowController(SID_ATTR_GLOW, *pBindings, *this)
    , maGlowColorController(SID_ATTR_GLOW_COLOR, *pBindings, *this)
    , maGlowRadiusController(SID_ATTR_GLOW_RADIUS, *pBindings, *this)
    , mpBindings(pBindings)
    , mxShowGlow(m_xBuilder->weld_check_button("SHOW_GLOW"))
    , mxGlowRadius(m_xBuilder->weld_metric_spin_button("LB_GLOW_RADIUS", FieldUnit::POINT))
    , mxLBGlowColor(new ColorListBox(m_xBuilder->weld_menu_button("LB_GLOW_COLOR"), GetFrameWeld()))
    , mxFTRadius(m_xBuilder->weld_label("radius"))
    , mxFTColor(m_xBuilder->weld_label("color"))
{
    Initialize();
}

GlowPropertyPanel::~GlowPropertyPanel() { disposeOnce(); }

void GlowPropertyPanel::dispose()
{
    mxShowGlow.reset();
    mxFTRadius.reset();
    mxGlowRadius.reset();
    mxFTColor.reset();
    mxLBGlowColor.reset();

    maGlowController.dispose();
    maGlowColorController.dispose();
    maGlowRadiusController.dispose();
    PanelLayout::dispose();
}

void GlowPropertyPanel::Initialize()
{
    mxShowGlow->set_state(TRISTATE_FALSE);
    mxShowGlow->connect_toggled(LINK(this, GlowPropertyPanel, ClickGlowHdl));
    mxLBGlowColor->SetSelectHdl(LINK(this, GlowPropertyPanel, ModifyGlowColorHdl));
    mxGlowRadius->connect_value_changed(LINK(this, GlowPropertyPanel, ModifyGlowRadiusHdl));
}

IMPL_LINK_NOARG(GlowPropertyPanel, ClickGlowHdl, weld::ToggleButton&, void)
{
    SdrOnOffItem aItem(SDRATTR_GLOW, mxShowGlow->get_state() != TRISTATE_FALSE);
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_GLOW, SfxCallMode::RECORD, { &aItem });
}

IMPL_LINK_NOARG(GlowPropertyPanel, ModifyGlowColorHdl, ColorListBox&, void)
{
    XColorItem aItem(SDRATTR_GLOW_COLOR, mxLBGlowColor->GetSelectEntryColor());
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_GLOW_COLOR, SfxCallMode::RECORD, { &aItem });
}

IMPL_LINK_NOARG(GlowPropertyPanel, ModifyGlowRadiusHdl, weld::MetricSpinButton&, void)
{
    SdrMetricItem aItem(SDRATTR_GLOW_RAD, Pt2EMU(mxGlowRadius->get_value(FieldUnit::POINT)));
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_GLOW_RADIUS, SfxCallMode::RECORD, { &aItem });
}

void GlowPropertyPanel::UpdateControls()
{
    const bool bEnabled = mxShowGlow->get_state() != TRISTATE_FALSE;
    mxGlowRadius->set_sensitive(bEnabled);
    mxLBGlowColor->set_sensitive(bEnabled);
    mxFTRadius->set_sensitive(bEnabled);
    mxFTColor->set_sensitive(bEnabled);
}

void GlowPropertyPanel::NotifyItemUpdate(sal_uInt16 nSID, SfxItemState eState,
                                         const SfxPoolItem* pState)
{
    switch (nSID)
    {
        case SID_ATTR_GLOW:
        {
            if (eState >= SfxItemState::DEFAULT)
            {
                const SdrOnOffItem* pItem = dynamic_cast<const SdrOnOffItem*>(pState);
                if (pItem)
                {
                    if (pItem->GetValue())
                        mxShowGlow->set_state(TRISTATE_TRUE);
                    else
                        mxShowGlow->set_state(TRISTATE_FALSE);
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
                    mxGlowRadius->set_value(EMU2Pt(pRadiusItem->GetValue()), FieldUnit::POINT);
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
