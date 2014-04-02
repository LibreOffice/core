/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

void DropTargetDropContext::acceptDrop( sal_Int8 dragOperation ) throw(std::exception)
{
    m_rManager.accept( dragOperation, m_aDropWindow, m_nTimestamp );
}

void DropTargetDropContext::rejectDrop() throw(std::exception)
{
    m_rManager.reject( m_aDropWindow, m_nTimestamp );
}

void DropTargetDropContext::dropComplete( sal_Bool success ) throw(std::exception)
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

void DropTargetDragContext::acceptDrag( sal_Int8 dragOperation ) throw(std::exception)
{
    m_rManager.accept( dragOperation, m_aDropWindow, m_nTimestamp );
}

void DropTargetDragContext::rejectDrag() throw(std::exception)
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

sal_Int32 DragSourceContext::getCurrentCursor() throw(std::exception)
{
    return m_rManager.getCurrentCursor();
}

void DragSourceContext::setCursor( sal_Int32 cursorId ) throw(std::exception)
{
    m_rManager.setCursor( cursorId, m_aDropWindow, m_nTimestamp );
}

void DragSourceContext::setImage( sal_Int32 imageId ) throw(std::exception)
{
    m_rManager.setImage( imageId, m_aDropWindow, m_nTimestamp );
}

void DragSourceContext::transferablesFlavorsChanged() throw(std::exception)
{
    m_rManager.transferablesFlavorsChanged();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
