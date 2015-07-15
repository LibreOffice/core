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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_SIDEBAR_CHARTELEMENTSPANEL_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_SIDEBAR_CHARTELEMENTSPANEL_HXX

#include <sfx2/sidebar/ControllerItem.hxx>
#include <sfx2/sidebar/IContextChangeReceiver.hxx>
#include <svx/sidebar/PanelLayout.hxx>

#include <com/sun/star/util/XModifyListener.hpp>

class FixedText;
class ListBox;
class NumericField;

namespace chart {

class ChartController;

namespace sidebar {

class ChartElementsPanel : public PanelLayout,
    public ::sfx2::sidebar::IContextChangeReceiver,
    public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    static VclPtr<vcl::Window> Create(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings, ChartController* pController);

    virtual void DataChanged(
        const DataChangedEvent& rEvent) SAL_OVERRIDE;

    virtual void HandleContextChange(
        const ::sfx2::sidebar::EnumContext& rContext) SAL_OVERRIDE;

    virtual void NotifyItemUpdate(
        const sal_uInt16 nSId,
        const SfxItemState eState,
        const SfxPoolItem* pState,
        const bool bIsEnabled) SAL_OVERRIDE;

    SfxBindings* GetBindings() { return mpBindings;}

    // constructor/destuctor
    ChartElementsPanel(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings, ChartController* pController);
    virtual ~ChartElementsPanel();
    virtual void dispose() SAL_OVERRIDE;

    void updateData();
    void modelInvalid();

private:
    //ui controls
    VclPtr<CheckBox> mpCBTitle;
    VclPtr<CheckBox> mpCBSubtitle;
    VclPtr<CheckBox> mpCBXAxis;
    VclPtr<CheckBox> mpCBXAxisTitle;
    VclPtr<CheckBox> mpCBYAxis;
    VclPtr<CheckBox> mpCBYAxisTitle;
    VclPtr<CheckBox> mpCBZAxis;
    VclPtr<CheckBox> mpCBZAxisTitle;
    VclPtr<CheckBox> mpCB2ndXAxis;
    VclPtr<CheckBox> mpCB2ndXAxisTitle;
    VclPtr<CheckBox> mpCB2ndYAxis;
    VclPtr<CheckBox> mpCB2ndYAxisTitle;
    VclPtr<CheckBox> mpCBLegend;
    VclPtr<CheckBox> mpCBGridVertical;
    VclPtr<CheckBox> mpCBGridHorizontal;
    VclPtr<CheckBox> mpCBShowLabel;
    VclPtr<CheckBox> mpCBTrendline;

    css::uno::Reference<css::frame::XFrame> mxFrame;
    ::sfx2::sidebar::EnumContext            maContext;
    SfxBindings*                            mpBindings;

    css::uno::Reference<css::frame::XModel> mxModel;
    css::uno::Reference<css::util::XModifyListener> mxListener;

    void Initialize();

    DECL_LINK(CheckBoxHdl, CheckBox*);
};

} } // end of namespace ::chart::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
