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
#pragma once

#include <sfx2/sidebar/ControllerItem.hxx>
#include <sfx2/sidebar/IContextChangeReceiver.hxx>
#include <sfx2/sidebar/SidebarModelUpdate.hxx>
#include <sfx2/sidebar/PanelLayout.hxx>

#include "ChartSidebarModifyListener.hxx"
#include "ChartSidebarSelectionListener.hxx"

namespace com::sun::star::util { class XModifyListener; }
namespace com::sun::star::view { class XSelectionChangeListener; }

namespace chart {

class ChartController;

namespace sidebar {

class ChartSeriesPanel : public PanelLayout,
    public ::sfx2::sidebar::IContextChangeReceiver,
    public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface,
    public sfx2::sidebar::SidebarModelUpdate,
    public ChartSidebarModifyListenerParent,
    public ChartSidebarSelectionListenerParent
{
public:
    static std::unique_ptr<PanelLayout> Create(
        weld::Widget* pParent,
        ChartController* pController);

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

    // constructor/destructor
    ChartSeriesPanel(
        weld::Widget* pParent,
        ChartController* pController);
    virtual ~ChartSeriesPanel() override;

    virtual void updateData() override;
    virtual void modelInvalid() override;

    virtual void selectionChanged(bool bCorrectType) override;

    virtual void updateModel(css::uno::Reference<css::frame::XModel> xModel) override;

private:
    //ui controls
    std::unique_ptr<weld::CheckButton> mxCBLabel;
    std::unique_ptr<weld::CheckButton> mxCBTrendline;
    std::unique_ptr<weld::CheckButton> mxCBXError;
    std::unique_ptr<weld::CheckButton> mxCBYError;

    std::unique_ptr<weld::RadioButton> mxRBPrimaryAxis;
    std::unique_ptr<weld::RadioButton> mxRBSecondaryAxis;

    std::unique_ptr<weld::Widget> mxBoxLabelPlacement;
    std::unique_ptr<weld::ComboBox> mxLBLabelPlacement;

    std::unique_ptr<weld::Label> mxFTSeriesName;
    std::unique_ptr<weld::Label> mxFTSeriesTemplate;

    rtl::Reference<::chart::ChartModel> mxModel;
    css::uno::Reference<css::util::XModifyListener> mxListener;
    css::uno::Reference<css::view::XSelectionChangeListener> mxSelectionListener;

    bool mbModelValid;

    void Initialize();
    void doUpdateModel(rtl::Reference<::chart::ChartModel> xModel);

    DECL_LINK(CheckBoxHdl, weld::Toggleable&, void);
    DECL_LINK(RadioBtnHdl, weld::Toggleable&, void);
    DECL_LINK(ListBoxHdl, weld::ComboBox&, void);
};

} } // end of namespace ::chart::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
