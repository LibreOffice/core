/*************************************************************************
 *
 *  $RCSfile: XTDataObject.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: tra $ $Date: 2000-10-19 08:53:23 $
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

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _TWRAPPERDATAOBJECT_HXX_
#include "XTDataObject.hxx"
#endif

#include <windows.h>
#include <ole2.h>
#include <stl/memory>

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------


//============================================================================
// OTWrapperDataObject
//============================================================================

//------------------------------------------------------------------------
// ctor
//------------------------------------------------------------------------

CXTDataObject::CXTDataObject( ) :
    m_nRefCnt( 0 )
{
}

//------------------------------------------------------------------------
// dtor
//------------------------------------------------------------------------

CXTDataObject::~CXTDataObject( )
{
}

//------------------------------------------------------------------------
// IUnknown->QueryInterface
//------------------------------------------------------------------------

STDMETHODIMP CXTDataObject::QueryInterface( REFIID iid, LPVOID* ppvObject )
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

STDMETHODIMP_(ULONG) CXTDataObject::AddRef( )
{
    return static_cast< ULONG >( InterlockedIncrement( &m_nRefCnt ) );
}

//------------------------------------------------------------------------
// IUnknown->Release
//------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CXTDataObject::Release( )
{
    // we need a helper variable because it's
    // not allowed to access a member variable
    // after an object is destroyed
    ULONG nRefCnt = static_cast< ULONG >( InterlockedDecrement( &m_nRefCnt ) );

    if ( 0 == nRefCnt )
    {
        delete this;
    }

    return nRefCnt;
}

//------------------------------------------------------------------------
// IDataObject->GetData
// warning: 'goto' ahead (to easy error handling without using exceptions)
//------------------------------------------------------------------------

STDMETHODIMP CXTDataObject::GetData(LPFORMATETC pFormatetc, LPSTGMEDIUM pmedium )
{
    OSL_ASSERT( ( NULL != pFormatetc ) &&
                ( !IsBadReadPtr( (LPVOID)pFormatetc, sizeof( FORMATETC ) ) ) );
    OSL_ASSERT( ( NULL != pmedium ) &&
                ( !IsBadWritePtr( (LPVOID)pmedium, sizeof( STGMEDIUM ) ) ) );

    if ( ( NULL == pFormatetc ) || ( NULL == pmedium ) )
        return E_INVALIDARG;

    HRESULT hr = S_OK;

    if ( CF_TEXT == pFormatetc->cfFormat )
    {
        char     buff[] = "Hello World, How are you!";
        LPSTREAM lpStream;

        hr = CreateStreamOnHGlobal( NULL, FALSE, &lpStream );
        if ( SUCCEEDED( hr ) )
        {
            hr = lpStream->Write( buff, sizeof( buff ) * sizeof( char ), NULL );
            if ( SUCCEEDED( hr ) )
            {
                HGLOBAL hGlob;

                GetHGlobalFromStream( lpStream, &hGlob );

                pmedium->tymed          = TYMED_HGLOBAL;
                pmedium->hGlobal        = hGlob;
                pmedium->pUnkForRelease = NULL;
            }
            lpStream->Release( );
        }
        else
        {
            pmedium->tymed = TYMED_NULL;
        }
    }

    return hr;
}

//------------------------------------------------------------------------
// IDataObject->EnumFormatEtc
//------------------------------------------------------------------------

STDMETHODIMP CXTDataObject::EnumFormatEtc( DWORD dwDirection, IEnumFORMATETC** ppenumFormatetc )
{
    if ( ( NULL == ppenumFormatetc ) || ( DATADIR_SET == dwDirection ) )
        return E_INVALIDARG;

    *ppenumFormatetc = NULL;

    HRESULT hr = E_FAIL;

    if ( DATADIR_GET == dwDirection )
    {
        *ppenumFormatetc = new CEnumFormatEtc( this );
        static_cast< LPUNKNOWN >( *ppenumFormatetc )->AddRef( );
        hr = S_OK;
    }

    return hr;
}

//------------------------------------------------------------------------
// IDataObject->QueryGetData
//------------------------------------------------------------------------

STDMETHODIMP CXTDataObject::QueryGetData( LPFORMATETC pFormatetc )
{
    return E_NOTIMPL;
}

//------------------------------------------------------------------------
// IDataObject->GetDataHere
//------------------------------------------------------------------------

STDMETHODIMP CXTDataObject::GetDataHere( LPFORMATETC, LPSTGMEDIUM )
{
    return E_NOTIMPL;
}

//------------------------------------------------------------------------
// IDataObject->GetCanonicalFormatEtc
//------------------------------------------------------------------------

STDMETHODIMP CXTDataObject::GetCanonicalFormatEtc( LPFORMATETC, LPFORMATETC )
{
    return E_NOTIMPL;
}

//------------------------------------------------------------------------
// IDataObject->SetData
//------------------------------------------------------------------------

STDMETHODIMP CXTDataObject::SetData( LPFORMATETC, LPSTGMEDIUM, BOOL )
{
    return E_NOTIMPL;
}

//------------------------------------------------------------------------
// IDataObject->DAdvise
//------------------------------------------------------------------------

STDMETHODIMP CXTDataObject::DAdvise( LPFORMATETC, DWORD, LPADVISESINK, DWORD * )
{
    return E_NOTIMPL;
}

//------------------------------------------------------------------------
// IDataObject->DUnadvise
//------------------------------------------------------------------------

STDMETHODIMP CXTDataObject::DUnadvise( DWORD )
{
    return E_NOTIMPL;
}

//------------------------------------------------------------------------
// IDataObject->EnumDAdvise
//------------------------------------------------------------------------

STDMETHODIMP CXTDataObject::EnumDAdvise( LPENUMSTATDATA * )
{
    return E_NOTIMPL;
}

//------------------------------------------------------------------------
// for our convenience
//------------------------------------------------------------------------

CXTDataObject::operator IDataObject*( )
{
    return static_cast< IDataObject* >( this );
}


//============================================================================
// CEnumFormatEtc
//============================================================================


//----------------------------------------------------------------------------
// ctor
//----------------------------------------------------------------------------

CEnumFormatEtc::CEnumFormatEtc( LPUNKNOWN pUnkDataObj ) :
    m_nRefCnt( 0 ),
    m_pUnkDataObj( pUnkDataObj ),
    m_nCurrentPos( 0 )
{
    m_cfFormats[0] = CF_TEXT;
    m_cfFormats[1] = CF_UNICODETEXT;
}

//----------------------------------------------------------------------------
// dtor
//----------------------------------------------------------------------------

CEnumFormatEtc::~CEnumFormatEtc( )
{
}

//----------------------------------------------------------------------------
// IUnknown->QueryInterface
//----------------------------------------------------------------------------

STDMETHODIMP CEnumFormatEtc::QueryInterface( REFIID iid, LPVOID* ppvObject )
{
    if ( NULL == ppvObject )
        return E_INVALIDARG;

    HRESULT hr = E_NOINTERFACE;

    *ppvObject = NULL;

    if ( ( __uuidof( IUnknown ) == iid ) || ( __uuidof( IEnumFORMATETC ) == iid ) )
    {
        *ppvObject = static_cast< IUnknown* >( this );
        static_cast< LPUNKNOWN >( *ppvObject )->AddRef( );
        hr = S_OK;
    }

    return hr;
}

//----------------------------------------------------------------------------
// IUnknown->AddRef
//----------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CEnumFormatEtc::AddRef( )
{
    // keep the dataobject alive
    m_pUnkDataObj->AddRef( );
    return InterlockedIncrement( &m_nRefCnt );
}

//----------------------------------------------------------------------------
// IUnknown->Release
//----------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CEnumFormatEtc::Release( )
{
    // release the outer dataobject
    m_pUnkDataObj->Release( );

    // we need a helper variable because it's
    // not allowed to access a member variable
    // after an object is destroyed
    ULONG nRefCnt = InterlockedDecrement( &m_nRefCnt );
    if ( 0 == nRefCnt )
        delete this;

    return nRefCnt;
}

//----------------------------------------------------------------------------
// IEnumFORMATETC->Next
//----------------------------------------------------------------------------

STDMETHODIMP CEnumFormatEtc::Next( ULONG celt, LPFORMATETC rgelt, ULONG* pceltFetched )
{
    OSL_ASSERT( ( ( celt > 0 ) && ( NULL != rgelt ) ) ||
                ( ( 0 == celt ) && ( NULL == rgelt ) ) );

    if ( ( 0 != celt ) && ( NULL == rgelt ) )
        return E_INVALIDARG;

    ULONG   ulFetched = 0;
    ULONG   ulToFetch = celt;
    HRESULT hr        = S_FALSE;

    while( ( m_nCurrentPos < sizeof( m_cfFormats ) ) && ( ulToFetch > 0 ) )
    {
        OSL_ASSERT( !IsBadWritePtr( (LPVOID)rgelt, sizeof( FORMATETC ) ) );

        rgelt->cfFormat = m_cfFormats[m_nCurrentPos];
        rgelt->ptd      = NULL;
        rgelt->dwAspect = DVASPECT_CONTENT;
        rgelt->lindex   = -1;
        rgelt->tymed    = TYMED_HGLOBAL;

        ++m_nCurrentPos;
        ++rgelt;
        --ulToFetch;
        ++ulFetched;
    }

    if ( ulFetched == celt )
        hr = S_OK;

    if ( NULL != pceltFetched )
    {
        OSL_ASSERT( !IsBadWritePtr( (LPVOID)pceltFetched, sizeof( ULONG ) ) );
        *pceltFetched = ulFetched;
    }

    return hr;
}

//----------------------------------------------------------------------------
// IEnumFORMATETC->Skip
//----------------------------------------------------------------------------

STDMETHODIMP CEnumFormatEtc::Skip( ULONG celt )
{
    HRESULT hr = S_FALSE;

    if ( ( m_nCurrentPos + celt ) < sizeof( m_cfFormats ) )
    {
        m_nCurrentPos += celt;
        hr = S_OK;
    }

    return hr;
}

//----------------------------------------------------------------------------
// IEnumFORMATETC->Reset
//----------------------------------------------------------------------------

STDMETHODIMP CEnumFormatEtc::Reset( )
{
    m_nCurrentPos = 0;
    return S_OK;
}

//----------------------------------------------------------------------------
// IEnumFORMATETC->Clone
//----------------------------------------------------------------------------

STDMETHODIMP CEnumFormatEtc::Clone( IEnumFORMATETC** ppenum )
{
    OSL_ASSERT( NULL != ppenum );

    if ( NULL == ppenum )
        return E_INVALIDARG;

    HRESULT hr = E_FAIL;

    *ppenum = NULL;

    CEnumFormatEtc* pCEnumFEtc = new CEnumFormatEtc( m_pUnkDataObj );
    if ( NULL != pCEnumFEtc )
    {
        pCEnumFEtc->m_nCurrentPos = m_nCurrentPos;
        *ppenum = static_cast< IEnumFORMATETC* >( pCEnumFEtc );
        static_cast< LPUNKNOWN >( *ppenum )->AddRef( );
        hr = NOERROR;
    }

    return hr;
}

