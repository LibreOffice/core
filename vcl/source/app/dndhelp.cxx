/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dndhelp.cxx,v $
 * $Revision: 1.9 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"



#include <vcl/dndhelp.hxx>

using namespace ::com::sun::star;

vcl::unohelper::DragAndDropClient::~DragAndDropClient() {}

void vcl::unohelper::DragAndDropClient::dragGestureRecognized( const ::com::sun::star::datatransfer::dnd::DragGestureEvent& /*dge*/ ) throw (::com::sun::star::uno::RuntimeException)
{
}

void vcl::unohelper::DragAndDropClient::dragDropEnd( const ::com::sun::star::datatransfer::dnd::DragSourceDropEvent& /*dsde*/ ) throw (::com::sun::star::uno::RuntimeException)
{
}

void vcl::unohelper::DragAndDropClient::dragEnter( const ::com::sun::star::datatransfer::dnd::DragSourceDragEvent& /*dsde*/ ) throw (::com::sun::star::uno::RuntimeException)
{
}

void vcl::unohelper::DragAndDropClient::dragExit( const ::com::sun::star::datatransfer::dnd::DragSourceEvent& /*dse*/ ) throw (::com::sun::star::uno::RuntimeException)
{
}

void vcl::unohelper::DragAndDropClient::dragOver( const ::com::sun::star::datatransfer::dnd::DragSourceDragEvent& /*dsde*/ ) throw (::com::sun::star::uno::RuntimeException)
{
}

void vcl::unohelper::DragAndDropClient::dropActionChanged( const ::com::sun::star::datatransfer::dnd::DragSourceDragEvent& /*dsde*/ ) throw (::com::sun::star::uno::RuntimeException)
{
}

void vcl::unohelper::DragAndDropClient::drop( const ::com::sun::star::datatransfer::dnd::DropTargetDropEvent& /*dtde*/ ) throw (::com::sun::star::uno::RuntimeException)
{
}

void vcl::unohelper::DragAndDropClient::dragEnter( const ::com::sun::star::datatransfer::dnd::DropTargetDragEnterEvent& /*dtdee*/ ) throw (::com::sun::star::uno::RuntimeException)
{
}

void vcl::unohelper::DragAndDropClient::dragExit( const ::com::sun::star::datatransfer::dnd::DropTargetEvent& /*dte*/ ) throw (::com::sun::star::uno::RuntimeException)
{
}

void vcl::unohelper::DragAndDropClient::dragOver( const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& /*dtde*/ ) throw (::com::sun::star::uno::RuntimeException)
{
}

void vcl::unohelper::DragAndDropClient::dropActionChanged( const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& /*dtde*/ ) throw (::com::sun::star::uno::RuntimeException)
{
}

// =======================================================================

vcl::unohelper::DragAndDropWrapper::DragAndDropWrapper( DragAndDropClient* pClient )
{
    mpClient = pClient;
}

vcl::unohelper::DragAndDropWrapper::~DragAndDropWrapper()
{
}

// uno::XInterface
uno::Any vcl::unohelper::DragAndDropWrapper::queryInterface( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                            SAL_STATIC_CAST( ::com::sun::star::lang::XEventListener*, (::com::sun::star::datatransfer::dnd::XDragGestureListener*)this ),
                            SAL_STATIC_CAST( ::com::sun::star::datatransfer::dnd::XDragGestureListener*, this ),
                            SAL_STATIC_CAST( ::com::sun::star::datatransfer::dnd::XDragSourceListener*, this ),
                            SAL_STATIC_CAST( ::com::sun::star::datatransfer::dnd::XDropTargetListener*, this ) );
    return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
}

// ::com::sun::star::lang::XEventListener
void vcl::unohelper::DragAndDropWrapper::disposing( const ::com::sun::star::lang::EventObject& rEvent ) throw (::com::sun::star::uno::RuntimeException)
{
    // Empty Source means it's the client, because the client is not a XInterface
    if ( !rEvent.Source.is() )
        mpClient = NULL;
}


// ::com::sun::star::datatransfer::dnd::XDragGestureListener
void vcl::unohelper::DragAndDropWrapper::dragGestureRecognized( const ::com::sun::star::datatransfer::dnd::DragGestureEvent& rDGE ) throw (::com::sun::star::uno::RuntimeException)
{
    if ( mpClient )
        mpClient->dragGestureRecognized( rDGE );
}

// ::com::sun::star::datatransfer::dnd::XDragSourceListener
void vcl::unohelper::DragAndDropWrapper::dragDropEnd( const ::com::sun::star::datatransfer::dnd::DragSourceDropEvent& rDSDE ) throw (::com::sun::star::uno::RuntimeException)
{
    if ( mpClient )
        mpClient->dragDropEnd( rDSDE );
}

void vcl::unohelper::DragAndDropWrapper::dragEnter( const ::com::sun::star::datatransfer::dnd::DragSourceDragEvent& dsde ) throw (::com::sun::star::uno::RuntimeException)
{
    if ( mpClient )
        mpClient->dragEnter( dsde );
}

void vcl::unohelper::DragAndDropWrapper::dragExit( const ::com::sun::star::datatransfer::dnd::DragSourceEvent& dse ) throw (::com::sun::star::uno::RuntimeException)
{
    if ( mpClient )
        mpClient->dragExit( dse );
}

void vcl::unohelper::DragAndDropWrapper::dragOver( const ::com::sun::star::datatransfer::dnd::DragSourceDragEvent& dsde ) throw (::com::sun::star::uno::RuntimeException)
{
    if ( mpClient )
        mpClient->dragOver( dsde );
}

void vcl::unohelper::DragAndDropWrapper::dropActionChanged( const ::com::sun::star::datatransfer::dnd::DragSourceDragEvent& dsde ) throw (::com::sun::star::uno::RuntimeException)
{
    if ( mpClient )
        mpClient->dropActionChanged( dsde );
}

// ::com::sun::star::datatransfer::dnd::XDropTargetListener
void vcl::unohelper::DragAndDropWrapper::drop( const ::com::sun::star::datatransfer::dnd::DropTargetDropEvent& rDTDE ) throw (::com::sun::star::uno::RuntimeException)
{
    if ( mpClient )
        mpClient->drop( rDTDE );
}

void vcl::unohelper::DragAndDropWrapper::dragEnter( const ::com::sun::star::datatransfer::dnd::DropTargetDragEnterEvent& rDTDEE ) throw (::com::sun::star::uno::RuntimeException)
{
    if ( mpClient )
        mpClient->dragEnter( rDTDEE );
}

void vcl::unohelper::DragAndDropWrapper::dragExit( const ::com::sun::star::datatransfer::dnd::DropTargetEvent& dte ) throw (::com::sun::star::uno::RuntimeException)
{
    if ( mpClient )
        mpClient->dragExit( dte );
}

void vcl::unohelper::DragAndDropWrapper::dragOver( const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& rDTDE ) throw (::com::sun::star::uno::RuntimeException)
{
    if ( mpClient )
        mpClient->dragOver( rDTDE );
}

void vcl::unohelper::DragAndDropWrapper::dropActionChanged( const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& rDTDE ) throw (::com::sun::star::uno::RuntimeException)
{
    if ( mpClient )
        mpClient->dropActionChanged( rDTDE );
}

