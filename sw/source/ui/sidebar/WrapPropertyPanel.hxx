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
#ifndef SW_SIDEBAR_WRAP_PROPERTY_PANEL_HXX
#define SW_SIDEBAR_WRAP_PROPERTY_PANEL_HXX

#include <svx/sidebar/PanelLayout.hxx>
#include <vcl/button.hxx>
#include <vcl/image.hxx>

#include <sfx2/sidebar/ControllerItem.hxx>
#include <com/sun/star/frame/XFrame.hpp>

#include <boost/scoped_ptr.hpp>

namespace sw { namespace sidebar {

    class WrapPropertyPanel
        : public PanelLayout
        , public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
    {
    public:
        static WrapPropertyPanel* Create(
            Window* pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame>& rxFrame,
            SfxBindings* pBindings );

        // interface of ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
        virtual void NotifyItemUpdate(
            const sal_uInt16 nSId,
            const SfxItemState eState,
            const SfxPoolItem* pState,
            const bool bIsEnabled);

    private:
        WrapPropertyPanel(
            Window* pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
            SfxBindings* pBindings );

        virtual ~WrapPropertyPanel();

        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > mxFrame;
        SfxBindings* mpBindings;

        RadioButton* mpRBNoWrap;
        RadioButton* mpRBWrapLeft;
        RadioButton* mpRBWrapRight;
        RadioButton* mpRBWrapParallel;
        RadioButton* mpRBWrapThrough;
        RadioButton* mpRBIdealWrap;

        //Image resource.
        ImageList aWrapIL;

        //Controler Items==================================
        ::sfx2::sidebar::ControllerItem maSwNoWrapControl;
        ::sfx2::sidebar::ControllerItem maSwWrapLeftControl;
        ::sfx2::sidebar::ControllerItem maSwWrapRightControl;
        ::sfx2::sidebar::ControllerItem maSwWrapParallelControl;
        ::sfx2::sidebar::ControllerItem maSwWrapThroughControl;
        ::sfx2::sidebar::ControllerItem maSwWrapIdealControl;

        void Initialize();

        DECL_LINK(WrapTypeHdl, void*);
    };

} } // end of namespace ::sw::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
