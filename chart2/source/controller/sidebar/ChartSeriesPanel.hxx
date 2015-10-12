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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_SIDEBAR_CHARTSERIESPANEL_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_SIDEBAR_CHARTSERIESPANEL_HXX

#include <sfx2/sidebar/ControllerItem.hxx>
#include <sfx2/sidebar/IContextChangeReceiver.hxx>
#include <sfx2/sidebar/SidebarModelUpdate.hxx>
#include <svx/sidebar/PanelLayout.hxx>
#include <vcl/layout.hxx>

#include "ChartSidebarModifyListener.hxx"
#include "ChartSidebarSelectionListener.hxx"

#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/view/XSelectionChangeListener.hpp>

class FixedText;
class ListBox;
class NumericField;

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
    static VclPtr<vcl::Window> Create(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        ChartController* pController);

    virtual void DataChanged(
        const DataChangedEvent& rEvent) override;

    virtual void HandleContextChange(
        const ::sfx2::sidebar::EnumContext& rContext) override;

    virtual void NotifyItemUpdate(
        const sal_uInt16 nSId,
        const SfxItemState eState,
        const SfxPoolItem* pState,
        const bool bIsEnabled) override;

    // constructor/destuctor
    ChartSeriesPanel(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        ChartController* pController);
    virtual ~ChartSeriesPanel();
    virtual void dispose() override;

    virtual void updateData() override;
    virtual void modelInvalid() override;

    virtual void selectionChanged(bool bCorrectType) override;
    virtual void SelectionInvalid() override;

    virtual void updateModel(css::uno::Reference<css::frame::XModel> xModel) override;

private:
    //ui controls
    VclPtr<CheckBox> mpCBLabel;
    VclPtr<CheckBox> mpCBTrendline;
    VclPtr<CheckBox> mpCBXError;
    VclPtr<CheckBox> mpCBYError;

    VclPtr<RadioButton> mpRBPrimaryAxis;
    VclPtr<RadioButton> mpRBSecondaryAxis;

    VclPtr<VclHBox> mpBoxLabelPlacement;
    VclPtr<ListBox> mpLBLabelPlacement;

    VclPtr<FixedText> mpFTSeriesName;
    VclPtr<FixedText> mpFTSeriesTemplate;

    css::uno::Reference<css::frame::XFrame> mxFrame;

    css::uno::Reference<css::frame::XModel> mxModel;
    css::uno::Reference<css::util::XModifyListener> mxListener;
    css::uno::Reference<css::view::XSelectionChangeListener> mxSelectionListener;

    bool mbModelValid;

    void Initialize();

    DECL_LINK_TYPED(CheckBoxHdl, Button*, void);
    DECL_LINK_TYPED(RadioBtnHdl, RadioButton&, void);
    DECL_LINK_TYPED(ListBoxHdl, ListBox&, void);
};

} } // end of namespace ::chart::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
