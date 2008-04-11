/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sourcelistener.hxx,v $
 * $Revision: 1.4 $
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
