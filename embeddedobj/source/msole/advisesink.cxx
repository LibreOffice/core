/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: advisesink.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 11:21:42 $
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
#include <advisesink.hxx>
#include <olecomponent.hxx>

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
    OleComponent* pLockComponent = NULL;

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
        pLockComponent->OnViewChange_Impl( dwAspect );
        pLockComponent->release();
    }
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
    // mainly handled by inprocess handler

    // TODO: sometimes it can be necessary to simulate OnShowWindow( False ) here
}

