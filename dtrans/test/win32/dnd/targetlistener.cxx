/*************************************************************************
 *
 *  $RCSfile: targetlistener.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 14:05:43 $
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
