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
#include <com/sun/star/frame/XFrame.hpp>

#include <svx/sidebar/PanelLayout.hxx>

#include <sfx2/sidebar/ControllerItem.hxx>

#include <svx/rulritem.hxx>

#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <svl/intitem.hxx>
#include <svl/poolitem.hxx>
#include <svl/eitem.hxx>
#include <svx/spacinglistbox.hxx>
#include <svx/samecontentlistbox.hxx>

namespace sw { namespace sidebar {

class PageFooterPanel:
    public PanelLayout,
    public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
    static VclPtr<vcl::Window> Create(
        vcl::Window* pParent,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
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
        vcl::Window* pParent,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
        SfxBindings* pBindings);
    virtual ~PageFooterPanel() override;
    virtual void dispose() override;

private:

    SfxBindings* mpBindings;

    ::sfx2::sidebar::ControllerItem maHFToggleController;
    ::sfx2::sidebar::ControllerItem maMetricController;
    ::sfx2::sidebar::ControllerItem maFooterLRMarginController;
    ::sfx2::sidebar::ControllerItem maFooterSpacingController;
    ::sfx2::sidebar::ControllerItem maFooterLayoutController;

    FieldUnit meFUnit;

    VclPtr<CheckBox>           mpFooterToggle;
    VclPtr<SpacingListBox>     mpFooterSpacingLB;
    VclPtr<SpacingListBox>     mpFooterMarginPresetLB;
    VclPtr<SameContentListBox> mpFooterLayoutLB;
    VclPtr<FixedText>          mpCustomEntry;
    OUString aCustomEntry;

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

    static FieldUnit GetCurrentUnit(SfxItemState eState, const SfxPoolItem* pState);

    DECL_LINK( FooterToggleHdl, Button*, void );
    DECL_LINK( FooterLRMarginHdl, ListBox&, void);
    DECL_LINK( FooterSpacingHdl, ListBox&, void);
    DECL_LINK( FooterLayoutHdl, ListBox&, void);
};

} } //end of namespace sw::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
