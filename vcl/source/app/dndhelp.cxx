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

void vcl::unohelper::DragAndDropClient::dragGestureRecognized( const css::datatransfer::dnd::DragGestureEvent& /*dge*/ )
    throw (css::uno::RuntimeException,
           std::exception)
{
}

void vcl::unohelper::DragAndDropClient::dragDropEnd( const css::datatransfer::dnd::DragSourceDropEvent& /*dsde*/ )
    throw (css::uno::RuntimeException,
           std::exception)
{
}

void vcl::unohelper::DragAndDropClient::drop( const css::datatransfer::dnd::DropTargetDropEvent& /*dtde*/ )
    throw (css::uno::RuntimeException,
           std::exception)
{
}

void vcl::unohelper::DragAndDropClient::dragEnter( const css::datatransfer::dnd::DropTargetDragEnterEvent& /*dtdee*/ )
    throw (css::uno::RuntimeException,
           std::exception)
{
}

void vcl::unohelper::DragAndDropClient::dragExit( const css::datatransfer::dnd::DropTargetEvent& /*dte*/ )
    throw (css::uno::RuntimeException,
           std::exception)
{
}

void vcl::unohelper::DragAndDropClient::dragOver( const css::datatransfer::dnd::DropTargetDragEvent& /*dtde*/ )
    throw (css::uno::RuntimeException,
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
                            (static_cast< css::lang::XEventListener* >( static_cast<css::datatransfer::dnd::XDragGestureListener*>(this)) ),
                            (static_cast< css::datatransfer::dnd::XDragGestureListener* >(this)),
                            (static_cast< css::datatransfer::dnd::XDragSourceListener* >(this)),
                            (static_cast< css::datatransfer::dnd::XDropTargetListener* >(this)) );
    return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
}

// css::lang::XEventListener
void vcl::unohelper::DragAndDropWrapper::disposing( const css::lang::EventObject& rEvent ) throw (css::uno::RuntimeException, std::exception)
{
    // Empty Source means it's the client, because the client is not a XInterface
    if ( !rEvent.Source.is() )
        mpClient = nullptr;
}

// css::datatransfer::dnd::XDragGestureListener
void vcl::unohelper::DragAndDropWrapper::dragGestureRecognized( const css::datatransfer::dnd::DragGestureEvent& rDGE ) throw (css::uno::RuntimeException, std::exception)
{
    if ( mpClient )
        mpClient->dragGestureRecognized( rDGE );
}

// css::datatransfer::dnd::XDragSourceListener
void vcl::unohelper::DragAndDropWrapper::dragDropEnd( const css::datatransfer::dnd::DragSourceDropEvent& rDSDE ) throw (css::uno::RuntimeException, std::exception)
{
    if ( mpClient )
        mpClient->dragDropEnd( rDSDE );
}

void vcl::unohelper::DragAndDropWrapper::dragEnter( const css::datatransfer::dnd::DragSourceDragEvent& ) throw (css::uno::RuntimeException, std::exception)
{
}

void vcl::unohelper::DragAndDropWrapper::dragExit( const css::datatransfer::dnd::DragSourceEvent& ) throw (css::uno::RuntimeException, std::exception)
{
}

void vcl::unohelper::DragAndDropWrapper::dragOver( const css::datatransfer::dnd::DragSourceDragEvent& ) throw (css::uno::RuntimeException, std::exception)
{
}

void vcl::unohelper::DragAndDropWrapper::dropActionChanged( const css::datatransfer::dnd::DragSourceDragEvent& ) throw (css::uno::RuntimeException, std::exception)
{
}

// css::datatransfer::dnd::XDropTargetListener
void vcl::unohelper::DragAndDropWrapper::drop( const css::datatransfer::dnd::DropTargetDropEvent& rDTDE ) throw (css::uno::RuntimeException, std::exception)
{
    if ( mpClient )
        mpClient->drop( rDTDE );
}

void vcl::unohelper::DragAndDropWrapper::dragEnter( const css::datatransfer::dnd::DropTargetDragEnterEvent& rDTDEE ) throw (css::uno::RuntimeException, std::exception)
{
    if ( mpClient )
        mpClient->dragEnter( rDTDEE );
}

void vcl::unohelper::DragAndDropWrapper::dragExit( const css::datatransfer::dnd::DropTargetEvent& dte ) throw (css::uno::RuntimeException, std::exception)
{
    if ( mpClient )
        mpClient->dragExit( dte );
}

void vcl::unohelper::DragAndDropWrapper::dragOver( const css::datatransfer::dnd::DropTargetDragEvent& rDTDE ) throw (css::uno::RuntimeException, std::exception)
{
    if ( mpClient )
        mpClient->dragOver( rDTDE );
}

void vcl::unohelper::DragAndDropWrapper::dropActionChanged( const css::datatransfer::dnd::DropTargetDragEvent& ) throw (css::uno::RuntimeException, std::exception)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
