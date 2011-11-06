/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
