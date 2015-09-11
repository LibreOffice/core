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

#ifndef INCLUDED_VCL_UNX_GENERIC_DTRANS_X11_DNDCONTEXT_HXX
#define INCLUDED_VCL_UNX_GENERIC_DTRANS_X11_DNDCONTEXT_HXX

#include <com/sun/star/datatransfer/dnd/XDragSourceContext.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTargetDropContext.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTargetDragContext.hpp>
#include <cppuhelper/implbase.hxx>

#include <prex.h>
#include <X11/Xlib.h>
#include <postx.h>

namespace x11 {

    class SelectionManager;

    class DropTargetDropContext :
        public ::cppu::WeakImplHelper<
    ::com::sun::star::datatransfer::dnd::XDropTargetDropContext
    >
    {
        ::Window                    m_aDropWindow;
        Time                        m_nTimestamp;
        SelectionManager&           m_rManager;
        com::sun::star::uno::Reference< XInterface >     m_xManagerRef;
    public:
        DropTargetDropContext( ::Window, Time, SelectionManager& );
        virtual ~DropTargetDropContext();

        // XDropTargetDropContext
        virtual void SAL_CALL acceptDrop( sal_Int8 dragOperation ) throw(std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL rejectDrop() throw(std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL dropComplete( sal_Bool success ) throw(std::exception) SAL_OVERRIDE;
    };

    class DropTargetDragContext :
        public ::cppu::WeakImplHelper<
    ::com::sun::star::datatransfer::dnd::XDropTargetDragContext
    >
    {
        ::Window                    m_aDropWindow;
        Time                        m_nTimestamp;
        SelectionManager&           m_rManager;
        com::sun::star::uno::Reference< XInterface >     m_xManagerRef;
    public:
        DropTargetDragContext( ::Window, Time, SelectionManager& );
        virtual ~DropTargetDragContext();

        // XDropTargetDragContext
        virtual void SAL_CALL acceptDrag( sal_Int8 dragOperation ) throw(std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL rejectDrag() throw(std::exception) SAL_OVERRIDE;
    };

    class DragSourceContext :
        public ::cppu::WeakImplHelper<
    ::com::sun::star::datatransfer::dnd::XDragSourceContext
    >
    {
        ::Window                    m_aDropWindow;
        Time                        m_nTimestamp;
        SelectionManager&           m_rManager;
        com::sun::star::uno::Reference< XInterface >     m_xManagerRef;
    public:
        DragSourceContext( ::Window, Time, SelectionManager& );
        virtual ~DragSourceContext();

        // XDragSourceContext
        virtual sal_Int32   SAL_CALL getCurrentCursor() throw(std::exception) SAL_OVERRIDE;
        virtual void        SAL_CALL setCursor( sal_Int32 cursorId ) throw(std::exception) SAL_OVERRIDE;
        virtual void        SAL_CALL setImage( sal_Int32 imageId ) throw(std::exception) SAL_OVERRIDE;
        virtual void        SAL_CALL transferablesFlavorsChanged() throw(std::exception) SAL_OVERRIDE;
    };
} // namespace

#endif // INCLUDED_VCL_UNX_GENERIC_DTRANS_X11_DNDCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
