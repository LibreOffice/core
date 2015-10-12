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

#ifndef INCLUDED_VCL_INC_DNDLCON_HXX
#define INCLUDED_VCL_INC_DNDLCON_HXX

#include <com/sun/star/datatransfer/dnd/XDragGestureRecognizer.hpp>
#include <com/sun/star/datatransfer/dnd/XDragSource.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTargetDragContext.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTargetDropContext.hpp>
#include <cppuhelper/compbase.hxx>

#include <vcl/unohelp2.hxx>

class DNDListenerContainer :    public vcl::unohelper::MutexHelper,
                                public ::cppu::WeakComponentImplHelper<
    ::com::sun::star::datatransfer::dnd::XDragGestureRecognizer, \
    ::com::sun::star::datatransfer::dnd::XDropTargetDragContext,
    ::com::sun::star::datatransfer::dnd::XDropTargetDropContext,
    ::com::sun::star::datatransfer::dnd::XDropTarget >
{
    bool m_bActive;
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

    virtual void SAL_CALL addDragGestureListener( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDragGestureListener >& dgl ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeDragGestureListener( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDragGestureListener >& dgl ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL resetRecognizer(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

       /*
     * XDropTargetDragContext
     */

    virtual void SAL_CALL acceptDrag( sal_Int8 dragOperation ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL rejectDrag(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

       /*
     * XDropTargetDropContext
     */

    virtual void SAL_CALL acceptDrop( sal_Int8 dropOperation ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL rejectDrop(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL dropComplete( sal_Bool success ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /*
     * XDropTarget
     */

    virtual void SAL_CALL addDropTargetListener( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTargetListener >& dtl ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeDropTargetListener( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTargetListener >& dtl ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isActive(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setActive( sal_Bool active ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int8 SAL_CALL getDefaultActions(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDefaultActions( sal_Int8 actions ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
