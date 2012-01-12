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
