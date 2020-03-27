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
#ifndef INCLUDED_SW_SOURCE_UIBASE_SIDEBAR_PAGEHEADERPANEL_HXX
#define INCLUDED_SW_SOURCE_UIBASE_SIDEBAR_PAGEHEADERPANEL_HXX

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

class PageHeaderPanel:
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
    PageHeaderPanel(
        vcl::Window* pParent,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
        SfxBindings* pBindings);
    virtual ~PageHeaderPanel() override;
    virtual void dispose() override;

private:

    SfxBindings* mpBindings;

    ::sfx2::sidebar::ControllerItem maHFToggleController;
    ::sfx2::sidebar::ControllerItem maMetricController;
    ::sfx2::sidebar::ControllerItem maHeaderLRMarginController;
    ::sfx2::sidebar::ControllerItem maHeaderSpacingController;
    ::sfx2::sidebar::ControllerItem maHeaderLayoutController;

    FieldUnit meFUnit;

    VclPtr<CheckBox>           mpHeaderToggle;
    VclPtr<SpacingListBox>     mpHeaderSpacingLB;
    VclPtr<SpacingListBox>     mpHeaderMarginPresetLB;
    VclPtr<SameContentListBox> mpHeaderLayoutLB;
    VclPtr<FixedText>          mpCustomEntry;
    OUString aCustomEntry;

    void Initialize();
    void SetMarginsAndSpacingFieldUnit();
    void UpdateHeaderCheck();
    void UpdateMarginControl();
    void UpdateSpacingControl();
    void UpdateLayoutControl();

    ::std::unique_ptr<SfxBoolItem>        mpHeaderItem;
    ::std::unique_ptr<SvxLongLRSpaceItem> mpHeaderLRMarginItem;
    ::std::unique_ptr<SvxLongULSpaceItem> mpHeaderSpacingItem;
    ::std::unique_ptr<SfxInt16Item>       mpHeaderLayoutItem;

    static FieldUnit GetCurrentUnit(SfxItemState eState, const SfxPoolItem* pState);

    DECL_LINK( HeaderToggleHdl, Button*, void );
    DECL_LINK( HeaderLRMarginHdl, ListBox&, void);
    DECL_LINK( HeaderSpacingHdl, ListBox&, void);
    DECL_LINK( HeaderLayoutHdl, ListBox&, void);
};

} } //end of namespace sw::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
