/*************************************************************************
 *
 *  $RCSfile: advisesink.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mav $ $Date: 2003-11-13 17:01:12 $
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

STDMETHODIMP_(void) OleWrapperAdviseSink::OnDataChange(LPFORMATETC pFEIn, LPSTGMEDIUM pSTM)
{
    // Unused for now ( no registration for IDataObject events )
}

STDMETHODIMP_(void) OleWrapperAdviseSink::OnViewChange(DWORD dwAspect, LONG lindex)
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

STDMETHODIMP_(void) OleWrapperAdviseSink::OnRename(LPMONIKER pmk)
{
    // handled by default inprocess handler
}

STDMETHODIMP_(void) OleWrapperAdviseSink::OnSave(void)
{
    // TODO: ???
    /*
     * A Container has nothing to do here as this notification is
     * only useful when we have an ADVFCACHE_ONSAVE advise set up,
     * which we don't.  So we ignore it.
     */
}

STDMETHODIMP_(void) OleWrapperAdviseSink::OnClose(void)
{
    // mainly handled by inprocess handler

    // TODO: sometimes it can be necessary to simulate OnShowWindow( False ) here
}

