/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: targetlistener.cxx,v $
 * $Revision: 1.8 $
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
#include "precompiled_dtrans.hxx"


#include "targetlistener.hxx"
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#include <com/sun/star/datatransfer/DataFlavor.hpp>

//using namespace com::sun::star::datatransfer::dnd;
using namespace com::sun::star::datatransfer::dnd::DNDConstants;
using namespace com::sun::star::datatransfer;
using namespace rtl;

DropTargetListener::DropTargetListener(HWND hEdit):m_hEdit( hEdit)
{
}
DropTargetListener::~DropTargetListener()
{
}

void SAL_CALL DropTargetListener::disposing( const EventObject& Source )
        throw(RuntimeException)
{

}



void SAL_CALL DropTargetListener::drop( const DropTargetDropEvent& e )
    throw(RuntimeException)
{
//  e.Context->dropComplete( sal_True);
//  e.Context->acceptDrop( ACTION_COPY);
    e.Context->rejectDrop();

    // if the Transferable contains text, then we send it to the edit window
//  Sequence<DataFlavor> flavors= e.Transferable->getTransferDataFlavors();
//  DataFlavor aFlavor;
//  for( int i=0; i < flavors.getLength(); i++)
//      aFlavor= flavors[4];

    DataFlavor flavor( OUString(OUString::createFromAscii("text/plain;charset=windows-1252")),
        OUString(L"Text plain"), getCppuType( ( Sequence<sal_Int8>*)0 ) );

    Any anyData= e.Transferable->getTransferData( flavor);
    Sequence<sal_Int8> seq= *( Sequence<sal_Int8>*)anyData.getValue();
    SendMessage( m_hEdit, WM_SETTEXT, 0, (LPARAM) seq.getConstArray() );
}

void SAL_CALL DropTargetListener::dragEnter( const DropTargetDragEnterEvent& dtde )
     throw(RuntimeException)
{
    //If one drags something that is not moveable
    if( !(dtde.SourceActions & dtde.DropAction) )
        dtde.Context->acceptDrag( ACTION_COPY);

//  dtde.Context->rejectDrag( );

}

void SAL_CALL DropTargetListener::dragExit( const DropTargetEvent& dte )
     throw(RuntimeException)
{
}

void SAL_CALL DropTargetListener::dragOver( const DropTargetDragEvent& dtde )
     throw(RuntimeException)
{
    if( !(dtde.SourceActions & dtde.DropAction) )
        dtde.Context->acceptDrag( ACTION_COPY);
}

void SAL_CALL DropTargetListener::dropActionChanged( const DropTargetDragEvent& dtde )
    throw(RuntimeException)
{
}
