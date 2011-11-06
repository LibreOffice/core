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
#include "precompiled_embeddedobj.hxx"

#include <osl/diagnose.h>

#include "olewrapclient.hxx"
#include "olecomponent.hxx"

// TODO: May be a mutex must be introduced

OleWrapperClientSite::OleWrapperClientSite( OleComponent* pOleComp )
: m_nRefCount( 0 )
, m_pOleComp( pOleComp )
{
    OSL_ENSURE( m_pOleComp, "No ole component is provided!\n" );
}

OleWrapperClientSite::~OleWrapperClientSite()
{
}

STDMETHODIMP OleWrapperClientSite::QueryInterface( REFIID riid , void** ppv )
{
    *ppv=NULL;

    if ( riid == IID_IUnknown )
        *ppv = (IUnknown*)this;

    if ( riid == IID_IOleClientSite )
        *ppv = (IOleClientSite*)this;

    if ( *ppv != NULL )
    {
        ((IUnknown*)*ppv)->AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) OleWrapperClientSite::AddRef()
{
    return osl_incrementInterlockedCount( &m_nRefCount);
}

STDMETHODIMP_(ULONG) OleWrapperClientSite::Release()
{
    ULONG nReturn = --m_nRefCount;
    if ( m_nRefCount == 0 )
        delete this;

    return nReturn;
}

void OleWrapperClientSite::disconnectOleComponent()
{
    // must not be called from the descructor of OleComponent!!!
    osl::MutexGuard aGuard( m_aMutex );
    m_pOleComp = NULL;
}

STDMETHODIMP OleWrapperClientSite::SaveObject()
{
    OleComponent* pLockComponent = NULL;
    HRESULT hResult = E_FAIL;

    {
        osl::MutexGuard aGuard( m_aMutex );
        if ( m_pOleComp )
        {
            pLockComponent = m_pOleComp;
            pLockComponent->acquire();
        }
    }

    if ( pLockComponent )
    {
        if ( pLockComponent->SaveObject_Impl() )
            hResult = S_OK;

        pLockComponent->release();
    }

    return hResult;
}

STDMETHODIMP OleWrapperClientSite::GetMoniker( DWORD, DWORD, LPMONIKER *ppmk )
{
    *ppmk = NULL;
    return E_NOTIMPL;
}

STDMETHODIMP OleWrapperClientSite::GetContainer( LPOLECONTAINER* ppContainer )
{
    *ppContainer = NULL;
    return E_NOTIMPL;
}

STDMETHODIMP OleWrapperClientSite::ShowObject(void)
{
    return S_OK;
}

STDMETHODIMP OleWrapperClientSite::OnShowWindow( BOOL bShow )
{
    OleComponent* pLockComponent = NULL;

    // TODO/LATER: redirect the notification to the main thread so that SolarMutex can be locked
    {
        osl::MutexGuard aGuard( m_aMutex );
        if ( m_pOleComp )
        {
            pLockComponent = m_pOleComp;
            pLockComponent->acquire();
        }
    }

    if ( pLockComponent )
    {
        pLockComponent->OnShowWindow_Impl( bShow ); // the result is not interesting
        pLockComponent->release();
    }

    return S_OK;
}

STDMETHODIMP OleWrapperClientSite::RequestNewObjectLayout(void)
{
    return E_NOTIMPL;
}

