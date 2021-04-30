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

#ifndef INCLUDED_VCL_DNDHELP_HXX
#define INCLUDED_VCL_DNDHELP_HXX

#include <vcl/dllapi.h>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/datatransfer/dnd/XDragGestureListener.hpp>
#include <com/sun/star/datatransfer/dnd/XDragSourceListener.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTargetListener.hpp>

namespace com::sun::star::datatransfer::dnd {
    struct DragGestureEvent;
    struct DragSourceDragEvent;
    struct DragSourceDropEvent;
    struct DragSourceEvent;
    struct DropTargetDragEvent;
    struct DropTargetDragEnterEvent;
    struct DropTargetDropEvent;
    struct DropTargetEvent;
}

namespace vcl::unohelper
{

class VCL_DLLPUBLIC DragAndDropClient
{
public:
    virtual ~DragAndDropClient() COVERITY_NOEXCEPT_FALSE;

    // css::datatransfer::dnd::XDragGestureListener
    /// @throws css::uno::RuntimeException
    virtual void dragGestureRecognized( const css::datatransfer::dnd::DragGestureEvent& dge );

    // css::datatransfer::dnd::XDragSourceListener
    /// @throws css::uno::RuntimeException
    virtual void dragDropEnd(const css::datatransfer::dnd::DragSourceDropEvent& dsde );

    // css::datatransfer::dnd::XDropTargetListener
    /// @throws css::uno::RuntimeException
    virtual void drop( const css::datatransfer::dnd::DropTargetDropEvent& dtde );
    /// @throws css::uno::RuntimeException
    virtual void dragEnter( const css::datatransfer::dnd::DropTargetDragEnterEvent& dtdee );
    /// @throws css::uno::RuntimeException
    virtual void dragExit( const css::datatransfer::dnd::DropTargetEvent& dte );
    /// @throws css::uno::RuntimeException
    virtual void dragOver( const css::datatransfer::dnd::DropTargetDragEvent& dtde );
};

class VCL_DLLPUBLIC DragAndDropWrapper final :
                            public css::datatransfer::dnd::XDragGestureListener,
                            public css::datatransfer::dnd::XDragSourceListener,
                            public css::datatransfer::dnd::XDropTargetListener,
                            public ::cppu::OWeakObject
{
private:
    DragAndDropClient* mpClient;

public:
                    DragAndDropWrapper( DragAndDropClient* pClient );
                    virtual ~DragAndDropWrapper() override;

    // css::uno::XInterface
    css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    void          SAL_CALL acquire() noexcept override  { OWeakObject::acquire(); }
    void          SAL_CALL release() noexcept override  { OWeakObject::release(); }

    // css::lang::XEventListener
    void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

    // css::datatransfer::dnd::XDragGestureListener
    void SAL_CALL dragGestureRecognized( const css::datatransfer::dnd::DragGestureEvent& dge ) override;

    // css::datatransfer::dnd::XDragSourceListener
    void SAL_CALL dragDropEnd( const css::datatransfer::dnd::DragSourceDropEvent& dsde ) override;
    void SAL_CALL dragEnter( const css::datatransfer::dnd::DragSourceDragEvent& dsde ) override;
    void SAL_CALL dragExit( const css::datatransfer::dnd::DragSourceEvent& dse ) override;
    void SAL_CALL dragOver( const css::datatransfer::dnd::DragSourceDragEvent& dsde ) override;
    void SAL_CALL dropActionChanged( const css::datatransfer::dnd::DragSourceDragEvent& dsde ) override;

    // css::datatransfer::dnd::XDropTargetListener
    void SAL_CALL drop( const css::datatransfer::dnd::DropTargetDropEvent& dtde ) override;
    void SAL_CALL dragEnter( const css::datatransfer::dnd::DropTargetDragEnterEvent& dtdee ) override;
    void SAL_CALL dragExit( const css::datatransfer::dnd::DropTargetEvent& dte ) override;
    void SAL_CALL dragOver( const css::datatransfer::dnd::DropTargetDragEvent& dtde ) override;
    void SAL_CALL dropActionChanged( const css::datatransfer::dnd::DropTargetDragEvent& dtde ) override;
};

}  // namespace vcl::unohelper

#endif // INCLUDED_VCL_DNDHELP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
