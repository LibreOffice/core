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

#ifndef _DNDLCON_HXX_
#define _DNDLCON_HXX_

#include <com/sun/star/datatransfer/dnd/XDragGestureRecognizer.hpp>
#include <com/sun/star/datatransfer/dnd/XDragSource.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTargetDragContext.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTargetDropContext.hpp>
#include <cppuhelper/compbase4.hxx>

#include <vcl/unohelp2.hxx>

class DNDListenerContainer :    public ::vcl::unohelper::MutexHelper,
                                public ::cppu::WeakComponentImplHelper4<
    ::com::sun::star::datatransfer::dnd::XDragGestureRecognizer, \
    ::com::sun::star::datatransfer::dnd::XDropTargetDragContext,
    ::com::sun::star::datatransfer::dnd::XDropTargetDropContext,
    ::com::sun::star::datatransfer::dnd::XDropTarget >
{
    sal_Bool m_bActive;
    sal_Int8 m_nDefaultActions;

    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTargetDragContext > m_xDropTargetDragContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTargetDropContext > m_xDropTargetDropContext;

public:

    DNDListenerContainer( sal_Int8 nDefaultActions );
    virtual ~DNDListenerContainer();

    sal_uInt32 fireDropEvent(
        const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTargetDropContext >& context,
        sal_Int8 dropAction, sal_Int32 locationX, sal_Int32 locationY, sal_Int8 sourceActions,
        const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& transferable );

    sal_uInt32 fireDragExitEvent();

    sal_uInt32 fireDragOverEvent(
        const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTargetDragContext >& context,
        sal_Int8 dropAction, sal_Int32 locationX, sal_Int32 locationY, sal_Int8 sourceActions );

    sal_uInt32 fireDragEnterEvent(
        const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTargetDragContext >& context,
        sal_Int8 dropAction, sal_Int32 locationX, sal_Int32 locationY, sal_Int8 sourceActions,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor >& dataFlavor );

    sal_uInt32 fireDropActionChangedEvent(
        const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTargetDragContext >& context,
        sal_Int8 dropAction, sal_Int32 locationX, sal_Int32 locationY, sal_Int8 sourceActions );

    sal_uInt32 fireDragGestureEvent(
        sal_Int8 dragAction, sal_Int32 dragOriginX, sal_Int32 dragOriginY,
        const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDragSource >& dragSource,
        const ::com::sun::star::uno::Any& triggerEvent );

    /*
     * XDragGestureRecognizer
     */

    virtual void SAL_CALL addDragGestureListener( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDragGestureListener >& dgl ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeDragGestureListener( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDragGestureListener >& dgl ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL resetRecognizer(  ) throw(::com::sun::star::uno::RuntimeException);

       /*
     * XDropTargetDragContext
     */

    virtual void SAL_CALL acceptDrag( sal_Int8 dragOperation ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL rejectDrag(  ) throw (::com::sun::star::uno::RuntimeException);


       /*
     * XDropTargetDropContext
     */

    virtual void SAL_CALL acceptDrop( sal_Int8 dropOperation ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL rejectDrop(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL dropComplete( sal_Bool success ) throw (::com::sun::star::uno::RuntimeException);

    /*
     * XDropTarget
     */

    virtual void SAL_CALL addDropTargetListener( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTargetListener >& dtl ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeDropTargetListener( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTargetListener >& dtl ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isActive(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setActive( sal_Bool active ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int8 SAL_CALL getDefaultActions(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDefaultActions( sal_Int8 actions ) throw(::com::sun::star::uno::RuntimeException);
};


//==================================================================================================
//
//==================================================================================================

#endif
