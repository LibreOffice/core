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

namespace com {
namespace sun {
namespace star {

namespace datatransfer {
namespace dnd {
    struct DragGestureEvent;
    struct DragSourceDragEvent;
    struct DragSourceDropEvent;
    struct DragSourceEvent;
    struct DropTargetDragEvent;
    struct DropTargetDragEnterEvent;
    struct DropTargetDropEvent;
    struct DropTargetEvent;
}}

}}}

namespace vcl
{

namespace unohelper
{

class VCL_DLLPUBLIC DragAndDropClient
{
public:
    virtual ~DragAndDropClient();

    // ::com::sun::star::datatransfer::dnd::XDragGestureListener
    virtual void dragGestureRecognized( const ::com::sun::star::datatransfer::dnd::DragGestureEvent& dge )
        throw (::com::sun::star::uno::RuntimeException,
               std::exception);

    // ::com::sun::star::datatransfer::dnd::XDragSourceListener
    virtual void dragDropEnd(const ::com::sun::star::datatransfer::dnd::DragSourceDropEvent& dsde )
        throw (::com::sun::star::uno::RuntimeException,
               std::exception);

    // ::com::sun::star::datatransfer::dnd::XDropTargetListener
    virtual void drop( const ::com::sun::star::datatransfer::dnd::DropTargetDropEvent& dtde )
        throw (::com::sun::star::uno::RuntimeException,
               std::exception);
    virtual void dragEnter( const ::com::sun::star::datatransfer::dnd::DropTargetDragEnterEvent& dtdee )
        throw (::com::sun::star::uno::RuntimeException,
               std::exception);
    virtual void dragExit( const ::com::sun::star::datatransfer::dnd::DropTargetEvent& dte )
        throw (::com::sun::star::uno::RuntimeException,
               std::exception);
    virtual void dragOver( const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& dtde )
        throw (::com::sun::star::uno::RuntimeException,
               std::exception);
};

class VCL_DLLPUBLIC DragAndDropWrapper :
                            public ::com::sun::star::datatransfer::dnd::XDragGestureListener,
                            public ::com::sun::star::datatransfer::dnd::XDragSourceListener,
                            public ::com::sun::star::datatransfer::dnd::XDropTargetListener,
                            public ::cppu::OWeakObject
{
private:
    DragAndDropClient* mpClient;

public:
                    DragAndDropWrapper( DragAndDropClient* pClient );
                    virtual ~DragAndDropWrapper();

    // ::com::sun::star::uno::XInterface
    ::com::sun::star::uno::Any                  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void                                        SAL_CALL acquire() throw() override  { OWeakObject::acquire(); }
    void                                        SAL_CALL release() throw() override  { OWeakObject::release(); }

    // ::com::sun::star::lang::XEventListener
    void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::datatransfer::dnd::XDragGestureListener
    void SAL_CALL dragGestureRecognized( const ::com::sun::star::datatransfer::dnd::DragGestureEvent& dge ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::datatransfer::dnd::XDragSourceListener
    void SAL_CALL dragDropEnd( const ::com::sun::star::datatransfer::dnd::DragSourceDropEvent& dsde ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL dragEnter( const ::com::sun::star::datatransfer::dnd::DragSourceDragEvent& dsde ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL dragExit( const ::com::sun::star::datatransfer::dnd::DragSourceEvent& dse ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL dragOver( const ::com::sun::star::datatransfer::dnd::DragSourceDragEvent& dsde ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL dropActionChanged( const ::com::sun::star::datatransfer::dnd::DragSourceDragEvent& dsde ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::datatransfer::dnd::XDropTargetListener
    void SAL_CALL drop( const ::com::sun::star::datatransfer::dnd::DropTargetDropEvent& dtde ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL dragEnter( const ::com::sun::star::datatransfer::dnd::DropTargetDragEnterEvent& dtdee ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL dragExit( const ::com::sun::star::datatransfer::dnd::DropTargetEvent& dte ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL dragOver( const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& dtde ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL dropActionChanged( const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& dtde ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
};

}}  // namespace vcl::unohelper

#endif // INCLUDED_VCL_DNDHELP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
