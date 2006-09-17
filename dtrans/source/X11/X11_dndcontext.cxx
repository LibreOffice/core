/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: X11_dndcontext.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 16:52:50 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dtrans.hxx"

#include <X11_dndcontext.hxx>
#include <X11_selection.hxx>

using namespace cppu;
using namespace x11;

/*
 *  DropTargetDropContext
 */

DropTargetDropContext::DropTargetDropContext(
    Window aDropWindow,
    Time aTimestamp,
    SelectionManager& rManager ) :
        m_aDropWindow( aDropWindow ),
        m_nTimestamp( aTimestamp ),
        m_rManager( rManager ),
        m_xManagerRef( static_cast< OWeakObject* >(&rManager) )
{
}

DropTargetDropContext::~DropTargetDropContext()
{
}

void DropTargetDropContext::acceptDrop( sal_Int8 dragOperation ) throw()
{
    m_rManager.accept( dragOperation, m_aDropWindow, m_nTimestamp );
}

void DropTargetDropContext::rejectDrop() throw()
{
    m_rManager.reject( m_aDropWindow, m_nTimestamp );
}

void DropTargetDropContext::dropComplete( sal_Bool success ) throw()
{
    m_rManager.dropComplete( success, m_aDropWindow, m_nTimestamp );
}


/*
 *  DropTargetDragContext
 */

DropTargetDragContext::DropTargetDragContext(
    Window aDropWindow,
    Time aTimestamp,
    SelectionManager& rManager ) :
        m_aDropWindow( aDropWindow ),
        m_nTimestamp( aTimestamp ),
        m_rManager( rManager ),
        m_xManagerRef( static_cast< OWeakObject* >(&rManager) )
{
}

DropTargetDragContext::~DropTargetDragContext()
{
}

void DropTargetDragContext::acceptDrag( sal_Int8 dragOperation ) throw()
{
    m_rManager.accept( dragOperation, m_aDropWindow, m_nTimestamp );
}

void DropTargetDragContext::rejectDrag() throw()
{
    m_rManager.reject( m_aDropWindow, m_nTimestamp );
}

/*
 *  DragSourceContext
 */

DragSourceContext::DragSourceContext(
    Window aDropWindow,
    Time aTimestamp,
    SelectionManager& rManager ) :
        m_aDropWindow( aDropWindow ),
        m_nTimestamp( aTimestamp ),
        m_rManager( rManager ),
        m_xManagerRef( static_cast< OWeakObject* >(&rManager) )
{
}

DragSourceContext::~DragSourceContext()
{
}

sal_Int32 DragSourceContext::getCurrentCursor() throw()
{
    return m_rManager.getCurrentCursor();
}

void DragSourceContext::setCursor( sal_Int32 cursorId ) throw()
{
    m_rManager.setCursor( cursorId, m_aDropWindow, m_nTimestamp );
}

void DragSourceContext::setImage( sal_Int32 imageId ) throw()
{
    m_rManager.setImage( imageId, m_aDropWindow, m_nTimestamp );
}

void DragSourceContext::transferablesFlavorsChanged() throw()
{
    m_rManager.transferablesFlavorsChanged();
}
