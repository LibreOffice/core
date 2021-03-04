/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SVX_SOURCE_SIDEBAR_POSSIZE_POSSIZEPROPERTYPANEL_HXX
#define INCLUDED_SVX_SOURCE_SIDEBAR_POSSIZE_POSSIZEPROPERTYPANEL_HXX

#include <sfx2/sidebar/ControllerItem.hxx>
#include <sfx2/sidebar/IContextChangeReceiver.hxx>
#include <sfx2/weldutils.hxx>
#include <sfx2/sidebar/PanelLayout.hxx>
#include <svl/poolitem.hxx>
#include <tools/fldunit.hxx>
#include <tools/fract.hxx>
#include <com/sun/star/ui/XSidebar.hpp>
#include <basegfx/range/b2drange.hxx>
#include <vcl/EnumContext.hxx>
#include <vcl/customweld.hxx>
#include <vcl/weld.hxx>

class SdrView;

namespace svx {
class DialControl;
};

namespace svx::sidebar {

class PosSizePropertyPanel
:   public PanelLayout,
    public ::sfx2::sidebar::IContextChangeReceiver,
    public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    virtual ~PosSizePropertyPanel() override;
    virtual void dispose() override;

    static VclPtr<PanelLayout> Create(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings,
        const css::uno::Reference<css::ui::XSidebar>& rxSidebar);

    virtual void DataChanged(
        const DataChangedEvent& rEvent) override;

    virtual void HandleContextChange(
        const vcl::EnumContext& rContext) override;

    virtual void NotifyItemUpdate(
        const sal_uInt16 nSId,
        const SfxItemState eState,
        const SfxPoolItem* pState) override;

    virtual void DumpAsPropertyTree(tools::JsonWriter&) override;

    SfxBindings* GetBindings() { return mpBindings;}

    // constructor/destructor
    PosSizePropertyPanel(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings,
        const css::uno::Reference<css::ui::XSidebar>& rxSidebar);

    virtual void GetControlState(
        const sal_uInt16 nSId,
        boost::property_tree::ptree& rState) override;

private:
    //Position
    std::unique_ptr<weld::Label> mxFtPosX;
    std::unique_ptr<weld::MetricSpinButton> mxMtrPosX;
    std::unique_ptr<weld::Label> mxFtPosY;
    std::unique_ptr<weld::MetricSpinButton> mxMtrPosY;

    // size
    std::unique_ptr<weld::Label> mxFtWidth;
    std::unique_ptr<weld::MetricSpinButton> mxMtrWidth;
    std::unique_ptr<weld::Label> mxFtHeight;
    std::unique_ptr<weld::MetricSpinButton> mxMtrHeight;
    std::unique_ptr<weld::CheckButton> mxCbxScale;

    //rotation
    std::unique_ptr<weld::Label> mxFtAngle;
    std::unique_ptr<weld::MetricSpinButton> mxMtrAngle;

    //rotation control
    std::unique_ptr<svx::DialControl> mxCtrlDial;
    std::unique_ptr<weld::CustomWeld> mxDial;

    //flip
    std::unique_ptr<weld::Label> mxFtFlip;
    std::unique_ptr<weld::Toolbar> mxFlipTbx;
    std::unique_ptr<ToolbarUnoDispatcher> mxFlipDispatch;

    std::unique_ptr<weld::Toolbar> mxArrangeTbx;
    std::unique_ptr<ToolbarUnoDispatcher> mxArrangeDispatch;
    std::unique_ptr<weld::Toolbar> mxArrangeTbx2;
    std::unique_ptr<ToolbarUnoDispatcher> mxArrangeDispatch2;

    std::unique_ptr<weld::Toolbar> mxAlignTbx;
    std::unique_ptr<ToolbarUnoDispatcher> mxAlignDispatch;
    std::unique_ptr<weld::Toolbar> mxAlignTbx2;
    std::unique_ptr<ToolbarUnoDispatcher> mxAlignDispatch2;

    //edit charts button for online's mobile view
    std::unique_ptr<weld::Button> mxBtnEditChart;

    // Internal variables
    basegfx::B2DRange                       maRect;
    basegfx::B2DRange                       maWorkArea;
    const SdrView*                          mpView;
    sal_uInt32                              mlOldWidth;
    sal_uInt32                              mlOldHeight;
    tools::Long                                    mlRotX;
    tools::Long                                    mlRotY;
    Fraction                                maUIScale;
    MapUnit                                 mePoolUnit;
    FieldUnit                               meDlgUnit;

    // Controller Items
    ::sfx2::sidebar::ControllerItem         maTransfPosXControl;
    ::sfx2::sidebar::ControllerItem         maTransfPosYControl;
    ::sfx2::sidebar::ControllerItem         maTransfWidthControl;
    ::sfx2::sidebar::ControllerItem         maTransfHeightControl;

    ::sfx2::sidebar::ControllerItem         maSvxAngleControl;
    ::sfx2::sidebar::ControllerItem         maRotXControl;
    ::sfx2::sidebar::ControllerItem         maRotYControl;
    ::sfx2::sidebar::ControllerItem         maProPosControl;
    ::sfx2::sidebar::ControllerItem         maProSizeControl;
    ::sfx2::sidebar::ControllerItem         maAutoWidthControl;
    ::sfx2::sidebar::ControllerItem         maAutoHeightControl;
    ::sfx2::sidebar::ControllerItem         m_aMetricCtl;

    vcl::EnumContext                        maContext;
    SfxBindings*                            mpBindings;

    bool                                    mbSizeProtected : 1;
    bool                                    mbPositionProtected : 1;
    bool                                    mbAutoWidth : 1;
    bool                                    mbAutoHeight : 1;
    bool                                    mbAdjustEnabled : 1;

    css::uno::Reference<css::ui::XSidebar> mxSidebar;

    DECL_LINK( ChangePosXHdl, weld::MetricSpinButton&, void );
    DECL_LINK( ChangePosYHdl, weld::MetricSpinButton&, void );
    DECL_LINK( ChangeWidthHdl, weld::MetricSpinButton&, void );
    DECL_LINK( ChangeHeightHdl, weld::MetricSpinButton&, void );
    DECL_LINK( ClickAutoHdl, weld::ToggleButton&, void );
    DECL_LINK( RotationHdl, svx::DialControl&, void );
    DECL_STATIC_LINK( PosSizePropertyPanel, ClickChartEditHdl, weld::Button&, void );

    void Initialize();
    void executeSize();

    void MetricState( SfxItemState eState, const SfxPoolItem* pState );
    static FieldUnit GetCurrentUnit( SfxItemState eState, const SfxPoolItem* pState );
    void DisableControls();
    void SetPosSizeMinMax();

    /** Check if the UI scale has changed and handle such a change.
        UI scale is an SD only feature.  The UI scale is represented by items
        ATTR_OPTIONS_SCALE_X and
        ATTR_OPTIONS_SCALE_Y.
        As we have no direct access (there is no dependency of svx on sd) we have to
        use a small trick (aka hack):
        a) call this method whenever a change of the metric item is notified,
        b) check if the UI scale has changed (strangely, the UI scale value is available at the SdrModel.
        c) invalidate the items for position and size to trigger notifications of their current values.
    */
    void UpdateUIScale();
};


} // end of namespace svx::sidebar


#endif // INCLUDED_SVX_SOURCE_SIDEBAR_POSSIZE_POSSIZEPROPERTYPANEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
