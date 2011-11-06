/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
