/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sfx2/sidebar/ControllerItem.hxx>
#include <sfx2/sidebar/PanelLayout.hxx>
#include <svx/svxdllapi.h>

class XColorItem;
class SdrPercentItem;
class SdrMetricItem;
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

    virtual void setGlowRadius(const SdrMetricItem& rGlowRadius) = 0;
    virtual void setGlowColor(const XColorItem& rGlowColor) = 0;
    virtual void setGlowTransparency(const SdrPercentItem& rGlowTransparency) = 0;
    virtual void setSoftEdgeRadius(const SdrMetricItem& rSoftEdgeRadius) = 0;

    void updateGlowRadius(bool bDefaultOrSet, const SfxPoolItem* pState) const;
    void updateGlowColor(bool bDefaultOrSet, const SfxPoolItem* pState) const;
    void updateGlowTransparency(bool bDefaultOrSet, const SfxPoolItem* pState) const;
    void updateSoftEdgeRadius(bool bDefaultOrSet, const SfxPoolItem* pState) const;

private:
    std::unique_ptr<weld::Label> mxFTTransparency;
    std::unique_ptr<weld::MetricSpinButton> mxGlowRadius;
    std::unique_ptr<ColorListBox> mxLBGlowColor;
    std::unique_ptr<weld::MetricSpinButton> mxGlowTransparency;
    std::unique_ptr<weld::Label> mxFTColor;
    std::unique_ptr<weld::MetricSpinButton> mxSoftEdgeRadius;

    void Initialize();
    void UpdateControls() const;

    DECL_DLLPRIVATE_LINK(ModifyGlowColorHdl, ColorListBox&, void);
    DECL_DLLPRIVATE_LINK(ModifyGlowRadiusHdl, weld::MetricSpinButton&, void);
    DECL_DLLPRIVATE_LINK(ModifyGlowTransparencyHdl, weld::MetricSpinButton&, void);
    DECL_DLLPRIVATE_LINK(ModifySoftEdgeRadiusHdl, weld::MetricSpinButton&, void);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
