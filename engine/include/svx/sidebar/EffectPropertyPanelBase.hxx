/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sfx2/sidebar/ControllerItem.hxx>
#include <sfx2/sidebar/PanelLayout.hxx>
#include <svx/EffectItems.hxx>
#include <svx/svxdllapi.h>

#include <memory>

class ColorListBox;

namespace svx::sidebar
{
class UNLESS_MERGELIBS(SVX_DLLPUBLIC) EffectPropertyPanelBase
    : public PanelLayout,
      public sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    explicit EffectPropertyPanelBase(weld::Widget* pParent);
    ~EffectPropertyPanelBase() override;

    void NotifyItemUpdate(sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState) override;

    void GetControlState(const sal_uInt16 /*nSId*/,
                         boost::property_tree::ptree& /*rState*/) override
    {
    }

    virtual void setGlow(const SdrGlowEffectItem& rGlow) = 0;
    virtual void setSoftEdge(const SdrSoftEdgeEffectItem& rSoftEdge) = 0;

    void updateGlow(bool bDefaultOrSet, const SfxPoolItem* pState);
    void updateSoftEdge(bool bDefaultOrSet, const SfxPoolItem* pState) const;

private:
    /// The glow of the selection, or none when the selection has no single glow.
    std::unique_ptr<SdrGlowEffectItem> mpGlow;

    std::unique_ptr<weld::Label> mxFTTransparency;
    std::unique_ptr<weld::MetricSpinButton> mxGlowRadius;
    std::unique_ptr<ColorListBox> mxLBGlowColor;
    std::unique_ptr<weld::MetricSpinButton> mxGlowTransparency;
    std::unique_ptr<weld::Label> mxFTColor;
    std::unique_ptr<weld::MetricSpinButton> mxSoftEdgeRadius;

    void Initialize();
    void UpdateControls() const;
    /// The glow that a change of one of its values starts from.
    SdrGlowEffectItem getGlow() const;
    /// Sets the glow on the selection and keeps it as the glow that the next change starts from.
    void changeGlow(const SdrGlowEffectItem& rGlow);

    DECL_DLLPRIVATE_LINK(ModifyGlowColorHdl, ColorListBox&, void);
    DECL_DLLPRIVATE_LINK(ModifyGlowRadiusHdl, weld::MetricSpinButton&, void);
    DECL_DLLPRIVATE_LINK(ModifyGlowTransparencyHdl, weld::MetricSpinButton&, void);
    DECL_DLLPRIVATE_LINK(ModifySoftEdgeRadiusHdl, weld::MetricSpinButton&, void);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
