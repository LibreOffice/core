/*************************************************************************
 *
 *  $RCSfile: idroptarget.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jl $ $Date: 2001-02-26 16:00:53 $
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
#include "idroptarget.hxx"
#include <oleidl.h>


IDropTargetImpl::IDropTargetImpl( DropTarget& pTarget): m_nRefCount( 0),
                                    m_rDropTarget( pTarget)
{
}

IDropTargetImpl::~IDropTargetImpl()
{
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
