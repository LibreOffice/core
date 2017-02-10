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

#include "APNDataObject.hxx"
#include <osl/diagnose.h>

#include <systools/win32/comtools.hxx>

#define FREE_HGLOB_ON_RELEASE   TRUE
#define KEEP_HGLOB_ON_RELEASE   FALSE

// ctor

CAPNDataObject::CAPNDataObject( IDataObjectPtr rIDataObject ) :
    m_rIDataObjectOrg( rIDataObject ),
    m_hGlobal( nullptr ),
    m_nRefCnt( 0 )
{

    OSL_ENSURE( m_rIDataObjectOrg.get( ), "constructing CAPNDataObject with empty data object" );

    // we marshal the IDataObject interface pointer here so
    // that it can be unmarshaled multiple times when this
    // class will be used from another apartment
    IStreamPtr pStm;
    HRESULT hr = CreateStreamOnHGlobal( nullptr, KEEP_HGLOB_ON_RELEASE, &pStm );

    OSL_ENSURE( E_INVALIDARG != hr, "invalid args passed to CreateStreamOnHGlobal" );

    if ( SUCCEEDED( hr ) )
    {
        HRESULT hr_marshal = CoMarshalInterface(
            pStm.get(),
            __uuidof(IDataObject),
            static_cast<LPUNKNOWN>(m_rIDataObjectOrg.get()),
            MSHCTX_LOCAL,
            nullptr,
            MSHLFLAGS_TABLEWEAK );

        OSL_ENSURE( CO_E_NOTINITIALIZED != hr_marshal, "COM is not initialized" );

        // marshalling may fail if COM is not initialized
        // for the calling thread which is a program time
        // error or because of stream errors which are runtime
        // errors for instance E_OUTOFMEMORY etc.

        hr = GetHGlobalFromStream(pStm.get(), &m_hGlobal );

        OSL_ENSURE( E_INVALIDARG != hr, "invalid stream passed to GetHGlobalFromStream" );

        // if the marshalling failed we free the
        // global memory again and set m_hGlobal
        // to a defined value
        if (FAILED(hr_marshal))
        {
            OSL_FAIL("marshalling failed");

            HGLOBAL hGlobal =
                GlobalFree(m_hGlobal);
            OSL_ENSURE(nullptr == hGlobal, "GlobalFree failed");
            m_hGlobal = nullptr;
        }
    }
}

CAPNDataObject::~CAPNDataObject( )
{
    if (m_hGlobal)
    {
        IStreamPtr pStm;
        HRESULT  hr = CreateStreamOnHGlobal(m_hGlobal, FREE_HGLOB_ON_RELEASE, &pStm);

        OSL_ENSURE( E_INVALIDARG != hr, "invalid args passed to CreateStreamOnHGlobal" );

        if (SUCCEEDED(hr))
        {
            hr = CoReleaseMarshalData(pStm.get());
            OSL_ENSURE(SUCCEEDED(hr), "CoReleaseMarshalData failed");
        }
    }
}

// IUnknown->QueryInterface

STDMETHODIMP CAPNDataObject::QueryInterface( REFIID iid, LPVOID* ppvObject )
{
    OSL_ASSERT( nullptr != ppvObject );

    if ( nullptr == ppvObject )
        return E_INVALIDARG;

    HRESULT hr = E_NOINTERFACE;
    *ppvObject = nullptr;

    if ( ( __uuidof( IUnknown ) == iid ) || ( __uuidof( IDataObject ) == iid ) )
    {
        *ppvObject = static_cast< IUnknown* >( this );
        static_cast<LPUNKNOWN>(*ppvObject)->AddRef( );
        hr = S_OK;
    }

    return hr;
}

// IUnknown->AddRef

STDMETHODIMP_(ULONG) CAPNDataObject::AddRef( )
{
    return static_cast< ULONG >( InterlockedIncrement( &m_nRefCnt ) );
}

// IUnknown->Release

STDMETHODIMP_(ULONG) CAPNDataObject::Release( )
{
    // we need a helper variable because it's not allowed to access
    // a member variable after an object is destroyed
    ULONG nRefCnt = static_cast< ULONG >( InterlockedDecrement( &m_nRefCnt ) );

    if ( 0 == nRefCnt )
        delete this;

    return nRefCnt;
}

// IDataObject->GetData

STDMETHODIMP CAPNDataObject::GetData( LPFORMATETC pFormatetc, LPSTGMEDIUM pmedium )
{
    HRESULT hr = m_rIDataObjectOrg->GetData( pFormatetc, pmedium );

    if (RPC_E_WRONG_THREAD == hr)
    {
        IDataObjectPtr pIDOTmp;
        hr = MarshalIDataObjectIntoCurrentApartment(&pIDOTmp);

        if (SUCCEEDED(hr))
            hr = pIDOTmp->GetData(pFormatetc, pmedium);
    }
    return hr;
}

// IDataObject->EnumFormatEtc

STDMETHODIMP CAPNDataObject::EnumFormatEtc( DWORD dwDirection, IEnumFORMATETC** ppenumFormatetc )
{
    HRESULT hr = m_rIDataObjectOrg->EnumFormatEtc(dwDirection, ppenumFormatetc);

    if (RPC_E_WRONG_THREAD == hr)
    {
        IDataObjectPtr pIDOTmp;
        hr = MarshalIDataObjectIntoCurrentApartment(&pIDOTmp);

        if (SUCCEEDED(hr))
            hr = pIDOTmp->EnumFormatEtc(dwDirection, ppenumFormatetc);
    }
    return hr;
}

// IDataObject->QueryGetData

STDMETHODIMP CAPNDataObject::QueryGetData( LPFORMATETC pFormatetc )
{
    HRESULT hr = m_rIDataObjectOrg->QueryGetData( pFormatetc );

    if (RPC_E_WRONG_THREAD == hr)
    {
        IDataObjectPtr pIDOTmp;
        hr = MarshalIDataObjectIntoCurrentApartment( &pIDOTmp );

        if (SUCCEEDED(hr))
            hr = pIDOTmp->QueryGetData(pFormatetc);
    }
    return hr;
}

// IDataObject->GetDataHere

STDMETHODIMP CAPNDataObject::GetDataHere( LPFORMATETC pFormatetc, LPSTGMEDIUM pmedium )
{
    HRESULT hr = m_rIDataObjectOrg->GetDataHere(pFormatetc, pmedium);

    if (RPC_E_WRONG_THREAD == hr)
    {
        IDataObjectPtr pIDOTmp;
        hr = MarshalIDataObjectIntoCurrentApartment(&pIDOTmp);

        if (SUCCEEDED(hr))
            hr = pIDOTmp->GetDataHere(pFormatetc, pmedium);
    }
    return hr;
}

// IDataObject->GetCanonicalFormatEtc

STDMETHODIMP CAPNDataObject::GetCanonicalFormatEtc(LPFORMATETC pFormatectIn, LPFORMATETC pFormatetcOut)
{
    HRESULT hr = m_rIDataObjectOrg->GetCanonicalFormatEtc( pFormatectIn, pFormatetcOut );

    if (RPC_E_WRONG_THREAD == hr)
    {
        IDataObjectPtr pIDOTmp;
        hr = MarshalIDataObjectIntoCurrentApartment(&pIDOTmp);

        if (SUCCEEDED(hr))
            hr = pIDOTmp->GetCanonicalFormatEtc(pFormatectIn, pFormatetcOut);
    }
    return hr;
}

// IDataObject->SetData

STDMETHODIMP CAPNDataObject::SetData( LPFORMATETC pFormatetc, LPSTGMEDIUM pmedium, BOOL fRelease )
{
    HRESULT hr = m_rIDataObjectOrg->SetData( pFormatetc, pmedium, fRelease );

    if (RPC_E_WRONG_THREAD == hr)
    {
        IDataObjectPtr pIDOTmp;
        hr = MarshalIDataObjectIntoCurrentApartment(&pIDOTmp);

        if (SUCCEEDED(hr))
            hr = pIDOTmp->SetData(pFormatetc, pmedium, fRelease);
    }
    return hr;
}

// IDataObject->DAdvise

STDMETHODIMP CAPNDataObject::DAdvise( LPFORMATETC pFormatetc, DWORD advf, LPADVISESINK pAdvSink, DWORD * pdwConnection )
{
    HRESULT hr = m_rIDataObjectOrg->DAdvise(pFormatetc, advf, pAdvSink, pdwConnection);

    if (RPC_E_WRONG_THREAD == hr)
    {
        IDataObjectPtr pIDOTmp;
        hr = MarshalIDataObjectIntoCurrentApartment(&pIDOTmp);

        if (SUCCEEDED(hr))
            hr = pIDOTmp->DAdvise(pFormatetc, advf, pAdvSink, pdwConnection);
    }
    return hr;
}

// IDataObject->DUnadvise

STDMETHODIMP CAPNDataObject::DUnadvise( DWORD dwConnection )
{
    HRESULT hr = m_rIDataObjectOrg->DUnadvise( dwConnection );

    if (RPC_E_WRONG_THREAD == hr)
    {
        IDataObjectPtr pIDOTmp;
        hr = MarshalIDataObjectIntoCurrentApartment(&pIDOTmp);

        if (SUCCEEDED(hr))
            hr = pIDOTmp->DUnadvise(dwConnection);
    }
    return hr;
}

// IDataObject->EnumDAdvise

STDMETHODIMP CAPNDataObject::EnumDAdvise( LPENUMSTATDATA * ppenumAdvise )
{
    HRESULT hr = m_rIDataObjectOrg->EnumDAdvise(ppenumAdvise);

    if (RPC_E_WRONG_THREAD == hr)
    {
        IDataObjectPtr pIDOTmp;
        hr = MarshalIDataObjectIntoCurrentApartment(&pIDOTmp);

        if (SUCCEEDED(hr))
            hr = pIDOTmp->EnumDAdvise(ppenumAdvise);
    }
    return hr;
}

// for our convenience

CAPNDataObject::operator IDataObject*( )
{
    return static_cast< IDataObject* >( this );
}

// helper function

HRESULT CAPNDataObject::MarshalIDataObjectIntoCurrentApartment( IDataObject** ppIDataObj )
{
    OSL_ASSERT(nullptr != ppIDataObj);

    *ppIDataObj = nullptr;
    HRESULT hr = E_FAIL;

    if (m_hGlobal)
    {
        IStreamPtr pStm;
        hr = CreateStreamOnHGlobal(m_hGlobal, KEEP_HGLOB_ON_RELEASE, &pStm);

        OSL_ENSURE(E_INVALIDARG != hr, "CreateStreamOnHGlobal with invalid args called");

        if (SUCCEEDED(hr))
        {
            hr = CoUnmarshalInterface(pStm.get(), __uuidof(IDataObject), reinterpret_cast<void**>(ppIDataObj));
            OSL_ENSURE(CO_E_NOTINITIALIZED != hr, "COM is not initialized");
        }
    }
    return hr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
