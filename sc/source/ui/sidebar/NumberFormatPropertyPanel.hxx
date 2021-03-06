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
#include <sfx2/weldutils.hxx>
#include <sfx2/sidebar/PanelLayout.hxx>
#include <vcl/EnumContext.hxx>

namespace sc::sidebar {

class NumberFormatPropertyPanel
:   public PanelLayout,
    public ::sfx2::sidebar::IContextChangeReceiver,
    public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
public:
    static std::unique_ptr<PanelLayout> Create(
        weld::Widget* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings);

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
    NumberFormatPropertyPanel(
        weld::Widget* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings);
    virtual ~NumberFormatPropertyPanel() override;
private:
    //ui controls
    std::unique_ptr<weld::ComboBox> mxLbCategory;
    std::unique_ptr<weld::Toolbar> mxTBCategory;
    std::unique_ptr<ToolbarUnoDispatcher> mxCategoryDispatch;
    std::unique_ptr<weld::Label> mxFtDecimals;
    std::unique_ptr<weld::SpinButton> mxEdDecimals;
    std::unique_ptr<weld::Label> mxFtDenominator;
    std::unique_ptr<weld::SpinButton> mxEdDenominator;
    std::unique_ptr<weld::Label> mxFtLeadZeroes;
    std::unique_ptr<weld::SpinButton> mxEdLeadZeroes;
    std::unique_ptr<weld::CheckButton> mxBtnNegRed;
    std::unique_ptr<weld::CheckButton> mxBtnThousand;
    std::unique_ptr<weld::CheckButton> mxBtnEngineering;

    ::sfx2::sidebar::ControllerItem         maNumFormatControl;
    ::sfx2::sidebar::ControllerItem         maFormatControl;

    sal_Int32                               mnCategorySelected;

    vcl::EnumContext                        maContext;
    SfxBindings*                            mpBindings;

    DECL_LINK(NumFormatSelectHdl, weld::ComboBox&, void);
    DECL_LINK(NumFormatValueHdl, weld::SpinButton&, void);
    DECL_LINK(NumFormatValueClickHdl, weld::ToggleButton&, void);

    void Initialize();
    void DisableControls();
};

} // end of namespace ::sc::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
