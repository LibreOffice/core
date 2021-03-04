/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SVX_SOURCE_SIDEBAR_AREA_SHADOWPROPERTYPANEL_HXX
#define INCLUDED_SVX_SOURCE_SIDEBAR_AREA_SHADOWPROPERTYPANEL_HXX

#include <sfx2/sidebar/ControllerItem.hxx>
#include <sfx2/sidebar/PanelLayout.hxx>

class ColorListBox;

namespace svx::sidebar {

class ShadowPropertyPanel
:   public PanelLayout,
    public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    virtual ~ShadowPropertyPanel() override;

    static std::unique_ptr<PanelLayout> Create(
    weld::Widget* pParent,
    SfxBindings* pBindings);

    virtual void NotifyItemUpdate(
        const sal_uInt16 nSId,
        const SfxItemState eState,
        const SfxPoolItem* pState) override;

    virtual void GetControlState(
        const sal_uInt16 /*nSId*/,
        boost::property_tree::ptree& /*rState*/) override {};

    SfxBindings* GetBindings() { return mpBindings;}

    void Initialize();

    ShadowPropertyPanel(
        weld::Widget* pParent,
        SfxBindings* pBindings);

private:
    ::sfx2::sidebar::ControllerItem maShadowController;
    ::sfx2::sidebar::ControllerItem maShadowTransController;
    ::sfx2::sidebar::ControllerItem maShadowBlurController;
    ::sfx2::sidebar::ControllerItem maShadowColorController;
    ::sfx2::sidebar::ControllerItem maShadowXDistanceController;
    ::sfx2::sidebar::ControllerItem maShadowYDistanceController;

    SfxBindings* mpBindings;
    tools::Long nX,nY,nXY;

    std::unique_ptr<weld::CheckButton> mxShowShadow;
    std::unique_ptr<weld::MetricSpinButton> mxShadowDistance;
    std::unique_ptr<ColorListBox> mxLBShadowColor;
    std::unique_ptr<weld::ComboBox> mxShadowAngle;
    std::unique_ptr<weld::Label> mxFTAngle;
    std::unique_ptr<weld::Label> mxFTDistance;
    std::unique_ptr<weld::Label> mxFTTransparency;
    std::unique_ptr<weld::Label> mxFTBlur;
    std::unique_ptr<weld::Label> mxFTColor;
    std::unique_ptr<weld::Scale> mxShadowTransSlider;
    std::unique_ptr<weld::MetricSpinButton> mxShadowTransMetric;
    std::unique_ptr<weld::MetricSpinButton> mxShadowBlurMetric;

    void InsertAngleValues();
    void SetTransparencyValue(tools::Long);
    void UpdateControls();
    void ModifyShadowDistance();

    DECL_LINK(ClickShadowHdl, weld::ToggleButton&, void);
    DECL_LINK(ModifyShadowColorHdl, ColorListBox&, void);
    DECL_LINK(ModifyShadowTransMetricHdl, weld::MetricSpinButton&, void);
    DECL_LINK(ModifyShadowAngleHdl, weld::ComboBox&, void);
    DECL_LINK(ModifyShadowDistanceHdl, weld::MetricSpinButton&, void);
    DECL_LINK(ModifyShadowTransSliderHdl, weld::Scale&, void);
    DECL_LINK(ModifyShadowBlurMetricHdl, weld::MetricSpinButton&, void);
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
