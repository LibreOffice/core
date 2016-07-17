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
#ifndef INCLUDED_SC_SOURCE_UI_SIDEBAR_NUMBERFORMATPROPERTYPANEL_HXX
#define INCLUDED_SC_SOURCE_UI_SIDEBAR_NUMBERFORMATPROPERTYPANEL_HXX

#include <sfx2/sidebar/ControllerItem.hxx>
#include <sfx2/sidebar/IContextChangeReceiver.hxx>
#include <svx/sidebar/PanelLayout.hxx>

class FixedText;
class ListBox;
class NumericField;
class Edit;

namespace sc { namespace sidebar {

class NumberFormatPropertyPanel
:   public PanelLayout,
    public ::sfx2::sidebar::IContextChangeReceiver,
    public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
public:
    static VclPtr<vcl::Window> Create(
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
        const SfxPoolItem* pState,
        const bool bIsEnabled) override;

    SfxBindings* GetBindings() { return mpBindings;}

    // constructor/destuctor
    NumberFormatPropertyPanel(
        vcl::Window* pParent,
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings);
    virtual ~NumberFormatPropertyPanel();
    virtual void dispose() override;
private:
    //ui controls
    VclPtr<ListBox>                                mpLbCategory;
    VclPtr<ToolBox>                                mpTBCategory;
    VclPtr<FixedText>                              mpFtDecimals;
    VclPtr<NumericField>                           mpEdDecimals;
    VclPtr<FixedText>                              mpFtDenominator;
    VclPtr<NumericField>                           mpEdDenominator;
    VclPtr<NumericField>                           mpEdLeadZeroes;
    VclPtr<CheckBox>                               mpBtnNegRed;
    VclPtr<CheckBox>                               mpBtnThousand;
    VclPtr<CheckBox>                               mpBtnEngineering;

    ::sfx2::sidebar::ControllerItem         maNumFormatControl;
    ::sfx2::sidebar::ControllerItem         maFormatControl;

    sal_Int32                               mnCategorySelected;

    vcl::EnumContext                        maContext;
    SfxBindings*                            mpBindings;

    DECL_LINK_TYPED(NumFormatSelectHdl, ListBox&, void);
    DECL_LINK_TYPED(NumFormatValueHdl, Edit&, void);
    DECL_LINK_TYPED(NumFormatValueClickHdl, Button*, void);

    void Initialize();
    void DisableControls();
};

} } // end of namespace ::sc::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
