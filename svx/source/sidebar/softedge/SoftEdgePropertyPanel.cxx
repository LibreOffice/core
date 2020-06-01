/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include "SoftEdgePropertyPanel.hxx"

#include <sfx2/dispatch.hxx>
#include <svx/sdmetitm.hxx>
#include <svx/svddef.hxx>
#include <svx/svxids.hrc>
#include <svx/xcolit.hxx>

namespace svx::sidebar
{
SoftEdgePropertyPanel::SoftEdgePropertyPanel(vcl::Window* pParent,
                                             const css::uno::Reference<css::frame::XFrame>& rxFrame,
                                             SfxBindings* pBindings)
    : PanelLayout(pParent, "SoftEdgePropertyPanel", "svx/ui/sidebarsoftedge.ui", rxFrame)
    , maSoftEdgeRadiusController(SID_ATTR_SOFTEDGE_RADIUS, *pBindings, *this)
    , mpBindings(pBindings)
    , mxSoftEdgeRadius(m_xBuilder->weld_metric_spin_button("SB_SOFTEDGE_RADIUS", FieldUnit::POINT))
    , mxFTRadius(m_xBuilder->weld_label("radius"))
{
    Initialize();
}

SoftEdgePropertyPanel::~SoftEdgePropertyPanel() { disposeOnce(); }

void SoftEdgePropertyPanel::dispose()
{
    mxFTRadius.reset();
    mxSoftEdgeRadius.reset();
    maSoftEdgeRadiusController.dispose();
    PanelLayout::dispose();
}

void SoftEdgePropertyPanel::Initialize()
{
    mxSoftEdgeRadius->connect_value_changed(
        LINK(this, SoftEdgePropertyPanel, ModifySoftEdgeRadiusHdl));
}

IMPL_LINK_NOARG(SoftEdgePropertyPanel, ModifySoftEdgeRadiusHdl, weld::MetricSpinButton&, void)
{
    SdrMetricItem aItem(SDRATTR_SOFTEDGE_RADIUS, mxSoftEdgeRadius->get_value(FieldUnit::MM_100TH));
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_SOFTEDGE_RADIUS, SfxCallMode::RECORD,
                                             { &aItem });
}

void SoftEdgePropertyPanel::NotifyItemUpdate(sal_uInt16 nSID, SfxItemState eState,
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
    }
}

VclPtr<vcl::Window>
SoftEdgePropertyPanel::Create(vcl::Window* pParent,
                              const css::uno::Reference<css::frame::XFrame>& rxFrame,
                              SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw css::lang::IllegalArgumentException(
            "no parent Window given to SoftEdgePropertyPanel::Create", nullptr, 0);
    if (!rxFrame.is())
        throw css::lang::IllegalArgumentException(
            "no XFrame given to SoftEdgePropertyPanel::Create", nullptr, 1);
    if (pBindings == nullptr)
        throw css::lang::IllegalArgumentException(
            "no SfxBindings given to SoftEdgePropertyPanel::Create", nullptr, 2);

    return VclPtr<SoftEdgePropertyPanel>::Create(pParent, rxFrame, pBindings);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
