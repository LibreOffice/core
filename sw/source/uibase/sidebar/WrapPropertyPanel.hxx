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
#ifndef INCLUDED_SW_SOURCE_UIBASE_SIDEBAR_WRAPPROPERTYPANEL_HXX
#define INCLUDED_SW_SOURCE_UIBASE_SIDEBAR_WRAPPROPERTYPANEL_HXX

#include <svx/sidebar/PanelLayout.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>
#include <com/sun/star/frame/XFrame.hpp>
#include <vcl/lstbox.hxx>
#include <svx/spacinglistbox.hxx>

namespace sw { namespace sidebar {

    class WrapPropertyPanel
        : public PanelLayout
        , public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
    {
    public:
        static VclPtr<vcl::Window> Create(
            vcl::Window* pParent,
            const css::uno::Reference< css::frame::XFrame>& rxFrame,
            SfxBindings* pBindings );

        // interface of ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
        virtual void NotifyItemUpdate(
            const sal_uInt16 nSId,
            const SfxItemState eState,
            const SfxPoolItem* pState) override;

        virtual void GetControlState(
            const sal_uInt16 /*nSId*/,
            boost::property_tree::ptree& /*rState*/) override {};

        virtual ~WrapPropertyPanel() override;
        virtual void dispose() override;

        WrapPropertyPanel(
            vcl::Window* pParent,
            const css::uno::Reference< css::frame::XFrame >& rxFrame,
            SfxBindings* pBindings );
    private:
        css::uno::Reference< css::frame::XFrame > mxFrame;
        SfxBindings* mpBindings;

        VclPtr<RadioButton> mpRBNoWrap;
        VclPtr<RadioButton> mpRBWrapLeft;
        VclPtr<RadioButton> mpRBWrapRight;
        VclPtr<RadioButton> mpRBWrapParallel;
        VclPtr<RadioButton> mpRBWrapThrough;
        VclPtr<RadioButton> mpRBIdealWrap;
        VclPtr<Button>      mpEditContour;
        VclPtr<CheckBox> mpEnableContour;
        VclPtr<SpacingListBox>  mpSpacingLB;
        VclPtr<FixedText> mpCustomEntry;

        //Spacing
        sal_uInt16 nTop;
        sal_uInt16 nBottom;
        sal_uInt16 nLeft;
        sal_uInt16 nRight;

        //custom entry
        OUString aCustomEntry;

        // Controller Items
        ::sfx2::sidebar::ControllerItem maSwNoWrapControl;
        ::sfx2::sidebar::ControllerItem maSwWrapLeftControl;
        ::sfx2::sidebar::ControllerItem maSwWrapRightControl;
        ::sfx2::sidebar::ControllerItem maSwWrapParallelControl;
        ::sfx2::sidebar::ControllerItem maSwWrapThroughControl;
        ::sfx2::sidebar::ControllerItem maSwWrapIdealControl;
        ::sfx2::sidebar::ControllerItem maSwEnableContourControl;
        ::sfx2::sidebar::ControllerItem maSwLRSpacingControl;
        ::sfx2::sidebar::ControllerItem maSwULSpacingControl;

        void Initialize();
        void UpdateEditContour();
        void UpdateSpacingLB();

        DECL_LINK(WrapTypeHdl, Button*, void);
        DECL_LINK(EnableContourHdl, Button*, void);
        DECL_LINK(EditContourHdl, Button*, void);
        DECL_LINK(SpacingLBHdl, ListBox&, void);
    };

} } // end of namespace ::sw::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
