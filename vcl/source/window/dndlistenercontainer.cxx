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

#include <vcl/dndlistenercontainer.hxx>

using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::dnd;

DNDListenerContainer::DNDListenerContainer( sal_Int8 nDefaultActions )
{
    m_bActive = true;
    m_nDefaultActions = nDefaultActions;
}

DNDListenerContainer::~DNDListenerContainer()
{
}

void SAL_CALL DNDListenerContainer::addDragGestureListener( const Reference< XDragGestureListener >& dgl )
{
    std::unique_lock g(m_aMutex);
    maDragGestureListeners.addInterface( g, dgl );
}

void SAL_CALL DNDListenerContainer::removeDragGestureListener( const Reference< XDragGestureListener >& dgl )
{
    std::unique_lock g(m_aMutex);
    maDragGestureListeners.removeInterface( g, dgl );
}

void SAL_CALL DNDListenerContainer::resetRecognizer(  )
{
}

void SAL_CALL DNDListenerContainer::addDropTargetListener( const Reference< XDropTargetListener >& dtl )
{
    std::unique_lock g(m_aMutex);
    maDropTargetListeners.addInterface( g, dtl );
}

void SAL_CALL DNDListenerContainer::removeDropTargetListener( const Reference< XDropTargetListener >& dtl )
{
    std::unique_lock g(m_aMutex);
    maDropTargetListeners.removeInterface( g, dtl );
}

sal_Bool SAL_CALL DNDListenerContainer::isActive(  )
{
    return m_bActive;
}

void SAL_CALL DNDListenerContainer::setActive( sal_Bool active )
{
    m_bActive = active;
}

sal_Int8 SAL_CALL DNDListenerContainer::getDefaultActions(  )
{
    return m_nDefaultActions;
}

void SAL_CALL DNDListenerContainer::setDefaultActions( sal_Int8 actions )
{
    m_nDefaultActions = actions;
}

sal_uInt32 DNDListenerContainer::fireDropEvent( const Reference< XDropTargetDropContext >& context,
    sal_Int8 dropAction, sal_Int32 locationX, sal_Int32 locationY, sal_Int8 sourceActions,
    const Reference< XTransferable >& transferable )
{
    std::unique_lock g(m_aMutex);
    if (!m_bActive || maDropTargetListeners.getLength(g) == 0)
        return 0;

    sal_uInt32 nRet = 0;

    comphelper::OInterfaceIteratorHelper4 aIterator( g, maDropTargetListeners );

    // remember context to use in own context methods
    m_xDropTargetDropContext = context;

    // do not construct the event before you are sure at least one listener is registered
    DropTargetDropEvent aEvent( static_cast < XDropTarget * > (this), 0,
        static_cast < XDropTargetDropContext * > (this), dropAction,
        locationX, locationY, sourceActions, transferable );

    while (aIterator.hasMoreElements())
    {
        Reference< XDropTargetListener > xListener( aIterator.next() );
        try
        {
            // fire drop until the first one has accepted
            if( m_xDropTargetDropContext.is() )
            {
                g.unlock();
                xListener->drop( aEvent );
            }
            else
            {
                g.unlock();
                DropTargetEvent aDTEvent( static_cast < XDropTarget * > (this), 0 );
                xListener->dragExit( aDTEvent );
            }

            g.lock();
            nRet++;
        }
        catch (const RuntimeException&)
        {
            aIterator.remove( g );
        }
    }

    // if context still valid, then reject drop
    if( m_xDropTargetDropContext.is() )
    {
        m_xDropTargetDropContext.clear();

        try
        {
            context->rejectDrop();
        }
        catch (const RuntimeException&)
        {
        }
    }

    return nRet;
}

sal_uInt32 DNDListenerContainer::fireDragExitEvent()
{
    std::unique_lock g(m_aMutex);
    if (!m_bActive || maDropTargetListeners.getLength(g) == 0)
        return 0;

    sal_uInt32 nRet = 0;

    // do not construct the event before you are sure at least one listener is registered
    DropTargetEvent aEvent( static_cast < XDropTarget * > (this), 0 );

    comphelper::OInterfaceIteratorHelper4 aIterator( g, maDropTargetListeners );
    g.unlock();
    while (aIterator.hasMoreElements())
    {
        Reference< XDropTargetListener > xListener( aIterator.next() );
        try
        {
            xListener->dragExit( aEvent );
            nRet++;
        }
        catch (const RuntimeException&)
        {
            g.lock();
            aIterator.remove( g );
            g.unlock();
        }
    }

    return nRet;
}

sal_uInt32 DNDListenerContainer::fireDragOverEvent( const Reference< XDropTargetDragContext >& context,
    sal_Int8 dropAction, sal_Int32 locationX, sal_Int32 locationY, sal_Int8 sourceActions )
{
    std::unique_lock g(m_aMutex);
    if (!m_bActive || maDropTargetListeners.getLength(g) == 0)
        return 0;

    sal_uInt32 nRet = 0;

    // fire DropTargetDropEvent on all XDropTargetListeners

    comphelper::OInterfaceIteratorHelper4 aIterator( g, maDropTargetListeners );

    // remember context to use in own context methods
    m_xDropTargetDragContext = context;

    // do not construct the event before you are sure at least one listener is registered
    DropTargetDragEvent aEvent( static_cast < XDropTarget * > (this), 0,
        static_cast < XDropTargetDragContext * > (this),
        dropAction, locationX, locationY, sourceActions );

    while (aIterator.hasMoreElements())
    {
        Reference< XDropTargetListener > xListener( aIterator.next() );
        try
        {
            if( m_xDropTargetDragContext.is() )
            {
                g.unlock();
                xListener->dragOver( aEvent );
                g.lock();
            }
            nRet++;
        }
        catch (const RuntimeException&)
        {
            aIterator.remove(g);
        }
    }

    // if context still valid, then reject drag
    if( m_xDropTargetDragContext.is() )
    {
        m_xDropTargetDragContext.clear();

        try
        {
            context->rejectDrag();
        }
        catch (const RuntimeException&)
        {
        }
    }

    return nRet;
}

sal_uInt32 DNDListenerContainer::fireDragEnterEvent( const Reference< XDropTargetDragContext >& context,
    sal_Int8 dropAction, sal_Int32 locationX, sal_Int32 locationY, sal_Int8 sourceActions,
    const Sequence< DataFlavor >& dataFlavors )
{
    std::unique_lock g(m_aMutex);
    if (!m_bActive || maDropTargetListeners.getLength(g) == 0)
        return 0;

    sal_uInt32 nRet = 0;

    comphelper::OInterfaceIteratorHelper4 aIterator( g, maDropTargetListeners );

    // remember context to use in own context methods
    m_xDropTargetDragContext = context;

    // do not construct the event before you are sure at least one listener is registered
    DropTargetDragEnterEvent aEvent( static_cast < XDropTarget * > (this), 0,
        static_cast < XDropTargetDragContext * > (this),
        dropAction, locationX, locationY, sourceActions, dataFlavors );

    while (aIterator.hasMoreElements())
    {
        Reference< XDropTargetListener > xListener( aIterator.next() );
        try
        {
            if( m_xDropTargetDragContext.is() )
            {
                g.unlock();
                xListener->dragEnter( aEvent );
                g.lock();
            }
            nRet++;
        }
        catch (const RuntimeException&)
        {
            aIterator.remove( g );
        }
    }

    // if context still valid, then reject drag
    if( m_xDropTargetDragContext.is() )
    {
        m_xDropTargetDragContext.clear();

        try
        {
            context->rejectDrag();
        }
        catch (const RuntimeException&)
        {
        }
    }

    return nRet;
}

sal_uInt32 DNDListenerContainer::fireDropActionChangedEvent( const Reference< XDropTargetDragContext >& context,
    sal_Int8 dropAction, sal_Int32 locationX, sal_Int32 locationY, sal_Int8 sourceActions )
{
    std::unique_lock g(m_aMutex);
    if (!m_bActive || maDropTargetListeners.getLength(g) == 0)
        return 0;

    sal_uInt32 nRet = 0;

    // fire DropTargetDropEvent on all XDropTargetListeners

    comphelper::OInterfaceIteratorHelper4 aIterator( g, maDropTargetListeners );

    // remember context to use in own context methods
    m_xDropTargetDragContext = context;

    // do not construct the event before you are sure at least one listener is registered
    DropTargetDragEvent aEvent( static_cast < XDropTarget * > (this), 0,
        static_cast < XDropTargetDragContext * > (this),
        dropAction, locationX, locationY, sourceActions );

    while (aIterator.hasMoreElements())
    {
        Reference< XDropTargetListener > xListener( aIterator.next() );
        try
        {
            if( m_xDropTargetDragContext.is() )
            {
                g.unlock();
                xListener->dropActionChanged( aEvent );
                g.lock();
            }
            nRet++;
        }
        catch (const RuntimeException&)
        {
            aIterator.remove( g );
        }
    }

    // if context still valid, then reject drag
    if( m_xDropTargetDragContext.is() )
    {
        m_xDropTargetDragContext.clear();

        try
        {
            context->rejectDrag();
        }
        catch (const RuntimeException&)
        {
        }
    }

    return nRet;
}

sal_uInt32 DNDListenerContainer::fireDragGestureEvent( sal_Int8 dragAction, sal_Int32 dragOriginX,
    sal_Int32 dragOriginY, const Reference< XDragSource >& dragSource, const Any& triggerEvent )
{
    std::unique_lock g(m_aMutex);
    if (maDragGestureListeners.getLength(g) == 0)
        return 0;

    sal_uInt32 nRet = 0;

    // do not construct the event before you are sure at least one listener is registered
    DragGestureEvent aEvent( static_cast < XDragGestureRecognizer * > (this), dragAction,
        dragOriginX, dragOriginY, dragSource, triggerEvent );

    comphelper::OInterfaceIteratorHelper4 aIterator( g, maDragGestureListeners );
    g.unlock();
    while( aIterator.hasMoreElements() )
    {
        Reference< XDragGestureListener > xListener( aIterator.next() );
        try
        {
            xListener->dragGestureRecognized( aEvent );
            nRet++;
        }
        catch (const RuntimeException&)
        {
            g.lock();
            aIterator.remove( g );
            g.unlock();
        }
    }

    return nRet;
}

void SAL_CALL DNDListenerContainer::acceptDrag( sal_Int8 dragOperation )
{
    std::unique_lock g(m_aMutex);
    if( !m_xDropTargetDragContext )
        return;
    auto xTmpDragContext = std::move(m_xDropTargetDragContext);
    g.unlock();
    xTmpDragContext->acceptDrag( dragOperation );
}

void SAL_CALL DNDListenerContainer::rejectDrag(  )
{
    // nothing to do here
}

void SAL_CALL DNDListenerContainer::acceptDrop( sal_Int8 dropOperation )
{
    if( m_xDropTargetDropContext.is() )
        m_xDropTargetDropContext->acceptDrop( dropOperation );
}

void SAL_CALL DNDListenerContainer::rejectDrop(  )
{
    // nothing to do here
}

void SAL_CALL DNDListenerContainer::dropComplete( sal_Bool success )
{
    if( m_xDropTargetDropContext.is() )
    {
        m_xDropTargetDropContext->dropComplete( success );
        m_xDropTargetDropContext.clear();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
