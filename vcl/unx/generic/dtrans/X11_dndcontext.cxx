/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <X11_dndcontext.hxx>
#include <X11_selection.hxx>

using namespace cppu;
using namespace x11;

/*
 *  DropTargetDropContext
 */

DropTargetDropContext::DropTargetDropContext(
    XLIB_Window aDropWindow,
    XLIB_Time aTimestamp,
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
    XLIB_Window aDropWindow,
    XLIB_Time aTimestamp,
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
    XLIB_Window aDropWindow,
    XLIB_Time aTimestamp,
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
