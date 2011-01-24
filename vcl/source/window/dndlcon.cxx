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

#include <dndlcon.hxx>

using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::dnd;

//==================================================================================================
//
//==================================================================================================

DNDListenerContainer::DNDListenerContainer( sal_Int8 nDefaultActions )
    : WeakComponentImplHelper4< XDragGestureRecognizer, XDropTargetDragContext, XDropTargetDropContext, XDropTarget >(GetMutex())
{
    m_bActive = sal_True;
    m_nDefaultActions = nDefaultActions;
}

//==================================================================================================
//
//==================================================================================================

DNDListenerContainer::~DNDListenerContainer()
{
}

//==================================================================================================
// DNDListenerContainer::addDragGestureListener
//==================================================================================================

void SAL_CALL DNDListenerContainer::addDragGestureListener( const Reference< XDragGestureListener >& dgl )
    throw(RuntimeException)
{
    rBHelper.addListener( getCppuType( ( const Reference< XDragGestureListener > * ) 0 ), dgl );
}

//==================================================================================================
// DNDListenerContainer::removeDragGestureListener
//==================================================================================================

void SAL_CALL DNDListenerContainer::removeDragGestureListener( const Reference< XDragGestureListener >& dgl )
    throw(RuntimeException)
{
    rBHelper.removeListener( getCppuType( ( const Reference< XDragGestureListener > * ) 0 ), dgl );
}

//==================================================================================================
// DNDListenerContainer::resetRecognizer
//==================================================================================================

void SAL_CALL DNDListenerContainer::resetRecognizer(  )
    throw(RuntimeException)
{
}

//==================================================================================================
// DNDListenerContainer::addDropTargetListener
//==================================================================================================

void SAL_CALL DNDListenerContainer::addDropTargetListener( const Reference< XDropTargetListener >& dtl )
    throw(RuntimeException)
{
    rBHelper.addListener( getCppuType( ( const Reference< XDropTargetListener > * ) 0 ), dtl );
}

//==================================================================================================
// DNDListenerContainer::removeDropTargetListener
//==================================================================================================

void SAL_CALL DNDListenerContainer::removeDropTargetListener( const Reference< XDropTargetListener >& dtl )
    throw(RuntimeException)
{
    rBHelper.removeListener( getCppuType( ( const Reference< XDropTargetListener > * ) 0 ), dtl );
}

//==================================================================================================
// DNDListenerContainer::isActive
//==================================================================================================

sal_Bool SAL_CALL DNDListenerContainer::isActive(  )
    throw(RuntimeException)
{
    return m_bActive;
}

//==================================================================================================
// DNDListenerContainer::setActive
//==================================================================================================

void SAL_CALL DNDListenerContainer::setActive( sal_Bool active )
    throw(RuntimeException)
{
    m_bActive = active;
}

//==================================================================================================
// DNDListenerContainer::getDefaultActions
//==================================================================================================

sal_Int8 SAL_CALL DNDListenerContainer::getDefaultActions(  )
    throw(RuntimeException)
{
    return m_nDefaultActions;
}

//==================================================================================================
// DNDListenerContainer::setDefaultActions
//==================================================================================================

void SAL_CALL DNDListenerContainer::setDefaultActions( sal_Int8 actions )
    throw(RuntimeException)
{
    m_nDefaultActions = actions;
}

//==================================================================================================
// DNDListenerContainer::fireDropEvent
//==================================================================================================

sal_uInt32 DNDListenerContainer::fireDropEvent( const Reference< XDropTargetDropContext >& context,
    sal_Int8 dropAction, sal_Int32 locationX, sal_Int32 locationY, sal_Int8 sourceActions,
    const Reference< XTransferable >& transferable )
{
    sal_uInt32 nRet = 0;

    // fire DropTargetDropEvent on all XDropTargetListeners
    OInterfaceContainerHelper *pContainer = rBHelper.getContainer( getCppuType( ( Reference < XDropTargetListener > * ) 0) );

    if( pContainer && m_bActive )
    {
        OInterfaceIteratorHelper aIterator( *pContainer );

        // remember context to use in own context methods
        m_xDropTargetDropContext = context;

        // do not construct the event before you are sure at least one listener is registered
        DropTargetDropEvent aEvent( static_cast < XDropTarget * > (this), 0,
            static_cast < XDropTargetDropContext * > (this), dropAction,
            locationX, locationY, sourceActions, transferable );

        while (aIterator.hasMoreElements())
        {
            // FIXME: this can be simplified as soon as the Iterator has a remove method
            Reference< XInterface > xElement( aIterator.next() );

            try
            {
                // this may result in a runtime exception
                Reference < XDropTargetListener > xListener( xElement, UNO_QUERY );

                if( xListener.is() )
                {
                    // fire drop until the first one has accepted
                    if( m_xDropTargetDropContext.is() )
                        xListener->drop( aEvent );
                    else
                    {
                        DropTargetEvent aDTEvent( static_cast < XDropTarget * > (this), 0 );
                        xListener->dragExit( aDTEvent );
                    }

                    nRet++;
                }
            }

            catch( RuntimeException exc )
            {
                pContainer->removeInterface( xElement );
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

            catch( RuntimeException exc )
            {
            }
        }
    }

    return nRet;
}

//==================================================================================================
// DNDListenerContainer::fireDragExitEvent
//==================================================================================================

sal_uInt32 DNDListenerContainer::fireDragExitEvent()
{
    sal_uInt32 nRet = 0;

    // fire DropTargetDropEvent on all XDropTargetListeners
    OInterfaceContainerHelper *pContainer = rBHelper.getContainer( getCppuType( ( Reference < XDropTargetListener > * ) 0) );

    if( pContainer && m_bActive )
    {
        OInterfaceIteratorHelper aIterator( *pContainer );

        // do not construct the event before you are sure at least one listener is registered
        DropTargetEvent aEvent( static_cast < XDropTarget * > (this), 0 );

        while (aIterator.hasMoreElements())
        {
            // FIXME: this can be simplified as soon as the Iterator has a remove method
            Reference< XInterface > xElement( aIterator.next() );

            try
            {
                // this may result in a runtime exception
                Reference < XDropTargetListener > xListener( xElement, UNO_QUERY );

                if( xListener.is() )
                {
                       xListener->dragExit( aEvent );
                    nRet++;
                }
            }

            catch( RuntimeException exc )
            {
                pContainer->removeInterface( xElement );
            }
        }
    }

    return nRet;
}

//==================================================================================================
// DNDListenerContainer::fireDragOverEvent
//==================================================================================================

sal_uInt32 DNDListenerContainer::fireDragOverEvent( const Reference< XDropTargetDragContext >& context,
    sal_Int8 dropAction, sal_Int32 locationX, sal_Int32 locationY, sal_Int8 sourceActions )
{
    sal_uInt32 nRet = 0;

    // fire DropTargetDropEvent on all XDropTargetListeners
    OInterfaceContainerHelper *pContainer = rBHelper.getContainer( getCppuType( ( Reference < XDropTargetListener > * ) 0) );

    if( pContainer && m_bActive )
    {
        OInterfaceIteratorHelper aIterator( *pContainer );

        // remember context to use in own context methods
        m_xDropTargetDragContext = context;

        // do not construct the event before you are sure at least one listener is registered
        DropTargetDragEvent aEvent( static_cast < XDropTarget * > (this), 0,
            static_cast < XDropTargetDragContext * > (this),
            dropAction, locationX, locationY, sourceActions );

        while (aIterator.hasMoreElements())
        {
            // FIXME: this can be simplified as soon as the Iterator has a remove method
            Reference< XInterface > xElement( aIterator.next() );

            try
            {
                // this may result in a runtime exception
                Reference < XDropTargetListener > xListener( xElement, UNO_QUERY );

                if( xListener.is() )
                {
                    if( m_xDropTargetDragContext.is() )
                        xListener->dragOver( aEvent );
                      nRet++;
                }
            }

            catch( RuntimeException exc )
            {
                pContainer->removeInterface( xElement );
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

            catch( RuntimeException exc )
            {
            }
        }
    }

    return nRet;
}

//==================================================================================================
// DNDListenerContainer::fireDragEnterEvent
//==================================================================================================

sal_uInt32 DNDListenerContainer::fireDragEnterEvent( const Reference< XDropTargetDragContext >& context,
    sal_Int8 dropAction, sal_Int32 locationX, sal_Int32 locationY, sal_Int8 sourceActions,
    const Sequence< DataFlavor >& dataFlavors )
{
    sal_uInt32 nRet = 0;

    // fire DropTargetDropEvent on all XDropTargetListeners
    OInterfaceContainerHelper *pContainer = rBHelper.getContainer( getCppuType( ( Reference < XDropTargetListener > * ) 0) );

    if( pContainer && m_bActive )
    {
        OInterfaceIteratorHelper aIterator( *pContainer );

        // remember context to use in own context methods
        m_xDropTargetDragContext = context;

        // do not construct the event before you are sure at least one listener is registered
        DropTargetDragEnterEvent aEvent( static_cast < XDropTarget * > (this), 0,
            static_cast < XDropTargetDragContext * > (this),
            dropAction, locationX, locationY, sourceActions, dataFlavors );

        while (aIterator.hasMoreElements())
        {
            // FIXME: this can be simplified as soon as the Iterator has a remove method
            Reference< XInterface > xElement( aIterator.next() );

            try
            {
                // this may result in a runtime exception
                Reference < XDropTargetListener > xListener( xElement, UNO_QUERY );

                if( xListener.is() )
                {
                    if( m_xDropTargetDragContext.is() )
                        xListener->dragEnter( aEvent );
                    nRet++;
                }
            }

            catch( RuntimeException exc )
            {
                pContainer->removeInterface( xElement );
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

            catch( RuntimeException exc )
            {
            }
        }
    }

    return nRet;
}

//==================================================================================================
// DNDListenerContainer::fireDropActionChangedEvent
//==================================================================================================

sal_uInt32 DNDListenerContainer::fireDropActionChangedEvent( const Reference< XDropTargetDragContext >& context,
    sal_Int8 dropAction, sal_Int32 locationX, sal_Int32 locationY, sal_Int8 sourceActions )
{
    sal_uInt32 nRet = 0;

    // fire DropTargetDropEvent on all XDropTargetListeners
    OInterfaceContainerHelper *pContainer = rBHelper.getContainer( getCppuType( ( Reference < XDropTargetListener > * ) 0) );

    if( pContainer && m_bActive )
    {
        OInterfaceIteratorHelper aIterator( *pContainer );

        // remember context to use in own context methods
        m_xDropTargetDragContext = context;

        // do not construct the event before you are sure at least one listener is registered
        DropTargetDragEvent aEvent( static_cast < XDropTarget * > (this), 0,
            static_cast < XDropTargetDragContext * > (this),
            dropAction, locationX, locationY, sourceActions );

        while (aIterator.hasMoreElements())
        {
            // FIXME: this can be simplified as soon as the Iterator has a remove method
            Reference< XInterface > xElement( aIterator.next() );

            try
            {
                // this may result in a runtime exception
                Reference < XDropTargetListener > xListener( xElement, UNO_QUERY );

                if( xListener.is() )
                {
                    if( m_xDropTargetDragContext.is() )
                        xListener->dropActionChanged( aEvent );
                       nRet++;
                }
            }

            catch( RuntimeException exc )
            {
                pContainer->removeInterface( xElement );
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

            catch( RuntimeException exc )
            {
            }
        }
    }

    return nRet;
}

//==================================================================================================
// DNDListenerContainer::fireDragGestureEvent
//==================================================================================================

sal_uInt32 DNDListenerContainer::fireDragGestureEvent( sal_Int8 dragAction, sal_Int32 dragOriginX,
    sal_Int32 dragOriginY, const Reference< XDragSource >& dragSource, const Any& triggerEvent )
{
    sal_uInt32 nRet = 0;

    // fire DropTargetDropEvent on all XDropTargetListeners
    OInterfaceContainerHelper *pContainer = rBHelper.getContainer( getCppuType( ( Reference < XDragGestureListener > * ) 0) );

    if( pContainer )
    {
        OInterfaceIteratorHelper aIterator( *pContainer );

        // do not construct the event before you are sure at least one listener is registered
        DragGestureEvent aEvent( static_cast < XDragGestureRecognizer * > (this), dragAction,
            dragOriginX, dragOriginY, dragSource, triggerEvent );

        while( aIterator.hasMoreElements() )
        {
            // FIXME: this can be simplified as soon as the Iterator has a remove method
            Reference< XInterface > xElement( aIterator.next() );

            try
            {
                // this may result in a runtime exception
                Reference < XDragGestureListener > xListener( xElement, UNO_QUERY );

                if( xListener.is() )
                {
                    xListener->dragGestureRecognized( aEvent );
                    nRet++;
                }
            }

            catch( RuntimeException exc )
            {
                pContainer->removeInterface( xElement );
            }
        }
    }

    return nRet;
}

//==================================================================================================
// DNDListenerContainer::acceptDrag
//==================================================================================================

void SAL_CALL DNDListenerContainer::acceptDrag( sal_Int8 dragOperation ) throw (RuntimeException)
{
    if( m_xDropTargetDragContext.is() )
    {
        m_xDropTargetDragContext->acceptDrag( dragOperation );
        m_xDropTargetDragContext.clear();
    }
}

//==================================================================================================
// DNDListenerContainer::rejectDrag
//==================================================================================================

void SAL_CALL DNDListenerContainer::rejectDrag(  ) throw (RuntimeException)
{
    // nothing to do here
}

//==================================================================================================
// DNDListenerContainer::acceptDrop
//==================================================================================================

void SAL_CALL DNDListenerContainer::acceptDrop( sal_Int8 dropOperation ) throw (RuntimeException)
{
    if( m_xDropTargetDropContext.is() )
        m_xDropTargetDropContext->acceptDrop( dropOperation );
}

//==================================================================================================
// DNDListenerContainer::rejectDrop
//==================================================================================================

void SAL_CALL DNDListenerContainer::rejectDrop(  ) throw (RuntimeException)
{
    // nothing to do here
}

//==================================================================================================
// DNDListenerContainer::dropComplete
//==================================================================================================

void SAL_CALL DNDListenerContainer::dropComplete( sal_Bool success ) throw (RuntimeException)
{
    if( m_xDropTargetDropContext.is() )
    {
        m_xDropTargetDropContext->dropComplete( success );
        m_xDropTargetDropContext.clear();
    }
}
