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

#include <vcl/slider.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/vclptr.hxx>
#include <sfx2/sidebar/SidebarPanelBase.hxx>
#include <vcl/ctrl.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>
#include <svx/sidebar/PanelLayout.hxx>
#include <svl/intitem.hxx>
#include <vcl/field.hxx>
#include <com/sun/star/ui/XUIElement.hpp>
#include <sfx2/request.hxx>
#include <svx/dlgctrl.hxx>

class SvxColorListBox;

namespace svx { namespace sidebar {

class ShadowPropertyPanel
:   public PanelLayout,
    public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    virtual ~ShadowPropertyPanel() override;
    virtual void dispose() override;

    static VclPtr<vcl::Window> Create(
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings);

    virtual void DataChanged(
    const DataChangedEvent& rEvent) override;

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
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings);

private:
    VclPtr<CheckBox>      mpShowShadow;
    VclPtr<MetricBox>     mpShadowDistance;
    VclPtr<SvxColorListBox> mpLBShadowColor;
    VclPtr<MetricBox>     mpShadowAngle;
    VclPtr<FixedText>     mpFTAngle;
    VclPtr<FixedText>     mpFTDistance;
    VclPtr<FixedText>     mpFTTransparency;
    VclPtr<FixedText>     mpFTColor;
    VclPtr<Slider>        mpShadowTransSlider;
    VclPtr<MetricField>   mpShadowTransMetric;

    ::sfx2::sidebar::ControllerItem maShadowController;
    ::sfx2::sidebar::ControllerItem maShadowTransController;
    ::sfx2::sidebar::ControllerItem maShadowColorController;
    ::sfx2::sidebar::ControllerItem maShadowXDistanceController;
    ::sfx2::sidebar::ControllerItem maShadowYDistanceController;

    SfxBindings* const mpBindings;
    long nX,nY,nXY;

    void InsertAngleValues();
    void SetTransparencyValue(long);
    void UpdateControls();
    DECL_LINK(ClickShadowHdl, Button*, void);
    DECL_LINK(ModifyShadowColorHdl, SvxColorListBox&, void);
    DECL_LINK(ModifyShadowTransMetricHdl, Edit&, void);
    DECL_LINK(ModifyShadowDistanceHdl, Edit&, void);
    DECL_LINK(ModifyShadowTransSliderHdl, Slider*, void);
};
}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
