/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sourcecontext.cxx,v $
 * $Revision: 1.10 $
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
#include "precompiled_dtrans.hxx"


#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>

#include "sourcecontext.hxx"
#include <rtl/unload.h>

using namespace com::sun::star::datatransfer::dnd;
using namespace com::sun::star::datatransfer::dnd::DNDConstants;
extern rtl_StandardModuleCount g_moduleCount;

SourceContext::SourceContext( DragSource* pSource,
                             const Reference<XDragSourceListener>& listener):
        WeakComponentImplHelper1<XDragSourceContext>(m_mutex),
        m_pDragSource( pSource),
        m_dragSource( static_cast<XDragSource*>( m_pDragSource) )
{
    g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
#if OSL_DEBUG_LEVEL > 1
    if( listener.is())
#endif
    rBHelper.addListener( ::getCppuType( &listener ), listener );
}

SourceContext::~SourceContext()
{
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
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
    e.Source= Reference<XInterface>( static_cast<XDragSourceContext*>( this), UNO_QUERY);

    OInterfaceContainerHelper* pContainer= rBHelper.getContainer(
        getCppuType( (Reference<XDragSourceListener>* )0 ) );

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
        e.Source= Reference<XInterface>( static_cast<XDragSourceContext*>( this), UNO_QUERY);

        OInterfaceContainerHelper* pContainer= rBHelper.getContainer(
            getCppuType( (Reference<XDragSourceListener>* )0 ) );

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
