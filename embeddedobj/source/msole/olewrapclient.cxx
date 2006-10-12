/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: olewrapclient.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 11:23:30 $
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

