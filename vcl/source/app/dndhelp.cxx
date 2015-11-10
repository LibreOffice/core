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

#include <vcl/dndhelp.hxx>

#include <cppuhelper/queryinterface.hxx>

using namespace ::com::sun::star;

vcl::unohelper::DragAndDropClient::~DragAndDropClient() {}

void vcl::unohelper::DragAndDropClient::dragGestureRecognized( const ::com::sun::star::datatransfer::dnd::DragGestureEvent& /*dge*/ )
    throw (::com::sun::star::uno::RuntimeException,
           std::exception)
{
}

void vcl::unohelper::DragAndDropClient::dragDropEnd( const ::com::sun::star::datatransfer::dnd::DragSourceDropEvent& /*dsde*/ )
    throw (::com::sun::star::uno::RuntimeException,
           std::exception)
{
}

void vcl::unohelper::DragAndDropClient::drop( const ::com::sun::star::datatransfer::dnd::DropTargetDropEvent& /*dtde*/ )
    throw (::com::sun::star::uno::RuntimeException,
           std::exception)
{
}

void vcl::unohelper::DragAndDropClient::dragEnter( const ::com::sun::star::datatransfer::dnd::DropTargetDragEnterEvent& /*dtdee*/ )
    throw (::com::sun::star::uno::RuntimeException,
           std::exception)
{
}

void vcl::unohelper::DragAndDropClient::dragExit( const ::com::sun::star::datatransfer::dnd::DropTargetEvent& /*dte*/ )
    throw (::com::sun::star::uno::RuntimeException,
           std::exception)
{
}

void vcl::unohelper::DragAndDropClient::dragOver( const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& /*dtde*/ )
    throw (::com::sun::star::uno::RuntimeException,
           std::exception)
{
}

vcl::unohelper::DragAndDropWrapper::DragAndDropWrapper( DragAndDropClient* pClient )
{
    mpClient = pClient;
}

vcl::unohelper::DragAndDropWrapper::~DragAndDropWrapper()
{
}

// uno::XInterface
uno::Any vcl::unohelper::DragAndDropWrapper::queryInterface( const uno::Type & rType ) throw(uno::RuntimeException, std::exception)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                            (static_cast< ::com::sun::star::lang::XEventListener* >( static_cast<com::sun::star::datatransfer::dnd::XDragGestureListener*>(this)) ),
                            (static_cast< ::com::sun::star::datatransfer::dnd::XDragGestureListener* >(this)),
                            (static_cast< ::com::sun::star::datatransfer::dnd::XDragSourceListener* >(this)),
                            (static_cast< ::com::sun::star::datatransfer::dnd::XDropTargetListener* >(this)) );
    return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
}

// ::com::sun::star::lang::XEventListener
void vcl::unohelper::DragAndDropWrapper::disposing( const ::com::sun::star::lang::EventObject& rEvent ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    // Empty Source means it's the client, because the client is not a XInterface
    if ( !rEvent.Source.is() )
        mpClient = nullptr;
}

// ::com::sun::star::datatransfer::dnd::XDragGestureListener
void vcl::unohelper::DragAndDropWrapper::dragGestureRecognized( const ::com::sun::star::datatransfer::dnd::DragGestureEvent& rDGE ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    if ( mpClient )
        mpClient->dragGestureRecognized( rDGE );
}

// ::com::sun::star::datatransfer::dnd::XDragSourceListener
void vcl::unohelper::DragAndDropWrapper::dragDropEnd( const ::com::sun::star::datatransfer::dnd::DragSourceDropEvent& rDSDE ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    if ( mpClient )
        mpClient->dragDropEnd( rDSDE );
}

void vcl::unohelper::DragAndDropWrapper::dragEnter( const ::com::sun::star::datatransfer::dnd::DragSourceDragEvent& ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
}

void vcl::unohelper::DragAndDropWrapper::dragExit( const ::com::sun::star::datatransfer::dnd::DragSourceEvent& ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
}

void vcl::unohelper::DragAndDropWrapper::dragOver( const ::com::sun::star::datatransfer::dnd::DragSourceDragEvent& ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
}

void vcl::unohelper::DragAndDropWrapper::dropActionChanged( const ::com::sun::star::datatransfer::dnd::DragSourceDragEvent& ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
}

// ::com::sun::star::datatransfer::dnd::XDropTargetListener
void vcl::unohelper::DragAndDropWrapper::drop( const ::com::sun::star::datatransfer::dnd::DropTargetDropEvent& rDTDE ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    if ( mpClient )
        mpClient->drop( rDTDE );
}

void vcl::unohelper::DragAndDropWrapper::dragEnter( const ::com::sun::star::datatransfer::dnd::DropTargetDragEnterEvent& rDTDEE ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    if ( mpClient )
        mpClient->dragEnter( rDTDEE );
}

void vcl::unohelper::DragAndDropWrapper::dragExit( const ::com::sun::star::datatransfer::dnd::DropTargetEvent& dte ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    if ( mpClient )
        mpClient->dragExit( dte );
}

void vcl::unohelper::DragAndDropWrapper::dragOver( const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& rDTDE ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    if ( mpClient )
        mpClient->dragOver( rDTDE );
}

void vcl::unohelper::DragAndDropWrapper::dropActionChanged( const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
