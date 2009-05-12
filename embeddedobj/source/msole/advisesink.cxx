/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: advisesink.cxx,v $
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
#include "precompiled_embeddedobj.hxx"

#include <osl/diagnose.h>
#include <advisesink.hxx>
#include <olecomponent.hxx>

#include <rtl/ref.hxx>

OleWrapperAdviseSink::OleWrapperAdviseSink( OleComponent* pOleComp )
: m_nRefCount( 0 )
, m_pOleComp( pOleComp )
{
    OSL_ENSURE( m_pOleComp, "No ole component is provided!\n" );
}

OleWrapperAdviseSink::~OleWrapperAdviseSink()
{
}

STDMETHODIMP OleWrapperAdviseSink::QueryInterface( REFIID riid , void** ppv )
{
    *ppv=NULL;

    if ( riid == IID_IUnknown )
        *ppv = (IUnknown*)this;

    if ( riid == IID_IAdviseSink )
        *ppv = (IAdviseSink*)this;

    if ( *ppv != NULL )
    {
        ((IUnknown*)*ppv)->AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) OleWrapperAdviseSink::AddRef()
{
    return osl_incrementInterlockedCount( &m_nRefCount);
}

STDMETHODIMP_(ULONG) OleWrapperAdviseSink::Release()
{
    ULONG nReturn = --m_nRefCount;
    if ( m_nRefCount == 0 )
        delete this;

    return nReturn;
}

void OleWrapperAdviseSink::disconnectOleComponent()
{
    // must not be called from the descructor of OleComponent!!!
    osl::MutexGuard aGuard( m_aMutex );
    m_pOleComp = NULL;
}

STDMETHODIMP_(void) OleWrapperAdviseSink::OnDataChange(LPFORMATETC, LPSTGMEDIUM)
{
    // Unused for now ( no registration for IDataObject events )
}

STDMETHODIMP_(void) OleWrapperAdviseSink::OnViewChange(DWORD dwAspect, LONG)
{
    ::rtl::Reference< OleComponent > xLockComponent;

    {
        osl::MutexGuard aGuard( m_aMutex );
        if ( m_pOleComp )
            xLockComponent = m_pOleComp;
    }

    if ( xLockComponent.is() )
        xLockComponent->OnViewChange_Impl( dwAspect );
}

STDMETHODIMP_(void) OleWrapperAdviseSink::OnRename(LPMONIKER)
{
    // handled by default inprocess handler
}

STDMETHODIMP_(void) OleWrapperAdviseSink::OnSave(void)
{
    // TODO: ???
    // The object knows about document saving already since it contolls it as ClienSite
    // other interested listeners must be registered for the object
}

STDMETHODIMP_(void) OleWrapperAdviseSink::OnClose(void)
{
    ::rtl::Reference< OleComponent > xLockComponent;

    {
        osl::MutexGuard aGuard( m_aMutex );
        if ( m_pOleComp )
            xLockComponent = m_pOleComp;
    }

    if ( xLockComponent.is() )
        xLockComponent->OnClose_Impl();

    // TODO: sometimes it can be necessary to simulate OnShowWindow( False ) here
}

