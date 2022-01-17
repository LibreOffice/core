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
#ifndef INCLUDED_SW_SOURCE_UIBASE_SIDEBAR_PAGEFOOTERPANEL_HXX
#define INCLUDED_SW_SOURCE_UIBASE_SIDEBAR_PAGEFOOTERPANEL_HXX

#include <memory>

#include <sfx2/sidebar/PanelLayout.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>
#include <svl/intitem.hxx>
#include <svl/poolitem.hxx>
#include <svl/eitem.hxx>
#include <svx/rulritem.hxx>

namespace sw::sidebar {

class PageFooterPanel :
    public PanelLayout,
    public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
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

    SfxBindings* GetBindings() const { return mpBindings; }
    PageFooterPanel(
        weld::Widget* pParent,
        SfxBindings* pBindings);
    virtual ~PageFooterPanel() override;

private:

    SfxBindings* mpBindings;

    ::sfx2::sidebar::ControllerItem maHFToggleController;
    ::sfx2::sidebar::ControllerItem maMetricController;
    ::sfx2::sidebar::ControllerItem maFooterLRMarginController;
    ::sfx2::sidebar::ControllerItem maFooterSpacingController;
    ::sfx2::sidebar::ControllerItem maFooterLayoutController;

    FieldUnit meFUnit;

    OUString m_aCustomEntry;

    void Initialize();
    void SetMarginsAndSpacingFieldUnit();
    void UpdateFooterCheck();
    void UpdateMarginControl();
    void UpdateSpacingControl();
    void UpdateLayoutControl();

    ::std::unique_ptr<SfxBoolItem>        mpFooterItem;
    ::std::unique_ptr<SvxLongLRSpaceItem> mpFooterLRMarginItem;
    ::std::unique_ptr<SvxLongULSpaceItem> mpFooterSpacingItem;
    ::std::unique_ptr<SfxInt16Item>       mpFooterLayoutItem;

    std::unique_ptr<weld::CheckButton> mxFooterToggle;
    std::unique_ptr<weld::ComboBox> mxFooterSpacingLB;
    std::unique_ptr<weld::ComboBox> mxFooterMarginPresetLB;
    std::unique_ptr<weld::ComboBox> mxFooterLayoutLB;
    std::unique_ptr<weld::Label> mxCustomEntry;

    static FieldUnit GetCurrentUnit(SfxItemState eState, const SfxPoolItem* pState);

    DECL_LINK( FooterToggleHdl, weld::Toggleable&, void );
    DECL_LINK( FooterLRMarginHdl, weld::ComboBox&, void);
    DECL_LINK( FooterSpacingHdl, weld::ComboBox&, void);
    DECL_LINK( FooterLayoutHdl, weld::ComboBox&, void);
};

} //end of namespace sw::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
