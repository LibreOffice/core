/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: idroptarget.cxx,v $
 * $Revision: 1.7 $
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
#include "idroptarget.hxx"
#include <rtl/unload.h>

#ifdef __MINGW32__
#define __uuidof(I) IID_##I
#endif

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
