/*************************************************************************
 *
 *  $RCSfile: dndhelp.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mt $ $Date: 2001-05-11 07:21:20 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

#include <dndhelp.hxx>

using namespace ::com::sun::star;


void vcl::unohelper::DragAndDropClient::dragGestureRecognized( const ::com::sun::star::datatransfer::dnd::DragGestureEvent& dge ) throw (::com::sun::star::uno::RuntimeException)
{
}

void vcl::unohelper::DragAndDropClient::dragDropEnd( const ::com::sun::star::datatransfer::dnd::DragSourceDropEvent& dsde ) throw (::com::sun::star::uno::RuntimeException)
{
}

void vcl::unohelper::DragAndDropClient::dragEnter( const ::com::sun::star::datatransfer::dnd::DragSourceDragEvent& dsde ) throw (::com::sun::star::uno::RuntimeException)
{
}

void vcl::unohelper::DragAndDropClient::dragExit( const ::com::sun::star::datatransfer::dnd::DragSourceEvent& dse ) throw (::com::sun::star::uno::RuntimeException)
{
}

void vcl::unohelper::DragAndDropClient::dragOver( const ::com::sun::star::datatransfer::dnd::DragSourceDragEvent& dsde ) throw (::com::sun::star::uno::RuntimeException)
{
}

void vcl::unohelper::DragAndDropClient::dropActionChanged( const ::com::sun::star::datatransfer::dnd::DragSourceDragEvent& dsde ) throw (::com::sun::star::uno::RuntimeException)
{
}

void vcl::unohelper::DragAndDropClient::drop( const ::com::sun::star::datatransfer::dnd::DropTargetDropEvent& dtde ) throw (::com::sun::star::uno::RuntimeException)
{
}

void vcl::unohelper::DragAndDropClient::dragEnter( const ::com::sun::star::datatransfer::dnd::DropTargetDragEnterEvent& dtdee ) throw (::com::sun::star::uno::RuntimeException)
{
}

void vcl::unohelper::DragAndDropClient::dragExit( const ::com::sun::star::datatransfer::dnd::DropTargetEvent& dte ) throw (::com::sun::star::uno::RuntimeException)
{
}

void vcl::unohelper::DragAndDropClient::dragOver( const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& dtde ) throw (::com::sun::star::uno::RuntimeException)
{
}

void vcl::unohelper::DragAndDropClient::dropActionChanged( const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& dtde ) throw (::com::sun::star::uno::RuntimeException)
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
                            SAL_STATIC_CAST( ::com::sun::star::datatransfer::dnd::XDragGestureListener*, this ),
                            SAL_STATIC_CAST( ::com::sun::star::datatransfer::dnd::XDragSourceListener*, this ),
                            SAL_STATIC_CAST( ::com::sun::star::datatransfer::dnd::XDropTargetListener*, this ) );
    return (aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ));
}

// ::com::sun::star::lang::XEventListener
void vcl::unohelper::DragAndDropWrapper::disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException)
{
}


// ::com::sun::star::datatransfer::dnd::XDragGestureListener
void vcl::unohelper::DragAndDropWrapper::dragGestureRecognized( const ::com::sun::star::datatransfer::dnd::DragGestureEvent& rDGE ) throw (::com::sun::star::uno::RuntimeException)
{
    mpClient->dragGestureRecognized( rDGE );
}

// ::com::sun::star::datatransfer::dnd::XDragSourceListener
void vcl::unohelper::DragAndDropWrapper::dragDropEnd( const ::com::sun::star::datatransfer::dnd::DragSourceDropEvent& rDSDE ) throw (::com::sun::star::uno::RuntimeException)
{
    mpClient->dragDropEnd( rDSDE );
}

void vcl::unohelper::DragAndDropWrapper::dragEnter( const ::com::sun::star::datatransfer::dnd::DragSourceDragEvent& dsde ) throw (::com::sun::star::uno::RuntimeException)
{
    mpClient->dragEnter( dsde );
}

void vcl::unohelper::DragAndDropWrapper::dragExit( const ::com::sun::star::datatransfer::dnd::DragSourceEvent& dse ) throw (::com::sun::star::uno::RuntimeException)
{
    mpClient->dragExit( dse );
}

void vcl::unohelper::DragAndDropWrapper::dragOver( const ::com::sun::star::datatransfer::dnd::DragSourceDragEvent& dsde ) throw (::com::sun::star::uno::RuntimeException)
{
    mpClient->dragOver( dsde );
}

void vcl::unohelper::DragAndDropWrapper::dropActionChanged( const ::com::sun::star::datatransfer::dnd::DragSourceDragEvent& dsde ) throw (::com::sun::star::uno::RuntimeException)
{
    mpClient->dropActionChanged( dsde );
}

// ::com::sun::star::datatransfer::dnd::XDropTargetListener
void vcl::unohelper::DragAndDropWrapper::drop( const ::com::sun::star::datatransfer::dnd::DropTargetDropEvent& rDTDE ) throw (::com::sun::star::uno::RuntimeException)
{
    mpClient->drop( rDTDE );
}

void vcl::unohelper::DragAndDropWrapper::dragEnter( const ::com::sun::star::datatransfer::dnd::DropTargetDragEnterEvent& rDTDEE ) throw (::com::sun::star::uno::RuntimeException)
{
    mpClient->dragEnter( rDTDEE );
}

void vcl::unohelper::DragAndDropWrapper::dragExit( const ::com::sun::star::datatransfer::dnd::DropTargetEvent& dte ) throw (::com::sun::star::uno::RuntimeException)
{
    mpClient->dragExit( dte );
}

void vcl::unohelper::DragAndDropWrapper::dragOver( const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& rDTDE ) throw (::com::sun::star::uno::RuntimeException)
{
    mpClient->dragOver( rDTDE );
}

void vcl::unohelper::DragAndDropWrapper::dropActionChanged( const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& rDTDE ) throw (::com::sun::star::uno::RuntimeException)
{
    mpClient->dropActionChanged( rDTDE );
}

