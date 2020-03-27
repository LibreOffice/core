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
#ifndef INCLUDED_SW_SOURCE_UIBASE_SIDEBAR_PAGEFORMATPANEL_HXX
#define INCLUDED_SW_SOURCE_UIBASE_SIDEBAR_PAGEFORMATPANEL_HXX

#include <com/sun/star/frame/XFrame.hpp>

#include <svx/sidebar/PanelLayout.hxx>

#include <sfx2/sidebar/ControllerItem.hxx>

#include <svx/pageitem.hxx>
#include <svx/rulritem.hxx>
#include <svx/papersizelistbox.hxx>

#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <tools/fldunit.hxx>
#include <svl/poolitem.hxx>
#include <svx/relfld.hxx>

#include <memory>

namespace sw { namespace sidebar {

class PageFormatPanel:
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

    PageFormatPanel(
        vcl::Window* pParent,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
        SfxBindings* pBindings);
    virtual ~PageFormatPanel() override;
    virtual void dispose() override;

    static FieldUnit GetCurrentUnit( SfxItemState eState, const SfxPoolItem* pState );

private:

    SfxBindings* mpBindings;

    VclPtr<PaperSizeListBox> mpPaperSizeBox;
    VclPtr<SvxRelativeField> mpPaperWidth;
    VclPtr<SvxRelativeField> mpPaperHeight;
    VclPtr<ListBox> mpPaperOrientation;
    VclPtr<ListBox> mpMarginSelectBox;
    VclPtr<FixedText> mpCustomEntry;

    ::sfx2::sidebar::ControllerItem maPaperSizeController;
    ::sfx2::sidebar::ControllerItem maPaperOrientationController;
    ::sfx2::sidebar::ControllerItem maMetricController;
    ::sfx2::sidebar::ControllerItem maSwPageLRControl;
    ::sfx2::sidebar::ControllerItem maSwPageULControl;

    std::unique_ptr<SvxPageItem>          mpPageItem;
    std::unique_ptr<SvxLongLRSpaceItem> mpPageLRMarginItem;
    std::unique_ptr<SvxLongULSpaceItem> mpPageULMarginItem;

    FieldUnit meFUnit;
    MapUnit meUnit;

    long mnPageLeftMargin;
    long mnPageRightMargin;
    long mnPageTopMargin;
    long mnPageBottomMargin;
    OUString aCustomEntry;

    void Initialize();
    void SetMarginFieldUnit();
    void UpdateMarginBox();
    void ExecuteMarginLRChange( const long nPageLeftMargin, const long nPageRightMargin );
    void ExecuteMarginULChange( const long nPageTopMargin, const long  nPageBottomMargin);
    DECL_LINK(PaperFormatModifyHdl, ListBox&, void);
    DECL_LINK(PaperSizeModifyHdl, Edit&, void);
    DECL_LINK(PaperModifyMarginHdl, ListBox&, void );
};

} } //end of namespace sw::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
