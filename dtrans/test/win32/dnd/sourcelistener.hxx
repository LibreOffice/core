/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sourcelistener.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 18:32:27 $
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

#ifndef _SOURCELISTENER_HXX_
#define _SOURCELISTENER_HXX_

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/datatransfer/dnd/XDragSourceListener.hpp>
#include <com/sun/star/datatransfer/dnd/DragSourceDropEvent.hpp>
#include <com/sun/star/datatransfer/dnd/DragSourceDragEvent.hpp>

using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::dnd;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

class DragSourceListener: public WeakImplHelper1<XDragSourceListener>
{
    // this is a window where droped data are shown as text (only text)
public:
    DragSourceListener( );
    ~DragSourceListener();

    virtual void SAL_CALL disposing( const EventObject& Source )
        throw(RuntimeException);

    virtual void SAL_CALL dragDropEnd( const DragSourceDropEvent& dsde )
        throw(RuntimeException);
    virtual void SAL_CALL dragEnter( const DragSourceDragEvent& dsde )
        throw(RuntimeException);
    virtual void SAL_CALL dragExit( const DragSourceEvent& dse )
        throw(RuntimeException);
    virtual void SAL_CALL dragOver( const DragSourceDragEvent& dsde )
        throw(RuntimeException);
    virtual void SAL_CALL dropActionChanged( const DragSourceDragEvent& dsde )
        throw(RuntimeException);

};

#endif
