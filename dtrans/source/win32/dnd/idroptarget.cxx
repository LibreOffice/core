/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: idroptarget.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 06:02:21 $
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
#include "idroptarget.hxx"
#include <rtl/unload.h>
extern rtl_StandardModuleCount g_moduleCount;

IDropTargetImpl::IDropTargetImpl( DropTarget& pTarget): m_nRefCount( 0),
                                    m_rDropTarget( pTarget)
{
    g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
}

IDropTargetImpl::~IDropTargetImpl()
{
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}


//IDropTarget
HRESULT STDMETHODCALLTYPE IDropTargetImpl::QueryInterface( REFIID riid, void  **ppvObject)
{
    if( !ppvObject)
        return E_POINTER;
    *ppvObject= NULL;

    if( riid == __uuidof( IUnknown))
        *ppvObject= static_cast<IUnknown*>( this);
    else if (  riid == __uuidof( IDropTarget))
        *ppvObject= static_cast<IDropTarget*>( this);

    if(*ppvObject)
    {
        AddRef();
        return S_OK;
    }
    else
        return E_NOINTERFACE;

}

ULONG STDMETHODCALLTYPE IDropTargetImpl::AddRef( void)
{
    return InterlockedIncrement( &m_nRefCount);
}

ULONG STDMETHODCALLTYPE IDropTargetImpl::Release( void)
{
    LONG count= InterlockedDecrement( &m_nRefCount);
    if( m_nRefCount == 0 )
        delete this;
    return count;
}

STDMETHODIMP IDropTargetImpl::DragEnter( IDataObject __RPC_FAR *pDataObj,
                                    DWORD grfKeyState,
                                    POINTL pt,
                                    DWORD  *pdwEffect)
{
    return m_rDropTarget.DragEnter( pDataObj, grfKeyState,
                                  pt, pdwEffect);
}

STDMETHODIMP IDropTargetImpl::DragOver( DWORD grfKeyState,
                                   POINTL pt,
                                   DWORD  *pdwEffect)
{
    return m_rDropTarget.DragOver( grfKeyState, pt, pdwEffect);
}

STDMETHODIMP IDropTargetImpl::DragLeave( void)
{
    return m_rDropTarget.DragLeave();
}

STDMETHODIMP IDropTargetImpl::Drop( IDataObject  *pDataObj,
                   DWORD grfKeyState,
                   POINTL pt,
                   DWORD __RPC_FAR *pdwEffect)
{
    return m_rDropTarget.Drop( pDataObj, grfKeyState,
                                   pt, pdwEffect);
}
