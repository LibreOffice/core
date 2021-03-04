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

#include <sfx2/sidebar/IContextChangeReceiver.hxx>
#include <sfx2/sidebar/SidebarModelUpdate.hxx>
#include <sfx2/sidebar/PanelLayout.hxx>
#include <vcl/EnumContext.hxx>
#include "ChartSidebarModifyListener.hxx"
#include <TitleHelper.hxx>

namespace com::sun::star::util { class XModifyListener; }

namespace chart {

class ChartController;

namespace sidebar {

class ChartElementsPanel : public PanelLayout,
    public ::sfx2::sidebar::IContextChangeReceiver,
    public sfx2::sidebar::SidebarModelUpdate,
    public ChartSidebarModifyListenerParent
{
public:
    static VclPtr<PanelLayout> Create(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        ChartController* pController);

    virtual void DataChanged(
        const DataChangedEvent& rEvent) override;

    virtual void HandleContextChange(
        const vcl::EnumContext& rContext) override;

    // constructor/destructor
    ChartElementsPanel(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        ChartController* pController);

    virtual ~ChartElementsPanel() override;

    virtual void dispose() override;

    virtual void updateData() override;
    virtual void modelInvalid() override;

    virtual void updateModel(css::uno::Reference<css::frame::XModel> xModel) override;

private:
    //ui controls
    std::unique_ptr<weld::CheckButton> mxCBTitle;
    std::unique_ptr<weld::Entry>       mxEditTitle;
    std::unique_ptr<weld::CheckButton> mxCBSubtitle;
    std::unique_ptr<weld::Entry>       mxEditSubtitle;
    std::unique_ptr<weld::CheckButton> mxCBXAxis;
    std::unique_ptr<weld::CheckButton> mxCBXAxisTitle;
    std::unique_ptr<weld::CheckButton> mxCBYAxis;
    std::unique_ptr<weld::CheckButton> mxCBYAxisTitle;
    std::unique_ptr<weld::CheckButton> mxCBZAxis;
    std::unique_ptr<weld::CheckButton> mxCBZAxisTitle;
    std::unique_ptr<weld::CheckButton> mxCB2ndXAxis;
    std::unique_ptr<weld::CheckButton> mxCB2ndXAxisTitle;
    std::unique_ptr<weld::CheckButton> mxCB2ndYAxis;
    std::unique_ptr<weld::CheckButton> mxCB2ndYAxisTitle;
    std::unique_ptr<weld::CheckButton> mxCBLegend;
    std::unique_ptr<weld::CheckButton> mxCBLegendNoOverlay;
    std::unique_ptr<weld::CheckButton> mxCBGridVerticalMajor;
    std::unique_ptr<weld::CheckButton> mxCBGridHorizontalMajor;
    std::unique_ptr<weld::CheckButton> mxCBGridVerticalMinor;
    std::unique_ptr<weld::CheckButton> mxCBGridHorizontalMinor;
    std::unique_ptr<weld::Label> mxTextTitle;
    std::unique_ptr<weld::Label> mxTextSubTitle;
    std::unique_ptr<weld::Label> mxLBAxis;
    std::unique_ptr<weld::Label> mxLBGrid;

    std::unique_ptr<weld::ComboBox> mxLBLegendPosition;
    std::unique_ptr<weld::Widget> mxBoxLegend;

    vcl::EnumContext maContext;

    css::uno::Reference<css::frame::XModel> mxModel;
    css::uno::Reference<css::util::XModifyListener> mxListener;

    bool mbModelValid;

    OUString maTextTitle;
    OUString maTextSubTitle;

    void Initialize();

    void setTitleVisible(TitleHelper::eTitleType eTitle, bool bVisible);

    DECL_LINK(CheckBoxHdl, weld::ToggleButton&, void);
    DECL_LINK(EditHdl, weld::Entry&, void);
    DECL_LINK(LegendPosHdl, weld::ComboBox&, void);
};

} } // end of namespace ::chart::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
