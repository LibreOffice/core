/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _DTRANS_X11_DNDCONTEXT_HXX
#define _DTRANS_X11_DNDCONTEXT_HXX

#include <com/sun/star/datatransfer/dnd/XDragSourceContext.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTargetDropContext.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTargetDragContext.hpp>
#include <cppuhelper/implbase1.hxx>

#include "tools/prex.h"
#include <X11/Xlib.h>
#include "tools/postx.h"

using namespace com::sun::star::uno;

namespace x11 {

    class SelectionManager;

    class DropTargetDropContext :
        public ::cppu::WeakImplHelper1<
    ::com::sun::star::datatransfer::dnd::XDropTargetDropContext
    >
    {
        XLIB_Window                 m_aDropWindow;
        XLIB_Time                   m_nTimestamp;
        SelectionManager&           m_rManager;
        com::sun::star::uno::Reference< XInterface >     m_xManagerRef;
    public:
        DropTargetDropContext( XLIB_Window, XLIB_Time, SelectionManager& );
        virtual ~DropTargetDropContext();

        // XDropTargetDropContext
        virtual void SAL_CALL acceptDrop( sal_Int8 dragOperation ) throw();
        virtual void SAL_CALL rejectDrop() throw();
        virtual void SAL_CALL dropComplete( sal_Bool success ) throw();
    };

    class DropTargetDragContext :
        public ::cppu::WeakImplHelper1<
    ::com::sun::star::datatransfer::dnd::XDropTargetDragContext
    >
    {
        XLIB_Window                 m_aDropWindow;
        XLIB_Time                   m_nTimestamp;
        SelectionManager&           m_rManager;
        com::sun::star::uno::Reference< XInterface >     m_xManagerRef;
    public:
        DropTargetDragContext( XLIB_Window, XLIB_Time, SelectionManager& );
        virtual ~DropTargetDragContext();

        // XDropTargetDragContext
        virtual void SAL_CALL acceptDrag( sal_Int8 dragOperation ) throw();
        virtual void SAL_CALL rejectDrag() throw();
    };

    class DragSourceContext :
        public ::cppu::WeakImplHelper1<
    ::com::sun::star::datatransfer::dnd::XDragSourceContext
    >
    {
        XLIB_Window                 m_aDropWindow;
        XLIB_Time                   m_nTimestamp;
        SelectionManager&           m_rManager;
        com::sun::star::uno::Reference< XInterface >     m_xManagerRef;
    public:
        DragSourceContext( XLIB_Window, XLIB_Time, SelectionManager& );
        virtual ~DragSourceContext();

        // XDragSourceContext
        virtual sal_Int32   SAL_CALL getCurrentCursor() throw();
        virtual void        SAL_CALL setCursor( sal_Int32 cursorId ) throw();
        virtual void        SAL_CALL setImage( sal_Int32 imageId ) throw();
        virtual void        SAL_CALL transferablesFlavorsChanged() throw();
    };
} // namespace

#endif // _DTRANS_X11_DNDCONTEXT_HXX
