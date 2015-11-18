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
    css::datatransfer::dnd::XDragGestureRecognizer,
    css::datatransfer::dnd::XDropTargetDragContext,
    css::datatransfer::dnd::XDropTargetDropContext,
    css::datatransfer::dnd::XDropTarget >
{
    bool m_bActive;
    sal_Int8 m_nDefaultActions;

    css::uno::Reference< css::datatransfer::dnd::XDropTargetDragContext > m_xDropTargetDragContext;
    css::uno::Reference< css::datatransfer::dnd::XDropTargetDropContext > m_xDropTargetDropContext;

public:

    DNDListenerContainer( sal_Int8 nDefaultActions );
    virtual ~DNDListenerContainer();

    sal_uInt32 fireDropEvent(
        const css::uno::Reference< css::datatransfer::dnd::XDropTargetDropContext >& context,
        sal_Int8 dropAction, sal_Int32 locationX, sal_Int32 locationY, sal_Int8 sourceActions,
        const css::uno::Reference< css::datatransfer::XTransferable >& transferable );

    sal_uInt32 fireDragExitEvent();

    sal_uInt32 fireDragOverEvent(
        const css::uno::Reference< css::datatransfer::dnd::XDropTargetDragContext >& context,
        sal_Int8 dropAction, sal_Int32 locationX, sal_Int32 locationY, sal_Int8 sourceActions );

    sal_uInt32 fireDragEnterEvent(
        const css::uno::Reference< css::datatransfer::dnd::XDropTargetDragContext >& context,
        sal_Int8 dropAction, sal_Int32 locationX, sal_Int32 locationY, sal_Int8 sourceActions,
        const css::uno::Sequence< css::datatransfer::DataFlavor >& dataFlavor );

    sal_uInt32 fireDropActionChangedEvent(
        const css::uno::Reference< css::datatransfer::dnd::XDropTargetDragContext >& context,
        sal_Int8 dropAction, sal_Int32 locationX, sal_Int32 locationY, sal_Int8 sourceActions );

    sal_uInt32 fireDragGestureEvent(
        sal_Int8 dragAction, sal_Int32 dragOriginX, sal_Int32 dragOriginY,
        const css::uno::Reference< css::datatransfer::dnd::XDragSource >& dragSource,
        const css::uno::Any& triggerEvent );

    /*
     * XDragGestureRecognizer
     */

    virtual void SAL_CALL addDragGestureListener( const css::uno::Reference< css::datatransfer::dnd::XDragGestureListener >& dgl ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeDragGestureListener( const css::uno::Reference< css::datatransfer::dnd::XDragGestureListener >& dgl ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL resetRecognizer(  ) throw(css::uno::RuntimeException, std::exception) override;

       /*
     * XDropTargetDragContext
     */

    virtual void SAL_CALL acceptDrag( sal_Int8 dragOperation ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL rejectDrag(  ) throw (css::uno::RuntimeException, std::exception) override;

       /*
     * XDropTargetDropContext
     */

    virtual void SAL_CALL acceptDrop( sal_Int8 dropOperation ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL rejectDrop(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL dropComplete( sal_Bool success ) throw (css::uno::RuntimeException, std::exception) override;

    /*
     * XDropTarget
     */

    virtual void SAL_CALL addDropTargetListener( const css::uno::Reference< css::datatransfer::dnd::XDropTargetListener >& dtl ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeDropTargetListener( const css::uno::Reference< css::datatransfer::dnd::XDropTargetListener >& dtl ) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isActive(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setActive( sal_Bool active ) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Int8 SAL_CALL getDefaultActions(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDefaultActions( sal_Int8 actions ) throw(css::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
