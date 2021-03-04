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
#ifndef INCLUDED_SC_SOURCE_UI_SIDEBAR_ALIGNMENTPROPERTYPANEL_HXX
#define INCLUDED_SC_SOURCE_UI_SIDEBAR_ALIGNMENTPROPERTYPANEL_HXX

#include <sfx2/sidebar/ControllerItem.hxx>
#include <sfx2/sidebar/IContextChangeReceiver.hxx>
#include <sfx2/weldutils.hxx>
#include <sfx2/sidebar/PanelLayout.hxx>
#include <vcl/EnumContext.hxx>

namespace sc::sidebar {

class AlignmentPropertyPanel
:   public PanelLayout,
    public ::sfx2::sidebar::IContextChangeReceiver,
    public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    static VclPtr<PanelLayout> Create(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings);

    virtual void DataChanged(
        const DataChangedEvent& rEvent) override;

    virtual void HandleContextChange(
        const vcl::EnumContext& rContext) override;

    virtual void NotifyItemUpdate(
        const sal_uInt16 nSId,
        const SfxItemState eState,
        const SfxPoolItem* pState) override;

    virtual void GetControlState(
        const sal_uInt16 /*nSId*/,
        boost::property_tree::ptree& /*rState*/) override {};

    SfxBindings* GetBindings() { return mpBindings;}

    // constructor/destructor
    AlignmentPropertyPanel(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings);
    virtual ~AlignmentPropertyPanel() override;
    virtual void dispose() override;

private:
    //ui controls
    std::unique_ptr<weld::Label> mxFTLeftIndent;
    std::unique_ptr<weld::MetricSpinButton> mxMFLeftIndent;
    std::unique_ptr<weld::CheckButton> mxCBXWrapText;
    std::unique_ptr<weld::CheckButton> mxCBXMergeCell;
    std::unique_ptr<weld::Label> mxFtRotate;
    std::unique_ptr<weld::MetricSpinButton> mxMtrAngle;
    std::unique_ptr<weld::RadioButton> mxRefEdgeBottom;
    std::unique_ptr<weld::RadioButton> mxRefEdgeTop;
    std::unique_ptr<weld::RadioButton> mxRefEdgeStd;
    std::unique_ptr<weld::CheckButton> mxCBStacked;
    std::unique_ptr<weld::Widget> mxTextOrientBox;

    std::unique_ptr<weld::Toolbar> mxHorizontalAlign;
    std::unique_ptr<ToolbarUnoDispatcher> mxHorizontalAlignDispatch;

    std::unique_ptr<weld::Toolbar> mxVertAlign;
    std::unique_ptr<ToolbarUnoDispatcher> mxVertAlignDispatch;

    std::unique_ptr<weld::Toolbar> mxWriteDirection;
    std::unique_ptr<ToolbarUnoDispatcher> mxWriteDirectionDispatch;

    std::unique_ptr<weld::Toolbar> mxIndentButtons;
    std::unique_ptr<ToolbarUnoDispatcher> mxIndentButtonsDispatch;

    ::sfx2::sidebar::ControllerItem             maAlignHorControl;
    ::sfx2::sidebar::ControllerItem             maLeftIndentControl;
    ::sfx2::sidebar::ControllerItem             maMergeCellControl;
    ::sfx2::sidebar::ControllerItem             maWrapTextControl;
    ::sfx2::sidebar::ControllerItem             maAngleControl;
    ::sfx2::sidebar::ControllerItem             maVrtStackControl;
    ::sfx2::sidebar::ControllerItem             maRefEdgeControl;

    bool                                        mbMultiDisable : 1;
    bool                                        mbSettingToggles : 1;

    vcl::EnumContext                            maContext;
    SfxBindings*                                mpBindings;

    DECL_LINK( MFLeftIndentMdyHdl, weld::MetricSpinButton&, void );
    DECL_LINK( CBOXMergnCellClkHdl, weld::ToggleButton&, void );
    DECL_LINK( CBOXWrapTextClkHdl, weld::ToggleButton&, void );
    DECL_LINK( AngleModifiedHdl, weld::MetricSpinButton&, void );
    DECL_LINK( ClickStackHdl, weld::ToggleButton&, void );
    DECL_LINK( ReferenceEdgeHdl, weld::ToggleButton&, void );

    void Initialize();
};

} // end of namespace ::sc::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
