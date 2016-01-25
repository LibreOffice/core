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

#ifndef INCLUDED_VCL_INC_DNDEVDIS_HXX
#define INCLUDED_VCL_INC_DNDEVDIS_HXX

#include <com/sun/star/datatransfer/dnd/XDropTargetListener.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTargetDragContext.hpp>

#include <com/sun/star/datatransfer/dnd/XDragGestureRecognizer.hpp>
#include <cppuhelper/implbase.hxx>
#include <vcl/window.hxx>

class DNDEventDispatcher: public ::cppu::WeakImplHelper<
    css::datatransfer::dnd::XDropTargetListener,
    css::datatransfer::dnd::XDropTargetDragContext,
    css::datatransfer::dnd::XDragGestureListener >
{
    VclPtr<vcl::Window> m_pTopWindow;

    VclPtr<vcl::Window> m_pCurrentWindow;
    void designate_currentwindow(vcl::Window *pWindow);
    DECL_LINK_TYPED(WindowEventListener, VclWindowEvent&, void);

    ::osl::Mutex m_aMutex;
    css::uno::Sequence< css::datatransfer::DataFlavor > m_aDataFlavorList;

    vcl::Window* findTopLevelWindow(Point location);
    /*
     * fire the events on the dnd listener container of the specified window
     */

    static sal_Int32 fireDragEnterEvent( vcl::Window *pWindow, const css::uno::Reference< css::datatransfer::dnd::XDropTargetDragContext >& xContext,
        const sal_Int8 nDropAction, const Point& rLocation, const sal_Int8 nSourceAction,
        const css::uno::Sequence< css::datatransfer::DataFlavor >& aFlavorList ) throw(css::uno::RuntimeException);

    static sal_Int32 fireDragOverEvent( vcl::Window *pWindow, const css::uno::Reference< css::datatransfer::dnd::XDropTargetDragContext >& xContext,
        const sal_Int8 nDropAction, const Point& rLocation, const sal_Int8 nSourceAction ) throw(css::uno::RuntimeException);

    static sal_Int32 fireDragExitEvent( vcl::Window *pWindow ) throw(css::uno::RuntimeException);

    static sal_Int32 fireDropActionChangedEvent( vcl::Window *pWindow, const css::uno::Reference< css::datatransfer::dnd::XDropTargetDragContext >& xContext,
        const sal_Int8 nDropAction, const Point& rLocation, const sal_Int8 nSourceAction ) throw(css::uno::RuntimeException);

    static sal_Int32 fireDropEvent( vcl::Window *pWindow, const css::uno::Reference< css::datatransfer::dnd::XDropTargetDropContext >& xContext,
        const sal_Int8 nDropAction, const Point& rLocation, const sal_Int8 nSourceAction,
        const css::uno::Reference< css::datatransfer::XTransferable >& xTransferable ) throw(css::uno::RuntimeException);

    static sal_Int32 fireDragGestureEvent( vcl::Window *pWindow, const css::uno::Reference< css::datatransfer::dnd::XDragSource >& xSource,
        const css::uno::Any& event, const Point& rOrigin, const sal_Int8 nDragAction )throw(css::uno::RuntimeException);

public:

    DNDEventDispatcher( vcl::Window * pTopWindow );
    virtual ~DNDEventDispatcher();

    /*
     * XDropTargetDragContext
     */

    virtual void SAL_CALL acceptDrag( sal_Int8 dropAction ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL rejectDrag() throw(css::uno::RuntimeException, std::exception) override;

    /*
     * XDropTargetListener
     */

    virtual void SAL_CALL drop( const css::datatransfer::dnd::DropTargetDropEvent& dtde ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL dragEnter( const css::datatransfer::dnd::DropTargetDragEnterEvent& dtdee ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL dragExit( const css::datatransfer::dnd::DropTargetEvent& dte ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL dragOver( const css::datatransfer::dnd::DropTargetDragEvent& dtde ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL dropActionChanged( const css::datatransfer::dnd::DropTargetDragEvent& dtde ) throw(css::uno::RuntimeException, std::exception) override;

    /*
     * XDragGestureListener
     */

    virtual void SAL_CALL dragGestureRecognized( const css::datatransfer::dnd::DragGestureEvent& dge ) throw(css::uno::RuntimeException, std::exception) override;

    /*
     * XEventListener
     */

    virtual void SAL_CALL disposing( const css::lang::EventObject& eo ) throw(css::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
