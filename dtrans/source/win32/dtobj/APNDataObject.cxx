/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: APNDataObject.cxx,v $
 * $Revision: 1.9 $
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

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------
#include "APNDataObject.hxx"
#include <osl/diagnose.h>

#include <systools/win32/comtools.hxx>
#ifdef __MINGW32__
#define __uuidof(I) IID_##I
#endif

//------------------------------------------------------------------------
// defines
//------------------------------------------------------------------------

#define FREE_HGLOB_ON_RELEASE   TRUE
#define KEEP_HGLOB_ON_RELEASE   FALSE

//------------------------------------------------------------------------
// ctor
//------------------------------------------------------------------------

CAPNDataObject::CAPNDataObject( IDataObjectPtr rIDataObject ) :
    m_rIDataObjectOrg( rIDataObject ),
    m_hGlobal( NULL ),
    m_nRefCnt( 0 )
{

    OSL_ENSURE( m_rIDataObjectOrg.get( ), "constructing CAPNDataObject with empty data object" );

    // we marshal the IDataObject interface pointer here so
    // that it can be unmarshaled multiple times when this
    // class will be used from another apartment
    IStreamPtr pStm;
    HRESULT hr = CreateStreamOnHGlobal( 0, KEEP_HGLOB_ON_RELEASE, &pStm );

    OSL_ENSURE( E_INVALIDARG != hr, "invalid args passed to CreateStreamOnHGlobal" );

    if ( SUCCEEDED( hr ) )
    {
        HRESULT hr_marshal = CoMarshalInterface(
            pStm.get(),
            __uuidof(IDataObject),
            static_cast<LPUNKNOWN>(m_rIDataObjectOrg.get()),
            MSHCTX_LOCAL,
            NULL,
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
            OSL_ENSURE(sal_False, "marshalling failed");

            #if OSL_DEBUG_LEVEL > 0
            HGLOBAL hGlobal =
            #endif
                GlobalFree(m_hGlobal);
            OSL_ENSURE(NULL == hGlobal, "GlobalFree failed");
            m_hGlobal = NULL;
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

//------------------------------------------------------------------------
// IUnknown->QueryInterface
//------------------------------------------------------------------------

STDMETHODIMP CAPNDataObject::QueryInterface( REFIID iid, LPVOID* ppvObject )
{
    OSL_ASSERT( NULL != ppvObject );

    if ( NULL == ppvObject )
        return E_INVALIDARG;

    HRESULT hr = E_NOINTERFACE;
    *ppvObject = NULL;

    if ( ( __uuidof( IUnknown ) == iid ) || ( __uuidof( IDataObject ) == iid ) )
    {
        *ppvObject = static_cast< IUnknown* >( this );
        ( (LPUNKNOWN)*ppvObject )->AddRef( );
        hr = S_OK;
    }

    return hr;
}

//------------------------------------------------------------------------
// IUnknown->AddRef
//------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CAPNDataObject::AddRef( )
{
    return static_cast< ULONG >( InterlockedIncrement( &m_nRefCnt ) );
}

//------------------------------------------------------------------------
// IUnknown->Release
//------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CAPNDataObject::Release( )
{
    // we need a helper variable because it's not allowed to access
    // a member variable after an object is destroyed
    ULONG nRefCnt = static_cast< ULONG >( InterlockedDecrement( &m_nRefCnt ) );

    if ( 0 == nRefCnt )
        delete this;

    return nRefCnt;
}

//------------------------------------------------------------------------
// IDataObject->GetData
//------------------------------------------------------------------------

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

//------------------------------------------------------------------------
// IDataObject->EnumFormatEtc
//------------------------------------------------------------------------

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

//------------------------------------------------------------------------
// IDataObject->QueryGetData
//------------------------------------------------------------------------

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

//------------------------------------------------------------------------
// IDataObject->GetDataHere
//------------------------------------------------------------------------

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

//------------------------------------------------------------------------
// IDataObject->GetCanonicalFormatEtc
//------------------------------------------------------------------------

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

//------------------------------------------------------------------------
// IDataObject->SetData
//------------------------------------------------------------------------

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

//------------------------------------------------------------------------
// IDataObject->DAdvise
//------------------------------------------------------------------------

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

//------------------------------------------------------------------------
// IDataObject->DUnadvise
//------------------------------------------------------------------------

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

//------------------------------------------------------------------------
// IDataObject->EnumDAdvise
//------------------------------------------------------------------------

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

//------------------------------------------------------------------------
// for our convenience
//------------------------------------------------------------------------

CAPNDataObject::operator IDataObject*( )
{
    return static_cast< IDataObject* >( this );
}

//------------------------------------------------------------------------
// helper function
//------------------------------------------------------------------------

HRESULT CAPNDataObject::MarshalIDataObjectIntoCurrentApartment( IDataObject** ppIDataObj )
{
    OSL_ASSERT(NULL != ppIDataObj);

    *ppIDataObj = NULL;
    HRESULT hr = E_FAIL;

    if (m_hGlobal)
    {
        IStreamPtr pStm;
        hr = CreateStreamOnHGlobal(m_hGlobal, KEEP_HGLOB_ON_RELEASE, &pStm);

        OSL_ENSURE(E_INVALIDARG != hr, "CreateStreamOnHGlobal with invalid args called");

        if (SUCCEEDED(hr))
        {
            hr = CoUnmarshalInterface(pStm.get(), __uuidof(IDataObject), (void**)ppIDataObj);
            OSL_ENSURE(CO_E_NOTINITIALIZED != hr, "COM is not initialized");
        }
    }
    return hr;
}
