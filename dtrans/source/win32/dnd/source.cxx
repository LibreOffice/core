/*************************************************************************
 *
 *  $RCSfile: source.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: mh $ $Date: 2001-01-31 15:37:17 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_DNDCONSTANTS_HPP_
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_XTRANSFERABLE_HPP_
#include <com/sun/star/datatransfer/XTransferable.hpp>
#endif

#include "source.hxx"
#include "target.hxx"
#include "globals.hxx"
#include "sourcecontext.hxx"
#include "TransferableWrapper.hxx"
#include <rtl/ustring.h>

using namespace rtl;
using namespace cppu;
using namespace osl;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::dnd;
using namespace com::sun::star::datatransfer::dnd::DNDConstants;
using namespace com::sun::star::uno;

DragSource::DragSource( const Reference<XMultiServiceFactory>& sf):
    m_serviceFactory( sf),
    WeakComponentImplHelper3<XInitialization,XDragSource,XDropTargetFactory>(m_mutex)
{
}

DragSource::~DragSource()
{
}

  // XInitialization

// aArguments contains a machine id
void SAL_CALL DragSource::initialize( const Sequence< Any >& aArguments )
    throw(Exception, RuntimeException)
{
}

// XDragSource
sal_Bool SAL_CALL DragSource::isDragImageSupported(  )
         throw(RuntimeException)
{
    return 0;
}

sal_Int32 SAL_CALL DragSource::getDefaultCursor( sal_Int8 dragAction )
          throw( IllegalArgumentException, RuntimeException)
{
    return 0;
}

void SAL_CALL DragSource::startDrag( const DragGestureEvent& trigger,
                             sal_Int8 sourceActions,
                             sal_Int32 cursor,
                             sal_Int32 image,
                             const Reference<XTransferable >& trans,
                             const Reference<XDragSourceListener >& listener )
        throw( InvalidDNDOperationException, RuntimeException)
{
    m_sourceListener= listener;
    m_sourceActions= sourceActions;

    DWORD effect=0;
    HRESULT hr;
    hr= DoDragDrop(
        static_cast<IDataObject*>( new CXTDataObject( trans)),
        static_cast<IDropSource*>( this),
        dndActionsToDropEffects( sourceActions),
        &effect);

        DragSourceDropEvent e;
        if( hr == DRAGDROP_S_DROP)
        {
            e.DropAction= dndOleDropEffectsToActions( effect);
            e.DropSuccess= sal_True;
        }
        else
        {
            e.DropAction= ACTION_NONE;
            e.DropSuccess= sal_False;
        }
        e.DragSourceContext= static_cast<XDragSourceContext*>( new SourceContext(
                             static_cast<DragSource*>(this) ) );
        m_sourceListener->dragDropEnd( e);

}

#ifdef DEBUG
void SAL_CALL DragSource::release()
{
    if( m_refCount == 1)
    {
        int a = m_refCount;
    }
    WeakComponentImplHelper3<XInitialization,XDragSource,XDropTargetFactory>::release();

}
#endif

// XDropTargetFactory -------------------------------------------------------------------

Reference<XDropTarget > SAL_CALL DragSource::createDropTarget( const Sequence< sal_Int8 >& windowId )
    throw( RuntimeException)
{
    Sequence<sal_Int8>& seqWin= const_cast<Sequence<sal_Int8> & >( windowId);
    HWND h= *(HWND*) (const HWND*)seqWin.getConstArray();
    DropTarget* pTarget= new DropTarget(m_serviceFactory, h);
    return Reference<XDropTarget>(static_cast<XDropTarget*>( pTarget), UNO_QUERY);
}


//IDropTarget ---------------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE DragSource::QueryInterface( REFIID riid, void  **ppvObject)
{
    if( !ppvObject)
        return E_POINTER;
    *ppvObject= NULL;

    if( IsEqualGUID( riid, __uuidof( IUnknown)))
        *ppvObject= static_cast<IUnknown*>( this);
    else if ( IsEqualGUID( riid, __uuidof( IDropSource)))
        *ppvObject= static_cast<IDropSource*>( this);

    if(*ppvObject)
    {
        AddRef();
        return S_OK;
    }
    else
        return E_NOINTERFACE;

}

ULONG STDMETHODCALLTYPE DragSource::AddRef( void)
{
    acquire();
    return (ULONG) m_refCount;
}

ULONG STDMETHODCALLTYPE DragSource::Release( void)
{
    release();
    return (ULONG) m_refCount;
}

// IDropSource
HRESULT STDMETHODCALLTYPE DragSource::QueryContinueDrag(
/* [in] */ BOOL fEscapePressed,
/* [in] */ DWORD grfKeyState)
{
    HRESULT retVal= S_OK; // default continue DnD

    if (fEscapePressed)
    {
        retVal= DRAGDROP_S_CANCEL;
    }
    else
    {
        if (!(grfKeyState & MK_LBUTTON))
        {
//          DragSourceDropEvent e;
//          e.DropAction= dndOleKeysToAction(grfKeyState);
//          e.DropSuccess= sal_True;
//          e.DragSourceContext= static_cast<XDragSourceContext*>( new SourceContext(
//                               static_cast<DragSource*>(this) ) );
//
//          m_sourceListener->dragDropEnd( e);

            retVal= DRAGDROP_S_DROP;
        }
    }
    return retVal;
}

HRESULT STDMETHODCALLTYPE DragSource::GiveFeedback(
/* [in] */ DWORD dwEffect)
{
    return DRAGDROP_S_USEDEFAULTCURSORS;
}