/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <svx/sidebar/EffectPropertyPanelBase.hxx>
#include <vcl/weld/Label.hxx>
#include <vcl/weld/MetricSpinButton.hxx>
#include <vcl/weld/Toggleable.hxx>

class ColorListBox;

namespace svx::sidebar
{
class EffectPropertyPanel : public EffectPropertyPanelBase
{
public:
    EffectPropertyPanel(weld::Widget* pParent, SfxBindings* pBindings);
    ~EffectPropertyPanel() override;

    static std::unique_ptr<PanelLayout> Create(weld::Widget* pParent, SfxBindings* pBindings);

    void setGlowRadius(const SdrMetricItem& rGlowRadius) override;
    void setGlowColor(const XColorItem& rGlowColor) override;
    void setGlowTransparency(const SdrPercentItem& rGlowTransparency) override;
    void setSoftEdgeRadius(const SdrMetricItem& rSoftEdgeRadius) override;

    void NotifyItemUpdate(sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState) override;

private:
    SfxBindings* mpBindings;

    // The reflection widgets come before the controllers, so they exist when a controller
    // reports the first state.
    std::unique_ptr<weld::Frame> mxReflectionFrame;
    std::unique_ptr<weld::CheckButton> mxReflection;
    std::unique_ptr<weld::Label> mxReflectionTransparencyLabel;
    std::unique_ptr<weld::MetricSpinButton> mxReflectionTransparency;
    std::unique_ptr<weld::Scale> mxReflectionTransparencySlider;
    std::unique_ptr<weld::Label> mxReflectionSizeLabel;
    std::unique_ptr<weld::MetricSpinButton> mxReflectionSize;
    std::unique_ptr<weld::Scale> mxReflectionSizeSlider;
    std::unique_ptr<weld::Label> mxReflectionDistanceLabel;
    std::unique_ptr<weld::MetricSpinButton> mxReflectionDistance;
    std::unique_ptr<weld::Label> mxReflectionBlurLabel;
    std::unique_ptr<weld::MetricSpinButton> mxReflectionBlur;

    // False while the selection has no reflection items.
    bool mbReflectionAvailable = true;

    sfx2::sidebar::ControllerItem maGlowColorController;
    sfx2::sidebar::ControllerItem maGlowRadiusController;
    sfx2::sidebar::ControllerItem maGlowTransparencyController;
    sfx2::sidebar::ControllerItem maSoftEdgeRadiusController;
    sfx2::sidebar::ControllerItem maReflectionController;
    sfx2::sidebar::ControllerItem maReflectionTransparencyController;
    sfx2::sidebar::ControllerItem maReflectionSizeController;
    sfx2::sidebar::ControllerItem maReflectionDistanceController;
    sfx2::sidebar::ControllerItem maReflectionBlurController;

    void UpdateReflectionControls();
    void SetReflectionTransparency(sal_uInt16 nTransparency);
    void SetReflectionSize(sal_uInt16 nSize);

    DECL_LINK(ToggleReflectionHdl, weld::Toggleable&, void);
    DECL_LINK(ModifyReflectionTransparencyHdl, weld::MetricSpinButton&, void);
    DECL_LINK(ModifyReflectionTransparencySliderHdl, weld::Scale&, void);
    DECL_LINK(ModifyReflectionSizeHdl, weld::MetricSpinButton&, void);
    DECL_LINK(ModifyReflectionSizeSliderHdl, weld::Scale&, void);
    DECL_LINK(ModifyReflectionDistanceHdl, weld::MetricSpinButton&, void);
    DECL_LINK(ModifyReflectionBlurHdl, weld::MetricSpinButton&, void);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
