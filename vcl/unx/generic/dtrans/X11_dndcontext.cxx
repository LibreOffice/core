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
    ::Window aDropWindow,
    SelectionManager& rManager ) :
        m_aDropWindow( aDropWindow ),
        m_xManager( &rManager )
{
}

DropTargetDropContext::~DropTargetDropContext()
{
}

void DropTargetDropContext::acceptDrop( sal_Int8 dragOperation )
{
    m_xManager->accept( dragOperation, m_aDropWindow );
}

void DropTargetDropContext::rejectDrop()
{
    m_xManager->reject( m_aDropWindow );
}

void DropTargetDropContext::dropComplete( sal_Bool success )
{
    m_xManager->dropComplete( success, m_aDropWindow );
}

/*
 *  DropTargetDragContext
 */

DropTargetDragContext::DropTargetDragContext(
    ::Window aDropWindow,
    SelectionManager& rManager ) :
        m_aDropWindow( aDropWindow ),
        m_xManager( &rManager )
{
}

DropTargetDragContext::~DropTargetDragContext()
{
}

void DropTargetDragContext::acceptDrag( sal_Int8 dragOperation )
{
    m_xManager->accept( dragOperation, m_aDropWindow );
}

void DropTargetDragContext::rejectDrag()
{
    m_xManager->reject( m_aDropWindow );
}

/*
 *  DragSourceContext
 */

DragSourceContext::DragSourceContext(
    ::Window aDropWindow,
    SelectionManager& rManager ) :
        m_aDropWindow( aDropWindow ),
        m_xManager( &rManager )
{
}

DragSourceContext::~DragSourceContext()
{
}

sal_Int32 DragSourceContext::getCurrentCursor()
{
    return m_xManager->getCurrentCursor();
}

void DragSourceContext::setCursor( sal_Int32 cursorId )
{
    m_xManager->setCursor( cursorId, m_aDropWindow );
}

void DragSourceContext::setImage( sal_Int32 )
{
}

void DragSourceContext::transferablesFlavorsChanged()
{
    m_xManager->transferablesFlavorsChanged();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
