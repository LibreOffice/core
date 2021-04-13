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

#include <com/sun/star/datatransfer/dnd/XDragSourceContext.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTargetDropContext.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTargetDragContext.hpp>
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>

#include <X11/X.h>

namespace x11 {

    class SelectionManager;

    class DropTargetDropContext :
        public ::cppu::WeakImplHelper<css::datatransfer::dnd::XDropTargetDropContext>
    {
        ::Window                              m_aDropWindow;
        rtl::Reference<SelectionManager>      m_xManager;
    public:
        DropTargetDropContext( ::Window, SelectionManager& );
        virtual ~DropTargetDropContext() override;

        // XDropTargetDropContext
        virtual void SAL_CALL acceptDrop( sal_Int8 dragOperation ) override;
        virtual void SAL_CALL rejectDrop() override;
        virtual void SAL_CALL dropComplete( sal_Bool success ) override;
    };

    class DropTargetDragContext :
        public ::cppu::WeakImplHelper<css::datatransfer::dnd::XDropTargetDragContext>
    {
        ::Window                              m_aDropWindow;
        rtl::Reference<SelectionManager>      m_xManager;
    public:
        DropTargetDragContext( ::Window, SelectionManager& );
        virtual ~DropTargetDragContext() override;

        // XDropTargetDragContext
        virtual void SAL_CALL acceptDrag( sal_Int8 dragOperation ) override;
        virtual void SAL_CALL rejectDrag() override;
    };

    class DragSourceContext :
        public ::cppu::WeakImplHelper<css::datatransfer::dnd::XDragSourceContext>
    {
        ::Window                              m_aDropWindow;
        rtl::Reference<SelectionManager>      m_xManager;
    public:
        DragSourceContext( ::Window, SelectionManager& );
        virtual ~DragSourceContext() override;

        // XDragSourceContext
        virtual sal_Int32   SAL_CALL getCurrentCursor() override;
        virtual void        SAL_CALL setCursor( sal_Int32 cursorId ) override;
        virtual void        SAL_CALL setImage( sal_Int32 imageId ) override;
        virtual void        SAL_CALL transferablesFlavorsChanged() override;
    };
} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
