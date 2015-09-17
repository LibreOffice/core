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
#include <vcl/image.hxx>
#include <vcl/fixed.hxx>
#include <sfx2/sidebar/ControllerItem.hxx>
#include <com/sun/star/frame/XFrame.hpp>
#include <vcl/lstbox.hxx>

namespace sw { namespace sidebar {

    class WrapPropertyPanel
        : public PanelLayout
        , public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
    {
    public:
        static VclPtr<vcl::Window> Create(
            vcl::Window* pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame>& rxFrame,
            SfxBindings* pBindings );

        // interface of ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
        virtual void NotifyItemUpdate(
            const sal_uInt16 nSId,
            const SfxItemState eState,
            const SfxPoolItem* pState,
            const bool bIsEnabled) SAL_OVERRIDE;

        virtual ~WrapPropertyPanel();
        virtual void dispose() SAL_OVERRIDE;

        WrapPropertyPanel(
            vcl::Window* pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
            SfxBindings* pBindings );
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > mxFrame;
        SfxBindings* mpBindings;

        VclPtr<RadioButton> mpRBNoWrap;
        VclPtr<RadioButton> mpRBWrapLeft;
        VclPtr<RadioButton> mpRBWrapRight;
        VclPtr<RadioButton> mpRBWrapParallel;
        VclPtr<RadioButton> mpRBWrapThrough;
        VclPtr<RadioButton> mpRBIdealWrap;
        VclPtr<Button>      mpEditContour;
        VclPtr<CheckBox> mpEnableContour;
        VclPtr<ListBox>  mpSpacingLB;
        VclPtr<FixedText> mpCustomEntry;

        //Spacing
        sal_uInt16 nTop;
        sal_uInt16 nBottom;
        sal_uInt16 nLeft;
        sal_uInt16 nRight;

        //custom entry
        OUString aCustomEntry;

        //Image resource.
        ImageList aWrapIL;

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

        DECL_LINK_TYPED(WrapTypeHdl, Button*, void);
        DECL_LINK_TYPED(EnableContourHdl, Button*, void);
        DECL_LINK_TYPED(EditContourHdl, Button*, void);
        DECL_LINK(SpacingLBHdl, ListBox*);
    };

} } // end of namespace ::sw::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
