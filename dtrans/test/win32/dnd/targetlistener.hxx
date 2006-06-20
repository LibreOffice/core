/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: targetlistener.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 06:09:01 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _TARGETLISTENER_HXX_
#define _TARGETLISTENER_HXX_

#if defined _MSC_VER
#pragma warning(push,1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/datatransfer/dnd/XDropTargetListener.hpp>
#include <com/sun/star/datatransfer/dnd/DropTargetDropEvent.hpp>
#include <com/sun/star/datatransfer/dnd/DropTargetDragEvent.hpp>
#include <com/sun/star/datatransfer/dnd/DropTargetDragEnterEvent.hpp>

using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::dnd;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

class DropTargetListener: public WeakImplHelper1<XDropTargetListener>
{
    // this is a window where droped data are shown as text (only text)
    HWND m_hEdit;
public:
    DropTargetListener( HWND hEdit);
    ~DropTargetListener();

    virtual void SAL_CALL disposing( const EventObject& Source )
        throw(RuntimeException);


    virtual void SAL_CALL drop( const DropTargetDropEvent& dtde )
        throw(RuntimeException);
    virtual void SAL_CALL dragEnter( const DropTargetDragEnterEvent& dtde )
        throw(RuntimeException);
    virtual void SAL_CALL dragExit( const DropTargetEvent& dte )
        throw(RuntimeException);
    virtual void SAL_CALL dragOver( const DropTargetDragEvent& dtde )
        throw(RuntimeException);
    virtual void SAL_CALL dropActionChanged( const DropTargetDragEvent& dtde )
        throw(RuntimeException);
};

#endif
