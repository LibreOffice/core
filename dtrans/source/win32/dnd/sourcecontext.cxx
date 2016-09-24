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

#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>

#include "sourcecontext.hxx"

using namespace com::sun::star::datatransfer::dnd;
using namespace com::sun::star::datatransfer::dnd::DNDConstants;

SourceContext::SourceContext( DragSource* pSource,
                             const Reference<XDragSourceListener>& listener):
        WeakComponentImplHelper<XDragSourceContext>(m_mutex),
        m_pDragSource( pSource),
        m_dragSource( static_cast<XDragSource*>( m_pDragSource) )
{
#if OSL_DEBUG_LEVEL > 1
    if( listener.is())
#endif
    rBHelper.addListener( cppu::UnoType<decltype(listener)>::get(), listener );
}

SourceContext::~SourceContext()
{
}

void SAL_CALL SourceContext::addDragSourceListener(
    const Reference<XDragSourceListener >& )
    throw( RuntimeException)
{
}

void SAL_CALL SourceContext::removeDragSourceListener(
     const Reference<XDragSourceListener >& )
    throw( RuntimeException)
{
}

sal_Int32 SAL_CALL SourceContext::getCurrentCursor(  )
    throw( RuntimeException)
{
    return 0;
}

void SAL_CALL SourceContext::setCursor( sal_Int32 /*cursorId*/ )
    throw( RuntimeException)
{
}

void SAL_CALL SourceContext::setImage( sal_Int32 /*imageId*/ )
    throw( RuntimeException)
{
}

void SAL_CALL SourceContext::transferablesFlavorsChanged(  )
    throw( RuntimeException)
{
}

// non -interface functions
// Fires XDragSourceListener::dragDropEnd events.
void SourceContext::fire_dragDropEnd( sal_Bool success, sal_Int8 effect)
{

    DragSourceDropEvent e;

    if( success == sal_True)
    {
        e.DropAction=  effect;
        e.DropSuccess= sal_True;
    }
    else
    {
        e.DropAction= ACTION_NONE;
        e.DropSuccess= sal_False;
    }
    e.DragSource= m_dragSource;
    e.DragSourceContext= static_cast<XDragSourceContext*>( this);
    e.Source.set( static_cast<XDragSourceContext*>( this), UNO_QUERY);

    OInterfaceContainerHelper* pContainer= rBHelper.getContainer(
        cppu::UnoType<XDragSourceListener>::get());

    if( pContainer)
    {
        OInterfaceIteratorHelper iter( *pContainer);
        while( iter.hasMoreElements())
        {
            Reference<XDragSourceListener> listener(
                static_cast<XDragSourceListener*>( iter.next()));
            listener->dragDropEnd( e);
        }
    }
}

void SourceContext::fire_dropActionChanged( sal_Int8 dropAction, sal_Int8 userAction)
{
    if( m_currentAction != dropAction)
    {
        m_currentAction= dropAction;
        DragSourceDragEvent e;
        e.DropAction= dropAction;
        e.UserAction= userAction;
        e.DragSource= m_dragSource;
        e.DragSourceContext= static_cast<XDragSourceContext*>( this);
        e.Source.set( static_cast<XDragSourceContext*>( this), UNO_QUERY);

        OInterfaceContainerHelper* pContainer= rBHelper.getContainer(
            cppu::UnoType<XDragSourceListener>::get());

        if( pContainer)
        {
            OInterfaceIteratorHelper iter( *pContainer);
            while( iter.hasMoreElements())
            {
                Reference<XDragSourceListener> listener(
                    static_cast<XDragSourceListener*>( iter.next()));
                listener->dropActionChanged( e);
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
