/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
    return osl_atomic_increment( &m_nRefCount);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
