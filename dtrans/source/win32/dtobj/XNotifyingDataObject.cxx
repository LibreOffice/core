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
#include <osl/diagnose.h>
#include "XNotifyingDataObject.hxx"
#include "..\clipb\WinClipbImpl.hxx"
#include "..\clipb\WinClipboard.hxx"
#include "..\..\inc\DtObjFactory.hxx"

#ifdef __MINGW32__
#define __uuidof(I) IID_##I
#endif

using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::clipboard;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Reference;


CXNotifyingDataObject::CXNotifyingDataObject(
    const IDataObjectPtr& aIDataObject,
    const Reference< XTransferable >& aXTransferable,
    const Reference< XClipboardOwner >& aXClipOwner,
    CWinClipbImpl* theWinClipImpl ) :
    m_nRefCnt( 0 ),
    m_aIDataObject( aIDataObject ),
    m_XTransferable( aXTransferable ),
    m_XClipboardOwner( aXClipOwner ),
    m_pWinClipImpl( theWinClipImpl )
{
}

STDMETHODIMP CXNotifyingDataObject::QueryInterface( REFIID iid, LPVOID* ppvObject )
{
    if ( NULL == ppvObject )
        return E_INVALIDARG;

    HRESULT hr = E_NOINTERFACE;

    *ppvObject = NULL;
    if ( ( __uuidof( IUnknown ) == iid ) ||
         ( __uuidof( IDataObject ) == iid ) )
    {
        *ppvObject = static_cast< IUnknown* >( this );
        ( (LPUNKNOWN)*ppvObject )->AddRef( );
        hr = S_OK;
    }

    return hr;
}

STDMETHODIMP_(ULONG) CXNotifyingDataObject::AddRef( )
{
    return static_cast< ULONG >( InterlockedIncrement( &m_nRefCnt ) );
}

STDMETHODIMP_(ULONG) CXNotifyingDataObject::Release( )
{
    ULONG nRefCnt =
        static_cast< ULONG >( InterlockedDecrement( &m_nRefCnt ) );

    if ( 0 == nRefCnt )
    {
        if ( m_pWinClipImpl )
            m_pWinClipImpl->onReleaseDataObject( this );

        delete this;
    }

    return nRefCnt;
}

STDMETHODIMP CXNotifyingDataObject::GetData( LPFORMATETC pFormatetc, LPSTGMEDIUM pmedium )
{
    return m_aIDataObject->GetData(pFormatetc, pmedium);
}

STDMETHODIMP CXNotifyingDataObject::EnumFormatEtc(
    DWORD dwDirection, IEnumFORMATETC** ppenumFormatetc )
{
    return m_aIDataObject->EnumFormatEtc(dwDirection, ppenumFormatetc);
}

STDMETHODIMP CXNotifyingDataObject::QueryGetData( LPFORMATETC pFormatetc )
{
    return m_aIDataObject->QueryGetData(pFormatetc);
}

STDMETHODIMP CXNotifyingDataObject::GetDataHere( LPFORMATETC lpFetc, LPSTGMEDIUM lpStgMedium )
{
    return m_aIDataObject->GetDataHere(lpFetc, lpStgMedium);
}

STDMETHODIMP CXNotifyingDataObject::GetCanonicalFormatEtc( LPFORMATETC lpFetc, LPFORMATETC lpCanonicalFetc )
{
    return m_aIDataObject->GetCanonicalFormatEtc(lpFetc, lpCanonicalFetc);
}

STDMETHODIMP CXNotifyingDataObject::SetData( LPFORMATETC lpFetc, LPSTGMEDIUM lpStgMedium, BOOL bRelease )
{
    return m_aIDataObject->SetData( lpFetc, lpStgMedium, bRelease );
}

STDMETHODIMP CXNotifyingDataObject::DAdvise(
    LPFORMATETC lpFetc, DWORD advf, LPADVISESINK lpAdvSink, DWORD* pdwConnection )
{
    return m_aIDataObject->DAdvise( lpFetc, advf, lpAdvSink, pdwConnection );
}

STDMETHODIMP CXNotifyingDataObject::DUnadvise( DWORD dwConnection )
{
    return m_aIDataObject->DUnadvise( dwConnection );
}

STDMETHODIMP CXNotifyingDataObject::EnumDAdvise( LPENUMSTATDATA * ppenumAdvise )
{
    return m_aIDataObject->EnumDAdvise( ppenumAdvise );
}

CXNotifyingDataObject::operator IDataObject*( )
{
    return static_cast< IDataObject* >( this );
}

void SAL_CALL CXNotifyingDataObject::lostOwnership( )
{
    try
    {
        if (m_XClipboardOwner.is())
            m_XClipboardOwner->lostOwnership(
                static_cast<XClipboardEx*>(m_pWinClipImpl->m_pWinClipboard ), m_XTransferable);
    }
    catch(RuntimeException&)
    {
        OSL_ENSURE( sal_False, "RuntimeException caught" );
    }
}
