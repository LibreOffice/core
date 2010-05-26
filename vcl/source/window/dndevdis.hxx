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

#ifndef _DNDEVDIS_HXX_
#define _DNDEVDIS_HXX_

#include <com/sun/star/datatransfer/dnd/XDropTargetListener.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTargetDragContext.hpp>

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDRAGESTURERECOGNIZER_HPP_
#include <com/sun/star/datatransfer/dnd/XDragGestureRecognizer.hpp>
#endif
#include <cppuhelper/implbase3.hxx>
#include <vcl/window.hxx>

class DNDEventDispatcher: public ::cppu::WeakImplHelper3<
    ::com::sun::star::datatransfer::dnd::XDropTargetListener,
    ::com::sun::star::datatransfer::dnd::XDropTargetDragContext,
    ::com::sun::star::datatransfer::dnd::XDragGestureListener >
{
    Window * m_pTopWindow;
    Window * m_pCurrentWindow;

    ::osl::Mutex m_aMutex;
    ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor > m_aDataFlavorList;

    /*
     * fire the events on the dnd listener container of the specified window
     */

    sal_Int32 fireDragEnterEvent( Window *pWindow, const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTargetDragContext >& xContext,
        const sal_Int8 nDropAction, const Point& rLocation, const sal_Int8 nSourceAction,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor >& aFlavorList ) throw(::com::sun::star::uno::RuntimeException);

    sal_Int32 fireDragOverEvent( Window *pWindow, const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTargetDragContext >& xContext,
        const sal_Int8 nDropAction, const Point& rLocation, const sal_Int8 nSourceAction ) throw(::com::sun::star::uno::RuntimeException);

    sal_Int32 fireDragExitEvent( Window *pWindow ) throw(::com::sun::star::uno::RuntimeException);

    sal_Int32 fireDropActionChangedEvent( Window *pWindow, const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTargetDragContext >& xContext,
        const sal_Int8 nDropAction, const Point& rLocation, const sal_Int8 nSourceAction ) throw(::com::sun::star::uno::RuntimeException);

    sal_Int32 fireDropEvent( Window *pWindow, const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTargetDropContext >& xContext,
        const sal_Int8 nDropAction, const Point& rLocation, const sal_Int8 nSourceAction,
        const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& xTransferable ) throw(::com::sun::star::uno::RuntimeException);

    sal_Int32 fireDragGestureEvent( Window *pWindow, const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDragSource >& xSource,
        const ::com::sun::star::uno::Any event, const Point& rOrigin, const sal_Int8 nDragAction )throw(::com::sun::star::uno::RuntimeException);

public:

    DNDEventDispatcher( Window * pTopWindow );
    virtual ~DNDEventDispatcher();

    /*
     * XDropTargetDragContext
     */

    virtual void SAL_CALL acceptDrag( sal_Int8 dropAction ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL rejectDrag() throw(::com::sun::star::uno::RuntimeException);

    /*
     * XDropTargetListener
     */

    virtual void SAL_CALL drop( const ::com::sun::star::datatransfer::dnd::DropTargetDropEvent& dtde ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL dragEnter( const ::com::sun::star::datatransfer::dnd::DropTargetDragEnterEvent& dtdee ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL dragExit( const ::com::sun::star::datatransfer::dnd::DropTargetEvent& dte ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL dragOver( const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& dtde ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL dropActionChanged( const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& dtde ) throw(::com::sun::star::uno::RuntimeException);

    /*
     * XDragGestureListener
     */

    virtual void SAL_CALL dragGestureRecognized( const ::com::sun::star::datatransfer::dnd::DragGestureEvent& dge ) throw(::com::sun::star::uno::RuntimeException);


    /*
     * XEventListener
     */

    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& eo ) throw(::com::sun::star::uno::RuntimeException);
};

//==================================================================================================
//
//==================================================================================================

#endif
